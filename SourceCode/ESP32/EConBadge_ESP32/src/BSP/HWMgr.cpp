/*******************************************************************************
 * @file HWMgr.cpp
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
#include <string>      /* std::string*/
#include <Types.h>     /* Defined Types */
#include <Logger.h>    /* Logging service */
#include <esp_mac.h>   /* Mac address provider */
#include <esp_timer.h> /* Timer provider */

/* Header File */
#include <HWMgr.h>

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
/** @brief See HWManage.h */
std::string HWManager::HWUID_;
/** @brief See HWManage.h */
std::string HWManager::MACADDR_;
/** @brief See HWManage.h */
uint64_t HWManager::TIME_;

/** @brief Generic SPI bus */
SPIClass GEN_SPI(HSPI);

/************************** Static global variables ***************************/
/** @brief Decimal to Hexadecimal convertion table */
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

const char* HWManager::GetHWUID(void)
{
    uint8_t value[6];
    uint8_t i;

    /* Check if the HWUID was already generated */
    if(HWUID_.size() == 0)
    {
        HWUID_ = "ECB-";

        esp_read_mac(value, ESP_MAC_BT);

        for(i = 0; i < 4; ++i)
        {
            HWUID_ += std::string(1, spkHexTable[(value[i] >> 4) & 0xF]) +
                      std::string(1, spkHexTable[value[i] & 0xF]);
        }
    }

    /* Copy HWUID */
    return HWUID_.c_str();
}

const char* HWManager::GetMacAddress(void)
{
    uint8_t i;
    uint8_t value[6];

    /* Check if the HWUID was already generated */
    if(MACADDR_.size() == 0)
    {
        esp_read_mac(value, ESP_MAC_BT);

        MACADDR_ = "";
        for(i = 0; i < 6; ++i)
        {
            MACADDR_ += std::string(1, spkHexTable[(value[i] >> 4) & 0xF]) +
                        std::string(1, spkHexTable[value[i] & 0xF]);
            if(i < 5)
            {
                MACADDR_ += ":";
            }
        }
    }

    /* Copy HWUID */
    return MACADDR_.c_str();
}

uint64_t HWManager::GetTime(void)
{
    TIME_ = (uint64_t)esp_timer_get_time();

    return TIME_;
}

void HWManager::Init(void)
{
    /* Begin ELINK and General SPI (with custom pins) */
    EINK_SPI.begin(GPIO_EINK_CLK, 33, GPIO_EINK_DIN, GPIO_EINK_CS);
    GENERAL_SPI.begin(GPIO_SD_CLK, GPIO_SD_MISO, GPIO_SD_MOSI, GPIO_SD_CS);

    /* Init time */
    TIME_ = 0;
}

void HWManager::DelayExecUs(const uint64_t kDelayUs, const bool kForcePassive)
{
    /* Check if we can use a precise wait */
    if(kDelayUs > 1000 && kDelayUs % 1000 == 0)
    {
        vTaskDelay((kDelayUs / 1000) / portTICK_PERIOD_MS);
    }
    else
    {
        if(kForcePassive)
        {
            LOG_ERROR(
                "Cannot perform passive wait with delay %llu\n",
                kDelayUs
            );
        }
        ets_delay_us(kDelayUs);
    }
}