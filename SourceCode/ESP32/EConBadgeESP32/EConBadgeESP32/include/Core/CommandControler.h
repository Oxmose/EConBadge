/*******************************************************************************
 * @file CommandControler.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file provides the command rontroler service.
 *
 * @details This file provides the command rontroler service. This files defines
 * the different features embedded in the command controller. It is used to
 * service IO input (buttons) and network (WIFI / Bluetooth) commands.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_COMMAND_CONTROLER_H_
#define __CORE_COMMAND_CONTROLER_H_

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
class CCommandControler;
typedef nsCommon::EErrorCode (*TCommandHandler)(const CCommandControler*, void*);

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

class CCommandControler
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CCommandControler(void);

        nsCommon::EErrorCode ParseCommand(const nsCommon::SSystemCommand & command) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        TCommandHandler handlers[nsCommon::COMM_MAX_ID];
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_COMMAND_CONTROLER_H_ */