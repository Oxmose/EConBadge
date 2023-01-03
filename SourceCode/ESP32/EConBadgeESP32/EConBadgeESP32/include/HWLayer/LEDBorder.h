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

#ifndef __HWLAYER_LEDBORDER_H_
#define __HWLAYER_LEDBORDER_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */
#include <vector>  /* std::vector */

#include <FastLED.h> /* Fast LED Service */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define LED_PIN     13
#define NUM_LEDS    72
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Hardware Layer Namespace
 * @details Hardware Layer Namespace used for definitions of hardware related
 * services.
 */
namespace nsHWL
{

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
 * CLASSES
 ******************************************************************************/

class CColorPattern
{
    public:
        CColorPattern(const bool isDynamic, const uint16_t ledCount);
        virtual ~CColorPattern(void);
        virtual bool IsDynamic(void) const;

        virtual void ApplyPattern(uint32_t * ledsColors) = 0;


    protected:
        uint16_t ledCount;

    private:
        bool     isDynamic;
};

class IColorAnimation
{
    public:
        virtual ~IColorAnimation(void){}
        virtual void ApplyAnimation(uint32_t * ledColors,
                                    const uint16_t ledCount,
                                    const uint32_t iterNum) = 0;

    protected:

    private:

};

class CLEDBorder
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        ~CLEDBorder(void);

        void Init(void);

        void Enable(void);
        void Disable(void);

        void Update(void);

        void SetPattern(void);
        void SetAnimation(void);

        CRGB * GetLEDArray(void);
        uint32_t * GetLEDArrayColors(void);

        bool IsEnabled(void) const;

        std::vector<IColorAnimation*> * GetColorAnimations(void);
        CColorPattern * GetColorPattern(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        CRGB     leds[NUM_LEDS];
        uint32_t ledsColors[NUM_LEDS];

        TaskHandle_t workerThread;

        std::vector<IColorAnimation*>   animations;
        CColorPattern                 * pattern;

        bool enabled = false;
};

} /* namespace nsHWL */

#endif /* #ifndef __HWLAYER_LEDBORDER_H_ */