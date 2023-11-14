/*******************************************************************************
 * @file Updater.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 12/11/2022
 *
 * @version 1.0
 *
 * @brief This file defines updater service.
 *
 * @details This file defines updater service. The updater allows to update the
 * EConBadge over the air (OTA).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>           /* Generic types */
#include <Logger.h>          /* System logger */
#include <Storage.h>         /* Storage service */
#include <BlueToothMgr.h>    /* Bluetooth manager */
#include <SystemState.h> /* System state service */
#include <HWLayer.h>     /* HW Layer service */
#include <version.h>     /* Versionning information */
#include <Update.h>      /* ESP32 Update manager */

/* Header File */
#include <Updater.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CUPD Updater

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define REQUEST_TMEOUT 30000 /* 30 seconds */

#define UPDATE_BUFFER_SIZE 8192

#define UPDATE_PACKET_MARKER_END 0xE0E0A0A0

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

CUPD::Updater(BluetoothManager * btMgr, SystemState * sysState)
{
    btMgr_    = btMgr;
    sysState_ = sysState;

    timeout_ = 0;
    state_   = EUpdateState::IDLE;
}

CUPD::~Updater(void)
{

}

uint64_t CUPD::GetTimeoutLeft(void) const
{
    uint64_t timeNow;

    timeNow = HWManager::GetTime();
    if(timeout_ > timeNow)
    {
        return timeout_ - timeNow;
    }
    else
    {
        return 0;
    }
}

EUpdateState CUPD::GetStatus(void) const
{
    return state_;
}

void CUPD::RequestUpdate(void)
{
    state_   = EUpdateState::WAITING_START;
    timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
}

void CUPD::Update(void)
{
    if(state_ == EUpdateState::WAITING_START)
    {
        WaitUpdateStart();
    }
    else if(state_ == EUpdateState::WAITING_VALID)
    {
        WaitUpdateValidation();
    }
    else if(state_ == EUpdateState::APPLYING_UPDATE)
    {
        ApplyUpdate();
    }
}

void CUPD::WaitUpdateStart(void)
{
    EUpdaterAction updateAction;

    /* Check timeout */
    if(GetTimeoutLeft() == 0)
    {
        state_ = EUpdateState::IDLE;
        return;
    }

    /* Wait for request */
    updateAction = sysState_->ConsumeUpdateAction();
    if(updateAction != EUpdaterAction::START_UPDATE_ACTION)
    {
        return;
    }

    LOG_DEBUG("Update started\n");

    /* Send back the software version */
    if(!sysState_->EnqueueResponse((uint8_t*)VERSION_SHORT,
                                    strlen(VERSION_SHORT)))
    {
        LOG_ERROR("Could not send Updater version\n");
        timeout_ = 0;
        state_   = EUpdateState::IDLE;
        return;
    }

    /* Update timeout */
    state_   = EUpdateState::WAITING_VALID;
    timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
}

void CUPD::WaitUpdateValidation(void)
{
    EUpdaterAction updateAction;

    /* Check timeout */
    if(GetTimeoutLeft() == 0)
    {
        state_ = EUpdateState::IDLE;
        return;
    }

    /* Wait for validation or not */
    updateAction = sysState_->ConsumeUpdateAction();
    if(updateAction == EUpdaterAction::CANCEL_ACTION)
    {
        LOG_DEBUG("Updated is canceled by server\n");
        if(!sysState_->EnqueueResponse((uint8_t*)"OK", 2))
        {
            LOG_ERROR("Could not send Updater OK command\n");
        }

        state_   = EUpdateState::IDLE;
        timeout_ = 0;
        return;
    }
    else if(updateAction != EUpdaterAction::VALIDATION_ACTION)
    {
        return;
    }

    LOG_DEBUG("Update validated\n");

    /* If valid, send ready, wait for update request */
    if(!sysState_->EnqueueResponse((uint8_t*)"READY", 5))
    {
        LOG_ERROR("Could not send Updater READY command\n");
        timeout_ = 0;
        state_   = EUpdateState::IDLE;
        return;
    }

    /* Update timeout */
    state_   = EUpdateState::APPLYING_UPDATE;
    timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
}

void CUPD::ApplyUpdate(void)
{
    EUpdaterAction updateAction;
    uint8_t *      buffer;
    uint32_t       toRead;
    uint32_t       packetSize;
    size_t         read;

    if(GetTimeoutLeft() == 0)
    {
        state_ = EUpdateState::IDLE;
        return;
    }

    /* Start update */
    updateAction = sysState_->ConsumeUpdateAction();
    if(updateAction != EUpdaterAction::START_TRANSFER_ACTION)
    {
        return;
    }

    LOG_DEBUG("Update tranfer started.\n");

    buffer = new uint8_t[UPDATE_BUFFER_SIZE];
    if(buffer == nullptr)
    {
        LOG_ERROR("Could not allocate the update buffer\n");
        timeout_ = 0;
        state_   = EUpdateState::IDLE;
        return;
    }

    if(!sysState_->SendResponseNow((uint8_t*)"READY_TRANS", 11))
    {
        LOG_ERROR("Could not send Updater READY_TRANS command\n");
        timeout_ = 0;
        state_   = EUpdateState::IDLE;
        delete[] buffer;
        return;
    }

    update_.begin();

    /* Get the data */
    timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
    do
    {
        toRead     = 8;
        packetSize = 0;

        /* Read the header (marker and size) */
        while(packetSize < toRead)
        {
            read = UPDATE_BUFFER_SIZE;

            btMgr_->ReceiveData(buffer + packetSize, read);
            if(read > 0)
            {
                timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
            }
            else if(GetTimeoutLeft() == 0)
            {
                LOG_ERROR("Timeout during update\n");
                update_.abort();
                timeout_ = 0;
                state_   = EUpdateState::IDLE;
                delete[] buffer;
                return;
            }

            packetSize += read;
        }

        LOG_DEBUG("First read received %d bytes\n", packetSize);

        /* Check and update the to read */
        toRead = *(uint32_t*)(buffer + 4);
        if(*(uint32_t*)buffer != UPDATE_PACKET_MARKER_END &&
           toRead != UPDATE_BUFFER_SIZE)
        {
            LOG_ERROR("Incorrect to read size during update %x, %x\n",
                      *(uint32_t*)buffer, toRead);
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] buffer;
            return;
        }

        LOG_DEBUG("Receiving %d bytes\n", toRead - 8);

        while(packetSize < toRead)
        {
            read = toRead - packetSize;

            btMgr_->ReceiveData(buffer + packetSize, read);
            if(read > 0)
            {
                timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
                packetSize += read;
            }
            else if(GetTimeoutLeft() == 0)
            {
                LOG_ERROR("Timeout during update\n");
                update_.abort();
                timeout_ = 0;
                state_   = EUpdateState::IDLE;
                delete[] buffer;
                return;
            }
        }

        LOG_DEBUG("Updating %d bytes\n", packetSize - 8);

        /* Send the update (skip header) */
        if(update_.write(buffer + 8, packetSize - 8) != packetSize - 8)
        {
            LOG_ERROR("Could not write update packet\n");
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] buffer;
            return;
        }

        LOG_DEBUG("Sending response\n");

        if(!sysState_->SendResponseNow((uint8_t*)"OK", 2))
        {
            LOG_ERROR("Could not write update packet\n");
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] buffer;
            return;
        }
    } while (*(uint32_t*)buffer != UPDATE_PACKET_MARKER_END);

    /* End update, send acknowledge */
    update_.end(true);

    timeout_ = 0;
    state_   = EUpdateState::IDLE;
    delete[] buffer;

    LOG_DEBUG("Update successful, restarting\n");

    if(!sysState_->SendResponseNow((uint8_t*)"UPDATE_SUCCESS", 14))
    {
        LOG_ERROR("Could not send Updater UPDATE_SUCCESS command\n");
    }

    /* Restart and ensurethe message was transmitted */
    delay(100);
    ESP.restart();
}

#undef CUPD