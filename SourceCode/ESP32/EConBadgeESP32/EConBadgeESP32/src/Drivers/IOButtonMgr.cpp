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
#include <cstring>            /* String manipulation*/
#include <Arduino.h>          /* IO service */
#include <Types.h>            /* Defined Types */
#include <HWLayer.h>     /* HW Layer service */
#include <Logger.h>      /* Logger service */

/* Header File */
#include <IOButtonMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CBTNMGR IOButtonMgr

#define BTN_KEEP_WAIT_TIME 1000

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

CBTNMGR::IOButtonMgr(void)
{
    /* Init pins and handlers */
    memset(btnPins_, -1, sizeof(int8_t) * EButtonID::BUTTON_MAX_ID);
    memset(btnLastPress_, 0, sizeof(uint64_t) * EButtonID::BUTTON_MAX_ID);
    memset(btnStates_, 0, sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
}

EErrorCode CBTNMGR::Init(void)
{
    EErrorCode retCode;

    retCode = SetupBtn(EButtonID::BUTTON_ENTER, EButtonPin::ENTER_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Could not init Enter button. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_UP, EButtonPin::UP_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Could not init Up Button. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_DOWN, EButtonPin::DOWN_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Could not init Down Button. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_BACK, EButtonPin::BACK_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Could not init Back Button. Error %d\n", retCode);
        return retCode;
    }

    return retCode;
}

EErrorCode CBTNMGR::SetupBtn(const EButtonID btnId, const EButtonPin buttonPin)
{
    EErrorCode retCode;

    if(btnId < EButtonID::BUTTON_MAX_ID)
    {
        pinMode(buttonPin, INPUT_PULLUP);
        btnPins_[btnId] = buttonPin;

        retCode = EErrorCode::NO_ERROR;
    }
    else
    {
        retCode = EErrorCode::INVALID_PARAM;
    }

    return retCode;
}

EErrorCode CBTNMGR::UpdateState(void)
{
    uint8_t  i;
    uint8_t  btnState;
    uint64_t currTime;

    /* Check all pins */
    for(i = 0; i < EButtonID::BUTTON_MAX_ID; ++i)
    {
        if(btnPins_[i] != -1)
        {
            btnState = digitalRead(btnPins_[i]);

            if(btnState != 1)
            {
                currTime = HWManager::GetTime();
                /* If this is the first time the button is pressed */
                if(btnStates_[i] == EButtonState::BTN_STATE_UP)
                {
                    btnStates_[i]    = EButtonState::BTN_STATE_DOWN;
                    btnLastPress_[i] = currTime;
                }
                else if(currTime - btnLastPress_[i] > BTN_KEEP_WAIT_TIME)
                {
                    btnStates_[i] = EButtonState::BTN_STATE_KEEP;
                }
            }
            else
            {
                /* When the button is released, its state is allways UP */
                btnStates_[i] = EButtonState::BTN_STATE_UP;
            }
        }
    }
    return EErrorCode::NO_ERROR;
}

EButtonState CBTNMGR::GetButtonState(const EButtonID btnId) const
{
    if(btnId < EButtonID::BUTTON_MAX_ID)
    {
        return btnStates_[btnId];
    }

    return EButtonState::BTN_STATE_DOWN;
}

uint64_t CBTNMGR::GetButtonKeepTime(const EButtonID btnId) const
{
    if(btnId < EButtonID::BUTTON_MAX_ID &&
       btnStates_[btnId] == EButtonState::BTN_STATE_KEEP)
    {
        return HWManager::GetTime() - btnLastPress_[btnId];
    }
    return 0;
}

#undef CBTNMGR