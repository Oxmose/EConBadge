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

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Standard Int Types */
#include <vector>  /* std vectors */
#include <Types.h> /* Defined types */

#include <SystemState.h>      /* System State Service */
#include <OLEDScreenMgr.h> /* OLED Screen Driver */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Core Namespace
 * @details Core Namespace used to gather the core services of the ESP32 module.
 */
namespace nsCore
{

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    MAIN_PAGE_IDX         = 0,
    WIFI_PAGE_IDX         = 1,
    UPDATE_PAGE_IDX       = 2,
    MAINTAINANCE_PAGE_IDX = 3,
    ABOUT_PAGE_IDX        = 4,
    WIFI_CLIENT_PAGE_IDX  = 5,
    MAX_PAGE_IDX
} EMenuPageIdx;

typedef enum
{
    /* Main Page Idx */
    MAINP_WIFI_ITEM_IDX         = 0,
    MAINP_UPDATE_ITEM_IDX       = 1,
    MAINP_MAINTAINANCE_ITEM_IDX = 2,
    MAINP_ABOUT_ITEM_IDX        = 3,

    /* WiFi Page Idx */
    WIFIP_BACK_ITEM_IDX   = 0,

    /* Maintainance Page Idx */
    MAINTAINANCEP_BACK_ITEM_IDX  = 0,
    MAINTAINANCEP_CLEAN_ITEM_IDX  = 1,

    /* About Page Idx */
    ABOUTP_BACK_ITEM_IDX  = 0
} EMenuItemIdx;

/* Forward declaration */
class CSystemState;

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

/* Forward declarations */
class CMenuPage;
class CMenu;

class CMenuItemAction
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CMenuItemAction(CMenuPage * parentPage, CMenu * parentMenu);
        virtual ~CMenuItemAction(void) {}
        virtual nsCommon::EErrorCode Execute(CSystemState & sysState) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        CMenuPage * parentPage;
        CMenu     * parentMenu;
    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
};

class CMenuItem
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CMenuItem(CMenuPage * parentPage, CMenuItemAction * action,
                  const char * itemText, const bool isSelectable);

        nsCommon::EErrorCode PerformAction(CSystemState & sysState);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        const char *      itemText;
        bool              isSelectable;
        CMenuItemAction * action;
        CMenuPage *       parentPage;

    friend class CMenuPage;
};

class CMenuPage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CMenuPage(CMenuPage * parentPage, const char * pageTitle);

        void AddItem(CMenuItem * item);
        nsCommon::EErrorCode PerformAction(CSystemState & sysState);
        void Display(nsHWL::COLEDScreenMgr * oledScreen, const String & popUp) const;
        void SelectNextItem(void);
        void SelectPrevItem(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

        const char *            pageTitle;
        uint8_t                 selectedItemIdx;
        std::vector<CMenuItem*> items;
        CMenuPage *             parentPage;
};

class CMenu
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CMenu(void);

        void Display(nsHWL::COLEDScreenMgr * oledScreen);
        void ForceUpdate(void);
        void SetPage(const EMenuPageIdx pageIdx);

        void PrintPopUp(const String & str);
        void ClosePopUp(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void AddPage(CMenuPage * page, const EMenuPageIdx pageIdx);

        void SelectNextItem(void);
        void SelectPrevItem(void);
        void ExecuteAction(void);

        CMenuItemAction * CreateItemAction(CMenuPage * page,
                                           const EMenuPageIdx pageIdx,
                                           const EMenuItemIdx itemIdx);

        std::vector<CMenuPage*> pages;
        uint8_t                 currPageIdx;
        bool                    needUpdate;
        String                  currPopUp;
};

class IMenuUpdater
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        virtual ~IMenuUpdater(void) {};
        virtual void operator()(CSystemState & sysState) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_MENU_H_ */