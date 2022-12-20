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
static const uint8_t MENU_PAGE_ITEM_COUNT[MENU_PAGE_COUNT] = {
    3,
    0,
    0,
    1
};

static const char * MENU_PAGE_ITEM_MAIN[3] = {
    "WIFI Menu",
    "BT Menu",
    "About"
};

static const char * MENU_PAGE_ITEM_ABOUT[1] = {
    "Back\n\n"
    "Built by Olson\n"
    "Version " VERSION_SHORT "\n"
    PROTO_REV
};

static const char ** MENU_PAGE_ITEMS[MENU_PAGE_COUNT] = {
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_ABOUT
};

static const char * MENU_PAGE_TITLES[MENU_PAGE_COUNT] = {
    "Main Menu",
    "WIFI Menu",
    "BT Menu",
    "About EConBadge"
};


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
    memset(this->currMenuItem, 0, sizeof(uint8_t) * MENU_PAGE_COUNT);

    prevState     = SYS_IDLE;
    lastEventTime = 0;
}

ESystemState CSYSSTATE::GetSystemState(void) const
{
    return this->currState;
}

void CSYSSTATE::SetSystemState(const ESystemState state)
{
    this->currState = state;
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
        this->buttonsState[btnId] = state;

        lastEventTime = millis();
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
        this->buttonsKeepTime[btnId] = keepTime;

        lastEventTime = millis();
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
                /* After SPLASH_TIME, switch to IDLE */
                if(millis() > SPLASH_TIME)
                {
                    LOG_DEBUG("Switching to idle state\n");
                    this->currState = SYS_IDLE;
                    this->prevState = SYS_START_SPLASH;
                }
                break;
            case SYS_MENU:
                ManageMenuState();
                this->prevState = SYS_MENU;
                break;
            case SYS_WAITING_WIFI_CLIENT:
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        ManageDebugState();
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

uint8_t CSYSSTATE::GetMenuPage(void) const
{
    return this->currMenuItem[this->currMenuPage];
}

void CSYSSTATE::ManageMenuState(void)
{
    if(this->prevState == SYS_MENU)
    {
        /* Update selected item */
        if(this->prevButtonsState[BUTTON_DOWN] != BTN_STATE_DOWN &&
           this->buttonsState[BUTTON_DOWN] == BTN_STATE_DOWN)
        {
            this->currMenuItem[this->currMenuPage] =
                (this->currMenuItem[this->currMenuPage] + 1) %
                MENU_PAGE_ITEM_COUNT[this->currMenuPage];
        }
        else if(this->prevButtonsState[BUTTON_UP] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_UP] == BTN_STATE_DOWN)
        {
            if(this->currMenuItem[this->currMenuPage] == 0)
            {
                this->currMenuItem[this->currMenuPage] =
                    MENU_PAGE_ITEM_COUNT[this->currMenuPage];
            }
            --this->currMenuItem[this->currMenuPage];
        }
        /* Check if enter was pressed */
        else if(this->prevButtonsState[BUTTON_ENTER] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_ENTER] == BTN_STATE_DOWN)
        {
            ManageMenuAction();
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Switching to menu mode\n");

        /* Init menu page and menu item */
        this->currMenuPage = 0;
        this->currMenuItem[0] = 0;
    }
}

void CSYSSTATE::ManageMenuAction(void)
{
    /* Main page action */
    if(this->currMenuPage == 0)
    {
        switch(this->currMenuItem[this->currMenuPage])
        {
            /* About page entry */
            case 2:
                this->currMenuPage = 3;
                this->currMenuItem[3] = 0;
                break;
            default:
                break;
        }
    }
    /* About page action */
    else if(this->currMenuPage == 3)
    {
        switch(this->currMenuItem[this->currMenuPage])
        {
            /* About page back button */
            case 0:
                this->currMenuPage = 0;
                break;
            default:
                break;
        }
    }
}

void CSYSSTATE::GetCurrentMenu(const char *** pMenuItem,
                               const char **  pMenuTitle,
                               uint8_t * pSelectedItemIdx,
                               uint8_t * pItemsCount) const
{
    *pMenuItem        = MENU_PAGE_ITEMS[this->currMenuPage];
    *pMenuTitle       = MENU_PAGE_TITLES[this->currMenuPage];
    *pItemsCount      = MENU_PAGE_ITEM_COUNT[this->currMenuPage];
    *pSelectedItemIdx = this->currMenuItem[this->currMenuPage];
}

#undef CSYSSTATE