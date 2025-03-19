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
#include <string>         /* std::string */
#include <Types.h>        /* Custom defined types */
#include <HWMgr.h>        /* HW layer component*/
#include <Logger.h>       /* System logger */
#include <version.h>      /* ECB versioning */
#include <Storage.h>      /* Storage service */
#include <NimBLEDevice.h> /* BLE Services */

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
/** @brief Data transfer characteristic UUID */
#define DATA_CHARACTERISTIC_UUID          "83670c18-0000-1000-8000-00805f9b34fb"

/** @brief Defines the data send end nimble size. */
#define DATA_END_NIMBLE_SIZE 16

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
        explicit ServerCallback(NimBLEConnInfo** pBleConnection)
        {
            pBleConnection_ = pBleConnection;
            *pBleConnection = nullptr;
        }

        /**
         * @brief Disconnect callback.
         *
         * @details Disconnect callback for the BLE server. Called when the
         * current pair has disconnected.
         *
         * @param[in, out] pServer The BLE server that got its pair disconnected.
         */
        void onDisconnect(NimBLEServer* pServer,
                          NimBLEConnInfo& connInfo,
                          int reason)
        {
            *pBleConnection_ = nullptr;
            /* On disconnect, start advertising */
            pServer->startAdvertising();
        }

        void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo)
        {
            /**
             *  We can use the connection handle here to ask for different connection parameters.
             *  Args: connection handle, min connection interval, max connection interval
             *  latency, supervision timeout.
             *  Units; Min/Max Intervals: 1.25 millisecond increments.
             *  Latency: number of intervals allowed to skip.
             *  Timeout: 10 millisecond increments.
             */
            pServer->updateConnParams(connInfo.getConnHandle(), 6, 8, 0, 180);
            *pBleConnection_ = &connInfo;
        }

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        NimBLEConnInfo** pBleConnection_;
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
        explicit CommandRequestCallback(BluetoothManager*  pBTManager,
                                        SemaphoreHandle_t* pLock)
        {
            pBTManager_ = pBTManager;
            pLock_      = pLock;
        }

        /**
         * @brief Callback when a write is detected on the characteristic.
         *
         * @param[in, out] pCharacteristic The characteristic that was written.
         */
        virtual void onWrite(BLECharacteristic* pCharacteristic,
                             NimBLEConnInfo&    rConnInfo)
        {
            /* Execute the command that was received. */
            pBTManager_->ExecuteCommand(
                pCharacteristic->getValue().data(),
                pCharacteristic->getLength()
            );
        }

        virtual void onStatus(BLECharacteristic* pCharacteristic, int code)
        {
            LOG_DEBUG("On status Comm %d\n", code);

            xSemaphoreGive(*pLock_);
        }

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the bluetooth manager associated to this callback. */
        BluetoothManager* pBTManager_;
        /** @brief Stores the notification lock. */
        SemaphoreHandle_t* pLock_;
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
        explicit DataTransferRequestCallback(SBLEBuffer* pSendBuffer,
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
        virtual void onWrite(BLECharacteristic* pCharacteristic,
                             NimBLEConnInfo&    rConnInfo)
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
                pCharacteristic->getValue().data(),
                msgLength
            );

            /* Release the reader */
            xSemaphoreGive(pReceiveBuffer_->rlock);
        }

        /**
         * @brief Callback when a status is updated on the characteristic.
         *
         * @param[in, out] pCharacteristic The characteristic that was updated.
         * @param[in] code And additional code for the status.
         */
        virtual void onStatus(BLECharacteristic* pCharacteristic, int code)
        {
            LOG_DEBUG("On status %d\n", code);

            /* Check if the action succeded or is a retry shall be done. */
            if(code == 0)
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

/** @brief Defines the end of data sent nimble. */
static const uint8_t skpDataEndNimble[DATA_END_NIMBLE_SIZE] = {
    0xFE, 0xDE, 0xAD, 0xC0, 0xDE, 0xEC, 0xBB, 0xAD,
    0x0E, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAA, 0xBB
};

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

    /* Create and initialize the notification lock */
    commandNotifyLock_ = xSemaphoreCreateBinary();
    xSemaphoreGive(commandNotifyLock_);
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
    if(!BLEDevice::init(HWManager::GetHWUID()))
    {
        LOG_ERROR("Failed to initialize BLE device\n");
        return;
    }

    pServer_ = BLEDevice::createServer();
    pMainService_ = pServer_->createService(MAIN_SERVICE_UUID);

    /* Setup server callback */
    pServer_->setCallbacks(new ServerCallback(&pBleConnetion_));

    /* Setup the VERSION characteristics */
    pNewCharacteristic = pMainService_->createCharacteristic(
        HW_VERSION_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ
    );
    pNewCharacteristic->setValue(PROTO_REV);
    pNewCharacteristic = pMainService_->createCharacteristic(
        SW_VERSION_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ
    );
    pNewCharacteristic->setValue(VERSION);

    /* Setup the COMMAND characteristics */
    pCommandCharacteristic_ = pMainService_->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE  |
        NIMBLE_PROPERTY::READ   |
        NIMBLE_PROPERTY::NOTIFY
    );
    pCommandCharacteristic_->setCallbacks(
        new CommandRequestCallback(this, &this->commandNotifyLock_)
    );

    /* Setup the DATA TRANSFER characteristics */
    pDataCharacteristic_ = pMainService_->createCharacteristic(
        DATA_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE  |
        NIMBLE_PROPERTY::READ   |
        NIMBLE_PROPERTY::NOTIFY
    );
    pDataCharacteristic_->setCallbacks(
        new DataTransferRequestCallback(
            &this->sendBuffer_,
            &this->receiveBuffer_
        )
    );

    /* Start the services */
    pMainService_->start();

    /* Start advertising */
    pAdvertising_ = BLEDevice::getAdvertising();

    pAdvertising_->setName(HWManager::GetHWUID());
    pAdvertising_->addServiceUUID(pMainService_->getUUID());
    pAdvertising_->enableScanResponse(true);
    pAdvertising_->start();
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

void BluetoothManager::ExecuteCommand(const uint8_t* kpCommandData,
                                      const size_t   kCommandLength)
{
    SCommandResponse      response;
    EErrorCode            errorCode;
    const SCommandHeader* kpHeader;
    SCommandRequest       request;

    LOG_DEBUG("Command with size %d\n", kCommandLength);

    /* Check the command sanity */
    if(kCommandLength < sizeof(SCommandHeader))
    {
        response.header.errorCode = INVALID_COMMAND_REQ;
        SendCommandResponse(response);
        return;
    }

    kpHeader = (const SCommandHeader*)kpCommandData;

    /* Check token */
    if(strncmp((char*)kpHeader->pToken, token_.c_str(), COMM_TOKEN_SIZE) != 0)
    {
        response.header.errorCode = INVALID_TOKEN;
        response.header.identifier = kpHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
        return;
    }

    /* Check size */
    if(kpHeader->size > COMMAND_DATA_SIZE ||
       kCommandLength != sizeof(SCommandHeader) + kpHeader->size)
    {
        response.header.errorCode = INVALID_COMMAND_SIZE;
        response.header.identifier = kpHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
        return;
    }

    LOG_DEBUG("Command ID %d\n", kpHeader->type);

    memset(&request, 0, sizeof(SCommandRequest));
    memcpy(&request, kpCommandData, kCommandLength);

    errorCode = pHandler_->EnqueueCommand(request);
    if(errorCode != NO_ERROR)
    {
        response.header.errorCode = errorCode;
        response.header.identifier = kpHeader->identifier;
        response.header.size = 0;
        SendCommandResponse(response);
    }
}

void BluetoothManager::SendCommandResponse(SCommandResponse& rResponse)
{
    bool    sendSuccess;
    uint8_t retry;

    if(pBleConnetion_ == nullptr)
    {
        return;
    }

    /* Check if response must be truncated */
    if(rResponse.header.size > COMMAND_RESPONSE_LENGTH)
    {
        rResponse.header.size = COMMAND_RESPONSE_LENGTH;
    }

    xSemaphoreTake(commandNotifyLock_, portMAX_DELAY);

    LOG_DEBUG("SENDING RESPONSE of size %d\n", rResponse.header.size + sizeof(SCommandHeader));

    retry = 0;
    do
    {
        sendSuccess = pCommandCharacteristic_->notify(
            (uint8_t*)&rResponse,
            rResponse.header.size + sizeof(SCommandHeader),
            BLE_HS_CONN_HANDLE_NONE
        );
        if(!sendSuccess)
        {
            LOG_DEBUG("Retry send %d\n", retry);
            HWManager::DelayExecUs(50000);
            ++retry;
        }

    } while(!sendSuccess && retry < 10);
}

ssize_t BluetoothManager::ReceiveData(uint8_t*       pBuffer,
                                      size_t         size,
                                      const uint64_t kTimeout)
{
    ssize_t toRead;
    ssize_t readBytes;

    if(pBleConnetion_ == nullptr)
    {
        return -1;
    }

    readBytes = 0;
    while(size > 0)
    {
        /* Wait for the receive buffer to be populated */
        if(xSemaphoreTake(receiveBuffer_.rlock,
                          kTimeout / portTICK_PERIOD_MS) != pdTRUE)
        {
            LOG_DEBUG("TIMEOUT\n");
            return -1;
        }

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
ssize_t BluetoothManager::SendData(const uint8_t* pBuffer,
                                   size_t         size,
                                   const uint64_t kTimeout)
{
    ssize_t toWrite;
    ssize_t wroteBytes;
    bool    first;
    bool    sendSuccess;
    uint8_t retry;

    if(pBleConnetion_ == nullptr)
    {
        return -1;
    }

    wroteBytes = 0;
    first      = true;
    while(size > 0)
    {
        if(first)
        {
            /* Wait for the send buffer to be empty and add rate limiting */
            if(xSemaphoreTake(sendBuffer_.wlock,
                              kTimeout / portTICK_PERIOD_MS) != pdTRUE)
            {
                return -1;
            }

            first = false;
        }

        /* Manage buffer */
        toWrite = MIN(size, BLE_MESSAGE_MTU);
        memcpy(sendBuffer_.pBuffer, pBuffer + wroteBytes, toWrite);
        sendBuffer_.cursor = 0;
        sendBuffer_.messageSize = toWrite;

        /* Send the message */
        retry = 0;
        do
        {
            sendSuccess = pDataCharacteristic_->notify(
                sendBuffer_.pBuffer,
                toWrite,
                BLE_HS_CONN_HANDLE_NONE
            );
            if(!sendSuccess)
            {
                LOG_DEBUG("Retry send %d\n", retry);
                HWManager::DelayExecUs(50000);
                ++retry;
            }

        } while(!sendSuccess && retry < 10);
        if(!sendSuccess)
        {
            return -1;
        }

        /* Wait for the send buffer to be empty and add rate limiting */
        if(xSemaphoreTake(sendBuffer_.wlock,
                          kTimeout / portTICK_PERIOD_MS) != pdTRUE)
        {
            return -1;
        }

        /* Check for retry */
        if(!sendBuffer_.retry)
        {
            size -= toWrite;
            wroteBytes += toWrite;
        }
    }

    /* Release the last semaphore take */
    xSemaphoreGive(sendBuffer_.wlock);

    return wroteBytes;
}

void BluetoothManager::SendDataEnd(void)
{
    bool    sendSuccess;
    uint8_t retry;

    if(pBleConnetion_ == nullptr)
    {
        return;
    }

    /* Wait for the send buffer to be empty and add rate limiting */
    if(xSemaphoreTake(sendBuffer_.wlock, 10000 / portTICK_PERIOD_MS) != pdTRUE)
    {
        return;
    }

    /* Send the message */
    retry = 0;
    do
    {
        sendSuccess = pDataCharacteristic_->notify(
            skpDataEndNimble,
            sizeof(skpDataEndNimble),
            BLE_HS_CONN_HANDLE_NONE
        );
        if(!sendSuccess)
        {
            LOG_DEBUG("Retry send %d\n", retry);
            HWManager::DelayExecUs(50000);
            ++retry;
        }

    } while(!sendSuccess && retry < 10);

    /* Release the last semaphore take */
    xSemaphoreGive(sendBuffer_.wlock);
}