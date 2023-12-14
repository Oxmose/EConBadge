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
#include <cstring>            /* String manipulation*/
#include <Types.h>            /* Defined Types */
#include <Logger.h>           /* Logger Service */
#include <version.h>          /* Versioning */
#include <HWLayer.h>          /* Hardware layer */
#include <Storage.h>          /* Storage service */
#include <LEDBorder.h>        /* LED Border manager */
#include <SystemState.h>      /* System state manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <OLEDScreenMgr.h>    /* OLED Screen service */
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
#define CMPAGESC MenuPageImageScroll
/** @brief Class namespace shortcut. */
#define CMPAGEAB MenuPageAbout
/** @brief Class namespace shortcut. */
#define CMENU Menu

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define LINE_SIZE_CHAR 21
#define MAC_PREFIX "MAC "

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
char spProtoRev[LINE_SIZE_CHAR + 1]       = PROTO_REV " ";
char spMacAddr[LINE_SIZE_CHAR + 1]        = MAC_PREFIX;
char spBtName[LINE_SIZE_CHAR + 1]         = "Name: ";
char spBtPin[LINE_SIZE_CHAR + 1]          = "PIN: ";
char spLedBorderState[LINE_SIZE_CHAR + 1] = "Enable LED Border";
char spOverlayToggle[LINE_SIZE_CHAR + 1]  = "Enable Overlay";
char spInfoName[LINE_SIZE_CHAR * 2];
char spInfoContact[LINE_SIZE_CHAR * 2];

/* The last three bytes store the NULL terminator followed by the reference
 * index
 */
char sppImageName[5][LINE_SIZE_CHAR + 3];

/* In Flash (constant) */

/**************** PAGE ITEMS ****************/

/* Control Center */
static const char * sppkMenuPageItemMain[EMenuItemIdx::MAINP_MAX_ITEM_IDX] = {
    "My Info",
    "Display",
    "LED Settings",
    "System",
    "About EConBadge"
};
static const bool sppkMenuPageItemMainSel[EMenuItemIdx::MAINP_MAX_ITEM_IDX] = {
    true, true, true, true, true
};


/* My Info */
static const char * sppkMenuPageItemMyInfo[EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX] = {
    spInfoName,
    spInfoContact
};
static const bool spkMenuPageItemMyInfoSel[EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX] = {
    false, false
};

/* Display */
static const char * sppkMenuPageItemDisplay[EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX] = {
    "Clear EInk Display",
    spOverlayToggle,
    "Update Image"
};
static const bool spkMenuPageItemDisplaySel[EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX] = {
    true, true, true
};

/* LED Settings */
static const char * sppkMenuPageItemLedSettings[EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX] = {
    spLedBorderState,
    "LED Brightness +",
    "LED Brightness -",
};
static const bool spkMenuPageItemLedSettingsSel[EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX] = {
    true, true, true
};

/* System */
static const char * sppkMenuPageItemSystem[EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX] = {
    "Bluetooth",
    "Update",
    "Factory Reset",
    "System Information"
};
static const bool spkMenuPageItemSystemSel[EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX] = {
    true, true, true, true
};

/* Update Image */
static const char * sppkMenuPageItemUpdateImg[EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX] = {
    sppImageName[0],
    sppImageName[1],
    sppImageName[2],
    sppImageName[3],
    sppImageName[4]
};
static const bool spkMenuPageItemUpdateImgSel[EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX] = {
    true, true, true, true, true
};

/* Bluetooth */
static const char * sppkMenuPageItemBluetooth[EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX] = {
    "Status: Enabled ",
    spBtName,
    spBtPin,
};
static const bool spkMenuPageItemBluetoothSel[EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX] = {
    true, false, false
};

/* Factory Reset */
static const char * sppkMenuPageItemReset[EMenuItemIdx::RESETP_MAX_ITEM_IDX] = {
    "Factory Reset?",
    "",
    "No",
    "Yes",
};
static const bool spkMenuPageItemResetSel[EMenuItemIdx::RESETP_MAX_ITEM_IDX] = {
    false, false, true, true
};

/* System Info */
static const char * sppkMenuPageItemSysInfo[EMenuItemIdx::SYSINFOP_MAX_ITEM_IDX] = {
    VERSION,
    spProtoRev,
    spMacAddr
};
static const bool spkMenuPageItemSysInfoSel[EMenuItemIdx::SYSINFOP_MAX_ITEM_IDX] = {
    false, false, false
};

/**************** PAGE TITLES ****************/
static const char * sppkMenuTitles[EMenuPageIdx::MAX_PAGE_IDX] = {
    "Control Center",
    "My Info",
    "Display",
    "LED Settings",
    "System",
    "About EConBadge",
    "Update EInk Image",
    "Bluetooth",
    "Factory Reset",
    "System Information"
};

/**************** PAGE SCROLL BEHAVIOR  ****************/
static const EScrollBehavior spkMenuScroll[EMenuPageIdx::MAX_PAGE_IDX] = {
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_IMAGELIST,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE,
    EScrollBehavior::SCROLL_NONE
};

/**************** PAGE PARENTS  ****************/
static const EMenuPageIdx spkMenuPageParent[EMenuPageIdx::MAX_PAGE_IDX] = {
    EMenuPageIdx::MAX_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::MAIN_PAGE_IDX,
    EMenuPageIdx::DISPLAY_PAGE_IDX,
    EMenuPageIdx::SYSTEM_PAGE_IDX,
    EMenuPageIdx::SYSTEM_PAGE_IDX,
    EMenuPageIdx::SYSTEM_PAGE_IDX
};


/**************** PAGE ITEM COUNTS ****************/
static const uint8_t spkMenuPageItemCount[EMenuPageIdx::MAX_PAGE_IDX] = {
    (uint8_t)EMenuItemIdx::MAINP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::MY_INFOP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::DISPLAYP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::LED_SETTINGSP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::SYSTEMP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::ABOUTP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::UPDIMGP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::BLUETOOTHP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::RESETP_MAX_ITEM_IDX,
    (uint8_t)EMenuItemIdx::SYSINFOP_MAX_ITEM_IDX
};

/**************** PAGE LINKS ****************/
static const char ** spppkMenuPageItems[EMenuPageIdx::MAX_PAGE_IDX] = {
    sppkMenuPageItemMain,
    sppkMenuPageItemMyInfo,
    sppkMenuPageItemDisplay,
    sppkMenuPageItemLedSettings,
    sppkMenuPageItemSystem,
    NULL,
    sppkMenuPageItemUpdateImg,
    sppkMenuPageItemBluetooth,
    sppkMenuPageItemReset,
    sppkMenuPageItemSysInfo,
};
static const bool * sppkMenuPageItemsSel[EMenuPageIdx::MAX_PAGE_IDX] = {
    sppkMenuPageItemMainSel,
    spkMenuPageItemMyInfoSel,
    spkMenuPageItemDisplaySel,
    spkMenuPageItemLedSettingsSel,
    spkMenuPageItemSystemSel,
    NULL,
    spkMenuPageItemUpdateImgSel,
    spkMenuPageItemBluetoothSel,
    spkMenuPageItemResetSel,
    spkMenuPageItemSysInfoSel
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
        ActionChangePage(Menu               * pParentMenu,
                         const EMenuPageIdx   kPageIdx)
        : MenuItemAction(pParentMenu), kPageIdx_(kPageIdx)
        {
        }

        virtual ~ActionChangePage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            pParentMenu_->SetPage(kPageIdx_);
            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        const EMenuPageIdx kPageIdx_;
};

class ActionDisplayMyInfoPage : public MenuItemAction
{
    public:
        explicit ActionDisplayMyInfoPage(Menu * pParentMenu)
        : MenuItemAction(pParentMenu)
        {
            pStore_ = Storage::GetInstance();
        }

        virtual ~ActionDisplayMyInfoPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            std::string infoVal;

            pStore_->GetOwner(infoVal);

            strcpy(spInfoName, "Owner: ");
            strncpy(spInfoName + 7, infoVal.c_str(), sizeof(spInfoName) - 8);
            spInfoName[sizeof(spInfoName) - 1] = 0;

            pStore_->GetContact(infoVal);

            strncpy(spInfoContact, infoVal.c_str(), sizeof(spInfoContact) - 1);
            spInfoContact[sizeof(spInfoContact) - 1] = 0;

            pParentMenu_->SetPage(EMenuPageIdx::MY_INFO_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        Storage * pStore_;
};

class ActionDisplaySysinfo : public MenuItemAction
{
    public:
        explicit ActionDisplaySysinfo(Menu * pParentMenu)
        : MenuItemAction(pParentMenu)
        {
        }

        virtual ~ActionDisplaySysinfo(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            /* Update the HW value */
            if(strlen(spProtoRev) <= strlen(PROTO_REV) + 2)
            {
                strncpy(spProtoRev + strlen(spProtoRev),
                        HWManager::GetHWUID(),
                        14);
                spProtoRev[LINE_SIZE_CHAR] = 0;
            }

            /* Update the HW value */
            if(strlen(spMacAddr) <= strlen(MAC_PREFIX) + 2)
            {
               strncpy(spMacAddr + strlen(spMacAddr),
                       HWManager::GetMacAddress(),
                       17);
                spMacAddr[LINE_SIZE_CHAR] = 0;
            }

            pParentMenu_->SetPage(EMenuPageIdx::SYSINFO_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
};

class ActionDisplayBtPage : public MenuItemAction
{
    public:
        explicit ActionDisplayBtPage(Menu * pParentMenu)
        : MenuItemAction(pParentMenu)
        {
            pStorage_ = Storage::GetInstance();
        }

        virtual ~ActionDisplayBtPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            std::string value;

            /* Update the values */
            pStorage_->GetBluetoothName(value);

            memset(spBtName, 0, LINE_SIZE_CHAR + 1);
            strncpy(spBtName, value.c_str(), LINE_SIZE_CHAR);

            pStorage_->GetBluetoothPin(value);

            memset(spBtPin, 0, LINE_SIZE_CHAR + 1);
            if(value.size() == 0)
            {
                strcpy(spBtPin, "No PIN set");
            }
            else
            {
                strncpy(spBtPin, value.c_str(), LINE_SIZE_CHAR);
            }

            pParentMenu_->SetPage(EMenuPageIdx::BLUETOOTH_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        Storage * pStorage_;
};

class ActionToggleBluetooth : public MenuItemAction
{
    public:
        ActionToggleBluetooth(Menu             * pParentMenu,
                              BluetoothManager * pBtMgr)
        : MenuItemAction(pParentMenu)
        {
            pBtMgr_ = pBtMgr;
        }

        virtual ~ActionToggleBluetooth(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            /* TODO: Toggle bluetooth */

            pParentMenu_->SetPage(EMenuPageIdx::BLUETOOTH_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        BluetoothManager * pBtMgr_;
};

class ActionDisplayLedSettingsPage : public MenuItemAction
{
    public:
        ActionDisplayLedSettingsPage(Menu * pParentMenu, LEDBorder * pLedBorder)
        : MenuItemAction(pParentMenu)
        {
            pLedBorder_ = pLedBorder;
        }

        virtual ~ActionDisplayLedSettingsPage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(pLedBorder_->IsEnabled() == true)
            {
                memcpy(spLedBorderState, "Disable LED Border\0", 19);
            }
            else
            {
                memcpy(spLedBorderState, "Enable LED Border\0", 18);
            }

            pParentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        LEDBorder * pLedBorder_;
};

class ActionToggleLedBorder : public MenuItemAction
{
    public:
        ActionToggleLedBorder(Menu * pParentMenu, LEDBorder * pLedBorder)
        : MenuItemAction(pParentMenu)
        {
            pLedBorder_ = pLedBorder;
        }

        virtual ~ActionToggleLedBorder(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            memset(spLedBorderState, 0, LINE_SIZE_CHAR + 1);
            if(pLedBorder_->IsEnabled() == true)
            {
                pLedBorder_->Disable();
                strcpy(spLedBorderState, "Enable LED Border");
            }
            else
            {
                pLedBorder_->Enable();
                strcpy(spLedBorderState, "Disable LED Border");
            }

            pParentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        LEDBorder * pLedBorder_;
};

class ActionUpdateLEDBorderBrightness : public MenuItemAction
{
    public:
        ActionUpdateLEDBorderBrightness(Menu       * pParentMenu,
                                        LEDBorder  * pLedBorder,
                                        const bool   kIncrease)
        : MenuItemAction(pParentMenu), kIncrease_(kIncrease)
        {
            pLedBorder_ = pLedBorder;
        }

        virtual ~ActionUpdateLEDBorderBrightness(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(kIncrease_ == true)
            {
                pLedBorder_->IncreaseBrightness();
            }
            else
            {
                pLedBorder_->ReduceBrightness();
            }

            pParentMenu_->SetPage(EMenuPageIdx::LED_SETTINGS_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        const bool        kIncrease_;

        LEDBorder       * pLedBorder_;
};

class ActionCleanEInk : public MenuItemAction
{
    public:
        ActionCleanEInk(Menu * pParentMenu, EInkDisplayManager * pEinkScreen)
        : MenuItemAction(pParentMenu)
        {
            pEinkScreen_ = pEinkScreen;
        }

        virtual ~ActionCleanEInk(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            pParentMenu_->PrintPopUp("\n\n    Clearing EInk    ");
            pEinkScreen_->RequestClear();
            pParentMenu_->ClosePopUp();

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        EInkDisplayManager * pEinkScreen_;
};

class ActionDisplayImgUpdatePage : public MenuItemAction
{
    public:
        ActionDisplayImgUpdatePage(Menu               * pParentMenu,
                                   EInkDisplayManager * pEInkDisplay,
                                   ImageList          * pImageList) :
                                   MenuItemAction(pParentMenu)
        {
            pEInkDisplay_ = pEInkDisplay;
            pImageList_   = pImageList;

            pStore_ = Storage::GetInstance();
        }

        virtual ~ActionDisplayImgUpdatePage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            size_t  listSize;
            size_t  i;
            uint8_t selectedItem;

            std::string currentImage;

            /* Update list of images */
            pEInkDisplay_->GetDisplayedImageName(currentImage);
            pStore_->GetImageListFrom(*pImageList_, currentImage, 5);

            /* Find the name in the list */
            listSize = pImageList_->size();
            for(i = 0; i < listSize; ++i)
            {
                if(currentImage == pImageList_->at(i).first)
                {
                    break;
                }
            }

            /* If found, start list with current image */
            if(i == listSize)
            {
                LOG_DEBUG("Current image not found\n");
                selectedItem = 0;
            }
            else
            {
                selectedItem = i;
            }

            /* Set page and select first image item */
            pParentMenu_->SetPage(EMenuPageIdx::UPDATE_IMG_PAGE_IDX);
            pParentMenu_->SetCurrentSelectedItem(selectedItem);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        EInkDisplayManager * pEInkDisplay_;
        ImageList          * pImageList_;
        Storage            * pStore_;
};

class ActionUpdateImage : public MenuItemAction
{
    public:
        ActionUpdateImage(Menu               * pParentMenu,
                          EInkDisplayManager * pEInkDisplay,
                          ImageList          * pImageList,
                          const uint8_t        kIdx)
        :MenuItemAction(pParentMenu), kIdx_(kIdx)
        {
            pEInkDisplay_ = pEInkDisplay;
            pImageList_   = pImageList;
        }

        virtual ~ActionUpdateImage(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            int32_t     refLink;
            std::string currImg;
            std::string newImg;


            /* Get the reference link */
            refLink = *(int32_t*)&sppImageName[kIdx_][LINE_SIZE_CHAR - 1];

            /* NULL reflink is back */
            if(refLink >= 0)
            {
                pEInkDisplay_->GetDisplayedImageName(currImg);
                newImg = pImageList_->at(refLink).first;

                if(currImg != newImg)
                {
                    pParentMenu_->PrintPopUp("\n\n    Updating Image    ");
                    LOG_DEBUG("Updating image with ref %d: %s\n",
                              refLink,
                              newImg.c_str());

                    pEInkDisplay_->SetDisplayedImage(newImg.c_str());
                    pParentMenu_->ClosePopUp();
                }
            }

            pParentMenu_->SetPage(EMenuPageIdx::DISPLAY_PAGE_IDX);

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        const uint8_t        kIdx_;

        EInkDisplayManager * pEInkDisplay_;
        ImageList          * pImageList_;
};

class ActionRequestOTA : public MenuItemAction
{
    public:
        ActionRequestOTA(Menu    * pParentMenu,
                         Updater * pUpdater)
        : MenuItemAction(pParentMenu)
        {
            pUpdater_ = pUpdater;
        }

        virtual ~ActionRequestOTA(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            pUpdater_->RequestUpdate();
            pParentMenu_->PrintPopUp("Waiting for update to start\n");

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        Updater * pUpdater_;
};

class ActionFactoryReset : public MenuItemAction
{
    public:
        ActionFactoryReset(Menu * pParentMenu)
        : MenuItemAction(pParentMenu)
        {
            pStore_ = Storage::GetInstance();
        }

        virtual ~ActionFactoryReset(void)
        {
        }

        virtual EErrorCode Execute(void)
        {
            if(LOGGER_FILE_STATE == true)
            {
                LOGGER_TOGGLE_FILE_LOG();
            }
            pParentMenu_->PrintPopUp("\n The badge will\n"
                                     " restart after the\n reset.");
            pStore_->Format();
            delay(5000);
            ESP.restart();

            return EErrorCode::NO_ERROR;
        }

    protected:

    private:
        Storage * pStore_;
};

CMACTION::MenuItemAction(Menu * pParentMenu)
{
    pParentMenu_ = pParentMenu;
}

CMACTION::~MenuItemAction(void)
{
}

/******************** MenuItem Definitions ********************/
CMITEM::MenuItem(MenuItemAction * pAction,
                 const char     * pkItemText,
                 const bool       kIsSelectable)
: kIsSelectable_(kIsSelectable)
{
    pAction_    = pAction;
    pkItemText_ = pkItemText;
}

EErrorCode CMITEM::PerformAction(void)
{
    if(pAction_ != nullptr)
    {
        return pAction_->Execute();
    }

    return EErrorCode::NO_ACTION;
}

/******************** MenuPage Definitions ********************/
CMPAGE::MenuPage(OLEDScreenMgr      * pOledScreen,
                 const char         * pkPageTitle,
                 const EMenuPageIdx   kParentPageIdx)
: kParentPageIdx_(kParentPageIdx)
{
    pkPageTitle_ = pkPageTitle;
    pOledScreen_ = pOledScreen;

    hasPopup_        = false;
    selectedItemIdx_ = 0;
}

void CMPAGE::AddItem(MenuItem * pItem)
{
    items_.push_back(pItem);
}

void CMPAGE::Display(const std::string & rkPopUp)
{
    Adafruit_SSD1306 * pDisplay;
    uint8_t            i;

    pDisplay = pOledScreen_->GetDisplay();

     /* Init Print */
    pDisplay->ssd1306_command(SSD1306_DISPLAYON);
    pDisplay->clearDisplay();
    pDisplay->setTextColor(WHITE);
    pDisplay->setTextSize(1);
    pDisplay->setCursor(0, 0);

    /* If PopUp is present */
    if(rkPopUp.size() != 0)
    {
        hasPopup_ = true;

        /* Draw Title */
        pDisplay->setTextColor(BLACK);
        pDisplay->setCursor(0, 5);
        pDisplay->fillRect(0, 0, 128, 16, WHITE);
        pDisplay->printf("  --  EXECUTING  --  ");

        /* Draw background */
        pDisplay->fillRect(1, 17, 126, 40, BLACK);
        pDisplay->drawRect(0, 16, 128, 42, WHITE);

        /* Print */
        pDisplay->setTextColor(WHITE);
        pDisplay->setCursor(2, 18);
        pDisplay->printf(rkPopUp.c_str());
    }
    else
    {
        /* Print menu title */
        pDisplay->printf("%s\n---------------------", pkPageTitle_);

        /* Print items */
        for(i = 0; i < items_.size(); ++i)
        {
            /* If selectable or selected, print selection character */
            if(i == selectedItemIdx_ && items_[i]->kIsSelectable_ == true)
            {
                pDisplay->printf("> ");
            }
            else if(i == selectedItemIdx_)
            {
                SelectNextItem();
            }
            else if(items_[i]->kIsSelectable_ == true)
            {
                pDisplay->printf("  ");
            }

            pDisplay->printf("%s\n", items_[i]->pkItemText_);
        }
        hasPopup_ = false;
    }

    pDisplay->display();
}

EMenuPageIdx CMPAGE::GetParentPageIdx(void) const
{
    return kParentPageIdx_;
}

void CMPAGE::SelectNextItem(void)
{
    uint8_t itemSize;
    uint8_t i;

    /* Don't perform anything on popup */
    if(hasPopup_ == true)
    {
        return;
    }

    itemSize = items_.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(items_[(selectedItemIdx_ + i) % itemSize]->kIsSelectable_ == true)
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
    if(hasPopup_ == true)
    {
        return;
    }

    itemSize = items_.size();
    for(i = 1; i < itemSize; ++i)
    {
        if(items_[(selectedItemIdx_ + itemSize - i) % itemSize]
            ->kIsSelectable_ == true)
        {
            selectedItemIdx_ = (selectedItemIdx_ + itemSize - i) % itemSize;
            break;
        }
    }
}

void CMPAGE::SetSelectedItem(const uint8_t kIdx)
{
    if(kIdx < items_.size() && items_[kIdx]->kIsSelectable_ == true)
    {
        selectedItemIdx_ = kIdx;
    }
}

EErrorCode CMPAGE::PerformAction(void)
{
    /* Don't perform anything on popup */
    if(hasPopup_ == true)
    {
        return EErrorCode::NO_ACTION;
    }

    return items_[selectedItemIdx_]->PerformAction();
}

CMPAGESC::MenuPageImageScroll(OLEDScreenMgr      * pOledScreen,
                              const char         * pkPageTitle,
                              const EMenuPageIdx   kParentPageIdx,
                              ImageList          * pImageList)
: MenuPage(pOledScreen, pkPageTitle, kParentPageIdx)
{
    pImageList_ = pImageList;
    pStore_     = Storage::GetInstance();
}

void CMPAGESC::SelectNextItem(void)
{
    uint8_t  itemSize;
    uint32_t listSize;

    /* Don't perform anything on popup */
    if(hasPopup_ == true)
    {
        return;
    }

    itemSize = items_.size();
    listSize = pImageList_->size();

    if(listSize == 0)
    {
        return;
    }

    if(selectedItemIdx_ == listSize - 1)
    {
        /* If we reached the end of the image list */
        if(listSize < itemSize)
        {
            pStore_->GetImageList(*pImageList_, 0, 5);
        }
        else
        {
            pStore_->GetImageList(*pImageList_,
                                  pImageList_->at(selectedItemIdx_).second,
                                  5);
        }

        selectedItemIdx_ = 0;

        /* Update the items */
        UpdateItems();
    }
    else
    {
        ++selectedItemIdx_;
    }
}

void CMPAGESC::SelectPrevItem(void)
{
    uint32_t listSize;
    uint32_t itemIdx;


    /* Don't perform anything on popup */
    if(hasPopup_ == true)
    {
        return;
    }

    listSize = pImageList_->size();

    if(listSize == 0)
    {
        return;
    }

    /* If we reached the start of the items */
    if(selectedItemIdx_ == 0)
    {
        itemIdx = pImageList_->at(0).second;

        /* Check if we are at the begining of the list */
        if(itemIdx == 0)
        {
            pStore_->GetImageList(*pImageList_, -1, 5);
            selectedItemIdx_ = pImageList_->size() - 1;
        }
        else if(itemIdx >= 5)
        {
            pStore_->GetImageList(*pImageList_, itemIdx - 4, 5);
            selectedItemIdx_ = pImageList_->size() - 1;
        }
        else
        {
            pStore_->GetImageList(*pImageList_, 0, 5);
            selectedItemIdx_ = itemIdx;
        }

        /* Update the items */
        UpdateItems();
    }
    else
    {
        --selectedItemIdx_;
    }
}

void CMPAGESC::SetSelectedItem(const uint8_t kIdx)
{
    if(kIdx < pImageList_->size())
    {
        selectedItemIdx_ = kIdx;
    }

    /* Update the items */
    UpdateItems();
}

void CMPAGESC::UpdateItems(void)
{
    uint8_t i;

    /* Update the items */
    for(i = 0; i < pImageList_->size(); ++i)
    {
        strncpy(sppImageName[i],
                pImageList_->at(i).first.c_str(),
                LINE_SIZE_CHAR - 2);
        *(int32_t*)&sppImageName[i][LINE_SIZE_CHAR - 1] = (int32_t)(i);
    }

    /* Empty the rest */
    for(; i < 5; ++i)
    {
        memset(sppImageName[i], 0, LINE_SIZE_CHAR - 1);
        *((int32_t*)(&sppImageName[i][LINE_SIZE_CHAR - 1])) = -1;
    }
}

CMPAGEAB::MenuPageAbout(OLEDScreenMgr      * pOledScreen,
                        const char         * pkPageTitle,
                        const EMenuPageIdx   kParentPageIdx)
: MenuPage(pOledScreen, pkPageTitle, kParentPageIdx)
{
}

void CMPAGEAB::AddItem(MenuItem * pItem)
{
    (void)pItem;
}

EErrorCode CMPAGEAB::PerformAction(void)
{
    return NO_ERROR;
}

void CMPAGEAB::Display(const std::string & rkPopUp)
{
    Adafruit_SSD1306 * pDisplay;

    pDisplay = pOledScreen_->GetDisplay();

     /* Init Print */
    pDisplay->ssd1306_command(SSD1306_DISPLAYON);
    pDisplay->clearDisplay();
    pDisplay->setTextColor(WHITE);
    pDisplay->setTextSize(1);
    pDisplay->setCursor(0, 0);

    /* If PopUp is present */
    if(rkPopUp.size() != 0)
    {
        hasPopup_ = true;

        /* Draw Title */
        pDisplay->setTextColor(BLACK);
        pDisplay->setCursor(0, 5);
        pDisplay->fillRect(0, 0, 128, 16, WHITE);
        pDisplay->printf("  --  EXECUTING  --  ");

        /* Draw background */
        pDisplay->fillRect(1, 17, 126, 40, BLACK);
        pDisplay->drawRect(0, 16, 128, 42, WHITE);

        /* Print */
        pDisplay->setTextColor(WHITE);
        pDisplay->setCursor(2, 18);
        pDisplay->printf(rkPopUp.c_str());
    }
    else
    {
        /* Print menu title */
        pDisplay->printf("%s\n---------------------", pkPageTitle_);
        pDisplay->setTextColor(BLACK);
        pDisplay->fillRect(0, 16, 128, 42, WHITE);
        pDisplay->setCursor(55, 24);
        pDisplay->printf(" Created By");
        pDisplay->setCursor(55, 32);
        pDisplay->printf("   Olson");
        pDisplay->setCursor(55, 48);
        pDisplay->printf("  OlsonTek");

        pDisplay->drawBitmap(1, 16, PKLOGO_BITMAP, LOGO_WIDTH, LOGO_HEIGHT, BLACK);
        hasPopup_ = false;
    }

    pDisplay->display();
}

void CMPAGEAB::SelectNextItem(void)
{
}

void CMPAGEAB::SelectPrevItem(void)
{
}

void CMPAGEAB::SetSelectedItem(const uint8_t kIdx)
{
    (void)kIdx;
}

/******************** CMenu Definitions ********************/
CMENU::Menu(OLEDScreenMgr      * pOledScreen,
            SystemState        * pSystemState,
            EInkDisplayManager * pEInkScreen,
            LEDBorder          * pLedBorder,
            Updater            * pUpdater,
            BluetoothManager   * pBtMgr)
{
    MenuPage * pPage;
    MenuItem * pItem;
    uint8_t    i;
    uint8_t    j;

    pOledScreen_    = pOledScreen;
    pSystemState_   = pSystemState;
    pEInkScreen_    = pEInkScreen;
    pLedBorder_     = pLedBorder;
    pUpdater_       = pUpdater;
    pBtMgr_         = pBtMgr;
    prevSystemSate_ = pSystemState->GetSystemState();
    pStore_         = Storage::GetInstance();

    wasUpdating_ = false;

    /* Setup pages */
    pages_.resize(EMenuPageIdx::MAX_PAGE_IDX);
    currPageIdx_ = EMenuPageIdx::MAIN_PAGE_IDX;

    LOG_DEBUG("Creating menu (%d pages)\n", EMenuPageIdx::MAX_PAGE_IDX);

    /* Init dynamics */
    for(i = 0; i < spkMenuPageItemCount[EMenuPageIdx::UPDATE_IMG_PAGE_IDX]; ++i)
    {
        memset(sppImageName[i], 0, LINE_SIZE_CHAR + 3);
    }

    /* Create Pages */
    for(i = 0; i < EMenuPageIdx::MAX_PAGE_IDX; ++i)
    {
        if(i == EMenuPageIdx::ABOUT_PAGE_IDX)
        {
            pPage = new MenuPageAbout(pOledScreen_,
                                      sppkMenuTitles[i],
                                      spkMenuPageParent[i]);
        }
        else if(spkMenuScroll[i] == EScrollBehavior::SCROLL_IMAGELIST)
        {
            pPage = new MenuPageImageScroll(pOledScreen_,
                                            sppkMenuTitles[i],
                                            spkMenuPageParent[i],
                                            &imageList_);
        }
        else
        {
            pPage = new MenuPage(pOledScreen_,
                                 sppkMenuTitles[i],
                                 spkMenuPageParent[i]);
        }

        LOG_DEBUG("Creating menu page %s\n", sppkMenuTitles[i]);

        for(j = 0; j < spkMenuPageItemCount[i]; ++j)
        {
            pItem = new MenuItem(CreateItemAction(pPage,
                                                  (EMenuPageIdx)i,
                                                  (EMenuItemIdx)j),
                                spppkMenuPageItems[i][j],
                                sppkMenuPageItemsSel[i][j]);
            LOG_DEBUG("Created menu item %s\n",
                      spppkMenuPageItems[i][j],
                      sppkMenuTitles[i]);
            pPage->AddItem(pItem);
        }

        /* Add page */
        pages_[i] = pPage;
    }

    needUpdate_ = false;
}

void CMENU::SetPage(const EMenuPageIdx kPageIdx)
{
    if(kPageIdx < pages_.size())
    {
        currPageIdx_ = kPageIdx;
        needUpdate_  = true;
    }
}

void CMENU::SetCurrentSelectedItem(const uint8_t kIdx)
{
    pages_[currPageIdx_]->SetSelectedItem(kIdx);
}

void CMENU::AddPage(MenuPage * pPage, const EMenuPageIdx kPageIdx)
{
    if(kPageIdx < pages_.size())
    {
        pages_[kPageIdx] = pPage;
    }
}

void CMENU::Update(void)
{
    uint8_t            debugState;
    EMenuAction        menuAction;
    ESystemState       sysState;
    Adafruit_SSD1306 * pDisplay;
    std::string        value;

    debugState = pSystemState_->GetDebugState();
    sysState   = pSystemState_->GetSystemState();
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

        menuAction = pSystemState_->ConsumeMenuAction();
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
            if(pLedBorder_->IsEnabled() == true)
            {
                memcpy(spLedBorderState, "Disable LED Border\0", 19);
            }
            else
            {
                memcpy(spLedBorderState, "Enable LED Border\0", 18);
            }
            needUpdate_ = true;
        }
        else if(menuAction == EMenuAction::REFRESH_MYINFO)
        {
            pStore_->GetOwner(value);
            strcpy(spInfoName, "Owner: ");
            strncpy(spInfoName + 7, value.c_str(), sizeof(spInfoName) - 8);
            spInfoName[sizeof(spInfoName) - 1] = 0;

            pStore_->GetContact(value);
            strncpy(spInfoContact, value.c_str(), sizeof(spInfoContact) - 1);
            spInfoContact[sizeof(spInfoContact) - 1] = 0;
            needUpdate_ = true;
        }
        else if(menuAction == EMenuAction::REFRESH_BT_INFO)
        {
            /* Update the values */
            memset(spBtName, 0, LINE_SIZE_CHAR + 1);
            pStore_->GetBluetoothName(value);
            strncpy(spBtName, value.c_str(), LINE_SIZE_CHAR);

            memset(spBtPin, 0, LINE_SIZE_CHAR + 1);
            pStore_->GetBluetoothPin(value);
            if(value.size() == 0)
            {
                strcpy(spBtPin, "No PIN set");
            }
            else
            {
                strncpy(spBtPin, value.c_str(), LINE_SIZE_CHAR);
            }
            needUpdate_ = true;
        }
        else if(menuAction == EMenuAction::VALIDATE_FACTORY_RESET)
        {
            SetPage(EMenuPageIdx::RESET_PAGE_IDX);
        }

        /* Check if an update is happening */
        if(pUpdater_->GetStatus() != EUpdateState::IDLE)
        {
            wasUpdating_ = true;

            /* A popup update is happening, update it */
            switch(pUpdater_->GetStatus())
            {
                case EUpdateState::WAITING_START:
                    currPopUp_ = std::string(" Waiting for update\n to start ") +
                        std::string(pUpdater_->GetTimeoutLeft() / 1000 , 10) +
                        std::string("s.");
                    break;

                case EUpdateState::WAITING_VALID:
                    currPopUp_ = std::string(" Waiting for \n validation ") +
                        std::string(pUpdater_->GetTimeoutLeft() / 1000 , 10) +
                        std::string("s.");
                    break;

                case EUpdateState::APPLYING_UPDATE:
                    currPopUp_ = "Updating...";
                    break;

                default:
                    break;
            }
            needUpdate_ = true;
        }
        else if(wasUpdating_ == true)
        {
            wasUpdating_ = false;
            ClosePopUp();
        }

        if(needUpdate_ == true)
        {
            pages_[currPageIdx_]->Display(currPopUp_);
            needUpdate_ = false;
        }
    }
    else if(sysState != ESystemState::SYS_START_SPLASH)
    {
        pDisplay = pOledScreen_->GetDisplay();
        pDisplay->ssd1306_command(SSD1306_DISPLAYOFF);
    }

    prevSystemSate_ = sysState;
}

void CMENU::SelectNextItem(void)
{
    pages_[currPageIdx_]->SelectNextItem();
    needUpdate_ = true;
}

void CMENU::SelectPrevItem(void)
{
    pages_[currPageIdx_]->SelectPrevItem();
    needUpdate_ = true;
}

void CMENU::ExecuteSelection(void)
{
    pages_[currPageIdx_]->PerformAction();
    needUpdate_ = true;
}

void CMENU::ExecuteBack(void)
{
    if(pages_[currPageIdx_]->GetParentPageIdx() != EMenuPageIdx::MAX_PAGE_IDX)
    {
        SetPage(pages_[currPageIdx_]->GetParentPageIdx());
    }
}


MenuItemAction * CMENU::CreateItemAction(MenuPage           * page,
                                         const EMenuPageIdx   kPageIdx,
                                         const EMenuItemIdx   kItemIdx)
{
    MenuItemAction * action;

    action = nullptr;

    /*********** Control Center Page ***********/
    if(kPageIdx == EMenuPageIdx::MAIN_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::MAINP_MYINFO_ITEM_IDX)
        {
            action = new ActionDisplayMyInfoPage(this);
        }
        else if(kItemIdx == EMenuItemIdx::MAINP_DISPLAY_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::DISPLAY_PAGE_IDX);
        }
        else if(kItemIdx == EMenuItemIdx::MAINP_LED_SETTINGS_ITEM_IDX)
        {
            action = new ActionDisplayLedSettingsPage(this, pLedBorder_);
        }
        else if(kItemIdx == EMenuItemIdx::MAINP_SYSTEM_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::SYSTEM_PAGE_IDX);
        }
        else if(kItemIdx == EMenuItemIdx::MAINP_ABOUT_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::ABOUT_PAGE_IDX);
        }
    }

    /*********** Display Page ***********/
    else if(kPageIdx == EMenuPageIdx::DISPLAY_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::DISPLAYP_CLEAR_ITEM_IDX)
        {
            action = new ActionCleanEInk(this, pEInkScreen_);
        }
        else if(kItemIdx == EMenuItemIdx::DISPLAYP_UPDATE_IMG_ITEM_IDX)
        {
            action = new ActionDisplayImgUpdatePage(this,
                                                    pEInkScreen_,
                                                    &imageList_);
        }
    }

    /*********** LED Settings Page ***********/
    else if(kPageIdx == EMenuPageIdx::LED_SETTINGS_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::LED_SETTINGSP_TOGGLE_ITEM_IDX)
        {
            action = new ActionToggleLedBorder(this, pLedBorder_);
        }
        else if(kItemIdx == EMenuItemIdx::LED_SETTINGSP_INC_BRIGHT_ITEM_IDX)
        {
            action = new ActionUpdateLEDBorderBrightness(this,
                                                         pLedBorder_,
                                                         true);
        }
        else if(kItemIdx == EMenuItemIdx::LED_SETTINGSP_RED_BRIGHT_ITEM_IDX)
        {
            action = new ActionUpdateLEDBorderBrightness(this,
                                                         pLedBorder_,
                                                         false);
        }
    }

    /*********** System Page ***********/
    else if(kPageIdx == EMenuPageIdx::SYSTEM_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::SYSTEMP_BLUETOOTH_ITEM_IDX)
        {
            action = new ActionDisplayBtPage(this);
        }
        else if(kItemIdx == EMenuItemIdx::SYSTEMP_UPDATE_ITEM_IDX)
        {
            action = new ActionRequestOTA(this, pUpdater_);
        }
        else if(kItemIdx == EMenuItemIdx::SYSTEMP_RESET_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::RESET_PAGE_IDX);
        }
        else if(kItemIdx == EMenuItemIdx::SYSTEMP_SYSINFO_ITEM_IDX)
        {
            action = new ActionDisplaySysinfo(this);
        }
    }

    /*********** Update Image Page ***********/
    else if(kPageIdx == EMenuPageIdx::UPDATE_IMG_PAGE_IDX)
    {
        action = new ActionUpdateImage(this,
                                       pEInkScreen_,
                                       &imageList_,
                                       kItemIdx);
    }

    /*********** Bluetooth Page ***********/
    else if(kPageIdx == EMenuPageIdx::BLUETOOTH_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::BLUETOOTHP_TOGGLE_ITEM_IDX)
        {
            action = new ActionToggleBluetooth(this, pBtMgr_);
        }
    }

    /*********** Factory Reset Page ***********/
    else if(kPageIdx == EMenuPageIdx::RESET_PAGE_IDX)
    {
        if(kItemIdx == EMenuItemIdx::RESETP_NO_ITEM_IDX)
        {
            action = new ActionChangePage(this, EMenuPageIdx::SYSTEM_PAGE_IDX);
        }
        else if(kItemIdx == EMenuItemIdx::RESETP_YES_ITEM_IDX)
        {
            action = new ActionFactoryReset(this);
        }
    }

    return action;
}

void CMENU::ForceUpdate(void)
{
    needUpdate_ = true;
}

void CMENU::PrintPopUp(const std::string & rkStr)
{
    currPopUp_ = rkStr;
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
    return currPopUp_.size() != 0;
}

void CMENU::DisplayDebug(const uint8_t kDebugState)
{
    Adafruit_SSD1306 * pDisplay;

    pDisplay = pOledScreen_->GetDisplay();

    pDisplay->ssd1306_command(SSD1306_DISPLAYON);

    pDisplay->clearDisplay();
    pDisplay->setTextColor(WHITE);
    pDisplay->setTextSize(1);
    pDisplay->setCursor(0, 0);

    /* Display System State */
    if(kDebugState == 1)
    {
        pDisplay->printf("DebugV | %s\n", VERSION);
        pDisplay->printf("STATE: %d\n", pSystemState_->GetSystemState());
        pDisplay->printf("LEVT: %llu\n", pSystemState_->GetLastEventTime());
        pDisplay->printf("CPU Freq: %d\n", ESP.getCpuFreqMHz());
        pDisplay->printf("Heap: %d\n", ESP.getMinFreeHeap());
        pDisplay->printf("PSRAM: %d\n", ESP.getFreePsram());
    }
    else if(kDebugState == 2)
    {
        pDisplay->printf("DebugV | %s\n", VERSION);
        /* Display Inputs State */
        pDisplay->printf("BU:%d (%llu) BD:%d (%llu)\nBE:%d (%llu) BB:%d (%llu)\n",
                        pSystemState_->GetButtonState(BUTTON_UP),
                        pSystemState_->GetButtonKeepTime(BUTTON_UP),
                        pSystemState_->GetButtonState(BUTTON_DOWN),
                        pSystemState_->GetButtonKeepTime(BUTTON_DOWN),
                        pSystemState_->GetButtonState(BUTTON_ENTER),
                        pSystemState_->GetButtonKeepTime(BUTTON_ENTER),
                        pSystemState_->GetButtonState(BUTTON_BACK),
                        pSystemState_->GetButtonKeepTime(BUTTON_BACK));
    }
    else if(kDebugState == 3)
    {
        pDisplay->printf("DebugV | %s\n", VERSION);
        pDisplay->printf("SDCard Type %d\n",
                        pStore_->GetSdCardType());
        pDisplay->printf("SDCard Size %llu\n",
                        pStore_->GetSdCardSize());
        pDisplay->printf("File Logger: %s\n",
                        (LOGGER_FILE_STATE ? "ON" : "OFF"));
    }
    else if(kDebugState == 4)
    {
        pDisplay->printf("\n\n\n     Exit Debug?");
    }

    pDisplay->display();
}

#undef CMACTION
#undef CMITEM
#undef CMPAGE
#undef CMENU