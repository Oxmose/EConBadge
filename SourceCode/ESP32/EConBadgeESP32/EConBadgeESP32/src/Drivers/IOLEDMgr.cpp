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
#include <SystemState.h>      /* System State Service */
#include <HWLayer.h>     /* HW Layer service */

/* Header File */
#include <IOLEDMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CLEDMGR IOLEDMgr

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

CLEDMGR::IOLEDMgr(SystemState * systemState)
{
    systemState_ = systemState;

    /* Init pins and handlers */
    memset(ledPins_, 0, sizeof(uint8_t) * ELEDID::LED_MAX_ID);
    memset(ledLastEvent_, -1, sizeof(int8_t) * ELEDID::LED_MAX_ID);
    memset(ledLastEvent_, 0, sizeof(uint64_t) * ELEDID::LED_MAX_ID);
    memset(ledStates_, 0, sizeof(ELEDState) * ELEDID::LED_MAX_ID);
}

EErrorCode CLEDMGR::SetupLED(const ELEDID ledID, const ELEDPin ledPin)
{
    EErrorCode retCode;

    if(ledID < ELEDID::LED_MAX_ID)
    {
        pinMode(ledPin, OUTPUT);
        ledPins_[ledID] = ledPin;

        retCode = EErrorCode::NO_ERROR;
    }
    else
    {
        retCode = EErrorCode::INVALID_PARAM;
    }

    return retCode;
}

void CLEDMGR::Update(void)
{
    /* First check debug state */
    if(systemState_->GetDebugState() == 0)
    {
        switch(systemState_->GetSystemState())
        {
            case ESystemState::SYS_IDLE:
                SetState(ELEDID::LED_MAIN, ELEDState::LED_STATE_OFF);
                SetState(ELEDID::LED_AUX, ELEDState::LED_STATE_OFF);
                break;
            case ESystemState::SYS_START_SPLASH:
                BlinkLED(ELEDID::LED_MAIN, 250, ELEDState::LED_STATE_OFF);
                SetState(ELEDID::LED_AUX, ELEDState::LED_STATE_OFF);
                break;
            case ESystemState::SYS_MENU:
                SetState(ELEDID::LED_MAIN, ELEDState::LED_STATE_OFF);
                SetState(ELEDID::LED_AUX, ELEDState::LED_STATE_OFF);
                break;
            default:
                break;
        }
    }
    else
    {
        /* Debug lights */
        BlinkLED(ELEDID::LED_MAIN, 1000, ELEDState::LED_STATE_OFF);
        BlinkLED(ELEDID::LED_AUX, 1000, ELEDState::LED_STATE_ON);
    }
}

void CLEDMGR::SetState(const ELEDID ledID,
                       const ELEDState state)
{
    if(ledID < ELEDID::LED_MAX_ID && ledPins_[ledID] != -1)
    {
        digitalWrite(ledPins_[ledID], state);

        ledStates_[ledID]    = state;
        ledLastEvent_[ledID] = 0;
    }
}

void CLEDMGR::BlinkLED(const ELEDID ledID,
                       const uint32_t period,
                       const ELEDState startState)
{
    uint64_t time;
    uint64_t elapsed;

    if(ledID < ELEDID::LED_MAX_ID && ledPins_[ledID] != -1)
    {
        if(ledLastEvent_[ledID] != 0)
        {
            time    = HWManager::GetTime();
            elapsed = time - ledLastEvent_[ledID];

            /* Check if we should update */
            if(elapsed >= period / 2)
            {
                ledLastEvent_[ledID] = time;
                ledStates_[ledID]    = (ELEDState)(!ledStates_[ledID]);

                digitalWrite(ledPins_[ledID], ledStates_[ledID]);
            }
        }
        else
        {
            /* This is the first time we call this function */
            digitalWrite(ledPins_[ledID], startState);

            ledLastEvent_[ledID] = HWManager::GetTime();
            ledStates_[ledID]    = startState;
        }
    }
}
#undef CLEDMGR