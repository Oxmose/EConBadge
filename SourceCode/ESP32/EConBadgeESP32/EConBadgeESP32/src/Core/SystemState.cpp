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

CSYSSTATE::SystemState(IOButtonMgr * buttonMgr, BluetoothManager * btMgr)
{
    buttonMgr_      = buttonMgr;
    btMgr_          = btMgr;

    prevState_      = ESystemState::SYS_IDLE;
    currState_      = ESystemState::SYS_START_SPLASH;
    nextMenuAction_ = EMenuAction::NONE;
    nextEInkAction_ = EEinkAction::EINK_NONE;
    lastEventTime_  = 0;
    currDebugState_ = 0;

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
    ECBCommand command;

    retCode = EErrorCode::NO_ERROR;

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

void CSYSSTATE::SetSystemState(const ESystemState state)
{
    prevState_     = currState_;
    currState_     = state;
    lastEventTime_ = millis();
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

void CSYSSTATE::HandleCommand(ECBCommand * command)
{
    size_t writeSize;

    switch(command->type)
    {
        case ECommandType::PING:
            /* Send response */
            writeSize = 5;
            btMgr_->TransmitData((const uint8_t*)"PONG", writeSize);
            if(writeSize != 5)
            {
                LOG_ERROR("Could not send PONG response.\n");
            }
            break;

        case ECommandType::CLEAR_EINK:

            /* Request clear EInk */
            nextEInkAction_ = EEinkAction::EINK_CLEAR;

            /* Send response */
            writeSize = 1;
            btMgr_->TransmitData((const uint8_t*)"\0", writeSize);
            if(writeSize != 1)
            {
                LOG_ERROR("Could not send Clean EInk response.\n");
            }
            break;

        case ECommandType::UPDATE_EINK:

            /* Request clear EInk */
            nextEInkAction_ = EEinkAction::EINK_UPDATE;

            /* Send response */
            writeSize = 1;
            btMgr_->TransmitData((const uint8_t*)"\0", writeSize);
            if(writeSize != 1)
            {
                LOG_ERROR("Could not send Update EInk response.\n");
            }
            break;

        default:
            LOG_ERROR("Unknown command type %d\n", command->type);
            break;
    }
}

#undef CSYSSTATE