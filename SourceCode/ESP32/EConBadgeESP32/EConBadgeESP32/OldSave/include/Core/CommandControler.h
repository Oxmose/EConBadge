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
#include <CommInterface.h> /* Communication interface */

/* Forward declarations */
namespace nsCore
{
    class CSystemState;
}

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

typedef enum
{
    COMM_PING_ID          = 0,
    COMM_UPDATE_WIFI_PASS = 1,
    COMM_CLEAR_EINK       = 2,
    COMM_UPDATE_EINK      = 3,
    COMM_CLEAR_BORDER     = 4,
    COMM_UPDATE_BORDER    = 5,
    COMM_MAX_ID
} ESystemCommandId;

typedef enum
{
    SET_COLOR_PATTERN = 0,
    SET_ANIMATIONS    = 1,
    LED_COMMAND_MAX_ID
} ELEDBorderCommId;

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
        nsCommon::EErrorCode ExecuteCommand(const uint32_t command,
                                            nsComm::ICommInterface * comm,
                                            CSystemState * sysState) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void CommPing(nsComm::ICommInterface * comm) const;
        void CommUpdateWifiPass(nsComm::ICommInterface * comm) const;
        void CommClearEInk(CSystemState * sysState) const;
        void CommUpdateEInk(CSystemState * sysState,
                            nsComm::ICommInterface * comm) const;
        void CommClearBorder(CSystemState * sysState) const;
        void CommUpdateBorder(CSystemState * sysState,
                              nsComm::ICommInterface * comm) const;
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_COMMAND_CONTROLER_H_ */