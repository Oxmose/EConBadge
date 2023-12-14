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

#include <string>             /* std::string */
#include <cstdint>            /* Generic Types */
#include <Types.h>            /* Defined Types */
#include <Adafruit_GFX.h>     /* OLED Screen Manipulation */
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

class OLEDScreenMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        OLEDScreenMgr  (void);
        ~OLEDScreenMgr (void);

        EErrorCode Init (void);

        void DisplaySplash (void);
        void DisplaySleep  (void);

        void SwitchOff (void);

        Adafruit_SSD1306 * GetDisplay (void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        Adafruit_SSD1306 * pDisplay_;
};

#endif /* #ifndef __DRIVERS_OLEDSCREENMGR_H_ */