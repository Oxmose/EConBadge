/*******************************************************************************
 * @file OLEDScreenMgr.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the OLED screen manager.
 *
 * @details This file contains the OLED screen manager. The file provides the
 * services to update the screen, enable and disable it.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>            /* String manipulation*/
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logger service */
#include <CommandControler.h> /* Command controller service */
#include <SystemState.h>      /* System State Service */
#include <version.h>          /* Versioning */
#include <HWLayer.h>          /* Hardware Services */

#include <Adafruit_GFX.h>     /* OLED Screen Manipulation */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

/* Header File */
#include <OLEDScreenDriver.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define COLMGR nsHWL::COLEDScreenMgr

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define LOGO_WIDTH    48
#define LOGO_HEIGHT   48

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

const unsigned char LOGO_BITMAP [] PROGMEM = {
	0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x78,
	0x00, 0x00, 0x00, 0x00, 0x78, 0x9e, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe7, 0x00, 0x00, 0x00, 0x03,
	0xc0, 0x79, 0xc0, 0x00, 0x00, 0x0f, 0x00, 0x1c, 0xf0, 0x00, 0x00, 0x1c, 0x03, 0x0f, 0x3c, 0x00,
	0x00, 0x78, 0x0f, 0x83, 0xce, 0x00, 0x01, 0xe0, 0x3f, 0xe0, 0xe7, 0x80, 0x07, 0x80, 0x7f, 0x30,
	0x79, 0xe0, 0x0e, 0x01, 0xfe, 0x0c, 0x1e, 0x70, 0x1c, 0x07, 0xf8, 0x03, 0x07, 0xb8, 0x18, 0x1f,
	0xe0, 0x00, 0xc1, 0xd8, 0x1b, 0x3f, 0xc0, 0x00, 0x60, 0xd8, 0x1b, 0x7f, 0x00, 0x00, 0x13, 0x18,
	0x1b, 0x7c, 0x00, 0x00, 0x0f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00,
	0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78,
	0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18,
	0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00,
	0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78,
	0x00, 0x00, 0x1f, 0x18, 0x1b, 0x78, 0x00, 0x00, 0x1f, 0x18, 0x1b, 0x7c, 0x00, 0x00, 0x3f, 0x18,
	0x1b, 0x7f, 0x00, 0x00, 0xff, 0x18, 0x1b, 0x3f, 0xc0, 0x03, 0xfc, 0x98, 0x18, 0x1f, 0xe0, 0x07,
	0xf9, 0xd8, 0x1c, 0x07, 0xf8, 0x1f, 0xe7, 0xb8, 0x0e, 0x01, 0xfe, 0x7f, 0x9e, 0x70, 0x07, 0x80,
	0x7f, 0xfe, 0x39, 0xe0, 0x01, 0xe0, 0x3f, 0xfc, 0xe7, 0x80, 0x00, 0x70, 0x0f, 0xf3, 0xce, 0x00,
	0x00, 0x3c, 0x03, 0xcf, 0x38, 0x00, 0x00, 0x0f, 0x01, 0x9c, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x73,
	0xc0, 0x00, 0x00, 0x00, 0xe0, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x78, 0x9e, 0x00, 0x00, 0x00, 0x00,
	0x1e, 0x78, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00
};



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

COLMGR::COLEDScreenMgr(void)
{
    this->display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    if(this->display != nullptr)
    {
        this->lastState = SYS_IDLE;
        LOG_INFO("Allocated the OLED screen manager\n");
    }
    else
    {
        LOG_CRITICAL("Could not allocate OLED Screen manager\n");
    }
}

COLMGR::~COLEDScreenMgr(void)
{
    if(this->display != nullptr)
    {
        delete this->display;
    }
}

EErrorCode COLMGR::Init(void)
{
    EErrorCode retCode;

    if(this->display->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        this->display->display();
        delay(100);
        this->display->setTextColor(WHITE);
        this->display->setTextSize(1);
        this->display->clearDisplay();
        this->display->setCursor(0, 0);
        this->display->display();
        retCode = NO_ERROR;
    }
    else
    {
        LOG_ERROR("SSD1306 allocation failed");
        retCode = ACTION_FAILED;
    }

    return retCode;
}

void COLMGR::DisplaySplash(void)
{
    char uniqueHWUID[HW_ID_LENGTH];

    /* Get the unique hardware ID */
    nsHWL::CHWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);

    this->display->ssd1306_command(SSD1306_DISPLAYON);
    this->display->clearDisplay();
    this->display->setTextColor(WHITE);
    this->display->setTextSize(1);
    this->display->setCursor(0, 0);
    this->display->printf("SW %s\n", VERSION_SHORT);
    this->display->printf(PROTO_REV " ");
    this->display->printf(uniqueHWUID);
    this->display->setCursor(55, 24);
    this->display->printf(" Created By");
    this->display->setCursor(55, 32);
    this->display->printf("   Olson");
    this->display->setCursor(55, 48);
    this->display->printf("  OlsonTek");

    this->display->drawBitmap(0, 16, LOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
    this->display->display();
}

void COLMGR::DisplayDebug(const nsCore::CSystemState & sysState)
{
    uint8_t debugState;

    this->display->ssd1306_command(SSD1306_DISPLAYON);

    this->display->clearDisplay();
    this->display->setTextColor(WHITE);
    this->display->setTextSize(1);
    this->display->setCursor(0, 0);

    debugState = sysState.GetDebugState();

    /* Display System State */
    if(debugState == 1)
    {
        this->display->printf("DebugV | %s\n", VERSION);
        this->display->printf("STATE: %d\n", sysState.GetSystemState());
        this->display->printf("LEVT: %d\n", sysState.GetLastEventTime());
        this->display->printf("WIFI: %d | BT: %d\n", 0, 0);
    }
    else if(debugState == 2)
    {
        /* Display Inputs State */
        this->display->printf("BU:%d (%u) BD:%d (%u)\nBE:%d (%u)\n",
                            sysState.GetButtonState(BUTTON_UP),
                            sysState.GetButtonKeepTime(BUTTON_UP),
                            sysState.GetButtonState(BUTTON_DOWN),
                            sysState.GetButtonKeepTime(BUTTON_DOWN),
                            sysState.GetButtonState(BUTTON_ENTER),
                            sysState.GetButtonKeepTime(BUTTON_ENTER));
    }
    else if(debugState == 3)
    {
        this->display->printf("\n\n\n     Exit Debug?");
    }

    this->display->display();
}

Adafruit_SSD1306* COLMGR::GetDisplay(void)
{
    return this->display;
}

#undef COLMGR