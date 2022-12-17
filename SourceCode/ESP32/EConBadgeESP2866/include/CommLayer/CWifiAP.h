/*******************************************************************************
 * @file CWifiAP.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/13/2022
 *
 * @version 1.0
 *
 * @brief Wifi access point class.
 *
 * @details Wifi access point class. This class defines the implementation of
 * the Wifi access point feature. The services provided allow to create,
 * initialize, enable and disable a Wifi access point with the ESP32 module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMLAYER_CWIFIAP_H_
#define __COMMLAYER_CWIFIAP_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <ESP8266WiFi.h>      /* Wifi drivers */
#include <WiFiClient.h>       /* Wifi client */
#include <WiFiServer.h>       /* Wifi server */
#include <Types.h>            /* Common module's types */
#include <SystemState.h>      /* System State Service */
#include <CommandControler.h> /* Command controler service */

using namespace nsCommon;
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
 * @brief Communication Services Namespace
 * @details Communication Services Namespace. This namespace gathers the
 * declarations and definitions related to the communication services.
 */
namespace nsComm
{
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
 * CLASSES
 ******************************************************************************/

/**
 * @brief Wifi Access Point service class.
 *
 * @details Wifi Access Point service class. This class provides the services
 * to initialize, enable and disable a wifi access point on the ESP32 module.
 * The access point can be configured to have various IP ranges and provide
 * difference security levels.
 */
class CWifiAP
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Initializes the access point.
         *
         * @details Initializes the access point with the provided parameters.
         * The function will return the initialization status (sucess or error
         * code). On success, the access point is usable right after this
         * function returns.
         *
         * @param[in] pSSID The network's SSID for the access point.
         * @param[in] pPassword The network's password for the access point.
         *
         * @return EErrorCode On success, NO_ERROR is returned, otherwise a
         * standard error code is returned (see EErrorCode).
         */
        EErrorCode InitAP(const char * pSSID,
                          const char * pPassword);

        EErrorCode StartAP(void);

        EErrorCode StopAP(void);

        String GetIPAddr(void) const;

        EErrorCode StartServer(uint16_t port);

        EErrorCode StopServer(uint16_t port);

        EErrorCode WaitClient(void);

        EErrorCode WaitCommand(SSystemCommand * command);

        EErrorCode UpdateState(const nsCore::CSystemState & sysState,
                               const nsCore::CCommandControler & comControler);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        WiFiClient   client;
        WiFiServer * server = nullptr;

        uint16_t   serverPort;

        bool isInit = false;
};

} /* namespace nsComm */

#endif /* #ifndef __COMMLAYER_CWIFIAP_H_ */