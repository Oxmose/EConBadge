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
#include <LEDBorder.h>        /* LED Border manager */
#include <Storage.h>          /* Storage service */
#include <BlueToothMgr.h>     /* Bluetooth manager */

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
#define CMPAGESC MenuPageImageScroll
/** @brief Class namespace shortcut. */
#define CMENU Menu

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define LINE_SIZE_CHAR 21

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
char dynProtoRev[LINE_SIZE_CHAR] = PROTO_REV " ";
char dynBtName[LINE_SIZE_CHAR] = "Name: ";
char dynBtPin[LINE_SIZE_CHAR] = "PIN: ";
char dynLedBorderState[LINE_SIZE_CHAR] = "Enable LED Border";
char dynOverlayToggle[LINE_SIZE_CHAR] = "Enable Overlay";
char dynInfoName[LINE_SIZE_CHAR * 2];
char dynInfoContact[LINE_SIZE_CHAR * 2];

/* The last three bytes store the NULL terminator followed by the reference
 * index
 */
char dynImageName[5][LINE_SIZE_CHAR + 3];

/* In Flash (constant) */

/**************** PAGE ITEMS ****************/

/* Control Center */
static const char * MENU_PAGE_ITEM_MAIN[EMenuItemIdx::MAINP_MAX_ITEM_IDX] = {
    "My Info",
    "Display",
    "LED Settings",
    "System",
    "About EConBadge"
};
static const bool MENU_PAGE_ITEM_MAIN_SEL[EMenuItemIdx::MAINP_MAX_ITEM_IDX] = {
    true, true, true, true, true
};


/* My Info */
static const char * MENU_PAGE_ITEM_MYINFO[EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX] = {
    dynInfoName,
    dynInfoContact
};
static const bool MENU_PAGE_ITEM_MYINFO_SEL[EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX] = {
    false, false
};

/* Display */
static const char * MENU_PAGE_ITEM_DISPLAY[EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX] = {
    "Clear EInk Display",
    dynOverlayToggle,
    "Update Image"
};
static const bool MENU_PAGE_ITEM_DISPLAY_SEL[EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX] = {
    true, true, true
};

/* LED Settings */
static const char * MENU_PAGE_ITEM_LED_SETTINGS[EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX] = {
    dynLedBorderState,
    "LED Brightness +",
    "LED Brightness -",
};
static const bool MENU_PAGE_ITEM_LED_SETTINGS_SEL[EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX] = {
    true, true, true
};

/* System */
static const char * MENU_PAGE_ITEM_SYSTEM[EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX] = {
    "Bluetooth",
    "Update",
    "Factory Reset"
};
static const bool MENU_PAGE_ITEM_SYSTEM_SEL[EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX] = {
    true, true, true
};

/* About */
static const char * MENU_PAGE_ITEM_ABOUT[EMenuItemIdx::ABOUTP_MAX_ITEM_IDX] = {
    "Telegram: @Olson_T",
    "Twitter: @Arch_Olson",
    "",
    "SW " VERSION_SHORT,
    dynProtoRev,
};
static const bool MENU_PAGE_ITEM_ABOUT_SEL[EMenuItemIdx::ABOUTP_MAX_ITEM_IDX] = {
    false, false, false, false, false
};

/* Update Image */
static const char * MENU_PAGE_ITEM_UPDATE_IMG[EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX] = {
    dynImageName[0],
    dynImageName[1],
    dynImageName[2],
    dynImageName[3],
    dynImageName[4]
};
static const bool MENU_PAGE_ITEM_UPDATE_IMG_SEL[EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX] = {
    true, true, true, true, true
};

/* Bluetooth */
static const char * MENU_PAGE_ITEM_BLUETOOTH[EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX] = {
    "Status: Enabled\n",
    dynBtName,
    dynBtPin,
};
static const bool MENU_PAGE_ITEM_BLUETOOTH_SEL[EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX] = {
    true, false, false
};

/* Factory Reset */
static const char * MENU_PAGE_ITEM_RESET[EMenuItemIdx::RESETP_MAX_ITEM_IDX] = {
    "Factory Reset?",
    "",
    "No",
    "Yes",
};
static const bool MENU_PAGE_ITEM_RESET_SEL[EMenuItemIdx::RESETP_MAX_ITEM_IDX] = {
    false, false, true, true
};

/**************** PAGE TITLES ****************/
static const char * MENU_TITLES[EMenuPageIdx::MAX_PAGE_IDX] = {
    "Control Center",
    "My Info",
    "Display",
    "LED Settings",
    "System",
    "About EConBadge",
    "Update EInk Image",
    "Bluetooth",
    "Factory Reset"
};

/**************** PAGE SCROLL BEHAVIOR  ****************/
static const EScrollBehavior MENU_SCROLL[EMenuPageIdx::MAX_PAGE_IDX] = {
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_IMAGELIST,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE
};

/**************** PAGE PARENTS  ****************/
static const EMenuPageIdx MENU_PAGE_PARENT[EMenuPageIdx::MAX_PAGE_IDX] = {
    EMenuPageIdx::MAX_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::DISPLAY_PAGE_IDX,
    EMenuPageIdx::SYSTEM_PAGE_IDX,
    EMenuPageIdx::SYSTEM_PAGE_IDX
};


/**************** PAGE ITEM COUNTS ****************/
static const uint8_t MENU_PAGE_ITEM_COUNT[EMenuPageIdx::MAX_PAGE_IDX] = {
    (uint8_t)EMenuItemIdx::MAINP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::ABOUTP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::RESETP_MAX_ITEM_IDX
};

/**************** PAGE LINKS ****************/
static const char ** MENU_PAGE_ITEMS[EMenuPageIdx::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN,
    MENU_PAGE_ITEM_MYINFO,
    MENU_PAGE_ITEM_DISPLAY,
    MENU_PAGE_ITEM_LED_SETTINGS,
    MENU_PAGE_ITEM_SYSTEM,
    MENU_PAGE_ITEM_ABOUT,
    MENU_PAGE_ITEM_UPDATE_IMG,
    MENU_PAGE_ITEM_BLUETOOTH,
    MENU_PAGE_ITEM_RESET
};
static const bool * MENU_PAGE_ITEMS_SEL[EMenuPageIdx::MAX_PAGE_IDX] = {
    MENU_PAGE_ITEM_MAIN_SEL,
    MENU_PAGE_ITEM_MYINFO_SEL,
    MENU_PAGE_ITEM_DISPLAY_SEL,
    MENU_PAGE_ITEM_LED_SETTINGS_SEL,
    MENU_PAGE_ITEM_SYSTEM_SEL,
    MENU_PAGE_ITEM_ABOUT_SEL,
    MENU_PAGE_ITEM_UPDATE_IMG_SEL,
    MENU_PAGE_ITEM_BLUETOOTH_SEL,
    MENU_PAGE_ITEM_RESET_SEL
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

/******************** MenuItemAction Definitions ********************/
class ActionChangePage : public MenuItemAction
{
    public:
        ActionChangePage(Menu * parentMenu,
                         const EMenuPageIdx pageIdx) : MenuItemAction(parentMenu)
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

class ActionDisplayMyInfoPage : public MenuItemAction
{
    public:
        ActionDisplayMyInfoPage(Menu * parentMenu) : MenuItemAction(parentMenu)
        {
        }
        virtual ~ActionDisplayMyInfoPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            std::string infoVal;
            Storage *   store;

            store = Storage::GetInstance();

            store->GetOwner(infoVal);
            strcpy(dynInfoName, "Owner: ");
            strncpy(dynInfoName + 7, infoVal.c_str(), sizeof(dynInfoName) - 8);
            dynInfoName[sizeof(dynInfoName) - 1] = 0;

            store->GetContact(infoVal);
            strncpy(dynInfoContact, infoVal.c_str(), sizeof(dynInfoContact) - 1);
            dynInfoContact[sizeof(dynInfoContact) - 1] = 0;

            parentMenu_->SetPage(EMenuPageIdx::MY_INFO_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
};

class ActionDisplayAbout : public MenuItemAction
{
    public:
        ActionDisplayAbout(Menu * parentMenu) : MenuItemAction(parentMenu)
        {
        }
        virtual ~ActionDisplayAbout(void)
        {
        }

        virtual EErrorCode Execute(void)
        {

            /* Update the HW value */
            if(strlen(dynProtoRev) <= strlen(PROTO_REV) + 2)
            {
               strncpy(dynProtoRev + strlen(dynProtoRev),
                       HWManager::GetHWUID(),
                       14);
            }

            parentMenu_->SetPage(EMenuPageIdx::ABOUT_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
};

class ActionDisplayBtPage : public MenuItemAction
{
    public:
        ActionDisplayBtPage(Menu * parentMenu) : MenuItemAction(parentMenu)
        {
        }
        virtual ~ActionDisplayBtPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            Storage * storage;
            std::string value;

            storage = Storage::GetInstance();
            /* Update the values */
            storage->GetBluetoothName(value);
            strncpy(dynBtName, value.c_str(), LINE_SIZE_CHAR);

            storage->GetBluetoothPin(value);
            if(value.size() == 0)
            {
                strcpy(dynBtPin, "No PIN set");
            }
            else
            {
                strncpy(dynBtPin, value.c_str(), LINE_SIZE_CHAR);
            }

            parentMenu_->SetPage(EMenuPageIdx::BLUETOOTH_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        BluetoothManager * btMgr_;
};

class ActionToggleBluetooth : public MenuItemAction
{
    public:
        ActionToggleBluetooth(Menu * parentMenu,
                              BluetoothManager * btMgr) : MenuItemAction(parentMenu)
        {
            btMgr_ = btMgr;
        }
        virtual ~ActionToggleBluetooth(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            /* TODO: Toggle bluetooth */

            parentMenu_->SetPage(EMenuPageIdx::BLUETOOTH_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        BluetoothManager * btMgr_;
};

class ActionDisplayLedSettingsPage : public MenuItemAction
{
    public:
        ActionDisplayLedSettingsPage(Menu * parentMenu, LEDBorder * ledBorder) :
                                     MenuItemAction(parentMenu)
        {
            ledBorder_ = ledBorder;
        }
        virtual ~ActionDisplayLedSettingsPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(ledBorder_->IsEnabled())
            {
                strcpy(dynLedBorderState, "Disable LED Border");
            }
            else
            {
                strcpy(dynLedBorderState, "Enable LED Border");
            }

            parentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        LEDBorder * ledBorder_;
};

class ActionToggleLedBorder : public MenuItemAction
{
    public:
        ActionToggleLedBorder(Menu * parentMenu, LEDBorder * ledBorder) :
                              MenuItemAction(parentMenu)
        {
            ledBorder_ = ledBorder;
        }
        virtual ~ActionToggleLedBorder(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(ledBorder_->IsEnabled())
            {
                ledBorder_->Disable();
                strcpy(dynLedBorderState, "Enable LED Border");
            }
            else
            {
                ledBorder_->Enable();
                strcpy(dynLedBorderState, "Disable LED Border");
            }

            parentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        LEDBorder * ledBorder_;
};

class ActionUpdateLEDBorderBrightness : public MenuItemAction
{
    public:
        ActionUpdateLEDBorderBrightness(Menu * parentMenu,
                                        LEDBorder * ledBorder,
                                        bool increase) : MenuItemAction(parentMenu)
        {
            ledBorder_ = ledBorder;
            increase_  = increase;
        }
        virtual ~ActionUpdateLEDBorderBrightness(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(increase_)
            {
                ledBorder_->IncreaseBrightness();
            }
            else
            {
                ledBorder_->ReduceBrightness();
            }

            parentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        LEDBorder * ledBorder_;
        bool        increase_;
};

class ActionDisplayDisplayPage : public MenuItemAction
{
    public:
        ActionDisplayDisplayPage(Menu * parentMenu,
                                 EInkDisplayManager * einkScreen) :
                                 MenuItemAction(parentMenu)
        {
            einkScreen_ = einkScreen;
        }
        virtual ~ActionDisplayDisplayPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            /* TODO: Manage overlay state display */

            parentMenu_->SetPage(EMenuPageIdx::DISPLAY_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        EInkDisplayManager * einkScreen_;
};

class ActionCleanEInk : public MenuItemAction
{
    public:
        ActionCleanEInk(Menu * parentMenu,
                        EInkDisplayManager * einkScreen) : MenuItemAction(parentMenu)
        {
            einkScreen_ = einkScreen;
        }
        virtual ~ActionCleanEInk(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            parentMenu_->PrintPopUp("\n\n    Clearing EInk    ");
            einkScreen_->RequestClear();
            parentMenu_->ClosePopUp();

            return EErrorCode::NO_ERROR;
        }
    private:
        EInkDisplayManager * einkScreen_;
};

class ActionToggleOverlayEInk : public MenuItemAction
{
    public:
        ActionToggleOverlayEInk(Menu * parentMenu,
                                EInkDisplayManager * einkScreen) :
                                MenuItemAction(parentMenu)
        {
            einkScreen_ = einkScreen;
        }
        virtual ~ActionToggleOverlayEInk(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            parentMenu_->PrintPopUp("\n\n    Enabling / Displaing (TODO) Overlay)    ");
            /* TODO: Manage overlay */
            parentMenu_->ClosePopUp();

            parentMenu_->SetPage(EMenuPageIdx::DISPLAY_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        EInkDisplayManager * einkScreen_;
};

class ActionDisplayImgUpdatePage : public MenuItemAction
{
    public:
        ActionDisplayImgUpdatePage(Menu * parentMenu,
                                   EInkDisplayManager * eInkDisplay,
                                   std::vector<std::string> * imageList) :
                                   MenuItemAction(parentMenu)
        {
            eInkDisplay_ = eInkDisplay;
            imageList_   = imageList;
        }
        virtual ~ActionDisplayImgUpdatePage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            uint32_t  i;
            uint32_t  startIndex;
            size_t    listSize;
            uint8_t   selectedItem;
            Storage * store;

            std::string currentImage;


            /* Update list of images */
            store = Storage::GetInstance();
            store->GetImageList(imageList_);
            eInkDisplay_->GetCurrentImageName(currentImage);

            LOG_DEBUG("Current image: %s\n", currentImage.c_str());

            /* Find the name in the list */
            listSize = imageList_->size();
            for(i = 0; i < listSize; ++i)
            {
                if(currentImage == imageList_->at(i))
                {
                    break;
                }
            }

            /* If found, start list with current image */
            if(i == listSize)
            {
                LOG_DEBUG("Current image not found in image list\n");
                startIndex = 0;
            }
            else
            {
                startIndex = i;
            }

            if(startIndex < 4)
            {
                selectedItem = startIndex;
                startIndex   = 0;
            }
            else
            {
                selectedItem = 0;
            }

            /* Set page and select first image item (1) */
            parentMenu_->SetPage(EMenuPageIdx::UPDATE_IMG_PAGE_IDX);
            parentMenu_->SetCurrentSelectedItem(selectedItem, startIndex);

            return EErrorCode::NO_ERROR;
        }

    private:
        EInkDisplayManager *      eInkDisplay_;
        std::vector<std::string> * imageList_;
};

class ActionUpdateImage : public MenuItemAction
{
    public:
        ActionUpdateImage(Menu * parentMenu,
                          EInkDisplayManager * einkScreen,
                          uint8_t idx,
                          std::vector<std::string> * imageList) :
                          MenuItemAction(parentMenu)
        {
            einkScreen_ = einkScreen;
            idx_        = idx;
            imageList_  = imageList;
        }
        virtual ~ActionUpdateImage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            uint16_t    refLink;
            std::string currImg;
            std::string newImg;


            /* Get the reference link */
            refLink = *(uint32_t*)&dynImageName[idx_][LINE_SIZE_CHAR - 1];

            /* NULL reflink is back */
            if(refLink != 0)
            {
                Storage::GetInstance()->GetCurrentImageName(currImg);
                newImg = imageList_->at(refLink);

                if(currImg != newImg)
                {
                    parentMenu_->PrintPopUp("\n\n    Updating Image    ");
                    LOG_DEBUG("Updating image with reference link %d: %s\n",
                              refLink,
                              newImg.c_str());

                    einkScreen_->SetDisplayedImage(newImg.c_str());
                    parentMenu_->ClosePopUp();
                }
            }

            parentMenu_->SetPage(EMenuPageIdx::DISPLAY_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }
    private:
        EInkDisplayManager *       einkScreen_;
        uint8_t                    idx_;
        std::vector<std::string> * imageList_;
};

class ActionRequestOTA : public MenuItemAction
{
    public:
        ActionRequestOTA(Menu * parentMenu,
                         Updater * updater) : MenuItemAction(parentMenu)
        {
            updater_ = updater;
        }
        virtual ~ActionRequestOTA(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            updater_->RequestUpdate();
            parentMenu_->PrintPopUp("Waiting for update to start\n");

            return EErrorCode::NO_ERROR;
        }
    private:
        Updater * updater_;
};

class ActionFactoryReset : public MenuItemAction
{
    public:
        ActionFactoryReset(Menu * parentMenu) : MenuItemAction(parentMenu)
        {
        }
        virtual ~ActionFactoryReset(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(LOGGER_FILE_STATE)
            {
                LOGGER_TOGGLE_FILE_LOG();
            }
            parentMenu_->PrintPopUp("\nThe badge will\n restart after the\n reset.");
            Storage::GetInstance()->Format();
            delay(5000);
            ESP.restart();

            return EErrorCode::NO_ERROR;
        }
    private:
};

CMACTION::MenuItemAction(Menu * parentMenu)
{
    parentMenu_ = parentMenu;
}

/******************** MenuItem Definitions ********************/
CMITEM::MenuItem(MenuItemAction * action,
                 const char * itemText, const bool isSelectable)
{
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
                 const char * pageTitle,
                 const EMenuPageIdx parentPageIdx)
{
    pageTitle_     = pageTitle;
    oledScreen_    = oledScreen;
    parentPageIdx_ = parentPageIdx;

    hasPopup_        = false;
    selectedItemIdx_ = 0;
}

void CMPAGE::AddItem(MenuItem * item)
{
    items_.push_back(item);
}

void CMPAGE::Display(const String & popUp)
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

    /* If PopUp is present */
    if(!popUp.isEmpty())
    {
        hasPopup_ = true;

        /* Draw Title */
        display->setTextColor(BLACK);
        display->setCursor(0, 5);
        display->fillRect(0, 0, 128, 16, WHITE);
        display->printf("  --  EXECUTING  --  ");

        /* Draw background */
        display->fillRect(1, 17, 126, 40, BLACK);
        display->drawRect(0, 16, 128, 42, WHITE);

        /* Print */
        display->setTextColor(WHITE);
        display->setCursor(2, 18);
        display->printf(popUp.c_str());
    }
    else
    {
        /* Print menu title */
        display->printf("%s\n---------------------", pageTitle_);

        /* Print items */
        for(i = 0; i < items_.size(); ++i)
        {
            /* If selectable or selected, print selection character */
            if(i == selectedItemIdx_ && items_[i]->isSelectable_)
            {
                display->printf("> ");
            }
            else if(i == selectedItemIdx_)
            {
                SelectNextItem();
            }
            else if(items_[i]->isSelectable_)
            {
                display->printf("  ");
            }

            display->printf("%s\n", items_[i]->itemText_);
        }
        hasPopup_ = false;
    }

    display->display();
}

EMenuPageIdx CMPAGE::GetParentPageIdx(void) const
{
    return parentPageIdx_;
}

void CMPAGE::SelectNextItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    /* Don't perform anything on popup */
    if(hasPopup_)
    {
        return;
    }

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

    /* Don't perform anything on popup */
    if(hasPopup_)
    {
        return;
    }

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

void CMPAGE::SetSelectedItem(const uint8_t idx, const uint32_t listIdx)
{
    (void)listIdx;
    if(idx < items_.size() && items_[idx]->isSelectable_)
    {
        selectedItemIdx_ = idx;
    }
}

EErrorCode CMPAGE::PerformAction(void)
{
    /* Don't perform anything on popup */
    if(hasPopup_)
    {
        return EErrorCode::NO_ACTION;
    }

    if(items_[selectedItemIdx_] != nullptr)
    {
        return items_[selectedItemIdx_]->PerformAction();
    }

    return EErrorCode::NOT_INITIALIZED;
}

CMPAGESC::MenuPageImageScroll(OLEDScreenMgr * oledScreen,
                              const char * pageTitle,
                              const EMenuPageIdx parentPageIdx,
                              std::vector<std::string> * imageList) :
                              MenuPage(oledScreen, pageTitle, parentPageIdx)
{
    imageList_             = imageList;
    startDisplayListIdx_   = 0;
}

void CMPAGESC::SelectNextItem(void)
{
    uint8_t  itemSize;
    uint32_t listSize;

    /* Don't perform anything on popup */
    if(hasPopup_)
    {
        return;
    }

    itemSize = items_.size();
    listSize = imageList_->size();

    if(selectedItemIdx_ == itemSize - 1)
    {
        /* If we reached the end of the image list */
        if(startDisplayListIdx_ >= listSize - itemSize)
        {
            selectedItemIdx_     = 0;
            startDisplayListIdx_ = 0;
        }
        /* If we reached the end of the item list but there are  still images to
        * display
        */
        else
        {
            ++startDisplayListIdx_;
        }
    }
    else
    {
        /* If the list is smaller than the items */
        if(selectedItemIdx_ == listSize - 1)
        {
            selectedItemIdx_ = 0;
        }
        else
        {
            ++selectedItemIdx_;
        }
    }

    /* Update the items */
    UpdateItems();
}


void CMPAGESC::SelectPrevItem(void)
{
    uint8_t  itemSize;
    uint32_t listSize;

    /* Don't perform anything on popup */
    if(hasPopup_)
    {
        return;
    }

    itemSize = items_.size();
    listSize = imageList_->size();

    /* If we reached the start of the items */
    if(selectedItemIdx_ == 0)
    {
        /* If there is no more image to display */
        if((startDisplayListIdx_ == 0))
        {
            /* If the list is bigger dans the item list */
            if(itemSize < listSize)
            {
                selectedItemIdx_     = itemSize - 1;
                startDisplayListIdx_ = listSize - itemSize;
            }
            else
            {
                selectedItemIdx_      = listSize - 1;
                startDisplayListIdx_  = 0;
            }
        }
        else
        {
            --startDisplayListIdx_;
        }
    }
    else
    {
        --selectedItemIdx_;
    }

    /* Update the items */
    UpdateItems();
}

void CMPAGESC::SetSelectedItem(const uint8_t idx, const uint32_t listIdx)
{
    if(idx < items_.size() && listIdx < imageList_->size())
    {
        selectedItemIdx_     = idx;
        startDisplayListIdx_ = listIdx;
    }

    /* Update the items */
    UpdateItems();
}

void CMPAGESC::UpdateItems(void)
{
    uint8_t i;

    /* Update the items */
    for(i = 0; i < 5 && i + startDisplayListIdx_ < imageList_->size(); ++i)
    {
        strncpy(dynImageName[i], imageList_->at(i + startDisplayListIdx_).c_str(), LINE_SIZE_CHAR - 2);
        *(uint16_t*)&dynImageName[i][LINE_SIZE_CHAR - 1] = (uint32_t)(i + startDisplayListIdx_);
    }
    /* Empty the rest */
    for(; i < 5; ++i)
    {
        memset(dynImageName[i], 0, LINE_SIZE_CHAR - 1);
    }
}

/******************** CMenu Definitions ********************/
CMENU::Menu(OLEDScreenMgr * oledScreen, SystemState * systemState,
            EInkDisplayManager * eInkScreen, LEDBorder * ledBorder,
            Updater * updater, BluetoothManager * btMgr)
{
    MenuPage * page;
    MenuItem * item;
    uint8_t    i;
    uint8_t    j;

    oledScreen_     = oledScreen;
    systemState_    = systemState;
    eInkScreen_     = eInkScreen;
    ledBorder_      = ledBorder;
    updater_        = updater;
    btMgr_          = btMgr;
    prevSystemSate_ = systemState->GetSystemState();

    wasUpdating_ = false;

    /* Setup pages */
    pages_.resize(EMenuPageIdx::MAX_PAGE_IDX);
    currPageIdx_ = EMenuPageIdx::MAIN_PAGE_IDX;

    LOG_DEBUG("Creating Menu With %d pages\n", EMenuPageIdx::MAX_PAGE_IDX);

    /* Init dynamics */
    for(i = 0; i < MENU_PAGE_ITEM_COUNT[EMenuPageIdx::UPDATE_IMG_PAGE_IDX]; ++i)
    {
        memset(dynImageName[i], 0, LINE_SIZE_CHAR + 3);
    }

    /* Create Pages */
    for(i = 0; i < EMenuPageIdx::MAX_PAGE_IDX; ++i)
    {
        if(MENU_SCROLL[i] == EScrollBehavior::SCROLL_IMAGELIST)
        {
            page = new MenuPageImageScroll(oledScreen_,
                                           MENU_TITLES[i],
                                           MENU_PAGE_PARENT[i],
                                           &imageList_);
        }
        else
        {
            page = new MenuPage(oledScreen_,
                                MENU_TITLES[i],
                                MENU_PAGE_PARENT[i]);
        }

        if(page == nullptr)
        {
            LOG_CRITICAL("Could not allocate menu page.");
        }

        LOG_DEBUG("Created menu page %d\n", i);

        for(j = 0; j < MENU_PAGE_ITEM_COUNT[i]; ++j)
        {
            item = new MenuItem(CreateItemAction(page,
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

void CMENU::SetCurrentSelectedItem(const uint8_t idx, const uint32_t listIdx = 0)
{
    pages_[currPageIdx_]->SetSelectedItem(idx, listIdx);
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
    Storage *          storage;
    std::string        value;

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
            SetPage(EMenuPageIdx::MAIN_PAGE_IDX);
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
        else if(menuAction == EMenuAction::BACK_MENU)
        {
            ExecuteBack();
        }
        else if(menuAction == EMenuAction::REFRESH_LEDB_STATE)
        {
            if(ledBorder_->IsEnabled())
            {
                strcpy(dynLedBorderState, "Disable LED Border");
            }
            else
            {
                ledBorder_->Enable();
                strcpy(dynLedBorderState, "Enable LED Border");
            }
            needUpdate_ = true;
        }
        else if(menuAction == EMenuAction::REFRESH_MYINFO)
        {
            storage = Storage::GetInstance();

            storage->GetOwner(value);
            strcpy(dynInfoName, "Owner: ");
            strncpy(dynInfoName + 7, value.c_str(), sizeof(dynInfoName) - 8);
            dynInfoName[sizeof(dynInfoName) - 1] = 0;

            storage->GetContact(value);
            strncpy(dynInfoContact, value.c_str(), sizeof(dynInfoContact) - 1);
            dynInfoContact[sizeof(dynInfoContact) - 1] = 0;
            needUpdate_ = true;
        }
        else if(menuAction == EMenuAction::REFRESH_BT_INFO)
        {
            storage = Storage::GetInstance();
            /* Update the values */
            storage->GetBluetoothName(value);
            strncpy(dynBtName, value.c_str(), LINE_SIZE_CHAR);

            storage->GetBluetoothPin(value);
            if(value.size() == 0)
            {
                strcpy(dynBtPin, "No PIN set");
            }
            else
            {
                strncpy(dynBtPin, value.c_str(), LINE_SIZE_CHAR);
            }
            needUpdate_ = true;
        }

        /* Check if an update is happening */
        if(updater_->GetStatus() != EUpdateState::IDLE)
        {
            wasUpdating_ = true;

            /* A popup update is happening, update it */
            switch(updater_->GetStatus())
            {
                case EUpdateState::WAITING_START:
                    currPopUp_ = String(" Waiting for update\n to start ") +
                                 String(updater_->GetTimeoutLeft() / 1000 , 10) +
                                 String("s.");
                    break;

                case EUpdateState::WAITING_VALID:
                    currPopUp_ = String(" Waiting for \n validation ") +
                                 String(updater_->GetTimeoutLeft() / 1000 , 10) +
                                 String("s.");
                    break;

                case EUpdateState::APPLYING_UPDATE:
                    currPopUp_ = "Updating...";
                    break;

                default:
                    break;
            }
            needUpdate_ = true;
        }
        else if(wasUpdating_)
        {
            wasUpdating_ = false;
            ClosePopUp();
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

void CMENU::ExecuteBack(void)
{
    if(pages_[currPageIdx_] != nullptr &&
       pages_[currPageIdx_]->GetParentPageIdx() != EMenuPageIdx::MAX_PAGE_IDX)
    {
        SetPage(pages_[currPageIdx_]->GetParentPageIdx());
    }
}


MenuItemAction * CMENU::CreateItemAction(MenuPage * page,
                                         const EMenuPageIdx pageIdx,
                                         const EMenuItemIdx itemIdx)
{
    MenuItemAction * action;

    action = nullptr;

    /*********** Control Center Page ***********/
    if(pageIdx == EMenuPageIdx::MAIN_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::MAINP_MYINFO_ITEM_IDX)
        {
            action = new ActionDisplayMyInfoPage(this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_DISPLAY_ITEM_IDX)
        {
            action = new ActionDisplayDisplayPage(this, eInkScreen_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_LED_SETTINGS_ITEM_IDX)
        {
            action = new ActionDisplayLedSettingsPage(this, ledBorder_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_SYSTEM_ITEM_IDX)
        {
            action = new ActionChangePage(this,
                                          EMenuPageIdx::SYSTEM_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::MAINP_ABOUT_ITEM_IDX)
        {
            action = new ActionDisplayAbout(this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    /*********** Display Page ***********/
    else if(pageIdx == EMenuPageIdx::DISPLAY_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::DISPLAYP_CLEAR_ITEM_IDX)
        {
            action = new ActionCleanEInk(this, eInkScreen_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::DISPLAYP_TOGGLE_OL_ITEM_IDX)
        {
            action = new ActionToggleOverlayEInk(this, eInkScreen_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::DISPLAYP_UPDATE_IMG_ITEM_IDX)
        {
            action = new ActionDisplayImgUpdatePage(this,
                                                    eInkScreen_,
                                                    &imageList_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    /*********** LED Settings Page ***********/
    else if(pageIdx == EMenuPageIdx::LED_SETTINGS_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::LED_SETTINGSP_TOGGLE_ITEM_IDX)
        {
            action = new ActionToggleLedBorder(this, ledBorder_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::LED_SETTINGSP_INC_BRIGHT_ITEM_IDX)
        {
            action = new ActionUpdateLEDBorderBrightness(this,
                                                         ledBorder_,
                                                         true);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::LED_SETTINGSP_RED_BRIGHT_ITEM_IDX)
        {
            action = new ActionUpdateLEDBorderBrightness(this,
                                                         ledBorder_,
                                                         false);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    /*********** System Page ***********/
    else if(pageIdx == EMenuPageIdx::SYSTEM_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::SYSTEMP_BLUETOOTH_ITEM_IDX)
        {
            action = new ActionDisplayBtPage(this);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::SYSTEMP_UPDATE_ITEM_IDX)
        {
            action = new ActionRequestOTA(this, updater_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::SYSTEMP_RESET_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::RESET_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    /*********** Update Image Page ***********/
    else if(pageIdx == EMenuPageIdx::UPDATE_IMG_PAGE_IDX)
    {
        action = new ActionUpdateImage(this, eInkScreen_, itemIdx, &imageList_);
        if(action == nullptr)
        {
            LOG_CRITICAL("Could not allocate menu action.");
        }
    }

    /*********** Bluetooth Page ***********/
    else if(pageIdx == EMenuPageIdx::BLUETOOTH_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::BLUETOOTHP_TOGGLE_ITEM_IDX)
        {
            action = new ActionToggleBluetooth(this, btMgr_);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
    }

    /*********** Factory Reset Page ***********/
    else if(pageIdx == EMenuPageIdx::RESET_PAGE_IDX)
    {
        if(itemIdx == EMenuItemIdx::RESETP_NO_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::SYSTEM_PAGE_IDX);
            if(action == nullptr)
            {
                LOG_CRITICAL("Could not allocate menu action.");
            }
        }
        else if(itemIdx == EMenuItemIdx::RESETP_YES_ITEM_IDX)
        {
            action = new ActionFactoryReset(this);
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

bool CMENU::HasPopup(void) const
{
    return !currPopUp_.isEmpty();
}

void CMENU::DisplayDebug(const uint8_t debugState)
{
    Adafruit_SSD1306 * display;
    uint8_t            i;
    std::string        value;

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
        display->printf("CPU Freq: %d\n", ESP.getCpuFreqMHz());
        display->printf("Heap: %d\n", ESP.getMinFreeHeap());
        display->printf("PSRAM: %d\n", ESP.getFreePsram());
    }
    else if(debugState == 2)
    {
        display->printf("DebugV | %s\n", VERSION);
        /* Display Inputs State */
        display->printf("BU:%d (%llu) BD:%d (%llu)\nBE:%d (%llu) BB: %d (%llu)\n",
                        systemState_->GetButtonState(BUTTON_UP),
                        systemState_->GetButtonKeepTime(BUTTON_UP),
                        systemState_->GetButtonState(BUTTON_DOWN),
                        systemState_->GetButtonKeepTime(BUTTON_DOWN),
                        systemState_->GetButtonState(BUTTON_ENTER),
                        systemState_->GetButtonKeepTime(BUTTON_ENTER),
                        systemState_->GetButtonState(BUTTON_BACK),
                        systemState_->GetButtonKeepTime(BUTTON_BACK));
    }
    else if(debugState == 3)
    {
        display->printf("DebugV | %s\n", VERSION);
        display->printf("SDCard Type %d\n",
                        Storage::GetInstance()->GetSdCardType());
        display->printf("SDCard Size %llu\n",
                        Storage::GetInstance()->GetSdCardSize());
        display->printf("File Logger: %s\n",
                        (LOGGER_FILE_STATE ? "ON" : "OFF"));
    }
    else if(debugState == 4)
    {
        display->printf("\n\n\n     Exit Debug?");
    }

    display->display();
}

#undef CMACTION
#undef CMITEM
#undef CMPAGE
#undef CMENU