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
#include <Storage.h> /* Storage service */

/* Header File */
#include <LEDBorder.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CLEDB LEDBorder
#define CPATTERN ColorPattern
#define CBUILD LedBorderBuilder

#define BORDER_REFRESH_PERIOD 2
#define MIN_BRIGHTNESS        3
#define MIN_COLOR             1
#define MIN_GRAD_SIZE         1

#define LED_PIN     25
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

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
        GradientPattern(const ELEDBorderColorPattern type) :
            ColorPattern(0, type)
        {
            colors_    = nullptr;
            gradSizes_ = nullptr;
        }

        GradientPattern(const uint32_t startColor, const uint32_t endColor,
                        const uint16_t gradSize,
                        const uint16_t ledCount) :
            ColorPattern(ledCount,
                         ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_1)
        {
            gradSizeCount_ = 1;

            InitGradient(&startColor, &endColor, &gradSize);
        }

        GradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                        const uint16_t gradSize0,
                        const uint32_t startColor1, const uint32_t endColor1,
                        const uint16_t gradSize1,
                        const uint16_t ledCount) :
            ColorPattern(ledCount,
                         ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_2)
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
                        const uint16_t ledCount) :
            ColorPattern(ledCount,
                         ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_3)
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
                        const uint16_t ledCount) :
            ColorPattern(ledCount,
                         ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_4)
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

        virtual size_t printTo(Print& p) const
        {
            uint16_t i;
            uint8_t  j;
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = p.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border pattern type\n");
                return 0;
            }

            /* Print the led count */
            printSize   = p.write((uint8_t*)&ledCount_, sizeof(ledCount_));
            totalPrint += printSize;
            if(printSize != sizeof(ledCount_))
            {
                LOG_ERROR("Could not write LED border led count\n");
                return 0;
            }

            /* Print the gradient size count state */
            printSize   = p.write((uint8_t*)&gradSizeCount_, sizeof(gradSizeCount_));
            totalPrint += printSize;
            if(printSize != sizeof(gradSizeCount_))
            {
                LOG_ERROR("Could not write LED border grad size count\n");
                return 0;
            }

            /* Print the grad sizes */
            printSize   = p.write((uint8_t*)gradSizes_, sizeof(gradSizes_[0]) * gradSizeCount_);
            totalPrint += printSize;
            if(printSize != sizeof(gradSizes_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not write LED border grad size\n");
                return 0;
            }

            /* Print the colors info */
            for(i = 0; i < ledCount_; ++i)
            {
                printSize   = p.write((uint8_t*)&colors_[i].raw, sizeof(colors_[i].raw));
                totalPrint += printSize;
                if(printSize != sizeof(colors_[i].raw))
                {
                    LOG_ERROR("Could not write LED color\n");
                    return 0;
                }
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream& s)
        {
            uint16_t i;

            /* Get the led count */
            if(s.readBytes((uint8_t*)&ledCount_, sizeof(ledCount_)) != sizeof(ledCount_))
            {
                LOG_ERROR("Could not read led count\n");
                return false;
            }
            delete[] colors_;
            colors_ = new CRGB[ledCount_];

            /* Get the gradient size count state */
            if(s.readBytes((uint8_t*)&gradSizeCount_, sizeof(gradSizeCount_)) != sizeof(gradSizeCount_))
            {
                LOG_ERROR("Could not read gradient size count\n");
                return false;
            }
            delete[] gradSizes_;
            gradSizes_ = new uint16_t[gradSizeCount_];

            /* Print the grad sizes */
            if(s.readBytes((uint8_t*)&gradSizes_[i],
                           sizeof(gradSizes_[0]) * gradSizeCount_) !=
                           sizeof(gradSizes_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not read gradient size\n");
                return false;
            }

            /* Print the colors info */
            for(i = 0; i < ledCount_; ++i)
            {
                if(s.readBytes((uint8_t*)&colors_[i].raw, sizeof(colors_[i].raw)) != sizeof(colors_[i].raw))
                {
                    LOG_ERROR("Could not read LED color\n");
                    return false;
                }
            }

            isApplied_ = false;
            return true;
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
                           const uint16_t ledCount) :
            ColorPattern(ledCount,
                         ELEDBorderColorPattern::LED_COLOR_PATTERN_PLAIN)
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

        virtual size_t printTo(Print& p) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = p.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border pattern type\n");
                return 0;
            }

            /* Print the led count */
            printSize   = p.write((uint8_t*)&ledCount_, sizeof(ledCount_));
            totalPrint += printSize;
            if(printSize != sizeof(ledCount_))
            {
                LOG_ERROR("Could not write LED border led count\n");
                return 0;
            }

            /* Print color */
            printSize   = p.write((uint8_t*)&color_, sizeof(color_));
            totalPrint += printSize;
            if(printSize != sizeof(color_))
            {
                LOG_ERROR("Could not write LED border led color\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream& s)
        {
            /* Get the led count */
            if(s.readBytes((uint8_t*)&ledCount_, sizeof(ledCount_)) != sizeof(ledCount_))
            {
                LOG_ERROR("Could not read led count\n");
                return false;
            }

            /* Get the gradient size count state */
            if(s.readBytes((uint8_t*)&color_, sizeof(color_)) != sizeof(color_))
            {
                LOG_ERROR("Could not read led color\n");
                return false;
            }

            isApplied_ = false;
            return true;
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
            type_        = ELEDBorderAnimation::LED_COLOR_ANIM_TRAIL;
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

        virtual size_t printTo(Print& p) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = p.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border animation type\n");
                return 0;
            }

            /* Print the max brightness  */
            printSize   = p.write((uint8_t*)&maxBrightness_, sizeof(maxBrightness_));
            totalPrint += printSize;
            if(printSize != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not write LED border max brightness\n");
                return 0;
            }

            /* Print rate divider  */
            printSize   = p.write((uint8_t*)&rateDivider_, sizeof(rateDivider_));
            totalPrint += printSize;
            if(printSize != sizeof(rateDivider_))
            {
                LOG_ERROR("Could not write LED border rate devider\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream& s)
        {
            /* Get the max brightness */
            if(s.readBytes((uint8_t*)&maxBrightness_, sizeof(maxBrightness_)) != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not read max brightness\n");
                return false;
            }

            /* Get the rate divider */
            if(s.readBytes((uint8_t*)&rateDivider_, sizeof(rateDivider_)) != sizeof(rateDivider_))
            {
                LOG_ERROR("Could not read rate divider\n");
                return false;
            }

            return true;
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
            type_           = ELEDBorderAnimation::LED_COLOR_ANIM_BREATH;
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

        virtual size_t printTo(Print& p) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = p.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border animation type\n");
                return 0;
            }

            /* Print the max brightness  */
            printSize   = p.write((uint8_t*)&maxBrightness_, sizeof(maxBrightness_));
            totalPrint += printSize;
            if(printSize != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not write LED border max brightness\n");
                return 0;
            }

            /* Print current brightness  */
            printSize   = p.write((uint8_t*)&currBrightness_, sizeof(currBrightness_));
            totalPrint += printSize;
            if(printSize != sizeof(currBrightness_))
            {
                LOG_ERROR("Could not write LED border current brightness\n");
            }

            /* Print speed increase */
            p.print(speedIncrease_);
            printSize   = p.write((uint8_t*)&speedIncrease_, sizeof(speedIncrease_));
            totalPrint += printSize;
            if(printSize != sizeof(speedIncrease_))
            {
                LOG_ERROR("Could not write LED border speed increase\n");
            }

            /* Print increase state */
            p.print(increase_);
            printSize   = p.write((uint8_t*)&increase_, sizeof(increase_));
            totalPrint += printSize;
            if(printSize != sizeof(increase_))
            {
                LOG_ERROR("Could not write LED border increase state\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream& s)
        {
            /* Get the max brightness */
            if(s.readBytes((uint8_t*)&maxBrightness_, sizeof(maxBrightness_)) != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not read max brightness\n");
                return false;
            }

            /* Get the rate divider */
            if(s.readBytes((uint8_t*)&currBrightness_, sizeof(currBrightness_)) != sizeof(currBrightness_))
            {
                LOG_ERROR("Could not read current brightness\n");
                return false;
            }

            /* Get the speed increase */
            if(s.readBytes((uint8_t*)&speedIncrease_, sizeof(speedIncrease_)) != sizeof(speedIncrease_))
            {
                LOG_ERROR("Could not read speed increase\n");
                return false;
            }

            /* Get the increase state */
            if(s.readBytes((uint8_t*)&increase_, sizeof(increase_)) != sizeof(increase_))
            {
                LOG_ERROR("Could not read increase state\n");
                return false;
            }

            return true;
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

static void WorkerRoutine(void * args);

/*******************************************************************************
 * FUNCTIONS
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
            FastLED.setBrightness(currBorderMgr->GetBrightness());

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
 * CLASS METHODS
 ******************************************************************************/

IColorAnimation* CBUILD::DeserializeAnimation(Stream& s)
{
    IColorAnimation*    newAnim;
    ELEDBorderAnimation type;

    newAnim = nullptr;

    /* Read the animation type */
    if(s.readBytes((uint8_t*)&type, sizeof(type)) == sizeof(type))
    {
        switch(type)
        {
            case ELEDBorderAnimation::LED_COLOR_ANIM_BREATH:
                newAnim = new BreathAnimation(1);
                break;
            case ELEDBorderAnimation::LED_COLOR_ANIM_TRAIL:
                newAnim = new TrailAnimation(1);
                break;
            default:
                LOG_ERROR("Incorrect read animation type %d\n", type);
                break;
        }
        if(newAnim != nullptr)
        {
            if(!newAnim->readFrom(s))
            {
                LOG_ERROR("Could not load new animation\n");
                delete newAnim;
                newAnim = nullptr;
            }
        }
        else
        {
            LOG_ERROR("Could not allocate memory for new animation\n");
        }
    }
    else
    {
        LOG_ERROR("Could not read the animation type\n");
    }

    return newAnim;
}

ColorPattern* CBUILD::DeserializePattern(Stream& s)
{
    ColorPattern*          newPattern;
    ELEDBorderColorPattern type;

    newPattern = nullptr;

    /* Read the animation type */
    if(s.readBytes((uint8_t*)&type, sizeof(type)) == sizeof(type))
    {
        switch(type)
        {
            case ELEDBorderColorPattern::LED_COLOR_PATTERN_PLAIN:
                newPattern = new SingleColorPattern(1, 1);
                break;
            case ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_1:
            case ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_2:
            case ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_3:
            case ELEDBorderColorPattern::LED_COLOR_PATTERN_GRADIENT_4:
                newPattern = new GradientPattern(type);
                break;
            default:
                LOG_ERROR("Incorrect read pattern type %d\n", type);
                break;
        }
        if(newPattern != nullptr)
        {
            if(!newPattern->readFrom(s))
            {
                LOG_ERROR("Could not load new pattern\n");
                delete newPattern;
                newPattern = nullptr;
            }
        }
        else
        {
            LOG_ERROR("Could not allocate memory for new pattern\n");
        }
    }
    else
    {
        LOG_ERROR("Could not read the pattern type\n");
    }

    return newPattern;
}

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
    uint8_t   i;
    Storage * store;
    bool      oldBrightness;

    /* Add LEDs and set init brightness */
    CLEDController &ctrl = FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_, NUM_LEDS);
    ctrl.setCorrection(TypicalLEDStrip);

    memset(ledsColors_, 0, sizeof(ledsColors_));

    pattern_ = nullptr;

    /* Try to load from saved state */
    oldBrightness = brightness_;
    store = Storage::GetInstance();
    if(!store->LoadLEDBorderSettings(enabled_,
                                     brightness_,
                                     &pattern_,
                                     animations_))
    {
        /* Not able to load, set init values */
        LOG_DEBUG("Loading LED Border with initial values\n")

        brightness_ = oldBrightness;
        FastLED.setBrightness(brightness_);
        enabled_ = false;

        delete pattern_;
        pattern_ = new SingleColorPattern(0xFFFFFFFF, NUM_LEDS);
        ClearAnimations();

        /* Save current state */
        store->SaveLEDBorderEnabled(enabled_);
        store->SaveLEDBorderBrightness(brightness_);
        store->SaveLEDBorderPattern(pattern_);
        store->RemoveLEDBorderAnimations();
    }
    else
    {
        LOG_DEBUG("Loaded LED Border with saved values\n")
    }

    threadWork = xSemaphoreCreateMutex();

    /* Launch worker thread */
    xTaskCreatePinnedToCore(WorkerRoutine, "LEDBorderWorker", 1024, this, 0,
                            &workerThread_, 0);
    vTaskSuspend(workerThread_);

    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(0);
    }
    FastLED.show();

    if(enabled_)
    {
        enabled_ = false;
        Enable();
    }
}

void CLEDB::Enable(void)
{
    if(!enabled_)
    {
        LOG_DEBUG("Resuming LEDBorder thread\n");
        FastLED.setBrightness(brightness_);
        vTaskResume(workerThread_);
        enabled_ = true;

        if(!Storage::GetInstance()->SaveLEDBorderEnabled(enabled_))
        {
            LOG_ERROR("Could not save the LED Border state\n");
        }
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

        if(!Storage::GetInstance()->SaveLEDBorderEnabled(enabled_))
        {
            LOG_ERROR("Could not save the LED Border state\n");
        }
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
            SetBrightness(actionParam[0]);
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
    /* Setup minimal brightness */
    if(brightness_ < 255)
    {
        brightness_ = MIN(255, brightness_ + 10);
    }

    SetBrightness(brightness_);
}

void CLEDB::ReduceBrightness(void)
{
    /* Setup minimal brightness */
    if(brightness_ > MIN_BRIGHTNESS)
    {
        brightness_ = MAX(MIN_BRIGHTNESS, brightness_ - 10);
    }

    SetBrightness(brightness_);
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
    if(!Storage::GetInstance()->SaveLEDBorderPattern(pattern_))
    {
        LOG_ERROR("Could not save LED Border pattern.\n");
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

        if(animation != nullptr)
        {
            animation->SetMaxBrightness(brightness_);
            Lock();
            animations_.push_back(animation);
            Unlock();
            if(!Storage::GetInstance()->SaveLEDBorderAnimation(animation, animations_.size() - 1))
            {
                LOG_ERROR("Could not save LED Border animation %d.\n", animations_.size() - 1);
            }
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
    if(!Storage::GetInstance()->RemoveLEDBorderAnimation(animIdx))
    {
        LOG_ERROR("Could not remove animation %d.\n", animIdx);
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
    if(!Storage::GetInstance()->RemoveLEDBorderAnimations())
    {
        LOG_ERROR("Could not remove animations.\n");
    }
    animations_.clear();

    Unlock();
}

uint8_t CLEDB::GetBrightness(void) const
{
    return brightness_;
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

void CLEDB::SetBrightness(const uint8_t brightness)
{
    uint8_t   i;
    Storage * store;

    store = Storage::GetInstance();

    if(brightness_ < MIN_BRIGHTNESS)
    {
        brightness_ = MIN_BRIGHTNESS;
    }

    for(i = 0; i < animations_.size(); ++i)
    {
        animations_[i]->SetMaxBrightness(brightness_);
        store->SaveLEDBorderAnimation(animations_[i], i);
    }
    FastLED.setBrightness(brightness_);
    if(!store->SaveLEDBorderBrightness(brightness_))
    {
        LOG_ERROR("Could not save the LED Border brightness\n");
    }
}

CPATTERN::ColorPattern(const uint16_t ledCount,
                       const ELEDBorderColorPattern type)
{
    ledCount_  = ledCount;
    type_      = type;
}

CPATTERN::~ColorPattern(void)
{

}

#undef CLEDB
#undef CPATTERN
#undef CBUILD