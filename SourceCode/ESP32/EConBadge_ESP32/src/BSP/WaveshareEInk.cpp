/*******************************************************************************
 * @file WaveshareEink.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/02/2025
 *
 * @version 1.0
 *
 * @brief This file provides the driver for the Waveshare EINK display.
 *
 * @details This file provides the driver for the Waveshare EINK display. The
 * driver provides functionalities to interract with the display.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*****************************************************************************
* | File      	:   EPD_5in65f.c
* | Author      :   Waveshare team
* | Function    :   5.65inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-07-08
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint>   /* Generic types */
#include <Types.h>   /* Custom types */
#include <HWMgr.h>   /* Hardware manager */
#include <Arduino.h> /* Arduino services */

/* Header File */
#include <WaveshareEInk.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Waits on the BUSY line until it is high.
 */
#define BUSY_HIGH_WAIT() while(!digitalRead(GPIO_EINK_BUSY)){}
/**
 * @brief Waits on the BUSY line until it is low.
 */
#define BUSY_LOW_WAIT() while(digitalRead(GPIO_EINK_BUSY)){}

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

WaveshareDriver::WaveshareDriver(void)
{
}

void WaveshareDriver::Init(void)
{
    /* Initialize the SPI bus */
    pinMode(GPIO_EINK_CS, OUTPUT);
    pinMode(GPIO_EINK_RESET, OUTPUT);
    pinMode(GPIO_EINK_DC, OUTPUT);
    pinMode(GPIO_EINK_BUSY, INPUT);
    EINK_SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

    /* Initialization sequence */
    Reset();
    BUSY_HIGH_WAIT();
    SendCommand(0x00);
    SendData(0xEF);
    SendData(0x08);
    SendCommand(0x01);
    SendData(0x37);
    SendData(0x00);
    SendData(0x23);
    SendData(0x23);
    SendCommand(0x03);
    SendData(0x00);
    SendCommand(0x06);
    SendData(0xC7);
    SendData(0xC7);
    SendData(0x1D);
    SendCommand(0x30);
    SendData(0x3C);
    SendCommand(0x41);
    SendData(0x00);
    SendCommand(0x50);
    SendData(0x37);
    SendCommand(0x60);
    SendData(0x22);
    SendCommand(0x61);
    SendData(0x02);
    SendData(0x58);
    SendData(0x01);
    SendData(0xC0);
    SendCommand(0xE3);
    SendData(0xAA);
    HWManager::DelayExecUs(100000);
    SendCommand(0x50);
    SendData(0x37);
    HWManager::DelayExecUs(200000);
}

void WaveshareDriver::SendCommand(const uint8_t kCommand)
{
    digitalWrite(GPIO_EINK_DC, LOW);
    digitalWrite(GPIO_EINK_CS, LOW);
    EINK_SPI.transfer(kCommand);
    digitalWrite(GPIO_EINK_CS, HIGH);
}

void WaveshareDriver::SendData(const uint8_t kData)
{
    digitalWrite(GPIO_EINK_DC, HIGH);
    digitalWrite(GPIO_EINK_CS, LOW);
    EINK_SPI.transfer(kData);
    digitalWrite(GPIO_EINK_CS, HIGH);
}

void WaveshareDriver::Reset(void)
{
    digitalWrite(GPIO_EINK_RESET, LOW);
    HWManager::DelayExecUs(1000);
    digitalWrite(GPIO_EINK_RESET, HIGH);
    HWManager::DelayExecUs(500000);
}

void WaveshareDriver::Display(const uint8_t* pImage)
{
    uint32_t i;
    uint32_t j;

    /* Set Resolution setting */
    SendCommand(0x61);
    SendData(0x02);
    SendData(0x58);
    SendData(0x01);
    SendData(0xC0);
    SendCommand(0x10);

    /* Send the image */
    for(i = 0; i < EPD_HEIGHT; ++i)
    {
        for(j = 0; j < EPD_WIDTH / 2; ++j)
        {
            SendData(pImage[j + ((EPD_WIDTH / 2) * i)]);
        }
    }

    SendCommand(0x04);
    BUSY_HIGH_WAIT();
    SendCommand(0x12);
    BUSY_HIGH_WAIT();
    SendCommand(0x02);
    BUSY_LOW_WAIT();
    HWManager::DelayExecUs(500000);
}

void WaveshareDriver::DisplayInitTrans(void)
{
    /* Set the resolution setting */
    SendCommand(0x61);
    SendData(0x02);
    SendData(0x58);
    SendData(0x01);
    SendData(0xC0);
    SendCommand(0x10);
}

void WaveshareDriver::DisplayPerformTrans(const uint8_t* pkBuffer,
                                          const uint32_t kSize)
{
    uint32_t i;

    for(i = 0; i < kSize; ++i)
    {
        SendData(pkBuffer[i]);
    }
}

void WaveshareDriver::DisplayEndTrans(void)
{
    SendCommand(0x04);
    BUSY_HIGH_WAIT();
    SendCommand(0x12);
    BUSY_HIGH_WAIT();
    SendCommand(0x02);
    BUSY_LOW_WAIT();
    HWManager::DelayExecUs(500000);
}

void WaveshareDriver::DisplayPart(const uint8_t* pImage,
                                  uint32_t       xStart,
                                  uint32_t       yStart,
                                  uint32_t       imageWidth,
                                  uint32_t       imageHeigh)
{
    uint32_t i;
    uint32_t j;

    /* Set Resolution setting */
    SendCommand(0x61);
    SendData(0x02);
    SendData(0x58);
    SendData(0x01);
    SendData(0xC0);
    SendCommand(0x10);

    for(i = 0; i < EPD_HEIGHT; ++i)
    {
        for(j = 0; j < EPD_WIDTH / 2; ++j)
        {
            if(i < imageHeigh + yStart &&
               i >= yStart &&
               j < (imageWidth + xStart) / 2 &&
               j >= xStart / 2)
            {
                SendData(
                    pgm_read_byte(&pImage[(j - xStart / 2) +
                                          (imageWidth / 2 * (i - yStart))])
                );
            }
            else
            {
                SendData(0x11);
            }
        }
    }
    SendCommand(0x04);
    BUSY_HIGH_WAIT();
    SendCommand(0x12);
    BUSY_HIGH_WAIT();
    SendCommand(0x02);
    BUSY_LOW_WAIT();
    HWManager::DelayExecUs(500000);
}

void WaveshareDriver::Clear(const uint8_t kColor)
{
    uint32_t i;
    uint32_t j;

    /* Set Resolution setting */
    SendCommand(0x61);
    SendData(0x02);
    SendData(0x58);
    SendData(0x01);
    SendData(0xC0);
    SendCommand(0x10);

    for(i = 0; i < EPD_WIDTH / 2; ++i)
    {
        for(j = 0; j < EPD_HEIGHT; ++j)
        {
            SendData((kColor << 4) | kColor);
        }
    }

    SendCommand(0x04);
    BUSY_HIGH_WAIT();
    SendCommand(0x12);
    BUSY_HIGH_WAIT();
    SendCommand(0x02);
    BUSY_LOW_WAIT();
    HWManager::DelayExecUs(500000);
}

void WaveshareDriver::Sleep(void)
{
    HWManager::DelayExecUs(100000);
    SendCommand(0x07);
    SendData(0xA5);
    HWManager::DelayExecUs(100000);
    digitalWrite(GPIO_EINK_RESET, 0);
    HWManager::DelayExecUs(50000);

    /* End the SPI transation */
    EINK_SPI.endTransaction();
}