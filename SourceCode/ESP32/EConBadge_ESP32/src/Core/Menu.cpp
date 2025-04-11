/*******************************************************************************
 * @file Menu.cpp
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <vector>             /* std::vector */
#include <string>             /* std::string */
#include <cstdint>            /* Generic types */
#include <HWMgr.h>            /* Hardware layer */
#include <Storage.h>          /* Storage service */
#include <version.h>          /* System versionning */
#include <DisplayInterface.h> /* Display interface */

/* Header File */
#include <Menu.h>

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
 * CLASS METHODS
 ******************************************************************************/
Menu::Menu(DisplayInterface* pDisplay)
{
    SMenuPage* pMainPage;
    SMenuPage* pTmpPage;
    SMenuPage* pPrevPage;
    SMenuPage* pSavePage;
    SMenuItem* pTmpItem;

    /* Init the display */
    pDisplay_ = pDisplay;

    /***************************************************************************
     * MENU Structure creation
     **************************************************************************/

    /***************************************************************************
     * Main page
     */
    pMainPage = new SMenuPage();
    pMainPage->pTitle = "Control Center";
    pMainPage->selectedItem = 0;
    pMainPage->updater = nullptr;
    pMainPage->scroller = nullptr;
    pMainPage->pPrevPage = nullptr;

    pMainPage_ = pMainPage;

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "My Info", 8);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)0;
    pMainPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Display", 8);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)1;
    pMainPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "LED Settings", 13);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)2;
    pMainPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "System", 7);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)3;
    pMainPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "About", 6);
    pTmpItem->action = MENU_ACTION_DISPLAY_ABOUT_PAGE;
    pTmpItem->actionParams = (void*)4;
    pMainPage->items.push_back(pTmpItem);

    pPrevPage = pMainPage;

    /***************************************************************************
     * My Info page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "My Info";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = UpdateMyInfoPage;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pPrevPage;
    pPrevPage->pNextPages.push_back(pTmpPage);

    /* Items
     * Not settings those items content, they will be updated in the updater.
     */
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * Display page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "Display";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pPrevPage;
    pPrevPage->pNextPages.push_back(pTmpPage);

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Clear EInk Display", 19);
    pTmpItem->action = MENU_ACTION_CLEAR_EINK;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Select EInk Image", 18);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)0;
    pTmpPage->items.push_back(pTmpItem);

    pSavePage = pTmpPage;

    /***************************************************************************
     * Select EInk page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "EInk Image";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = UpdateEInkImageListPage;
    pTmpPage->scroller = ScrollEInkImageListPage;
    pTmpPage->pPrevPage = pSavePage;
    pSavePage->pNextPages.push_back(pTmpPage);

    /***************************************************************************
     * LED Settings page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "LED Settings";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = UpdateLEDSettings;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pPrevPage;
    pPrevPage->pNextPages.push_back(pTmpPage);

    /* Items
     * Not settings all those items content, they will be updated in the
     * updater.
     */
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_ACTION_SET_LED_ENABLE;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Brightness +", 13);;
    pTmpItem->action = MENU_ACTION_LED_INCREASE_BRIGHTNESS;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Brightness -", 13);;
    pTmpItem->action = MENU_ACTION_LED_DECREASE_BRIGHTNESS;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * System page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "System";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pPrevPage;
    pPrevPage->pNextPages.push_back(pTmpPage);

    pSavePage = pTmpPage;

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Update", 7);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)0;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Bluetooth", 10);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)1;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Factory Reset", 14);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)2;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "System Info", 12);
    pTmpItem->action = MENU_ACTION_NEXT_PAGE;
    pTmpItem->actionParams = (void*)3;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * Update page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "Update";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pSavePage;
    pSavePage->pNextPages.push_back(pTmpPage);

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "To update:\n", 12);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "1) Open the App", 16);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "2) Go to Settings", 18);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "3) Firmware Update", 19);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * Bluetooth page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "Bluetooth";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = UpdateBluetoothInfo;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pSavePage;
    pSavePage->pNextPages.push_back(pTmpPage);

    /* Items
     * Not settings those items content, they will be updated in the updater.
     */
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * Factory reset page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "Factory Reset";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pSavePage;
    pSavePage->pNextPages.push_back(pTmpPage);

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "  Confirm reset?\n", 18);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "No", 3);
    pTmpItem->action = MENU_ACTION_PREVIOUS_PAGE;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "Yes", 4);
    pTmpItem->action = MENU_ACTION_FACTORY_RESET;
    pTmpPage->items.push_back(pTmpItem);


    /***************************************************************************
     * System information page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "System Info";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pSavePage;
    pSavePage->pNextPages.push_back(pTmpPage);

    /* Items */
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, VERSION, strlen(VERSION) + 1);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, PROTO_REV, strlen(PROTO_REV) + 1);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);
    pTmpItem = new SMenuItem();
    memcpy(pTmpItem->pContent, "\nMAC ", 5);
    memcpy(pTmpItem->pContent + 5, HWManager::GetMacAddress(), 17);
    pTmpItem->action = MENU_NO_ACTION;
    pTmpPage->items.push_back(pTmpItem);

    /***************************************************************************
     * About page
     */
    pTmpPage = new SMenuPage();
    pTmpPage->pTitle = "About";
    pTmpPage->selectedItem = 0;
    pTmpPage->updater = nullptr;
    pTmpPage->scroller = nullptr;
    pTmpPage->pPrevPage = pPrevPage;
    pPrevPage->pNextPages.push_back(pTmpPage);

    /* Send the main page for display */
    pCurrentPage_ = pMainPage;
}

void Menu::SendButtonAction(const SButtonAction& rkBtnAction,
                            SCommandRequest& rCommandRequest)
{
    SMenuItem*  pItem;
    size_t      i;
    uint8_t     nextItem;

    /* Init the command request with no request */
    rCommandRequest.header.identifier = -1;

    if(pCurrentPage_ == nullptr)
    {
        return;
    }

    switch(rkBtnAction.id)
    {
        case BUTTON_DOWN:
            if(rkBtnAction.state == BTN_STATE_DOWN)
            {
                if(pCurrentPage_->scroller == nullptr)
                {
                    /* Find the next selectable item */
                    nextItem = pCurrentPage_->selectedItem;
                    for(i = 0; i < pCurrentPage_->items.size(); ++i)
                    {
                        nextItem = (nextItem + 1) % pCurrentPage_->items.size();
                        if(pCurrentPage_->items[nextItem]->action !=
                           MENU_NO_ACTION)
                        {
                            break;
                        }
                    }
                    pCurrentPage_->selectedItem = nextItem;
                }
                else
                {
                    pCurrentPage_->selectedItem = pCurrentPage_->scroller(
                        pCurrentPage_,
                        true
                    );
                }
            }
            break;
        case BUTTON_UP:
            if(rkBtnAction.state == BTN_STATE_DOWN)
            {
                if(pCurrentPage_->scroller == nullptr)
                {
                    /* Find the next selectable item */
                    nextItem = pCurrentPage_->selectedItem;
                    for(i = 0; i < pCurrentPage_->items.size(); ++i)
                    {
                        if(nextItem == 0)
                        {
                            nextItem = pCurrentPage_->items.size() - 1;
                        }
                        else
                        {
                            --nextItem;
                        }

                        if(pCurrentPage_->items[nextItem]->action !=
                           MENU_NO_ACTION)
                        {
                            break;
                        }
                    }
                    pCurrentPage_->selectedItem = nextItem;
                }
                else
                {
                    pCurrentPage_->selectedItem = pCurrentPage_->scroller(
                        pCurrentPage_,
                        false
                    );
                }
            }
            break;
        case BUTTON_BACK:
            if(rkBtnAction.state == BTN_STATE_DOWN)
            {
                if(pCurrentPage_->pPrevPage != nullptr)
                {
                    pCurrentPage_ = pCurrentPage_->pPrevPage;
                    pDisplay_->DisplayPage(pCurrentPage_);
                }
            }
            break;
        case BUTTON_ENTER:
            if(rkBtnAction.state == BTN_STATE_DOWN)
            {
                if(pCurrentPage_->items.size() > pCurrentPage_->selectedItem)
                {
                    pItem = pCurrentPage_->items[pCurrentPage_->selectedItem];
                    if(pItem->action == MENU_ACTION_NEXT_PAGE)
                    {
                        pCurrentPage_ =
                            pCurrentPage_->pNextPages[
                                (uintptr_t)pItem->actionParams
                            ];
                        pCurrentPage_->needsUpdate = true;

                        for(i = 0; i < pCurrentPage_->items.size(); ++i)
                        {
                            /* Select next item */
                            if(pCurrentPage_->items[i]->action !=
                               MENU_NO_ACTION)
                            {
                                pCurrentPage_->selectedItem = i;
                                break;
                            }
                        }
                        pDisplay_->DisplayPage(pCurrentPage_);
                    }
                    else if(pItem->action == MENU_ACTION_PREVIOUS_PAGE)
                    {
                        if(pCurrentPage_->pPrevPage != nullptr)
                        {
                            pCurrentPage_ = pCurrentPage_->pPrevPage;
                            pCurrentPage_->needsUpdate = true;

                            pDisplay_->DisplayPage(pCurrentPage_);
                        }
                    }
                    else if(pItem->action == MENU_ACTION_DISPLAY_ABOUT_PAGE)
                    {
                        pCurrentPage_ =
                            pCurrentPage_->pNextPages[
                                (uintptr_t)pItem->actionParams
                            ];
                        pCurrentPage_->needsUpdate = true;
                        pDisplay_->DisplayPage(nullptr);
                    }
                    else
                    {
                        /* Leave the caller handle the action */
                        HandleMenuAction(pItem, rCommandRequest);
                    }
                }
            }
            break;
        default:
            break;
    }
}

void Menu::Display(void)
{
    pDisplay_->DisplayPage(pCurrentPage_);
}

void Menu::Reset(void)
{
    pCurrentPage_ = pMainPage_;
    pDisplay_->DisplayPage(pCurrentPage_);
}

void Menu::UpdateMyInfoPage(SMenuPage* pPage)
{
    std::string contentStr;
    Storage*    pStore;
    size_t      toCopy;

    pStore = Storage::GetInstance();

    /* Update the info name and contact */
    pStore->GetContent(OWNER_FILE_PATH, "", contentStr, true);
    toCopy = MIN(LINE_SIZE_CHAR * 2 - 7, contentStr.size());

    memcpy(pPage->items[0]->pContent, "Owner: ", 7);
    memcpy(pPage->items[0]->pContent + 7, contentStr.c_str(), toCopy);
    pPage->items[0]->pContent[toCopy + 7] = 0;

    pStore->GetContent(CONTACT_FILE_PATH, "", contentStr, true);
    toCopy = MIN(LINE_SIZE_CHAR * 2 - 9, contentStr.size());

    memcpy(pPage->items[1]->pContent, contentStr.c_str(), toCopy);
    pPage->items[1]->pContent[toCopy] = 0;
}

void Menu::UpdateEInkImageListPage(SMenuPage* pPage)
{
    std::string contentStr;
    Storage*    pStore;
    uint8_t     i;
    SMenuItem*  pTmpItem;
    uint8_t     size;

    std::vector<std::string> imageList;

    pStore = Storage::GetInstance();

    /* Get the current image */
    pStore->GetContent(CURRENT_IMG_NAME_FILE_PATH, "", contentStr, true);

    /* Update the image list */
    pStore->GetFilesListFrom(IMAGE_DIR_PATH, imageList, contentStr, 0, 6);

    /* Clear old items */
    for(i = 0; i < pPage->items.size(); ++i)
    {
        delete pPage->items[i];
    }
    pPage->items.clear();

    /* Add the items */
    for(i = 0; i < imageList.size(); ++i)
    {
        size = MIN(imageList[i].size(), LINE_SIZE_CHAR);
        pTmpItem = new SMenuItem();
        pTmpItem->actionParams = (void*)(uintptr_t)size;
        pTmpItem->action = MENU_ACTION_SET_EINK_IMAGE;
        memcpy(pTmpItem->pContent, imageList[i].c_str(), size);
        pTmpItem->pContent[size + 1] = 0;

        pPage->items.push_back(pTmpItem);
    }

    pPage->needsUpdate = false;
}

uint8_t Menu::ScrollEInkImageListPage(SMenuPage* pPage,
                                      const bool kDown)
{
    std::string contentStr;
    Storage*    pStore;
    uint8_t     i;
    SMenuItem*  pTmpItem;
    uint8_t     size;
    uint8_t     nextItem;
    uint8_t     prevItems;

    std::vector<std::string> imageList;

    pStore = Storage::GetInstance();

    if(kDown)
    {
        nextItem = pPage->selectedItem + 1;
        prevItems = 0;
    }
    else
    {
        nextItem = pPage->selectedItem;
        prevItems = 1;
    }

    /* Get the current item image */
    if(pPage->items.size() > nextItem)
    {
        contentStr = pPage->items[nextItem]->pContent;
    }
    else
    {
        contentStr = "";
    }

    /* Update the image list */
    pStore->GetFilesListFrom(
        IMAGE_DIR_PATH,
        imageList,
        contentStr,
        prevItems,
        6
    );

    /* Clear old items */
    for(i = 0; i < pPage->items.size(); ++i)
    {
        delete pPage->items[i];
    }
    pPage->items.clear();

    /* Add the items */
    for(i = 0; i < imageList.size(); ++i)
    {
        size = MIN(imageList[i].size(), LINE_SIZE_CHAR);
        pTmpItem = new SMenuItem();
        pTmpItem->actionParams = (void*)(uintptr_t)size;
        pTmpItem->action = MENU_ACTION_SET_EINK_IMAGE;
        memcpy(pTmpItem->pContent, imageList[i].c_str(), size);
        pTmpItem->pContent[size + 1] = 0;

        pPage->items.push_back(pTmpItem);
    }

    return 0;
}

void Menu::UpdateBluetoothInfo(SMenuPage* pPage)
{
    std::string contentStr;
    Storage*    pStore;

    pStore = Storage::GetInstance();

    /* Update the name and token */
    memcpy(pPage->items[0]->pContent, "Name: ", 6);
    memcpy(pPage->items[0]->pContent + 6, HWManager::GetHWUID(), HW_ID_LENGTH);
    pPage->items[0]->pContent[HW_ID_LENGTH + 6] = 0;

    pStore->GetContent(BLUETOOTH_TOKEN_FILE_PATH, "Unknown", contentStr, true);
    memcpy(pPage->items[1]->pContent, "\nToken:\n", 8);
    memcpy(pPage->items[1]->pContent + 8, contentStr.c_str(), COMM_TOKEN_SIZE);
    pPage->items[1]->pContent[COMM_TOKEN_SIZE + 8] = 0;
}

void Menu::UpdateLEDSettings(SMenuPage* pPage)
{
    std::string contentStr;
    Storage*    pStore;

    pStore = Storage::GetInstance();

    /* Update the current state */
    pStore->GetContent(LEDBORDER_ENABLED_FILE_PATH, "0", contentStr, true);
    if(contentStr == "0")
    {
        memcpy(pPage->items[0]->pContent, "Enable", 7);
        pPage->items[0]->actionParams = (void*)1;
    }
    else
    {
        memcpy(pPage->items[0]->pContent, "Disable", 8);
        pPage->items[0]->actionParams = (void*)0;
    }

    /* Get the current brightness */
    pStore->GetContent(LEDBORDER_BRIGHTNESS_FILE_PATH, "0", contentStr, true);
    memcpy(pPage->items[3]->pContent, "\n=> Brightness ", 16);
    memcpy(
        pPage->items[3]->pContent + 15,
        contentStr.c_str(),
        contentStr.size() + 1
    );
}

void Menu::HandleMenuAction(const SMenuItem* pkAction,
                            SCommandRequest& rCommandRequest)
{
    switch(pkAction->action)
    {
        case MENU_ACTION_CLEAR_EINK:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 0;
            rCommandRequest.header.type = CMD_EINK_CLEAR;
            break;
        case MENU_ACTION_SET_EINK_IMAGE:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 0;
            rCommandRequest.header.type = CMD_EINK_SELECT_IMAGE;
            memcpy(
                rCommandRequest.pCommand,
                pkAction->pContent,
                (size_t)pkAction->actionParams + 1
            );
            break;
        case MENU_ACTION_FACTORY_RESET:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 0;
            rCommandRequest.header.type = CMD_FACTORY_RESET;
            break;
        case MENU_ACTION_SET_LED_ENABLE:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 1;
            rCommandRequest.pCommand[0] =
                (uint8_t)(uintptr_t)pkAction->actionParams;
            rCommandRequest.header.type = CMD_LEDBORDER_SET_ENABLE;
            break;
        case MENU_ACTION_LED_INCREASE_BRIGHTNESS:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 0;
            rCommandRequest.header.type = CMD_LEDBORDER_INC_BRIGHTNESS;
            break;
        case MENU_ACTION_LED_DECREASE_BRIGHTNESS:
            rCommandRequest.header.identifier = 0;
            rCommandRequest.header.size = 0;
            rCommandRequest.header.type = CMD_LEDBORDER_DEC_BRIGHTNESS;
            break;

        default:
            break;
    }
}