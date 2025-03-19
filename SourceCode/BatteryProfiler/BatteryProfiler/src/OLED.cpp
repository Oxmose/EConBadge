/*******************************************************************************
 * @file OLED.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/04/2024
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
#include <Logger.h>           /* Logger service */

#include <Adafruit_GFX.h>     /* OLED Screen Manipulation */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

/* Header File */
#include <OLED.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

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

OLED::OLED(void)
{
    pDisplay_ = new Adafruit_SSD1306(SCREEN_WIDTH,
                                     SCREEN_HEIGHT,
                                     &Wire,
                                     OLED_RESET);
}

OLED::~OLED(void)
{
    delete pDisplay_;
}

void OLED::Init(void)
{
    if(pDisplay_ == nullptr)
    {
        LOG_ERROR("SSD1306 allocation failed.\n");
    }

    if(pDisplay_->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        pDisplay_->ssd1306_command(SSD1306_DISPLAYON);
        pDisplay_->setTextColor(WHITE);
        pDisplay_->setTextSize(1);
        pDisplay_->clearDisplay();
        pDisplay_->setCursor(0, 0);
        pDisplay_->display();
    }
    else
    {
        LOG_ERROR("SSD1306 initialization failed.\n");
    }
}

void OLED::DisplaySleep(void)
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

void OLED::SwitchOff(void)
{
    pDisplay_->ssd1306_command(SSD1306_DISPLAYOFF);
}

Adafruit_SSD1306* OLED::GetDisplay(void)
{
    return pDisplay_;
}