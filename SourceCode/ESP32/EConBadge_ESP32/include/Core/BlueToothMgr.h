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
#include <string>         /* std::string */
#include <Types.h>        /* Custom defined types */
#include <Storage.h>      /* Storage service */
#include <NimBLEDevice.h> /* BLE Services */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Defines the message length. */
#define BLE_MESSAGE_MTU 503

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

/**
 * @brief Command handler interface.
 *
 * @details The command handler interface is used to plug a command handler
 * used when receiving a command through the BLE interface.
 */
class CommandHandler
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Destroy the Command Handler object
         */
        virtual ~CommandHandler(void){}

        /**
         * @brief Enqueues a command.
         *
         * @details Enqueues a command received form the BLE layer.
         *
         * @param[in] rCommand The command to enqueue.
         *
         * @return Returns an error code on failure.
         */
        virtual EErrorCode EnqueueCommand(SCommandRequest& rCommand) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /* None */
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
        /**
         * @brief Construct a new Bluetooth Manager object.
         *
         * @details The BLE internal data will be initialized.
         */
        BluetoothManager(void);

        /**
         * @brief Initialize the BLE manager with a command handler.
         *
         * @details Initialize the BLE manager with a command handler. This
         * function starts the BLE services and finish the initialization
         * started in the constructor.
         *
         * @param[in] pHandler The command handler.
         */
        void Init(CommandHandler* pHandler);

        /**
         * @brief Set the BLE communication token.
         *
         * @param[in] rkNewToken The new token to set.
         *
         * @return true is returned on success, false otherwise.
         */
        bool SetToken(const std::string& rkNewToken);

        /**
         * @brief Get the BLE communication token.
         *
         * @param[out] rToken The string buffer that receives the BLE
         * communication token.
         */
        void GetToken(std::string& rToken);

        /**
         * @brief Sends a response to a command.
         *
         * @details This function send a response to a command requested by the
         * client. The response might be altered by the underling BLE services
         * but is sent as defined prior to this call.
         *
         * @param[in,out] rResponse The response to send.
         */
        void SendCommandResponse(SCommandResponse& rResponse);

        /**
         * @brief Receives data from the BLE interface.
         *
         * @details Receives data from the BLE interface. Raw data will be read
         * and stored in the provided buffer.
         *
         * @param[out] pBuffer The buffer that receives the received data.
         * @param[in] size The size of the data to receive.
         * @param[in] kTimeout The timeout in microseconds.
         *
         * @return The size of the actually received data is returned. -1 is
         * returned on error.
         */
        ssize_t ReceiveData(uint8_t* pBuffer,
                            size_t   size,
                            const    uint64_t kTimeout);

        /**
         * @brief Sends data to the BLE interface.
         *
         * @details Sends data to the BLE interface. Raw data will be send
         * from the provided buffer.
         *
         * @param[in] pBuffer The buffer that contains the data to send.
         * @param[in] size The size of the data to send.
         * @param[in] kTimeout The timeout in microseconds.
         *
         * @return The size of the actually sent data is returned. -1 is
         * returned on error.
         */
        ssize_t SendData(const uint8_t* pBuffer,
                         size_t         size,
                         const uint64_t kTimeout);

        /**
         * @brief Executes a command received by the BLE callbacks.
         *
         * @details Executes a command received by the BLE callbacks. Only a BLE
         * callback should call this function.
         *
         * @param[in] kpCommandData The raw data command that was received.
         * @param[in] kCommandLength The size of the command in bytes.
         */
        void ExecuteCommand(const uint8_t* kpCommandData,
                            const size_t   kCommandLength);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the BLE communication token */
        std::string        token_;
        /** @brief Stores the storage singleton. */
        Storage*           pStorage_;
        /** @brief Stores the command handler. */
        CommandHandler*    pHandler_;

        /** @brief Stores the BLE server instance. */
        BLEServer*         pServer_;
        /** @brief Stores the BLE main service instance. */
        BLEService*        pMainService_;
        /** @brief Stores the BLE advertising instance. */
        BLEAdvertising*    pAdvertising_;
        /** @brief Stores the BLE command characteristic instance. */
        BLECharacteristic* pCommandCharacteristic_;
        /** @brief Stores the BLE data characteristic instance. */
        BLECharacteristic* pDataCharacteristic_;

        /** @brief Command notification lock. */
        SemaphoreHandle_t  commandNotifyLock_;

        /** @brief Stores the current Nimble connection */
        NimBLEConnInfo*    pBleConnetion_;

        /** @brief Stores the send buffer used for raw data tranfers */
        SBLEBuffer         sendBuffer_;
        /** @brief Stores the receive buffer used for raw data tranfers */
        SBLEBuffer         receiveBuffer_;
};

#endif /* #ifndef __CORE_BLUETOOTH_MGR_H_ */