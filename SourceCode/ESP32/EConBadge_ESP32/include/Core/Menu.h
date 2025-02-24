/*******************************************************************************
 * @file Menu.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/02/2025
 *
 * @version 2.0
 *
 * @brief This file defines the menu classes.
 *
 * @details This file defines the menu classes. This module provides the service
 * for menu management and action trigger.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_MENU_H_
#define __CORE_MENU_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>        /* std::string */
#include <cstdint>       /* Generic types */
#include <IOButtonMgr.h> /* Button manager */

/* Forward declaration */
class DisplayInterface;


/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

 #define LINE_SIZE_CHAR 21

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    MENU_NO_ACTION,
    MENU_ACTION_NEXT_PAGE,
    MENU_ACTION_PREVIOUS_PAGE,
    MENU_ACTION_CLEAR_EINK,
    MENU_ACTION_SET_EINK_IMAGE,
    MENU_ACTION_DISPLAY_ABOUT_PAGE,
    MENU_ACTION_FACTORY_RESET
} EMenuAction;

typedef struct
{
    char        pContent[LINE_SIZE_CHAR * 2 + 1];
    EMenuAction action;
    void*       actionParams;
} SMenuItem;

typedef struct SMenuPage
{
    const char* pTitle;

    uint8_t selectedItem;

    std::vector<SMenuItem*> items;

    void (*updater)(struct SMenuPage*);
    uint8_t (*scroller)(struct SMenuPage*, const bool kDown);

    std::vector<struct SMenuPage*> pNextPages;
    struct SMenuPage* pPrevPage;

    bool needsUpdate;
} SMenuPage;


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
class Menu
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        Menu(DisplayInterface* pDisplay);

        void SendButtonAction(const SButtonAction& rkBtnAction,
                              SCommandRequest& rCommandRequest);

        void Display(void);
        void Reset(void);


    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        static void UpdateMyInfoPage(SMenuPage* pPage);
        static void UpdateEInkImageListPage(SMenuPage* pPage);
        static void UpdateBluetoothInfo(SMenuPage* pPage);

        static uint8_t ScrollEInkImageListPage(SMenuPage* pPage,
                                               const bool kDown);

        void HandleMenuAction(const SMenuItem* pkAction,
                              SCommandRequest& rCommandRequest);

        DisplayInterface* pDisplay_;
        SMenuPage*        pMainPage_;
        SMenuPage*        pCurrentPage_;
};

#endif /* #ifndef __CORE_MENU_H_ */