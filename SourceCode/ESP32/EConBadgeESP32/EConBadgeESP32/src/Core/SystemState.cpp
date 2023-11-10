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
#include <cstring>            /* String manipulation*/
#include <Types.h>            /* Defined Types */
#include <Arduino.h>          /* Arduino Services */
#include <Logger.h>           /* Logging service */
#include <version.h>          /* Versioning */
#include <HWLayer.h>          /* Hardware Services */
#include <IOButtonMgr.h>      /* Wakeup PIN */
#include <BlueToothMgr.h>     /* Bluetooth manager */

/* Header File */
#include <SystemState.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSYSSTATE SystemState

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef struct STXQueueNode
{
    uint8_t * data;
    size_t    data_size;

    struct STXQueueNode * next;
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

CSYSSTATE::SystemState(IOButtonMgr * buttonMgr, BluetoothManager * btMgr)
{
    buttonMgr_      = buttonMgr;
    btMgr_          = btMgr;

    prevState_           = ESystemState::SYS_IDLE;
    currState_           = ESystemState::SYS_START_SPLASH;
    nextMenuAction_      = EMenuAction::NONE;
    nextEInkAction_      = EEinkAction::EINK_NONE;
    nextLEDBorderAction_ = ELEDBorderAction::ELEDBORDER_NONE;
    lastEventTime_       = 0;
    currDebugState_      = 0;
    txQueue_             = nullptr;

    memset(buttonsState_, EButtonState::BTN_STATE_DOWN, sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
    memset(prevButtonsState_, EButtonState::BTN_STATE_DOWN, sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
    memset(buttonsKeepTime_, 0, sizeof(uint32_t) * EButtonID::BUTTON_MAX_ID);
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

EEinkAction CSYSSTATE::ConsumeEInkAction(void)
{
    EEinkAction retVal;

    retVal          = nextEInkAction_;
    nextEInkAction_ = EEinkAction::EINK_NONE;

    return retVal;
}

EErrorCode CSYSSTATE::Update(void)
{
    EErrorCode retCode;
    SCBCommand command;

    retCode = EErrorCode::NO_ERROR;

    /* Send Bluetooth messages */
    if(!SendPendingTransmission())
    {
        LOG_ERROR("Could not send all pending transmissions\n");
    }

    /* Check Bluetooth Command */
    if(btMgr_->ReceiveCommand(&command))
    {
        LOG_INFO("Received command type %d\n", command.type);
        HandleCommand(&command);
    }

    /* Update internal button states */
    UpdateButtonsState();

    /* Check the prioritary events */
    if(currDebugState_ == 0 &&
       buttonsKeepTime_[EButtonID::BUTTON_UP] >= DEBUG_BTN_PRESS_TIME &&
       buttonsKeepTime_[EButtonID::BUTTON_DOWN] >= DEBUG_BTN_PRESS_TIME)
    {
        LOG_DEBUG("Enabling to debug state\n");
        currDebugState_ = 1;
    }

    /* If not in debug state */
    if(currDebugState_ == 0)
    {
        /* Check the regular states management */
        switch(currState_)
        {
            case ESystemState::SYS_IDLE:
                ManageIdleState();
                break;
            case ESystemState::SYS_START_SPLASH:
                /* After SPLASH_TIME, switch to IDLE */
                if(millis() > SPLASH_TIME)
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

uint32_t CSYSSTATE::GetLastEventTime(void) const
{
    return lastEventTime_;
}

EButtonState CSYSSTATE::GetButtonState(const EButtonID btnId) const
{
    if(btnId < EButtonID::BUTTON_MAX_ID)
    {
        return buttonsState_[btnId];
    }

    return EButtonState::BTN_STATE_DOWN;
}

uint32_t CSYSSTATE::GetButtonKeepTime(const EButtonID btnId) const
{
    if(btnId < EButtonID::BUTTON_MAX_ID)
    {
        return buttonsKeepTime_[btnId];
    }

    return 0;
}

bool CSYSSTATE::EnqueueResponse(const uint8_t * buffer, const uint8_t size)
{
    STXQueueNode * newNode;

    /* Create a new node */
    newNode = new STXQueueNode;
    if(newNode == nullptr)
    {
        return false;
    }

    /* Create a new buffer */
    newNode->data = new uint8_t[size + 5];
    if(newNode->data == nullptr)
    {
        delete newNode;
        return false;
    }

    /* Set the magic and size */
    newNode->data[0] = (RESPONSE_MAGIC >> 24) & 0xFF;
    newNode->data[1] = (RESPONSE_MAGIC >> 16) & 0xFF;
    newNode->data[2] = (RESPONSE_MAGIC >> 8) & 0xFF;
    newNode->data[3] = (RESPONSE_MAGIC >> 0) & 0xFF;
    newNode->data[4] = size;

    /* Copy content */
    memcpy(newNode->data + 5, buffer, size);
    newNode->data_size = size + 5;

    /* Link new node */
    newNode->next = (STXQueueNode*)txQueue_;
    txQueue_      = (uint8_t*)newNode;

    return true;
}

bool CSYSSTATE::SendResponseNow(const uint8_t * buffer, const uint8_t size)
{
    size_t  sendSize;
    uint8_t header[5];

    /* Send header */
    header[0] = (RESPONSE_MAGIC >> 24) & 0xFF;
    header[1] = (RESPONSE_MAGIC >> 16) & 0xFF;
    header[2] = (RESPONSE_MAGIC >> 8) & 0xFF;
    header[3] = (RESPONSE_MAGIC >> 0) & 0xFF;
    header[4] = size;
    sendSize = 5;

    btMgr_->TransmitData(header, sendSize);
    if(sendSize != 5)
    {
        LOG_ERROR("Could not transmit immediate TX (send %d, expected %d)",
                  sendSize,
                  5);
        return false;
    }

    /* Send data */
    sendSize = size;
    btMgr_->TransmitData(buffer, sendSize);
    if(sendSize != size)
    {
        LOG_ERROR("Could not transmit immediate TX (send %d, expected %d)",
                  sendSize,
                  size);
        return false;
    }

    return true;
}

void CSYSSTATE::SetSystemState(const ESystemState state)
{
    prevState_     = currState_;
    currState_     = state;
    lastEventTime_ = millis();
}

bool CSYSSTATE::SendPendingTransmission(void)
{
    STXQueueNode * cursor;
    STXQueueNode * save;
    size_t         sendSize;
    bool           status;

    /* Walk the pending message */
    status = true;
    cursor = (STXQueueNode*)txQueue_;
    while(cursor != nullptr)
    {
        /* Send data */
        sendSize = cursor->data_size;
        btMgr_->TransmitData(cursor->data, sendSize);
        if(sendSize != cursor->data_size)
        {
            LOG_ERROR("Could not transmit pending TX (send %d, expected %d)",
                      sendSize,
                      cursor->data_size);
            status = false;
        }

        /* Clean memory */
        delete[] cursor->data;

        /* Go to next */
        save   = cursor;
        cursor = cursor->next;

        /* Clean memory */
        delete save;
    }

    /* Reset queue */
    txQueue_ = nullptr;

    return status;
}

void CSYSSTATE::UpdateButtonsState(void)
{
    uint8_t      i;
    EButtonState newState;
    uint32_t     newKeepTime;
    uint32_t     timeNow;

    timeNow = millis();

    for(i = 0; i < EButtonID::BUTTON_MAX_ID; ++i)
    {
        prevButtonsState_[i] = buttonsState_[i];

        newState = buttonMgr_->GetButtonState((EButtonID)i);
        if(buttonsState_[i] != newState)
        {
            buttonsState_[i] = newState;
            lastEventTime_   = timeNow;
        }
        newKeepTime = buttonMgr_->GetButtonKeepTime((EButtonID)i);
        if(buttonsKeepTime_[i] != newKeepTime)
        {
            buttonsKeepTime_[i] = newKeepTime;
            lastEventTime_      = timeNow;
        }
    }
}

void CSYSSTATE::ManageDebugState(void)
{
    /* Check if we should switch to next debug state */
    if(prevButtonsState_[EButtonID::BUTTON_DOWN] != EButtonState::BTN_STATE_DOWN &&
        buttonsState_[EButtonID::BUTTON_DOWN] == EButtonState::BTN_STATE_DOWN)
    {
        if(currDebugState_ == 3)
        {
            currDebugState_ = 0;
        }
        ++currDebugState_;
    }
    else if(prevButtonsState_[EButtonID::BUTTON_UP] != EButtonState::BTN_STATE_DOWN &&
            buttonsState_[EButtonID::BUTTON_UP] == EButtonState::BTN_STATE_DOWN)
    {
        if(currDebugState_ == 1)
        {
            currDebugState_ = 4;
        }
        --currDebugState_;
    }
    else if(prevButtonsState_[EButtonID::BUTTON_ENTER] != EButtonState::BTN_STATE_DOWN &&
            buttonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_DOWN &&
            currDebugState_ == 3)
    {
        currDebugState_ = 0;
        LOG_DEBUG("Disabling to debug state\n");
    }
}

void CSYSSTATE::ManageIdleState(void)
{
    /* Check if we should enter menu mode */
    if(buttonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_KEEP &&
       buttonsKeepTime_[EButtonID::BUTTON_ENTER] >= MENU_BTN_PRESS_TIME)
    {
        SetSystemState(ESystemState::SYS_MENU);

        if(!setCpuFrequencyMhz(240))
        {
            LOG_ERROR("Could not set CPU frequency\n");
        }
        else
        {
            Serial.updateBaudRate(115200);
            LOG_INFO("Set CPU Freq: %d\n", getCpuFrequencyMhz());
        }
    }
    else if(prevState_ != ESystemState::SYS_IDLE &&
            buttonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_UP)
    {
        LOG_DEBUG("Switching to IDLE state\n");
        SetSystemState(ESystemState::SYS_IDLE);

        /* Ensure we flushed everything */
        Serial.flush();

        /* Simply reduce the CPU frequency */
        if(!setCpuFrequencyMhz(80))
        {
            LOG_ERROR("Could not set CPU frequency\n");
        }
        else
        {
            Serial.updateBaudRate(115200);
            LOG_INFO("Set CPU Freq: %d\n", getCpuFrequencyMhz());
        }
    }
}

void CSYSSTATE::ManageMenuState(void)
{
    if(prevState_ == ESystemState::SYS_MENU)
    {
        /* Update selected item */
        if(prevButtonsState_[EButtonID::BUTTON_DOWN] != EButtonState::BTN_STATE_DOWN &&
           buttonsState_[EButtonID::BUTTON_DOWN] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::SELECT_NEXT;
        }
        else if(prevButtonsState_[EButtonID::BUTTON_UP] != EButtonState::BTN_STATE_DOWN &&
                buttonsState_[EButtonID::BUTTON_UP] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::SELECT_PREV;
        }
        /* Check if enter was pressed */
        else if(prevButtonsState_[EButtonID::BUTTON_ENTER] != EButtonState::BTN_STATE_DOWN &&
                buttonsState_[EButtonID::BUTTON_ENTER] == EButtonState::BTN_STATE_DOWN)
        {
            nextMenuAction_ = EMenuAction::EXECUTE_SEL;
        }

        /* Manage IDLE detection in menu */
        if(currState_ == ESystemState::SYS_MENU)
        {
            if(millis() - lastEventTime_ > SYSTEM_IDLE_TIME)
            {
                SetSystemState(ESystemState::SYS_IDLE);
            }
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Switching to menu mode\n");

        /* Init menu page and menu item */
        SetSystemState(ESystemState::SYS_MENU);
    }
}

void CSYSSTATE::HandleCommand(SCBCommand * command)
{
    size_t  writeSize;

    switch(command->type)
    {
        case ECommandType::PING:
            /* Send response */
            EnqueueResponse((const uint8_t*)"PONG", 4);
            break;

        case ECommandType::CLEAR_EINK:
            nextEInkAction_ = EEinkAction::EINK_CLEAR;
            break;

        case ECommandType::UPDATE_EINK:
            nextEInkAction_ = EEinkAction::EINK_UPDATE;
            break;

        case ECommandType::ENABLE_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::ENABLE_LEDB_ACTION;
            break;

        case ECommandType::DISABLE_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::DISABLE_LEDB_ACTION;
            break;

        case ECommandType::ADD_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::ADD_ANIMATION_LEDB_ACTION;
            memcpy(nextLEDBorderMeta_, command->commandData, COMMAND_DATA_SIZE);
            break;

        case ECommandType::REMOVE_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::REMOVE_ANIMATION_LEDB_ACTION;
            memcpy(nextLEDBorderMeta_, command->commandData, COMMAND_DATA_SIZE);
            break;

        case ECommandType::SET_PATTERN_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::SET_PATTERN_LEDB_ACTION;
            memcpy(nextLEDBorderMeta_, command->commandData, COMMAND_DATA_SIZE);
            break;

        case ECommandType::CLEAR_ANIMATION_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::CLEAR_ANIMATION_LEDB_ACTION;
            break;

        case ECommandType::CLEAR_PATTERN_LEDB:
            nextLEDBorderAction_ = ELEDBorderAction::CLEAR_PATTERN_LEDB_ACTION;
            break;

        default:
            LOG_ERROR("Unknown command type %d\n", command->type);
            break;
    }
}

#undef CSYSSTATE