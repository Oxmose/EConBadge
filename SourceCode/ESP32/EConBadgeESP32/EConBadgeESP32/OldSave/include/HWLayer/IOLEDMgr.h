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

#ifndef __HWLAYER_IOLEDMGR_H_
#define __HWLAYER_IOLEDMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */

#include <Types.h>            /* Defined Types */
#include <CommandControler.h> /* Command controller service */
#include <SystemState.h>      /* System State Service */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#
#define LED_MAIN_PIN     18
#define LED_AUX_PIN      19

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Hardware Layer Namespace
 * @details Hardware Layer Namespace used for definitions of hardware related
 * services.
 */
namespace nsHWL
{

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef void (*TInputBtnHandler)(void);

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
        CIOLEDMgr(void);

        nsCommon::EErrorCode SetupLED(const nsCommon::ELEDID LEDId, const uint8_t LEDPin);
        nsCommon::EErrorCode UpdateState(nsCore::CSystemState & sysState);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetState(const nsCommon::ELEDID LEDId, const nsCommon::ELEDState state);
        void BlinkLED(const nsCommon::ELEDID LEDId,
                      const uint32_t period,
                      const nsCommon::ELEDState startState);

        int8_t LEDPins[nsCommon::LED_MAX_ID];
        uint32_t LEDLastEvent[nsCommon::LED_MAX_ID];
        nsCommon::ELEDState LEDStates[nsCommon::LED_MAX_ID];
};

} /* namespace nsHWL */

#endif /* #ifndef __HWLAYER_IOLEDMGR_H_ */