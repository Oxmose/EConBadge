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
#include <cstring>   /* String manipulation*/
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
#define CLEDB    LEDBorder
#define CPATTERN ColorPattern
#define CBUILD   LedBorderBuilder

#define BORDER_REFRESH_PERIOD 5
#define MIN_BRIGHTNESS        3
#define MIN_COLOR             5
#define MIN_GRAD_SIZE         5

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
        explicit GradientPattern(const ELEDBorderPattern kType)
        : ColorPattern(0, kType)
        {
            pColors_      = nullptr;
            pGradSizes_   = nullptr;
            pStartColors_ = nullptr;
            pEndColors_   = nullptr;
        }

        GradientPattern(const uint32_t kStartColor,
                        const uint32_t kEndColor,
                        const uint16_t kGradSize,
                        const uint16_t kLedCount)
        : ColorPattern(kLedCount,
                       ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_1)
        {
            gradSizeCount_ = 1;

            InitGradient(&kStartColor, &kEndColor, &kGradSize);
        }

        GradientPattern(const uint32_t kStartColor0,
                        const uint32_t kEndColor0,
                        const uint16_t kGradSize0,
                        const uint32_t kStartColor1,
                        const uint32_t kEndColor1,
                        const uint16_t kGradSize1,
                        const uint16_t kLedCount)
        : ColorPattern(kLedCount,
                       ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_2)
        {
            gradSizeCount_ = 2;

            const uint32_t pkStartColor[2] = {
                kStartColor0,
                kStartColor1
            };
            const uint32_t pkEndColor[2] = {
                kEndColor0,
                kEndColor1
            };
            const uint16_t pkGradSize[2] = {
                kGradSize0,
                kGradSize1
            };

            InitGradient(pkStartColor, pkEndColor, pkGradSize);
        }

        GradientPattern(const uint32_t kStartColor0,
                        const uint32_t kEndColor0,
                        const uint16_t kGradSize0,
                        const uint32_t kStartColor1,
                        const uint32_t kEndColor1,
                        const uint16_t kGradSize1,
                        const uint32_t kStartColor2,
                        const uint32_t kEndColor2,
                        const uint16_t kGradSize2,
                        const uint16_t kLedCount)
        : ColorPattern(kLedCount,
                       ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_3)
        {
            gradSizeCount_ = 3;

            const uint32_t pkStartColor[3] = {
                kStartColor0,
                kStartColor1,
                kStartColor2
            };
            const uint32_t pkEndColor[3] = {
                kEndColor0,
                kEndColor1,
                kEndColor2
            };
            const uint16_t pkGradSize[3] = {
                kGradSize0,
                kGradSize1,
                kGradSize2
            };

            InitGradient(pkStartColor, pkEndColor, pkGradSize);
        }

        GradientPattern(const uint32_t kStartColor0,
                        const uint32_t kEndColor0,
                        const uint16_t kGradSize0,
                        const uint32_t kStartColor1,
                        const uint32_t kEndColor1,
                        const uint16_t kGradSize1,
                        const uint32_t kStartColor2,
                        const uint32_t kEndColor2,
                        const uint16_t kGradSize2,
                        const uint32_t kStartColor3,
                        const uint32_t kEndColor3,
                        const uint16_t kGradSize3,
                        const uint16_t kLedCount)
        : ColorPattern(kLedCount,
                       ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_4)
        {
            gradSizeCount_ = 4;

            const uint32_t pkStartColor[4] = {
                kStartColor0,
                kStartColor1,
                kStartColor2,
                kStartColor3
            };
            const uint32_t pkEndColor[4] = {
                kEndColor0,
                kEndColor1,
                kEndColor2,
                kEndColor3
            };
            const uint16_t pkGradSize[4] = {
                kGradSize0,
                kGradSize1,
                kGradSize2,
                kGradSize3
            };

            InitGradient(pkStartColor, pkEndColor, pkGradSize);
        }

        virtual ~GradientPattern(void)
        {
            delete[] pColors_;
            delete[] pGradSizes_;
            delete[] pStartColors_;
            delete[] pEndColors_;
        }

        virtual void ApplyPattern(uint32_t * pLedsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(!isApplied_)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < ledCount_; ++i)
                {
                    pLedsColors[i] = pColors_[i].r << 16 |
                                     pColors_[i].g << 8 |
                                     pColors_[i].b;
                }
                isApplied_  = true;
            }
        }

        virtual size_t printTo(Print & rPrinter) const
        {
            uint16_t i;
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = rPrinter.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border pattern type\n");
                return 0;
            }

            /* Print the led count */
            printSize   = rPrinter.write((uint8_t*)&ledCount_,
                                         sizeof(ledCount_));
            totalPrint += printSize;
            if(printSize != sizeof(ledCount_))
            {
                LOG_ERROR("Could not write LED border led count\n");
                return 0;
            }

            /* Print the gradient size count state */
            printSize   = rPrinter.write((uint8_t*)&gradSizeCount_,
                                         sizeof(gradSizeCount_));
            totalPrint += printSize;
            if(printSize != sizeof(gradSizeCount_))
            {
                LOG_ERROR("Could not write LED border grad size count\n");
                return 0;
            }

            /* Print the grad sizes */
            printSize   = rPrinter.write((uint8_t*)pGradSizes_,
                                         sizeof(pGradSizes_[0]) *
                                         gradSizeCount_);
            totalPrint += printSize;
            if(printSize != sizeof(pGradSizes_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not write LED border grad size\n");
                return 0;
            }

            /* Print the start and end colors */
            printSize   = rPrinter.write((uint8_t*)pStartColors_,
                                         sizeof(pStartColors_[0]) *
                                         gradSizeCount_);
            totalPrint += printSize;
            if(printSize != sizeof(pStartColors_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not write LED border grad size\n");
                return 0;
            }
            printSize   = rPrinter.write((uint8_t*)pEndColors_,
                                         sizeof(pEndColors_[0]) *
                                         gradSizeCount_);
            totalPrint += printSize;
            if(printSize != sizeof(pEndColors_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not write LED border grad size\n");
                return 0;
            }

            /* Print the colors info */
            for(i = 0; i < ledCount_; ++i)
            {
                printSize   = rPrinter.write((uint8_t*)&pColors_[i].raw,
                                             sizeof(pColors_[i].raw));
                totalPrint += printSize;
                if(printSize != sizeof(pColors_[i].raw))
                {
                    LOG_ERROR("Could not write LED color\n");
                    return 0;
                }
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream & rStream)
        {
            uint8_t i;

            /* Get the led count */
            if(rStream.readBytes((uint8_t*)&ledCount_,
                                 sizeof(ledCount_)) != sizeof(ledCount_))
            {
                LOG_ERROR("Could not read led count\n");
                return false;
            }
            delete[] pColors_;
            pColors_ = new CRGB[ledCount_];

            /* Get the gradient size count state */
            if(rStream.readBytes((uint8_t*)&gradSizeCount_,
                                 sizeof(gradSizeCount_)) !=
                sizeof(gradSizeCount_))
            {
                LOG_ERROR("Could not read gradient size count\n");
                return false;
            }

            delete[] pGradSizes_;
            pGradSizes_ = new uint16_t[gradSizeCount_];

            /* Get the grad sizes */
            if(rStream.readBytes((uint8_t*)&pGradSizes_[0],
                                 sizeof(pGradSizes_[0]) * gradSizeCount_) !=
                sizeof(pGradSizes_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not read gradient size\n");
                return false;
            }

            /* Get the saved colors */
            delete[] pStartColors_;
            delete[] pEndColors_;

            pStartColors_ = new uint32_t[gradSizeCount_];
            pEndColors_   = new uint32_t[gradSizeCount_];

            if(rStream.readBytes((uint8_t*)&pStartColors_[0],
                                 sizeof(pStartColors_[0]) * gradSizeCount_) !=
                sizeof(pStartColors_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not read start colors\n");
                return false;
            }
            if(rStream.readBytes((uint8_t*)&pEndColors_[0],
                                 sizeof(pEndColors_[0]) * gradSizeCount_) !=
                sizeof(pEndColors_[0]) * gradSizeCount_)
            {
                LOG_ERROR("Could not end start colors\n");
                return false;
            }

            /* Get the colors info */
            for(i = 0; i < ledCount_; ++i)
            {
                if(rStream.readBytes((uint8_t*)&pColors_[i].raw,
                                     sizeof(pColors_[i].raw)) !=
                    sizeof(pColors_[i].raw))
                {
                    LOG_ERROR("Could not read LED color\n");
                    return false;
                }
            }

            isApplied_ = false;
            return true;
        }

        ELEDBorderPattern GetType(void) const
        {
            return type_;
        }

        void GetRawParam(SLEDBorderPatternParam * pPar) const
        {
            uint8_t i;

            memset(pPar, 0, sizeof(SLEDBorderPatternParam));

            for(i = 0; i < gradSizeCount_; ++i)
            {
                pPar->startColorCode[i] = pStartColors_[i];
                pPar->endColorCode[i]   = pEndColors_[i];
                pPar->gradientSize[i]   = pGradSizes_[i];
            }
        }

    private:
        void InitGradient(const uint32_t * pkStartColor,
                          const uint32_t * pkEndColor,
                          const uint16_t * pkGradSize)
        {
            uint32_t startColorFix;
            uint32_t endColorFix;
            uint16_t pathSize;
            int32_t  r;
            int32_t  g;
            int32_t  b;
            uint8_t  i;


            pColors_      = new CRGB[ledCount_];
            pGradSizes_   = new uint16_t[gradSizeCount_];
            pStartColors_ = new uint32_t[gradSizeCount_];
            pEndColors_   = new uint32_t[gradSizeCount_];

            pathSize = ledCount_ / gradSizeCount_;

            fill_solid(pColors_, ledCount_, CRGB::Black);
            for(i = 0; i < gradSizeCount_; ++i)
            {
                FIX_COLOR(startColorFix, r, g, b, pkStartColor[i]);
                FIX_COLOR(endColorFix, r, g, b, pkEndColor[i]);
                FIX_GRAD_SIZE(pGradSizes_[i], pathSize, pkGradSize[i]);

                pStartColors_[i] = startColorFix;
                pEndColors_[i]   = endColorFix;

                fill_gradient_RGB(&pColors_[i * pathSize], pGradSizes_[i],
                                  startColorFix, endColorFix);
            }
        }

        uint16_t * pGradSizes_;
        uint32_t * pStartColors_;
        uint32_t * pEndColors_;
        uint8_t    gradSizeCount_;

        CRGB     * pColors_;
};

class SingleColorPattern : public ColorPattern
{
    public:
        SingleColorPattern(const uint32_t kColor,
                           const uint16_t kLedCount)
        : ColorPattern(kLedCount,
                       ELEDBorderPattern::LED_COLOR_PATTERN_PLAIN)
        {
            int32_t r;
            int32_t g;
            int32_t b;

            FIX_COLOR(color_, r, g, b, kColor);
        }

        virtual ~SingleColorPattern(void)
        {
        }

        virtual void ApplyPattern(uint32_t * pLedsColors)
        {
            uint16_t i;

            /* Apply only once */
            if(isApplied_ == false)
            {
                /* Apply the palette to the leds */
                for(i = 0; i < ledCount_; ++i)
                {
                    pLedsColors[i] = color_;
                }
                isApplied_ = true;
            }
        }

        virtual size_t printTo(Print & rPrinter) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = rPrinter.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border pattern type\n");
                return 0;
            }

            /* Print the led count */
            printSize   = rPrinter.write((uint8_t*)&ledCount_,
                                         sizeof(ledCount_));
            totalPrint += printSize;
            if(printSize != sizeof(ledCount_))
            {
                LOG_ERROR("Could not write LED border led count\n");
                return 0;
            }

            /* Print color */
            printSize   = rPrinter.write((uint8_t*)&color_, sizeof(color_));
            totalPrint += printSize;
            if(printSize != sizeof(color_))
            {
                LOG_ERROR("Could not write LED border led color\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream & rStream)
        {
            /* Get the led count */
            if(rStream.readBytes((uint8_t*)&ledCount_,
                                 sizeof(ledCount_)) !=
                sizeof(ledCount_))
            {
                LOG_ERROR("Could not read led count\n");
                return false;
            }

            /* Get the gradient size count state */
            if(rStream.readBytes((uint8_t*)&color_,
                                 sizeof(color_)) !=
                sizeof(color_))
            {
                LOG_ERROR("Could not read led color\n");
                return false;
            }

            isApplied_ = false;
            return true;
        }

        ELEDBorderPattern GetType(void) const
        {
            return type_;
        }

        void GetRawParam(SLEDBorderPatternParam * pPar) const
        {
            pPar->plainColorCode = color_;
        }

    private:
        uint32_t color_;
};

/*************************** ANIMATION DEFINITIONS ****************************/

IColorAnimation::~IColorAnimation(void)
{
}

class TrailAnimation : public IColorAnimation
{
    public:
        explicit TrailAnimation(const uint8_t kRateDivider)
        {
            if(kRateDivider > 20)
            {
                rateDivider_ = 1;
            }
            else
            {
                rateDivider_ = 21 - kRateDivider;
            }
            type_          = ELEDBorderAnimation::LED_COLOR_ANIM_TRAIL;
            maxBrightness_ = 255;
        }

        ~TrailAnimation(void)
        {

        }

        void SetMaxBrightness(const uint8_t kMaxBrightness)
        {
            maxBrightness_ = kMaxBrightness;
        }

        bool ApplyAnimation(uint32_t       * pLedColors,
                            const uint16_t   kLedCount,
                            const uint32_t   kIterNum)
        {
            uint16_t i;
            uint32_t saveLast;

            /* Divide the iternum */
            if(kIterNum % rateDivider_ == 0)
            {
                saveLast = pLedColors[0];
                /* Just set the next LED as the previous one */
                for(i = 1; i < kLedCount; ++i)
                {
                    pLedColors[i - 1] = pLedColors[i];
                }
                pLedColors[kLedCount - 1] = saveLast;
            }

            return false;
        }

        virtual size_t printTo(Print & rPrinter) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = rPrinter.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border animation type\n");
                return 0;
            }

            /* Print the max brightness  */
            printSize   = rPrinter.write((uint8_t*)&maxBrightness_,
                                         sizeof(maxBrightness_));
            totalPrint += printSize;
            if(printSize != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not write LED border max brightness\n");
                return 0;
            }

            /* Print rate divider  */
            printSize   = rPrinter.write((uint8_t*)&rateDivider_,
                                         sizeof(rateDivider_));
            totalPrint += printSize;
            if(printSize != sizeof(rateDivider_))
            {
                LOG_ERROR("Could not write LED border rate devider\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream & rStream)
        {
            /* Get the max brightness */
            if(rStream.readBytes((uint8_t*)&maxBrightness_,
                                 sizeof(maxBrightness_)) !=
                sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not read max brightness\n");
                return false;
            }

            /* Get the rate divider */
            if(rStream.readBytes((uint8_t*)&rateDivider_,
                                 sizeof(rateDivider_)) !=
                sizeof(rateDivider_))
            {
                LOG_ERROR("Could not read rate divider\n");
                return false;
            }

            return true;
        }

        ELEDBorderAnimation GetType(void) const
        {
            return type_;
        }

        uint8_t GetRawParam(void) const
        {

            if(rateDivider_ <= 1)
            {
                return 20;
            }
            else
            {
                return 21 - rateDivider_;
            }
        }

    private:
        uint8_t rateDivider_;
};

class BreathAnimation : public IColorAnimation
{
    public:
        explicit BreathAnimation(const uint8_t kSpeedIncrease)
        {
            if(kSpeedIncrease > 20)
            {
                speedIncrease_ = 1;
            }
            else
            {
                speedIncrease_  = 21 - kSpeedIncrease;
            }
            sIncrease_ = true;
            type_      = ELEDBorderAnimation::LED_COLOR_ANIM_BREATH;
        }

        ~BreathAnimation(void)
        {

        }

        void SetMaxBrightness(const uint8_t kMaxBrightness)
        {
            maxBrightness_ = kMaxBrightness;
        }

        bool ApplyAnimation(uint32_t       * pLedColors,
                            const uint16_t   kLedCount,
                            const uint32_t   kIterNum)
        {
            (void) pLedColors;
            (void) kLedCount;

            uint8_t currBrightness;
            if(kIterNum % speedIncrease_ == 0)
            {
                currBrightness = FastLED.getBrightness();
                if(sIncrease_ == true)
                {
                    if(currBrightness + 1 <= maxBrightness_)
                    {
                        ++currBrightness;
                    }
                    else
                    {
                        currBrightness = maxBrightness_;
                        sIncrease_ = false;
                    }
                }
                else
                {
                    if(currBrightness >= 1)
                    {
                        --currBrightness;
                    }
                    else
                    {
                        currBrightness = 0;
                        sIncrease_ = true;
                    }
                }
                FastLED.setBrightness(currBrightness);
            }

            return true;
        }

        virtual size_t printTo(Print & rPrinter) const
        {
            size_t   printSize;
            size_t   totalPrint;

            totalPrint = 0;

            /* Print the type */
            printSize   = rPrinter.write((uint8_t*)&type_, sizeof(type_));
            totalPrint += printSize;
            if(printSize != sizeof(type_))
            {
                LOG_ERROR("Could not write LED border animation type\n");
                return 0;
            }

            /* Print the max brightness  */
            printSize   = rPrinter.write((uint8_t*)&maxBrightness_,
                                         sizeof(maxBrightness_));
            totalPrint += printSize;
            if(printSize != sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not write LED border max brightness\n");
                return 0;
            }

            /* Print speed increase */
            rPrinter.print(speedIncrease_);
            printSize   = rPrinter.write((uint8_t*)&speedIncrease_,
                                         sizeof(speedIncrease_));
            totalPrint += printSize;
            if(printSize != sizeof(speedIncrease_))
            {
                LOG_ERROR("Could not write LED border speed increase\n");
            }

            /* Print increase state */
            rPrinter.print(sIncrease_);
            printSize   = rPrinter.write((uint8_t*)&sIncrease_,
                                         sizeof(sIncrease_));
            totalPrint += printSize;
            if(printSize != sizeof(sIncrease_))
            {
                LOG_ERROR("Could not write LED border increase state\n");
            }

            return totalPrint;
        }

        virtual bool readFrom(Stream & rStream)
        {
            /* Get the max brightness */
            if(rStream.readBytes((uint8_t*)&maxBrightness_,
                                 sizeof(maxBrightness_)) !=
                sizeof(maxBrightness_))
            {
                LOG_ERROR("Could not read max brightness\n");
                return false;
            }

            /* Get the speed increase */
            if(rStream.readBytes((uint8_t*)&speedIncrease_,
                                 sizeof(speedIncrease_)) !=
                sizeof(speedIncrease_))
            {
                LOG_ERROR("Could not read speed increase\n");
                return false;
            }

            /* Get the increase state */
            if(rStream.readBytes((uint8_t*)&sIncrease_,
                                 sizeof(sIncrease_)) !=
                sizeof(sIncrease_))
            {
                LOG_ERROR("Could not read increase state\n");
                return false;
            }

            return true;
        }

        ELEDBorderAnimation GetType(void) const
        {
            return type_;
        }

        uint8_t GetRawParam(void) const
        {
            if(speedIncrease_ <= 1)
            {
                return 20;
            }
            else
            {
                return 21 - speedIncrease_;
            }
        }

    private:
        uint8_t     speedIncrease_;
        static bool sIncrease_;

};

bool BreathAnimation::sIncrease_;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/* None */

/************************** Static global variables ***************************/
static SemaphoreHandle_t sThreadWork;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

static void WorkerRoutine(void * pArgs);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void WorkerRoutine(void * pArgs)
{
    LEDBorder                     * pCurrBorderMgr;
    ColorPattern                  * pPattern;
    std::vector<IColorAnimation*> * pAnimations;

    uint32_t * ledArrayColors;
    uint32_t   iterNum;
    uint32_t   i;
    uint8_t    brightnessUpdated;

    pCurrBorderMgr = (LEDBorder*)pArgs;
    ledArrayColors = pCurrBorderMgr->GetLEDArrayColors();

    LOG_DEBUG("Worker thread on core %d\n", xPortGetCoreID());

    iterNum = 0;
    while(1)
    {
        if(pCurrBorderMgr->IsEnabled())
        {
            if(uxSemaphoreGetCount(sThreadWork) == 1)
            {
                xSemaphoreTake(sThreadWork, portMAX_DELAY);
            }
            pCurrBorderMgr->Lock();

            pPattern    = pCurrBorderMgr->GetColorPattern();
            pAnimations = pCurrBorderMgr->GetColorAnimations();

            pPattern->ApplyPattern(ledArrayColors);

            brightnessUpdated = false;
            for(i = 0; i < pAnimations->size(); ++i)
            {
                brightnessUpdated |= pAnimations->at(i)
                                                ->ApplyAnimation(ledArrayColors,
                                                                 NUM_LEDS,
                                                                 iterNum);
            }

            if(brightnessUpdated == false)
            {
                FastLED.setBrightness(pCurrBorderMgr->GetBrightness());
            }

            pCurrBorderMgr->Refresh();
            pCurrBorderMgr->Unlock();
            FastLED.delay(2);
            ++iterNum;
        }
        else
        {
            if(uxSemaphoreGetCount(sThreadWork) == 0)
            {
                xSemaphoreGive(sThreadWork);
            }
            delay(BORDER_REFRESH_PERIOD);
        }
    }
}

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

IColorAnimation * CBUILD::DeserializeAnimation(Stream & rStream)
{
    IColorAnimation     * pNewAnim;
    ELEDBorderAnimation   type;

    pNewAnim = nullptr;

    /* Read the animation type */
    if(rStream.readBytes((uint8_t*)&type, sizeof(type)) == sizeof(type))
    {
        switch(type)
        {
            case ELEDBorderAnimation::LED_COLOR_ANIM_BREATH:
                pNewAnim = new BreathAnimation(1);
                break;
            case ELEDBorderAnimation::LED_COLOR_ANIM_TRAIL:
                pNewAnim = new TrailAnimation(1);
                break;
            default:
                LOG_ERROR("Incorrect read animation type %d\n", type);
                return nullptr;
        }

        if(!pNewAnim->readFrom(rStream))
        {
            LOG_ERROR("Could not load new animation\n");
            delete pNewAnim;
            pNewAnim = nullptr;
        }
    }
    else
    {
        LOG_ERROR("Could not read the animation type\n");
    }

    return pNewAnim;
}

ColorPattern* CBUILD::DeserializePattern(Stream & rStream)
{
    ColorPattern      * pNewPattern;
    ELEDBorderPattern   type;

    pNewPattern = nullptr;

    /* Read the animation type */
    if(rStream.readBytes((uint8_t*)&type, sizeof(type)) == sizeof(type))
    {
        switch(type)
        {
            case ELEDBorderPattern::LED_COLOR_PATTERN_PLAIN:
                pNewPattern = new SingleColorPattern(1, 1);
                break;
            case ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_1:
            case ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_2:
            case ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_3:
            case ELEDBorderPattern::LED_COLOR_PATTERN_GRADIENT_4:
                pNewPattern = new GradientPattern(type);
                break;
            default:
                LOG_ERROR("Incorrect read pattern type %d\n", type);
                return nullptr;
        }

        if(pNewPattern->readFrom(rStream) == false)
        {
            LOG_ERROR("Could not load new pattern\n");
            delete pNewPattern;
        }
    }
    else
    {
        LOG_ERROR("Could not read the pattern type\n");
    }

    return pNewPattern;
}

CLEDB::LEDBorder(SystemState * pSystemState)
{
    pSystemState_ = pSystemState;
    pStore_       = Storage::GetInstance();
    pPattern_     = nullptr;
    brightness_   = 50;
    enabled_      = false;
}

CLEDB::~LEDBorder(void)
{
    size_t i;

    enabled_ = false;

    vTaskDelete(workerThread_);
    Unlock();

    delete pPattern_;

    for(i = 0; i < animations_.size(); ++i)
    {
        delete animations_[i];
    }
    animations_.clear();

}

void CLEDB::Init(void)
{
    uint8_t   i;
    bool      oldBrightness;

    driverLock_ = xSemaphoreCreateBinary();
    xSemaphoreGive(driverLock_);

    /* Add LEDs and set init brightness */
    CLEDController &ctrl = FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds_, NUM_LEDS);
    ctrl.setCorrection(TypicalLEDStrip);

    memset(pLedsColors_, 0, sizeof(pLedsColors_));

    pPattern_ = nullptr;

    /* Try to load from saved state */
    oldBrightness = brightness_;
    if(!pStore_->LoadLEDBorderSettings(enabled_,
                                       brightness_,
                                       &pPattern_,
                                       animations_))
    {
        /* Not able to load, set init values */
        LOG_DEBUG("Loading LED Border with initial values\n")

        brightness_ = oldBrightness;
        FastLED.setBrightness(brightness_);
        enabled_ = false;

        delete pPattern_;
        pPattern_ = new SingleColorPattern(0xFFFFFFFF, NUM_LEDS);

        ClearAnimations();

        /* Save current state */
        pStore_->SaveLEDBorderEnabled(enabled_);
        pStore_->SaveLEDBorderBrightness(brightness_);
        pStore_->SaveLEDBorderPattern(pPattern_);
        pStore_->RemoveLEDBorderAnimations();
    }
    else
    {
        LOG_DEBUG("Loaded LED Border with saved values\n")
    }

    sThreadWork = xSemaphoreCreateMutex();

    /* Launch worker thread */
    xTaskCreatePinnedToCore(WorkerRoutine,
                            "LEDBorderWorker",
                            2048,
                            this,
                            0,
                            &workerThread_,
                            0);
    vTaskSuspend(workerThread_);

    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(0);
    }
    FastLED.show();

    if(enabled_ == true)
    {
        enabled_ = false;
        Enable();
    }
}

void CLEDB::Stop(void)
{
    if(enabled_ == true)
    {
        enabled_ = false;

        /* Suspend working thread */
        LOG_DEBUG("Suspending LEDBorder thread\n");
        vTaskSuspend(workerThread_);
        vTaskDelete(workerThread_);

        /* Set brightness to 0 */
        FastLED.setBrightness(0);
        FastLED.clear(true);
        FastLED.show();
    }
}

void CLEDB::Enable(void)
{
    if(enabled_ == false)
    {
        LOG_DEBUG("Resuming LEDBorder thread\n");
        FastLED.setBrightness(brightness_);
        vTaskResume(workerThread_);
        enabled_ = true;

        if(pStore_->SaveLEDBorderEnabled(enabled_) == false)
        {
            LOG_ERROR("Could not save the LED Border state\n");
        }
    }
}

void CLEDB::Disable(void)
{
    uint8_t i;

    if(enabled_ == true)
    {
        enabled_ = false;

        /* Suspend working thread */
        while(uxSemaphoreGetCount(sThreadWork) == 0);
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

        if(pStore_->SaveLEDBorderEnabled(enabled_) == false)
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
    uint8_t          responseSize;
    uint8_t          pResponse[4];
    uint8_t          pActionParam[COMMAND_DATA_SIZE];
    ELEDBorderAction action;

    action = pSystemState_->ConsumeELEDBorderAction(pActionParam);

    responseSize = 2;
    pResponse[0]  = 'O';
    pResponse[1]  = 'K';
    switch(action)
    {
        case ENABLE_LEDB_ACTION:
            Enable();
            break;

        case DISABLE_LEDB_ACTION:
            Disable();
            break;

        case ADD_ANIMATION_LEDB_ACTION:
            pResponse[2] = AddAnimation((ELEDBorderAnimation)pActionParam[0],
                                   (SLEDBorderAnimationParam*)&pActionParam[1]);
            /* On errror update response */
            if(pResponse[2] == 255)
            {
                pResponse[0] = 'K';
                pResponse[1] = 'O';
            }
            ++responseSize;
            break;

        case REMOVE_ANIMATION_LEDB_ACTION:
            RemoveAnimation(pActionParam[0]);
            break;

        case SET_PATTERN_LEDB_ACTION:
            SetPattern((ELEDBorderPattern)pActionParam[0],
                       (SLEDBorderPatternParam*)&pActionParam[1]);
            break;

        case CLEAR_ANIMATION_LEDB_ACTION:
            ClearAnimations();
            break;

        case SET_BRIGHTNESS_LEDB_ACTION:
            SetBrightness(pActionParam[0]);
            break;

        default:
            responseSize = 0;
            break;
    }

    if(responseSize != 0)
    {
        pSystemState_->EnqueueResponse(pResponse, responseSize);
    }
}

void CLEDB::Refresh(void)
{
    uint8_t i;

    /* Set all LEDs to previous state */
    for(i = 0; i < NUM_LEDS; ++i)
    {
        leds_[i].setColorCode(pLedsColors_[i]);
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

void CLEDB::SetPattern(const ELEDBorderPattern        kPatternId,
                       const SLEDBorderPatternParam * pkPatternParam)
{
    Lock();
    delete pPattern_;

    switch(kPatternId)
    {
        case LED_COLOR_PATTERN_PLAIN:
            pPattern_ = new SingleColorPattern(pkPatternParam->plainColorCode,
                                               NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_1:
            pPattern_ = new GradientPattern(pkPatternParam->startColorCode[0],
                                            pkPatternParam->endColorCode[0],
                                            pkPatternParam->gradientSize[0],
                                            NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_2:
            pPattern_ = new GradientPattern(pkPatternParam->startColorCode[0],
                                            pkPatternParam->endColorCode[0],
                                            pkPatternParam->gradientSize[0],
                                            pkPatternParam->startColorCode[1],
                                            pkPatternParam->endColorCode[1],
                                            pkPatternParam->gradientSize[1],
                                            NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_3:
            pPattern_ = new GradientPattern(pkPatternParam->startColorCode[0],
                                            pkPatternParam->endColorCode[0],
                                            pkPatternParam->gradientSize[0],
                                            pkPatternParam->startColorCode[1],
                                            pkPatternParam->endColorCode[1],
                                            pkPatternParam->gradientSize[1],
                                            pkPatternParam->startColorCode[2],
                                            pkPatternParam->endColorCode[2],
                                            pkPatternParam->gradientSize[2],
                                            NUM_LEDS);
            break;
        case LED_COLOR_PATTERN_GRADIENT_4:
            pPattern_ = new GradientPattern(pkPatternParam->startColorCode[0],
                                            pkPatternParam->endColorCode[0],
                                            pkPatternParam->gradientSize[0],
                                            pkPatternParam->startColorCode[1],
                                            pkPatternParam->endColorCode[1],
                                            pkPatternParam->gradientSize[1],
                                            pkPatternParam->startColorCode[2],
                                            pkPatternParam->endColorCode[2],
                                            pkPatternParam->gradientSize[2],
                                            pkPatternParam->startColorCode[3],
                                            pkPatternParam->endColorCode[3],
                                            pkPatternParam->gradientSize[3],
                                            NUM_LEDS);
            break;
        default:
            /* White color basic */
            pPattern_ = new SingleColorPattern(0x00FFFFFF, NUM_LEDS);
    }

    if(pStore_->SaveLEDBorderPattern(pPattern_) == false)
    {
        LOG_ERROR("Could not save LED Border pattern\n");
    }

    Unlock();
}

uint8_t CLEDB::AddAnimation(const ELEDBorderAnimation        kAnimId,
                            const SLEDBorderAnimationParam * pParam)
{
    IColorAnimation * pAnimation;

    /* We limite the number of animations to 255 */
    if(animations_.size() < 255)
    {
        pAnimation = nullptr;
        switch(kAnimId)
        {
            case LED_COLOR_ANIM_TRAIL:
                pAnimation = new TrailAnimation(pParam->rateDivider);
                break;
            case LED_COLOR_ANIM_BREATH:
                pAnimation = new BreathAnimation(pParam->speedIncrease);
                break;
            default:
                return 255;
        }

        pAnimation->SetMaxBrightness(brightness_);

        Lock();
        animations_.push_back(pAnimation);
        Unlock();

        if(pStore_->SaveLEDBorderAnimation(pAnimation, animations_.size() - 1)
           == false)
        {
            LOG_ERROR("Could not save LED Border animation %d\n",
                      animations_.size() - 1);
        }

        return animations_.size() - 1;
    }
    else
    {
        return 255;
    }

}

void CLEDB::RemoveAnimation(const uint8_t kAnimIdx)
{
    Lock();
    if(kAnimIdx < animations_.size())
    {
        delete animations_[kAnimIdx];
        animations_.erase(animations_.begin() + kAnimIdx);
    }
    if(pStore_->RemoveLEDBorderAnimation(kAnimIdx) == false)
    {
        LOG_ERROR("Could not remove animation %d\n", kAnimIdx);
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
    pStore_->RemoveLEDBorderAnimations();
    animations_.clear();

    Unlock();
}

uint8_t CLEDB::GetBrightness(void) const
{
    return brightness_;
}

uint32_t * CLEDB::GetLEDArrayColors(void)
{
    return pLedsColors_;
}

std::vector<IColorAnimation*> * CLEDB::GetColorAnimations(void)
{
    return &animations_;
}

CPATTERN * CLEDB::GetColorPattern(void)
{
    return pPattern_;
}

void CLEDB::Lock(void)
{
    xSemaphoreTake(driverLock_, portMAX_DELAY);
}

void CLEDB::Unlock(void)
{
    xSemaphoreGive(driverLock_);
}

void CLEDB::SetBrightness(const uint8_t kBrightness)
{
    uint8_t   i;

    if(kBrightness < MIN_BRIGHTNESS)
    {
        brightness_ = MIN_BRIGHTNESS;
    }
    else
    {
        brightness_ = kBrightness;
    }

    Lock();

    for(i = 0; i < animations_.size(); ++i)
    {
        animations_[i]->SetMaxBrightness(brightness_);
        pStore_->SaveLEDBorderAnimation(animations_[i], i);
    }
    FastLED.setBrightness(brightness_);

    Unlock();

    if(pStore_->SaveLEDBorderBrightness(brightness_) == false)
    {
        LOG_ERROR("Could not save the LED Border brightness\n");
    }
}

CPATTERN::ColorPattern(const uint16_t          kLedCount,
                       const ELEDBorderPattern kType)
{
    ledCount_  = kLedCount;
    type_      = kType;
    isApplied_ = false;
}

CPATTERN::~ColorPattern(void)
{

}

#undef CLEDB
#undef CPATTERN
#undef CBUILD