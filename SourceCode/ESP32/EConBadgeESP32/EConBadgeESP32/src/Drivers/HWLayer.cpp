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
#include <string>  /* std::string*/
#include <WiFi.h>  /* Mac address provider */
#include <Types.h> /* Defined Types */

/* Header File */
#include <HWLayer.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CHWMGR HWManager

#define GEN_SPI_SCK_PIN  14
#define GEN_SPI_MISO_PIN 13
#define GEN_SPI_MOSI_PIN 26
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
std::string CHWMGR::HWUID_;
std::string CHWMGR::MACADDR_;
uint64_t    CHWMGR::TIME_ = 0;

SPIClass    GEN_SPI(HSPI);

/************************** Static global variables ***************************/
static const char spkHexTable[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

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

const char * CHWMGR::GetHWUID(void)
{
    uint32_t uid;
    uint8_t  i;
    uint8_t  curVal;

    /* Check if the HWUID was already generated */
    if(CHWMGR::HWUID_.size() == 0)
    {
        CHWMGR::HWUID_ = "ECB-";

        uid = (uint32_t)ESP.getEfuseMac();

        for(i = 0; i < 8; ++i)
        {
            curVal = uid >> (28 - i * 4);
            CHWMGR::HWUID_ += spkHexTable[curVal & 0xF];
        }
    }

    /* Copy HWUID */
    return CHWMGR::HWUID_.c_str();
}

const char * CHWMGR::GetMacAddress(void)
{
    uint8_t  i;
    uint8_t  curVal;
    uint64_t value;

    /* Check if the HWUID was already generated */
    if(CHWMGR::MACADDR_.size() == 0)
    {
        value = ESP.getEfuseMac();

        CHWMGR::MACADDR_ = "";
        for(i = 0; i < 12; ++i)
        {
            curVal = value >> (i * 4);

            if(i % 2 == 0 && i != 0)
            {
                CHWMGR::MACADDR_ += ":";
            }
            CHWMGR::MACADDR_ += spkHexTable[curVal & 0xF];
        }
    }

    /* Copy HWUID */
    return CHWMGR::MACADDR_.c_str();
}

uint64_t CHWMGR::GetTime(void)
{
    uint64_t timeLow;
    uint64_t timeHigh;
    uint32_t timeNow;

    timeLow  = CHWMGR::TIME_ & 0xFFFFFFFF;
    timeHigh = (CHWMGR::TIME_ >> 32) & 0xFFFFFFFF;

    /* Manage rollover */
    timeNow = millis();
    if(timeLow > timeNow)
    {
        ++timeHigh;
    }
    timeLow = timeNow;

    CHWMGR::TIME_ = timeLow | (timeHigh << 32);

    return CHWMGR::TIME_;
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