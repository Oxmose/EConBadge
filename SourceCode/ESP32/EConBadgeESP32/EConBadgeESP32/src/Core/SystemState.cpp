/*******************************************************************************
 * @file SystemState.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the system state service.
 *
 * @details This file provides the system state service. This files defines
 * the different features embedded in the system state.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>            /* String manipulation*/
#include <Types.h>            /* Defined Types */
#include <Arduino.h>          /* Arduino Services */
#include <Logger.h>           /* Logging service */
#include <version.h>          /* Versioning */
#include <HWLayer.h>          /* Hardware Services */
#include <OLEDScreenDriver.h> /* OLED driver */
#include <CommandControler.h> /* Command controler service */
#include <epd5in65f.h>        /* EInk Driver */
#include <EEPROM.h>           /* Flash EEPROM driver */
#include <IOButtonMgr.h>      /* Wakeup PIN */
#include <LEDBorder.h>        /* LED border driver */

/* Header File */
#include <SystemState.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSYSSTATE nsCore::CSystemState

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

CSYSSTATE::CSystemState(void)
{
    /* Init button states */
    memset(this->buttonsState, 0, sizeof(EButtonState) * BUTTON_MAX_ID);
    memset(this->prevButtonsState, 0, sizeof(EButtonState) * BUTTON_MAX_ID);
    memset(this->buttonsKeepTime, 0, sizeof(uint32_t) * BUTTON_MAX_ID);

    prevState     = SYS_IDLE;
    lastEventTime = 0;
}

void CSYSSTATE::Init(nsHWL::COLEDScreenMgr * oledDriver,
                     Epd * eInkDriver,
                     nsHWL::CLEDBorder * ledBorderDriver)
{
    this->oledDriver = oledDriver;
    this->eInkDriver = eInkDriver;
    this->ledBorderDriver = ledBorderDriver;
}

ESystemState CSYSSTATE::GetSystemState(void) const
{
    return this->currState;
}

void CSYSSTATE::SetSystemState(const ESystemState state)
{
    this->prevState = this->currState;
    this->currState = state;
    lastEventTime = millis();
}

uint32_t CSYSSTATE::GetLastEventTime(void) const
{
    return this->lastEventTime;
}

uint8_t CSYSSTATE::GetDebugState(void) const
{
    return this->currDebugState;
}

EButtonState CSYSSTATE::GetButtonState(const EButtonID btnId) const
{
    if(btnId < BUTTON_MAX_ID)
    {
        return this->buttonsState[btnId];
    }

    return BTN_STATE_DOWN;
}

void CSYSSTATE::SetButtonState(const EButtonID btnId, const EButtonState state)
{
    if(btnId < BUTTON_MAX_ID)
    {
        this->prevButtonsState[btnId] = this->buttonsState[btnId];
        if(this->buttonsState[btnId] != state)
        {
            lastEventTime = millis();
        }
        this->buttonsState[btnId] = state;
    }
}

uint32_t CSYSSTATE::GetButtonKeepTime(const EButtonID btnId) const
{
    if(btnId < BUTTON_MAX_ID)
    {
        return this->buttonsKeepTime[btnId];
    }

    return 0;
}
void CSYSSTATE::SetButtonKeepTime(const EButtonID btnId,
                                  const uint32_t keepTime)
{
    if(btnId < BUTTON_MAX_ID)
    {
        if(this->buttonsKeepTime[btnId] != keepTime)
        {
            this->buttonsKeepTime[btnId] = keepTime;
            lastEventTime = millis();
        }
    }
}

EErrorCode CSYSSTATE::ComputeState(void)
{
    EErrorCode retCode;

    retCode = NO_ERROR;

    /* Check the prioritary events */
    if(this->currDebugState == 0 &&
       this->buttonsState[BUTTON_UP] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_UP] >= DEBUG_BTN_PRESS_TIME &&
       this->buttonsState[BUTTON_DOWN] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_DOWN] >= DEBUG_BTN_PRESS_TIME)
    {
        LOG_DEBUG("Enabling to debug state\n");
        this->currDebugState = 1;
    }

    /* If in debug state */
    if(this->currDebugState == 0)
    {
        /* Check the regular states management */
        switch(this->currState)
        {
            case SYS_IDLE:
                ManageIdleState();
                break;
            case SYS_START_SPLASH:
                if(this->prevState != this->currState)
                {
                    this->oledDriver->DisplaySplash();
                    this->prevState = this->currState;
                }
                /* After SPLASH_TIME, switch to IDLE */
                if(millis() > SPLASH_TIME)
                {
                    this->currState = SYS_IDLE;
                    this->prevState = SYS_START_SPLASH;
                }
                break;
            case SYS_MENU_WIFI_WAIT:
            case SYS_MENU_WIFI_EXIT:
            case SYS_MENU_WIFI_WAITCOMM:
                ManageWifiState();
            case SYS_MENU:
                ManageMenuState();
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        ManageDebugState();
        this->oledDriver->DisplayDebug(*this);
    }

    return retCode;
}

void CSYSSTATE::ManageDebugState(void)
{
    /* Check if we should switch to next debug state */
    if(this->prevButtonsState[BUTTON_DOWN] != BTN_STATE_DOWN &&
        this->buttonsState[BUTTON_DOWN] == BTN_STATE_DOWN)
    {
        if(this->currDebugState == 3)
        {
            this->currDebugState = 0;
        }
        ++this->currDebugState;
    }
    else if(this->prevButtonsState[BUTTON_UP] != BTN_STATE_DOWN &&
            this->buttonsState[BUTTON_UP] == BTN_STATE_DOWN)
    {
        if(this->currDebugState == 1)
        {
            this->currDebugState = 4;
        }
        --this->currDebugState;
    }
    else if(this->prevButtonsState[BUTTON_ENTER] != BTN_STATE_DOWN &&
            this->buttonsState[BUTTON_ENTER] == BTN_STATE_DOWN &&
            this->currDebugState == 3)
    {
        this->currDebugState = 0;
        this->menu.ForceUpdate();
        LOG_DEBUG("Disabling to debug state\n");
    }
}

void CSYSSTATE::ManageIdleState(void)
{
    /* Check if we should enter menu mode */
    if(this->buttonsState[BUTTON_ENTER] == BTN_STATE_KEEP &&
       this->buttonsKeepTime[BUTTON_ENTER] >= MENU_BTN_PRESS_TIME)
    {
        SetSystemState(SYS_MENU);

        LOG_DEBUG("Increasing CPU frequency\n");
        if(!setCpuFrequencyMhz(240))
        {
            LOG_ERROR("Could not set CPU frequency\n");
        }
        else
        {
            Serial.updateBaudRate(115200);
            LOG_INFO("Set CPU Freq: %d\n", getCpuFrequencyMhz());
        }
    }
    else if(this->prevState != SYS_IDLE &&
            this->buttonsState[BUTTON_ENTER] == BTN_STATE_UP)
    {
        LOG_DEBUG("Switching to IDLE state\n");
        SetSystemState(SYS_IDLE);

        /* Shut down screen */
        this->oledDriver->GetDisplay()->ssd1306_command(SSD1306_DISPLAYOFF);

        /* Ensure we flushed everything */
        Serial.flush();

        /* Got to sleep if the LED border is off  */
        if(!this->ledBorderDriver->IsEnabled())
        {
            LOG_DEBUG("Going to Light Sleep\n");
            esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_ENTER_PIN, LOW);
            esp_light_sleep_start();
        }
        else
        {
            /* Simply reduce the CPU frequency */
            LOG_DEBUG("Reducing CPU frequency\n");
            if(!setCpuFrequencyMhz(80))
            {
                LOG_ERROR("Could not set CPU frequency\n");
            }
            else
            {
                Serial.updateBaudRate(115200);
                LOG_INFO("Set CPU Freq: %d\n", getCpuFrequencyMhz());
            }
        }
    }
}

void CSYSSTATE::ManageMenuState(void)
{
    if(this->prevState == SYS_MENU || this->prevState == SYS_MENU_WIFI_WAIT ||
       this->prevState == SYS_MENU_WIFI_WAITCOMM)
    {
        /* Update selected item */
        if(this->prevButtonsState[BUTTON_DOWN] != BTN_STATE_DOWN &&
           this->buttonsState[BUTTON_DOWN] == BTN_STATE_DOWN)
        {
            this->menu.SelectNextItem();
            lastEventTime = millis();
        }
        else if(this->prevButtonsState[BUTTON_UP] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_UP] == BTN_STATE_DOWN)
        {
            this->menu.SelectPrevItem();
            lastEventTime = millis();
        }
        /* Check if enter was pressed */
        else if(this->prevButtonsState[BUTTON_ENTER] != BTN_STATE_DOWN &&
                this->buttonsState[BUTTON_ENTER] == BTN_STATE_DOWN)
        {
            this->menu.ExecuteSelection(*this);
            lastEventTime = millis();
        }

        /* Manage IDLE detection in menu */
        if(this->currState == SYS_MENU)
        {
            if(millis() - this->lastEventTime > SYSTEM_IDLE_TIME)
            {
                SetSystemState(SYS_IDLE);
            }
        }
    }
    else
    {
        /* If this is the first time we enter the menu */
        LOG_DEBUG("Switching to menu mode\n");

        /* Init menu page and menu item */
        SetSystemState(SYS_MENU);
        this->menu.ForceUpdate();
    }

    /* Update display */
    this->menu.Display(this->oledDriver);
}

void CSYSSTATE::SetStateMenuPageUpdater(nsCore::IMenuUpdater * menuUpdater)
{
    this->menuUpdater = menuUpdater;
}

void CSYSSTATE::ManageWifiState(void)
{
    EErrorCode         retCode;
    char               hwuid[HW_ID_LENGTH];
    char               wifiPass[EEPROM_SIZE_WIFI_PASS + 1];
    uint32_t           readCount;
    char               popup[26];
    uint32_t           command;
    Adafruit_SSD1306 * display;

    /* Check if we just entered the wifi mode */
    if(this->currState == SYS_MENU_WIFI_WAIT &&
       this->prevState != SYS_MENU_WIFI_WAIT &&
       this->prevState != SYS_MENU_WIFI_WAITCOMM)
    {
        nsHWL::CHWManager::GetHWUID(hwuid, HW_ID_LENGTH);
        /* Get Wifi Password */
        readCount = EEPROM.readBytes(EEPROM_ADDR_WIFI_PASS, wifiPass, EEPROM_SIZE_WIFI_PASS);
        if(readCount <= 0)
        {
            strncpy(wifiPass, "econpasswd", 11);
        }
        wifiPass[EEPROM_SIZE_WIFI_PASS + 1] = 0;
        /* Init the WIFI AP */
        retCode = wifiAP.InitAP(hwuid, wifiPass);
        if(retCode == NO_ERROR)
        {
            LOG_INFO("EConBadge IP: %s\n", wifiAP.GetIPAddr().c_str());
            retCode = wifiAP.StartServer(SERVER_COMM_PORT);
            if(retCode == NO_ERROR)
            {
                LOG_INFO("Wifi Server started on port %d\n", SERVER_COMM_PORT);
            }
            else
            {
                LOG_ERROR("Could not start Wifi server: Error %d\n", retCode);
            }
        }
        else
        {
            LOG_ERROR("Could not start Wifi AP: Error %d\n", retCode);
        }

        SetSystemState(SYS_MENU_WIFI_WAIT);
        (*this->menuUpdater)(*this);
    }
    /* Check if we are waiting for a client */
    else if(this->currState == SYS_MENU_WIFI_WAIT)
    {
        /* Update display */
        if(this->prevState == SYS_MENU_WIFI_WAITCOMM)
        {
            this->menu.SetPage(WIFI_PAGE_IDX);
            this->menu.Display(this->oledDriver);
            SetSystemState(SYS_MENU_WIFI_WAIT);

            (*this->menuUpdater)(*this);
        }

        retCode = wifiAP.WaitClient();
        if(retCode == NO_ERROR)
        {
            /* A client got connected */
            SetSystemState(SYS_MENU_WIFI_WAITCOMM);
        }
        else if(retCode != NO_ACTION)
        {
            LOG_ERROR("Error while waiting for client: %d\n", retCode);
        }

        /* Check idle state */
        if(wifiAP.isIdle())
        {
            SetSystemState(SYS_MENU_WIFI_EXIT);
        }
    }
    /* Check if we are waiting for a interraction */
    else if(this->currState == SYS_MENU_WIFI_WAITCOMM)
    {
        /* If this si the first time we check the connected state, display
         * it page
         */
        if(this->prevState != this->currState)
        {
            this->menu.SetPage(WIFI_CLIENT_PAGE_IDX);
            this->menu.Display(this->oledDriver);
            SetSystemState(SYS_MENU_WIFI_WAITCOMM);
        }

        /* If nothing happened during a defined span of time, disable
         * the wifi mode.
         */
        if(!wifiAP.isIdle())
        {
            /* Wait for the next command */
            retCode = wifiAP.WaitCommand(&command);
            if(retCode == NO_ERROR)
            {
                LOG_DEBUG("Received WIFI command: %d\n", command);

                /* Directly write as poppup current command */
                sprintf(popup, "Executing Command\n -> %d\0", command);
                this->menu.PrintPopUp(String(popup));
                this->menu.Display(this->oledDriver);

                retCode = commControler.ExecuteCommand(command, &wifiAP, this);
                if(retCode != NO_ERROR)
                {
                    LOG_ERROR("Error while executing command: %d\n", retCode);
                }

                /* Clear popup */
                this->menu.ClosePopUp();
                this->menu.Display(this->oledDriver);
            }
            else if(retCode == NO_CONNECTION)
            {
                /* We got disconnected, got back to waiting for a client */
                LOG_DEBUG("Lost client connection, waiting for next one\n");
                SetSystemState(SYS_MENU_WIFI_WAIT);
            }
            else if(retCode != NO_ACTION)
            {
                LOG_ERROR("Error while getting WIFI command: %d\n", retCode);
            }
        }
        else
        {
            SetSystemState(SYS_MENU_WIFI_EXIT);
        }
    }
    /* Check if we want to exit */
    else if(this->currState == SYS_MENU_WIFI_EXIT)
    {
        /* Disable AP */
        retCode = wifiAP.StopServer();
        if(retCode == NO_ERROR)
        {
            LOG_INFO("Wifi Server stopped\n");

            retCode = wifiAP.StopAP();
            if(retCode == NO_ERROR)
            {
                LOG_INFO("Wifi AP stopped\n");
            }
            else
            {
                LOG_ERROR("Could not stop Wifi AP: Error %d\n", retCode);
            }
        }
        else
        {
            LOG_ERROR("Could not stop Wifi server: Error %d\n", retCode);
        }

        /* Switch mode */
        SetSystemState(SYS_MENU);
        (*this->menuUpdater)(*this);
    }
}

nsComm::CWifiAP * CSYSSTATE::GetWifiMgr(void)
{
    return &this->wifiAP;
}

nsCore::CMenu * CSYSSTATE::GetMenu(void)
{
    return &this->menu;
}

Epd * CSYSSTATE::GetEInkDriver(void)
{
    return this->eInkDriver;
}

nsHWL::COLEDScreenMgr * CSYSSTATE::GetOLEDDriver(void)
{
    return this->oledDriver;
}

nsHWL::CLEDBorder * CSYSSTATE::GetLEDBorderDriver(void)
{
    return this->ledBorderDriver;
}

#undef CSYSSTATE