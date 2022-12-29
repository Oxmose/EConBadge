/*******************************************************************************
 * @file BLEMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 29/12/2022
 *
 * @version 1.0
 *
 * @brief Bluetooth Low Energy manager class.
 *
 * @details Bluetooth Low Energy manager class. This class defines the
 * implementation of the BLE access point feature. The services provided allow
 * to create, initialize, enable and disable a BLE server on the ESP32
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMLAYER_BLEMGR_H_
#define __COMMLAYER_BLEMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <WiFi.h>             /* Wifi drivers */
#include <WiFiClient.h>       /* Wifi client */
#include <WiFiServer.h>       /* Wifi server */
#include <Types.h>            /* Common module's types */
#include <CommInterface.h>    /* Communication Interface */
#include <BLEDevice.h>        /* BLE Dev */
#include <BLEUtils.h>         /* BLE Utils */
#include <BLEServer.h>        /* BLE Server */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define BLE_IDLE_TIME    300000 /* NS : 5mins */

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

class CBLEMgr : public ICommInterface
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        virtual ~CBLEMgr(void);

        virtual nsCommon::EErrorCode ReadBytes(uint32_t * readSize, void * buffer);
        virtual nsCommon::EErrorCode WriteBytes(uint32_t * writeSize, const void * buffer);

        nsCommon::EErrorCode InitServer(const char * devName);
        nsCommon::EErrorCode StartServer(void);
        nsCommon::EErrorCode StopServer(void);

        String GetBLEName(void) const;
        bool IsEnabled(void) const;

        nsCommon::EErrorCode WaitClient(void);
        nsCommon::EErrorCode WaitCommand(uint32_t * command);

        bool isIdle(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

        bool isInit = false;
        bool isEnabled = false;

        uint32_t lastEvent;

        String devName;

        BLEServer         * pServer;
        BLEService        * pService;
        BLECharacteristic * pSndCommCharacteristic;
        BLECharacteristic * pSndDataCharacteristic;
        BLECharacteristic * pRcvCommCharacteristic;
        BLECharacteristic * pRcvDataCharacteristic;
        BLEAdvertising    * pAdvertising;
};

} /* namespace nsComm */

#endif /* #ifndef __COMMLAYER_BLEMGR_H_ */