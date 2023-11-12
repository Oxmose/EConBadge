/*******************************************************************************
 * @file LEDBorder.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 30/12/2022
 *
 * @version 1.0
 *
 * @brief This file defines the LED border manager and driver.
 *
 * @details This file defines the LED border manager and driver. It initializes
 * the LED border and provides the different services used to manage the border.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __DRIVERS_LEDBORDER_H_
#define __DRIVERS_LEDBORDER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */
#include <vector>  /* std::vector */
#include <mutex>   /* std::mutex */
#include <SystemState.h> /* System state manager */

#include <FastLED.h> /* Fast LED Service */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define LED_PIN     27
#define NUM_LEDS    120
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    LED_COLOR_PATTERN_PLAIN      = 0,
    LED_COLOR_PATTERN_GRADIENT_1 = 1,
    LED_COLOR_PATTERN_GRADIENT_2 = 2,
    LED_COLOR_PATTERN_GRADIENT_3 = 3,
    LED_COLOR_PATTERN_GRADIENT_4 = 4
} ELEDBorderColorPattern;

typedef struct
{
    uint32_t startColorCode[4];
    uint32_t endColorCode[4];
    union
    {
        uint8_t  gradientSize[4];
        uint32_t plainColorCode;
    };
} SLEDBorderColorPatternParam;

typedef enum
{
    LED_COLOR_ANIM_TRAIL  = 0,
    LED_COLOR_ANIM_BREATH = 1,
} ELEDBorderAnimation;

typedef struct
{
    union
    {
        uint8_t speedIncrease;
        uint8_t rateDivider;
    };
} SLEDBorderAnimationParam;

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

class ColorPattern
{
    public:
        ColorPattern(const bool isDynamic, const uint16_t ledCount);
        virtual ~ColorPattern(void);
        virtual bool IsDynamic(void) const;

        virtual void ApplyPattern(uint32_t * ledsColors) = 0;

    protected:
        uint16_t ledCount_;

    private:
        bool     isDynamic_;
};

class IColorAnimation
{
    public:
        virtual ~IColorAnimation(void){}
        virtual void SetMaxBrightness(const uint8_t maxBrightness) = 0;
        virtual void ApplyAnimation(uint32_t * ledColors,
                                    const uint16_t ledCount,
                                    const uint32_t iterNum) = 0;

    protected:
        volatile uint8_t maxBrightness_;

    private:

};

class LEDBorder
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        LEDBorder(SystemState * systemState);
        ~LEDBorder(void);

        void Init(void);

        void Enable(void);
        void Disable(void);
        bool IsEnabled(void) const;

        void Update(void);
        void Refresh(void);

        void IncreaseBrightness(void);
        void ReduceBrightness(void);

        uint32_t * GetLEDArrayColors(void);

        std::vector<IColorAnimation*> * GetColorAnimations(void);
        ColorPattern *                  GetColorPattern(void);

        void Lock(void);
        void Unlock(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void    SetPattern(const ELEDBorderColorPattern patternId,
                           const SLEDBorderColorPatternParam * patternParam);
        uint8_t AddAnimation(const ELEDBorderAnimation animId,
                             const SLEDBorderAnimationParam * param);
        void    RemoveAnimation(const uint8_t animIdx);
        void    ClearAnimations(void);

        CRGB     leds_[NUM_LEDS];
        uint32_t ledsColors_[NUM_LEDS];

        TaskHandle_t workerThread_;

        uint8_t brightness_;

        std::vector<IColorAnimation*>   animations_;
        ColorPattern                  * pattern_;

        std::mutex driverLock_;

        SystemState * systemState_;

        bool enabled_;
};


#endif /* #ifndef __DRIVERS_LEDBORDER_H_ */