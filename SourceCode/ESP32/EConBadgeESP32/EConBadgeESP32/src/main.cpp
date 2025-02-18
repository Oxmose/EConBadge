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
#include <Menu.h>             /* Menu manager */
#include <Types.h>            /* Custom types */
#include <cstdint>            /* Generic types */
#include <Logger.h>           /* Logger */
#include <Arduino.h>          /* Arduino Main Header File */
#include <HWLayer.h>          /* Hardware services */
#include <Updater.h>          /* Udpater service */
#include <version.h>          /* Versionning */
#include <LEDBorder.h>        /* LED Border manager */
#include <BatteryMgr.h>       /* Battery manager */
#include <SystemState.h>      /* System state manager*/
#include <IOButtonMgr.h>      /* Button manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <OLEDScreenMgr.h>    /* OLED Screen manager */
#include <WaveshareEInkMgr.h> /* EInk manager */

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
static IOButtonMgr*        spIoBtnMgr;
static OLEDScreenMgr*      spOledScreenMgr;
static BluetoothManager*   spBtMgr;
static SystemState*        spSystemState;
static Updater*            spUpdater;
static EInkDisplayManager* spEInkMgr;
static LEDBorder*          spLedBorderMgr;
static Menu*               spMenuMgr;
static BatteryMgr*         spBatteryMgr;

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
    char       uniqueHWUID[HW_ID_LENGTH + 1];

    /* Init Hardware Layer */
    HWManager::Init();

    /* Init logger */
    INIT_LOGGER(LOG_LEVEL_DEBUG);

    /* Get the unique hardware ID */
    strncpy(uniqueHWUID, HWManager::GetHWUID(), HW_ID_LENGTH);
    uniqueHWUID[HW_ID_LENGTH] = 0;

    LOG_INFO("#=====================#\n");
    LOG_INFO("| HWUID: %s |\n", uniqueHWUID);
    LOG_INFO("#=====================#\n");
    LOG_INFO("===> SW " VERSION "\n");
    LOG_INFO("===> CPU Frequency: %dMHz\n", getCpuFrequencyMhz());

    spIoBtnMgr = new IOButtonMgr();

    /* Init the buttons */
    retCode = spIoBtnMgr->Init();
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init buttons (%d)\n", retCode);
    }
    LOG_INFO("Buttons initialized\n");


    spOledScreenMgr = new OLEDScreenMgr();
    spBtMgr         = new BluetoothManager();
    spEInkMgr       = new EInkDisplayManager(spBtMgr);
    spSystemState   = new SystemState(
        spOledScreenMgr,
        spIoBtnMgr,
        spBtMgr,
        spEInkMgr
    );
    spLedBorderMgr  = new LEDBorder(spSystemState);
    spBatteryMgr    = new BatteryMgr(spLedBorderMgr);
    spSystemState->SetLedBorder(spLedBorderMgr);

    spUpdater = new Updater(spBtMgr, spSystemState);
    spSystemState->SetUpdater(spUpdater);


    spMenuMgr  = new Menu(spOledScreenMgr,
                          spSystemState,
                          spEInkMgr,
                          spLedBorderMgr,
                          spUpdater,
                          spBtMgr,
                          spBatteryMgr);


    /* Init the BT manager */
    spBtMgr->Init(spSystemState);
    LOG_INFO("Bluetooth initialized\n");

    /* Init the OLED screen */
    retCode = spOledScreenMgr->Init();
    if(retCode != EErrorCode::NO_ERROR)
    {
        LOG_ERROR("Failed to init OLED Manager (%d)\n", retCode);
    }
    LOG_INFO("OLED Manager initialized\n");
    spOledScreenMgr->DisplaySplash();

    spLedBorderMgr->Init();
    LOG_INFO("LEDBorder initialized\n");

    /* Init the eInk screen */
    spEInkMgr->Init();
    LOG_INFO("EInk initialized\n");
}

void loop(void)
{
    EErrorCode retCode;
    uint64_t   startTime;
    uint64_t   endTime;
    uint64_t   diffTime;

    startTime = HWManager::GetTime();

    /* Update the inputs */
    retCode = spIoBtnMgr->Update();
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Failed to update IO buttons (%d)\n", retCode);
    }

    /* Update the system state */
    retCode = spSystemState->Update();
    if(retCode != NO_ERROR)
    {
        LOG_ERROR("Failed to update system state (%d)\n", retCode);
    }

    /* Update the menu */
    spMenuMgr->Update();

    /* If we have a popup, don't let the system idle */
    if(spMenuMgr->HasPopup())
    {
        spSystemState->Ping();
    }

    /* Check if an update is occuring */
    spUpdater->Update();

    /* Update the led border */
    spLedBorderMgr->Update();

    /* Update the EInk Screen */
    spEInkMgr->Update();

    endTime = HWManager::GetTime();
    diffTime = endTime - startTime;
    if(diffTime < 25)
    {
        delay(25 - diffTime);
    }
}