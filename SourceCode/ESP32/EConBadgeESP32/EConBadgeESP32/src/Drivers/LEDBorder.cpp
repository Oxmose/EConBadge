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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>  /* String manipulation*/
#include <Types.h>   /* Defined Types */
#include <Logger.h>  /* Logger service */
#include <FastLED.h> /* Fast LED Service */

/* Header File */
#include <LEDBorder.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CLEDB LEDBorder
#define CPATTERN ColorPattern

#define BORDER_REFRESH_PERIOD 10

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

/**************************** PATTERN DEFINITIONS *****************************/
class GradientPattern : public ColorPattern
{
    public:
        GradientPattern(const uint32_t startColor, const uint32_t endColor,
                        const uint16_t gradSize,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            isApplied_     = false;
            gradSizeCount_ = 1;
            colors_        = new CRGB[ledCount];
            gradSizes_     = new uint16_t[gradSizeCount_];

            /* Create the palette */
            gradSizes_[0] = MIN(ledCount, gradSize);
            fill_solid(colors_, ledCount, CRGB::Black);
            fill_gradient_RGB(colors_, gradSizes_[0],
                              startColor, endColor);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            isApplied_     = false;
            gradSizeCount_ = 2;
            colors_        = new CRGB[ledCount];
            gradSizes_     = new uint16_t[gradSizeCount_];

            pathSize = ledCount / gradSizeCount_;

            /* Create the palette */
            gradSizes_[0] = MIN(pathSize, gradSize0);
            gradSizes_[1] = MIN(pathSize, gradSize1);
            fill_solid(colors_, ledCount, CRGB::Black);
            fill_gradient_RGB(colors_, gradSizes_[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&colors_[pathSize], gradSizes_[1],
                              startColor1, endColor1);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint32_t startColor2, const uint32_t endColor2,
                        const uint16_t gradSize2,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            isApplied_     = false;
            gradSizeCount_ = 3;
            colors_        = new CRGB[ledCount];
            gradSizes_     = new uint16_t[gradSizeCount_];

            pathSize = ledCount / gradSizeCount_;

            /* Create the palette */
            gradSizes_[0] = MIN(pathSize, gradSize0);
            gradSizes_[1] = MIN(pathSize, gradSize1);
            gradSizes_[2] = MIN(pathSize, gradSize2);
            fill_solid(colors_, ledCount, CRGB::Black);
            fill_gradient_RGB(colors_, gradSizes_[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&colors_[pathSize], gradSizes_[1],
                              startColor1, endColor1);
            fill_gradient_RGB(&colors_[pathSize * 2], gradSizes_[2],
                              startColor2, endColor2);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint32_t startColor2, const uint32_t endColor2,
                        const uint16_t gradSize2,
                        const uint32_t startColor3, const uint32_t endColor3,
                        const uint16_t gradSize3,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            isApplied_     = false;
            gradSizeCount_ = 4;
            colors_        = new CRGB[ledCount];
            gradSizes_     = new uint16_t[gradSizeCount_];

            pathSize = ledCount / gradSizeCount_;

            /* Create the palette */
            gradSizes_[0] = MIN(pathSize, gradSize0);
            gradSizes_[1] = MIN(pathSize, gradSize1);
            gradSizes_[2] = MIN(pathSize, gradSize2);
            gradSizes_[3] = MIN(pathSize, gradSize3);
            fill_solid(colors_, ledCount, CRGB::Black);
            fill_gradient_RGB(colors_, gradSizes_[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&colors_[pathSize], gradSizes_[1],
                              startColor1, endColor1);
            fill_gradient_RGB(&colors_[pathSize * 2], gradSizes_[2],
                              startColor2, endColor2);
            fill_gradient_RGB(&colors_[pathSize * 3], gradSizes_[3],
                              startColor3, endColor3);
        }

        virtual ~GradientPattern(void)
        {
            delete colors_;
            delete gradSizes_;
        }

        virtual void ApplyPattern(uint32_t * ledsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(!isApplied_)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < ledCount_; ++i)
                {
                    ledsColors[i] = colors_[i].r << 16 |
                                    colors_[i].g << 8 |
                                    colors_[i].b;
                }
                isApplied_  = true;
            }
        }

    private:
        bool       isApplied_;
        CRGB     * colors_;
        uint16_t * gradSizes_;
        uint8_t    gradSizeCount_;
};

class SingleColorPattern : public ColorPattern
{
    public:
        SingleColorPattern(const uint32_t color,
                           const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            color_     = color;
            isApplied_ = false;
        }

        virtual ~SingleColorPattern(void)
        {
        }

        virtual void ApplyPattern(uint32_t * ledsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(!isApplied_)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < ledCount_; ++i)
                {
                    ledsColors[i] = color_;
                }
                isApplied_ = true;
            }
        }

    private:
        bool     isApplied_;
        uint32_t color_;
};

/*************************** ANIMATION DEFINITIONS ****************************/
class TrailAnimation : public IColorAnimation
{
    public:
        TrailAnimation(const uint8_t rateDivider)
        {
            rateDivider_ = rateDivider;
        }

        ~TrailAnimation(void)
        {

        }

        void ApplyAnimation(uint32_t * ledColors,
                            const uint16_t ledCount,
                            const uint32_t iterNum)
        {
            uint16_t i;
            uint32_t saveLast;

            /* Divide the iternum */
            if(iterNum % rateDivider_ == 0)
            {
                saveLast = ledColors[0];
                /* Just set the next LED as the previous one */
                for(i = 1; i < ledCount; ++i)
                {
                    ledColors[i - 1] = ledColors[i];
                }
                ledColors[ledCount - 1] = saveLast;
            }
        }

    private:
        uint8_t rateDivider_;
};

class BreathAnimation : public IColorAnimation
{
    public:
        BreathAnimation(const uint8_t speedIncrease)
        {
            speedIncrease_  = speedIncrease;
            currBrightness_ = 0;
            increase_       = true;
        }

        ~BreathAnimation(void)
        {

        }

        void ApplyAnimation(uint32_t * ledColors,
                            const uint16_t ledCount,
                            const uint32_t iterNum)
        {
            if(increase_)
            {
                if(currBrightness_ + speedIncrease_ <= 255)
                {
                    currBrightness_ += speedIncrease_;
                }
                else
                {
                    currBrightness_ = 255;
                    increase_ = false;
                }
            }
            else
            {
                if(currBrightness_ >= speedIncrease_)
                {
                    currBrightness_ -= speedIncrease_;
                }
                else
                {
                    currBrightness_ = 0;
                    increase_ = true;
                }
            }
            FastLED.setBrightness(currBrightness_);
        }

    private:
        uint16_t currBrightness_;
        uint8_t  speedIncrease_;
        bool     increase_;

};

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/


/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

static void WorkerRoutine(void * args)
{
    LEDBorder                     * currBorderMgr;
    ColorPattern                  * pattern;
    std::vector<IColorAnimation*> * animations;

    uint32_t * ledArrayColors;
    uint32_t   iterNum;
    uint32_t   i;

    currBorderMgr  = (LEDBorder*)args;
    ledArrayColors = currBorderMgr->GetLEDArrayColors();

    LOG_DEBUG("Worker thread on core %d\n", xPortGetCoreID());

    iterNum = 0;
    while(1)
    {
        if(currBorderMgr->IsEnabled())
        {
            currBorderMgr->Lock();

            pattern    = currBorderMgr->GetColorPattern();
            animations = currBorderMgr->GetColorAnimations();

            if(pattern != nullptr)
            {
                pattern->ApplyPattern(ledArrayColors);
            }
            for(i = 0; i < animations->size(); ++i)
            {
                if(animations->at(i) != nullptr)
                {
                    animations->at(i)->ApplyAnimation(ledArrayColors,
                                                      NUM_LEDS,
                                                      iterNum);
                }
            }

            currBorderMgr->Unlock();

            currBorderMgr->Refresh();
            FastLED.delay(BORDER_REFRESH_PERIOD);
            ++iterNum;
        }
        else
        {
            delay(BORDER_REFRESH_PERIOD);
        }
    }
}

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

CLEDB::LEDBorder(SystemState * systemState)
{
    systemState_ = systemState;
}

CLEDB::~LEDBorder(void)
{
    uint32_t i;

    enabled_ = false;

    vTaskDelete(workerThread_);
    Unlock();

    if(pattern_ != nullptr)
    {
        delete pattern_;
    }
    for(i = 0; i < animations_.size(); ++i)
    {
        if(animations_[i] != nullptr)
        {
            delete animations_[i];
            animations_[i] = nullptr;
        }
        animations_.clear();
    }

}

void CLEDB::Init(void)
{
    /* Add LEDs and set init brightness */
    CLEDController &ctrl = FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_, NUM_LEDS);
    ctrl.setCorrection(TypicalLEDStrip);

    FastLED.setBrightness(255);

    memset(ledsColors_, 0, sizeof(ledsColors_));
    pattern_ = nullptr;

    /* Launch worker thread */
    xTaskCreatePinnedToCore(WorkerRoutine, "LEDBorderWorker", 2048, this, 0,
                            &workerThread_, 0);
    Disable();
}

void CLEDB::Enable(void)
{
    vTaskResume(workerThread_);

    Refresh();
    FastLED.show();

    enabled_ = true;
}

void CLEDB::Disable(void)
{
    uint8_t i;

    /* Set all LEDs to 0 */
    vTaskSuspend(workerThread_);
    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(0);
    }
    FastLED.show();


    enabled_ = false;
}

void CLEDB::Update(void)
{
    /* TODO: Update based on system state, check command */
}

void CLEDB::Refresh(void)
{
    uint8_t i;

    /* Set all LEDs to previous state */
    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(ledsColors_[i]);
    }
}

void CLEDB::SetBrightness(const uint8_t brightness)
{
    /* TODO: Set led brightness */
}

void CLEDB::SetPattern(const ELEDBorderColorPattern patternId,
                       const SLEDBorderColorPatternParam & patternParam)
{
    Lock();
    delete pattern_;

    switch(patternId)
    {
        case LED_COLOR_PATTERN_PLAIN:
            pattern_ = new SingleColorPattern(patternParam.plainColorCode, NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_1:
            pattern_ = new GradientPattern(patternParam.startColorCode[0],
                                           patternParam.endColorCode[0],
                                           patternParam.gradientSize[0],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_2:
            pattern_ = new GradientPattern(patternParam.startColorCode[0],
                                           patternParam.endColorCode[0],
                                           patternParam.gradientSize[0],
                                           patternParam.startColorCode[1],
                                           patternParam.endColorCode[1],
                                           patternParam.gradientSize[1],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_3:
            pattern_ = new GradientPattern(patternParam.startColorCode[0],
                                           patternParam.endColorCode[0],
                                           patternParam.gradientSize[0],
                                           patternParam.startColorCode[1],
                                           patternParam.endColorCode[1],
                                           patternParam.gradientSize[1],
                                           patternParam.startColorCode[2],
                                           patternParam.endColorCode[2],
                                           patternParam.gradientSize[2],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_4:
            pattern_ = new GradientPattern(patternParam.startColorCode[0],
                                           patternParam.endColorCode[0],
                                           patternParam.gradientSize[0],
                                           patternParam.startColorCode[1],
                                           patternParam.endColorCode[1],
                                           patternParam.gradientSize[1],
                                           patternParam.startColorCode[2],
                                           patternParam.endColorCode[2],
                                           patternParam.gradientSize[2],
                                           patternParam.startColorCode[3],
                                           patternParam.endColorCode[3],
                                           patternParam.gradientSize[3],
                                           NUM_LEDS);
            break;
        default:
            /* White color basic */
            pattern_ = new SingleColorPattern(0x00FFFFFF, NUM_LEDS);
    }
    Unlock();
}

void CLEDB::AddAnimation(const ELEDBorderAnimation animId,
                         const SLEDBorderAnimationParam & param)
{
    IColorAnimation * animation;

    animation = nullptr;
    switch(animId)
    {
        case LED_COLOR_ANIM_TRAIL:
            animation = new TrailAnimation(param.rateDivider);
            break;
        case LED_COLOR_ANIM_BREATH:
            animation = new BreathAnimation(param.speedIncrease);
            break;
        default:
            break;
    }

    if(animation != nullptr)
    {
        Lock();
        animations_.push_back(animation);
        Unlock();
    }

}

void CLEDB::RemoveAnimation(const uint8_t animIdx)
{
    Lock();
    if(animIdx < animations_.size())
    {
        delete animations_[animIdx];
        animations_.erase(animations_.begin() + animIdx);
    }
    Unlock();
}

void CLEDB::ClearAnimations(void)
{
    uint8_t i;

    Lock();

    /* Clear all animations */
    for(i = 0; i < animations_.size(); ++i)
    {
        delete animations_[i];
    }
    animations_.clear();

    Unlock();
}

void CLEDB::ClearPattern(void)
{
    Lock();

    delete pattern_;
    pattern_ = nullptr;

    Unlock();
}

CRGB * CLEDB::GetLEDArray(void)
{
    return leds_;
}

uint32_t * CLEDB::GetLEDArrayColors(void)
{
    return ledsColors_;
}

bool CLEDB::IsEnabled(void) const
{
    return enabled_;
}

std::vector<IColorAnimation*> * CLEDB::GetColorAnimations(void)
{
    return &animations_;
}

CPATTERN * CLEDB::GetColorPattern(void)
{
    return pattern_;
}

void CLEDB::Lock(void)
{
    driverLock_.lock();
}

void CLEDB::Unlock(void)
{
    driverLock_.unlock();
}

CPATTERN::ColorPattern(const bool isDynamic, const uint16_t ledCount)
{
    isDynamic_ = isDynamic;
    ledCount_  = ledCount;
}

CPATTERN::~ColorPattern(void)
{

}

bool CPATTERN::IsDynamic(void) const
{
    return isDynamic_;
}

#undef CLEDB
#undef CPATTERN