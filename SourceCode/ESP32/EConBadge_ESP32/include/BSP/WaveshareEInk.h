/*******************************************************************************
 * @file WaveshareEink.h
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
* | File      	:   EPD_5in65f.h
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

#ifndef __BSP_WAVESHAREEINK_H_
#define __BSP_WAVESHAREEINK_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint> /* Generic types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief EINK Screen width in pixels. */
#define EPD_WIDTH 600
/** @brief EINK Screen height in pixels. */
#define EPD_HEIGHT 448

/** @brief EINK Color: Black */
#define EPD_5IN65F_BLACK 0x0
/** @brief EINK Color: White */
#define EPD_5IN65F_WHITE 0x1
/** @brief EINK Color: Green */
#define EPD_5IN65F_GREEN 0x2
/** @brief EINK Color: Blue */
#define EPD_5IN65F_BLUE 0x3
/** @brief EINK Color: Red */
#define EPD_5IN65F_RED 0x4
/** @brief EINK Color: Yellow */
#define EPD_5IN65F_YELLOW 0x5
/** @brief EINK Color: Orange */
#define EPD_5IN65F_ORANGE 0x6
/** @brief EINK Color: Clean */
#define EPD_5IN65F_CLEAN 0x7

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
 * CLASSES
 ******************************************************************************/

class WaveshareDriver
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Construct a new Waveshare Driver object
         *
         * @details This function will initialize the driver and the screen.
         */
        WaveshareDriver(void);

        /**
         * @brief This function will initialize the driver and the screen.
         *
         */
        void Init(void);

        /**
         * @brief Module reset.
         *
         * @details often used to awaken the module in deep sleep, see Sleep();
         */
        void Reset(void);

        /**
         * @brief Send the image to the screen.
         *
         * @details Sends the image buffer in RAM to e-Paper and displays.
         *
         * @param[in] pImage The buffer that contains the image to send.
         */
        void Display(const uint8_t* pImage);

        /**
         * @brief Init the transaction to send the image buffer in RAM to
         * e-Paper.
         */
        void DisplayInitTrans(void);

        /**
         * @brief Send the image buffer in RAM to e-Paper.
         *
         * @param[in] pkBuffer The buffer that contains the image.
         * @param[in] kSize The size of the buffer.
         */
        void DisplayPerformTrans(const uint8_t* pkBuffer, const uint32_t kSize);

        /**
         * @brief End the transaction to send the image buffer in RAM to e-Paper
         * and display.
         */
        void DisplayEndTrans(void);

        /**
         * @brief Sends the part image buffer in RAM to e-Paper and displays.
         *
         * @param[in] pImage The image to send.
         * @param[in] xStart The x position to start to send.
         * @param[in] yStart The y position to start to send.
         * @param[in] imageWidth The width to send.
         * @param[in] imageHeigh The height to send.
         */
        void DisplayPart(const uint8_t* pImage,
                         uint32_t       xStart,
                         uint32_t       yStart,
                         uint32_t       imageWidth,
                         uint32_t       imageHeigh);

        /**
         * @brief Basic function for sending commands.
         *
         * @param[in] command The command to send.
         */
        void SendCommand(const uint8_t kCommand);

        /**
         * @brief Basic function for sending data.
         *
         * @param[in] command The command to send.
         */
        void SendData(const uint8_t kData);

        /**
         * @brief Clear the screen with a given color.
         *
         * @param[in] kColor The color to use.
         */
        void Clear(const uint8_t kColor);

        /**
         * @brief Puts the screen to sleep.
         *
         * @details After this command is transmitted, the chip would enter the
         *          deep-sleep mode to save power.
         *          The deep sleep mode would return to standby by hardware
         *          reset.
         *          The only one parameter is a check code, the command would be
         *          You can use Reset() to awaken.
         */
        void Sleep(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /* None */
};

#endif /* #ifndef __BSP_WAVESHAREEINK_H_ */