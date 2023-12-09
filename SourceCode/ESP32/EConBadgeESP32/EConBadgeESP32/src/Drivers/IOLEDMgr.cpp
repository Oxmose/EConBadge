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
#include <cstring>       /* String manipulation*/
#include <Types.h>       /* Defined Types */
#include <Logger.h>      /* Logger service */
#include <HWLayer.h>     /* HW Layer service */
#include <SystemState.h> /* System State Service */

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
    pSystemState_ = systemState;

    /* Init pins and handlers */
    memset(pLedPins_, 0, sizeof(uint8_t) * ELEDID::LED_MAX_ID);
    memset(pLedLastEvent_, 0, sizeof(uint64_t) * ELEDID::LED_MAX_ID);
    memset(pLedStates_, 0, sizeof(ELEDState) * ELEDID::LED_MAX_ID);
}

EErrorCode CLEDMGR::SetupLED(const ELEDID kLedID, const ELEDPin kLedPin)
{
    EErrorCode retCode;

    if(kLedID < ELEDID::LED_MAX_ID)
    {
        pinMode(kLedPin, OUTPUT);
        pLedPins_[kLedID] = kLedPin;

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
    if(pSystemState_->GetDebugState() == 0)
    {
        switch(pSystemState_->GetSystemState())
        {
            case ESystemState::SYS_IDLE:
                SetState(ELEDID::LED_MAIN, ELEDState::LED_STATE_OFF);
                break;
            case ESystemState::SYS_START_SPLASH:
                BlinkLED(ELEDID::LED_MAIN, 250, ELEDState::LED_STATE_OFF);
                break;
            case ESystemState::SYS_MENU:
                SetState(ELEDID::LED_MAIN, ELEDState::LED_STATE_ON);
                break;
            default:
                break;
        }
    }
    else
    {
        /* Debug lights */
        BlinkLED(ELEDID::LED_MAIN, 1000, ELEDState::LED_STATE_OFF);
    }
}

void CLEDMGR::SetState(const ELEDID kLedID,
                       const ELEDState kState)
{
    if(kLedID < ELEDID::LED_MAX_ID && pLedPins_[kLedID] != -1)
    {
        digitalWrite(pLedPins_[kLedID], kState);

        pLedStates_[kLedID]    = kState;
        pLedLastEvent_[kLedID] = 0;
    }
}

void CLEDMGR::BlinkLED(const ELEDID kLedID,
                       const uint32_t kPeriod,
                       const ELEDState kStartState)
{
    uint64_t time;
    uint64_t elapsed;

    if(kLedID < ELEDID::LED_MAX_ID && pLedPins_[kLedID] != -1)
    {
        if(pLedLastEvent_[kLedID] != 0)
        {
            time    = HWManager::GetTime();
            elapsed = time - pLedLastEvent_[kLedID];

            /* Check if we should update */
            if(elapsed >= kPeriod / 2)
            {
                pLedLastEvent_[kLedID] = time;
                pLedStates_[kLedID]    = (ELEDState)(!pLedStates_[kLedID]);

                digitalWrite(pLedPins_[kLedID], pLedStates_[kLedID]);
            }
        }
        else
        {
            /* This is the first time we call this function */
            digitalWrite(pLedPins_[kLedID], kStartState);

            pLedLastEvent_[kLedID] = HWManager::GetTime();
            pLedStates_[kLedID]    = kStartState;
        }
    }
}
#undef CLEDMGR