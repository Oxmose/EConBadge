/*******************************************************************************
 * @file BLEMgr.cpp
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logging services */
#include <SystemState.h>      /* System State Service */
#include <BLEDevice.h>        /* BLE Dev */
#include <BLEUtils.h>         /* BLE Utils */
#include <BLEServer.h>        /* BLE Server */

/* Header File */
#include <BLEMgr.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CBLEMGR nsComm::CBLEMgr


#define BLE_SERVICE_UUID       "21a8d618-14d6-4558-a15d-cb3740980fa6"
#define BLE_SND_COMM_CHAR_UUID "dd93d880-6de6-4223-9a02-385e0f0dc8c2"
#define BLE_SND_DATA_CHAR_UUID "8b3f17b2-a4ba-4184-8a95-52ca45679c8d"
#define BLE_RCV_COMM_CHAR_UUID "6ff51cbb-1614-433a-bb4a-c1cdae956062"
#define BLE_RCV_DATA_CHAR_UUID "7ede505c-e5d4-4f66-8008-556ae80b6d0f"

#define BLE_MAX_MSG_SIZE 512

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

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
 * CLASS METHODS
 ******************************************************************************/

CBLEMGR::~CBLEMgr(void)
{

}

EErrorCode CBLEMGR::InitServer(const char * devName)
{
    EErrorCode retCode;

    retCode = NO_ERROR;

    LOG_DEBUG("Setting up BLE\n");

    if(!this->isInit)
    {

        /* Init the BLE device and Server */
        BLEDevice::init(devName);
        this->devName = String(devName);

        this->pServer = BLEDevice::createServer();
        if(this->pServer != nullptr)
        {
            this->pService = this->pServer->createService(BLE_SERVICE_UUID);
            if(this->pService == nullptr)
            {
                retCode = NO_MEMORY;
            }
        }
        else
        {
            retCode = NO_MEMORY;
        }
    }
    else
    {
        retCode = NO_ACTION;
    }

    /* Create the send and receive characteristics */
    if(retCode == NO_ERROR)
    {
        this->pSndCommCharacteristic = pService->createCharacteristic(BLE_SND_COMM_CHAR_UUID,
                                                                     BLECharacteristic::PROPERTY_NOTIFY |
                                                                     BLECharacteristic::PROPERTY_READ |
                                                                     BLECharacteristic::PROPERTY_WRITE);
        if(this->pSndCommCharacteristic != nullptr)
        {
            LOG_INFO("Added BLE SND Comm characteristic\n");
        }
        else
        {
            retCode = NO_MEMORY;
        }
    }
    if(retCode == NO_ERROR)
    {
        this->pSndDataCharacteristic = pService->createCharacteristic(BLE_SND_DATA_CHAR_UUID,
                                                                     BLECharacteristic::PROPERTY_READ |
                                                                     BLECharacteristic::PROPERTY_WRITE);
        if(this->pSndDataCharacteristic != nullptr)
        {
            LOG_INFO("Added BLE SND Data characteristic\n");
        }
        else
        {
            retCode = NO_MEMORY;
        }
    }
    if(retCode == NO_ERROR)
    {
        this->pRcvCommCharacteristic = pService->createCharacteristic(BLE_RCV_COMM_CHAR_UUID,
                                                                      BLECharacteristic::PROPERTY_NOTIFY |
                                                                      BLECharacteristic::PROPERTY_READ |
                                                                      BLECharacteristic::PROPERTY_WRITE);
        if(this->pRcvCommCharacteristic != nullptr)
        {
            LOG_INFO("Added BLE RCV Comm characteristic\n");
        }
        else
        {
            retCode = NO_MEMORY;
        }
    }
    if(retCode == NO_ERROR)
    {
        this->pRcvDataCharacteristic = pService->createCharacteristic(BLE_RCV_DATA_CHAR_UUID,
                                                                      BLECharacteristic::PROPERTY_READ |
                                                                      BLECharacteristic::PROPERTY_WRITE);
        if(this->pRcvDataCharacteristic != nullptr)
        {
            LOG_INFO("Added BLE RCV Data characteristic\n");
        }
        else
        {
            retCode = NO_MEMORY;
        }
    }

    this->isEnabled = false;

    return retCode;
}

EErrorCode CBLEMGR::StartServer(void)
{
    EErrorCode retCode;

    retCode = NO_ERROR;

    pService->start();
    BLEAdvertising * pAdvertising = BLEDevice::getAdvertising();
    if(this->pAdvertising == nullptr)
    {
        pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        BLEDevice::startAdvertising();

        this->isEnabled = true;
    }
    else
    {
        retCode = NO_MEMORY;
    }

    return retCode;
}

EErrorCode CBLEMGR::StopServer(void)
{
    this->isEnabled = false;

    BLEDevice::stopAdvertising();
    pAdvertising->stop();
    pService->stop();

    return NO_ERROR;
}

String  CBLEMGR::GetBLEName(void) const
{
    return this->devName;
}

bool  CBLEMGR::IsEnabled(void) const
{
    return this->isEnabled;
}

EErrorCode CBLEMGR::WaitClient(void)
{
    EErrorCode retCode;

    return retCode;
}

EErrorCode CBLEMGR::WaitCommand(uint32_t * command)
{
    EErrorCode retCode;

    return retCode;
}

bool CBLEMGR::isIdle(void)
{
    bool idle = true;

    if(this->isInit && this->isEnabled)
    {
        /* Compute the elapsed time */
        idle = (millis() - this->lastEvent > WIFI_IDLE_TIME);
    }

    return idle;
}

EErrorCode CBLEMGR::ReadBytes(uint32_t * readSize, void * buffer)
{
    EErrorCode retCode;


    return retCode;
}

EErrorCode CBLEMGR::WriteBytes(uint32_t * writeSize, const void * buffer)
{
    EErrorCode retCode;

    if(writeSize > BLT_MAX_MSG_SIZE)

    return retCode;
}

#undef CBLEMGR