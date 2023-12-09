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
#include <cstring>            /* String manipulation*/
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logger service */
#include <HWLayer.h>          /* Hardware Services */
#include <Storage.h>          /* Storage service */

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

#define IMAGE_READ_TIMEOUT 15000 /* 5 seconds */

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

CEINK::EInkDisplayManager(SystemState * systemState, BluetoothManager * btMgr)
{
    systemState_ = systemState;
    btMgr_       = btMgr;
}

CEINK::~EInkDisplayManager(void)
{

}

void CEINK::Init(void)
{
    eInkDriver_.Init(false);
    eInkDriver_.Sleep();

    /* Get the current image name if stored */
    Storage::GetInstance()->GetCurrentImageName(currentImageName_);
}

void CEINK::Update(void)
{
    EEinkAction action;
    char        filename[COMMAND_DATA_SIZE];

    /* Check for new command */
    action = systemState_->ConsumeEInkAction((uint8_t*)filename);
    switch(action)
    {
        case EEinkAction::EINK_CLEAR:
            Clear();
            break;
        case EEinkAction::EINK_UPDATE:
            FormatFilename(filename);
            DownloadAndUpdateImage(filename);
            break;
        case EEinkAction::EINK_SELECT_LOADED:
            FormatFilename(filename);
            SetDisplayedImage(filename);
            systemState_->EnqueueResponse((uint8_t*)"OK", 2);
            break;
        case EEinkAction::EINK_SEND_CURRENT_IMAGE:
            FormatFilename(filename);
            SendImage(filename);
            break;
        default:
            break;
    }
}

void CEINK::RequestClear(void)
{
    Clear();
}

void CEINK::SetDisplayedImage(const char * filename)
{
    size_t   toRead;
    size_t   offset;
    bool     status;

    uint32_t   leftToTransfer = EINK_IMAGE_SIZE;

    Storage * store;

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Cannot display new image: empty image name\n");
    }

    LOG_DEBUG("Updating eInk with image %s\n", filename);

    store = Storage::GetInstance();

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

        status = store->ReadImagePart(filename, offset, internalBuffer_, toRead);
        if(status && toRead > 0)
        {
            offset += toRead;
            leftToTransfer -= toRead;

            eInkDriver_.EPD_5IN65F_DisplayPerformTrans((char*)internalBuffer_, toRead);
            LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);
        }
        else
        {
            LOG_ERROR("Could not read file part %s\n", filename);
            break;
        }
    }

    eInkDriver_.EPD_5IN65F_DisplayEndTrans();
    eInkDriver_.Sleep();

    if(leftToTransfer == 0)
    {
        currentImageName_ = filename;
        if(!store->SetCurrentImageName(filename))
        {
            LOG_ERROR("Could not save current image name\n");
        }
    }

    LOG_DEBUG("Updated EINK Image.\n");
}

void CEINK::GetCurrentImageName(std::string& imageName) const
{
    imageName = currentImageName_;
}

void CEINK::Clear(void)
{
    LOG_DEBUG("Cleaning eInk\n");
    eInkDriver_.Init(true);
    eInkDriver_.Clear(EPD_5IN65F_WHITE);
    eInkDriver_.Sleep();

    currentImageName_ = "";
    Storage::GetInstance()->SetCurrentImageName("");
}

void CEINK::DownloadAndUpdateImage(const char * filename)
{
    size_t   toRead;

    uint32_t   leftToTransfer = EINK_IMAGE_SIZE;

    Storage * store;
    bool      storageActive;

    uint64_t  timeout;

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Empty image name\n");
        systemState_->EnqueueResponse((const uint8_t*)"EMPTY", 6);
        return;
    }

    LOG_DEBUG("Updating eInk with image %s\n", filename);

    store = Storage::GetInstance();

    eInkDriver_.Init(true);
    eInkDriver_.EPD_5IN65F_DisplayInitTrans();

    if(!store->CreateImage(filename))
    {
        storageActive = false;
        LOG_ERROR("Could not create storage for image %s\n", filename);
    }
    else
    {
        storageActive = true;
    }

    if(!systemState_->SendResponseNow((const uint8_t*)"READY", 5))
    {
        eInkDriver_.EPD_5IN65F_DisplayEndTrans();
        eInkDriver_.Sleep();
        LOG_ERROR("Could not send READY response form eInk update\n");
        return;
    }

    LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);

    /* Get the full image data */
    timeout = HWManager::GetTime() + IMAGE_READ_TIMEOUT;
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

        btMgr_->ReceiveData(internalBuffer_, toRead);
        if(toRead > 0)
        {
            timeout = HWManager::GetTime() + IMAGE_READ_TIMEOUT;
            /* Save image part */
            if(storageActive)
            {
                if(!store->SaveImagePart(filename, internalBuffer_, toRead))
                {
                    LOG_ERROR("Could not save image part %s\n", filename);
                    storageActive = false;
                    if(!store->RemoveImage(filename))
                    {
                        LOG_ERROR("Could not remove image %s\n", filename);
                    }
                }
            }

            eInkDriver_.EPD_5IN65F_DisplayPerformTrans((const char*)internalBuffer_, toRead);
            leftToTransfer -= toRead;

            /* Send ACK */
            if(!systemState_->SendResponseNow((const uint8_t*)"OK", 2))
            {
                LOG_ERROR("Could not send OK response form eInk update\n");
                break;
            }
            LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);
        }
        else if(HWManager::GetTime() > timeout)
        {
            LOG_ERROR("Timeout on image receive\n");
            break;
        }
    }

    eInkDriver_.EPD_5IN65F_DisplayEndTrans();
    eInkDriver_.Sleep();

    currentImageName_ = filename;
    if(!store->SetCurrentImageName(filename))
    {
        LOG_ERROR("Could not save current image name\n");
    }

    systemState_->EnqueueResponse((const uint8_t*)"UPDATED", 7);

    LOG_DEBUG("Updated EINK Image.\n");
}

void CEINK::FormatFilename(char * filename)
{
    uint8_t i;

    /* Add null terminator to be safe */
    filename[COMMAND_DATA_SIZE - 1] = 0;

    /* Change spaces by underscores */
    for(i = 0; i < COMMAND_DATA_SIZE && filename[i] != 0; ++i)
    {
        if(filename[i] == ' ')
        {
            filename[i] = '_';
        }
    }
}

void CEINK::SendImage(const char * imageName)
{
    size_t   toRead;
    size_t   offset;
    bool     status;

    uint32_t   leftToTransfer = EINK_IMAGE_SIZE;

    Storage * store;

    if(strlen(imageName) == 0)
    {
        leftToTransfer = 0;
        toRead         = sizeof(leftToTransfer);
        btMgr_->TransmitData((uint8_t*)&leftToTransfer, toRead);
        return;
    }

    LOG_DEBUG("Sending EINK Image %s. Left: %d\n",
                  imageName,
                  leftToTransfer);

    store = Storage::GetInstance();
    toRead = sizeof(leftToTransfer);
    btMgr_->TransmitData((uint8_t*)&leftToTransfer, toRead);

    /* Get the full image data */
    offset = 0;
    while(leftToTransfer > 0)
    {
        /* Wati for ready */
        toRead = 1;
        LOG_DEBUG("Wait ACK.\n");
        do
        {
            btMgr_->ReceiveData(internalBuffer_, toRead);
        } while(toRead != 1);
        LOG_DEBUG("Received ACK %d.\n", internalBuffer_[0]);

        if(leftToTransfer < INTERNAL_BUFFER_SIZE)
        {
            toRead = leftToTransfer;
        }
        else
        {
            toRead = INTERNAL_BUFFER_SIZE;
        }

        status = store->ReadImagePart(imageName,
                                        offset,
                                        internalBuffer_,
                                        toRead);
        if(status && toRead > 0)
        {
            offset += toRead;
            leftToTransfer -= toRead;

            btMgr_->TransmitData(internalBuffer_, toRead);
            LOG_DEBUG("Sending EINK Image. Left: %d\n", leftToTransfer);
        }
        else
        {
            LOG_ERROR("Could not read file part %s\n", imageName);
            break;
        }
    }

    LOG_DEBUG("Sent EINK Image.\n");
}

#undef CEINK