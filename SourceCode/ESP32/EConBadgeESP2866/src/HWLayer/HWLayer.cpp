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
#include <ESP8266WiFi.h>  /* Wifi driver */
#include <Types.h>        /* Defined Types */

/* Header File */
#include <HWLayer.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CHWMGR nsHWL::CHWManager

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
    if(this->HWUID.length() == 0)
    {
        this->HWUID = "EConBadge-" + String(ESP.getChipId());
    }

    /* Copy HWUID */
    strncpy(pBuffer, this->HWUID.c_str(), maxLength);
}

#undef CHWMGR