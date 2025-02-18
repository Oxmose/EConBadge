/*******************************************************************************
 * @file BlueToothMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/11/2023
 *
 * @version 1.0
 *
 * @brief This file provides the BlueTooth service
 *
 * @details This file provides the BlueTooth service. This files defines
 * the different features embedded for the BT service.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_BLUETOOTH_MGR_H_
#define __CORE_BLUETOOTH_MGR_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <map>               /* std::map */
#include <vector>            /* std::vector */
#include <Storage.h>         /* Storage service */
#include <BLEUtils.h>        /* BLE Untils Services*/
#include <BLEDevice.h>       /* BLE Device Services*/
#include <BLEServer.h>       /* BLE Server Services*/
#include <BluetoothSerial.h> /* Bluetooth driver */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Defines the message length. */
#define BLE_MESSAGE_MTU 512

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Define a data buffer for BLE communication. */
typedef struct
{
    /** @brief Buffer reader lock. */
    SemaphoreHandle_t rlock;

    /** @brief Buffer writer lock. */
    SemaphoreHandle_t wlock;

    /** @brief Retry command */
    bool retry;

    /** @brief Buffer that stores the message. */
    uint8_t           pBuffer[BLE_MESSAGE_MTU];
    /** @brief Cursor in the buffer. */
    size_t            cursor;
    /** @brief Size of the data in the buffer. */
    size_t            messageSize;
} SBLEBuffer;

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

class CommandHandler
{
    public:
        virtual ~CommandHandler(void){}
        virtual EErrorCode EnqueueCommand(SCommandRequest command) = 0;
};

/**
 * @brief The bluetooth manager class.
 *
 * @details The bluetooth manager class provides the functionalities used to
 * create and use a bluetooth connection.
 */
class BluetoothManager
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        BluetoothManager(void);

        void Init(CommandHandler* pHandler);

        void AddCommandListener(CommandHandler* pHandler);

        bool SetToken(const std::string& rkNewToken);
        void GetToken(std::string& rToken);

        void SendCommandResponse(SCommandResponse& rResponse);

        ssize_t ReceiveData(uint8_t* pBuffer, size_t size);
        ssize_t SendData(const uint8_t* pBuffer, size_t size);

        void ExecuteCommand(uint8_t* pCommandData, const size_t kCommandLength);

        /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

        /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

        std::string        token_;
        Storage*           pStorage_;
        CommandHandler*    pHandler_;

        BLEServer*         pServer_;
        BLEService*        pMainService_;
        BLEAdvertising*    pAdvertising_;
        BLECharacteristic* pCommandCharacteristic_;
        BLECharacteristic* pDataCharacteristic_;
        BLECharacteristic* pCommandDeferCharacteristic_;

        SBLEBuffer         sendBuffer_;
        SBLEBuffer         receiveBuffer_;
};

#endif /* #ifndef __CORE_BLUETOOTH_MGR_H_ */