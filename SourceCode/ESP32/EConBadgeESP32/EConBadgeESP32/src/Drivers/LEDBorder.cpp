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

#define BORDER_REFRESH_PERIOD 2
#define MIN_BRIGHTNESS        3
#define MIN_COLOR             1
#define MIN_GRAD_SIZE         1

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))

#define FIX_GRAD_SIZE(GRADSIZEFIX, LED_COUNT, GRADSIZE) {               \
    if(GRADSIZE < MIN_GRAD_SIZE)                                        \
    {                                                                   \
        GRADSIZEFIX = MIN_GRAD_SIZE;                                    \
    }                                                                   \
    else                                                                \
    {                                                                   \
        GRADSIZEFIX = MIN(LED_COUNT, GRADSIZE);                         \
    }                                                                   \
}

#define FIX_COLOR(COLORFIX, R, G, B, COLOR) {                           \
    R = (COLOR >> 16) & 0xFF;                                           \
    G = (COLOR >>  8) & 0xFF;                                           \
    B = COLOR & 0xFF;                                                   \
                                                                        \
    /* Check the minimal color value */                                 \
    if(R + G + B < MIN_COLOR)                                           \
    {                                                                   \
        R = MIN_COLOR;                                                  \
        G = MIN_COLOR;                                                  \
        B = MIN_COLOR;                                                  \
    }                                                                   \
                                                                        \
    COLORFIX = (R << 16) | (G << 8) | B;                                \
}

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
            gradSizeCount_ = 1;

            InitGradient(&startColor, &endColor, &gradSize);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            gradSizeCount_ = 2;

            uint32_t startColor[2] = {
                startColor0,
                startColor1
            };
            uint32_t endColor[2] = {
                endColor0,
                endColor1
            };
            uint16_t gradSize[2] = {
                gradSize0,
                gradSize1
            };

            InitGradient(startColor, endColor, gradSize);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint32_t startColor2, const uint32_t endColor2,
                        const uint16_t gradSize2,
                        const uint16_t ledCount) : ColorPattern(false, ledCount)
        {
            gradSizeCount_ = 3;

            uint32_t startColor[3] = {
                startColor0,
                startColor1,
                startColor2
            };
            uint32_t endColor[3] = {
                endColor0,
                endColor1,
                endColor2
            };
            uint16_t gradSize[3] = {
                gradSize0,
                gradSize1,
                gradSize2
            };

            InitGradient(startColor, endColor, gradSize);
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
            gradSizeCount_ = 4;

            uint32_t startColor[4] = {
                startColor0,
                startColor1,
                startColor2,
                startColor3
            };
            uint32_t endColor[4] = {
                endColor0,
                endColor1,
                endColor2,
                endColor3
            };
            uint16_t gradSize[4] = {
                gradSize0,
                gradSize1,
                gradSize2,
                gradSize3
            };

            InitGradient(startColor, endColor, gradSize);
        }

        virtual ~GradientPattern(void)
        {
            delete[] colors_;
            delete[] gradSizes_;
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
        void InitGradient(const uint32_t * startColor,
                          const uint32_t * endColor,
                          const uint16_t * gradSize)
        {
            uint32_t startColorFix;
            uint32_t endColorFix;
            uint16_t pathSize;
            int32_t  r;
            int32_t  g;
            int32_t  b;
            uint8_t  i;

            isApplied_ = false;
            colors_    = new CRGB[ledCount_];
            gradSizes_ = new uint16_t[gradSizeCount_];

            pathSize = ledCount_ / gradSizeCount_;

            fill_solid(colors_, ledCount_, CRGB::Black);
            for(i = 0; i < gradSizeCount_; ++i)
            {
                FIX_COLOR(startColorFix, r, g, b, startColor[i]);
                FIX_COLOR(endColorFix, r, g, b, endColor[i]);
                FIX_GRAD_SIZE(gradSizes_[i], pathSize, gradSize[i]);

                fill_gradient_RGB(&colors_[i * pathSize], gradSizes_[i],
                                  startColorFix, endColorFix);
            }
        }

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
            int32_t r;
            int32_t g;
            int32_t b;

            isApplied_ = false;

            FIX_COLOR(color_, r, g, b, color);
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
            rateDivider_ = rateDivider == 0 ? 1 : rateDivider;
        }

        ~TrailAnimation(void)
        {

        }

        void SetMaxBrightness(const uint8_t maxBrightness)
        {
            maxBrightness_ = maxBrightness;
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
            speedIncrease_  = speedIncrease == 0 ? 1 : speedIncrease;
            currBrightness_ = 0;
            increase_       = true;
        }

        ~BreathAnimation(void)
        {

        }

        void SetMaxBrightness(const uint8_t maxBrightness)
        {
            maxBrightness_ = maxBrightness;
        }

        void ApplyAnimation(uint32_t * ledColors,
                            const uint16_t ledCount,
                            const uint32_t iterNum)
        {
            if(iterNum % speedIncrease_ == 0)
            {
                if(increase_)
                {
                    if(currBrightness_ + 1 <= maxBrightness_)
                    {
                        ++currBrightness_;
                    }
                    else
                    {
                        currBrightness_ = maxBrightness_;
                        increase_ = false;
                    }
                }
                else
                {
                    if(currBrightness_ >= 1)
                    {
                        --currBrightness_;
                    }
                    else
                    {
                        currBrightness_ = 0;
                        increase_ = true;
                    }
                }
                FastLED.setBrightness(currBrightness_);
            }
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
SemaphoreHandle_t threadWork;

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
            if(uxSemaphoreGetCount(threadWork) == 1)
            {
                xSemaphoreTake(threadWork, portMAX_DELAY);
            }
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
            if(uxSemaphoreGetCount(threadWork) == 0)
            {
                xSemaphoreGive(threadWork);
            }
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
    brightness_  = 50;
}

CLEDB::~LEDBorder(void)
{
    uint32_t i;

    enabled_    = false;

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
    uint8_t i;

    /* Add LEDs and set init brightness */
    CLEDController &ctrl = FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_, NUM_LEDS);
    ctrl.setCorrection(TypicalLEDStrip);

    FastLED.setBrightness(brightness_);

    memset(ledsColors_, 0, sizeof(ledsColors_));

    enabled_ = false;
    pattern_ = new SingleColorPattern(0xFFFFFFFF, NUM_LEDS);

    threadWork = xSemaphoreCreateMutex();

    /* Launch worker thread */
    xTaskCreatePinnedToCore(WorkerRoutine, "LEDBorderWorker", 2048, this, 0,
                            &workerThread_, 0);
    vTaskSuspend(workerThread_);

    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(0);
    }
    FastLED.show();
}

void CLEDB::Enable(void)
{
    if(!enabled_)
    {
        LOG_DEBUG("Resuming LEDBorder thread\n");
        FastLED.setBrightness(brightness_);
        vTaskResume(workerThread_);
        enabled_      = true;
    }
}

void CLEDB::Disable(void)
{
    uint8_t i;

    if(enabled_)
    {
        enabled_ = false;

        /* Suspend working thread */
        while(uxSemaphoreGetCount(threadWork) == 0);
        LOG_DEBUG("Suspending LEDBorder thread\n");

        vTaskSuspend(workerThread_);

        /* Set brightness to 0 */
        FastLED.setBrightness(0);

        /* Set all LEDs to 0 */
        for(i = 0; i < NUM_LEDS; ++i)
        {
            leds_[i].setColorCode(0);
        }

        FastLED.clear(true);
        FastLED.show();
    }
}

bool CLEDB::IsEnabled(void) const
{
    return enabled_;
}

void CLEDB::Update(void)
{
    uint8_t          i;
    uint8_t          response[4];
    uint8_t          responseSize;
    ELEDBorderAction action;
    uint8_t          actionParam[COMMAND_DATA_SIZE];

    action = systemState_->ConsumeELEDBorderAction(actionParam);

    responseSize = 2;
    response[0]  = 'O';
    response[1]  = 'K';
    switch(action)
    {
        case ENABLE_LEDB_ACTION:
            Enable();
            break;

        case DISABLE_LEDB_ACTION:
            Disable();
            break;

        case ADD_ANIMATION_LEDB_ACTION:
            response[2] = AddAnimation((ELEDBorderAnimation)actionParam[0],
                                       (SLEDBorderAnimationParam*)&actionParam[1]);
            /* On errror update response */
            if(response[2] == 255)
            {
                response[0] = 'K';
                response[1] = 'O';
            }
            ++responseSize;
            break;

        case REMOVE_ANIMATION_LEDB_ACTION:
            RemoveAnimation(actionParam[0]);
            break;

        case SET_PATTERN_LEDB_ACTION:
            SetPattern((ELEDBorderColorPattern)actionParam[0],
                       (SLEDBorderColorPatternParam*)&actionParam[1]);
            break;

        case CLEAR_ANIMATION_LEDB_ACTION:
            ClearAnimations();
            break;

        case SET_BRIGHTNESS_LEDB_ACTION:
            brightness_ = actionParam[0];
            /* Setup minimal brightness */
            if(brightness_ < MIN_BRIGHTNESS)
            {
                brightness_ = MIN_BRIGHTNESS;
            }
            for(i = 0; i < animations_.size(); ++i)
            {
                animations_[i]->SetMaxBrightness(brightness_);
            }
            FastLED.setBrightness(brightness_);
            break;

        default:
            responseSize = 0;
            break;
    }

    if(responseSize != 0)
    {
        if(!systemState_->EnqueueResponse(response, responseSize))
        {
            LOG_ERROR("Could not send LEDBorder command response\n");
        }
    }
}

void CLEDB::Refresh(void)
{
    uint8_t i;

    /* Set all LEDs to previous state */
    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(ledsColors_[i]);
    }
    FastLED.show();
}

void CLEDB::IncreaseBrightness(void)
{
    uint8_t i;

    /* Setup minimal brightness */
    if(brightness_ < 255)
    {
        brightness_ = MIN(255, brightness_ + 10);
    }
    for(i = 0; i < animations_.size(); ++i)
    {
        animations_[i]->SetMaxBrightness(brightness_);
    }
    FastLED.setBrightness(brightness_);
}

void CLEDB::ReduceBrightness(void)
{
    uint8_t i;

    /* Setup minimal brightness */
    if(brightness_ > MIN_BRIGHTNESS)
    {
        brightness_ = MAX(MIN_BRIGHTNESS, brightness_ - 10);
    }
    for(i = 0; i < animations_.size(); ++i)
    {
        animations_[i]->SetMaxBrightness(brightness_);
    }
    FastLED.setBrightness(brightness_);
}

void CLEDB::SetPattern(const ELEDBorderColorPattern patternId,
                       const SLEDBorderColorPatternParam * patternParam)
{
    Lock();
    delete pattern_;

    switch(patternId)
    {
        case LED_COLOR_PATTERN_PLAIN:
            pattern_ = new SingleColorPattern(patternParam->plainColorCode, NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_1:
            pattern_ = new GradientPattern(patternParam->startColorCode[0],
                                           patternParam->endColorCode[0],
                                           patternParam->gradientSize[0],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_2:
            pattern_ = new GradientPattern(patternParam->startColorCode[0],
                                           patternParam->endColorCode[0],
                                           patternParam->gradientSize[0],
                                           patternParam->startColorCode[1],
                                           patternParam->endColorCode[1],
                                           patternParam->gradientSize[1],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_3:
            pattern_ = new GradientPattern(patternParam->startColorCode[0],
                                           patternParam->endColorCode[0],
                                           patternParam->gradientSize[0],
                                           patternParam->startColorCode[1],
                                           patternParam->endColorCode[1],
                                           patternParam->gradientSize[1],
                                           patternParam->startColorCode[2],
                                           patternParam->endColorCode[2],
                                           patternParam->gradientSize[2],
                                           NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_4:
            pattern_ = new GradientPattern(patternParam->startColorCode[0],
                                           patternParam->endColorCode[0],
                                           patternParam->gradientSize[0],
                                           patternParam->startColorCode[1],
                                           patternParam->endColorCode[1],
                                           patternParam->gradientSize[1],
                                           patternParam->startColorCode[2],
                                           patternParam->endColorCode[2],
                                           patternParam->gradientSize[2],
                                           patternParam->startColorCode[3],
                                           patternParam->endColorCode[3],
                                           patternParam->gradientSize[3],
                                           NUM_LEDS);
            break;
        default:
            /* White color basic */
            pattern_ = new SingleColorPattern(0x00FFFFFF, NUM_LEDS);
    }
    Unlock();
}

uint8_t CLEDB::AddAnimation(const ELEDBorderAnimation animId,
                            const SLEDBorderAnimationParam * param)
{
    IColorAnimation * animation;

    /* We limite the number of animations to 255 */
    if(animations_.size() < 255)
    {
        animation = nullptr;
        switch(animId)
        {
            case LED_COLOR_ANIM_TRAIL:
                animation = new TrailAnimation(param->rateDivider);
                break;
            case LED_COLOR_ANIM_BREATH:
                animation = new BreathAnimation(param->speedIncrease);
                break;
            default:
                break;
        }

        animation->SetMaxBrightness(brightness_);

        if(animation != nullptr)
        {
            Lock();
            animations_.push_back(animation);
            Unlock();
        }
        return animations_.size() - 1;
    }
    else
    {
        return 255;
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

uint32_t * CLEDB::GetLEDArrayColors(void)
{
    return ledsColors_;
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