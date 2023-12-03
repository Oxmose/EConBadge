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
#define COLMGR OLEDScreenMgr

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

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

COLMGR::OLEDScreenMgr(void)
{
    display_ = new Adafruit_SSD1306(SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    &Wire,
                                    OLED_RESET);
    if(display_ != nullptr)
    {
        LOG_DEBUG("Allocated the OLED screen manager\n");
    }
    else
    {
        LOG_CRITICAL("Could not allocate OLED Screen manager\n");
    }
}

COLMGR::~OLEDScreenMgr(void)
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
    display_->ssd1306_command(SSD1306_DISPLAYON);
    display_->clearDisplay();
    display_->setTextColor(WHITE);
    display_->setTextSize(1);
    display_->setCursor(0, 0);
    display_->printf("SW %s\n", VERSION_SHORT);
    display_->printf(PROTO_REV " ");
    display_->printf(HWManager::GetHWUID());
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