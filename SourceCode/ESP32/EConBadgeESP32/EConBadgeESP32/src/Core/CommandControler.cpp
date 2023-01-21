/*******************************************************************************
 * @file CommandControler.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the command rontroler service.
 *
 * @details This file prvides the command rontroler service. This files defines
 * the different features embedded in the command controller. It is used to
 * service IO input (buttons) and network (WIFI / Bluetooth) commands.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>        /* String manipulation*/
#include <Types.h>        /* Defined Types */
#include <Logger.h>       /* Logger Service */
#include <SystemState.h>  /* System State Service */
#include <HWLayer.h>      /* Hardware Layer Service */
#include <EEPROM.h>       /* EEPROM Memory driver */

/* Header File */
#include <CommandControler.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CCTRL nsCore::CCommandControler

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

EErrorCode CCTRL::ExecuteCommand(const uint32_t command,
                                 nsComm::ICommInterface * comm,
                                 nsCore::CSystemState * sysState) const
{
    EErrorCode retCode;

    /* Check command bound */
    retCode = NO_ERROR;
    if(command < COMM_MAX_ID)
    {
        LOG_DEBUG("Executing Command %d\n", command);

        switch(command)
        {
            case COMM_PING_ID:
                CommPing(comm);
                break;
            case COMM_UPDATE_WIFI_PASS:
                CommUpdateWifiPass(comm);
                break;
            case COMM_CLEAR_EINK:
                CommClearEInk(sysState);
                break;
            case COMM_UPDATE_EINK:
                CommUpdateEInk(sysState, comm);
                break;
            case COMM_CLEAR_BORDER:
                CommClearBorder(sysState);
                break;
            case COMM_UPDATE_BORDER:
                CommUpdateBorder(sysState, comm);
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        retCode = NO_ACTION;
    }

    return retCode;
}

void CCTRL::CommPing(nsComm::ICommInterface * comm) const
{
    uint32_t writeSize;
    char * buffer = "PONG";
    LOG_INFO("PONG\n");
    writeSize = 4;
    comm->WriteBytes(&writeSize, buffer);
}

void CCTRL::CommClearEInk(nsCore::CSystemState * sysState) const
{
    sysState->GetEInkDriver()->Init(true);
    sysState->GetEInkDriver()->Clear(EPD_5IN65F_WHITE);
    sysState->GetEInkDriver()->Sleep();
}

void CCTRL::CommUpdateWifiPass(nsComm::ICommInterface * comm) const
{
    EErrorCode retCode;
    uint32_t   readSize;
    uint32_t   writeSize;

    char buffer[EEPROM_SIZE_WIFI_PASS + 1];
    char currPass[EEPROM_SIZE_WIFI_PASS + 1];

    /* Get the new SSID */
    readSize = EEPROM_SIZE_WIFI_PASS;
    retCode = comm->ReadBytes(&readSize, buffer);
    if(retCode == NO_ERROR)
    {
        /* Validate and update if needed */
        if(readSize >= 8 && readSize <= EEPROM_SIZE_WIFI_PASS)
        {
            if(readSize < EEPROM_SIZE_WIFI_PASS)
            {
                buffer[readSize] = 0;
            }

            EEPROM.readBytes(EEPROM_ADDR_WIFI_PASS, currPass, EEPROM_SIZE_WIFI_PASS);
            if(strncmp(buffer, currPass, EEPROM_SIZE_WIFI_PASS) != 0)
            {
                /* Write to EEPROM */
                EEPROM.writeBytes(EEPROM_ADDR_WIFI_PASS, buffer, EEPROM_SIZE_WIFI_PASS);
                EEPROM.commit();
                LOG_INFO("Updated Password in EEPROM\n");
            }
            else
            {
                LOG_INFO("WiFi Password is the same\n");
            }
        }
        else
        {
            LOG_ERROR("Invalid WiFi password\n");
            retCode = INVALID_PARAM;
        }
    }
    else
    {
        LOG_ERROR("Cannot read WiFi password from network\n");
        retCode = ACTION_FAILED;
    }

    /* Acknowledge */
    writeSize = sizeof(EErrorCode);
    retCode = comm->WriteBytes(&writeSize, &retCode);
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not send acknowledge to communication device: %d (wrote %d)\n",
                  retCode, writeSize);
    }
}

void CCTRL::CommUpdateEInk(nsCore::CSystemState * sysState,
                           nsComm::ICommInterface * comm) const
{
    EErrorCode retCode;
    uint32_t   readSize;
    uint32_t   toRead;
    uint32_t   writeSize;

    char     * imageData;
    uint32_t   leftToTransfer = EINK_IMAGE_SIZE;

    imageData = new char[4096];
    if(imageData != nullptr)
    {
        sysState->GetEInkDriver()->Init(true);
        sysState->GetEInkDriver()->EPD_5IN65F_DisplayInitTrans();

        /* Get the full image data */
        retCode = NO_ERROR;
        while(leftToTransfer > 0 && retCode == NO_ERROR)
        {
            if(leftToTransfer < 4096)
            {
                toRead = leftToTransfer;
            }
            else
            {
                toRead = 4096;
            }
            readSize = toRead;
            retCode = comm->ReadBytes(&readSize, imageData);
            if(retCode == NO_ERROR && readSize == toRead)
            {
                LOG_INFO("Updating EINK Image. Left: %d\n", leftToTransfer);
                sysState->GetEInkDriver()->EPD_5IN65F_DisplayPerformTrans(imageData, toRead);

                leftToTransfer -= toRead;
            }
            else
            {
                LOG_ERROR("Could not retreive new image: %d (read %d)\n", retCode, readSize);
                retCode = ACTION_FAILED;
            }
        }

        sysState->GetEInkDriver()->EPD_5IN65F_DisplayEndTrans();
        sysState->GetEInkDriver()->Sleep();
        delete imageData;
    }
    else
    {
        LOG_ERROR("Could not allocate memory buffer\n");
        retCode = NO_MEMORY;
    }

    /* Acknowledge */
    writeSize = sizeof(EErrorCode);
    retCode = comm->WriteBytes(&writeSize, &retCode);
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not send acknowledge to communication device: %d (wrote %d)\n",
                  retCode, writeSize);
    }
}

void CCTRL::CommClearBorder(CSystemState * sysState) const
{
    nsHWL::CLEDBorder * ledBorderDriver;

    ledBorderDriver = sysState->GetLEDBorderDriver();

    ledBorderDriver->ClearAnimations();
    ledBorderDriver->ClearPattern();
    ledBorderDriver->Disable();
}
void CCTRL::CommUpdateBorder(CSystemState * sysState,
                             nsComm::ICommInterface * comm) const
{
    nsHWL::CLEDBorder * ledBorderDriver;

    uint8_t  command;
    uint32_t          readSize;
    uint8_t           updateId;
    uint8_t           animCount;
    nsHWL::SLEDBorderAnimationParam    animParam;
    nsHWL::SLEDBorderColorPatternParam colorParam;

    ledBorderDriver = sysState->GetLEDBorderDriver();

    /* Get the number update type */
    readSize = sizeof(uint8_t);
    comm->ReadBytes(&readSize, &command);
    if(readSize != sizeof(uint8_t))
    {
        LOG_ERROR("Could not correctly read LED Bodrer update command\n");
        return;
    }

    switch(command)
    {
        case SET_COLOR_PATTERN:
            LOG_DEBUG("Updating CLED pattern\n");

            /* Read the update ID */
            readSize = sizeof(uint8_t);
            comm->ReadBytes(&readSize, &updateId);
            if(readSize != sizeof(uint8_t))
            {
                LOG_ERROR("Could not correctly read LED Bodrer update command\n");
                return;
            }

            LOG_DEBUG("Updated ID: %d\n", updateId);

            /* Read the parameters */
            readSize = sizeof(nsHWL::SLEDBorderColorPatternParam);
            comm->ReadBytes(&readSize, &colorParam);
            if(readSize != sizeof(nsHWL::SLEDBorderColorPatternParam))
            {
                LOG_ERROR("Could not correctly read LED Bodrer param command %d instead or %d\n", readSize, sizeof(nsHWL::SLEDBorderColorPatternParam));
                return;
            }

            ledBorderDriver->SetPattern((nsHWL::ELEDBorderColorPattern)updateId, colorParam);
            ledBorderDriver->Enable();
            break;
        case SET_ANIMATIONS:
            ledBorderDriver->ClearAnimations();

            /* Read the number of animations */
            readSize = sizeof(uint8_t);
            comm->ReadBytes(&readSize, &animCount);
            if(readSize != sizeof(uint8_t))
            {
                LOG_ERROR("Could not correctly read LED Bodrer count command\n");
                return;
            }

            while(animCount-- != 0)
            {
                /* Read the update ID */
                readSize = sizeof(uint8_t);
                comm->ReadBytes(&readSize, &updateId);
                if(readSize != sizeof(uint8_t))
                {
                    LOG_ERROR("Could not correctly read LED Bodrer param command\n");
                    return;
                }

                /* Read the parameters */
                readSize = sizeof(uint8_t);
                comm->ReadBytes(&readSize, &animParam);
                if(readSize != sizeof(uint8_t))
                {
                    LOG_ERROR("Could not correctly read LED Bodrer param command\n");
                    return;
                }
                ledBorderDriver->AddAnimation((nsHWL::ELEDBorderAnimation)updateId, animParam);
                ledBorderDriver->Enable();
            }
            break;
        default:
            break;
    }


}

#undef CCTRL