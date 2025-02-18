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

#include <string>             /* std::string */
#include <vector>             /* std::vector */
#include <cstdint>            /* Standard Int Types */
#include <Types.h>            /* Defined types */
#include <Updater.h>          /* Updater manager */
#include <LEDBorder.h>        /* LED Border manager */
#include <BatteryMgr.h>       /* Battery manager */
#include <SystemState.h>      /* System state manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <OLEDScreenMgr.h>    /* OLED Screen manager */
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

/** @brief Defines the menu page indices. */
typedef enum
{
    /** @brief Main page index. */
    MAIN_PAGE_IDX         = 0,
    /** @brief Information page index. */
    MY_INFO_PAGE_IDX      = 1,
    /** @brief Display page index. */
    DISPLAY_PAGE_IDX      = 2,
    /** @brief LED settings page index. */
    LED_SETTINGS_PAGE_IDX = 3,
    /** @brief System page index. */
    SYSTEM_PAGE_IDX       = 4,
    /** @brief About page index. */
    ABOUT_PAGE_IDX        = 5,
    /** @brief Update image page index. */
    UPDATE_IMG_PAGE_IDX   = 6,
    /** @brief Bluetooth page index. */
    BLUETOOTH_PAGE_IDX    = 7,
    /** @brief Reset page index. */
    RESET_PAGE_IDX        = 8,
    /** @brief System information page index. */
    SYSINFO_PAGE_IDX      = 9,
    /** @brief Maximal page index. */
    MAX_PAGE_IDX
} EMenuPageIdx;

/** @brief Defines the menu items indices. */
typedef enum
{
    /* Main Page Idx */
    MAINP_MYINFO_ITEM_IDX       = 0,
    MAINP_DISPLAY_ITEM_IDX      = 1,
    MAINP_LED_SETTINGS_ITEM_IDX = 2,
    MAINP_SYSTEM_ITEM_IDX       = 3,
    MAINP_ABOUT_ITEM_IDX        = 4,
    MAINP_MAX_ITEM_IDX          = 5,

    /* My Info Page Idx */
    MY_INFOP_OWNER_ITEM_IDX   = 0,
    MY_INFOP_CONTACT_ITEM_IDX = 1,
    MY_INFOP_MAX_ITEM_IDX     = 2,

    /* Display Page Idx */
    DISPLAYP_CLEAR_ITEM_IDX      = 0,
    DISPLAYP_TOGGLE_OL_ITEM_IDX  = 1,
    DISPLAYP_UPDATE_IMG_ITEM_IDX = 2,
    DISPLAYP_MAX_ITEM_IDX        = 3,

    /* LED Settings Page Idx */
    LED_SETTINGSP_TOGGLE_ITEM_IDX     = 0,
    LED_SETTINGSP_INC_BRIGHT_ITEM_IDX = 1,
    LED_SETTINGSP_RED_BRIGHT_ITEM_IDX = 2,
    LED_SETTINGSP_MAX_ITEM_IDX        = 3,

    /* System Page Idx */
    SYSTEMP_BLUETOOTH_ITEM_IDX = 0,
    SYSTEMP_UPDATE_ITEM_IDX    = 1,
    SYSTEMP_RESET_ITEM_IDX     = 2,
    SYSTEMP_SYSINFO_ITEM_IDX   = 3,
    SYSTEMP_MAX_ITEM_IDX       = 4,

    /* About Page Idx */
    ABOUTP_MAX_ITEM_IDX = 0,

    /* Update Image Page Idx */
    UPDIMGP_IMG0_ITEM_IDX = 0,
    UPDIMGP_IMG1_ITEM_IDX = 1,
    UPDIMGP_IMG2_ITEM_IDX = 2,
    UPDIMGP_IMG3_ITEM_IDX = 3,
    UPDIMGP_IMG4_ITEM_IDX = 4,
    UPDIMGP_MAX_ITEM_IDX  = 5,

    /* Bluetooth Page Idx */
    BLUETOOTHP_TOKEN_ITEM_IDX = 0,
    BLUETOOTHP_MAX_ITEM_IDX   = 1,

    /* Factory Reset Page Idx */
    RESETP_INFO_ITEM_IDX = 0,
    RESETP_NONE_ITEM_IDX = 1,
    RESETP_NO_ITEM_IDX   = 2,
    RESETP_YES_ITEM_IDX  = 3,
    RESETP_MAX_ITEM_IDX  = 4,

    /* System Info Page Idx */
    SYSINFOP_SWVERS_IDX   = 0,
    SYSINFOP_HWVERS_IDX   = 1,
    SYSINFOP_MAC_IDX      = 2,
    SYSINFOP_MAX_ITEM_IDX = 3
} EMenuItemIdx;

/** @brief Defines the scroll behavior of the menu. */
typedef enum
{
    /** @brief No scroll. */
    SCROLL_NONE      = 0,
    /** @brief Image list scroll behavior. */
    SCROLL_IMAGELIST = 1,
} EScrollBehavior;


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
        MenuItemAction(Menu* pParentMenu);
        virtual ~MenuItemAction(void);

        virtual EErrorCode Execute(void) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        Menu* pParentMenu_;
    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /* None */
};

class MenuItem
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuItem(MenuItemAction* pAction,
                 const char*     pkItemText,
                 const bool      kIsSelectable);

        EErrorCode PerformAction(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        const bool      kIsSelectable_;
        const char*     pkItemText_;

        MenuItemAction* pAction_;

    friend class MenuPage;
};

class MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPage(OLEDScreenMgr*     pOledScreen,
                 const char*        pkPageTitle,
                 const EMenuPageIdx kParentPageIdx);

        virtual void AddItem(MenuItem* pItem);

        virtual EErrorCode PerformAction(void);

        virtual void Display(const std::string& rkPopUp, Menu* pMenu);

        EMenuPageIdx GetParentPageIdx(void) const;

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t kIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        bool                   hasPopup_;
        uint8_t                selectedItemIdx_;
        const char*            pkPageTitle_;

        std::vector<MenuItem*> items_;

        OLEDScreenMgr*         pOledScreen_;
        const EMenuPageIdx     kParentPageIdx_;


    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
            /* None */

};

class MenuPageImageScroll : public MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPageImageScroll(OLEDScreenMgr*     pOledScreen,
                            const char*        pkPageTitle,
                            const EMenuPageIdx kParentPageIdx,
                            ImageList*         pImageList);

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t kIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void UpdateItems(void);

        ImageList* pImageList_;
        Storage*   pStore_;

};

class MenuPageAbout : public MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPageAbout(OLEDScreenMgr*     pOledScreen,
                      const char*        pkPageTitle,
                      const EMenuPageIdx kParentPageIdx);

        virtual void AddItem(MenuItem * pItem);

        virtual EErrorCode PerformAction(void);

        virtual void Display(const std::string& rkPopUp, Menu* pMenu);

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t kIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /* None */

};

class Menu
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        Menu(OLEDScreenMgr*      pOledScreen,
             SystemState*        pSystemState,
             EInkDisplayManager* pEInkScreen,
             LEDBorder*          pLedBorder,
             Updater*            pUpdater,
             BluetoothManager*   pBtMgr,
             BatteryMgr*         pBatteryMgr);

        void Update(void);
        void ForceUpdate(void);

        void PrintPopUp(const std::string& rkStr);
        void ClosePopUp(void);
        bool HasPopup(void) const;

        void SetPage(const EMenuPageIdx kPageIdx);
        void SetCurrentSelectedItem(const uint8_t kIdx);

        void PrintBattery(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void DisplayDebug(const uint8_t kDebugState);


        void SelectNextItem(void);
        void SelectPrevItem(void);
        void ExecuteSelection(void);
        void ExecuteBack(void);

        void AddPage(MenuPage* pPage, const EMenuPageIdx kPageIdx);
        MenuItemAction* CreateItemAction(MenuPage*          pPage,
                                         const EMenuPageIdx kPageIdx,
                                         const EMenuItemIdx kItemIdx);

        uint8_t                currPageIdx_;
        bool                   needUpdate_;
        bool                   wasUpdating_;
        uint8_t                lastBatteryAnimVal_;

        std::vector<MenuPage*> pages_;
        std::string            currPopUp_;

        ESystemState           prevSystemSate_;
        ImageList              imageList_;
        OLEDScreenMgr*         pOledScreen_;
        SystemState*           pSystemState_;
        EInkDisplayManager*    pEInkScreen_;
        LEDBorder*             pLedBorder_;
        Updater*               pUpdater_;
        BluetoothManager*      pBtMgr_;
        Storage*               pStore_;
        BatteryMgr*            pBatteryMgr_;

};

#endif /* #ifndef __CORE_MENU_H_ */