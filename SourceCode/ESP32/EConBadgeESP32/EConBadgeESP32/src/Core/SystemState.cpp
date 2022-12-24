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
#include <cstring>        /* String manipulation*/
#include <Types.h>        /* Defined Types */
#include <Arduino.h>      /* Arduino Services */
#include <Logger.h>       /* Logging service */
#include <version.h>      /* Versioning */
#include <HWLayer.h>      /* Hardware Services */
#include <OLEDScreenDriver.h> /* OLED driver */

/* Header File */
#include <SystemState.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSYSSTATE nsCore::CSystemState

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

CSYSSTATE::CSystemState(void)
{
    /* Init button states */
    memset(this->buttonsState, 0, sizeof(EButtonState) * BUTTON_MAX_ID);
    memset(this->prevButtonsState, 0, sizeof(EButtonState) * BUTTON_MAX_ID);
    memset(this->buttonsKeepTime, 0, sizeof(uint32_t) * BUTTON_MAX_ID);

    prevState     = SYS_IDLE;
    lastEventTime = 0;
}

void CSYSSTATE::Init(nsHWL::COLEDScreenMgr * oledDriver)
{
    this->oledDriver = oledDriver;
}

ESystemState CSYSSTATE::GetSystemState(void) const
{
    return this->currState;
}

void CSYSSTATE::SetSystemState(const ESystemState state)
{
    this->currState = state;
    lastEventTime = millis();
}

uint32_t CSYSSTATE::GetLastEventTime(void) const
{
    return this->lastEventTime;
}

uint8_t CSYSSTATE::GetDebugState(void) const
{
    return this->currDebugState;
}

EButtonState CSYSSTATE::GetButtonState(const EButtonID btnId) const
{
    if(btnId < BUTTON_MAX_ID)
    {
        return this->buttonsState[btnId];
    }

    return BTN_STATE_DOWN;
}

void CSYSSTATE::SetButtonState(const EButtonID btnId, const EButtonState state)
{
    if(btnId < BUTTON_MAX_ID)
    {
        this->prevButtonsState[btnId] = this->buttonsState[btnId];
        if(this->buttonsState[btnId] != state)
        {
            lastEventTime = millis();
        }
        this->buttonsState[btnId] = state;
    }
}

uint32_t CSYSSTATE::GetButtonKeepTime(const EButtonID btnId) const
{
    if(btnId < BUTTON_MAX_ID)
    {
        return this->buttonsKeepTime[btnId];
    }

    return 0;
}
void CSYSSTATE::SetButtonKeepTime(const EButtonID btnId,
                                  const uint32_t keepTime)
{
    if(btnId < BUTTON_MAX_ID)
    {
        if(this->buttonsKeepTime[btnId] != keepTime)
        {
            this->buttonsKeepTime[btnId] = keepTime;
            lastEventTime = millis();
        }
    }
}

EErrorCode CSYSSTATE::ComputeState(void)
{
    EErrorCode retCode;

    retCode = NO_ERROR;

    /* Check the prioritary events */
    if(this->currDebugState == 0 &&
       this->buttonsState[BUTTON_UP] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_UP] >= DEBUG_BTN_PRESS_TIME &&
       this->buttonsState[BUTTON_DOWN] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_DOWN] >= DEBUG_BTN_PRESS_TIME)
    {
        LOG_DEBUG("Enabling to debug state\n");
        this->currDebugState = 1;
    }

    /* If in debug state */
    if(this->currDebugState == 0)
    {
        /* Check the regular states management */
        switch(this->currState)
        {
            case SYS_IDLE:
                ManageIdleState();
                this->prevState = SYS_IDLE;
                break;
            case SYS_START_SPLASH:
                if(this->prevState != this->currState)
                {
                    this->oledDriver->DisplaySplash();
                    this->prevState = this->currState;
                }
                /* After SPLASH_TIME, switch to IDLE */
                if(millis() > SPLASH_TIME)
                {
                    LOG_DEBUG("Switching to idle state\n");
                    this->currState = SYS_IDLE;
                    this->prevState = SYS_START_SPLASH;
                }
                break;
            case SYS_MENU:
            case SYS_MENU_WIFI:
                ManageMenuState();
                this->menu.Display(this->oledDriver);
                this->prevState = this->currState;
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        ManageDebugState();
        this->oledDriver->DisplayDebug(*this);
    }

    return retCode;
}

void CSYSSTATE::ManageDebugState(void)
{
    /* Check if we should switch to next debug state */
    if(this->prevButtonsState[BUTTON_DOWN] != BTN_STATE_DOWN &&
        this->buttonsState[BUTTON_DOWN] == BTN_STATE_DOWN)
    {
        if(this->currDebugState == 3)
        {
            this->currDebugState = 0;
        }
        ++this->currDebugState;
    }
    else if(this->prevButtonsState[BUTTON_UP] != BTN_STATE_DOWN &&
            this->buttonsState[BUTTON_UP] == BTN_STATE_DOWN)
    {
        if(this->currDebugState == 1)
        {
            this->currDebugState = 4;
        }
        --this->currDebugState;
    }
    else if(this->prevButtonsState[BUTTON_ENTER] != BTN_STATE_DOWN &&
            this->buttonsState[BUTTON_ENTER] == BTN_STATE_DOWN &&
            this->currDebugState == 3)
    {
        this->currDebugState = 0;
        this->menu.ForceUpdate();
        LOG_DEBUG("Disabling to debug state\n");
    }
}

void CSYSSTATE::ManageIdleState(void)
{
    /* Check if we should enter menu mode */
    if(this->buttonsState[BUTTON_ENTER] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_ENTER] >= MENU_BTN_PRESS_TIME)
    {
        this->currState = SYS_MENU;
    }
    else
    {
        /* TODO: Check, if after a while, nothing happened, we should enter
         * sleep mode */
    }
}

void CSYSSTATE::ManageMenuState(void)
{
    if(this->prevState == SYS_MENU || this->prevState == SYS_MENU_WIFI)
    {
        /* Update selected item */
        if(this->prevButtonsState[BUTTON_DOWN] != BTN_STATE_DOWN &&
           this->buttonsState[BUTTON_DOWN] == BTN_STATE_DOWN)
        {
            this->menu.SelectNextItem();
        }
        else if(this->prevButtonsState[BUTTON_UP] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_UP] == BTN_STATE_DOWN)
        {
            this->menu.SelectPrevItem();
        }
        /* Check if enter was pressed */
        else if(this->prevButtonsState[BUTTON_ENTER] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_ENTER] == BTN_STATE_DOWN)
        {
            this->menu.ExecuteSelection(*this);
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Switching to menu mode\n");

        /* Init menu page and menu item */
        this->menu.SetPage(MAIN_PAGE_IDX);
    }
}

#undef CSYSSTATE