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
#include <OLEDScreenMgr.h> /* OLED Screen service */
#include <SystemState.h> /* System state manager */
#include <WaveshareEInkMgr.h> /* eInk display manager */

/* Header File */
#include <Menu.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CMACTION MenuItemAction
/** @brief Class namespace shortcut. */
#define CMITEM MenuItem
/** @brief Class namespace shortcut. */
#define CMPAGE MenuPage
/** @brief Class namespace shortcut. */
#define CMENU Menu

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
char dynBtName[32] = "Name: \0";
char dynBtPin[32] = "PIN: \0";

/* In Flash (constant) */

static const char * MENU_PAGE_ITEM_BLUETOOTH[4] = {
    "Back\n",
    "Status: Enabled",
    dynBtName,
    dynBtPin,
};


static const char * MENU_PAGE_TITLES[EMenuPageIdx::MAX_PAGE_IDX] = {
    "Control Center",
    "Bluetooth",
    "Software Update",
    "Maintainance",
    "About EConBadge"
};

static const uint8_t MENU_PAGE_ITEM_COUNT[EMenuPageIdx::MAX_PAGE_IDX] = {
    4,
    4,
    1,
    2,
    4
};

static const char * MENU_PAGE_ITEM_MAIN[4] = {
    "Bluetooth",
    "Software Update",
    "Maintainance",
    "About EConBadge"
};

static const char * MENU_PAGE_ITEM_MAINTAINANCE[2] = {
    "Back\n",
    "Clear EInk Display"
};

static const char * MENU_PAGE_ITEM_UPDATE[2] = {
    "Back",
};

static const char * MENU_PAGE_ITEM_ABOUT[4] = {
    "Back\n",
    "Telegram | Twitter\nOlson_T  | Arch_Olson",
    "SW " VERSION_SHORT,
    dynProtoRev,
};

static const bool MENU_PAGE_ITEM_MAIN_SEL[4] = {
    true, true, true, true
};

static const bool MENU_PAGE_ITEM_ABOUT_SEL[4] = {
    true, false, false, false
};

static const bool MENU_PAGE_ITEM_MAINTAINANCE_SEL[2] = {
    true, true
};

static const bool MENU_PAGE_ITEM_BLUETOOTH_SEL[4] = {
    true, false, false, false
};

static const bool MENU_PAGE_ITEM_UPDATE_SEL[1] = {
    true
};

static const char ** MENU_PAGE_ITEMS[EMenuPageIdx::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_BLUETOOTH,
    MENU_PAGE_ITEM_UPDATE,
    MENU_PAGE_ITEM_MAINTAINANCE,
    MENU_PAGE_ITEM_ABOUT
};

static const bool * MENU_PAGE_ITEMS_SEL[EMenuPageIdx::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN_SEL,
    MENU_PAGE_ITEM_BLUETOOTH_SEL,
    MENU_PAGE_ITEM_UPDATE_SEL,
    MENU_PAGE_ITEM_MAINTAINANCE_SEL,
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

/******************** IMenuUpdater Definitions ********************/


/******************** MenuItemAction Definitions ********************/
class ActionChangePage : public MenuItemAction
{
    public:
        ActionChangePage(MenuPage * parentPage,
                         Menu * parentMenu,
                         const EMenuPageIdx pageIdx) : MenuItemAction(parentPage, parentMenu)
        {
            pageIdx_ = pageIdx;
        }
        virtual ~ActionChangePage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            parentMenu_->SetPage(pageIdx_);

            return EErrorCode::NO_ERROR;
        }

    private:
        EMenuPageIdx pageIdx_;
};

class ActionDisplayAbout : public MenuItemAction
{
    public:
        ActionDisplayAbout(MenuPage * parentPage,
                           Menu * parentMenu) : MenuItemAction(parentPage, parentMenu)
        {
        }
        virtual ~ActionDisplayAbout(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            char uniqueHWUID[HW_ID_LENGTH];

            /* Update the HW value */
            if(strlen(dynProtoRev) <= strlen(PROTO_REV) + 2)
            {
               HWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);
               strncpy(dynProtoRev + strlen(dynProtoRev), uniqueHWUID, 14);
            }

            parentMenu_->SetPage(EMenuPageIdx::ABOUT_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
};

class ActionDisplayBtPage : public MenuItemAction
{
    public:
        ActionDisplayBtPage(MenuPage * parentPage,
                           Menu * parentMenu) : MenuItemAction(parentPage, parentMenu)
        {
        }
        virtual ~ActionDisplayBtPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            char uniqueHWUID[HW_ID_LENGTH];

            /* Update the HW value */
            if(strlen(dynBtName) <= HW_ID_LENGTH)
            {
               HWManager::GetHWUID(uniqueHWUID, HW_ID_LENGTH);
               strncpy(dynBtName + strlen(dynBtName), uniqueHWUID, 14);
            }

            /* TODO: Manage PIN */
            strcpy(dynBtPin, "PIN: None");

            parentMenu_->SetPage(EMenuPageIdx::BLUETOOTH_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
};

class ActionCleanEInk : public MenuItemAction
{
    public:
        ActionCleanEInk(MenuPage * parentPage,
                        Menu * parentMenu,
                        EInkDisplayManager * einkScreen) : MenuItemAction(parentPage, parentMenu)
        {
            einkScreen_ = einkScreen;
        }
        virtual ~ActionCleanEInk(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            parentMenu_->PrintPopUp("Clearing EInk");
            einkScreen_->RequestClear();
            parentMenu_->ClosePopUp();

            return EErrorCode::NO_ERROR;
        }
    private:
        EInkDisplayManager * einkScreen_;
};

CMACTION::MenuItemAction(MenuPage * parentPage, Menu * parentMenu)
{
    parentPage_ = parentPage;
    parentMenu_ = parentMenu;
}

/******************** MenuItem Definitions ********************/
CMITEM::MenuItem(MenuPage * parentPage, MenuItemAction * action,
                 const char * itemText, const bool isSelectable)
{
    parentPage_   = parentPage;
    action_       = action;
    itemText_     = itemText;
    isSelectable_ = isSelectable;
}

EErrorCode CMITEM::PerformAction(void)
{
    if(action_ != nullptr)
    {
        return action_->Execute();
    }

    return EErrorCode::NOT_INITIALIZED;
}

/******************** MenuPage Definitions ********************/
CMPAGE::MenuPage(OLEDScreenMgr * oledScreen,
                 MenuPage * parentPage,
                 const char * pageTitle)
{
    parentPage_ = parentPage;
    pageTitle_  = pageTitle;
    oledScreen_ = oledScreen;

    selectedItemIdx_ = 0;
}

void CMPAGE::AddItem(MenuItem * item)
{
    items_.push_back(item);
}

EErrorCode CMPAGE::PerformAction(void)
{
    if(items_[selectedItemIdx_] != nullptr)
    {
        return items_[selectedItemIdx_]->PerformAction();
    }

    return EErrorCode::NOT_INITIALIZED;
}

void CMPAGE::Display(const String & popUp) const
{
    Adafruit_SSD1306 * display;
    uint8_t            i;

    display = oledScreen_->GetDisplay();

     /* Init Print */
    display->ssd1306_command(SSD1306_DISPLAYON);
    display->clearDisplay();
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->setCursor(0, 0);

    /* Print menu title */
    display->printf("%s\n---------------------", pageTitle_);

    /* Print items */
    for(i = 0; i < items_.size(); ++i)
    {
        /* If selectable or selected, print selection character */
        if(i == selectedItemIdx_)
        {
           display->printf("> ");
        }
        else if(items_[i]->isSelectable_)
        {
            display->printf("  ");
        }

        display->printf("%s\n", items_[i]->itemText_);
    }

    /* If PopUp is present */
    if(!popUp.isEmpty())
    {
        /* Draw background */
        display->fillRect(2, 30, 124, 20, BLACK);
        display->drawRect(1, 29, 126, 22, WHITE);

        /* Print */
        display->setTextColor(WHITE);
        display->setCursor(4, 32);
        display->printf(popUp.c_str());
    }

    display->display();
}

void CMPAGE::SelectNextItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    itemSize = items_.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(items_[(selectedItemIdx_ + i) % itemSize]->isSelectable_)
        {
            selectedItemIdx_ = (selectedItemIdx_ + i) % itemSize;
            break;
        }
    }
}

void CMPAGE::SelectPrevItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    itemSize = items_.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(items_[(selectedItemIdx_ + itemSize - i) % itemSize]->isSelectable_)
        {
            selectedItemIdx_ = (selectedItemIdx_ + itemSize - i) % itemSize;
            break;
        }
    }
}

/******************** CMenu Definitions ********************/
CMENU::Menu(OLEDScreenMgr * oledScreen, SystemState * systemState,
            EInkDisplayManager * eInkScreen)
{
    MenuPage * page;
    MenuItem * item;
    uint8_t    i;
    uint8_t    j;

    oledScreen_  = oledScreen;
    systemState_ = systemState;
    eInkScreen_  = eInkScreen;

    /* Setup pages */
    pages_.resize(EMenuPageIdx::MAX_PAGE_IDX);
    currPageIdx_ = EMenuPageIdx::MAIN_PAGE_IDX;

    LOG_DEBUG("Creating Menu With %d pages\n", EMenuPageIdx::MAX_PAGE_IDX);

    /* Create Pages */
    for(i = 0; i < EMenuPageIdx::MAX_PAGE_IDX; ++i)
    {
        page = new MenuPage(oledScreen_, nullptr, MENU_PAGE_TITLES[i]);
        if(page == nullptr)
        {
            LOG_CRITICAL("Could not allocate menu page.");
        }

        LOG_DEBUG("Created menu page %d\n", i);

        for(j = 0; j < MENU_PAGE_ITEM_COUNT[i]; ++j)
        {
            item = new MenuItem(page,
                                CreateItemAction(page,
                                                (EMenuPageIdx)i,
                                                (EMenuItemIdx)j),
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
        pages_[i] = page;
    }

    needUpdate_ = false;
}

void CMENU::SetPage(const EMenuPageIdx pageIdx)
{
    if(pageIdx < pages_.size())
    {
        currPageIdx_ = pageIdx;
        needUpdate_  = true;
    }
}

void CMENU::AddPage(MenuPage * page, const EMenuPageIdx pageIdx)
{
    if(pageIdx < pages_.size())
    {
        pages_[pageIdx] = page;
    }
}

void CMENU::Update(void)
{
    uint8_t            debugState;
    EMenuAction        menuAction;
    ESystemState       sysState;
    Adafruit_SSD1306 * display;

    debugState = systemState_->GetDebugState();
    sysState   = systemState_->GetSystemState();
    if(debugState != 0)
    {
        DisplayDebug(debugState);
        needUpdate_ = true;
    }
    else if(sysState == ESystemState::SYS_MENU)
    {
        if (sysState != prevSystemSate_)
        {
            needUpdate_ = true;
        }

        menuAction = systemState_->ConsumeMenuAction();
        if(menuAction == EMenuAction::SELECT_NEXT)
        {
            SelectNextItem();
        }
        else if(menuAction == EMenuAction::SELECT_PREV)
        {
            SelectPrevItem();
        }
        else if(menuAction == EMenuAction::EXECUTE_SEL)
        {
            ExecuteSelection();
        }

        if(pages_[currPageIdx_] != nullptr && needUpdate_)
        {
            pages_[currPageIdx_]->Display(currPopUp_);
            needUpdate_ = false;
        }
    }
    else if(sysState != ESystemState::SYS_START_SPLASH)
    {
        display = oledScreen_->GetDisplay();
        display->ssd1306_command(SSD1306_DISPLAYOFF);
    }

    prevSystemSate_ = sysState;
}

void CMENU::SelectNextItem(void)
{
    if(pages_[currPageIdx_] != nullptr)
    {
        pages_[currPageIdx_]->SelectNextItem();
        needUpdate_ = true;
    }
}

void CMENU::SelectPrevItem(void)
{
    if(pages_[currPageIdx_] != nullptr)
    {
        pages_[currPageIdx_]->SelectPrevItem();
        needUpdate_ = true;
    }
}

void CMENU::ExecuteSelection(void)
{
    if(pages_[currPageIdx_] != nullptr)
    {
        pages_[currPageIdx_]->PerformAction();
        needUpdate_ = true;
    }
}

MenuItemAction * CMENU::CreateItemAction(MenuPage * page,
                                         const EMenuPageIdx pageIdx,
                                         const EMenuItemIdx itemIdx)
{
    MenuItemAction * action;

    action = nullptr;

    if(pageIdx == EMenuPageIdx::MAIN_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::MAINP_BLUETOOTH_ITEM_IDX)
        {
            action = new ActionDisplayBtPage(page, this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_UPDATE_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::UPDATE_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_MAINTAINANCE_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::MAINTAINANCE_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_ABOUT_ITEM_IDX)
        {
            action = new ActionDisplayAbout(page, this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }
    else if(pageIdx == EMenuPageIdx::BLUETOOTH_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::BTP_BACK_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::MAIN_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }
    else if(pageIdx == EMenuPageIdx::UPDATE_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::UPDATEP_BACK_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::MAIN_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }
    else if(pageIdx == EMenuPageIdx::MAINTAINANCE_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::MAINTAINANCEP_BACK_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::MAIN_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINTAINANCEP_CLEAN_ITEM_IDX)
        {
            action = new ActionCleanEInk(page, this, eInkScreen_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }
    else if(pageIdx == EMenuPageIdx::ABOUT_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::ABOUTP_BACK_ITEM_IDX)
        {
            action = new ActionChangePage(page,
                                          this,
                                          EMenuPageIdx::MAIN_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    return action;
}

void CMENU::ForceUpdate(void)
{
    needUpdate_ = true;
}

void CMENU::PrintPopUp(const String & str)
{
    currPopUp_  = str;
    pages_[currPageIdx_]->Display(currPopUp_);
    needUpdate_ = true;
}

void CMENU::ClosePopUp(void)
{
    currPopUp_.clear();
    pages_[currPageIdx_]->Display(currPopUp_);
    needUpdate_ = true;
}

void CMENU::DisplayDebug(const uint8_t debugState)
{
    Adafruit_SSD1306 * display;
    uint8_t            i;

    display = oledScreen_->GetDisplay();

    display->ssd1306_command(SSD1306_DISPLAYON);

    display->clearDisplay();
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->setCursor(0, 0);

    /* Display System State */
    if(debugState == 1)
    {
        display->printf("DebugV | %s\n", VERSION);
        display->printf("STATE: %d\n", systemState_->GetSystemState());
        display->printf("LEVT: %d\n", systemState_->GetLastEventTime());
        display->printf("WIFI: %d | BT: %d\n", 0, 0);
    }
    else if(debugState == 2)
    {
        /* Display Inputs State */
        display->printf("BU:%d (%u) BD:%d (%u)\nBE:%d (%u)\n",
                        systemState_->GetButtonState(BUTTON_UP),
                        systemState_->GetButtonKeepTime(BUTTON_UP),
                        systemState_->GetButtonState(BUTTON_DOWN),
                        systemState_->GetButtonKeepTime(BUTTON_DOWN),
                        systemState_->GetButtonState(BUTTON_ENTER),
                        systemState_->GetButtonKeepTime(BUTTON_ENTER));
    }
    else if(debugState == 3)
    {
        display->printf("\n\n\n     Exit Debug?");
    }

    display->display();
}

#undef CMACTION
#undef CMITEM
#undef CMPAGE
#undef CMENU