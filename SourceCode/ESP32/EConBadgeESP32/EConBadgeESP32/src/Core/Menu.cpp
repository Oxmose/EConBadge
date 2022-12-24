/*******************************************************************************
 * @file Menu.cpp
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>        /* String manipulation*/
#include <Types.h>        /* Defined Types */
#include <Logger.h>       /* Logger Service */
#include <version.h>      /* Versioning */
#include <HWLayer.h>      /* Hardware layer */

/* Header File */
#include <Menu.h>

#include <SystemState.h> /* System State service */

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CMACTION nsCore::CMenuItemAction
/** @brief Class namespace shortcut. */
#define CMITEM nsCore::CMenuItem
/** @brief Class namespace shortcut. */
#define CMPAGE nsCore::CMenuPage
/** @brief Class namespace shortcut. */
#define CMENU nsCore::CMenu

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

/* In RAM (dynamically editable) */
char dynProtoRev[32] = PROTO_REV " ";

char * MENU_PAGE_ITEM_WIFI[5] = {
    "Back\n",
    "Status: Disabled",
    "IP: 888.888.888.888",
    "SSID: 123456789111315",
    "PASS: 123456789111315"
};

/* In Flash (constant) */
static const char * MENU_PAGE_TITLES[nsCore::MAX_PAGE_IDX] = {
    "Control Center",
    "WiFi Control",
    "Bluetooth Control",
    "Software Update",
    "About EConBadge"
};

static const uint8_t MENU_PAGE_ITEM_COUNT[nsCore::MAX_PAGE_IDX] = {
    4,
    5,
    4,
    4,
    4
};

static const char * MENU_PAGE_ITEM_MAIN[4] = {
    "WiFi Control",
    "Bluetooth Control",
    "Software Update",
    "About EConBadge"
};

static const char * MENU_PAGE_ITEM_ABOUT[4] = {
    "Back",
    "Created by Olson\nTelegram @Olson_T\n",
    "SW " VERSION_SHORT,
    dynProtoRev,
};

static const bool MENU_PAGE_ITEM_MAIN_SEL[4] = {
    true, true, true, true
};

static const bool MENU_PAGE_ITEM_WIFI_SEL[5] = {
    true, false, false, false, false
};

static const bool MENU_PAGE_ITEM_ABOUT_SEL[4] = {
    true, false, false, false
};

static const char ** MENU_PAGE_ITEMS[nsCore::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN,
    (const char **)MENU_PAGE_ITEM_WIFI,
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_ABOUT
};

static const bool * MENU_PAGE_ITEMS_SEL[nsCore::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN_SEL,
    MENU_PAGE_ITEM_WIFI_SEL,
    MENU_PAGE_ITEM_MAIN_SEL,
    MENU_PAGE_ITEM_MAIN_SEL,
    MENU_PAGE_ITEM_ABOUT_SEL
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

/******************** CMenuItemAction Definitions ********************/
namespace nsCore
{
    class CActionChangePage : public CMenuItemAction
    {
        public:
            CActionChangePage(nsCore::CMenuPage * parentPage,
                              nsCore::CMenu * parentMenu,
                              const EMenuPageIdx pageIdx) : CMenuItemAction(parentPage, parentMenu)
            {
                this->pageIdx = pageIdx;
            }
            virtual ~CActionChangePage(void){}

            virtual EErrorCode Execute(nsCore::CSystemState & sysState)
            {
                this->parentMenu->SetPage(this->pageIdx);

                return NO_ERROR;
            }
        private:
            EMenuPageIdx pageIdx;
    };

    class CActionDisplayAbout : public CMenuItemAction
    {
        public:
            CActionDisplayAbout(nsCore::CMenuPage * parentPage,
                                nsCore::CMenu * parentMenu) : CMenuItemAction(parentPage, parentMenu)
            {
            }
            virtual ~CActionDisplayAbout(void){}

            virtual EErrorCode Execute(nsCore::CSystemState & sysState)
            {
                char uniqueHWUID[HW_ID_LENGTH];

                /* Update the HW value */
                if(strlen(dynProtoRev) <= strlen(PROTO_REV) + 2)
                {
                    nsHWL::CHWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);
                    strncpy(dynProtoRev + strlen(dynProtoRev), uniqueHWUID, 14);
                }

                this->parentMenu->SetPage(ABOUT_PAGE_IDX);

                return NO_ERROR;
            }
        private:
    };

    class CActionWifiMenu : public CMenuItemAction
    {
        /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
        public:
            CActionWifiMenu(nsCore::CMenuPage * parentPage,
                            nsCore::CMenu * parentMenu) : CMenuItemAction(parentPage, parentMenu)
            {
            }
            virtual ~CActionWifiMenu(void){}

            virtual EErrorCode Execute(nsCore::CSystemState & sysState)
            {
                /* Sets the system state to WIFI Mode */
                sysState.SetSystemState(SYS_MENU_WIFI);
                //sysState.SetStateMenuPageUpdater(UpdateMenuPage);

                this->parentMenu->SetPage(WIFI_PAGE_IDX);

                return NO_ERROR;
            }

            void UpdateMenuPage(nsCore::CSystemState & sysState)
            {

            }
        private:
    };

    class CActionExitWifiMenu : public CMenuItemAction
    {
        /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
        public:
            CActionExitWifiMenu(nsCore::CMenuPage * parentPage,
                                nsCore::CMenu * parentMenu) : CMenuItemAction(parentPage, parentMenu)
            {
            }
            virtual ~CActionExitWifiMenu(void){}

            virtual EErrorCode Execute(nsCore::CSystemState & sysState)
            {
                /* Sets the system state to WIFI Mode */
                sysState.SetSystemState(SYS_MENU);
                //sysState.SetStateMenuPageUpdater(UpdateMenuPage);

                this->parentMenu->SetPage(MAIN_PAGE_IDX);

                return NO_ERROR;
            }

            void UpdateMenuPage(nsCore::CSystemState & sysState)
            {

            }
        private:
    };
}

CMACTION::CMenuItemAction(nsCore::CMenuPage * parentPage, nsCore::CMenu * parentMenu)
{
    this->parentPage = parentPage;
    this->parentMenu = parentMenu;
}

/******************** CMenuItem Definitions ********************/
CMITEM::CMenuItem(nsCore::CMenuPage * parentPage, nsCore::CMenuItemAction * action,
                    const char * itemText, const bool isSelectable)
{
    this->parentPage   = parentPage;
    this->action       = action;
    this->itemText     = itemText;
    this->isSelectable = isSelectable;
}

EErrorCode CMITEM::PerformAction(nsCore::CSystemState & sysState)
{
    if(this->action != nullptr)
    {
        return this->action->Execute(sysState);
    }

    return NOT_INITIALIZED;
}

/******************** CMenuPage Definitions ********************/
CMPAGE::CMenuPage(nsCore::CMenuPage * parentPage, const char * pageTitle)
{
    this->parentPage = parentPage;
    this->pageTitle  = pageTitle;

    this->selectedItemIdx = 0;
}

void CMPAGE::AddItem(CMenuItem * item)
{
    this->items.push_back(item);
}

EErrorCode CMPAGE::PerformAction(nsCore::CSystemState & sysState)
{
    if(this->items[this->selectedItemIdx] != nullptr)
    {
        return this->items[this->selectedItemIdx]->PerformAction(sysState);
    }

    return NOT_INITIALIZED;
}

void CMPAGE::Display(nsHWL::COLEDScreenMgr * oledScreen) const
{
    Adafruit_SSD1306 * display;
    uint8_t            i;

    display = oledScreen->GetDisplay();

     /* Init Print */
    display->ssd1306_command(SSD1306_DISPLAYON);
    display->clearDisplay();
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->setCursor(0, 0);

    LOG_DEBUG("Displaying Menu\n");

    /* Print menu title */
    display->printf("%s\n---------------------", this->pageTitle);

    /* Print items */
    /* TODO: Scrolling feature */
    for(i = 0; i < this->items.size(); ++i)
    {
        /* If selectable or selected, print selection character */
        if(i == this->selectedItemIdx)
        {
           display->printf("> ");
        }
        else if(this->items[i]->isSelectable)
        {
            display->printf("  ");
        }

        display->printf("%s\n", this->items[i]->itemText);
    }

    display->display();
}

void CMPAGE::SelectNextItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    itemSize = this->items.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(this->items[(selectedItemIdx + i) % itemSize]->isSelectable)
        {
            this->selectedItemIdx = (selectedItemIdx + i) % itemSize;
            break;
        }
    }
}

void CMPAGE::SelectPrevItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    itemSize = this->items.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(this->items[(selectedItemIdx + itemSize - i) % itemSize]->isSelectable)
        {
            this->selectedItemIdx = (selectedItemIdx + itemSize - i) % itemSize;
            break;
        }
    }
}

/******************** CMenu Definitions ********************/
CMENU::CMenu(void)
{
    nsCore::CMenuPage * page;
    nsCore::CMenuItem * item;
    uint8_t             i;
    uint8_t             j;

    /* Setup pages */
    this->pages.resize(MAX_PAGE_IDX);
    this->currPageIdx = nsCore::MAIN_PAGE_IDX;

    LOG_DEBUG("Creating Menu With %d pages\n", MAX_PAGE_IDX);

    /* Create Pages */
    for(i = 0; i < nsCore::MAX_PAGE_IDX; ++i)
    {
        page = new nsCore::CMenuPage(nullptr, MENU_PAGE_TITLES[i]);
        if(page == nullptr)
        {
            LOG_CRITICAL("Could not allocate menu page.");
        }

        LOG_DEBUG("Created menu page %d\n", i);

        for(j = 0; j < MENU_PAGE_ITEM_COUNT[i]; ++j)
        {
            item = new nsCore::CMenuItem(page,
                                         CreateItemAction(page,
                                                          (nsCore::EMenuPageIdx)i,
                                                          (nsCore::EMenuItemIdx)j),
                                         MENU_PAGE_ITEMS[i][j],
                                         MENU_PAGE_ITEMS_SEL[i][j]);
            if(item == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu item.");
            }
            LOG_DEBUG("Created menu item %d for mage page %d\n", j, i);
            page->AddItem(item);
        }

        /* Add page */
        this->pages[i] = page;
    }

    this->needUpdate = true;
}

void CMENU::SetPage(const nsCore::EMenuPageIdx pageIdx)
{
    if(pageIdx < this->pages.size())
    {
        this->currPageIdx = pageIdx;
        this->needUpdate  = true;
    }
}

void CMENU::AddPage(nsCore::CMenuPage * page, const nsCore::EMenuPageIdx pageIdx)
{
    if(pageIdx < this->pages.size())
    {
        this->pages[pageIdx] = page;
    }
}

void CMENU::Display(nsHWL::COLEDScreenMgr * oledScreen)
{
    if(this->pages[this->currPageIdx] != nullptr && this->needUpdate)
    {
        this->pages[this->currPageIdx]->Display(oledScreen);
        this->needUpdate = false;
    }
}

void CMENU::SelectNextItem(void)
{
    if(this->pages[this->currPageIdx] != nullptr)
    {
        this->pages[this->currPageIdx]->SelectNextItem();
        this->needUpdate = true;
    }
}

void CMENU::SelectPrevItem(void)
{
    if(this->pages[this->currPageIdx] != nullptr)
    {
        this->pages[this->currPageIdx]->SelectPrevItem();
        this->needUpdate = true;
    }
}

void CMENU::ExecuteSelection(nsCore::CSystemState & sysState)
{
    if(this->pages[this->currPageIdx] != nullptr)
    {
        this->pages[this->currPageIdx]->PerformAction(sysState);
        this->needUpdate = true;
    }
}

nsCore::CMenuItemAction * CMENU::CreateItemAction(nsCore::CMenuPage * page,
                                                  const nsCore::EMenuPageIdx pageIdx,
                                                  const nsCore::EMenuItemIdx itemIdx)
{
    CMenuItemAction * action;

    action = nullptr;

    if(pageIdx == MAIN_PAGE_IDX)
    {
        if(itemIdx == MAINP_WIFI_ITEM_IDX)
        {
            action = new CActionWifiMenu(page, this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocated menu action.");
            }
        }
        else if(itemIdx == MAINP_BT_ITEM_IDX)
        {

        }
        else if(itemIdx == MAINP_UPDATE_ITEM_IDX)
        {

        }
        else if(itemIdx == MAINP_ABOUT_ITEM_IDX)
        {
            action = new CActionDisplayAbout(page, this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocated menu action.");
            }
        }
    }
    else if(pageIdx == WIFI_PAGE_IDX)
    {
        if(itemIdx == WIFIP_BACK_ITEM_IDX)
        {
            action = new CActionExitWifiMenu(page, this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocated menu action.");
            }
        }
    }
    else if(pageIdx == BT_PAGE_IDX)
    {

    }
    else if(pageIdx == UPDATE_PAGE_IDX)
    {

    }
    else if(pageIdx == ABOUT_PAGE_IDX)
    {
        if(itemIdx == ABOUTP_BACK_ITEM_IDX)
        {
            action = new CActionChangePage(page, this, nsCore::MAIN_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocated menu action.");
            }
        }
    }

    return action;
}

void CMENU::ForceUpdate(void)
{
    this->needUpdate = true;
}

#undef CMACTION
#undef CMITEM
#undef CMPAGE
#undef CMENU