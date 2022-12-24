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
#include <OLEDScreenDriver.h> /* OLED Screen service */
#include <IOButtonMgr.h>      /* IO button service */
#include <IOLEDMgr.h>         /* IO LED service */

#include <SPI.h>
#include "imagedata.h"
#include "epd5in65f.h"

using namespace nsCommon;
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

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
static nsHWL::COLEDScreenMgr     oledMgr;
static nsHWL::CIOButtonMgr       ioBtnMgr;
static nsHWL::CIOLEDMgr          ioLEDMgr;


/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

void SystemUpdate(void);

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

void SystemUpdate(void)
{
    EErrorCode   retCode;

    /* Check the inputs */
    retCode = ioBtnMgr.UpdateState(systemState, commandController);
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not update IO buttons state. Error: %d\n", retCode);
    }

    /* Update the system state */
    retCode = systemState.ComputeState();
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not compute next state. Error %d\n", retCode);
    }

    /* Update the outputs */
    retCode = ioLEDMgr.UpdateState(systemState, commandController);
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not update IO LED manager. Error %d\n", retCode);
    }
}

void setup(void)
{
    EErrorCode retCode;
    char       uniqueHWUID[HW_ID_LENGTH];

    /* Get the unique hardware ID */
    nsHWL::CHWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);

    /* Init logger */
    INIT_LOGGER(_LOG_LEVEL);

    LOG_INFO("#=====================#\n");
    LOG_INFO("| HWUID: %s |\n", uniqueHWUID);
    LOG_INFO("#=====================#\n");

    systemState.SetSystemState(SYS_IDLE);

    //epd.Clear(EPD_5IN65F_WHITE);
    //epd.EPD_5IN65F_Display(gImage_5in65f);
#if 0
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
#endif
    /* Init the OLED Screen */
    retCode = oledMgr.Init();
    if(retCode == NO_ERROR)
    {
        LOG_INFO("OLED Screen initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init OLED screen. Error %d\n", retCode);
    }

    /* Init the LEDS */
    retCode = ioLEDMgr.SetupLED(LED_MAIN, LED_MAIN_PIN);
    if(retCode == NO_ERROR)
    {
        LOG_INFO("Main LED initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Main LED. Error %d\n", retCode);
    }
    retCode = ioLEDMgr.SetupLED(LED_AUX, LED_AUX_PIN);
    if(retCode == NO_ERROR)
    {
        LOG_INFO("Aux LED initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Aux LED. Error %d\n", retCode);
    }

    /* Init the buttons */
    retCode = ioBtnMgr.SetupBtn(BUTTON_ENTER, BUTTON_ENTER_PIN);
    if(retCode == NO_ERROR)
    {
        LOG_INFO("Enter Button initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Enter Button. Error %d\n", retCode);
    }
    retCode = ioBtnMgr.SetupBtn(BUTTON_UP, BUTTON_UP_PIN);
    if(retCode == NO_ERROR)
    {
        LOG_INFO("Up Button initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Up Button. Error %d\n", retCode);
    }
    retCode = ioBtnMgr.SetupBtn(BUTTON_DOWN, BUTTON_DOWN_PIN);
    if(retCode == NO_ERROR)
    {
        LOG_INFO("Down Button initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Down Button. Error %d\n", retCode);
    }

    /* First State Init */
    systemState.Init(&oledMgr);
    systemState.SetSystemState(SYS_START_SPLASH);
    delay(2000);
}

void loop(void)
{
    SystemUpdate();
    delay(50);
}