/*******************************************************************************
 * @file main.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 08/11/2023
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
#include <cstdint>   /* Generic types */
#include <Arduino.h> /* Arduino Main Header File */

#include <IOButtonMgr.h> /* Button manager */
#include <OLEDScreenMgr.h> /* OLED Screen manager */
#include <HWLayer.h> /* Hardware services */
#include <Types.h>   /* Custom types */
#include <Logger.h>  /* Logger */
#include <Menu.h>    /* Menu manager */
#include <SystemState.h> /* System state manager*/
#include <IOLEDMgr.h> /* IO LED manager */
#include <BlueToothMgr.h> /* Bluetooth manager */
#include <WaveshareEInkMgr.h> /* EInk manager */
#include <LEDBorder.h> /* LED Border manager */
#include <Updater.h>   /* Udpater service */
#include <version.h>   /* Versionning */

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
/* None */

/************************** Static global variables ***************************/
static IOButtonMgr        ioBtnMgr;
static OLEDScreenMgr      oledScreenMgr;
static BluetoothManager   btMgr;
static SystemState        systemState(&ioBtnMgr, &btMgr);
static Updater            updater(&btMgr, &systemState);
static EInkDisplayManager eInkMgr(&systemState, &btMgr);
static LEDBorder          ledBorderMgr(&systemState);
static Menu               menuMgr(&oledScreenMgr,
                                  &systemState,
                                  &eInkMgr,
                                  &ledBorderMgr,
                                  &updater,
                                  &btMgr);
static IOLEDMgr           ioLEDMgr(&systemState);

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
    EErrorCode retCode;
    char       uniqueHWUID[HW_ID_LENGTH];

    /* Init Hardware Layer */
    HWManager::Init();

    /* Init logger */
    INIT_LOGGER(LOG_LEVEL_DEBUG, false);
    LOG_INFO("Hardware Manager initialized.\n");

    /* Get the unique hardware ID */
    strncpy(uniqueHWUID, HWManager::GetHWUID(), HW_ID_LENGTH);
    uniqueHWUID[HW_ID_LENGTH - 1] = 0;

    LOG_INFO("#=====================#\n");
    LOG_INFO("| HWUID: %s |\n", uniqueHWUID);
    LOG_INFO("#=====================#\n");
    LOG_INFO("===> SW " VERSION "\n");
    LOG_INFO("===> CPU Frequency: %dMHz\n", getCpuFrequencyMhz());

    /* Init the BT manager */
    btMgr.Init();
    LOG_INFO("Bluetooth initialized.\n");

    /* Init the OLED screen */
    retCode = oledScreenMgr.Init();
    if(retCode == EErrorCode::NO_ERROR)
    {
        LOG_INFO("OLED Manager initialized.\n");
        oledScreenMgr.DisplaySplash();
    }
    else
    {
        LOG_ERROR("Could not init OLED Manager. Error %d\n", retCode);
    }

    /* Init the buttons */
    retCode = ioBtnMgr.Init();
    if(retCode == EErrorCode::NO_ERROR)
    {
        LOG_INFO("Buttons initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Buttons. Error %d\n", retCode);
    }

    /* Init the LEDs */
    retCode = ioLEDMgr.SetupLED(ELEDID::LED_MAIN, ELEDPin::MAIN_PIN);
    if(retCode == EErrorCode::NO_ERROR)
    {
        LOG_INFO("Main LED initialized.\n");
    }
    else
    {
        LOG_ERROR("Could not init Main LED. Error %d\n", retCode);
    }

    /* Init the eInk screen */
    eInkMgr.Init();
    LOG_INFO("eInk initialized.\n");

    /* Init the LED border manager */
    ledBorderMgr.Init();
    LOG_INFO("LED Border initialized.\n");

    /* Finish init */
    systemState.SetLedBorder(&ledBorderMgr);
    systemState.SetUpdater(&updater);
}

void loop(void)
{
    EErrorCode retCode;
    uint64_t   startTime;
    uint64_t   endTime;
    uint64_t   diffTime;

    startTime = HWManager::GetTime();

    /* Update the inputs */
    retCode = ioBtnMgr.UpdateState();
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Could not update IO buttons state. Error: %d\n", retCode);
    }

    /* Update the system state */
    retCode = systemState.Update();
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Error while updating the system state. Error: %d\n", retCode);
    }

    /* Update the menu */
    menuMgr.Update();

    /* If we have a popup, don't let the system idle */
    if(menuMgr.HasPopup())
    {
        systemState.Ping();
    }

    /* Check if an update is occuring */
    updater.Update();

    /* Update the LEDs */
    ioLEDMgr.Update();

    /* Update the led border */
    ledBorderMgr.Update();

    /* Update the eInk display */
    eInkMgr.Update();

    endTime = HWManager::GetTime();
    diffTime = endTime - startTime;
    if(diffTime < 25)
    {
        delay(25 - diffTime);
    }
}