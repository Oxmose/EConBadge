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
#include <string>          /* std::string */
#include <Types.h>         /* Defined Types */
#include <HWMgr.h>         /* Hardware manager */
#include <Storage.h>       /* Storage service */
#include <BlueToothMgr.h>  /* Bluetooth Manager */
#include <WaveshareEInk.h> /* EInk Driver */

/* Header File */
#include <WaveshareEInkMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Size of an EInk image in bytes. */
#define EINK_IMAGE_SIZE ((EPD_WIDTH * EPD_HEIGHT) / 2)

/** @brief Read image timeout. */
#define IMAGE_READ_TIMEOUT 5000000 /* 5 seconds */

/** @brief Size in bytes of the internal buffer used for transations. */
#define INTERNAL_BUFFER_SIZE 16384

/** @brief Path to the images directory. */
#define IMAGE_DIR_PATH "/images"

/** @brief Path to the current image name file. */
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

EInkDisplayManager::EInkDisplayManager(BluetoothManager* pBtMgr)
{
    pBtMgr_ = pBtMgr;
    pStore_ = Storage::GetInstance();
}

void EInkDisplayManager::Init(void)
{
    /* Init EInk display */
    eInkDriver_.Init();
    eInkDriver_.Sleep();

    /* Get the current image name if stored */
    if(!pStore_->CreateDirectory(IMAGE_DIR_PATH))
    {
        LOG_ERROR("Failed to create image directory.\n");
    }
    pStore_->GetContent(
        CURRENT_IMG_NAME_FILE_PATH,
        "",
        currentImageName_,
        true
    );
}

void EInkDisplayManager::GetDisplayedImageName(SCommandResponse& rResponse) const
{
    size_t size;

    size = MIN(currentImageName_.size() + 1, COMMAND_RESPONSE_LENGTH);
    rResponse.header.errorCode = NO_ERROR;
    rResponse.header.size = size;
    memcpy(rResponse.pResponse, currentImageName_.c_str(), size);
}

void EInkDisplayManager::Clear(SCommandResponse& rResponse)
{
    if(pStore_->SetContent(CURRENT_IMG_NAME_FILE_PATH, "", true))
    {
        currentImageName_ = "";
        rResponse.header.errorCode = NO_ERROR;
        rResponse.header.size = 0;

        /* Reset the EInk display */
        eInkDriver_.Init();
        eInkDriver_.Clear(EPD_5IN65F_WHITE);
        eInkDriver_.Sleep();
    }
    else
    {
        rResponse.header.errorCode = IMG_NAME_UDPATE_FAIL;
        rResponse.header.size = 0;
    }
}

void EInkDisplayManager::RemoveImage(const std::string& rkFilename,
                                     SCommandResponse&  rResponse)
{
    bool status;

    rResponse.header.errorCode = NO_ERROR;
    rResponse.header.size = 0;

    if(rkFilename == currentImageName_)
    {
        Clear(rResponse);
    }

    if(rResponse.header.errorCode == NO_ERROR)
    {
        status = pStore_->Remove(
            IMAGE_DIR_PATH + std::string("/") + rkFilename
        );
        if(!status)
        {
            rResponse.header.errorCode = ACTION_FAILED;
        }
    }
}

void EInkDisplayManager::SetDisplayedImage(const std::string& rkFilename,
                                           SCommandResponse&  rResponse)
{
    size_t      toRead;
    size_t      offset;
    ssize_t     readBytes;
    uint32_t    leftToTransfer;
    std::string formatedName;
    uint8_t*    pBuffer;
    FsFile      file;

    if(rkFilename == currentImageName_)
    {
        rResponse.header.errorCode = NO_ERROR;
        rResponse.header.size = 0;
        return;
    }

    if(rkFilename.size() == 0)
    {
        rResponse.header.errorCode = FILE_NOT_FOUND;
        rResponse.header.size = 0;
        return;
    }
    formatedName = IMAGE_DIR_PATH + std::string("/") + rkFilename;

    /* Check if file exists */
    if(!pStore_->FileExists(formatedName))
    {
        rResponse.header.errorCode = FILE_NOT_FOUND;
        rResponse.header.size = 0;
        return;
    }

    /* Allocate buffer */
    pBuffer = new uint8_t[INTERNAL_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        rResponse.header.errorCode = NO_MORE_MEMORY;
        rResponse.header.size = 0;
        return;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_READ);
    if(!file)
    {
        rResponse.header.errorCode = OPEN_FILE_FAILED;
        rResponse.header.size = 0;
        delete[] pBuffer;
        return;
    }

    leftToTransfer = EINK_IMAGE_SIZE;

    /* Init the EInk display */
    eInkDriver_.Init();
    eInkDriver_.DisplayInitTrans();

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
            eInkDriver_.DisplayPerformTrans(pBuffer, readBytes);
            LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);

            leftToTransfer -= readBytes;
            offset += readBytes;
        }
        else
        {
            break;
        }
    }

    /* End EINK transation */
    eInkDriver_.DisplayEndTrans();
    eInkDriver_.Sleep();

    if(leftToTransfer == 0)
    {
        currentImageName_ = rkFilename;
        if(!pStore_->SetContent(CURRENT_IMG_NAME_FILE_PATH, rkFilename, true))
        {
            LOG_ERROR("Could not save current image name.\n");
            rResponse.header.errorCode = IMG_NAME_UDPATE_FAIL;
            rResponse.header.size = 0;
        }
        else
        {
            rResponse.header.errorCode = NO_ERROR;
            rResponse.header.size = 0;
        }
    }
    else
    {
        rResponse.header.errorCode = WRITE_FILE_FAILED;
        rResponse.header.size = 0;
    }

    LOG_DEBUG("Updated EINK Image\n");

    delete[] pBuffer;
    file.close();
}

void EInkDisplayManager::DisplayNewImage(const std::string& rkFilename,
                                         SCommandResponse&  rResponse)
{
    size_t      toRead;
    uint32_t    leftToTransfer;
    ssize_t     readBytes;
    ssize_t     wroteBytes;
    size_t      offset;
    uint64_t    timeout;
    std::string formatedName;
    uint8_t*    pBuffer;
    FsFile      file;
    EErrorCode  retCode;

    if(rkFilename.size() == 0)
    {
        rResponse.header.errorCode = FILE_NOT_FOUND;
        rResponse.header.size = 0;
        return;
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
        rResponse.header.errorCode = NO_MORE_MEMORY;
        rResponse.header.size = 0;
        return;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_WRITE);
    if(!file)
    {
        rResponse.header.errorCode = OPEN_FILE_FAILED;
        rResponse.header.size = 0;
        delete[] pBuffer;
        return;
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
            /* Write the update file */
            offset = 0;
            do
            {
                wroteBytes = file.write(pBuffer + offset, readBytes);
                if(wroteBytes < 0)
                {
                    retCode = WRITE_FILE_FAILED;
                    break;
                }
                offset += wroteBytes;
                readBytes -= wroteBytes;
                leftToTransfer -= wroteBytes;
            } while(readBytes != 0);
        }
        LOG_DEBUG("Downloading EINK Image. Left: %d\n", leftToTransfer);


        if(HWManager::GetTime() > timeout)
        {
            LOG_ERROR("Timeout on image receive\n");
            break;
        }
    }

    delete[] pBuffer;
    file.close();

    rResponse.header.errorCode = retCode;
    rResponse.header.size = 0;

    if(retCode != NO_ERROR)
    {
        pStore_->Remove(formatedName);
    }
    else
    {
        SetDisplayedImage(rkFilename, rResponse);
    }
}

void EInkDisplayManager::SendDisplayedImage(SCommandResponse& rResponse) const
{
    size_t      toRead;
    uint32_t    leftToTransfer;
    ssize_t     readBytes;
    ssize_t     wroteBytes;
    uint64_t    timeout;
    std::string formatedName;
    uint8_t*    pBuffer;
    FsFile      file;
    EErrorCode  retCode;

    /* Get current image */
    pStore_->GetContent(CURRENT_IMG_NAME_FILE_PATH, "", formatedName, true);

    formatedName = IMAGE_DIR_PATH + std::string("/") + formatedName;

    if(!pStore_->FileExists(formatedName))
    {
        rResponse.header.errorCode = FILE_NOT_FOUND;
        rResponse.header.size = 0;
        return;
    }

    LOG_DEBUG("Sending image %s\n", formatedName.c_str());

    /* Allocate buffer */
    pBuffer = new uint8_t[INTERNAL_BUFFER_SIZE];
    if(pBuffer == nullptr)
    {
        rResponse.header.errorCode = NO_MORE_MEMORY;
        rResponse.header.size = 0;
        return;
    }

    /* Open file */
    file = pStore_->Open(formatedName, FILE_READ);
    if(!file)
    {
        rResponse.header.errorCode = OPEN_FILE_FAILED;
        rResponse.header.size = 0;
        delete[] pBuffer;
        return;
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
                retCode = TRANS_SEND_FAILED;
                LOG_ERROR("Error while uploading image.");
                break;
            }
            leftToTransfer -= readBytes;
        }

        LOG_DEBUG("Uploading EINK Image. Left: %d\n", leftToTransfer);


        if(HWManager::GetTime() > timeout)
        {
            LOG_ERROR("Timeout on image send\n");
            break;
        }
    }

    delete[] pBuffer;
    file.close();

    rResponse.header.errorCode = retCode;
    rResponse.header.size = 0;
}