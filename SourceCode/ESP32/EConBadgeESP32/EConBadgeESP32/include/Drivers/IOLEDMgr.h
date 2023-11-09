/*******************************************************************************
 * @file IOLEDMgr.h
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

#ifndef __DRIVERS_IOLEDMGR_H_
#define __DRIVERS_IOLEDMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */

#include <Types.h>            /* Defined Types */
#include <SystemState.h>      /* System State Service */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON  = 1
} ELEDState;

typedef enum
{
    LED_MAIN     = 0,
    LED_AUX      = 1,
    LED_MAX_ID
} ELEDID;

typedef enum
{
    MAIN_PIN = 33,
    AUX_PIN  = 32
} ELEDPin;

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
 * CLASSES
 ******************************************************************************/

class CIOLEDMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CIOLEDMgr(CSystemState * systemState);

        EErrorCode SetupLED(const ELEDID ledID, const ELEDPin ledPin);

        void Update(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetState(const ELEDID ledID, const ELEDState state);
        void BlinkLED(const ELEDID ledID,
                      const uint32_t period,
                      const ELEDState startState);

        int8_t    ledPins_[ELEDID::LED_MAX_ID];
        uint32_t  ledLastEvent_[ELEDID::LED_MAX_ID];
        ELEDState ledStates_[ELEDID::LED_MAX_ID];

        CSystemState * systemState_;
};

#endif /* #ifndef __DRIVERS_IOLEDMGR_H_ */