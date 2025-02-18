/*******************************************************************************
 * @file DisplayInterface.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/02/2025
 *
 * @version 1.0
 *
 * @brief This file defines the types user interface manager.
 *
 * @details This file defines the types user interface manager. The manager
 * displays and compose what is sent to the screen.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>          /* std::string */
#include <vector>          /* std::vector */
#include <OLEDScreenMgr.h> /* OLED screen manager */

/* Header File */
#include <DisplayInterface.h>

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
 * CLASS METHODS
 ******************************************************************************/
DisplayInterface::DisplayInterface(OLEDScreenMgr* pOLEDScreen)
{
    pOLEDScreen_ = pOLEDScreen;
}

void DisplayInterface::Enable(const bool kEnabled)
{
    /* Nothing to do */
    if(isEnabled_ == kEnabled)
    {
        return;
    }

    if(kEnabled)
    {
        pOLEDScreen_->SwitchOn();
    }
    else
    {
        pOLEDScreen_->SwitchOff();
    }

    isEnabled_ = kEnabled;
}