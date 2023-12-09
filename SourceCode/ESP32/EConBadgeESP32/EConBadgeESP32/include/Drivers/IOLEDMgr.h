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

#include <string>        /* std::string */
#include <cstdint>       /* Generic Types */
#include <Types.h>       /* Defined Types */
#include <SystemState.h> /* System State Service */


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
    LED_MAX_ID
} ELEDID;

typedef enum
{
    MAIN_PIN = 33,
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

class IOLEDMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        IOLEDMgr (SystemState * pSystemState);

        EErrorCode SetupLED (const ELEDID kLedID, const ELEDPin kLedPin);

        void Update (void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetState (const ELEDID kLedID, const ELEDState kState);
        void BlinkLED (const ELEDID    kLedID,
                       const uint32_t  kPeriod,
                       const ELEDState kStartState);

        int8_t    pLedPins_[ELEDID::LED_MAX_ID];
        uint64_t  pLedLastEvent_[ELEDID::LED_MAX_ID];
        ELEDState pLedStates_[ELEDID::LED_MAX_ID];

        SystemState * pSystemState_;
};

#endif /* #ifndef __DRIVERS_IOLEDMGR_H_ */