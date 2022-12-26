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
#include <WiFi.h>             /* Wifi drivers */
#include <WiFiClient.h>       /* Wifi client */
#include <WiFiServer.h>       /* Wifi server */
#include <Types.h>            /* Common module's types */
#include <CommInterface.h>    /* Communication Interface */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SERVER_COMM_PORT 5000
#define IDLE_TIME        120000 /* NS : 2mins */

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
class CWifiAP : public ICommInterface
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        virtual ~CWifiAP(void);

        virtual nsCommon::EErrorCode ReadBytes(uint32_t * readSize, void * buffer);

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
        nsCommon::EErrorCode InitAP(const char * pSSID,
                                    const char * pPassword);
        nsCommon::EErrorCode StopAP(void);

        String GetIPAddr(void) const;
        String GetSSID(void) const;
        String GetPassword(void) const;
        bool IsEnabled(void) const;

        nsCommon::EErrorCode StartServer(uint16_t port);
        nsCommon::EErrorCode StopServer(void);

        nsCommon::EErrorCode WaitClient(void);
        nsCommon::EErrorCode WaitCommand(uint32_t * command);

        bool isIdle(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        WiFiClient   client;
        WiFiServer * server = nullptr;

        String password;
        String SSID;

        uint16_t   serverPort;

        bool isInit = false;
        bool isEnabled = false;

        uint32_t lastEvent;
};

} /* namespace nsComm */

#endif /* #ifndef __COMMLAYER_CWIFIAP_H_ */