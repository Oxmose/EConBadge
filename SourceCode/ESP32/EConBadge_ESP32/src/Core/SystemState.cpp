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
#include <HWMgr.h>            /* Hardware manager */
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logging service */
#include <Arduino.h>          /* Arduino services */
#include <Storage.h>          /* Storage service */
#include <Updater.h>          /* Updater service */
#include <LEDBorder.h>        /* LED border manager */
#include <BatteryMgr.h>       /* Battery manager */
#include <IOButtonMgr.h>      /* Wakeup PIN */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <DisplayInterface.h> /* Display interface */
#include <WaveshareEInkMgr.h> /* EInk display manager */

/* Header File */
#include <SystemState.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define MAX_COMMAND_WAIT 10

#define DEBUG_BTN_PRESS_TIME       3000000
#define MENU_BTN_PRESS_TIME        1000000

#define SYSTEM_IDLE_TIME 15000000 /* US : 15 sec*/

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

SystemState::SystemState(IOButtonMgr*        pButtonMgr,
                         DisplayInterface*   pDisplayInterface,
                         BluetoothManager*   pBlueToothManager,
                         EInkDisplayManager* pEinkManager,
                         LEDBorder*          pLEDBorder,
                         BatteryManager*     pBatteryMgr)
{
    pStore_            = Storage::GetInstance();
    pBatteryMgr_       = pBatteryMgr;
    pButtonMgr_        = pButtonMgr;
    pEinkManager_      = pEinkManager;
    pDisplayInterface_ = pDisplayInterface;
    pBlueToothManager_ = pBlueToothManager;
    pLEDBorder_        = pLEDBorder;
    pMenu_             = new Menu(pDisplayInterface_);

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

    currState_      = SYS_SPLASH;
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

    /* Execute the commands is nedded */
    ExecuteCommands();

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
            case SYS_SPLASH:
                SetSystemState(SYS_MENU);
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

EErrorCode SystemState::EnqueueCommand(SCommandRequest& rCommand)
{
    EErrorCode retCode;

    /* Add the command to the queue */
    xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    if(commandsQueue_.size() < MAX_COMMAND_WAIT)
    {
        commandsQueue_.push(std::make_pair(rCommand, true));
        retCode = NO_ERROR;
    }
    else
    {
        retCode = MAX_COMMAND_REACHED;
    }
    xSemaphoreGive(commandsQueueLock_);

    return retCode;
}

EErrorCode SystemState::EnqueueLocalCommand(SCommandRequest& rCommand)
{
    EErrorCode retCode;

    /* Add the command to the queue */
    xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    if(commandsQueue_.size() < MAX_COMMAND_WAIT)
    {
        commandsQueue_.push(std::make_pair(rCommand, false));
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

void SystemState::ExecuteCommands(void)
{
    std::pair<SCommandRequest, bool> request;
    SCommandResponse                 response;
    bool                             result;

    /* Add the command to the queue */
    xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    while(commandsQueue_.size() > 0)
    {
        /* Get the request and let go the lock the time we execute the
         * command
         */
        request = commandsQueue_.front();
        commandsQueue_.pop();
        xSemaphoreGive(commandsQueueLock_);

        /* Init response */
        response.header.identifier = request.first.header.identifier;
        memcpy(response.header.pToken,
               request.first.header.pToken,
               COMM_TOKEN_SIZE);

        switch(request.first.header.type)
        {
            case CMD_PING:
                response.header.errorCode = NO_ERROR;
                response.header.size = 4;
                memcpy(response.pResponse, "PONG", 4);
                LOG_DEBUG("PONG\n");
                break;

            case CMD_SET_BT_TOKEN:
                result = pBlueToothManager_->SetToken(
                    std::string((char*)request.first.pCommand)
                );
                if(result)
                {
                    response.header.errorCode = NO_ERROR;
                }
                else
                {
                    response.header.errorCode = INVALID_PARAM;
                }
                response.header.size = 0;
                break;

            case CMD_EINK_CLEAR:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Clearing EInk display, please wait..."
                );
                pEinkManager_->Clear(response);
                pDisplayInterface_->HidePopup();
                break;
            case CMD_EINK_NEW_IMAGE:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Updating new image, please wait..."
                );
                pEinkManager_->DisplayNewImage(
                    std::string((char*)request.first.pCommand),
                    response
                );
                pDisplayInterface_->HidePopup();
                break;
            case CMD_EINK_REMOVE_IMAGE:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Removing EInk image, please wait..."
                );
                pEinkManager_->RemoveImage(
                    std::string((char*)request.first.pCommand),
                    response
                );
                pDisplayInterface_->HidePopup();
                break;
            case CMD_EINK_SELECT_IMAGE:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Updating new image, please wait..."
                );
                pEinkManager_->SetDisplayedImage(
                    std::string((char*)request.first.pCommand),
                    response
                );
                pDisplayInterface_->HidePopup();
                break;
            case CMD_EINK_GET_CURRENT_IMG_NAME:
                pEinkManager_->GetDisplayedImageName(response);
                break;
            case CMD_EINK_GET_IMAGE_DATA:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Uploading image, please wait..."
                );
                pEinkManager_->SendImageData(
                    std::string((char*)request.first.pCommand),
                    response
                );
                pDisplayInterface_->HidePopup();
                break;
            case CMD_EINK_GET_IMAGE_LIST:
                pDisplayInterface_->DisplayPopup(
                    "EInk Update",
                    "Uploading image list, please wait..."
                );
                pEinkManager_->SendImageList(response);
                pDisplayInterface_->HidePopup();
                break;
            case CMD_FACTORY_RESET:
                pStore_->Format();
                pMenu_->Reset();

                /* Init response */
                response.header.errorCode = NO_ERROR;
                response.header.size = 0;
                response.header.identifier = request.first.header.identifier;
                memcpy(response.header.pToken,
                       request.first.header.pToken,
                       COMM_TOKEN_SIZE);

                /* Send the bluetooth response */
                pBlueToothManager_->SendCommandResponse(response);

                HWManager::DelayExecUs(1000000);
                ESP.restart();
                break;

            case CMD_SET_OWNER:
                SetOwner((char*)request.first.pCommand, response);
                break;
            case CMD_SET_CONTACT:
                SetContact((char*)request.first.pCommand, response);
                break;
            case CMD_GET_OWNER:
                GetOwner(response);
                break;
            case CMD_GET_CONTACT:
                GetContact(response);
                break;

            case CMD_FIRMWARE_UPDATE:
                PerformUpdate(request.first.pCommand, response);
                break;

            case CMD_LEDBORDER_SET_ENABLE:
                pLEDBorder_->Enable(request.first.pCommand[0]);
                response.header.errorCode = NO_ERROR;
                response.header.size = 0;
                break;
            case CMD_LEDBORDER_GET_ENABLE:
                response.header.errorCode = NO_ERROR;
                response.header.size = 1;
                response.pResponse[0] = pLEDBorder_->IsEnabled();
                break;
            case CMD_LEDBORDER_INC_BRIGHTNESS:
                pLEDBorder_->IncreaseBrightness(response);
                break;
            case CMD_LEDBORDER_DEC_BRIGHTNESS:
                pLEDBorder_->ReduceBrightness(response);
                break;
            case CMD_LEDBORDER_SET_BRIGHTNESS:
                pLEDBorder_->SetBrightness(request.first.pCommand, response);
                break;
            case CMD_LEDBORDER_GET_BRIGHTNESS:
                response.header.errorCode = NO_ERROR;
                response.header.size = 1;
                response.pResponse[0] = pLEDBorder_->GetBrightness();
                break;
            case CMD_LEDBORDER_CLEAR:
                pLEDBorder_->Clear(response);
                break;
            case CMD_LEDBORDER_ADD_PATTERN:
                pLEDBorder_->AddPattern(request.first.pCommand, response);
                break;
            case CMD_LEDBORDER_REMOVE_PATTERN:
                pLEDBorder_->RemovePattern(request.first.pCommand, response);
                break;
            case CMD_LEDBORDER_CLEAR_PATTERNS:
                pLEDBorder_->ClearPatterns(response);
                break;
            case CMD_LEDBORDER_GET_PATTERNS:
                pLEDBorder_->GetPatterns(response);
                break;
            case CMD_LEDBORDER_ADD_ANIMATION:
                pLEDBorder_->AddAnimation(request.first.pCommand, response);
                break;
            case CMD_LEDBORDER_REMOVE_ANIMATION:
                pLEDBorder_->RemoveAnimation(request.first.pCommand, response);
                break;
            case CMD_LEDBORDER_CLEAR_ANIMATIONS:
                pLEDBorder_->ClearAnimation(response);
                break;
            case CMD_LEDBORDER_GET_ANIMATIONS:
                pLEDBorder_->GetAnimations(response);
                break;

            default:
                response.header.errorCode = INVALID_COMMAND_REQ;
                response.header.size = 0;
        }

        /* Check if a response shall be given */
        if(request.second)
        {
            /* Init response */
            response.header.identifier = request.first.header.identifier;
            memcpy(response.header.pToken,
                   request.first.header.pToken,
                   COMM_TOKEN_SIZE);

            /* Send the bluetooth response */
            pBlueToothManager_->SendCommandResponse(response);
        }

        lastEventTime_ = HWManager::GetTime();

        /* Take back the lock */
        xSemaphoreTake(commandsQueueLock_, portMAX_DELAY);
    }
    xSemaphoreGive(commandsQueueLock_);
}

void SystemState::ManageDebugState(void)
{
    uint8_t      i;
    SDebugInfo_t debugInfo;

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
    pDisplayInterface_->Enable(true);

    debugInfo.debugState = currDebugState_;

    debugInfo.systemState = currState_;
    debugInfo.lastEventTime = lastEventTime_;

    for(i = 0; i < BUTTON_MAX_ID; ++i)
    {
        debugInfo.buttonsState[i] = pButtonsState_[i];
        debugInfo.buttonsKeepTime[i] = pButtonsKeepTime_[i];
    }

    debugInfo.batteryState = pBatteryMgr_->GetPercentage();

    pDisplayInterface_->SetDebugDisplay(debugInfo);
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

    }
    pDisplayInterface_->Enable(false);
}

void SystemState::ManageMenuState(void)
{
    SButtonAction   btnAction;
    SCommandRequest request;

    btnAction.id = BUTTON_MAX_ID;

    if(prevState_ == SYS_MENU)
    {
        /* Manage IDLE detection in menu */
        if(currState_ == SYS_MENU)
        {
            /* Update selected item */
            btnAction.state = BTN_STATE_DOWN;
            if(pPrevButtonsState_[BUTTON_DOWN] != BTN_STATE_DOWN &&
            pButtonsState_[BUTTON_DOWN] == BTN_STATE_DOWN)
            {
                btnAction.id = BUTTON_DOWN;
            }
            else if(pPrevButtonsState_[BUTTON_UP] != BTN_STATE_DOWN &&
                    pButtonsState_[BUTTON_UP] == BTN_STATE_DOWN)
            {
                btnAction.id = BUTTON_UP;
            }
            /* Check if enter was pressed */
            else if(pPrevButtonsState_[BUTTON_ENTER] != BTN_STATE_DOWN &&
                    pButtonsState_[BUTTON_ENTER] == BTN_STATE_DOWN)
            {
                btnAction.id = BUTTON_ENTER;
            }
            /* Check if back was pressed */
            else if(pPrevButtonsState_[BUTTON_BACK] != BTN_STATE_DOWN &&
                    pButtonsState_[BUTTON_BACK] == BTN_STATE_DOWN)
            {
                btnAction.id = BUTTON_BACK;
            }

            if(btnAction.id != BUTTON_MAX_ID)
            {
                pMenu_->SendButtonAction(btnAction, request);
                if(request.header.identifier != -1)
                {
                    EnqueueLocalCommand(request);
                }
            }

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
        pMenu_->Display();
    }
}

void SystemState::SetOwner(const char* kpOwner, SCommandResponse& rReponse)
{
    /* Check size */
    if(strlen(kpOwner) > LINE_SIZE_CHAR * 2)
    {
        rReponse.header.errorCode = DATA_TOO_LONG;
        rReponse.header.size = 0;
        return;
    }

    /* Save to storage */
    if(pStore_->SetContent(OWNER_FILE_PATH, std::string(kpOwner), true))
    {
        rReponse.header.errorCode = NO_ERROR;
        rReponse.header.size = 0;
    }
    else
    {
        rReponse.header.errorCode = ACTION_FAILED;
        rReponse.header.size = 0;
    }
}

void SystemState::SetContact(const char* kpContact, SCommandResponse& rReponse)
{
    /* Check size */
    if(strlen(kpContact) > LINE_SIZE_CHAR * 2)
    {
        rReponse.header.errorCode = DATA_TOO_LONG;
        rReponse.header.size = 0;
        return;
    }

    /* Save to storage */
    if(pStore_->SetContent(CONTACT_FILE_PATH, std::string(kpContact), true))
    {
        rReponse.header.errorCode = NO_ERROR;
        rReponse.header.size = 0;
    }
    else
    {
        rReponse.header.errorCode = ACTION_FAILED;
        rReponse.header.size = 0;
    }
}

void SystemState::GetOwner(SCommandResponse& rReponse)
{
    std::string contentStr;

    pStore_->GetContent(OWNER_FILE_PATH, "", contentStr, true);
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = contentStr.size();
    memcpy(rReponse.pResponse, contentStr.c_str(), contentStr.size());
}

void SystemState::GetContact(SCommandResponse& rReponse)
{
    std::string contentStr;

    pStore_->GetContent(CONTACT_FILE_PATH, "", contentStr, true);
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = contentStr.size();
    memcpy(rReponse.pResponse, contentStr.c_str(), contentStr.size());
}

void SystemState::PerformUpdate(const uint8_t*    kpData,
                                SCommandResponse& rReponse)
{
    uint8_t     progress;
    Updater     updater(pBlueToothManager_);
    std::string popupContent;

    /* Request the update */
    rReponse.header.errorCode = NO_ERROR;
    updater.RequestUpdate(kpData, rReponse);

    /* Monitor the update and stop when needed */
    while(rReponse.header.errorCode == NO_ERROR)
    {
        progress = updater.GetProgress();
        popupContent = "Progress: " + std::to_string(progress) + "%%";
        pDisplayInterface_->DisplayPopup("Updating...", popupContent);
        if(progress == 100)
        {
            HWManager::DelayExecUs(100000);
            break;
        }

        lastEventTime_ = HWManager::GetTime();

        HWManager::DelayExecUs(25000);
    }

    pDisplayInterface_->HidePopup();
}