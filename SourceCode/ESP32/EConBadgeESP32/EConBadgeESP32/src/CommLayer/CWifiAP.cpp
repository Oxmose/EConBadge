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

CWIFIAP::~CWifiAP(void)
{

}

EErrorCode CWIFIAP::InitAP(const char * pSSID,
                           const char * pPassword)
{
    EErrorCode retCode;

    LOG_DEBUG("Setting up AP | SSID: %s | Password: %s\n", pSSID, pPassword);

    this->password = String(pPassword);
    this->SSID     = String(pSSID);

    /* Set WIFI Mode */
    if(WiFi.mode(WIFI_AP))
    {
        LOG_DEBUG("WIFI set to AP mode.\n");
        if(WiFi.softAPConfig(IPAddress(192, 168, 0, 1),
                             IPAddress(192, 168, 0, 100),
                             IPAddress(255, 255, 255, 0)))
        {
            LOG_DEBUG("WIFI AP configuration done..\n");
            if(WiFi.softAP(pSSID, pPassword))
            {
                LOG_INFO("AP Initialized\n");

                this->isInit    = true;
                this->isEnabled = false;
                this->lastEvent = millis();
                retCode         = NO_ERROR;
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

EErrorCode CWIFIAP::StopAP(void)
{
    EErrorCode retCode;

    if(WiFi.softAPdisconnect(true))
    {
        retCode = NO_ERROR;
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

String CWIFIAP::GetSSID(void) const
{
    return this->SSID;
}

String CWIFIAP::GetPassword(void) const
{
    return this->password;
}

bool CWIFIAP::IsEnabled(void) const
{
    return this->isEnabled;
}

EErrorCode CWIFIAP::StartServer(uint16_t port)
{
    EErrorCode retCode;

    if(this->isInit)
    {
        this->server = new WiFiServer(port);
        this->server->begin();

        this->isEnabled = true;
        this->lastEvent = millis();

        retCode = NO_ERROR;
    }
    else
    {
        retCode = NOT_INITIALIZED;
    }

    return retCode;
}

EErrorCode CWIFIAP::StopServer()
{
    EErrorCode retCode;

    /* Start server on init */
    if(this->isInit)
    {
        this->server->stop();
        delay(10);
        this->server->close();
        delay(10);
        delete this->server;

        this->isEnabled = false;
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
            this->lastEvent = millis();
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

EErrorCode CWIFIAP::WaitCommand(uint32_t * command)
{
    size_t     readCount;
    EErrorCode retCode;

    *command = -1;

    /* Check the client's state */
    if(this->isInit && this->client && this->client.connected())
    {
        /* Read the command ID */
        readCount = this->client.readBytes((char*)command, sizeof(uint32_t));
        if(readCount > 0)
        {
            this->lastEvent = millis();
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

bool CWIFIAP::isIdle(void)
{
    bool idle = true;

    if(this->isInit && this->isEnabled)
    {
        /* Compute the elapsed time */
        idle = (millis() - this->lastEvent > IDLE_TIME);
    }

    return idle;
}

EErrorCode CWIFIAP::ReadBytes(uint32_t * readSize, void * buffer)
{
    EErrorCode retCode;

    /* Check the client's state */
    if(this->isInit && this->client && this->client.connected())
    {
        /* Read the command ID */
        *readSize = this->client.readBytes((char*)buffer, *readSize);
        if(*readSize > 0)
        {
            this->lastEvent = millis();
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

#undef CWIFIAP