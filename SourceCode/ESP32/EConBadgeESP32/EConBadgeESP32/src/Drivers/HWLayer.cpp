/*******************************************************************************
 * @file HWLayer.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 30/03/2021
 *
 * @version 1.0
 *
 * @brief This file defines the hardware layer.
 *
 * @details This file defines the hardware layer. This layer provides services
 * to interact with the ESP32 module hardware.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>        /* String manipulation*/
#include <WiFi.h>         /* Wifi driver */
#include <Types.h>        /* Defined Types */

/* Header File */
#include <HWLayer.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CHWMGR HWManager

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

void CHWMGR::GetHWUID(char * pBuffer, const uint32_t maxLength)
{
    /* Check if the HWUID was already generated */
    if(CHWMGR::HWUID.length() == 0)
    {
        CHWMGR::HWUID = "ECB-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    }

    /* Copy HWUID */
    strncpy(pBuffer, CHWMGR::HWUID.c_str(), maxLength);
}

String CHWMGR::HWUID;

#undef CHWMGR