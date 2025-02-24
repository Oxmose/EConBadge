/*******************************************************************************
 * @file BlueToothMgr.cpp
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>      /* std::string */
#include <Types.h>     /* Custom defined types */
#include <HWMgr.h>     /* HW layer component*/
#include <Logger.h>    /* System logger */
#include <version.h>   /* ECB versioning */
#include <Storage.h>   /* Storage service */
#include <BLE2902.h>   /* BLE 2902 descriptor */
#include <BLEUtils.h>  /* BLE Untils Services*/
#include <BLEDevice.h> /* BLE Device Services*/
#include <BLEServer.h> /* BLE Server Services*/

/* Header File */
#include <BlueToothMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Defines the default bluetooth token */
#define DEFAULT_BT_TOKEN "0000000000000000"

/** @brief Defines the token path in the flash. */
#define BLUETOOTH_TOKEN_FILE_PATH "/bttoken"

/** @brief Main BLE service UUID */
#define MAIN_SERVICE_UUID "d3e63261-0000-1000-8000-00805f9b34fb"

/** @brief Hardware version characteristic UUID */
#define HW_VERSION_CHARACTERISTIC_UUID    "997ca8f9-0000-1000-8000-00805f9b34fb"
/** @brief Software version characteristic UUID */
#define SW_VERSION_CHARACTERISTIC_UUID    "20a14f57-0000-1000-8000-00805f9b34fb"
/** @brief Command manager characteristic UUID */
#define COMMAND_CHARACTERISTIC_UUID       "2d3a8ac3-0000-1000-8000-00805f9b34fb"
/** @brief Defered command manager characteristic UUID */
#define COMMAND_DEFER_CHARACTERISTIC_UUID "45fd43da-0000-1000-8000-00805f9b34fb"
/** @brief Data transfer characteristic UUID */
#define DATA_CHARACTERISTIC_UUID          "83670c18-0000-1000-8000-00805f9b34fb"

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief BLE server callback */
class ServerCallback: public BLEServerCallbacks
{
    /******************** PUBLIC METHODS AND ATTRIBUTES ***********************/
    public:
        /**
         * @brief Disconnect callback.
         *
         * @details Disconnect callback for the BLE server. Called when the
         * current pair has disconnected.
         *
         * @param[in, out] pServer The BLE server that got its pair disconnected.
         */
        void onDisconnect(BLEServer* pServer)
        {
            /* On disconnect, start advertising */
            pServer->startAdvertising();
        }

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /* None */
};

/**
 * @brief Command request characteristic callback class.
 */
class CommandRequestCallback: public BLECharacteristicCallbacks
{
    /******************** PUBLIC METHODS AND ATTRIBUTES ***********************/
    public:
        /**
         * @brief Construct a new Command Request Callback object.
         *
         * @param[in, out] pBTManager The bluetooth manager used with this
         * callback.
         */
        CommandRequestCallback(BluetoothManager* pBTManager)
        {
            pBTManager_ = pBTManager;
        }

        /**
         * @brief Callback when a write is detected on the characteristic.
         *
         * @param[in, out] pCharacteristic The characteristic that was written.
         */
        void onWrite(BLECharacteristic* pCharacteristic)
        {
            /* Execute the command that was received. */
            pBTManager_->ExecuteCommand(
                pCharacteristic->getData(),
                pCharacteristic->getLength()
            );
        }

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the bluetooth manager associated to this callback. */
        BluetoothManager* pBTManager_;
};

/**
 * @brief Data tranfer characteristic callback class.
 */
class DataTransferRequestCallback: public BLECharacteristicCallbacks
{
    /******************** PUBLIC METHODS AND ATTRIBUTES ***********************/
    public:
        /**
         * @brief Construct a new Data Transfer Request Callback object.
         *
         * @param[in, out] pSendBuffer The send buffer used for the transfer.
         * @param[in, out] pReceiveBuffer The receive buffer used for the
         * transfer.
         */
        DataTransferRequestCallback(SBLEBuffer* pSendBuffer,
                                    SBLEBuffer* pReceiveBuffer)
        {
            pSendBuffer_ = pSendBuffer;
            pReceiveBuffer_ = pReceiveBuffer;
        }

        /**
         * @brief Callback when a write is detected on the characteristic.
         *
         * @param[in, out] pCharacteristic The characteristic that was written.
         */
        void onWrite(BLECharacteristic* pCharacteristic)
        {
            size_t msgLength;

            msgLength = pCharacteristic->getLength();
            if(msgLength > BLE_MESSAGE_MTU)
            {
                LOG_ERROR("Received too long message, discarding.\n");
                return;
            }

            /* Wait for the receive buffer to be empty */
            xSemaphoreTake(pReceiveBuffer_->wlock, portMAX_DELAY);

            pReceiveBuffer_->cursor = 0;
            pReceiveBuffer_->messageSize = msgLength;
            memcpy(
                pReceiveBuffer_->pBuffer,
                pCharacteristic->getData(),
                msgLength
            );

            /* Release the reader */
            xSemaphoreGive(pReceiveBuffer_->rlock);
        }

        /**
         * @brief Callback when a status is updated on the characteristic.
         *
         * @param[in, out] pCharacteristic The characteristic that was updated.
         * @param[in] status The status that was updated.
         * @param[in] code And additional code for the status.
         */
        void onStatus(BLECharacteristic* pCharacteristic,
                      Status             status,
                      uint32_t           code)
        {
            LOG_DEBUG("On status %d\n", status);

            /* Check if the action succeded or is a retry shall be done. */
            if(status == BLECharacteristicCallbacks::SUCCESS_NOTIFY)
            {
                pSendBuffer_->retry = false;
            }
            else
            {
                pSendBuffer_->retry = true;
            }
            xSemaphoreGive(pSendBuffer_->wlock);
        }

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the send buffer. */
        SBLEBuffer* pSendBuffer_;
        /** @brief Stores the receive buffer. */
        SBLEBuffer* pReceiveBuffer_;
};


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
 * CLASS METHODS
 ******************************************************************************/

BluetoothManager::BluetoothManager(void)
{
    pHandler_ = nullptr;
    pStorage_ = Storage::GetInstance();

    /* Prepare the buffers */

    sendBuffer_.rlock = xSemaphoreCreateBinary();
    sendBuffer_.wlock = xSemaphoreCreateBinary();
    sendBuffer_.cursor = 0;
    sendBuffer_.retry = false;
    sendBuffer_.messageSize = 0;
    /* Since the receiving buffer is empty, give the semaphore */
    xSemaphoreGive(sendBuffer_.wlock);

    receiveBuffer_.rlock = xSemaphoreCreateBinary();
    receiveBuffer_.wlock = xSemaphoreCreateBinary();
    receiveBuffer_.cursor = 0;
    receiveBuffer_.retry = false;
    receiveBuffer_.messageSize = 0;
    /* Since the receiving buffer is empty, give the semaphore */
    xSemaphoreGive(receiveBuffer_.wlock);
}

void BluetoothManager::Init(CommandHandler* pHandler)
{
    BLECharacteristic* pNewCharacteristic;

    /* Setup the command handler */
    pHandler_ = pHandler;

    /* Get the bluetooth token */
    pStorage_->GetContent(
        BLUETOOTH_TOKEN_FILE_PATH,
        DEFAULT_BT_TOKEN,
        token_,
        true
    );

    /* Initialize the server and services */
    BLEDevice::init(HWManager::GetHWUID());
    pServer_ = BLEDevice::createServer();
    pMainService_ = pServer_->createService(BLEUUID(MAIN_SERVICE_UUID), 15U, 0);

    /* Setup server callback */
    pServer_->setCallbacks(new ServerCallback());

    /* Setup the VERSION characteristics */
    pNewCharacteristic = pMainService_->createCharacteristic(
        HW_VERSION_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pNewCharacteristic->setValue(PROTO_REV);
    pNewCharacteristic = pMainService_->createCharacteristic(
        SW_VERSION_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    pNewCharacteristic->setValue(VERSION);

    /* Setup the COMMAND characteristics */
    pCommandCharacteristic_ = pMainService_->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCommandCharacteristic_->setCallbacks(new CommandRequestCallback(this));
    pCommandCharacteristic_->addDescriptor(new BLE2902());

    /* Setup the DATA TRANSFER characteristics */
    pDataCharacteristic_ = pMainService_->createCharacteristic(
        DATA_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pDataCharacteristic_->setCallbacks(
        new DataTransferRequestCallback(
            &this->sendBuffer_,
            &this->receiveBuffer_
        )
    );
    pDataCharacteristic_->addDescriptor(new BLE2902());

    /* Start the services */
    pMainService_->start();

    /* Start advertising */
    pAdvertising_ = BLEDevice::getAdvertising();

    pAdvertising_->addServiceUUID(MAIN_SERVICE_UUID);
    pAdvertising_->setScanResponse(true);
    pAdvertising_->setMinPreferred(0x06);
    pAdvertising_->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
}

bool BluetoothManager::SetToken(const std::string& pkNewToken)
{
    bool success;

    if(pkNewToken.size() == 0)
    {
        return false;
    }

    success = pStorage_->SetContent(
        BLUETOOTH_TOKEN_FILE_PATH,
        pkNewToken,
        true
    );
    if(!success)
    {
        LOG_ERROR("Failed to save the bluetooth token\n");
        return false;
    }
    token_ = pkNewToken;

    LOG_DEBUG("New token: %s\n", token_.c_str());

    return true;
}

void BluetoothManager::GetToken(std::string& rToken)
{
    rToken = token_;
}

void BluetoothManager::ExecuteCommand(uint8_t* pCommandData, const size_t kCommandLength)
{
    SCommandResponse response;
    SCommandHeader*  pHeader;
    EErrorCode       errorCode;

    /* Check the command sanity */
    if(kCommandLength < sizeof(SCommandHeader))
    {
        response.header.errorCode = INVALID_COMMAND_REQ;
        SendCommandResponse(response);
        return;
    }

    pHeader = (SCommandHeader*)pCommandData;

    /* Check token */
    if(strncmp((char*)pHeader->pToken, token_.c_str(), COMM_TOKEN_SIZE) != 0)
    {
        response.header.errorCode = INVALID_TOKEN;
        response.header.identifier = pHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
        return;
    }

    /* Check size */
    if(pHeader->size > COMMAND_DATA_SIZE ||
       kCommandLength != sizeof(SCommandHeader) + pHeader->size)
    {
        response.header.errorCode = INVALID_COMMAND_SIZE;
        response.header.identifier = pHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
        return;
    }

    errorCode = pHandler_->EnqueueCommand(*(SCommandRequest*)pCommandData);
    if(errorCode != NO_ERROR)
    {
        response.header.errorCode = errorCode;
        response.header.identifier = pHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
    }
}

void BluetoothManager::SendCommandResponse(SCommandResponse& rResponse)
{
    /* Check if response must be truncated */
    if(rResponse.header.size > COMMAND_RESPONSE_LENGTH)
    {
        rResponse.header.size = COMMAND_RESPONSE_LENGTH;
    }

    pCommandCharacteristic_->setValue(
        (uint8_t*)&rResponse,
        rResponse.header.size + sizeof(SCommandHeader)
    );
    pCommandCharacteristic_->notify(true);
}

ssize_t BluetoothManager::ReceiveData(uint8_t* pBuffer, size_t size)
{
    ssize_t toRead;
    ssize_t readBytes;

    readBytes = 0;
    while(size > 0)
    {
        /* Wait for the receive buffer to be populated */
        xSemaphoreTake(receiveBuffer_.rlock, portMAX_DELAY);

        /* Read what we can */
        toRead = MIN(size, receiveBuffer_.messageSize - receiveBuffer_.cursor);
        memcpy(
            pBuffer + readBytes,
            receiveBuffer_.pBuffer + receiveBuffer_.cursor,
            toRead
        );

        receiveBuffer_.cursor += toRead;

        if(receiveBuffer_.cursor == receiveBuffer_.messageSize)
        {
            /* When the whole message is read, release the writer */
            xSemaphoreGive(receiveBuffer_.wlock);
        }
        else
        {
            /* When there is still left in the buffer, release for a future
             * read
             */
            xSemaphoreGive(receiveBuffer_.rlock);
        }

        readBytes += toRead;
        size -= toRead;
    }

    return readBytes;
}
ssize_t BluetoothManager::SendData(const uint8_t* pBuffer, size_t size)
{
    ssize_t toWrite;
    ssize_t wroteBytes;
    bool    first;

    LOG_DEBUG("Sending Dat Start\n");

    wroteBytes = 0;
    toWrite = 0;

    first = true;
    while(size > 0)
    {
        if(first)
        {
            /* Wait for the send buffer to be empty and add rate limiting */
            xSemaphoreTake(sendBuffer_.wlock, portMAX_DELAY);

            first = false;
        }

        /* Manage buffer */
        toWrite = MIN(size, BLE_MESSAGE_MTU);
        memcpy(sendBuffer_.pBuffer, pBuffer + wroteBytes, toWrite);
        sendBuffer_.cursor = 0;
        sendBuffer_.messageSize = toWrite;

        /* Send the message */
        pDataCharacteristic_->setValue(sendBuffer_.pBuffer, toWrite);
        pDataCharacteristic_->notify(true);

        /* Wait for the send buffer to be empty and add rate limiting */
        xSemaphoreTake(sendBuffer_.wlock, portMAX_DELAY);

        /* Check for retry */
        if(!sendBuffer_.retry)
        {
            size -= toWrite;
            wroteBytes += toWrite;
        }
    }

    /* Release the last semaphore take */
    xSemaphoreGive(sendBuffer_.wlock);

    LOG_DEBUG("Sending Data End\n");

    return wroteBytes;
}