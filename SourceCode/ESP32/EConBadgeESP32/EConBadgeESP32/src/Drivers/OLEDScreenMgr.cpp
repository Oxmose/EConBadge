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
#include <version.h>          /* Versioning */
#include <HWLayer.h>          /* Hardware Services */

#include <Adafruit_GFX.h>     /* OLED Screen Manipulation */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

/* Header File */
#include <OLEDScreenMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define COLMGR COLEDScreenMgr

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
    display_ = new Adafruit_SSD1306(SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    &Wire,
                                    OLED_RESET);
    if(display_ != nullptr)
    {
        LOG_INFO("Allocated the OLED screen manager\n");
    }
    else
    {
        LOG_CRITICAL("Could not allocate OLED Screen manager\n");
    }
}

COLMGR::~COLEDScreenMgr(void)
{
    if(display_ != nullptr)
    {
        delete display_;
    }
}

EErrorCode COLMGR::Init(void)
{
    EErrorCode retCode;

    if(display_->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        display_->display();

        delay(100);

        display_->setTextColor(WHITE);
        display_->setTextSize(1);
        display_->clearDisplay();
        display_->setCursor(0, 0);
        display_->display();

        retCode = EErrorCode::NO_ERROR;
    }
    else
    {
        LOG_ERROR("SSD1306 allocation failed");

        retCode = EErrorCode::ACTION_FAILED;
    }

    return retCode;
}

void COLMGR::DisplaySplash(void)
{
    char uniqueHWUID[HW_ID_LENGTH];

    /* Get the unique hardware ID */
    CHWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);

    display_->ssd1306_command(SSD1306_DISPLAYON);
    display_->clearDisplay();
    display_->setTextColor(WHITE);
    display_->setTextSize(1);
    display_->setCursor(0, 0);
    display_->printf("SW %s\n", VERSION_SHORT);
    display_->printf(PROTO_REV " ");
    display_->printf(uniqueHWUID);
    display_->setCursor(55, 24);
    display_->printf(" Created By");
    display_->setCursor(55, 32);
    display_->printf("   Olson");
    display_->setCursor(55, 48);
    display_->printf("  OlsonTek");

    display_->drawBitmap(0, 16, LOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
    display_->display();
}

Adafruit_SSD1306* COLMGR::GetDisplay(void)
{
    return display_;
}

#undef COLMGR