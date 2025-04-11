/*******************************************************************************
 * @file LEDBorder.h
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

#ifndef __DRIVERS_LEDBORDER_H_
#define __DRIVERS_LEDBORDER_H_

#define FASTLED_INTERNAL

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <vector>         /* std::vector */
#include <cstdint>        /* Generic Types */
#include <Types.h>        /* Custom types */
#include <Storage.h>      /* Storage manager */
#include <FastLED.h>      /* Fast LED Service */
#include <BlueToothMgr.h> /* Bleutooth services */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define STRIP_LED_COUNT 120

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/
class LEDBorder;

typedef enum
{
    LED_PATTERN_PLAIN    = 0,
    LED_PATTERN_GRADIENT = 1,
    LED_PATTERN_MAX_ID   = 2,
} ELEDBorderPatternType;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint32_t startColorCode;
    uint32_t endColorCode;
    uint8_t startLedIdx;
    uint8_t endLedIdx;
    union
    {
        uint8_t applied;
        uint8_t param0;
    };
} SLEDBorderPattern;

typedef enum
{
    LED_ANIMATION_TRAIL  = 0,
    LED_ANIMATION_BREATH = 1,
    LED_ANIMATION_MAX_ID = 2,
} ELEDBorderAnimationType;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t startLedIdx;
    uint8_t endLedIdx;
    union
    {
        uint8_t direction;
        uint8_t param0;
    };
    union
    {
        uint8_t speed;
        uint8_t param1;
    };
    uint32_t step;
} SLEDBorderAnimation;

typedef void (*TPatternHandler)(SLEDBorderPattern&, LEDBorder*);
typedef void (*TAnimationHandler)(SLEDBorderAnimation&, LEDBorder*);

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
class LEDBorder
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        LEDBorder(BluetoothManager* pBtManager);

        void Enable(const bool kEnabled);
        bool IsEnabled(void) const;

        void IncreaseBrightness(SCommandResponse& rReponse);
        void ReduceBrightness(SCommandResponse& rReponse);
        void SetBrightness(const uint8_t* kpData, SCommandResponse& rReponse);

        uint8_t GetBrightness(void) const;

        void Clear(SCommandResponse& rReponse);

        void AddPattern(const uint8_t* kpData, SCommandResponse& rReponse);
        void RemovePattern(const uint8_t*    kpPatternIdx,
                           SCommandResponse& rReponse);
        void ClearPatterns(SCommandResponse& rReponse);
        void GetPatterns(SCommandResponse& rReponse) const;

        void AddAnimation(const uint8_t* kpData, SCommandResponse& rReponse);
        void RemoveAnimation(const uint8_t*    kpAnimationIdx,
                             SCommandResponse& rReponse);
        void ClearAnimation(SCommandResponse& rReponse);
        void GetAnimations(SCommandResponse& rReponse) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        static void UpdateLEDBorder(void* pParam);

        static void PatternHandlerPlain(SLEDBorderPattern& rPattern,
                                        LEDBorder*         pBorder);
        static void PatternHandlerGradient(SLEDBorderPattern& rPattern,
                                           LEDBorder*         pBorder);
        static void AnimHandlerTrail(SLEDBorderAnimation& rAnim,
                                     LEDBorder*           pBorder);
        static void AnimHandlerBreath(SLEDBorderAnimation& rAnim,
                                      LEDBorder*           pBorder);

        void SavePatterns(void) const;
        void SaveAnimations(void) const;

        void LoadState(void);
        void ResetState(void);

        BluetoothManager* pBtManager_;

        bool                          isEnabled_;
        uint8_t                       brightness_;

        uint32_t                      ledsColors_[STRIP_LED_COUNT];
        CRGB                          ledsData_[STRIP_LED_COUNT];

        std::vector<SLEDBorderAnimation> animations_;
        std::vector<SLEDBorderPattern>   patterns_;

        Storage*                      pStore_;
        TaskHandle_t                  workerThread_;
        SemaphoreHandle_t             lock_;

        static TPatternHandler PATTERN_HANDLERS[LED_PATTERN_MAX_ID];
        static TAnimationHandler ANIMATION_HANDLERS[LED_ANIMATION_MAX_ID];
};


#endif /* #ifndef __DRIVERS_LEDBORDER_H_ */