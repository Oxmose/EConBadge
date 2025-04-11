/*******************************************************************************
 * @file LEDBorder.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 26/02/2025
 *
 * @version 2.0
 *
 * @brief This file defines the LED border manager and driver.
 *
 * @details This file defines the LED border manager and driver. It initializes
 * the LED border and provides the different services used to manage the border.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#define FASTLED_INTERNAL

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <vector>         /* std::vector */
#include <cstdint>        /* Generic Types */
#include <HWMgr.h>        /* HW manager */
#include <Types.h>        /* Custom types */
#include <Storage.h>      /* Storage manager */
#include <FastLED.h>      /* Fast LED Service */
#include <BlueToothMgr.h> /* Bleutooth services */

/* Header File */
#include <LEDBorder.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SEND_INFO_TIMEOUT 10000 /* 10 seconds */
#define FASTLED_MAX_BRIGHTNESS 255U
#define MAX_BRIGHTNESS 100U
#define MIN_BRIGHTNESS 5U
#define MAX_PATTERN_COUNT 120
#define MAX_ANIM_COUNT 120
#define MIN_COLOR 5U

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define IS_OVERLAPPING(S0, E0, S1, E1) (!((E0) < (S1) || (S0) > (E1)))

#define FIX_COLOR(COLORFIX, R, G, B, COLOR) {                           \
    (R) = ((COLOR) >> 16) & 0xFF;                                       \
    (G) = ((COLOR) >>  8) & 0xFF;                                       \
    (B) = (COLOR) & 0xFF;                                               \
                                                                        \
    /* Check the minimal color value */                                 \
    if((R) < MIN_COLOR)                                                 \
    {                                                                   \
        (R) = 0;                                                        \
    }                                                                   \
    if((G) < MIN_COLOR)                                                 \
    {                                                                   \
        (G) = 0;                                                        \
    }                                                                   \
    if((B) < MIN_COLOR)                                                 \
    {                                                                   \
        (B) = 0;                                                        \
    }                                                                   \
                                                                        \
    (COLORFIX) = ((R) << 16) | ((G) << 8) | (B);                        \
}

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
/* Initialize the handlers */
TPatternHandler LEDBorder::PATTERN_HANDLERS[LED_PATTERN_MAX_ID] = {
    PatternHandlerPlain,    // LED_PATTERN_PLAIN
    PatternHandlerGradient, // LED_PATTERN_GRADIENT
};

TAnimationHandler LEDBorder::ANIMATION_HANDLERS[LED_ANIMATION_MAX_ID] = {
    AnimHandlerTrail,  // LED_ANIMATION_TRAIL
    AnimHandlerBreath, // LED_ANIMATION_BREATH
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

LEDBorder::LEDBorder(BluetoothManager* pBtManager)
{
    uint8_t i;

    /* Initialize members */
    isEnabled_  = false;
    brightness_ = 0;
    pStore_     = Storage::GetInstance();
    pBtManager_ = pBtManager;

    memset(ledsColors_, 0, STRIP_LED_COUNT * sizeof(uint32_t));
    for(i = 0; i < STRIP_LED_COUNT; ++i)
    {
        ledsData_[i].setColorCode(0);
    }

    /* Load the current LEDborder state */
    if(pStore_->FileExists(LEDBORDER_DIR_PATH))
    {
        LoadState();
    }
    else
    {
        pStore_->CreateDirectory(LEDBORDER_DIR_PATH);
    }

    /* Add the leds to fastled interface */
    FastLED.addLeds<WS2812B, GPIO_LED_DATA, GRB>(
        ledsData_,
        STRIP_LED_COUNT
    ).setCorrection(TypicalLEDStrip);

    /* Set GPIO enable pin */
    pinMode(GPIO_LED_ENABLE, OUTPUT);
    if(isEnabled_)
    {
        digitalWrite(GPIO_LED_ENABLE, HIGH);
    }
    else
    {
        digitalWrite(GPIO_LED_ENABLE, LOW);
    }

    /* Create the LED border lock */
    lock_ = xSemaphoreCreateBinary();
    xSemaphoreGive(lock_);

    /* Create the led thread */
    xTaskCreatePinnedToCore(
        UpdateLEDBorder,
        "LEDThread",
        4096,
        this,
        22,
        &workerThread_,
        1
    );
}

void LEDBorder::Enable(const bool kEnabled)
{
    /* Nothing to do */
    if(isEnabled_ == kEnabled)
    {
        return;
    }

    /* Set state and resume worker thread is needed */
    isEnabled_ = kEnabled;
    if(kEnabled)
    {
        vTaskResume(workerThread_);
    }

    LOG_DEBUG("Enabling LED: ? %d\n", isEnabled_);

    /* Save new state */
    pStore_->SetContent(
        LEDBORDER_ENABLED_FILE_PATH,
        std::to_string(isEnabled_),
        true
    );
}

bool LEDBorder::IsEnabled(void) const
{
    return isEnabled_;
}

void LEDBorder::IncreaseBrightness(SCommandResponse& rReponse)
{
    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Setup new brightness */
    if(brightness_ < MAX_BRIGHTNESS)
    {
        brightness_ = MIN(100, brightness_ + MIN_BRIGHTNESS);

        /* Save new brightness */
        pStore_->SetContent(
            LEDBORDER_BRIGHTNESS_FILE_PATH,
            std::to_string(brightness_),
            true
        );
    }

    xSemaphoreGive(lock_);

    /* Set the return value to return the current brightness */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = brightness_;
}

void LEDBorder::ReduceBrightness(SCommandResponse& rReponse)
{
    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Setup new brightness */
    if(brightness_ > MIN_BRIGHTNESS)
    {
        brightness_ = MAX(MIN_BRIGHTNESS, brightness_ - MIN_BRIGHTNESS);

        /* Save new brightness */
        pStore_->SetContent(
            LEDBORDER_BRIGHTNESS_FILE_PATH,
            std::to_string(brightness_),
            true
        );
    }

    xSemaphoreGive(lock_);

    /* Set the return value to return the current brightness */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = brightness_;
}

void LEDBorder::SetBrightness(const uint8_t* kpData, SCommandResponse& rReponse)
{
    xSemaphoreTake(lock_, portMAX_DELAY);

    if(*kpData >= MIN_BRIGHTNESS && *kpData <= MAX_BRIGHTNESS)
    {
        brightness_ = *kpData;

        /* Save new brightness */
        pStore_->SetContent(
            LEDBORDER_BRIGHTNESS_FILE_PATH,
            std::to_string(brightness_),
            true
        );
    }

    xSemaphoreGive(lock_);

    /* Set the return value to return the current brightness */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = brightness_;
}

uint8_t LEDBorder::GetBrightness(void) const
{
    return brightness_;
}

void LEDBorder::Clear(SCommandResponse& rReponse)
{
    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Clear the animations and patterns  */
    animations_.clear();
    patterns_.clear();

    /* Remove from storage */
    pStore_->Remove(LEDBORDER_PATTERN_FILE_PATH);
    pStore_->Remove(LEDBORDER_ANIM_FILE_PATH);

    ResetState();

    xSemaphoreGive(lock_);


    /* Set the return value */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
}

void LEDBorder::AddPattern(const uint8_t* kpData, SCommandResponse& rReponse)
{
    uint8_t                  i;
    bool                     isOverlapping;
    const SLEDBorderPattern* kpPattern;

    /* Check if we can add a new pattern */
    if(patterns_.size() >= MAX_PATTERN_COUNT)
    {
        rReponse.header.errorCode = NO_MORE_MEMORY;
        rReponse.header.size = 0;
        return;
    }

    kpPattern = (SLEDBorderPattern*)kpData;

    LOG_DEBUG("Adding pattern %d, %d, %d, %d ,%d, %d\n", kpPattern->type, kpPattern->startColorCode, kpPattern->endColorCode, kpPattern->startLedIdx, kpPattern->endLedIdx, kpPattern->param0);

    /* Check the pattern type */
    if(LED_PATTERN_MAX_ID <= kpPattern->type)
    {
        rReponse.header.errorCode = INVALID_PARAM;
        rReponse.header.size = 0;
        return;
    }

    /* Check the pattern bounds */
    if(kpPattern->startLedIdx >= STRIP_LED_COUNT ||
       kpPattern->endLedIdx >= STRIP_LED_COUNT ||
       kpPattern->startLedIdx >= kpPattern->endLedIdx)
    {
        rReponse.header.errorCode = INVALID_INDEX;
        rReponse.header.size = 0;
        return;
    }

    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Search for a conflicting pattern */
    for(i = 0; i < patterns_.size(); ++i)
    {
        isOverlapping = IS_OVERLAPPING(
            kpPattern->startLedIdx,
            kpPattern->endLedIdx,
            patterns_[i].startLedIdx,
            patterns_[i].endLedIdx
        );
        if(isOverlapping)
        {
            xSemaphoreGive(lock_);

            rReponse.header.errorCode = OVERLAPPING_PATTERNS;
            rReponse.header.size = 1;
            /* Add the pattern index */
            rReponse.pResponse[0] = i;
            return;
        }
    }

    /* Set all patterns as non applied */
    for(i = 0; i < patterns_.size(); ++i)
    {
        patterns_[i].applied = false;
    }

    patterns_.push_back(*kpPattern);

    SavePatterns();

    ResetState();

    xSemaphoreGive(lock_);

    /* Add the pattern and set the response to return the new index */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = patterns_.size() - 1;
}

void LEDBorder::RemovePattern(const uint8_t*    kpPatternIdx,
                              SCommandResponse& rReponse)
{
    uint8_t i;

    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Check if the index exists */
    if(*kpPatternIdx >= patterns_.size())
    {
        xSemaphoreGive(lock_);

        rReponse.header.errorCode = INVALID_PARAM;
        rReponse.header.size = 0;
        return;
    }

    /* Remove and set all patterns as non applied */
    for(i = 0; i < patterns_.size() - 1; ++i)
    {
        if(i >= *kpPatternIdx)
        {
            patterns_[i] = patterns_[i + 1];
        }
        patterns_[i].applied = false;
    }

    patterns_.pop_back();

    SavePatterns();

    ResetState();

    xSemaphoreGive(lock_);

    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
}

void LEDBorder::ClearPatterns(SCommandResponse& rReponse)
{
    /* Clear the patterns */
    xSemaphoreTake(lock_, portMAX_DELAY);
    patterns_.clear();
    SavePatterns();

    ResetState();

    xSemaphoreGive(lock_);

    /* Set the return value */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
}

void LEDBorder::GetPatterns(SCommandResponse& rReponse) const
{
    uint8_t i;
    uint8_t pBuffer[512];
    uint8_t patternIdx;


    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Send the number of patterns */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = (uint8_t)patterns_.size();
    pBtManager_->SendCommandResponse(rReponse);

    /* Start sending patterns */
    patternIdx = 0;
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
    while(patternIdx < patterns_.size())
    {
        /* Copy what we can in the buffer */
        for(i = 0;
            i * sizeof(SLEDBorderPattern) < sizeof(pBuffer) &&
            patternIdx < patterns_.size();
            ++i)
        {
            memcpy(
                pBuffer + (i * sizeof(SLEDBorderPattern)),
                &patterns_[patternIdx],
                sizeof(SLEDBorderPattern)
            );
            ++patternIdx;
        }

        /* Send */
        if(pBtManager_->SendData(pBuffer, i * sizeof(SLEDBorderPattern),
                                 SEND_INFO_TIMEOUT) !=
           i * sizeof(SLEDBorderPattern))
        {
            rReponse.header.errorCode = TRANS_SEND_FAILED;
            break;
        }
    }

    xSemaphoreGive(lock_);


}

void LEDBorder::AddAnimation(const uint8_t* kpData, SCommandResponse& rReponse)
{
    const SLEDBorderAnimation* kpAnimation;

    /* Check if we can add a new animation */
    if(animations_.size() >= MAX_ANIM_COUNT)
    {
        rReponse.header.errorCode = NO_MORE_MEMORY;
        rReponse.header.size = 0;
        return;
    }

    kpAnimation = (SLEDBorderAnimation*)kpData;

    /* Check the pattern type */
    if(LED_ANIMATION_MAX_ID <= kpAnimation->type)
    {
        rReponse.header.errorCode = INVALID_PARAM;
        rReponse.header.size = 0;
        return;
    }

    /* Check the pattern bounds */
    if(kpAnimation->startLedIdx >= STRIP_LED_COUNT ||
       kpAnimation->endLedIdx >= STRIP_LED_COUNT ||
       kpAnimation->endLedIdx < kpAnimation->startLedIdx)
    {
        rReponse.header.errorCode = INVALID_INDEX;
        rReponse.header.size = 0;
        return;
    }

    xSemaphoreTake(lock_, portMAX_DELAY);

    animations_.push_back(*kpAnimation);

    /* Set all patterns as non applied */
    for(SLEDBorderPattern& rPattern : patterns_)
    {
        rPattern.applied = false;
    }

    SaveAnimations();

    ResetState();

    xSemaphoreGive(lock_);

    /* Add the animation and set the response to return the new index */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = animations_.size() - 1;
}

void LEDBorder::RemoveAnimation(const uint8_t*    kpAnimationIdx,
                                SCommandResponse& rReponse)
{
    uint8_t i;

    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Check if the index exists */
    if(*kpAnimationIdx >= animations_.size())
    {
        xSemaphoreGive(lock_);

        rReponse.header.errorCode = INVALID_PARAM;
        rReponse.header.size = 0;
        return;
    }

    /* Remove */
    for(i = *kpAnimationIdx; i < animations_.size(); ++i)
    {
        animations_[i] = animations_[i + 1];
    }
    animations_.pop_back();

    SavePatterns();

    ResetState();

    xSemaphoreGive(lock_);

    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
}

void LEDBorder::ClearAnimation(SCommandResponse& rReponse)
{
    /* Clear the animations */
    xSemaphoreTake(lock_, portMAX_DELAY);
    animations_.clear();
    SaveAnimations();

    ResetState();

    xSemaphoreGive(lock_);

    /* Set the return value */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
}

void LEDBorder::GetAnimations(SCommandResponse& rReponse) const
{
    uint8_t i;
    uint8_t pBuffer[512];
    uint8_t animIdx;

    xSemaphoreTake(lock_, portMAX_DELAY);

    /* Send the number of animations */
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 1;
    rReponse.pResponse[0] = (uint8_t)animations_.size();
    pBtManager_->SendCommandResponse(rReponse);

    /* Start sending animations */
    animIdx = 0;
    rReponse.header.errorCode = NO_ERROR;
    rReponse.header.size = 0;
    while(animIdx < animations_.size())
    {
        /* Copy what we can in the buffer */
        for(i = 0;
            i * sizeof(SLEDBorderAnimation) < sizeof(pBuffer) &&
            animIdx < animations_.size();
            ++i)
        {
            memcpy(
                pBuffer + (i * sizeof(SLEDBorderAnimation)),
                &animations_[animIdx],
                sizeof(SLEDBorderAnimation)
            );
            ++animIdx;
        }
        /* Send */
        if(pBtManager_->SendData(pBuffer, i * sizeof(SLEDBorderAnimation),
                                 SEND_INFO_TIMEOUT) !=
           i * sizeof(SLEDBorderAnimation))
        {
            rReponse.header.errorCode = TRANS_SEND_FAILED;
            break;
        }
    }

    xSemaphoreGive(lock_);
}

void LEDBorder::UpdateLEDBorder(void* pParam)
{
    LEDBorder* pBorder;
    uint8_t    i;

    pBorder = (LEDBorder*)pParam;

    while(true)
    {
        if(pBorder->isEnabled_)
        {
            xSemaphoreTake(pBorder->lock_, portMAX_DELAY);

            /* Set the brightness */
            FastLED.setBrightness(
                ((uint32_t)pBorder->brightness_ * FASTLED_MAX_BRIGHTNESS) / 100
            );


            /* Apply patterns */
            for(SLEDBorderPattern& rPattern : pBorder->patterns_)
            {
                pBorder->PATTERN_HANDLERS[rPattern.type](rPattern, pBorder);
            }

            /* Apply animations */
            for(SLEDBorderAnimation& rAnim : pBorder->animations_)
            {
                pBorder->ANIMATION_HANDLERS[rAnim.type](rAnim, pBorder);
            }

            xSemaphoreGive(pBorder->lock_);

            /* Refresh */
            for(i = 0; i < STRIP_LED_COUNT; ++i)
            {
                pBorder->ledsData_[i].setColorCode(pBorder->ledsColors_[i]);
            }
            
            FastLED.show();

            /* Rate limiting */
            HWManager::DelayExecUs(500);
        }
        else
        {
            LOG_DEBUG("Disabling LED Border\n");
            /* Disable the strip */
            digitalWrite(GPIO_LED_ENABLE, LOW);
            vTaskSuspend(nullptr);
            digitalWrite(GPIO_LED_ENABLE, HIGH);
            LOG_DEBUG("Enabling LED Border\n");
        }
    }
}

void LEDBorder::PatternHandlerPlain(SLEDBorderPattern& rPattern,
                                    LEDBorder*         pBorder)
{
    uint8_t  i;
    uint32_t r;
    uint32_t g;
    uint32_t b;
    uint32_t color;

    /* Only apply patterns color fix once */
    if(!rPattern.applied)
    {
        /* Fix color */
        FIX_COLOR(color, r, g, b, rPattern.startColorCode);

        /* Apply the color */
        for(i = rPattern.startLedIdx; i <= rPattern.endLedIdx; ++i)
        {
            pBorder->ledsColors_[i] = color & 0x00FFFFFF;
        }

        rPattern.applied = true;
    }
}

void LEDBorder::PatternHandlerGradient(SLEDBorderPattern& rPattern,
                                       LEDBorder*         pBorder)
{
    uint32_t r;
    uint32_t g;
    uint32_t b;
    uint32_t colorFixStart;
    uint32_t colorFixEnd;
    CRGB     crgbColors[rPattern.endLedIdx - rPattern.startLedIdx + 1];

    /* Only apply patterns color fix once */
    if(!rPattern.applied)
    {
        /* Fix color */
        FIX_COLOR(colorFixStart, r, g, b, rPattern.startColorCode);
        FIX_COLOR(colorFixEnd, r, g, b, rPattern.endColorCode);

        /* Apply the color */
        fill_gradient_RGB(
            crgbColors,
            (uint16_t)(rPattern.endLedIdx - rPattern.startLedIdx + 1),
            colorFixStart,
            colorFixEnd
        );

        for(int i = rPattern.startLedIdx; i <= rPattern.endLedIdx; ++i)
        {
            pBorder->ledsColors_[i] =
                (uint32_t)crgbColors[i - rPattern.startLedIdx] & 0x00FFFFFF;
        }

        rPattern.applied = true;
    }
}

void LEDBorder::AnimHandlerTrail(SLEDBorderAnimation& rAnim,
                                 LEDBorder*           pBorder)
{
    uint8_t  i;
    uint32_t saveColor;

    if(rAnim.speed == 0 || rAnim.step % rAnim.speed == 0)
    {
        /* Get the direction */
        if(rAnim.direction)
        {
            saveColor = pBorder->ledsColors_[rAnim.startLedIdx];
            for(i = rAnim.startLedIdx; i < rAnim.endLedIdx; ++i)
            {
                pBorder->ledsColors_[i] = pBorder->ledsColors_[i + 1];
            }
            pBorder->ledsColors_[i] = saveColor;
        }
        else
        {
            saveColor = pBorder->ledsColors_[rAnim.endLedIdx];
            for(i = rAnim.endLedIdx; i > rAnim.startLedIdx; --i)
            {
                pBorder->ledsColors_[i] = pBorder->ledsColors_[i - 1];
            }
            pBorder->ledsColors_[rAnim.startLedIdx] = saveColor;
        }
    }
    ++rAnim.step;
}

void LEDBorder::AnimHandlerBreath(SLEDBorderAnimation& rAnim,
                                  LEDBorder*           pBorder)
{
    int32_t  brightness;
    uint32_t increment;
    uint8_t  i;
    uint8_t  distance;
    CHSV     targetHSV;

    if(rAnim.speed == 0 || rAnim.step % rAnim.speed == 0)
    {
        distance = pBorder->brightness_ - MIN_BRIGHTNESS;
        increment = (rAnim.step / (rAnim.speed + 1)) %
                    ((distance + 1) * 2);

        if(increment > distance)
        {
            brightness = pBorder->brightness_ - increment + distance;
        }
        else
        {
            brightness = MIN_BRIGHTNESS + increment;
        }

        for(i = rAnim.startLedIdx; i <= rAnim.endLedIdx; ++i)
        {
            if(pBorder->ledsColors_[i] == 0)
            {
                continue;
            }
            targetHSV = rgb2hsv_approximate(pBorder->ledsColors_[i]);
            targetHSV.v = (uint8_t)(brightness * FASTLED_MAX_BRIGHTNESS / 100U);
            pBorder->ledsColors_[i] = (uint32_t)CRGB(targetHSV);
        }
    }
    ++rAnim.step;
}

void LEDBorder::SavePatterns(void) const
{
    FsFile  file;
    FsFile  replaceFile;
    size_t  written;
    uint8_t buffer;

    /* Open TMP file */
    pStore_->Remove(LEDBORDER_PATTERN_TMP_FILE);
    file = pStore_->Open(LEDBORDER_PATTERN_TMP_FILE, FILE_WRITE);

    if(!file)
    {
        LOG_ERROR("Failed to open file to save patterns\n");
        return;
    }

    /* Set the number of patterns */
    buffer = (uint8_t)patterns_.size();
    written = file.write(&buffer, sizeof(uint8_t));
    if(written == 0)
    {
        LOG_ERROR("Failed to write patterns file\n");
        file.close();
        return;
    }

    /* Save */
    for(const SLEDBorderPattern& krPattern : patterns_)
    {
        written = file.write((uint8_t*)&krPattern, sizeof(SLEDBorderPattern));
        if(written == 0)
        {
            LOG_ERROR("Failed to write patterns file\n");
            file.close();
            return;
        }
    }

    /* Overwrite */
    replaceFile = pStore_->Open(LEDBORDER_PATTERN_FILE_PATH, FILE_WRITE);
    if(!replaceFile)
    {
        LOG_ERROR("Failed to open current patterns file\n");
        file.close();
        return;
    }

    if(!replaceFile.rename(LEDBORDER_PATTERN_FILE_PATH ".old"))
    {
        LOG_ERROR("Failed to rename current patterns file\n");
        file.close();
        return;
    }

    if(!file.rename(LEDBORDER_PATTERN_FILE_PATH))
    {
        replaceFile.rename(LEDBORDER_PATTERN_FILE_PATH);
        LOG_ERROR("Failed to replace patterns file\n");
    }
    replaceFile.remove();
    file.close();
}

void LEDBorder::SaveAnimations(void) const
{
    FsFile  file;
    FsFile  replaceFile;
    size_t  written;
    uint8_t buffer;

    /* Open TMP file */
    pStore_->Remove(LEDBORDER_ANIM_TMP_FILE);
    file = pStore_->Open(LEDBORDER_ANIM_TMP_FILE, FILE_WRITE);

    if(!file)
    {
        LOG_ERROR("Failed to open file to save animations\n");
        return;
    }

    /* Set the number of animations */
    buffer = (uint8_t)animations_.size();
    written = file.write(&buffer, sizeof(uint8_t));
    if(written == 0)
    {
        LOG_ERROR("Failed to write animations file\n");
        file.close();
        return;
    }

    /* Save */
    for(const SLEDBorderAnimation& krAnim : animations_)
    {
        written = file.write((uint8_t*)&krAnim, sizeof(SLEDBorderAnimation));
        if(written == 0)
        {
            LOG_ERROR("Failed to write animations file\n");
            file.close();
            return;
        }
    }

    /* Overwrite */
    replaceFile = pStore_->Open(LEDBORDER_ANIM_FILE_PATH, FILE_WRITE);
    if(!replaceFile)
    {
        LOG_ERROR("Failed to open current animations file\n");
        file.close();
        return;
    }

    if(!replaceFile.rename(LEDBORDER_ANIM_FILE_PATH ".old"))
    {
        LOG_ERROR("Failed to rename current animations file\n");
        file.close();
        return;
    }

    if(!file.rename(LEDBORDER_ANIM_FILE_PATH))
    {
        replaceFile.rename(LEDBORDER_ANIM_FILE_PATH);
        LOG_ERROR("Failed to replace animations file\n");
    }
    replaceFile.remove();
    file.close();
}

void LEDBorder::LoadState(void)
{
    FsFile      file;
    std::string content;
    uint8_t     pBuffer[64];
    uint8_t     counter;
    uint8_t     i;

    /* Load the state */
    pStore_->GetContent(LEDBORDER_ENABLED_FILE_PATH, "0", content, true);
    isEnabled_ = (content != "0");

    /* Load the brightness */
    pStore_->GetContent(LEDBORDER_BRIGHTNESS_FILE_PATH, "0", content, true);
    brightness_ = std::stoi(content);
    if(brightness_ > MAX_BRIGHTNESS)
    {
        brightness_ = MAX_BRIGHTNESS;
    }
    else if(brightness_ < MIN_BRIGHTNESS)
    {
        brightness_ = MIN_BRIGHTNESS;
    }

    /* Load the animations */
    file = pStore_->Open(LEDBORDER_ANIM_FILE_PATH, FILE_READ);
    if(file)
    {
        /* Read the number of animations */
        if(file.read(&counter, sizeof(uint8_t)) == sizeof(uint8_t))
        {
            /* Read the animations */
            for(i = 0; i < MIN(MAX_ANIM_COUNT, counter); ++i)
            {
                if(file.read(&pBuffer, sizeof(SLEDBorderAnimation)) ==
                   sizeof(SLEDBorderAnimation))
                {
                    animations_.push_back(*(SLEDBorderAnimation*)pBuffer);
                }
                else
                {
                    LOG_ERROR("Failed to read animation\n");
                }
            }
        }
        else
        {
            LOG_ERROR("Failed to get the number of animations\n")
        }

        file.close();
    }
    else
    {
        LOG_ERROR("Failed to load animations\n");
    }

    /* Load the patterns */
    file = pStore_->Open(LEDBORDER_PATTERN_FILE_PATH, FILE_READ);
    if(file)
    {
        /* Read the number of patterns */
        if(file.read(&counter, sizeof(uint8_t)) == sizeof(uint8_t))
        {
            /* Read the patterns */
            for(i = 0; i < MIN(MAX_PATTERN_COUNT, counter); ++i)
            {
                if(file.read(&pBuffer, sizeof(SLEDBorderPattern)) ==
                   sizeof(SLEDBorderPattern))
                {
                    patterns_.push_back(*(SLEDBorderPattern*)pBuffer);
                    patterns_[patterns_.size() - 1].applied = false;
                }
                else
                {
                    LOG_ERROR("Failed to read pattern\n");
                }
            }
        }
        else
        {
            LOG_ERROR("Failed to get the number of patterns\n")
        }

        file.close();
    }
    else
    {
        LOG_ERROR("Failed to load patterns\n");
    }
}

void LEDBorder::ResetState(void)
{
    memset(ledsColors_, 0, STRIP_LED_COUNT * sizeof(uint32_t));
    /* Set all patterns as non applied */
    for(SLEDBorderPattern& rPattern : patterns_)
    {
        rPattern.applied = false;
    }
}