/*******************************************************************************
 * @file Menu.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 24/12/2022
 *
 * @version 1.0
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

#include <cstring> /* String */
#include <cstdint> /* Standard Int Types */
#include <vector>  /* std vectors */
#include <Types.h> /* Defined types */
#include <OLEDScreenMgr.h> /* OLED Screen manager */
#include <SystemState.h> /* System state manager */
#include <WaveshareEInkMgr.h> /* eInk Display manager */

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

typedef enum
{
    MAIN_PAGE_IDX         = 0,
    BLUETOOTH_PAGE_IDX    = 1,
    UPDATE_PAGE_IDX       = 2,
    MAINTAINANCE_PAGE_IDX = 3,
    ABOUT_PAGE_IDX        = 4,
    MAX_PAGE_IDX          = 5
} EMenuPageIdx;

typedef enum
{
    /* Main Page Idx */
    MAINP_BLUETOOTH_ITEM_IDX    = 0,
    MAINP_UPDATE_ITEM_IDX       = 1,
    MAINP_MAINTAINANCE_ITEM_IDX = 2,
    MAINP_ABOUT_ITEM_IDX        = 3,

    /* Bluetooth Page Idx */
    BTP_BACK_ITEM_IDX   = 0,

    /* Update Page Idx */
    UPDATEP_BACK_ITEM_IDX   = 0,

    /* Maintainance Page Idx */
    MAINTAINANCEP_BACK_ITEM_IDX  = 0,
    MAINTAINANCEP_CLEAN_ITEM_IDX  = 1,

    /* About Page Idx */
    ABOUTP_BACK_ITEM_IDX  = 0
} EMenuItemIdx;

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
class MenuPage;
class Menu;

class MenuItemAction
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuItemAction(MenuPage * parentPage, Menu * parentMenu);
        virtual ~MenuItemAction(void) {}

        virtual EErrorCode Execute(void) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        MenuPage * parentPage_;
        Menu     * parentMenu_;
    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
};

class MenuItem
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuItem(MenuPage * parentPage,
                 MenuItemAction * action,
                 const char * itemText,
                 const bool isSelectable);

        EErrorCode PerformAction(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        const char *     itemText_;
        bool             isSelectable_;
        MenuItemAction * action_;
        MenuPage *       parentPage_;

    friend class MenuPage;
};

class MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPage(OLEDScreenMgr * oledScreen,
                 MenuPage * parentPage,
                 const char * pageTitle);

        void AddItem(MenuItem * item);

        EErrorCode PerformAction(void);

        void Display(const String & popUp) const;

        void SelectNextItem(void);
        void SelectPrevItem(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        const char *           pageTitle_;
        uint8_t                selectedItemIdx_;
        std::vector<MenuItem*> items_;
        MenuPage *             parentPage_;
        OLEDScreenMgr *        oledScreen_;
};

class Menu
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        Menu(OLEDScreenMgr * oledScreen, SystemState * systemState,
             EInkDisplayManager * eInkScreen);

        void Update(void);
        void ForceUpdate(void);

        void PrintPopUp(const String & str);
        void ClosePopUp(void);

        void SetPage(const EMenuPageIdx pageIdx);
    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void DisplayDebug(const uint8_t debugState);


        void SelectNextItem(void);
        void SelectPrevItem(void);
        void ExecuteSelection(void);

        void AddPage(MenuPage * page, const EMenuPageIdx pageIdx);
        MenuItemAction * CreateItemAction(MenuPage * page,
                                          const EMenuPageIdx pageIdx,
                                          const EMenuItemIdx itemIdx);

        std::vector<MenuPage*> pages_;
        uint8_t                currPageIdx_;
        bool                   needUpdate_;
        String                 currPopUp_;
        ESystemState           prevSystemSate_;
        OLEDScreenMgr *        oledScreen_;
        SystemState *          systemState_;
        EInkDisplayManager *   eInkScreen_;
};

class IMenuUpdater
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        virtual ~IMenuUpdater(void) {};
        virtual void operator()(void) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
};


#endif /* #ifndef __CORE_MENU_H_ */