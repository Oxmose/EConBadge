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

#include <string>        /* std::string */
#include <vector>        /* std::vector */
#include <cstdint>       /* Generic Types */
#include <FastLED.h>     /* Fast LED Service */
#include <SystemState.h> /* System state manager */


/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define NUM_LEDS 120

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
} ELEDBorderPattern;

typedef struct
{
    uint32_t startColorCode[4];
    uint32_t endColorCode[4];
    union
    {
        uint8_t  gradientSize[4];
        uint32_t plainColorCode;
    };
} SLEDBorderPatternParam;

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

class ColorPattern : public Printable
{
    public:
        ColorPattern          (const uint16_t               kLedCount,
                               const ELEDBorderPattern kType);
        virtual ~ColorPattern (void);

        virtual size_t printTo  (Print & rPrinter) const = 0;
        virtual bool   readFrom (Stream & rStream) = 0;

        virtual void ApplyPattern (uint32_t * pLedsColors) = 0;

        virtual ELEDBorderPattern GetType (void) const = 0;

        virtual void GetRawParam (SLEDBorderPatternParam * pPar) const = 0;

    protected:
        bool                   isApplied_;
        uint16_t               ledCount_;

        ELEDBorderPattern type_;

    private:
};

class IColorAnimation : public Printable
{
    public:
        virtual ~IColorAnimation (void);

        virtual size_t printTo  (Print & rPrinter) const = 0;
        virtual bool   readFrom (Stream & rStream) = 0;

        virtual void SetMaxBrightness (const uint8_t lKaxBrightness) = 0;
        virtual bool ApplyAnimation   (uint32_t       * pLedColors,
                                       const uint16_t   kLedCount,
                                       const uint32_t   kIterNum) = 0;

        virtual ELEDBorderAnimation GetType     (void) const = 0;
        virtual uint8_t             GetRawParam (void) const = 0;

    protected:
        volatile uint8_t    maxBrightness_;

        ELEDBorderAnimation type_;

    private:

};

class LedBorderBuilder
{
    public:
        static IColorAnimation * DeserializeAnimation (Stream & rStream);
        static ColorPattern *    DeserializePattern   (Stream & rStream);

    protected:

    private:
};

class LEDBorder
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        LEDBorder  (SystemState * pSystemState);
        ~LEDBorder (void);

        void Init (void);

        void Stop      (void);
        void Enable    (void);
        void Disable   (void);
        bool IsEnabled (void) const;

        void Update  (void);
        void Refresh (void);

        void IncreaseBrightness (void);
        void ReduceBrightness   (void);

        uint8_t GetBrightness (void) const;

        uint32_t * GetLEDArrayColors (void);

        std::vector<IColorAnimation*> * GetColorAnimations (void);
        ColorPattern *                  GetColorPattern    (void);

        void Lock(void);
        void Unlock(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void    SetPattern      (const ELEDBorderPattern        kPatternId,
                                 const SLEDBorderPatternParam * pkPatternParam);
        uint8_t AddAnimation    (const ELEDBorderAnimation        kAnimId,
                                 const SLEDBorderAnimationParam * pkParam);
        void    RemoveAnimation (const uint8_t kAnimIdx);
        void    ClearAnimations (void);

        void    SetBrightness (const uint8_t kBrightness);

        bool                            enabled_;
        uint8_t                         brightness_;
        uint32_t                        pLedsColors_[NUM_LEDS];

        std::vector<IColorAnimation*>   animations_;

        CRGB                            leds_[NUM_LEDS];
        Storage                       * pStore_;
        SystemState                   * pSystemState_;
        TaskHandle_t                    workerThread_;
        ColorPattern                  * pPattern_;
        SemaphoreHandle_t               driverLock_;
};


#endif /* #ifndef __DRIVERS_LEDBORDER_H_ */