/*******************************************************************************
 * @file CWifiAP.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/13/2022
 *
 * @version 1.0
 *
 * @brief Wifi access point class.
 *
 * @details Wifi access point class. This class defines the implementation of
 * the Wifi access point feature. The services provided allow to create,
 * initialize, enable and disable a Wifi access point with the ESP32 module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logging services */
#include <WiFi.h>             /* Wifi drivers */
#include <SystemState.h>      /* System State Service */
#include <CommandControler.h> /* Command controler service */

/* Header File */
#include <CWIfiAP.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CWIFIAP nsComm::CWifiAP

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

EErrorCode CWIFIAP::InitAP(const char * pSSID,
                           const char * pPassword)
{
    EErrorCode retCode;

    LOG_DEBUG("Setting up AP | SSID: %s | Password: %s\n", pSSID, pPassword);

    /* Set WIFI Mo de */
    if(WiFi.mode(WIFI_AP))
    {
        LOG_DEBUG("WIFI set to AP mode.\n");
        if(WiFi.softAPConfig(IPAddress(192, 168, 0, 1),
                             IPAddress(192, 168, 0, 100),
                             IPAddress(255, 255, 255, 0)))
        {
            LOG_DEBUG("WIFI AP configuration done.\n");
            if(WiFi.softAP(pSSID, pPassword))
            {
                LOG_INFO("AP Initialized\n");

                this->isInit = true;
                retCode      = NO_ERROR;
            }
            else
            {
                LOG_ERROR("Could not start AP\n");
                retCode = ACTION_FAILED;
            }
        }
        else
        {
            LOG_ERROR("Could not setup AP\n");
            retCode = ACTION_FAILED;
        }
    }
    else
    {
        retCode = ACTION_FAILED;
    }

    return retCode;
}

String CWIFIAP::GetIPAddr(void) const
{
    IPAddress ipAddr = WiFi.softAPIP();

    String strIp = String(ipAddr[0]) + String(".") +
                   String(ipAddr[1]) + String(".") +
                   String(ipAddr[2]) + String(".") +
                   String(ipAddr[3]);

    return strIp;
}

EErrorCode CWIFIAP::StartServer(uint16_t port)
{
    EErrorCode retCode;

    if(this->isInit)
    {
        this->server = new WiFiServer(port);
        this->server->begin();

        retCode = NO_ERROR;
    }
    else
    {
        retCode = NOT_INITIALIZED;
    }

    return retCode;
}

EErrorCode CWIFIAP::StopServer(uint16_t port)
{
    EErrorCode retCode;

    /* Start server on init */
    if(this->isInit)
    {
        this->server->stop();
        delete this->server;
        retCode = NO_ERROR;
    }
    else
    {
        retCode = NOT_INITIALIZED;
    }

    return retCode;
}

EErrorCode CWIFIAP::WaitClient(void)
{
    EErrorCode retCode;

    if(this->isInit && this->server != nullptr)
    {
        /* Wait for client */
        this->client = this->server->available();
        if(this->client && client.connected())
        {
            retCode = NO_ERROR;
        }
        else
        {
            retCode = NO_ACTION;
        }
    }
    else
    {
        retCode = NOT_INITIALIZED;
    }
    return retCode;
}

EErrorCode CWIFIAP::WaitCommand(SSystemCommand * command)
{
    size_t     readCount;
    EErrorCode retCode;

    command->command = 0;

    /* Check the client's state */
    if(this->isInit && this->client && this->client.connected())
    {
        /* Read the command ID */
        readCount = this->client.readBytes((char*)&command->command,
                                            sizeof(ESystemCommandId));
        if(readCount > 0)
        {
            /* TODO: Based on the command ID read the args */
            retCode = NO_ERROR;
        }
        else
        {
            retCode = NO_ACTION;
        }
    }
    else
    {
        retCode = NO_CONNECTION;
    }

    return retCode;
}

EErrorCode CWIFIAP::UpdateState(nsCore::CSystemState & sysState,
                                const nsCore::CCommandControler & comControler)
{
    EErrorCode     retCode;
    SSystemCommand command;

    retCode = WaitClient();
    if(retCode == NO_ERROR)
    {
        retCode = WaitCommand(&command);
        if(retCode == NO_ERROR)
        {
            LOG_DEBUG("Received WIFI command: %d\n", command.command);
            comControler.ParseCommand(command);
        }
        else if(retCode != NO_ACTION)
        {
            LOG_ERROR("Error while getting WIFI command: %d\n", retCode);
        }
        else
        {
            /* No action is not an error for higher level */
            retCode = NO_ERROR;
        }
    }
    else if(retCode != NO_ACTION)
    {
        LOG_ERROR("Error while waiting for client: %d\n", retCode);
    }
    else
    {
        /* No action is not an error for higher level */
        retCode = NO_ERROR;
    }

    return retCode;
}

#undef CWIFIAP