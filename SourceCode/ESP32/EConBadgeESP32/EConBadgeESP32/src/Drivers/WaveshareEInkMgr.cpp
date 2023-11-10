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

#define UPDATE_RX_SIZE 8192

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
}

void CEINK::Update(void)
{
    EEinkAction action;

    /* Check for new command */
    action = systemState_->ConsumeEInkAction();
    switch(action)
    {
        case EEinkAction::EINK_CLEAR:
            Clear();
            break;
        case EEinkAction::EINK_UPDATE:
            UpdateDisplay();
        default:
            break;
    }
}

void CEINK::RequestClear(void)
{
    Clear();
}

void CEINK::Clear(void)
{
    LOG_DEBUG("Cleaning eInk\n");
    eInkDriver_.Init(true);
    eInkDriver_.Clear(EPD_5IN65F_WHITE);
    eInkDriver_.Sleep();

    if(!systemState_->EnqueueResponse((const uint8_t*)"OK", 2))
    {
        LOG_ERROR("Could not send eInk Clear response\n");
    }
}

void CEINK::UpdateDisplay(void)
{
    uint32_t readSize;
    uint32_t toRead;

    uint8_t  * imageData;
    uint32_t   leftToTransfer = EINK_IMAGE_SIZE;

    LOG_DEBUG("Updating eInk\n");

    imageData = new uint8_t[UPDATE_RX_SIZE];
    if(imageData != nullptr)
    {

        eInkDriver_.Init(true);
        eInkDriver_.EPD_5IN65F_DisplayInitTrans();
        
        if(!systemState_->SendResponseNow((const uint8_t*)"READY", 5))
        {
            eInkDriver_.EPD_5IN65F_DisplayEndTrans();
            eInkDriver_.Sleep();
            LOG_ERROR("Could not send READY response form eInk update\n");
            return;
        }

        LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);

        /* Get the full image data */
        while(leftToTransfer > 0)
        {
            if(leftToTransfer < UPDATE_RX_SIZE)
            {
                toRead = leftToTransfer;
            }
            else
            {
                toRead = UPDATE_RX_SIZE;
            }

            btMgr_->ReceiveData(imageData, toRead);
            if(toRead > 0)
            {
                eInkDriver_.EPD_5IN65F_DisplayPerformTrans((const char*)imageData, toRead);
                leftToTransfer -= toRead;

                /* Send ACK */
                if(!systemState_->SendResponseNow((const uint8_t*)"OK", 2))
                {
                    LOG_ERROR("Could not send OK response form eInk update\n");
                    break;
                }
                LOG_DEBUG("Updating EINK Image. Left: %d\n", leftToTransfer);
            }
        }

        eInkDriver_.EPD_5IN65F_DisplayEndTrans();
        eInkDriver_.Sleep();

        if(!systemState_->EnqueueResponse((const uint8_t*)"UPDATED", 7))
        {
            LOG_ERROR("Could not send eInk Update response\n");
        }

        LOG_DEBUG("Updated EINK Image.\n");

        delete[] imageData;
    }
    else
    {
        LOG_ERROR("Could not allocate memory buffer\n");
    }
}

#undef CEINK