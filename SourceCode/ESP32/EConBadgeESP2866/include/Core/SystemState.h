/*******************************************************************************
 * @file SystemState.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the system state service.
 *
 * @details This file provides the system state service. This files defines
 * the different features embedded in the system state.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_SYSTEM_STATE_H_
#define __CORE_SYSTEM_STATE_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Standard Int Types */
#include <Types.h> /* Defined types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Core Namespace
 * @details Core Namespace used to gather the core services of the ESP32 module.
 */
namespace nsCore
{

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None*/

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

class CSystemState
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        nsCommon::ESystemState GetSystemState(void) const;

        void SetSystemState(const nsCommon::ESystemState state);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        nsCommon::ESystemState currState;
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */