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
#include <HWLayer.h>   /* Hardware services */
#include <Arduino.h>   /* Arduino Main Header File */
#include <LEDBorder.h> /* LED border manager */

/* Header File */
#include <BatteryMgr.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CBMGR BatteryMgr

/** @brief Battery refresh period in milliseconds */
#define BATTERY_REFRESH_PERIOD 1000 /* TODO: Update this */

/** @brief Battery ramp-up and stabilization time in milliseconds */
#define BATTERY_RAMPUP_TIME 50

/** @brief Margin between the discharge voltage and the charging voltage */
#define BATTERY_CHARGE_MARGIN 100

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
    3483 /* 0   % */,    3484 /* 1   % */,    3486 /* 2   % */,
    3487 /* 3   % */,    3489 /* 4   % */,    3491 /* 5   % */,
    3493 /* 6   % */,    3495 /* 7   % */,    3497 /* 8   % */,
    3499 /* 9   % */,    3502 /* 10  % */,    3504 /* 11  % */,
    3507 /* 12  % */,    3510 /* 13  % */,    3513 /* 14  % */,
    3515 /* 15  % */,    3519 /* 16  % */,    3522 /* 17  % */,
    3525 /* 18  % */,    3528 /* 19  % */,    3532 /* 20  % */,
    3535 /* 21  % */,    3539 /* 22  % */,    3542 /* 23  % */,
    3546 /* 24  % */,    3550 /* 25  % */,    3554 /* 26  % */,
    3558 /* 27  % */,    3562 /* 28  % */,    3566 /* 29  % */,
    3570 /* 30  % */,    3574 /* 31  % */,    3578 /* 32  % */,
    3583 /* 33  % */,    3587 /* 34  % */,    3592 /* 35  % */,
    3596 /* 36  % */,    3601 /* 37  % */,    3605 /* 38  % */,
    3610 /* 39  % */,    3614 /* 40  % */,    3619 /* 41  % */,
    3624 /* 42  % */,    3629 /* 43  % */,    3633 /* 44  % */,
    3638 /* 45  % */,    3643 /* 46  % */,    3648 /* 47  % */,
    3653 /* 48  % */,    3658 /* 49  % */,    3663 /* 50  % */,
    3668 /* 51  % */,    3673 /* 52  % */,    3678 /* 53  % */,
    3682 /* 54  % */,    3687 /* 55  % */,    3692 /* 56  % */,
    3697 /* 57  % */,    3702 /* 58  % */,    3707 /* 59  % */,
    3712 /* 60  % */,    3717 /* 61  % */,    3722 /* 62  % */,
    3727 /* 63  % */,    3732 /* 64  % */,    3737 /* 65  % */,
    3742 /* 66  % */,    3747 /* 67  % */,    3751 /* 68  % */,
    3756 /* 69  % */,    3761 /* 70  % */,    3766 /* 71  % */,
    3770 /* 72  % */,    3775 /* 73  % */,    3779 /* 74  % */,
    3784 /* 75  % */,    3788 /* 76  % */,    3793 /* 77  % */,
    3797 /* 78  % */,    3802 /* 79  % */,    3806 /* 80  % */,
    3810 /* 81  % */,    3814 /* 82  % */,    3818 /* 83  % */,
    3822 /* 84  % */,    3826 /* 85  % */,    3830 /* 86  % */,
    3834 /* 87  % */,    3838 /* 88  % */,    3842 /* 89  % */,
    3845 /* 90  % */,    3849 /* 91  % */,    3852 /* 92  % */,
    3856 /* 93  % */,    3859 /* 94  % */,    3862 /* 95  % */,
    3865 /* 96  % */,    3868 /* 97  % */,    3871 /* 98  % */,
    3874 /* 99  % */,    3876 /* 100 % */,
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

CBMGR::BatteryMgr(LEDBorder* pLEDBorder)
{
    isCharging_ = false;
    isUpdated_  = false;
    percentage_ = 0;
    lastUpdate_ = HWManager::GetTime();

    pLEDBorderMgr_ = pLEDBorder;
}

EErrorCode CBMGR::Init(void)
{
    /* Initialize the ADC GPIO */
    pinMode(GPIO_ADC_BAT, INPUT_PULLUP);
    analogReadResolution(12);
    /* TODO: Check that */
    analogSetPinAttenuation(GPIO_ADC_BAT, ADC_11db);

    return NO_ERROR;
}

EErrorCode CBMGR::Update(void)
{
    bool     charging;
    bool     ledBorderEnabled;
    uint64_t timeNow;
    uint32_t adcValue;
    uint32_t lastClose;
    uint32_t currClose;
    uint8_t  i;

    charging = false;

    /* Check if we need to update */
    timeNow = HWManager::GetTime();

    if(!timeNow - lastUpdate_ >= BATTERY_REFRESH_PERIOD && isCharging_)
    {
        /* Put the badge in power measurement mode (disable all peripherals) */
        ledBorderEnabled = pLEDBorderMgr_->IsEnabled();
        if(ledBorderEnabled)
        {
            pLEDBorderMgr_->Disable();
        }

        /* Wait for voltage to ramp-up and stabilize */
        delay(BATTERY_RAMPUP_TIME);

        /* Measure voltage TODO: Check millivolt is ok function */
        adcValue = 0;
        for(i = 0; i < 100; ++i)
        {
            adcValue += analogReadMilliVolts(GPIO_ADC_BAT) * 2;
        }
        adcValue /= 100;

        /* Get percentage TODO: Get correct function / table */
        lastClose = 0xFFFFFFFF;
        for(i = 0; i < 101; ++i)
        {
            if(adcValue > skBatteryProfile[i])
            {
                currClose = adcValue - skBatteryProfile[i];
            }
            else
            {
                currClose = skBatteryProfile[i] - adcValue;
            }

            /* Check if we were closer beffore */
            if(lastClose < currClose)
            {
                break;
            }
            lastClose = currClose;
        }
        percentage_ = i - 1;

        /* Set to be updated */
        isUpdated_ = false;
        lastUpdate_ = timeNow;

        /* Enable the peripherals again */
        if(ledBorderEnabled)
        {
            pLEDBorderMgr_->Enable();
        }
    }
    else
    {
        /* Measure voltage TODO: Check millivolt is ok function */
        adcValue = analogReadMilliVolts(GPIO_ADC_BAT);

        /* We are charging when the voltage is higher than the maximal value */
        if(adcValue + BATTERY_CHARGE_MARGIN > skBatteryProfile[100])
        {
            charging = true;
        }
        else
        {
            charging = false;
        }

        /* Fast check the percentage to detect charging state */
        if(!isCharging_ && charging)
        {
            isCharging_ = true;
        }
        else if(!isCharging_ && charging)
        {
            /* Force to retrieve the new battery level */
            lastUpdate_ = 0;
            isCharging_ = false;
        }
        isUpdated_ = false;
    }

    return NO_ERROR;
}

bool CBMGR::IsUpdated(void)
{
    bool save;

    save = isUpdated_;
    isUpdated_ = true;
    return save;
}

bool CBMGR::IsCharging(void)
{
    return isCharging_;
}

uint32_t CBMGR::GetPercentage(void)
{
    return percentage_;
}

#undef CBMGR