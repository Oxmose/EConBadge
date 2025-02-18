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
#include <Types.h>            /* Defined Types */
#include <HWMgr.h>            /* Hardware Services */
#include <Logger.h>           /* Logger service */
#include <version.h>          /* Versioning */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

/* Header File */
#include <OLEDScreenMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief The screen width in pixels. */
#define SCREEN_WIDTH  128
/** @brief The screen height in pixels. */
#define SCREEN_HEIGHT 64

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

OLEDScreenMgr::OLEDScreenMgr(void)
{
    /* Set the I2C Pins */
    Wire.setPins(GPIO_OLED_SDA, GPIO_OLED_SCL);
    pDisplay_ = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

    /* Startup the screen */
    if(pDisplay_ != nullptr && pDisplay_->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        pDisplay_->setTextColor(WHITE);
        pDisplay_->setTextSize(1);
        pDisplay_->clearDisplay();
        pDisplay_->setCursor(0, 0);
        pDisplay_->display();
        pDisplay_->ssd1306_command(SSD1306_DISPLAYON);
    }
    else
    {
        LOG_ERROR("SSD1306 initialization failed");
    }
}

void OLEDScreenMgr::DisplaySplash(void)
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

void OLEDScreenMgr::SwitchOff(void)
{
    pDisplay_->ssd1306_command(SSD1306_DISPLAYOFF);
}

void OLEDScreenMgr::SwitchOn(void)
{
    pDisplay_->ssd1306_command(SSD1306_DISPLAYON);
}

Adafruit_SSD1306* OLEDScreenMgr::GetDisplay(void)
{
    return pDisplay_;
}