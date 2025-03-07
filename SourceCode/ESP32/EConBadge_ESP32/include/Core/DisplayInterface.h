/*******************************************************************************
 * @file DisplayInterface.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/02/2025
 *
 * @version 1.0
 *
 * @brief This file defines the types user interface manager.
 *
 * @details This file defines the types user interface manager. The manager
 * displays and compose what is sent to the screen.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_DISPLAY_INTERFACE_H_
#define __CORE_DISPLAY_INTERFACE_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>          /* std::string */
#include <vector>          /* std::vector */
#include <Menu.h>          /* Menu manager */
#include <BatteryMgr.h>    /* Battery manager */
#include <OLEDScreenMgr.h> /* OLED screen manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/
/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/
/** @brief Defines the debug informations */
typedef struct
{
    uint8_t debugState;
    uint8_t systemState;
    uint64_t lastEventTime;
    uint8_t buttonsState[BUTTON_MAX_ID];
    uint64_t buttonsKeepTime[BUTTON_MAX_ID];
    uint32_t batteryState;
} SDebugInfo_t;

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

class DisplayInterface
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        DisplayInterface(OLEDScreenMgr*  pOLEDScreen,
                         BatteryManager* pBatteryManager);

        void Enable(const bool kEnabled);

        void DisplayPage(SMenuPage* pkPage);

        void DisplayPopup(const std::string& rkTitle,
                          const std::string& rkContent);

        void HidePopup(void);

        void SetDebugDisplay(const SDebugInfo_t& rkDebugState);

        void DrawImage(const uint8_t* pkBitmap,
                       const uint8_t  kXPos,
                       const uint8_t  kYPos,
                       const uint8_t  kWidth,
                       const uint8_t  kHeight);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        static void UpdateScreen(void*);
        void PrintBattery(void);
        void InternalDisplayPage(void);
        void InternalDisplayPopup(void);
        void DisplayDebug(void);
        void DisplaySplash(void);

        TaskHandle_t      uiThread_;
        SemaphoreHandle_t menuPageLock_;

        SMenuPage*       pkCurrentPage_;
        bool             isEnabled_;
        OLEDScreenMgr*   pOLEDScreen_;
        BatteryManager*  pBatteryManager_;
        uint8_t          lastBatteryAnimVal_;

        SDebugInfo_t debugInfo_;

        bool displayPopup_;
        std::string popupTitle_;
        std::string popupContent_;
};

#endif /* #ifndef __CORE_DISPLAY_INTERFACE_H_ */