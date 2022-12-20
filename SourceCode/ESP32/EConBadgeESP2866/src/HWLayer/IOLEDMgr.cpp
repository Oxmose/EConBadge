/*******************************************************************************
 * @file IOLEDMgr.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the IO LED manager.
 *
 * @details This file contains the IO LED manager. The file provides the
 * services to write LED state.
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
#include <IOLEDMgr.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CLEDMGR nsHWL::CIOLEDMgr

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

CLEDMGR::CIOLEDMgr(void)
{
    /* Init pins and handlers */
    memset(this->LEDPins, -1, sizeof(int8_t) * LED_MAX_ID);
    memset(this->LEDLastEvent, 0, sizeof(uint32_t) * LED_MAX_ID);
    memset(this->LEDStates, 0, sizeof(ELEDState) * LED_MAX_ID);
}

EErrorCode CLEDMGR::SetupLED(const ELEDID LEDId, const uint8_t LEDPin)
{
    EErrorCode retCode;

    if(LEDId < LED_MAX_ID)
    {
        pinMode(LEDPin, OUTPUT);
        this->LEDPins[LEDId] = LEDPin;

        retCode = NO_ERROR;
    }
    else
    {
        retCode = INVALID_PARAM;
    }

    return retCode;
}

EErrorCode CLEDMGR::UpdateState(nsCore::CSystemState & sysState,
                                const nsCore::CCommandControler & comControler)
{
    EErrorCode retCode;

    retCode = NO_ERROR;

    /* First check debug state */
    if(sysState.GetDebugState() == 0)
    {
        switch(sysState.GetSystemState())
        {
            case SYS_IDLE:
                SetState(LED_MAIN, LED_STATE_OFF);
                SetState(LED_AUX, LED_STATE_OFF);
                break;
            case SYS_START_SPLASH:
                BlinkLED(LED_MAIN, 250, LED_STATE_OFF);
                SetState(LED_AUX, LED_STATE_OFF);
                break;
            case SYS_MENU:
                SetState(LED_MAIN, LED_STATE_OFF);
                SetState(LED_AUX, LED_STATE_OFF);
                break;
            case SYS_WAITING_WIFI_CLIENT:
                BlinkLED(LED_MAIN, 500, LED_STATE_OFF);
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        /* Debug lights */
        BlinkLED(LED_MAIN, 1000, LED_STATE_OFF);
        BlinkLED(LED_AUX, 1000, LED_STATE_ON);
    }

    return retCode;
}

void CLEDMGR::SetState(const nsCommon::ELEDID LEDId,
                       const nsCommon::ELEDState state)
{
    if(LEDId < LED_MAX_ID && this->LEDPins[LEDId] != -1)
    {
        digitalWrite(this->LEDPins[LEDId], state);
        this->LEDStates[LEDId]    = state;
        this->LEDLastEvent[LEDId] = 0;
    }
}

void CLEDMGR::BlinkLED(const nsCommon::ELEDID LEDId,
                       const uint32_t period,
                       const nsCommon::ELEDState startState)
{
    uint32_t time;
    uint32_t elapsed;

    if(LEDId < LED_MAX_ID && this->LEDPins[LEDId] != -1)
    {
        if(this->LEDLastEvent[LEDId] != 0)
        {
            time = millis();
            /* Rollover management */
            if(this->LEDLastEvent[LEDId] > time)
            {
                elapsed = 0xFFFFFFFF - this->LEDLastEvent[LEDId] + time;
            }
            else
            {
                elapsed = time - this->LEDLastEvent[LEDId];
            }
            /* Check if we should update */
            if(elapsed >= period / 2)
            {
                this->LEDLastEvent[LEDId] = time;
                this->LEDStates[LEDId] = (ELEDState)(!this->LEDStates[LEDId]);
                digitalWrite(this->LEDPins[LEDId], this->LEDStates[LEDId]);
            }
        }
        else
        {
            /* This is the first time we call this function */
            digitalWrite(this->LEDPins[LEDId], startState);
            this->LEDLastEvent[LEDId] = millis();
            this->LEDStates[LEDId] = startState;
        }
    }
}

#undef CLEDMGR