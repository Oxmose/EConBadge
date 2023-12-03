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

#define GEN_SPI_SCK_PIN  14
#define GEN_SPI_MISO_PIN 26
#define GEN_SPI_MOSI_PIN 13
#define GEN_SPI_CS_PIN   15

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
String   CHWMGR::HWUID;
String   CHWMGR::MACADDR;
uint64_t CHWMGR::TIME = 0;
SPIClass GEN_SPI(HSPI);

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

const char* CHWMGR::GetHWUID(void)
{
    /* Check if the HWUID was already generated */
    if(CHWMGR::HWUID.length() == 0)
    {
        CHWMGR::HWUID = "ECB-" +
                        String((uint32_t)ESP.getEfuseMac(), HEX);
        CHWMGR::HWUID.toUpperCase();
    }

    /* Copy HWUID */
    return CHWMGR::HWUID.c_str();
}

const char* CHWMGR::GetMacAddress(void)
{
    size_t i;
    String newString;

    /* Check if the HWUID was already generated */
    if(CHWMGR::MACADDR.length() == 0)
    {
        CHWMGR::MACADDR = String(ESP.getEfuseMac(), HEX);
        CHWMGR::MACADDR.toUpperCase();
        for(i = 0; i < CHWMGR::MACADDR.length(); i += 2)
        {
            if(i != 0 && i % 2 == 0)
            {
                newString += ":";
            }
            newString += String(CHWMGR::MACADDR[10 - i]) + String(CHWMGR::MACADDR[10 - i + 1]);
        }
    }

    /* Copy HWUID */
    CHWMGR::MACADDR = newString;
    return CHWMGR::MACADDR.c_str();
}

uint64_t CHWMGR::GetTime(void)
{
    uint64_t timeLow;
    uint64_t timeHigh;
    uint32_t timeNow;

    timeLow  = CHWMGR::TIME & 0xFFFFFFFF;
    timeHigh = (CHWMGR::TIME >> 32) & 0xFFFFFFFF;

    /* Manage rollover */
    timeNow = millis();
    if(timeLow > timeNow)
    {
        ++timeHigh;
    }
    timeLow = timeNow;

    CHWMGR::TIME = timeLow | (timeHigh << 32);

    return CHWMGR::TIME;
}

void CHWMGR::Init(void)
{
    /* Begin ELINK and General SPI (with custom pins) */
    EINK_SPI.begin();
    GEN_SPI.begin(GEN_SPI_SCK_PIN,
                  GEN_SPI_MISO_PIN,
                  GEN_SPI_MOSI_PIN,
                  GEN_SPI_CS_PIN);
}

#undef CHWMGR