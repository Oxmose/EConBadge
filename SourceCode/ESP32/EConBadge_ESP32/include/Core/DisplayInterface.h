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

class DisplayInterface
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        DisplayInterface(OLEDScreenMgr* pOLEDScreen);

        void Enable(const bool kEnabled);

        void DisplayPopup(const std::string& rkTitle,
                          const std::string& rkContent);

        void HidePopup(void);

        void DisplayPage(const std::string&              rkTitle,
                         const std::vector<std::string>& rkItems,
                         const int8_t                    kSelected);

        void DrawImage(const uint8_t* pkBitmap,
                       const uint8_t  kXPos,
                       const uint8_t  kYPos,
                       const uint8_t  kWidth,
                       const uint8_t  kHeight);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        bool           isEnabled_;
        bool           hasPopup_;
        bool           updateRequested_;
        OLEDScreenMgr* pOLEDScreen_;
};

#endif /* #ifndef __CORE_DISPLAY_INTERFACE_H_ */