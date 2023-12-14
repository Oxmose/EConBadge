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
    pDisplay_ = new Adafruit_SSD1306(SCREEN_WIDTH,
                                     SCREEN_HEIGHT,
                                     &Wire,
                                     OLED_RESET);
}

COLMGR::~OLEDScreenMgr(void)
{
    delete pDisplay_;
}

EErrorCode COLMGR::Init(void)
{
    EErrorCode retCode;

    if(pDisplay_->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        pDisplay_->display();

        delay(100);

        pDisplay_->setTextColor(WHITE);
        pDisplay_->setTextSize(1);
        pDisplay_->clearDisplay();
        pDisplay_->setCursor(0, 0);
        pDisplay_->display();

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
    pDisplay_->ssd1306_command(SSD1306_DISPLAYON);
    pDisplay_->clearDisplay();
    pDisplay_->setTextColor(WHITE);
    pDisplay_->setTextSize(1);
    pDisplay_->setCursor(0, 0);
    pDisplay_->printf("SW %s\n", VERSION_SHORT);
    pDisplay_->printf(PROTO_REV " ");
    pDisplay_->printf(HWManager::GetHWUID());
    pDisplay_->setCursor(55, 24);
    pDisplay_->printf(" Created By");
    pDisplay_->setCursor(55, 32);
    pDisplay_->printf("   Olson");
    pDisplay_->setCursor(55, 48);
    pDisplay_->printf("  OlsonTek");

    pDisplay_->drawBitmap(0, 16, PKLOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
    pDisplay_->display();
}

void COLMGR::DisplaySleep(void)
{
    pDisplay_->ssd1306_command(SSD1306_DISPLAYON);
    pDisplay_->clearDisplay();
    pDisplay_->setTextSize(1);

    /* Draw Title */
    pDisplay_->setTextColor(BLACK);
    pDisplay_->setCursor(0, 5);
    pDisplay_->fillRect(0, 0, 128, 16, WHITE);
    pDisplay_->printf("   --   SLEEP   --   ");

    /* Draw background */
    pDisplay_->fillRect(1, 17, 126, 40, BLACK);
    pDisplay_->drawRect(0, 16, 128, 42, WHITE);

    /* Print */
    pDisplay_->setTextColor(WHITE);
    pDisplay_->setCursor(4, 22);
    pDisplay_->printf("  Going to sleep...\n\n         Bye!");

    pDisplay_->display();
}

void COLMGR::SwitchOff(void)
{
    pDisplay_->ssd1306_command(SSD1306_DISPLAYOFF);
}

Adafruit_SSD1306* COLMGR::GetDisplay(void)
{
    return pDisplay_;
}

#undef COLMGR