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
#include <BluetoothSerial.h> /* Bluetooth driver */
#include <Logger.h>          /* System logger */
#include <HWLayer.h>         /* HW layer component*/
#include <Storage.h>         /* Storage service */

/* Header File */
#include <BlueToothMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CBTMGR BluetoothManager

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

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

CBTMGR::BluetoothManager(void)
{
    magicStep_          = 0;
    recomposingCommand_ = 0;
}

CBTMGR::~BluetoothManager(void)
{

}

void CBTMGR::Init(void)
{
    Storage * storage;

    /* Init the bluetooth serial interface */
    storage = Storage::GetInstance();
    storage->GetBluetoothName(name_);
    storage->GetBluetoothPin(pin_);

    if(btSerialIface_.begin(name_.c_str()))
    {
        if(!btSerialIface_.setPin(pin_.c_str()))
        {
            LOG_ERROR("Could not set Bluetooth PIN\n");
        }
    }
    else
    {
        LOG_ERROR("Could not start Bluetooth service\n");
    }
}

bool CBTMGR::ReceiveCommand(SCBCommand * command)
{
    uint8_t data;
    size_t  readSize;
    bool    commandReceived;

    commandReceived = false;

    /* Check if we should continue recomposing or not */
    if(recomposingCommand_ == false)
    {
        while(btSerialIface_.available())
        {
            data = btSerialIface_.read();
            if(data == (0xFF & (COMMAND_MAGIC >> (8 * (3 - magicStep_)))))
            {
                ++magicStep_;
            }
            else
            {
                magicStep_ = 0;
            }
            if(magicStep_ == sizeof(uint32_t))
            {
                recomposingCommand_ = true;
                magicStep_          = 0;
                commCursor_         = 0;

                LOG_DEBUG("Got BT Magic, recompossing command.\n");

                break;
            }
        }
    }

    /* If we successfully got the magic */
    if(recomposingCommand_ == true)
    {
        /* Try to recompose the command */
        while(btSerialIface_.available() && commCursor_ < sizeof(SCBCommand))
        {
            readSize = btSerialIface_.readBytes((uint8_t*)&comm_ + commCursor_,
                                                sizeof(SCBCommand) - commCursor_);
            commCursor_ += readSize;
        }
    }

    btSerialIface_.flush();

    /* Check if we have a complete command */
    if(commCursor_ == sizeof(SCBCommand))
    {
        recomposingCommand_ = false;
        commandReceived     = true;
        commCursor_         = 0;

        memcpy(command, &comm_, sizeof(SCBCommand));

        LOG_DEBUG("Recomposed BT command.\n");
    }

    return commandReceived;
}

void CBTMGR::ReceiveData(uint8_t * buffer, size_t& size)
{
    size_t totalSize;

    totalSize = 0;

    while(btSerialIface_.available() && totalSize < size)
    {
        totalSize += btSerialIface_.readBytes(buffer + totalSize,
                                              size - totalSize);
    }
    btSerialIface_.flush();

    size = totalSize;
}

void CBTMGR::TransmitData(const uint8_t * buffer, size_t& size)
{
    size = btSerialIface_.write(buffer, size);
    btSerialIface_.flush();
}

bool CBTMGR::UpdateName(const char * name)
{
    char actual[22];

    strncpy(actual, name, 21);
    actual[21] = 0;

    btSerialIface_.end();

    if(btSerialIface_.begin(actual))
    {
        if(btSerialIface_.setPin(pin_.c_str()))
        {
            name_ = std::string(actual);
            if(!Storage::GetInstance()->SetBluetoothName(name_))
            {
                LOG_ERROR("Could not store new Bluetooth name\n");
                return false;
            }
        }
        else
        {
            LOG_ERROR("Could not set Bluetooth PIN after name change\n");
            return false;
        }
    }
    else
    {
        LOG_ERROR("Could not start Bluetooth service after name change\n");
        return false;
    }

    LOG_DEBUG("GOING TRUE\n");

    return true;
}

bool CBTMGR::UpdatePin(const char * pin)
{
    char actual[5];

    strncpy(actual, pin, 4);
    actual[4] = 0;

    if(btSerialIface_.setPin(actual))
    {
        pin_ = std::string(actual);
        if(!Storage::GetInstance()->SetBluetoothPin(pin_))
        {
            return false;
        }
    }
    else
    {
        LOG_ERROR("Could not change Bluetooth PIN\n");
        return false;
    }
    return true;
}

#undef CBTMGR