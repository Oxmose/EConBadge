/*******************************************************************************
 * @file Updater.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 12/11/2023
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
#include <cstdint>        /* Generic types */
#include <Update.h>       /* ESP32 Update manager */
#include <Logger.h>       /* System logger */
#include <version.h>      /* Versionning information */
#include <HWLayer.h>      /* HW Layer service */
#include <Storage.h>      /* Storage service */
#include <SystemState.h>  /* System state service */
#include <BlueToothMgr.h> /* Bluetooth manager */

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

CUPD::Updater(BluetoothManager * pBtMgr, SystemState * pSysState)
{
    pBtMgr_    = pBtMgr;
    pSysState_ = pSysState;

    timeout_ = 0;
    state_   = EUpdateState::IDLE;
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
    updateAction = pSysState_->ConsumeUpdateAction();
    if(updateAction != EUpdaterAction::START_UPDATE_ACTION)
    {
        return;
    }

    LOG_DEBUG("Update started\n");

    /* Send back the software version */
    pSysState_->EnqueueResponse((uint8_t*)VERSION_SHORT, strlen(VERSION_SHORT));

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
    updateAction = pSysState_->ConsumeUpdateAction();
    if(updateAction == EUpdaterAction::CANCEL_ACTION)
    {
        LOG_DEBUG("Updated canceled by server\n");

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
    pSysState_->EnqueueResponse((uint8_t*)"READY", 5);

    /* Update timeout */
    state_   = EUpdateState::APPLYING_UPDATE;
    timeout_ = HWManager::GetTime() + REQUEST_TMEOUT;
}

void CUPD::ApplyUpdate(void)
{
    EUpdaterAction   updateAction;
    uint8_t        * pBuffer;
    uint32_t         toRead;
    uint32_t         packetSize;
    size_t           read;

    if(GetTimeoutLeft() == 0)
    {
        state_ = EUpdateState::IDLE;
        return;
    }

    /* Start update */
    updateAction = pSysState_->ConsumeUpdateAction();
    if(updateAction != EUpdaterAction::START_TRANSFER_ACTION)
    {
        return;
    }

    LOG_DEBUG("Update tranfer started\n");

    pBuffer = new uint8_t[UPDATE_BUFFER_SIZE];

    if(!pSysState_->SendResponseNow((uint8_t*)"READY_TRANS", 11))
    {
        LOG_ERROR("Could not send Updater READY_TRANS command\n");
        timeout_ = 0;
        state_   = EUpdateState::IDLE;
        delete[] pBuffer;
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

            pBtMgr_->ReceiveData(pBuffer + packetSize, read);
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
                delete[] pBuffer;
                return;
            }

            packetSize += read;
        }

        /* Check and update the to read */
        toRead = *(uint32_t*)(pBuffer + 4);
        if(*(uint32_t*)pBuffer != UPDATE_PACKET_MARKER_END &&
           toRead != UPDATE_BUFFER_SIZE)
        {
            LOG_ERROR("Incorrect to read size during update %x, %x\n",
                      *(uint32_t*)pBuffer, toRead);
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] pBuffer;
            return;
        }

        LOG_DEBUG("Receiving %d bytes\n", toRead - 8);

        while(packetSize < toRead)
        {
            read = toRead - packetSize;

            pBtMgr_->ReceiveData(pBuffer + packetSize, read);
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
                delete[] pBuffer;
                return;
            }
        }

        LOG_DEBUG("Updating %d bytes\n", packetSize - 8);

        /* Send the update (skip header) */
        if(update_.write(pBuffer + 8, packetSize - 8) != packetSize - 8)
        {
            LOG_ERROR("Could not write update packet\n");
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] pBuffer;
            return;
        }

        if(!pSysState_->SendResponseNow((uint8_t*)"OK", 2))
        {
            LOG_ERROR("Could not write update packet\n");
            update_.abort();
            timeout_ = 0;
            state_   = EUpdateState::IDLE;
            delete[] pBuffer;
            return;
        }
    } while (*(uint32_t*)pBuffer != UPDATE_PACKET_MARKER_END);

    /* End update, send acknowledge */
    update_.end(true);

    timeout_ = 0;
    state_   = EUpdateState::IDLE;
    delete[] pBuffer;

    LOG_DEBUG("Update successful, restarting\n");

    if(!pSysState_->SendResponseNow((uint8_t*)"UPDATE_SUCCESS", 14))
    {
        LOG_ERROR("Could not send UPDATE_SUCCESS command\n");
    }

    /* Restart and ensurethe message was transmitted */
    delay(100);
    ESP.restart();
}

#undef CUPD