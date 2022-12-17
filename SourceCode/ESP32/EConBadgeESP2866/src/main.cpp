/*******************************************************************************
 * @file main.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief ESP32 Main entry point file.
 *
 * @details ESP32 Main entry point file. This file contains the main functions
 * of the ESP32 software module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <Arduino.h> /* Arduino Main Header File */

#include <Types.h>            /* Defined Types */
#include <CWifiAp.h>          /* Access Point Service */
#include <HWLayer.h>          /* Hardware abstracction layer */
#include <Logger.h>           /* Logging Services */
#include <SystemState.h>      /* System state service */
#include <CommandControler.h> /* Command Controller service */

using namespace nsCommon;
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define HW_ID_LENGTH 32

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
namespace nsCommon
{
    /* TODO: Read from flash config */
    uint8_t loggingLevel = 0;

    /* TODO: Read from flash config */
    uint16_t serverPort = 5000;
}

/************************** Static global variables ***************************/
static nsCore::CSystemState      systemState;
static nsCore::CCommandControler commandController;
static nsComm::CWifiAP           wifiAP;
static nsHWL::CHWManager         hwManager;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Setup function of the ESP32 software module.
 *
 * @details Setup function of the ESP32 software module. This function is used
 * to initialize the module's state and hardware. It is only called once on
 * board's or CPU reset.
 */
void setup(void);

/**
 * @brief Main execution loop of the ESP32 software module.
 *
 * @details Main execution loop of the ESP32 software module. This function
 * never returns and performs the main loop of the ESP32 software module.
 */
void loop(void);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void setup(void)
{
    EErrorCode      retCode;
    char            uniqueHWUID[HW_ID_LENGTH];

    /* Get the unique hardware ID */
    hwManager.GetHWUID(uniqueHWUID, HW_ID_LENGTH);

    /* Init logger */
    INIT_LOGGER(_LOG_LEVEL);

    LOG_INFO("#==========================#\n");
    LOG_INFO("| HWUID: %s |\n", uniqueHWUID);
    LOG_INFO("#==========================#\n");

    /* Init the WIFI AP */
    retCode = wifiAP.InitAP(uniqueHWUID, "econbadgepsswd");
    if(retCode == NO_ERROR)
    {
        LOG_INFO("EConBadge IP: %s\n", wifiAP.GetIPAddr().c_str());
        retCode = wifiAP.StartServer(serverPort);
        if(retCode == NO_ERROR)
        {
            LOG_INFO("Wifi Server started on port %d\n", serverPort);
        }
        else
        {
            LOG_ERROR("Could not start Wifi server: Error %d\n", retCode);
        }
    }
    else
    {
        LOG_ERROR("Could not start Wifi AP: Error %d\n", retCode);
    }

    /* On start the system state is waiting for wifi client.
     * TODO: Later in the project, we should load this value from flash to
     * know what to do after a reset
     */
    systemState.SetSystemState(SYS_WAITING_WIFI_CLIENT);
}

void loop(void)
{
    EErrorCode   retCode;
    ESystemState sysState;

    /* Check the current mode */
    sysState = systemState.GetSystemState();
    if(sysState !=  SYS_IDLE)
    {
        /* Check the action to execute */
        if(sysState == SYS_WAITING_WIFI_CLIENT)
        {
            retCode = wifiAP.UpdateState(systemState, commandController);
            if(retCode != NO_ERROR)
            {
                LOG_ERROR("Could not update WIFI AP. Error %d\n", retCode);
            }
        }

        /* In wake more, we always check the IO states */
        /* TODO: BtnIOManager.UpdateState(systemState, commandController) */
        /* TODO: LEDIOManager.UpdateState(systemState, commandController) */
    }
    else
    {
        /* We are in IDLE mode */

    }
    delay(100);
}

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

/* None */

/********************************* Private ************************************/

/******************************** Protected ***********************************/

/********************************** Public ************************************/

#undef TOFILL