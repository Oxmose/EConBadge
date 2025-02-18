/*******************************************************************************
 * @file OLEDScreenMgr.h
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

#ifndef __DRIVERS_OLEDSCREENMGR_H_
#define __DRIVERS_OLEDSCREENMGR_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>            /* Generic Types */
#include <Types.h>            /* Defined Types */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

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
 * CLASSES
 ******************************************************************************/
/**
 * @brief The OLED manager class.
 *
 * @details The OLED manager class. This class provides the basic
 * functionalities to manage the OLED screen.
 */
class OLEDScreenMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Constructs a new OLEDScreenMgr object.
         *
         * @details Constructs a new OLEDScreenMgr object. The function
         * initializes the GPIO for the screen and their internal objects.
         */
        OLEDScreenMgr(void);

        /**
         * @brief Displays the splash screen.
         *
         * @details Displays the splash screen.
         */
        void DisplaySplash(void);

        /**
         * @brief Turns off the screen.
         *
         * @details Turns off the screen.
         */
        void SwitchOff(void);

        /**
         * @brief Turns on the screen.
         *
         * @details Turns on the screen.
         */
        void SwitchOn(void);

        /**
         * @brief Returns the internal Adafruit_SSD1306 object for manipulation.
         *
         * @details Returns the internal Adafruit_SSD1306 object for
         * manipulation. This object can be used to write to the screen.
         *
         * @return The internal Adafruit_SSD1306 object is returned.
         */
        Adafruit_SSD1306* GetDisplay(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the internal display object. */
        Adafruit_SSD1306* pDisplay_;
};

#endif /* #ifndef __DRIVERS_OLEDSCREENMGR_H_ */