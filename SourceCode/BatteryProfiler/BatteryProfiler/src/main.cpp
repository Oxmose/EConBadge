/*******************************************************************************
 * @file main.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/04/2024
 *
 * @version 1.0
 *
 * @brief ESP32 Main entry point file.
 *
 * @details ESP32 Main entry point file. This file contains the main functions
 * of the ESP32 software module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>   /* Generic types */
#include <Arduino.h> /* Arduino Main Header File */
#include <HWLayer.h> /* Hardware services */
#include <Logger.h>  /* Logger */
#include <WiFi.h>
#include <OLED.h>    /* OLED screen */
#include <ESPAsyncWebServer.h>
#include <stdio.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
#define V_CALIB 8

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
static OLED sOledScreen;
const char* ssid = "ECONBATPROFILE";
const char* password = "ECONBATPROFILE";

AsyncWebServer server(80);

// Variable to store the HTTP request
String header;

float r1 = 222.33;
float r2 = 19.48;
float maxV = 8.44;
/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/**
 * @brief Setup function of the ESP32 software module.
 *
 * @details Setup function of the ESP32 software module. This function is used
 * to initialize the module's state and hardware. It is only called once on
 * board's or CPU reset.
 */
void setup(void);

/**
 * @brief Main execution loop of the ESP32 software module.
 *
 * @details Main execution loop of the ESP32 software module. This function
 * never returns and performs the main loop of the ESP32 software module.
 */
void loop(void);

void resquestFn(AsyncWebServerRequest* request);
/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void setup(void)
{
    Adafruit_SSD1306* pScreen;
    char uniqueHWUID[HW_ID_LENGTH + 1];

    /* Init logger */
    INIT_LOGGER(LOG_LEVEL_DEBUG, false);

    /* Get the unique hardware ID */
    strncpy(uniqueHWUID, HWLayer::GetHWUID(), HW_ID_LENGTH);
    uniqueHWUID[HW_ID_LENGTH] = 0;

    LOG_INFO("#========================#\n");
    LOG_INFO("| HWUID: %s    |\n", uniqueHWUID);
    LOG_INFO("| MAC: %s |\n", HWLayer::GetMacAddress())
    LOG_INFO("#========================#\n");

    sOledScreen.Init();

    pinMode(GPIO_NUM_32, INPUT_PULLDOWN);
    analogSetWidth(10);
    analogReadResolution(10);
    analogSetPinAttenuation(GPIO_NUM_32, ADC_0db);


    pScreen = sOledScreen.GetDisplay();
    pScreen->setCursor(0, 0);
    pScreen->clearDisplay();


    WiFi.softAP(ssid, password);

    server.on("/", HTTP_GET, resquestFn);
    server.begin();
}

void loop(void)
{
    Adafruit_SSD1306* pScreen;
    uint32_t value;
    float voltageDirect;
    uint32_t i;


    voltageDirect = 0;
    value = 0;
    for (i = 0; i < 100; ++i) {
        value += analogRead(GPIO_NUM_32);
        voltageDirect += (analogReadMilliVolts(GPIO_NUM_32) + V_CALIB) * ((r2 + r1) / r2);
        HWLayer::DelayExecUs(1000, false);
    }
    value /= i;
    voltageDirect /= i;

    pScreen = sOledScreen.GetDisplay();
    pScreen->setCursor(0, 0);
    pScreen->clearDisplay();
    pScreen->printf("Wifi IP: %s\n", WiFi.softAPIP().toString().c_str());
    pScreen->printf("\nDAC: %d\nVoltage (mV)\n   %f\n", value, voltageDirect);
    pScreen->display();
}

void resquestFn(AsyncWebServerRequest* request)
{
    uint32_t value;
    float voltageDirect;
    char response[512];
    uint32_t i;

    voltageDirect = 0;
    value = 0;
    for (i = 0; i < 100; ++i) {
        value += analogRead(GPIO_NUM_32);
        voltageDirect += (analogReadMilliVolts(GPIO_NUM_32) + V_CALIB) * ((r2 + r1) / r2);
        HWLayer::DelayExecUs(1000, false);
    }
    value /= i;
    voltageDirect /= i;


    memset(response, 0, sizeof(response));
    snprintf(response, 512, "%d, %f", value, voltageDirect);
    request->send(200, "text/html", response);
}