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
#include <cstring>   /* String manipulation*/
#include <Types.h>   /* Defined Types */
#include <Logger.h>  /* Logger service */
#include <Arduino.h> /* Arduino service */
#include <HWLayer.h> /* HW Layer service */

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
    memset(pBtnPins_, -1, sizeof(int8_t) * EButtonID::BUTTON_MAX_ID);
    memset(pBtnLastPress_, 0, sizeof(uint64_t) * EButtonID::BUTTON_MAX_ID);
    memset(pBtnStates_, 0, sizeof(EButtonState) * EButtonID::BUTTON_MAX_ID);
}

EErrorCode CBTNMGR::Init(void)
{
    EErrorCode retCode;

    retCode = SetupBtn(EButtonID::BUTTON_ENTER, EButtonPin::ENTER_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init ENTER. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_UP, EButtonPin::UP_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init UP. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_DOWN, EButtonPin::DOWN_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init DOWN. Error %d\n", retCode);
        return retCode;
    }

    retCode = SetupBtn(EButtonID::BUTTON_BACK, EButtonPin::BACK_PIN);
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init BACK. Error %d\n", retCode);
        return retCode;
    }

    return retCode;
}

EErrorCode CBTNMGR::SetupBtn(const EButtonID kBtnId, const EButtonPin kBtnPin)
{
    EErrorCode retCode;

    if(kBtnId < EButtonID::BUTTON_MAX_ID)
    {
        if((int32_t)kBtnId > 33)
        {
            pinMode(kBtnPin, INPUT_PULLDOWN);
        }
        else
        {
            pinMode(kBtnPin, INPUT_PULLUP);
        }
        pBtnPins_[kBtnId] = kBtnPin;

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
        if(pBtnPins_[i] != -1)
        {
            btnState = digitalRead(pBtnPins_[i]);

            if(btnState != 1)
            {
                currTime = HWManager::GetTime();
                /* If this is the first time the button is pressed */
                if(pBtnStates_[i] == EButtonState::BTN_STATE_UP)
                {
                    pBtnStates_[i]    = EButtonState::BTN_STATE_DOWN;
                    pBtnLastPress_[i] = currTime;
                }
                else if(currTime - pBtnLastPress_[i] > BTN_KEEP_WAIT_TIME)
                {
                    pBtnStates_[i] = EButtonState::BTN_STATE_KEEP;
                }
            }
            else
            {
                /* When the button is released, its state is allways UP */
                pBtnStates_[i] = EButtonState::BTN_STATE_UP;
            }
        }
    }
    return EErrorCode::NO_ERROR;
}

EButtonState CBTNMGR::GetButtonState(const EButtonID kBtnId) const
{
    if(kBtnId < EButtonID::BUTTON_MAX_ID)
    {
        return pBtnStates_[kBtnId];
    }

    return EButtonState::BTN_STATE_DOWN;
}

uint64_t CBTNMGR::GetButtonKeepTime(const EButtonID kBtnId) const
{
    if(kBtnId < EButtonID::BUTTON_MAX_ID &&
       pBtnStates_[kBtnId] == EButtonState::BTN_STATE_KEEP)
    {
        return HWManager::GetTime() - pBtnLastPress_[kBtnId];
    }
    return 0;
}

#undef CBTNMGR