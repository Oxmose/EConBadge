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
#include <Logger.h>          /* System logger */
#include <HWLayer.h>         /* HW layer component*/
#include <Storage.h>         /* Storage service */
#include <BluetoothSerial.h> /* Bluetooth driver */

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
    recomposingCommand_ = false;
    pStorage_           = Storage::GetInstance();
}

void CBTMGR::Init(void)
{
    /* Init the bluetooth serial interface */
    pStorage_->GetBluetoothName(name_);
    pStorage_->GetBluetoothPin(pin_);

    if(btIface_.begin(name_.c_str()))
    {
        if(btIface_.setPin(pin_.c_str()) == false)
        {
            LOG_ERROR("Failed to set BT PIN\n");
        }
    }
    else
    {
        LOG_ERROR("Failed to start BT service\n");
    }

    LOG_DEBUG("Initialized BT Manager\n");
}

bool CBTMGR::ReceiveCommand(SCBCommand * pCommand)
{
    uint8_t data;
    size_t  readSize;
    bool    commandReceived;

    commandReceived = false;

    /* Check if we should continue recomposing or not */
    if(recomposingCommand_ == false)
    {
        while(btIface_.available() == true)
        {
            data = btIface_.read();
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
                comCursor_          = 0;

                LOG_DEBUG("BT Magic, recompossing\n");

                break;
            }
        }
    }

    /* If we successfully got the magic */
    if(recomposingCommand_ == true)
    {
        /* Try to recompose the command */
        while(btIface_.available() == true && comCursor_ < sizeof(SCBCommand))
        {
            readSize = btIface_.readBytes((uint8_t*)&comm_ + comCursor_,
                                          sizeof(SCBCommand) - comCursor_);
            comCursor_ += readSize;
        }
    }

    btIface_.flush();

    /* Check if we have a complete command */
    if(comCursor_ == sizeof(SCBCommand))
    {
        recomposingCommand_ = false;
        commandReceived     = true;
        comCursor_          = 0;

        memcpy(pCommand, &comm_, sizeof(SCBCommand));
        LOG_DEBUG("Recomposed BT command\n");
    }

    return commandReceived;
}

void CBTMGR::ReceiveData(uint8_t * pBuffer, size_t & rSize)
{
    size_t totalSize;

    totalSize = 0;

    while(btIface_.available() == true && totalSize < rSize)
    {
        totalSize += btIface_.readBytes(pBuffer + totalSize, rSize - totalSize);
    }
    btIface_.flush();

    rSize = totalSize;
}

void CBTMGR::TransmitData(const uint8_t * pkBuffer, size_t & rSize)
{
    if(rSize == 0)
    {
        return;
    }
    rSize = btIface_.write(pkBuffer, rSize);

    btIface_.flush();
}

bool CBTMGR::UpdateSettings(const uint8_t * pkBuffer)
{
    char pActualName[22];
    char pActualPin[5];

    memcpy(pActualName, pkBuffer, 21);
    pActualName[21] = 0;

    if(strlen(pActualName) == 0)
    {
        return false;
    }

    memcpy(pActualPin, pkBuffer + 21, 4);
    pActualPin[4] = 0;

    btIface_.end();

    if(btIface_.begin(pActualName) == true)
    {
        name_ = std::string(pActualName);
        if(btIface_.setPin(pActualPin) == true)
        {
            pin_ = std::string(pActualPin);
            LOG_DEBUG("BT Name: %s | PIN: %s\n",pActualName, pActualPin);

            if(pStorage_->SetBluetoothName(name_) == false)
            {
                LOG_ERROR("Failed to store BT Name\n");
                return false;
            }
            if(pStorage_->SetBluetoothPin(pin_) == false)
            {
                LOG_ERROR("Failed to store BT PIN\n");;
                return false;
            }
        }
        else
        {
            LOG_ERROR("Failed to set BT PIN\n");
            return false;
        }
    }
    else
    {
        LOG_ERROR("Failed to set BT name\n");
        return false;
    }

    LOG_DEBUG("Udpated BT Settings\n");

    return true;
}


#undef CBTMGR