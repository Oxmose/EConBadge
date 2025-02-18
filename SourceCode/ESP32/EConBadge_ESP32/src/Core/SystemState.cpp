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
#include <HWMgr.h>            /* Hardware managee */
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logging service */
#include <Arduino.h>          /* Arduino Services */
#include <Storage.h>          /* Storage service */
#include <IOButtonMgr.h>      /* Wakeup PIN */
#include <DisplayInterface.h> /* Display interface */

/* Header File */
#include <SystemState.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define MAX_COMMAND_WAIT 10

#define SPLASH_TIME                3000
#define DEBUG_BTN_PRESS_TIME       3000
#define MENU_BTN_PRESS_TIME        1000

#define SYSTEM_IDLE_TIME 3000000 /* US : 3 sec*/

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/


 typedef enum
 {
    CMD_PING                      = 0,
    CMD_SET_BT_TOKEN              = 1,

    CMD_EINK_CLEAR                = 2,
    CMD_EINK_NEW_IMAGE            = 3,
    CMD_EINK_REMOVE_IMAGE         = 4,
    CMD_EINK_SELECT_IMAGE         = 5,
    CMD_EINK_GET_CURRENT_IMG_NAME = 6,
    CMD_EINK_GET_CURRENT_IMG      = 7,
    MAX_COMMAND_TYPE
 } ECommandType;

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

SystemState::SystemState(IOButtonMgr*        pButtonMgr,
                         DisplayInterface*   pDisplayInterface,
                         EInkDisplayManager* pEinkManager)
{
    pStore_            = Storage::GetInstance();
    pButtonMgr_        = pButtonMgr;
    pEinkManager_      = pEinkManager;
    pDisplayInterface_ = pDisplayInterface;

    memset(
        pButtonsState_,
        BTN_STATE_DOWN,
        sizeof(EButtonState) * BUTTON_MAX_ID
    );
    memset(
        pPrevButtonsState_,
        BTN_STATE_DOWN,
        sizeof(EButtonState) * BUTTON_MAX_ID
    );
    memset(pButtonsKeepTime_, 0, sizeof(uint64_t) * BUTTON_MAX_ID);

    /* Initialize command manager */
    commandsQueueLock_ = xSemaphoreCreateMutex();

    currState_      = SYS_MENU;
    prevState_      = SYS_IDLE;
    currDebugState_ = 0;
    lastEventTime_  = 0;
}

SystemState::~SystemState(void)
{

}

void SystemState::Update(void)
{
    /* Update internal button states */
    UpdateButtonsState();

    /* Check the prioritary events */
    if(currDebugState_ == 0 &&
       pButtonsKeepTime_[BUTTON_UP] >= DEBUG_BTN_PRESS_TIME &&
       pButtonsKeepTime_[BUTTON_DOWN] >= DEBUG_BTN_PRESS_TIME)
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
            case SYS_IDLE:
                ManageIdleState();
                break;
            case SYS_MENU:
                ManageMenuState();
                break;
            default:
                break;
        }
    }
    else
    {
        ManageDebugState();
    }
}

EErrorCode SystemState::EnqueueCommand(SCommandRequest command)
{
    EErrorCode retCode;

    /* Add the command to the queue */
    xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    if(commandsQueue_.size() < MAX_COMMAND_WAIT)
    {
        commandsQueue_.push(std::make_pair(command, true));
        retCode = NO_ERROR;
    }
    else
    {
        retCode = MAX_COMMAND_REACHED;
    }
    xSemaphoreGive(commandsQueueLock_);

    return retCode;
}

EErrorCode SystemState::EnqueueLocalCommand(SCommandRequest command)
{
    EErrorCode retCode;

    /* Add the command to the queue */
    xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    if(commandsQueue_.size() < MAX_COMMAND_WAIT)
    {
        commandsQueue_.push(std::make_pair(command, false));
        retCode = NO_ERROR;
    }
    else
    {
        retCode = MAX_COMMAND_REACHED;
    }
    xSemaphoreGive(commandsQueueLock_);

    return retCode;
}

void SystemState::SetSystemState(const ESystemState kState)
{
    prevState_     = currState_;
    currState_     = kState;
    lastEventTime_ = HWManager::GetTime();
}

void SystemState::UpdateButtonsState(void)
{
    uint8_t      i;
    EButtonState newState;
    uint64_t     newKeepTime;
    uint64_t     timeNow;

    /* Request button state update */
    pButtonMgr_->Update();

    timeNow = HWManager::GetTime();

    for(i = 0; i < BUTTON_MAX_ID; ++i)
    {
        pPrevButtonsState_[i] = pButtonsState_[i];

        newState = pButtonMgr_->GetButtonState((EButtonID)i);
        if(pButtonsState_[i] != newState)
        {
            pButtonsState_[i] = newState;
            lastEventTime_    = timeNow;
        }
        newKeepTime = pButtonMgr_->GetButtonKeepTime((EButtonID)i);
        if(pButtonsKeepTime_[i] != newKeepTime)
        {
            pButtonsKeepTime_[i] = newKeepTime;
            lastEventTime_       = timeNow;
        }
    }
}

void SystemState::ManageDebugState(void)
{
    /* Check if we should switch to next debug state */
    if(pPrevButtonsState_[BUTTON_DOWN] != BTN_STATE_DOWN &&
       pButtonsState_[BUTTON_DOWN] == BTN_STATE_DOWN)
    {
        if(currDebugState_ == 4)
        {
            currDebugState_ = 0;
        }
        ++currDebugState_;
    }
    else if(pPrevButtonsState_[BUTTON_UP] != BTN_STATE_DOWN &&
            pButtonsState_[BUTTON_UP] == BTN_STATE_DOWN)
    {
        if(currDebugState_ == 1)
        {
            currDebugState_ = 5;
        }
        --currDebugState_;
    }
    else if(pPrevButtonsState_[BUTTON_ENTER] != BTN_STATE_DOWN &&
            pButtonsState_[BUTTON_ENTER] == BTN_STATE_DOWN &&
            currDebugState_ == 4)
    {
        currDebugState_ = 0;
        LOG_DEBUG("Disabling debug state\n");
    }
}

void SystemState::ManageIdleState(void)
{
    /* Check if we should enter menu mode */
    if(pButtonsState_[BUTTON_ENTER] == BTN_STATE_KEEP &&
       pButtonsKeepTime_[BUTTON_ENTER] >= MENU_BTN_PRESS_TIME)
    {
        SetSystemState(SYS_MENU);
    }
    else if(prevState_ != SYS_IDLE &&
            pButtonsState_[BUTTON_ENTER] == BTN_STATE_UP)
    {
        LOG_DEBUG("IDLE mode\n");
        SetSystemState(SYS_IDLE);
        pDisplayInterface_->Enable(false);
    }
}

void SystemState::ManageMenuState(void)
{
    if(prevState_ == SYS_MENU)
    {
        /* Update selected item */
        if(pPrevButtonsState_[BUTTON_DOWN] != BTN_STATE_DOWN &&
           pButtonsState_[BUTTON_DOWN] == BTN_STATE_DOWN)
        {

        }
        else if(pPrevButtonsState_[BUTTON_UP] != BTN_STATE_DOWN &&
                pButtonsState_[BUTTON_UP] == BTN_STATE_DOWN)
        {

        }
        /* Check if enter was pressed */
        else if(pPrevButtonsState_[BUTTON_ENTER] != BTN_STATE_DOWN &&
                pButtonsState_[BUTTON_ENTER] == BTN_STATE_DOWN)
        {

        }
        /* Check if back was pressed */
        else if(pPrevButtonsState_[BUTTON_BACK] != BTN_STATE_DOWN &&
                pButtonsState_[BUTTON_BACK] == BTN_STATE_DOWN)
        {

        }

        /* Manage IDLE detection in menu */
        if(currState_ == SYS_MENU)
        {
            if(HWManager::GetTime() - lastEventTime_ > SYSTEM_IDLE_TIME)
            {
                SetSystemState(SYS_IDLE);
            }
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Menu mode\n");

        /* Init menu page and menu item */
        SetSystemState(SYS_MENU);

        pDisplayInterface_->Enable(true);
    }
}