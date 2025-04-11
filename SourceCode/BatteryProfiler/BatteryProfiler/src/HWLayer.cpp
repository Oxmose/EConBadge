/*******************************************************************************
 * @file HWLayer.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/04/2024
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

#include <esp_mac.h>  /* Mac address provider */
#include <esp_timer.h>  /* Timer provider */
#include <Arduino.h>     /* Task and Delay services */
#include <Logger.h> /* Logger service */
/* Header File */
#include <HWLayer.h>

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
std::string HWLayer::HWUID_;
std::string HWLayer::MACADDR_;
uint64_t    HWLayer::TIME_ = 0;

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

const char * HWLayer::GetHWUID(void)
{
    uint8_t  value[6];
    uint8_t  i;

    /* Check if the HWUID was already generated */
    if(HWLayer::HWUID_.size() == 0)
    {
        HWLayer::HWUID_ = "FSL-";

        esp_read_mac(value, ESP_MAC_BT);

        for(i = 0; i < 4; ++i)
        {
            HWLayer::HWUID_ += std::string(1, spkHexTable[(value[i] >> 4) & 0xF]) +
                               std::string(1, spkHexTable[value[i] & 0xF]);
        }
    }

    /* Copy HWUID */
    return HWLayer::HWUID_.c_str();
}

const char * HWLayer::GetMacAddress(void)
{
    uint8_t  i;
    uint8_t  value[6];

    /* Check if the HWUID was already generated */
    if(HWLayer::MACADDR_.size() == 0)
    {
        esp_read_mac(value, ESP_MAC_BT);

        HWLayer::MACADDR_ = "";
        for(i = 0; i < 6; ++i)
        {
            HWLayer::MACADDR_ += std::string(1, spkHexTable[(value[i] >> 4) & 0xF]) +
                                 std::string(1, spkHexTable[value[i] & 0xF]);
            if(i < 5)
            {
                HWLayer::MACADDR_ += ":";
            }
        }
    }

    /* Copy HWUID */
    return HWLayer::MACADDR_.c_str();
}

uint64_t HWLayer::GetTime(void)
{
    HWLayer::TIME_ = (uint64_t)esp_timer_get_time();

    return HWLayer::TIME_;
}

void HWLayer::DelayExecUs(const uint64_t kDelayUs, const bool kForcePassive)
{
    /* Check if we can use a precise wait */
    if(kDelayUs > 1000 && kDelayUs % 1000 == 0)
    {
        vTaskDelay(kDelayUs / 1000);
    }
    else if(kForcePassive)
    {
        LOG_ERROR("Cannot perform passive wait with delay %llu\n", kDelayUs);
    }
    ets_delay_us(kDelayUs);
}