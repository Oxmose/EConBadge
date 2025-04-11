/*******************************************************************************
 * @file Battery.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/02/2024
 *
 * @version 1.0
 *
 * @brief This file contains the battery manager.
 *
 * @details This file contains the battery manager. The file provides the
 * services to get the battery status such as its charching state and its
 * current capacity.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>     /* Generic Types */
#include <Types.h>     /* Defined Types */
#include <HWMgr.h>     /* Hardware services */
#include <Arduino.h>   /* Arduino Main Header File */
#include <LEDBorder.h> /* LED border manager */

/* Header File */
#include <BatteryMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define BATTERY_PROFILER_DEV 0

/** @brief Battery refresh period in microseconds */
#define BATTERY_REFRESH_PERIOD 10000000 /* 10s */

/** @brief Battery ramp-up and stabilization time in microseconds */
#define BATTERY_RAMPUP_TIME 50000

/** @brief Margin between the discharge voltage and the charging voltage */
#define BATTERY_CHARGE_MARGIN 100

#define V_ADC_CALIB -70
#define V_DIVIDER_R1 222.33f
#define V_DIVIDER_R2 19.48f

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
static const uint32_t skBatteryProfile[101] = {
    5137 /* 0   % */,    5837 /* 1   % */,    6266 /* 2   % */,
    6517 /* 3   % */,    6656 /* 4   % */,    6730 /* 5   % */,
    6767 /* 6   % */,    6787 /* 7   % */,    6801 /* 8   % */,
    6816 /* 9   % */,    6833 /* 10  % */,    6853 /* 11  % */,
    6876 /* 12  % */,    6901 /* 13  % */,    6925 /* 14  % */,
    6949 /* 15  % */,    6970 /* 16  % */,    6990 /* 17  % */,
    7007 /* 18  % */,    7022 /* 19  % */,    7035 /* 20  % */,
    7047 /* 21  % */,    7057 /* 22  % */,    7068 /* 23  % */,
    7078 /* 24  % */,    7089 /* 25  % */,    7100 /* 26  % */,
    7113 /* 27  % */,    7127 /* 28  % */,    7141 /* 29  % */,
    7157 /* 30  % */,    7174 /* 31  % */,    7191 /* 32  % */,
    7209 /* 33  % */,    7226 /* 34  % */,    7244 /* 35  % */,
    7261 /* 36  % */,    7278 /* 37  % */,    7294 /* 38  % */,
    7309 /* 39  % */,    7324 /* 40  % */,    7337 /* 41  % */,
    7350 /* 42  % */,    7361 /* 43  % */,    7372 /* 44  % */,
    7382 /* 45  % */,    7392 /* 46  % */,    7401 /* 47  % */,
    7410 /* 48  % */,    7418 /* 49  % */,    7427 /* 50  % */,
    7436 /* 51  % */,    7444 /* 52  % */,    7453 /* 53  % */,
    7463 /* 54  % */,    7473 /* 55  % */,    7483 /* 56  % */,
    7493 /* 57  % */,    7504 /* 58  % */,    7516 /* 59  % */,
    7529 /* 60  % */,    7542 /* 61  % */,    7555 /* 62  % */,
    7570 /* 63  % */,    7585 /* 64  % */,    7601 /* 65  % */,
    7618 /* 66  % */,    7635 /* 67  % */,    7654 /* 68  % */,
    7672 /* 69  % */,    7692 /* 70  % */,    7711 /* 71  % */,
    7731 /* 72  % */,    7752 /* 73  % */,    7772 /* 74  % */,
    7792 /* 75  % */,    7812 /* 76  % */,    7832 /* 77  % */,
    7852 /* 78  % */,    7871 /* 79  % */,    7890 /* 80  % */,
    7909 /* 81  % */,    7928 /* 82  % */,    7947 /* 83  % */,
    7966 /* 84  % */,    7986 /* 85  % */,    8007 /* 86  % */,
    8029 /* 87  % */,    8052 /* 88  % */,    8075 /* 89  % */,
    8099 /* 90  % */,    8124 /* 91  % */,    8148 /* 92  % */,
    8171 /* 93  % */,    8192 /* 94  % */,    8212 /* 95  % */,
    8232 /* 96  % */,    8253 /* 97  % */,    8281 /* 98  % */,
    8324 /* 99  % */,
};

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

#if BATTERY_PROFILER_DEV
static void BatteryProfiler(void);
#endif

static void SortValues(uint32_t* values, size_t size)
{
    uint32_t i;
    uint32_t j;
    uint32_t min;
    uint32_t temp;

    for(i = 0; i < size - 1; ++i)
    {
        min = i;
        for(j = i + 1; j < size; ++j)
        {
            if(values[j] < values[min])
            {
                min = j;
            }
        }
        if(min != i)
        {
            temp = values[min];
            values[min] = values[i];
            values[i] = temp;
        }
    }
}

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#if BATTERY_PROFILER_DEV
#include <WiFi.h>
static void BatteryProfiler(void)
{
    const char* ssid = "ECONBATPROFILE";
    const char* password = "ECONBATPROFILE";
    WiFiClient  client;
    WiFiServer  server(80);
    String      httpContent;
    float       adcMVValue;
    uint32_t    i;
    uint32_t    values[200];

    WiFi.softAP(ssid, password);
    server.begin();

    LOG_DEBUG("ECONBATPROFILE: %s\n", WiFi.softAPIP().toString().c_str());

    while(true)
    {
        client = server.available();
        if(client)
        {
            while(client.connected())
            {
                if(client.available())
                {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();

                    /* Measure voltage */
                    memset(values, 0, sizeof(uint32_t) * 200);
                    for(i = 0; i < 200; ++i)
                    {
                        values[i] = analogReadMilliVolts(GPIO_ADC_BAT);
                        HWManager::DelayExecUs(1000);
                    }

                    /* Sort array */
                    SortValues(values, 200);

                    /* Get the medians values */
                    adcMVValue = 0;
                    for(i = 90; i < 110; ++i)
                    {
                        adcMVValue += values[i];
                    }

                    adcMVValue = adcMVValue / 20;
                    adcMVValue *= ((V_DIVIDER_R2 + V_DIVIDER_R1) /
                                   V_DIVIDER_R2);
                    adcMVValue += V_ADC_CALIB;

                    LOG_DEBUG("Battery profiler, read %fmV\n", adcMVValue)
                    client.printf("%f\n", adcMVValue);

                    client.println();
                    client.flush();
                    HWManager::DelayExecUs(1000000);
                    client.stop();
                }
            }


        }
    }
}
#endif

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

BatteryManager::BatteryManager(LEDBorder* pLEDBorder)
{
    percentage_ = 0;
    lastUpdate_ = 0;

    pLEDBorderMgr_ = pLEDBorder;

    /* Initialize the ADC GPIO */
    pinMode(GPIO_ADC_BAT, INPUT);
    analogSetWidth(10);
    analogReadResolution(10);
    analogSetPinAttenuation(GPIO_ADC_BAT, ADC_0db);
}

void BatteryManager::Update(void)
{
#if BATTERY_PROFILER_DEV
    BatteryProfiler();
#else

    bool     ledBorderEnabled;
    uint64_t timeNow;
    float    adcMVValue;
    uint32_t lastClose;
    uint32_t currClose;
    uint8_t  i;
    uint32_t values[200];

    /* Check if we need to update */
    timeNow = HWManager::GetTime();

    if(timeNow - lastUpdate_ >= BATTERY_REFRESH_PERIOD)
    {
        /* Put the badge in power measurement mode (disable all peripherals) */
        ledBorderEnabled = pLEDBorderMgr_->IsEnabled();
        if(ledBorderEnabled)
        {
            pLEDBorderMgr_->Enable(false);
        }

        /* Wait for voltage to ramp-up and stabilize */
        HWManager::DelayExecUs(BATTERY_RAMPUP_TIME);

        /* Measure voltage */
        for(i = 0; i < 200; ++i)
        {
            values[i] = analogReadMilliVolts(GPIO_ADC_BAT);
            HWManager::DelayExecUs(1000);
        }

        /* Sort array */
        SortValues(values, 200);

        /* Get the medians values */
        adcMVValue = 0;
        for(i = 90; i < 110; ++i)
        {
            adcMVValue += values[i];
        }

        adcMVValue = adcMVValue / 20;
        adcMVValue *= ((V_DIVIDER_R2 + V_DIVIDER_R1) /
                       V_DIVIDER_R2);
        adcMVValue += V_ADC_CALIB;

        /* Get percentage */
        lastClose = 0xFFFFFFFF;
        for(i = 0; i < 101; ++i)
        {
            if(adcMVValue > skBatteryProfile[i])
            {
                currClose = adcMVValue - skBatteryProfile[i];
            }
            else
            {
                currClose = skBatteryProfile[i] - adcMVValue;
            }

            /* Check if we were closer beffore */
            if(lastClose < currClose)
            {
                break;
            }
            lastClose = currClose;
        }
        percentage_ = i - 1;
        LOG_DEBUG("Battery now: %d\n", percentage_);

        /* Set to be updated */
        lastUpdate_ = timeNow;

        /* Enable the peripherals again */
        pLEDBorderMgr_->Enable(ledBorderEnabled);
    }
#endif /* #ifndef BATTERY_PROFILER_DEV */
}

uint32_t BatteryManager::GetPercentage(void)
{
    return percentage_;
}