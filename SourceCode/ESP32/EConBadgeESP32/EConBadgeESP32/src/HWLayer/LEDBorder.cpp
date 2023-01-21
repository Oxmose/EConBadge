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

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CLEDB nsHWL::CLEDBorder
#define CPATTERN nsHWL::CColorPattern

#define BORDER_REFRESH_PERIOD 10

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/* None */

namespace nsHWL
{

/**************************** PATTERN DEFINITIONS *****************************/
class CGradientPattern : public CColorPattern
{
    public:
        CGradientPattern(const uint32_t startColor, const uint32_t endColor,
                         const uint16_t gradSize,
                         const uint16_t ledCount) :
            CColorPattern(false, ledCount)
        {
            this->isApplied     = false;
            this->gradSizeCount = 1;
            this->colors        = new CRGB[ledCount];
            this->gradSizes     = new uint16_t[this->gradSizeCount];

            /* Create the palette */
            this->gradSizes[0] = MIN(ledCount, gradSize);
            fill_solid(this->colors, ledCount, CRGB::Black);
            fill_gradient_RGB(this->colors, this->gradSizes[0],
                              startColor, endColor);
        }

        CGradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                         const uint16_t gradSize0,
                         const uint32_t startColor1, const uint32_t endColor1,
                         const uint16_t gradSize1,
                         const uint16_t ledCount) :
            CColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            this->isApplied     = false;
            this->gradSizeCount = 2;
            this->colors        = new CRGB[ledCount];
            this->gradSizes     = new uint16_t[this->gradSizeCount];

            pathSize = ledCount / this->gradSizeCount;

            /* Create the palette */
            this->gradSizes[0] = MIN(pathSize, gradSize0);
            this->gradSizes[1] = MIN(pathSize, gradSize1);
            fill_solid(this->colors, ledCount, CRGB::Black);
            fill_gradient_RGB(this->colors, this->gradSizes[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&this->colors[pathSize], this->gradSizes[1],
                              startColor1, endColor1);
        }

        CGradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                         const uint16_t gradSize0,
                         const uint32_t startColor1, const uint32_t endColor1,
                         const uint16_t gradSize1,
                         const uint32_t startColor2, const uint32_t endColor2,
                         const uint16_t gradSize2,
                         const uint16_t ledCount) :
            CColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            this->isApplied     = false;
            this->gradSizeCount = 3;
            this->colors        = new CRGB[ledCount];
            this->gradSizes     = new uint16_t[this->gradSizeCount];

            pathSize = ledCount / this->gradSizeCount;

            /* Create the palette */
            this->gradSizes[0] = MIN(pathSize, gradSize0);
            this->gradSizes[1] = MIN(pathSize, gradSize1);
            this->gradSizes[2] = MIN(pathSize, gradSize2);
            fill_solid(this->colors, ledCount, CRGB::Black);
            fill_gradient_RGB(this->colors, this->gradSizes[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&this->colors[pathSize], this->gradSizes[1],
                              startColor1, endColor1);
            fill_gradient_RGB(&this->colors[pathSize * 2], this->gradSizes[2],
                              startColor2, endColor2);
        }

        CGradientPattern(const uint32_t startColor0, const uint32_t endColor0,
                         const uint16_t gradSize0,
                         const uint32_t startColor1, const uint32_t endColor1,
                         const uint16_t gradSize1,
                         const uint32_t startColor2, const uint32_t endColor2,
                         const uint16_t gradSize2,
                         const uint32_t startColor3, const uint32_t endColor3,
                         const uint16_t gradSize3,
                         const uint16_t ledCount) :
            CColorPattern(false, ledCount)
        {
            uint16_t pathSize;

            this->isApplied     = false;
            this->gradSizeCount = 4;
            this->colors        = new CRGB[ledCount];
            this->gradSizes     = new uint16_t[this->gradSizeCount];

            pathSize = ledCount / this->gradSizeCount;

            /* Create the palette */
            this->gradSizes[0] = MIN(pathSize, gradSize0);
            this->gradSizes[1] = MIN(pathSize, gradSize1);
            this->gradSizes[2] = MIN(pathSize, gradSize2);
            this->gradSizes[3] = MIN(pathSize, gradSize3);
            fill_solid(this->colors, ledCount, CRGB::Black);
            fill_gradient_RGB(this->colors, this->gradSizes[0],
                              startColor0, endColor0);
            fill_gradient_RGB(&this->colors[pathSize], this->gradSizes[1],
                              startColor1, endColor1);
            fill_gradient_RGB(&this->colors[pathSize * 2], this->gradSizes[2],
                              startColor2, endColor2);
            fill_gradient_RGB(&this->colors[pathSize * 3], this->gradSizes[3],
                              startColor3, endColor3);
        }

        virtual ~CGradientPattern(void)
        {
            delete this->colors;
            delete this->gradSizes;
        }

        virtual void ApplyPattern(uint32_t * ledsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(!this->isApplied)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < this->ledCount; ++i)
                {
                    ledsColors[i] = this->colors[i].r << 16 |
                                    this->colors[i].g << 8 |
                                    this->colors[i].b;
                }
                this->isApplied  = true;
            }
        }

    private:
        bool       isApplied;
        CRGB     * colors;
        uint16_t * gradSizes;
        uint8_t    gradSizeCount;
};

class CSingleColorPattern : public CColorPattern
{
    public:
        CSingleColorPattern(const uint32_t color,
                            const uint16_t ledCount) :
            CColorPattern(false, ledCount)
        {
            this->color     = color;
            this->isApplied = false;
        }

        virtual ~CSingleColorPattern(void)
        {
        }

        virtual void ApplyPattern(uint32_t * ledsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(!this->isApplied)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < this->ledCount; ++i)
                {
                    ledsColors[i] = this->color;
                }
                this->isApplied = true;
            }
        }

    private:
        bool     isApplied;
        uint32_t color;
};

/*************************** ANIMATION DEFINITIONS ****************************/
class CTrailAnimation : public IColorAnimation
{
    public:
        CTrailAnimation(const uint8_t rateDivider)
        {
            this->rateDivider = rateDivider;
        }

        ~CTrailAnimation(void)
        {

        }

        void ApplyAnimation(uint32_t * ledColors,
                            const uint16_t ledCount,
                            const uint32_t iterNum)
        {
            uint16_t i;
            uint32_t saveLast;

            /* Divide the iternum */
            if(iterNum % this->rateDivider == 0)
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
        uint8_t rateDivider;
};

class CBreathAnimation : public IColorAnimation
{
    public:
        CBreathAnimation(const uint8_t speedIncrease)
        {
            this->speedIncrease  = speedIncrease;
            this->currBrightness = 0;
            this->increase       = true;
        }

        ~CBreathAnimation(void)
        {

        }

        void ApplyAnimation(uint32_t * ledColors, const uint16_t ledCount,
                            const uint32_t iterNum)
        {
            if(this->increase)
            {
                if(this->currBrightness + this->speedIncrease <= 255)
                {
                    this->currBrightness += this->speedIncrease;
                }
                else
                {
                    this->currBrightness = 255;
                    this->increase = false;
                }
            }
            else
            {
                if(this->currBrightness >= this->speedIncrease)
                {
                    this->currBrightness -= this->speedIncrease;
                }
                else
                {
                    this->currBrightness = 0;
                    this->increase = true;
                }
            }
            FastLED.setBrightness(this->currBrightness);
        }

    private:
        uint16_t currBrightness;
        uint8_t  speedIncrease;
        bool     increase;

};

}

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
    nsHWL::CLEDBorder                    * currBorderMgr;
    nsHWL::CColorPattern                 * pattern;
    std::vector<nsHWL::IColorAnimation*> * animations;

    uint32_t * ledArrayColors;
    uint32_t   iterNum;
    uint32_t   i;

    currBorderMgr  = (nsHWL::CLEDBorder*)args;
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

            currBorderMgr->Update();
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

CLEDB::~CLEDBorder(void)
{
    uint32_t i;

    vTaskDelete(this->workerThread);
    Unlock();

    if(this->pattern != nullptr)
    {
        delete this->pattern;
    }
    for(i = 0; i < this->animations.size(); ++i)
    {
        if(this->animations[i] != nullptr)
        {
            delete this->animations[i];
            this->animations[i] = nullptr;
        }
        this->animations.clear();
    }

}

void CLEDB::Init(void)
{
    /* Add LEDs and set init brightness */
    CLEDController &ctrl = FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    ctrl.setCorrection(TypicalLEDStrip);

    FastLED.setBrightness(255);

    memset(this->ledsColors, 0, sizeof(this->ledsColors));
    this->pattern = nullptr;


    this->pattern = new CGradientPattern(0x000000FF, 0x00000000, NUM_LEDS / 2, NUM_LEDS);
    this->animations.push_back(new CTrailAnimation(2));

    /* Launch worker thread */
    this->enabled = true;
    Update();
    FastLED.show();
    xTaskCreatePinnedToCore(WorkerRoutine, "LEDBorderWorker", 2048, this, 0,
                            &this->workerThread, 0);
}

void CLEDB::Enable(void)
{
    vTaskResume(this->workerThread);

    Update();
    FastLED.show();

    this->enabled = true;
}

void CLEDB::Disable(void)
{
    uint8_t i;

    /* Set all LEDs to 0 */
    vTaskSuspend(this->workerThread);
    for(i = 0; i < NUM_LEDS; ++i)
    {
        this->leds[i].setColorCode(0);
    }
    FastLED.show();


    this->enabled = false;
}

void CLEDB::Update(void)
{
    uint8_t i;

    /* Set all LEDs to previous state */
    for(i = 0; i < NUM_LEDS; ++i)
    {
        this->leds[i].setColorCode(ledsColors[i]);
    }
}

void CLEDB::SetPattern(const ELEDBorderColorPattern patternId,
                       const SLEDBorderColorPatternParam & patternParam)
{
    Lock();
    delete this->pattern;

    switch(patternId)
    {
        case LED_COLOR_PATTERN_PLAIN:
            this->pattern = new CSingleColorPattern(patternParam.plainColorCode, NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_1:
            this->pattern = new CGradientPattern(patternParam.startColorCode[0],
                                                 patternParam.endColorCode[0],
                                                 patternParam.gradientSize[0],
                                                 NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_2:
            this->pattern = new CGradientPattern(patternParam.startColorCode[0],
                                                 patternParam.endColorCode[0],
                                                 patternParam.gradientSize[0],
                                                 patternParam.startColorCode[1],
                                                 patternParam.endColorCode[1],
                                                 patternParam.gradientSize[1],
                                                 NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_3:
            this->pattern = new CGradientPattern(patternParam.startColorCode[0],
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
            this->pattern = new CGradientPattern(patternParam.startColorCode[0],
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
            this->pattern = new CSingleColorPattern(0x00FFFFFF, NUM_LEDS);
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
            animation = new CTrailAnimation(param.rateDivider);
            break;
        case LED_COLOR_ANIM_BREATH:
            animation = new CBreathAnimation(param.speedIncrease);
            break;
        default:
            break;
    }

    if(animation != nullptr)
    {
        Lock();
        this->animations.push_back(animation);
        Unlock();
    }

}

void CLEDB::RemoveAnimation(const uint8_t animIdx)
{
    Lock();
    if(animIdx < this->animations.size())
    {
        delete this->animations[animIdx];
        this->animations.erase(this->animations.begin() + animIdx);
    }
    Unlock();
}

void CLEDB::ClearAnimations(void)
{
    uint8_t i;

    Lock();

    /* Clear all animations */
    for(i = 0; i < this->animations.size(); ++i)
    {
        delete this->animations[i];
    }
    this->animations.clear();

    Unlock();
}

void CLEDB::ClearPattern(void)
{
    Lock();

    delete this->pattern;
    this->pattern = nullptr;

    Unlock();
}

CRGB * CLEDB::GetLEDArray(void)
{
    return this->leds;
}

uint32_t * CLEDB::GetLEDArrayColors(void)
{
    return this->ledsColors;
}

bool CLEDB::IsEnabled(void) const
{
    return this->enabled;
}

std::vector<nsHWL::IColorAnimation*> * CLEDB::GetColorAnimations(void)
{
    return &this->animations;
}

CPATTERN * CLEDB::GetColorPattern(void)
{
    return this->pattern;
}

void CLEDB::Lock(void)
{
    this->driverLock.lock();
}

void CLEDB::Unlock(void)
{
    this->driverLock.unlock();
}

CPATTERN::CColorPattern(const bool isDynamic, const uint16_t ledCount)
{
    this->isDynamic = isDynamic;
    this->ledCount  = ledCount;
}

CPATTERN::~CColorPattern(void)
{

}

bool CPATTERN::IsDynamic(void) const
{
    return this->isDynamic;
}

#undef CLEDB