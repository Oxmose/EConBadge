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
#include <cstdint>            /* Generic types */
#include <HWMgr.h>            /* Hardware services */
#include <Logger.h>           /* Logger */
#include <version.h>          /* Versionning */
#include <IOButtonMgr.h>      /* Buttons manager */
#include <SystemState.h>      /* System state manager */
#include <BlueToothMgr.h>     /* Bluetooth Manager */
#include <OLEDScreenMgr.h>    /* OLED screem manager */
#include <WaveshareEInkMgr.h> /* EInk manager */
#include <DisplayInterface.h> /* Display interface */

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
/** @brief System state manager */
static SystemState* spSystemState;

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
    IOButtonMgr*        pIOButtonsManager;
    OLEDScreenMgr*      pOLEDManager;
    BluetoothManager*   pBlueToothManager;
    DisplayInterface*   pDisplayInterface;
    EInkDisplayManager* pEInkManager;

    /* Init Hardware Layer */
    HWManager::Init();

    /* Init logger */
    INIT_LOGGER(ECB_LOG_LEVEL_DEBUG);

    /* Init OLED and splash during the loading */
    pOLEDManager = new OLEDScreenMgr();
    pOLEDManager->DisplaySplash();

    /* Init log */
    LOG_INFO("#==============================#\n");
    LOG_INFO("| HWUID: %s          |\n", HWManager::GetHWUID());
    LOG_INFO("| " PROTO_REV "                      |\n")
    LOG_INFO("| " VERSION " |\n")
    LOG_INFO("#==============================#\n");

    /* Create the objects */
    pIOButtonsManager = new IOButtonMgr();
    pDisplayInterface = new DisplayInterface(pOLEDManager);
    pBlueToothManager = new BluetoothManager();
    pEInkManager      = new EInkDisplayManager(pBlueToothManager);
    spSystemState     = new SystemState(
        pIOButtonsManager,
        pDisplayInterface,
        pBlueToothManager,
        pEInkManager
    );
    LOG_INFO("Instanciated modules\n");

    /* Initialize the objects */
    pBlueToothManager->Init(spSystemState);
    LOG_INFO("Initialized the BlueTooth manager.\n");
    pEInkManager->Init();
    LOG_INFO("Initialized the EInk manager.\n");
}

void loop(void)
{
    uint64_t startTime;
    uint64_t endTime;
    uint64_t diffTime;

    startTime = HWManager::GetTime();

    /* Update the system state */
    spSystemState->Update();

    endTime = HWManager::GetTime();
    diffTime = endTime - startTime;
    if(diffTime < 25000)
    {
        HWManager::DelayExecUs(25000 - diffTime, false);
    }
}