/*******************************************************************************
 * @file SystemState.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the system state service.
 *
 * @details This file provides the system state service. This files defines
 * the different features embedded in the system state.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>         /* String manipulation*/
#include <Types.h>         /* Defined Types */
#include <Logger.h>        /* Logging service */
#include <Arduino.h>       /* Arduino Services */
#include <version.h>       /* Versioning */
#include <HWLayer.h>       /* Hardware Services */
#include <Storage.h>       /* Storage service */
#include <Updater.h>       /* Updater service */
#include <LEDBorder.h>     /* LED Border manager */
#include <IOButtonMgr.h>   /* Wakeup PIN */
#include <BlueToothMgr.h>  /* Bluetooth manager */
#include <OLEDScreenMgr.h> /* OLED screen manager */

/* Header File */
#include <SystemState.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSYSSTATE SystemState

#define SPLASH_TIME                3000
#define DEBUG_BTN_PRESS_TIME       3000
#define MENU_BTN_PRESS_TIME        1000
#define HIBER_BTN_PRESS_TIME       3000

#define SYSTEM_IDLE_TIME 30000 /* NS : 30 sec*/

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef struct STXQueueNode
{
    uint8_t * pData;
    size_t    dataSize;

    struct STXQueueNode * pNext;
} STXQueueNode;

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

CSYSSTATE::SystemState(OLEDScreenMgr    * pOLEDMgr,
                       IOButtonMgr      * pButtonMgr,
                       BluetoothManager * pBtMgr)
{
    pOLEDMgr_      = pOLEDMgr;
    pButtonMgr_    = pButtonMgr;
    pBtMgr_        = pBtMgr;
    pStore_        = Storage::GetInstance();
    pUpdater_      = nullptr;
    pLedBorderMgr_ = nullptr;

    prevState_           = ESystemState::SYS_IDLE;
    currState_           = ESystemState::SYS_START_SPLASH;
    nextMenuAction_      = EMenuAction::NONE;
    nextEInkAction_      = EEinkAction::EINK_NONE;
    nextLEDBorderAction_ = ELEDBorderAction::ELEDBORDER_NONE;
    nextUpdateAction_    = EUpdaterAction::EUPDATER_NONE;
    lastEventTime_       = 0;
    currDebugState_      = 0;
    pTxQueue_            = nullptr;

    memset(pNextLEDBorderMeta_, 0, COMMAND_DATA_SIZE);
    memset(pButtonsState_,
           EButtonState::BTN_STATE_DOWN,
           sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
    memset(pPrevButtonsState_,
           EButtonState::BTN_STATE_DOWN,
           sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
    memset(pButtonsKeepTime_,
           0,
           sizeof(uint64_t) * EButtonID::BUTTON_MAX_ID);
}

ESystemState CSYSSTATE::GetSystemState(void) const
{
    return currState_;
}

uint8_t CSYSSTATE::GetDebugState(void) const
{
    return currDebugState_;
}

EMenuAction CSYSSTATE::ConsumeMenuAction(void)
{
    EMenuAction retVal;

    retVal          = nextMenuAction_;
    nextMenuAction_ = EMenuAction::NONE;

    return retVal;
}

EEinkAction CSYSSTATE::ConsumeEInkAction(uint8_t pBuffer[COMMAND_DATA_SIZE])
{
    EEinkAction retVal;

    retVal          = nextEInkAction_;
    nextEInkAction_ = EEinkAction::EINK_NONE;

    if(retVal != EEinkAction::EINK_NONE)
    {
        memcpy(pBuffer, pNextEInkMeta_, COMMAND_DATA_SIZE);
    }

    return retVal;
}

ELEDBorderAction CSYSSTATE::ConsumeELEDBorderAction(uint8_t pBuffer[COMMAND_DATA_SIZE])
{
    ELEDBorderAction action;

    action               = nextLEDBorderAction_;
    nextLEDBorderAction_ = ELEDBorderAction::ELEDBORDER_NONE;

    if(action != ELEDBorderAction::ELEDBORDER_NONE)
    {
        memcpy(pBuffer, pNextLEDBorderMeta_, COMMAND_DATA_SIZE);
    }

    return action;
}

EUpdaterAction CSYSSTATE::ConsumeUpdateAction(void)
{
    EUpdaterAction retVal;

    retVal            = nextUpdateAction_;
    nextUpdateAction_ = EUpdaterAction::EUPDATER_NONE;

    return retVal;
}

EErrorCode CSYSSTATE::Update(void)
{
    EErrorCode retCode;
    SCBCommand command;

    retCode = EErrorCode::NO_ERROR;

    /* Send Bluetooth messages */
    if(SendPendingTransmission() == false)
    {
        LOG_ERROR("Failed to send pending transmissions\n");
    }

    /* Check Bluetooth Command */
    if(pBtMgr_->ReceiveCommand(&command) == true)
    {
        LOG_DEBUG("Received command type %d\n", command.type);
        HandleCommand(&command);
    }

    /* Update internal button states */
    UpdateButtonsState();

    /* Check the prioritary events */
    if(currDebugState_ == 0 &&
       pButtonsKeepTime_[EButtonID::BUTTON_UP] >= DEBUG_BTN_PRESS_TIME &&
       pButtonsKeepTime_[EButtonID::BUTTON_DOWN] >= DEBUG_BTN_PRESS_TIME)
    {
        LOG_DEBUG("Enabling to debug state\n");
        currDebugState_ = 1;
    }

    /* If not in debug state */
    if(currDebugState_ == 0)
    {
        /* Check if user is requesting hibernation and manages if we just booted
         * and the user still presses the button.
         */
        if(pButtonsKeepTime_[EButtonID::BUTTON_BACK] >= HIBER_BTN_PRESS_TIME &&
           pButtonsKeepTime_[EButtonID::BUTTON_BACK] <
            HIBER_BTN_PRESS_TIME + 100)
        {
            Hibernate(true);
        }

        /* Check the regular states management */
        switch(currState_)
        {
            case ESystemState::SYS_IDLE:
                ManageIdleState();
                break;
            case ESystemState::SYS_START_SPLASH:
                /* After SPLASH_TIME, switch to IDLE */
                if(HWManager::GetTime() > SPLASH_TIME + startTime_)
                {
                    currState_ = ESystemState::SYS_IDLE;
                    prevState_ = ESystemState::SYS_START_SPLASH;
                }
                break;
            case ESystemState::SYS_MENU:
                ManageMenuState();
                break;
            default:
                retCode = EErrorCode::NO_ACTION;
        }
    }
    else
    {
        ManageDebugState();
    }

    return retCode;
}

void CSYSSTATE::Ping(void)
{
    lastEventTime_ = HWManager::GetTime();
}

void CSYSSTATE::SetLedBorder(LEDBorder * pLedBorder)
{
    pLedBorderMgr_ = pLedBorder;
}

void CSYSSTATE::SetUpdater(Updater * pUpdater)
{
    pUpdater_ = pUpdater;
}

uint64_t CSYSSTATE::GetLastEventTime(void) const
{
    return lastEventTime_;
}

EButtonState CSYSSTATE::GetButtonState(const EButtonID kBtnId) const
{
    if(kBtnId < EButtonID::BUTTON_MAX_ID)
    {
        return pButtonsState_[kBtnId];
    }

    return EButtonState::BTN_STATE_DOWN;
}

uint64_t CSYSSTATE::GetButtonKeepTime(const EButtonID kBtnId) const
{
    if(kBtnId < EButtonID::BUTTON_MAX_ID)
    {
        return pButtonsKeepTime_[kBtnId];
    }

    return 0;
}

void CSYSSTATE::EnqueueResponse(const uint8_t * pkBuffer, const uint8_t kSize)
{
    STXQueueNode * pNewNode;

    /* Create a new node */
    pNewNode = new STXQueueNode;

    /* Create a new buffer */
    pNewNode->pData = new uint8_t[kSize + 5];

    /* Set the magic and size */
    pNewNode->pData[0] = (RESPONSE_MAGIC >> 24) & 0xFF;
    pNewNode->pData[1] = (RESPONSE_MAGIC >> 16) & 0xFF;
    pNewNode->pData[2] = (RESPONSE_MAGIC >> 8) & 0xFF;
    pNewNode->pData[3] = (RESPONSE_MAGIC >> 0) & 0xFF;
    pNewNode->pData[4] = kSize;

    /* Copy content */
    memcpy(pNewNode->pData + 5, pkBuffer, kSize);
    pNewNode->dataSize = kSize + 5;

    /* Link new node */
    pNewNode->pNext = (STXQueueNode*)pTxQueue_;
    pTxQueue_       = (uint8_t*)pNewNode;
}

bool CSYSSTATE::SendResponseNow(const uint8_t * pkBuffer, const uint8_t kSize)
{
    size_t  sendSize;
    uint8_t pHeader[5];

    /* Send header */
    pHeader[0] = (RESPONSE_MAGIC >> 24) & 0xFF;
    pHeader[1] = (RESPONSE_MAGIC >> 16) & 0xFF;
    pHeader[2] = (RESPONSE_MAGIC >> 8) & 0xFF;
    pHeader[3] = (RESPONSE_MAGIC >> 0) & 0xFF;
    pHeader[4] = kSize;
    sendSize = 5;

    pBtMgr_->TransmitData(pHeader, sendSize);
    if(sendSize != 5)
    {
        LOG_ERROR("Failed to transmit (sent %d, expected %d)",
                  sendSize,
                  5);
        return false;
    }

    /* Send data */
    sendSize = kSize;
    pBtMgr_->TransmitData(pkBuffer, sendSize);
    if(sendSize != kSize)
    {
        LOG_ERROR("Failed to transmit (sent %d, expected %d)",
                  sendSize,
                  kSize);
        return false;
    }

    return true;
}

void CSYSSTATE::ClearTransmissionQueue(void)
{
    STXQueueNode * pCursor;
    STXQueueNode * pSave;

    /* Walk the pending message */
    pCursor = (STXQueueNode*)pTxQueue_;
    while(pCursor != nullptr)
    {
        /* Clean memory */
        delete[] pCursor->pData;

        /* Go to next */
        pSave   = pCursor;
        pCursor = pCursor->pNext;

        /* Clean memory */
        delete pSave;
    }

    /* Reset queue */
    pTxQueue_ = nullptr;
}

void CSYSSTATE::SetSystemState(const ESystemState kState)
{
    prevState_     = currState_;
    currState_     = kState;
    lastEventTime_ = HWManager::GetTime();
}

bool CSYSSTATE::SendPendingTransmission(void)
{
    STXQueueNode * cursor;
    STXQueueNode * save;
    size_t         sendSize;
    bool           status;

    /* Walk the pending message */
    status = true;
    cursor = (STXQueueNode*)pTxQueue_;
    while(cursor != nullptr)
    {
        /* Send data */
        sendSize = cursor->dataSize;
        pBtMgr_->TransmitData(cursor->pData, sendSize);
        if(sendSize != cursor->dataSize)
        {
            LOG_ERROR("Failed to transmit (sent %d, expected %d)\n",
                      sendSize,
                      cursor->dataSize);
            status = false;
        }

        /* Clean memory */
        delete[] cursor->pData;

        /* Go to next */
        save   = cursor;
        cursor = cursor->pNext;

        /* Clean memory */
        delete save;
    }

    /* Reset queue */
    pTxQueue_ = nullptr;

    return status;
}

void CSYSSTATE::UpdateButtonsState(void)
{
    uint8_t      i;
    EButtonState newState;
    uint64_t     newKeepTime;
    uint64_t     timeNow;

    timeNow = HWManager::GetTime();

    for(i = 0; i < EButtonID::BUTTON_MAX_ID; ++i)
    {
        pPrevButtonsState_[i] = pButtonsState_[i];

        newState = pButtonMgr_->GetButtonState((EButtonID)i);
        if(pButtonsState_[i] != newState)
        {
            pButtonsState_[i] = newState;
            lastEventTime_   = timeNow;
        }
        newKeepTime = pButtonMgr_->GetButtonKeepTime((EButtonID)i);
        if(pButtonsKeepTime_[i] != newKeepTime)
        {
            pButtonsKeepTime_[i] = newKeepTime;
            lastEventTime_      = timeNow;
        }
    }
}

void CSYSSTATE::ManageDebugState(void)
{
    /* Check if we should switch to next debug state */
    if(pPrevButtonsState_[EButtonID::BUTTON_DOWN] != EButtonState::BTN_STATE_DOWN &&
       pButtonsState_[EButtonID::BUTTON_DOWN] == EButtonState::BTN_STATE_DOWN)
    {
        if(currDebugState_ == 4)
        {
            currDebugState_ = 0;
        }
        ++currDebugState_;
    }
    else if(pPrevButtonsState_[EButtonID::BUTTON_UP] != EButtonState::BTN_STATE_DOWN &&
            pButtonsState_[EButtonID::BUTTON_UP] == EButtonState::BTN_STATE_DOWN)
    {
        if(currDebugState_ == 1)
        {
            currDebugState_ = 5;
        }
        --currDebugState_;
    }
    else if(pPrevButtonsState_[EButtonID::BUTTON_ENTER] != EButtonState::BTN_STATE_DOWN &&
            pButtonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_DOWN &&
            currDebugState_ == 4)
    {
        currDebugState_ = 0;
        LOG_DEBUG("Disabling debug state\n");
    }
    /* Check if we should toggle logging to SD card */
    else if(pPrevButtonsState_[EButtonID::BUTTON_BACK] != EButtonState::BTN_STATE_DOWN &&
            pButtonsState_[EButtonID::BUTTON_BACK] == EButtonState::BTN_STATE_DOWN &&
            currDebugState_ == 3)
    {
        LOGGER_TOGGLE_FILE_LOG();
    }
}

void CSYSSTATE::ManageIdleState(void)
{
    /* Check if we should enter menu mode */
    if(pButtonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_KEEP &&
       pButtonsKeepTime_[EButtonID::BUTTON_ENTER] >= MENU_BTN_PRESS_TIME)
    {
        SetSystemState(ESystemState::SYS_MENU);
    }
    else if(prevState_ != ESystemState::SYS_IDLE &&
            pButtonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_UP)
    {
        LOG_DEBUG("IDLE mode\n");
        SetSystemState(ESystemState::SYS_IDLE);
    }
}

void CSYSSTATE::ManageMenuState(void)
{
    if(prevState_ == ESystemState::SYS_MENU)
    {
        /* Update selected item */
        if(pPrevButtonsState_[EButtonID::BUTTON_DOWN] != EButtonState::BTN_STATE_DOWN &&
           pButtonsState_[EButtonID::BUTTON_DOWN] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::SELECT_NEXT;
        }
        else if(pPrevButtonsState_[EButtonID::BUTTON_UP] != EButtonState::BTN_STATE_DOWN &&
                pButtonsState_[EButtonID::BUTTON_UP] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::SELECT_PREV;
        }
        /* Check if enter was pressed */
        else if(pPrevButtonsState_[EButtonID::BUTTON_ENTER] != EButtonState::BTN_STATE_DOWN &&
                pButtonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::EXECUTE_SEL;
        }
        /* Check if back was pressed */
        else if(pPrevButtonsState_[EButtonID::BUTTON_BACK] != EButtonState::BTN_STATE_DOWN &&
                pButtonsState_[EButtonID::BUTTON_BACK] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::BACK_MENU;
        }

        /* Manage IDLE detection in menu */
        if(currState_ == ESystemState::SYS_MENU)
        {
            if(HWManager::GetTime() - lastEventTime_ > SYSTEM_IDLE_TIME)
            {
                SetSystemState(ESystemState::SYS_IDLE);
            }
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Menu mode\n");

        /* Init menu page and menu item */
        SetSystemState(ESystemState::SYS_MENU);
    }
}

void CSYSSTATE::HandleCommand(SCBCommand * pCommand)
{
    switch(pCommand->type)
    {
        case ECommandType::PING:
            /* Send response */
            EnqueueResponse((uint8_t*)"PONG", 4);
            break;

        case ECommandType::CLEAR_EINK:
            nextEInkAction_ = EEinkAction::EINK_CLEAR;
            break;

        case ECommandType::UPDATE_EINK:
            nextEInkAction_ = EEinkAction::EINK_UPDATE;
            memcpy(pNextEInkMeta_, pCommand->pCommandData, COMMAND_DATA_SIZE);
            break;

        case ECommandType::ENABLE_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::ENABLE_LEDB_ACTION;
            nextMenuAction_      = EMenuAction::REFRESH_LEDB_STATE;
            break;

        case ECommandType::DISABLE_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::DISABLE_LEDB_ACTION;
            nextMenuAction_      = EMenuAction::REFRESH_LEDB_STATE;
            break;

        case ECommandType::ADD_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::ADD_ANIMATION_LEDB_ACTION;
            memcpy(pNextLEDBorderMeta_,
                   pCommand->pCommandData,
                   COMMAND_DATA_SIZE);
            break;

        case ECommandType::REMOVE_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::REMOVE_ANIMATION_LEDB_ACTION;
            memcpy(pNextLEDBorderMeta_,
                   pCommand->pCommandData,
                   COMMAND_DATA_SIZE);
            break;

        case ECommandType::SET_PATTERN_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::SET_PATTERN_LEDB_ACTION;
            memcpy(pNextLEDBorderMeta_,
                   pCommand->pCommandData,
                   COMMAND_DATA_SIZE);
            break;

        case ECommandType::CLEAR_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::CLEAR_ANIMATION_LEDB_ACTION;
            break;

        case ECommandType::SET_BRIGHTNESS_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::SET_BRIGHTNESS_LEDB_ACTION;
            memcpy(pNextLEDBorderMeta_,
                   pCommand->pCommandData,
                   COMMAND_DATA_SIZE);
            break;

        case ECommandType::SET_OWNER_VALUE:
            if(pStore_->SetOwner(std::string((char*)pCommand->pCommandData)))
            {
                nextMenuAction_ = EMenuAction::REFRESH_MYINFO;
                EnqueueResponse((uint8_t*)"OK", 2);
            }
            else
            {
                LOG_ERROR("Could not update owner\n");
                EnqueueResponse((uint8_t*)"KO", 2);
            }
            break;

        case ECommandType::SET_CONTACT_VALUE:
            if(pStore_->SetContact(std::string((char*)pCommand->pCommandData)))
            {
                nextMenuAction_ = EMenuAction::REFRESH_MYINFO;
                EnqueueResponse((uint8_t*)"OK", 2);
            }
            else
            {
                LOG_ERROR("Could not update contact\n");
                EnqueueResponse((uint8_t*)"KO", 2);
            }
            break;

        case ECommandType::SET_BT_SETTINGS:
            if(pBtMgr_->UpdateSettings(pCommand->pCommandData))
            {
                nextMenuAction_ = EMenuAction::REFRESH_BT_INFO;
                ClearTransmissionQueue();
            }
            else
            {
                LOG_ERROR("Could not update Bluetooth name\n");
                EnqueueResponse((uint8_t*)"KO", 2);
            }
            break;

        case ECommandType::REQUEST_FACTORY_RESET:
            nextMenuAction_ = EMenuAction::VALIDATE_FACTORY_RESET;
            currState_      = SYS_MENU;
            EnqueueResponse((uint8_t*)"OK", 2);
            break;

        case ECommandType::START_UPDATE:
            nextUpdateAction_ = EUpdaterAction::START_UPDATE_ACTION;
            break;

        case ECommandType::VALIDATE_UPDATE:
            nextUpdateAction_ = EUpdaterAction::VALIDATION_ACTION;
            break;

        case ECommandType::CANCEL_UPDATE:
            nextUpdateAction_ = EUpdaterAction::CANCEL_ACTION;
            break;

        case ECommandType::START_TRANS_UPDATE:
            nextUpdateAction_ = EUpdaterAction::START_TRANSFER_ACTION;
            break;

        case ECommandType::GET_INFO:
            SendBadgeInfo();
            break;

        case ECommandType::REQUEST_UPDATE:
            pUpdater_->RequestUpdate();
            currState_ = SYS_MENU;
            EnqueueResponse((uint8_t*)"OK", 2);
            break;

        case ECommandType::GET_INFO_LEDBORDER:
            SendLedBorderInfo();
            break;

        case ECommandType::GET_IMAGES_NAME:
            SendEInkImagesName(*(uint32_t*)pCommand->pCommandData,
                               *(((uint32_t*)pCommand->pCommandData) + 1));
            break;

        case ECommandType::REMOVE_IMAGE:
            if(pStore_->RemoveImage((char*)pCommand->pCommandData))
            {
                nextEInkAction_ = EEinkAction::EINK_CLEAR;

                /* Directly send response because the EINK clear will take too
                 * much time.
                 */
                if(!SendResponseNow((uint8_t*)"OK", 2))
                {
                    LOG_ERROR("Could not send OK response\n");
                }
            }
            else
            {
                EnqueueResponse((uint8_t*)"KO", 2);
            }
            break;

        case ECommandType::SELECT_LOADED_IMAGE:
            nextEInkAction_ = EEinkAction::EINK_SELECT_LOADED;
            memcpy(pNextEInkMeta_, pCommand->pCommandData, COMMAND_DATA_SIZE);
            break;

        case ECommandType::GET_CURRENT_IMAGE:
            nextEInkAction_ = EEinkAction::EINK_SEND_CURRENT_IMAGE;
            memcpy(pNextEInkMeta_, pCommand->pCommandData, COMMAND_DATA_SIZE);
            break;

        default:
            LOG_ERROR("Unknown command type %d\n", pCommand->type);
            EnqueueResponse((uint8_t*)"UKN_CMD", 7);
            break;
    }
}

void CSYSSTATE::SendBadgeInfo(void)
{
    std::string owner;
    std::string contact;
    std::string imgName;
    std::string btPin;

    uint8_t *   pBuffer;
    size_t      ownerSize;
    size_t      contactSize;
    size_t      swVersionSize;
    size_t      bufferSize;
    size_t      hwVersionSize;
    size_t      imageNameSize;
    size_t      btPinSize;
    size_t      cursor;

    /* Get information and send them */
    pStore_->GetOwner(owner);
    pStore_->GetContact(contact);
    pStore_->GetDisplayedImageName(imgName);
    pStore_->GetBluetoothPin(btPin);
    if(imgName == "")
    {
        imgName = "None";
    }
    if(btPin == "")
    {
        btPin = "None";
    }

    ownerSize     = owner.size();
    contactSize   = contact.size();
    swVersionSize = strlen(VERSION_SHORT);
    hwVersionSize = strlen(PROTO_REV);
    imageNameSize = imgName.size();
    btPinSize     = btPin.size();

    bufferSize = ownerSize +
                 contactSize +
                 swVersionSize +
                 hwVersionSize +
                 imageNameSize +
                 btPinSize +
                 7;

    pBuffer = new uint8_t[bufferSize];

    /* Add owner */
    memcpy(pBuffer, owner.c_str(), ownerSize);
    cursor = ownerSize;

    /* Add separator and contact  */
    pBuffer[cursor++] = 6;
    memcpy(pBuffer + cursor, contact.c_str(), contactSize);
    cursor += contactSize;

    /* Add separator and SW version */
    pBuffer[cursor++] = 6;
    memcpy(pBuffer + cursor, VERSION_SHORT, swVersionSize);
    cursor += swVersionSize;

    /* Add separator and HW version */
    pBuffer[cursor++] = 6;
    memcpy(pBuffer + cursor, PROTO_REV, hwVersionSize);
    cursor += hwVersionSize;

    /* Add separator and Led border state */
    pBuffer[cursor++] = 6;
    pBuffer[cursor++] = pLedBorderMgr_->IsEnabled();

    /* Add separator and current image name */
    pBuffer[cursor++] = 6;
    memcpy(pBuffer + cursor, imgName.c_str(), imageNameSize);
    cursor += imageNameSize;

    /* Add separator and current BT Pin */
    pBuffer[cursor++] = 6;
    memcpy(pBuffer + cursor, btPin.c_str(), btPinSize);

    EnqueueResponse(pBuffer, bufferSize);

    delete[] pBuffer;
}

void CSYSSTATE::SendLedBorderInfo(void)
{
    std::vector<IColorAnimation*> * pAnimations;
    ColorPattern                  * pPattern;
    uint8_t                       * pBuffer;
    SLEDBorderPatternParam     patternParams;
    size_t                          bufferSize;
    size_t                          cursor;
    size_t                          animCount;
    size_t                          i;

    pAnimations = pLedBorderMgr_->GetColorAnimations();
    pPattern    = pLedBorderMgr_->GetColorPattern();

    animCount  = pAnimations->size();
    bufferSize = 40 + 2 * animCount;

    pBuffer = new uint8_t[bufferSize];

    cursor = 0;

    /* Add the state */
    pBuffer[cursor++] = (uint8_t)pLedBorderMgr_->IsEnabled();

    /* Add the type */
    pBuffer[cursor++] = (uint8_t)pPattern->GetType();

    /* Add metadata */
    pPattern->GetRawParam(&patternParams);
    memcpy(pBuffer + cursor,
            &patternParams,
            sizeof(SLEDBorderPatternParam));
    cursor += sizeof(SLEDBorderPatternParam);

    /* Add brightness */
    pBuffer[cursor++] = pLedBorderMgr_->GetBrightness();

    /* Add animations */
    pBuffer[cursor++] = animCount;
    for(i = 0; i < animCount; ++i)
    {
        pBuffer[cursor++] = (uint8_t)pAnimations->at(i)->GetType();
        pBuffer[cursor++] = pAnimations->at(i)->GetRawParam();
    }

    EnqueueResponse(pBuffer, bufferSize);

    delete[] pBuffer;
}

void CSYSSTATE::SendEInkImagesName(const uint32_t kStartIdx, uint32_t count)
{
    uint8_t * pBuffer;
    size_t    actualSize;

    /* First, bound the update, we will only send the 50 next images */
    if(count > 50)
    {
        count = 50;
    }

    pBuffer = new uint8_t[count * COMMAND_DATA_SIZE];
    pStore_->GetImageList(pBuffer, actualSize, kStartIdx, count);

    EnqueueResponse(pBuffer, actualSize);

    delete[] pBuffer;
}

void CSYSSTATE::Hibernate(const bool kDisplay)
{
    esp_err_t status;
    status = esp_sleep_enable_ext0_wakeup((gpio_num_t)EButtonPin::BOOT_PIN,
                                          EButtonState::BTN_STATE_UP);

    if(status == ESP_OK)
    {
        LOG_DEBUG("Enabling Deep Sleep\n");
        pStore_->Stop();
        pLedBorderMgr_->Stop();
        if(kDisplay == true)
        {
            pOLEDMgr_->DisplaySleep();
            delay(3000);
            pOLEDMgr_->SwitchOff();
        }
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);

        esp_deep_sleep_start();
    }
    else
    {
        LOG_ERROR("Could not setup deep sleep wakeup (%d)\n", status);
    }
}

void CSYSSTATE::ManageBoot(void)
{
    esp_sleep_wakeup_cause_t wakeupReason;
    EButtonState             btState;

    wakeupReason = esp_sleep_get_wakeup_cause();

    LOG_DEBUG("Boot reason: %d\n", wakeupReason);

    /* On normal boot, return */
    if(wakeupReason != ESP_SLEEP_WAKEUP_EXT0)
    {
        startTime_ = HWManager::GetTime();
        return;
    }

    do
    {
        pButtonMgr_->UpdateState();
        btState = pButtonMgr_->GetButtonState(EButtonID::BUTTON_BOOT);

        if(pButtonMgr_->GetButtonKeepTime(EButtonID::BUTTON_BOOT) >
           HIBER_BTN_PRESS_TIME)
        {
            startTime_ = HWManager::GetTime();
            return;
        }

        delay(100);
    } while(btState == EButtonState::BTN_STATE_DOWN ||
            btState == EButtonState::BTN_STATE_KEEP);

    /* We did not wait for the amount of time */
    Hibernate(false);
}

#undef CSYSSTATE