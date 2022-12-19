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
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logger service */
#include <CommandControler.h> /* Command controller service */
#include <SystemState.h>      /* System State Service */

/* Header File */
#include <IOButtonMgr.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CBTNMGR nsHWL::CIOButtonMgr

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

CBTNMGR::CIOButtonMgr(void)
{
    /* Init pins and handlers */
    memset(this->btnPins, -1, sizeof(int8_t) * BUTTON_MAX_ID);
    memset(this->btnLastPress, 0, sizeof(uint32_t) * BUTTON_MAX_ID);
    memset(this->btnStates, 0, sizeof(EButtonState) * BUTTON_MAX_ID);
}

EErrorCode CBTNMGR::SetupBtn(const EButtonID btnId, const uint8_t buttonPin)
{
    EErrorCode retCode;

    if(btnId < BUTTON_MAX_ID)
    {
        pinMode(buttonPin, INPUT_PULLUP);
        this->btnPins[btnId] = buttonPin;

        retCode = NO_ERROR;
    }
    else
    {
        retCode = INVALID_PARAM;
    }

    return retCode;
}

EErrorCode CBTNMGR::UpdateState(nsCore::CSystemState & sysState,
                                const nsCore::CCommandControler & comControler)
{
    uint8_t  i;
    uint8_t  btnState;
    uint32_t currTime;

    /* Check all pins */
    for(i = 0; i < BUTTON_MAX_ID; ++i)
    {
        if(this->btnPins[i] != -1)
        {
            btnState = digitalRead(this->btnPins[i]);

            if(btnState != 1)
            {
                currTime = millis();
                /* If this is the first time the button is pressed */
                if(btnStates[i] == BTN_STATE_UP)
                {
                    btnStates[i]    = BTN_STATE_DOWN;
                    btnLastPress[i] = currTime;
                }
                else if(currTime > btnLastPress[i] + BTN_KEEP_WAIT_TIME)
                {
                    btnStates[i] = BTN_STATE_KEEP;
                    sysState.SetButtonKeepTime((EButtonID)i, currTime - btnLastPress[i]);
                }
                /* Rollover of the millis function */
                else if(currTime < btnLastPress[i])
                {
                    /* Compute the distance */
                    currTime = currTime + (0xFFFFFFFF - btnLastPress[i]);
                    if(currTime > BTN_KEEP_WAIT_TIME)
                    {
                        btnStates[i] = BTN_STATE_KEEP;
                        sysState.SetButtonKeepTime((EButtonID)i, currTime);
                    }
                }
            }
            else
            {
                /* When the button is release, its state is allways UP */
                btnStates[i] = BTN_STATE_UP;
                sysState.SetButtonKeepTime((EButtonID)i, 0);
            }
            sysState.SetButtonState((EButtonID)i, btnStates[i]);
        }
    }
    return NO_ERROR;
}

#undef CBTNMGR