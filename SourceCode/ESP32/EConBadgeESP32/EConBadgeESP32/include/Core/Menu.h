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
#include <LEDBorder.h>        /* LED Border manager */
#include <Updater.h>          /* Updater manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */

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
    MY_INFO_PAGE_IDX      = 1,
    DISPLAY_PAGE_IDX      = 2,
    LED_SETTINGS_PAGE_IDX = 3,
    SYSTEM_PAGE_IDX       = 4,
    ABOUT_PAGE_IDX        = 5,
    UPDATE_IMG_PAGE_IDX   = 6,
    BLUETOOTH_PAGE_IDX    = 7,
    RESET_PAGE_IDX        = 8,
    SYSINFO_PAGE_IDX      = 9,
    MAX_PAGE_IDX
} EMenuPageIdx;

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
    BLUETOOTHP_TOGGLE_ITEM_IDX = 0,
    BLUETOOTHP_NAME_ITEM_IDX   = 1,
    BLUETOOTHP_PIN_ITEM_IDX    = 2,
    BLUETOOTHP_MAX_ITEM_IDX    = 3,

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

typedef enum
{
    SCROLL_NONE      = 0,
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
        MenuItemAction(Menu * parentMenu);
        virtual ~MenuItemAction(void) {}

        virtual EErrorCode Execute(void) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        Menu     * parentMenu_;
    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
};

class MenuItem
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuItem(MenuItemAction * action,
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

    friend class MenuPage;
};

class MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPage(OLEDScreenMgr * oledScreen,
                 const char * pageTitle,
                 const EMenuPageIdx parentPageIdx);

        virtual void AddItem(MenuItem * item);

        virtual EErrorCode PerformAction(void);

        virtual void Display(const String & popUp);

        EMenuPageIdx GetParentPageIdx(void) const;

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t idx, const uint32_t listIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        const char *           pageTitle_;
        uint8_t                selectedItemIdx_;
        std::vector<MenuItem*> items_;
        OLEDScreenMgr *        oledScreen_;
        bool                   hasPopup_;
        EMenuPageIdx           parentPageIdx_;

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

};

class MenuPageImageScroll : public MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPageImageScroll(OLEDScreenMgr * oledScreen,
                            const char * pageTitle,
                            const EMenuPageIdx parentPageIdx,
                            std::vector<std::string> * imageList);

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t idx, const uint32_t listIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        std::vector<std::string> * imageList_;
        uint32_t                   startDisplayListIdx_;

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void UpdateItems(void);

};

class MenuPageAbout : public MenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        MenuPageAbout(OLEDScreenMgr * oledScreen,
                      const char * pageTitle,
                      const EMenuPageIdx parentPageIdx);

        virtual void AddItem(MenuItem * item);

        virtual EErrorCode PerformAction(void);

        virtual void Display(const String & popUp);

        virtual void SelectNextItem(void);
        virtual void SelectPrevItem(void);
        virtual void SetSelectedItem(const uint8_t idx, const uint32_t listIdx);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

};

class Menu
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        Menu(OLEDScreenMgr * oledScreen, SystemState * systemState,
             EInkDisplayManager * eInkScreen, LEDBorder * ledBorder,
             Updater * updater, BluetoothManager * btMgr);

        void Update(void);
        void ForceUpdate(void);

        void PrintPopUp(const String & str);
        void ClosePopUp(void);
        bool HasPopup(void) const;

        void SetPage(const EMenuPageIdx pageIdx);
        void SetCurrentSelectedItem(const uint8_t idx,
                                    const uint32_t listIdx /*= 0*/);
    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void DisplayDebug(const uint8_t debugState);

        void SelectNextItem(void);
        void SelectPrevItem(void);
        void ExecuteSelection(void);
        void ExecuteBack(void);

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
        LEDBorder *            ledBorder_;
        Updater *              updater_;
        BluetoothManager *     btMgr_;
        bool                   wasUpdating_;

        std::vector<std::string> imageList_;
};

#endif /* #ifndef __CORE_MENU_H_ */