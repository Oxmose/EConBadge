/*******************************************************************************
 * @file WaveshareEInkMgr.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 21/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the Waveshare EInk Display Driver.
 *
 * @details This file contains the Waveshare EInk Display Driver. The file
 * provides the services to update the screen, enable and disable it.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>   /* String manipulation*/
#include <Types.h>   /* Defined Types */
#include <Logger.h>  /* Logger service */
#include <HWLayer.h> /* Hardware Services */
#include <Storage.h> /* Storage service */

/* Header File */
#include <WaveshareEInkMgr.h>


/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CEINK EInkDisplayManager

#define EINK_DISPLAY_WIDTH  600
#define EINK_DISPLAY_HEIGHT 448
#define EINK_IMAGE_SIZE     ((EINK_DISPLAY_WIDTH * EINK_DISPLAY_HEIGHT) / 2)

#define IMAGE_READ_TIMEOUT 5000 /* 5 seconds */

#define INTERNAL_BUFFER_SIZE 16384

#define IMAGE_DIR_PATH "/images"
#define CURRENT_IMG_NAME_FILE_PATH "/currimg"

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/

/* None */

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

CEINK::EInkDisplayManager(BluetoothManager* pBtMgr)
{
    pBtMgr_ = pBtMgr;
    updateAction_.action = EINK_NO_ACTION;
}

void CEINK::Init(void)
{
    eInkDriver_.Init(false);
    eInkDriver_.Sleep();

    /* Get the current image name if stored */
    pStore_ = Storage::GetInstance();
    if(!pStore_->CreateDirectory(IMAGE_DIR_PATH))
    {
        LOG_ERROR("Failed to create image direcotry.");
    }
    pStore_->GetDisplayedImageName(currentImageName_);
}

void CEINK::Update(void)
{
    SCommandResponse reponse;

    /* This function performs the long actions that cannot be done during
     * their commanding.
     */
    switch(updateAction_.action)
    {
        case EINK_CLEAR:
            reponse.errorCode = InternalClear(&reponse);
            break;
        case EINK_SET_DISPLAYED_IMAGE:
            reponse.errorCode = InternalSetDisplayedImage(
                updateAction_.parameter,
                &reponse
            );
            break;
        case EINK_DISPLAY_NEW_IMAGE:
            reponse.errorCode = InternalDisplayNewImage(
                updateAction_.parameter,
                &reponse
            );
            break;
        case EINK_SEND_DISPLAYED_IMAGE:
            reponse.errorCode = InternalSendDisplayedImage(&reponse);
            break;
        case EINK_NO_ACTION:
            /* Nothing to do */
            return;
        default:
            reponse.errorCode = INVALID_PARAM;
            memcpy(reponse.response, "Unknown EINK action", 20);
            reponse.size = 20;
    }
    updateAction_.action = EINK_NO_ACTION;

    /* Send the long action result */
    pBtMgr_->SendCommandResponse(reponse);
}

EErrorCode CEINK::Clear(void)
{
    /* Long action, defer */
    updateAction_.action = EINK_CLEAR;
    return NO_ERROR;
}

EErrorCode CEINK::SetDisplayedImage(const std::string& rkFilename)
{
    /* Long action, defer */
    updateAction_.action = EINK_SET_DISPLAYED_IMAGE;
    updateAction_.parameter = rkFilename;

    return NO_ERROR;
}

EErrorCode CEINK::DisplayNewImage(const std::string& rkFilename)
{
    /* Long action, defer */
    updateAction_.action = EINK_DISPLAY_NEW_IMAGE;
    updateAction_.parameter = rkFilename;

    return NO_ERROR;
}

EErrorCode CEINK::SendDisplayedImage(void)
{
    /* Long action, defer */
    updateAction_.action = EINK_SEND_DISPLAYED_IMAGE;

    return NO_ERROR;
}

void CEINK::GetDisplayedImageName(std::string& rFileName) const
{
    rFileName = currentImageName_;
}

EErrorCode CEINK::InternalClear(SCommandResponse* pResponse)
{
    eInkDriver_.Init(true);
    eInkDriver_.Clear(EPD_5IN65F_WHITE);
    eInkDriver_.Sleep();

    currentImageName_ = "";
    if(pStore_->SetCurrentImageName(""))
    {
        pResponse->size = 0;
        return NO_ERROR;
    }
    else
    {
        memcpy(pResponse->response, "Failed to reset current image.", 31);
        pResponse->size = 31;
        return ACTION_FAILED;
    }
}

EErrorCode CEINK::InternalSetDisplayedImage(const std::string& rkFilename,
                                            SCommandResponse* pResponse)
{
    size_t      toRead;
    size_t      offset;
    ssize_t     readBytes;
    uint32_t    leftToTransfer;
    std::string formatedName;
    uint8_t*    pBuffer;
    File        file;
    EErrorCode  retCode;

    if(rkFilename.size() == 0)
    {
        memcpy(pResponse->response, "Invalid file name.", 19);
        pResponse->size = 19;
        return INVALID_PARAM;
    }
    formatedName = IMAGE_DIR_PATH + std::string("/") + rkFilename;

    /* Check if file exists */
    if(!pStore_->FileExists(formatedName))
    {
        memcpy(pResponse->response, "File does not exists.", 22);
        pResponse->size = 22;
        return FILE_NOT_FOUND;
    }

    /* Allocate buffer */
    pBuffer = new uint8_t[INTERNAL_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        memcpy(pResponse->response, "Failed to allocate memory.", 27);
        pResponse->size = 27;
        return NO_MORE_MEMORY;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_READ);
    if(!file)
    {
        memcpy(pResponse->response, "Failed to open file.", 21);
        pResponse->size = 21;
        delete[] pBuffer;
        return ACTION_FAILED;
    }

    leftToTransfer = EINK_IMAGE_SIZE;

    eInkDriver_.Init(true);
    eInkDriver_.EPD_5IN65F_DisplayInitTrans();

    LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);

    /* Get the full image data */
    offset = 0;
    while(leftToTransfer > 0)
    {
        if(leftToTransfer < INTERNAL_BUFFER_SIZE)
        {
            toRead = leftToTransfer;
        }
        else
        {
            toRead = INTERNAL_BUFFER_SIZE;
        }

        readBytes = file.read(pBuffer, toRead);
        if(readBytes > 0)
        {
            eInkDriver_.EPD_5IN65F_DisplayPerformTrans(
                (char*)pBuffer,
                readBytes
            );
            LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);

            leftToTransfer -= readBytes;
            offset += readBytes;
        }
        else
        {
            break;
        }
    }

    eInkDriver_.EPD_5IN65F_DisplayEndTrans();
    eInkDriver_.Sleep();

    if(leftToTransfer == 0)
    {
        currentImageName_ = rkFilename;
        if(!pStore_->SetContent(CURRENT_IMG_NAME_FILE_PATH, rkFilename, true))
        {
            LOG_ERROR("Could not save current image name.\n");
        }
        retCode = NO_ERROR;
    }
    else
    {
        memcpy(pResponse->response, "Failed to write file.", 22);
        pResponse->size = 22;
        retCode = ACTION_FAILED;
    }

    LOG_DEBUG("Updated EINK Image\n");

    delete[] pBuffer;
    if(!pStore_->Close(file))
    {
        LOG_ERROR("Failed to close file.");
    }

    return retCode;
}

EErrorCode CEINK::InternalDisplayNewImage(const std::string& rkFilename,
                                          SCommandResponse* pResponse)
{
    size_t      toRead;
    uint32_t    leftToTransfer;
    ssize_t     readBytes;
    ssize_t     wroteBytes;
    uint64_t    timeout;
    std::string formatedName;
    uint8_t*    pBuffer;
    File        file;
    EErrorCode  retCode;

    if(rkFilename.size() == 0)
    {
        memcpy(pResponse->response, "Invalid file name.", 19);
        pResponse->size = 19;
        return INVALID_PARAM;
    }
    formatedName = IMAGE_DIR_PATH + std::string("/") + rkFilename;

    /* Check if file exists */
    if(pStore_->FileExists(formatedName))
    {
        pStore_->Remove(formatedName);
    }

    /* Allocate buffer */
    pBuffer = new uint8_t[INTERNAL_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        memcpy(pResponse->response, "Failed to allocate memory.", 27);
        pResponse->size = 27;
        return NO_MORE_MEMORY;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_WRITE);
    if(!file)
    {
        memcpy(pResponse->response, "Failed to open file.", 21);
        pResponse->size = 21;
        delete[] pBuffer;
        return ACTION_FAILED;
    }

    leftToTransfer = EINK_IMAGE_SIZE;
    LOG_DEBUG("Downloading image %s\n", rkFilename.c_str());

    /* Get the full image data */
    retCode = NO_ERROR;
    while(leftToTransfer > 0)
    {
        timeout = HWManager::GetTime() + IMAGE_READ_TIMEOUT;
        if(leftToTransfer < INTERNAL_BUFFER_SIZE)
        {
            toRead = leftToTransfer;
        }
        else
        {
            toRead = INTERNAL_BUFFER_SIZE;
        }

        readBytes = pBtMgr_->ReceiveData(pBuffer, toRead);
        if(readBytes > 0)
        {
            wroteBytes = file.write(pBuffer, readBytes);
            if(wroteBytes != readBytes)
            {
                retCode = ACTION_FAILED;
                LOG_ERROR("Error while storing downloaded image.");
                break;
            }
        }
        leftToTransfer -= toRead;
        LOG_DEBUG("Downloading EINK Image. Left: %d\n", leftToTransfer);


        if(HWManager::GetTime() > timeout)
        {
            LOG_ERROR("Timeout on image receive\n");
            break;
        }
    }

    delete[] pBuffer;
    if(retCode != NO_ERROR)
    {
        memcpy(
            pResponse->response,
            "Error while storing downloaded image.",
            38
        );
        pResponse->size = 38;
        pStore_->RemoveImage(formatedName);
    }

    if(!pStore_->Close(file))
    {
        LOG_ERROR("Failed to close file.");
    }

    if(retCode == NO_ERROR)
    {
        return SetDisplayedImage(rkFilename);
    }
    else
    {
        return retCode;
    }
}

EErrorCode CEINK::InternalSendDisplayedImage(SCommandResponse* pResponse)
{
    size_t      toRead;
    uint32_t    leftToTransfer;
    ssize_t     readBytes;
    ssize_t     wroteBytes;
    uint64_t    timeout;
    std::string formatedName;
    uint8_t*    pBuffer;
    File        file;
    EErrorCode  retCode;

    /* Get current image */
    pStore_->GetContent(CURRENT_IMG_NAME_FILE_PATH, "", formatedName, true);

    formatedName = IMAGE_DIR_PATH + std::string("/") + formatedName;

    if(!pStore_->FileExists(formatedName))
    {
        memcpy(pResponse->response, "File not found.", 16);
        pResponse->size = 16;
        return FILE_NOT_FOUND;
    }

    LOG_DEBUG("Sending image %s\n", formatedName.c_str());

    /* Allocate buffer */
    pBuffer = new uint8_t[INTERNAL_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        memcpy(pResponse->response, "Failed to allocate memory.", 27);
        pResponse->size = 27;
        return NO_MORE_MEMORY;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_READ);
    if(!file)
    {
        memcpy(pResponse->response, "Failed to open file.", 21);
        pResponse->size = 21;
        delete[] pBuffer;
        return ACTION_FAILED;
    }

    leftToTransfer = EINK_IMAGE_SIZE;
    LOG_DEBUG("Uploading image %s\n", formatedName.c_str());

    /* Get the full image data */
    retCode = NO_ERROR;
    while(leftToTransfer > 0)
    {
        timeout = HWManager::GetTime() + IMAGE_READ_TIMEOUT;
        if(leftToTransfer < INTERNAL_BUFFER_SIZE)
        {
            toRead = leftToTransfer;
        }
        else
        {
            toRead = INTERNAL_BUFFER_SIZE;
        }

        readBytes = file.read(pBuffer, toRead);
        if(readBytes > 0)
        {
            wroteBytes = pBtMgr_->SendData(pBuffer, readBytes);
            if(wroteBytes != readBytes)
            {
                retCode = ACTION_FAILED;
                memcpy(pResponse->response, "Failed to send file.", 21);
                pResponse->size = 21;
                LOG_ERROR("Error while uploading image.");
                break;
            }
        }

        leftToTransfer -= toRead;
        LOG_DEBUG("Uploading EINK Image. Left: %d\n", leftToTransfer);


        if(HWManager::GetTime() > timeout)
        {
            LOG_ERROR("Timeout on image send\n");
            break;
        }
    }

    delete[] pBuffer;
    if(!pStore_->Close(file))
    {
        LOG_ERROR("Failed to close file.");
    }

    return retCode;
}

#undef CEINK