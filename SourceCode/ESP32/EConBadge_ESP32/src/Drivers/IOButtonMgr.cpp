/*******************************************************************************
 * @file IOButtonMgr.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the IO buttons manager.
 *
 * @details This file contains the IO buttons manager. The file provides the
 * services read input buttons and associate interrupts to the desired pins.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <HWMgr.h>   /* HW Layer service */
#include <Types.h>   /* Defined Types */
#include <Logger.h>  /* Logger service */
#include <Arduino.h> /* Arduino service */
/* Header File */
#include <IOButtonMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Time in microseconds after which we consider a button keeped. */
#define BTN_KEEP_WAIT_TIME 1000000

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

IOButtonMgr::IOButtonMgr(void)
{
    uint8_t  i;

    /* Init pins and handlers */
    memset(pBtnPins_, -1, sizeof(int8_t) * BUTTON_MAX_ID);
    memset(pBtnLastPress_, 0, sizeof(uint64_t) * BUTTON_MAX_ID);
    memset(pBtnStates_, 0, sizeof(EButtonState) * BUTTON_MAX_ID);

    /* Init the GPIOs */
    pBtnPins_[BUTTON_UP]    = GPIO_BTN_UP;
    pBtnPins_[BUTTON_DOWN]  = GPIO_BTN_DOWN;
    pBtnPins_[BUTTON_ENTER] = GPIO_BTN_ENTER;
    pBtnPins_[BUTTON_BACK]  = GPIO_BTN_BACK;

    pBtnPinsMux_[BUTTON_UP]    = GPIO_BTN_UP_MUX;
    pBtnPinsMux_[BUTTON_DOWN]  = GPIO_BTN_DOWN_MUX;
    pBtnPinsMux_[BUTTON_ENTER] = GPIO_BTN_ENTER_MUX;
    pBtnPinsMux_[BUTTON_BACK]  = GPIO_BTN_BACK_MUX;

    /* Setup pinmux */
    for(i = 0; i < BUTTON_MAX_ID; ++i)
    {
        pinMode(pBtnPins_[i], pBtnPinsMux_[i]);
    }
}

void IOButtonMgr::Update(void)
{
    uint8_t  i;
    uint8_t  btnState;
    uint64_t currTime;

    /* Check all pins */
    for(i = 0; i < BUTTON_MAX_ID; ++i)
    {
        btnState = digitalRead(pBtnPins_[i]);

        /* Manage pinmux */
        if(pBtnPinsMux_[i] == INPUT_PULLUP)
        {
            btnState = !btnState;
        }

        if(btnState != 0)
        {
            currTime = HWManager::GetTime();
            /* If this is the first time the button is pressed */
            if(pBtnStates_[i] == BTN_STATE_UP)
            {
                pBtnStates_[i]    = BTN_STATE_DOWN;
                pBtnLastPress_[i] = currTime;
            }
            else if(currTime - pBtnLastPress_[i] > BTN_KEEP_WAIT_TIME)
            {
                pBtnStates_[i] = BTN_STATE_KEEP;
            }
        }
        else
        {
            /* When the button is released, its state is allways UP */
            pBtnStates_[i] = BTN_STATE_UP;
        }
    }
}

EButtonState IOButtonMgr::GetButtonState(const EButtonID kBtnId) const
{
    if(kBtnId < BUTTON_MAX_ID)
    {
        return pBtnStates_[kBtnId];
    }

    return BTN_STATE_DOWN;
}

uint64_t IOButtonMgr::GetButtonKeepTime(const EButtonID kBtnId) const
{
    if(kBtnId < BUTTON_MAX_ID && pBtnStates_[kBtnId] == BTN_STATE_KEEP)
    {
        return HWManager::GetTime() - pBtnLastPress_[kBtnId];
    }
    return 0;
}