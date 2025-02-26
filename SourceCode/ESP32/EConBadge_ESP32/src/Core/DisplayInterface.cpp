/*******************************************************************************
 * @file DisplayInterface.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/02/2025
 *
 * @version 1.0
 *
 * @brief This file defines the types user interface manager.
 *
 * @details This file defines the types user interface manager. The manager
 * displays and compose what is sent to the screen.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>          /* std::string */
#include <vector>          /* std::vector */
#include <HWMgr.h>         /* Hardware manager */
#include <Logger.h>        /* Logging service */
#include <Storage.h>       /* Storage service */
#include <version.h>       /* ECB versionning */
#include <OLEDScreenMgr.h> /* OLED screen manager */

/* Header File */
#include <DisplayInterface.h>

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

DisplayInterface::DisplayInterface(OLEDScreenMgr* pOLEDScreen)
{
    pOLEDScreen_ = pOLEDScreen;
    lastBatteryAnimVal_ = 0;
    isEnabled_ = true;
    displayPopup_ = false;
    pkCurrentPage_ = nullptr;
    memset(&debugInfo_, 0, sizeof(SDebugInfo_t));

    /* Create the page lock */
    menuPageLock_ = xSemaphoreCreateBinary();
    xSemaphoreGive(menuPageLock_);

    /* Create the display thread */
    xTaskCreatePinnedToCore(
        UpdateScreen,
        "UIThread",
        4096,
        this,
        10,
        &uiThread_,
        tskNO_AFFINITY
    );
}

void DisplayInterface::Enable(const bool kEnabled)
{
    /* Nothing to do */
    if(isEnabled_ == kEnabled)
    {
        return;
    }

    isEnabled_ = kEnabled;
    if(kEnabled)
    {
        vTaskResume(uiThread_);
    }

}

void DisplayInterface::DisplayPage(SMenuPage* pkPage)
{
    xSemaphoreTake(menuPageLock_, portMAX_DELAY);
    pkCurrentPage_ = pkPage;
    xSemaphoreGive(menuPageLock_);
}

void DisplayInterface::DisplayPopup(const std::string& rkTitle,
                                    const std::string& rkContent)
{
    popupTitle_ = rkTitle;
    popupContent_ = rkContent;
    displayPopup_ = true;
    vTaskResume(uiThread_);
}

void DisplayInterface::HidePopup(void)
{
    displayPopup_ = false;
}

void DisplayInterface::SetDebugDisplay(const SDebugInfo_t& rkDebugState)
{
    debugInfo_ = rkDebugState;
}

void DisplayInterface::UpdateScreen(void* pParams)
{
    DisplayInterface*  iFace;
    uint64_t           startTime;
    uint64_t           endTime;
    uint64_t           diffTime;

    iFace = (DisplayInterface*)pParams;

    while(true)
    {
        startTime = HWManager::GetTime();

        if(iFace->debugInfo_.debugState == 0)
        {
            if(iFace->displayPopup_)
            {
                iFace->pOLEDScreen_->SwitchOn();
                iFace->InternalDisplayPopup();
            }
            else if(iFace->isEnabled_)
            {

                if(iFace->pkCurrentPage_ != nullptr)
                {
                    iFace->InternalDisplayPage();
                }
                else
                {
                    iFace->DisplaySplash();
                }
            }
            else
            {
                iFace->pOLEDScreen_->SwitchOff();
                vTaskSuspend(NULL);
                iFace->pOLEDScreen_->SwitchOn();
            }
        }
        else
        {
            iFace->DisplayDebug();
        }

        /* Rate limiting */
        endTime = HWManager::GetTime();
        diffTime = endTime - startTime;
        if(diffTime < 25000)
        {
            HWManager::DelayExecUs(25000 - diffTime, false);
        }
    }
}


void DisplayInterface::PrintBattery(void)
{
    Adafruit_SSD1306 * pDisplay;

    pDisplay = pOLEDScreen_->GetDisplay();

    /* Battery logo */
    pDisplay->drawRect(90, 0, 22, 8, WHITE);
    pDisplay->fillRect(112, 2, 3, 4, WHITE);
    //TODO: if(pBatteryMgr_->IsCharging())
    if(true)
    {
        /* Charging logo */
        pDisplay->drawBitmap(118, 0, PKCHARGING_BITMAP, 8, 10, WHITE);

        /* Charging animation */
        pDisplay->fillRect(91, 0, lastBatteryAnimVal_ / 3 + 1, 8, WHITE);
        lastBatteryAnimVal_ = (lastBatteryAnimVal_ + 1) % 60;
    }
    // else
    // {
    //     /* Battery bar */
    //     pDisplay->fillRect(91, 0, pBatteryMgr_->GetPercentage() / 5, 8, WHITE);
    // }
}

void DisplayInterface::InternalDisplayPage(void)
{
    Adafruit_SSD1306* pDisplay;
    uint32_t          i;
    bool              hasSelectable;

    pDisplay = pOLEDScreen_->GetDisplay();

    /* Init Print */
    pDisplay->clearDisplay();
    pDisplay->setTextColor(WHITE);
    pDisplay->setTextSize(1);
    pDisplay->setCursor(0, 0);


    xSemaphoreTake(menuPageLock_, portMAX_DELAY);

    /* Update the page if needed */
    if(pkCurrentPage_->updater != nullptr && pkCurrentPage_->needsUpdate)
    {
        pkCurrentPage_->updater(pkCurrentPage_);
    }

    /* Print menu title */
    pDisplay->printf(
        "%s\n---------------------",
        pkCurrentPage_->pTitle
    );

    /* Check if we have selectable items */
    hasSelectable = false;

    for(i = 0; i < pkCurrentPage_->items.size(); ++i)
    {
        if(pkCurrentPage_->items[i]->action != MENU_NO_ACTION)
        {
            hasSelectable = true;
            break;
        }
    }

    /* Print items */
    for(i = 0; i < pkCurrentPage_->items.size(); ++i)
    {
        /* If selectable or selected, print selection character */
        if(i == pkCurrentPage_->selectedItem &&
           pkCurrentPage_->items[i]->action != MENU_NO_ACTION)
        {
            pDisplay->printf("> ");
        }
        else if(hasSelectable)
        {
            pDisplay->printf("  ");
        }
        pDisplay->printf(
            "%s\n",
            pkCurrentPage_->items[i]->pContent
        );
    }

    xSemaphoreGive(menuPageLock_);

    /* Call the additional display function */
    PrintBattery();

    /* Display screen */
    pDisplay->display();
}

void DisplayInterface::InternalDisplayPopup(void)
{
    Adafruit_SSD1306* pDisplay;
    pDisplay = pOLEDScreen_->GetDisplay();

    /* Init Print */
    pDisplay->clearDisplay();
    pDisplay->setTextSize(1);

    /* Draw Title */
    pDisplay->setTextColor(BLACK);
    pDisplay->setCursor(0, 5);
    pDisplay->fillRect(0, 0, 128, 16, WHITE);
    pDisplay->printf("> %s", popupTitle_.c_str());

    /* Print */
    pDisplay->setTextColor(WHITE);
    pDisplay->setCursor(0, 18);
    pDisplay->printf(popupContent_.c_str());

    /* Display screen */
    pDisplay->display();
}

void DisplayInterface::DisplayDebug(void)
{
    Adafruit_SSD1306* pDisplay;
    Storage*          pStore;

    pDisplay = pOLEDScreen_->GetDisplay();
    pStore = Storage::GetInstance();

    pDisplay->ssd1306_command(SSD1306_DISPLAYON);

    pDisplay->clearDisplay();
    pDisplay->setTextColor(WHITE);
    pDisplay->setTextSize(1);
    pDisplay->setCursor(0, 0);

    /* Display System State */
    pDisplay->printf("DebugV | %s\n", VERSION);
    if(debugInfo_.debugState == 1)
    {
        pDisplay->printf("STATE: %d\n", debugInfo_.systemState);
        pDisplay->printf("LEVT: %llu\n", debugInfo_.lastEventTime);
        pDisplay->printf("CPU Freq: %d\n", ESP.getCpuFreqMHz());
        pDisplay->printf("Heap: %d\n", ESP.getMinFreeHeap());
        if(debugInfo_.batteryCharging)
        {
            pDisplay->printf("Battery: Charging...\n");
        }
        else
        {
            pDisplay->printf("Battery: %d%%\n", debugInfo_.batteryState);
        }
    }
    else if(debugInfo_.debugState == 2)
    {
        /* Display Inputs State */
        pDisplay->printf(
            "BU:%d (%llu)\nBD:%d (%llu)\nBE:%d (%llu)\nBB:%d (%llu)",
            debugInfo_.buttonsState[BUTTON_UP],
            debugInfo_.buttonsKeepTime[BUTTON_UP],
            debugInfo_.buttonsState[BUTTON_DOWN],
            debugInfo_.buttonsKeepTime[BUTTON_DOWN],
            debugInfo_.buttonsState[BUTTON_ENTER],
            debugInfo_.buttonsKeepTime[BUTTON_ENTER],
            debugInfo_.buttonsState[BUTTON_BACK],
            debugInfo_.buttonsKeepTime[BUTTON_BACK]
        );
    }
    else if(debugInfo_.debugState == 3)
    {
        pDisplay->printf("SDCard Type %d\n", pStore->GetSdCardType());
        pDisplay->printf("SDCard Size %llu\n", pStore->GetSdCardSize());
    }
    else if(debugInfo_.debugState == 4)
    {
        pDisplay->printf("\n\n\n     Exit Debug?");
    }

    pDisplay->display();
}

void DisplayInterface::DisplaySplash(void)
{
    pOLEDScreen_->DisplaySplash();
}