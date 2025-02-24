/*******************************************************************************
 * @file WaveshareEInkMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 21/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the Waveshare EInk Display Driver.
 *
 * @details This file contains the Waveshare EInk Display Driver. The file
 * provides the services to update the screen, enable and disable it.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __DRIVERS_WAVESHARE_EINK_MGR_H_
#define __DRIVERS_WAVESHARE_EINK_MGR_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <string>          /* std::string */
#include <Types.h>         /* Defined Types */
#include <Storage.h>       /* Storage service */
#include <BlueToothMgr.h>  /* Bluetooth Manager */
#include <WaveshareEInk.h> /* EInk Driver */

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
 * CLASSES
 ******************************************************************************/

class EInkDisplayManager
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Construct a new EInkDisplayManager object
         *
         * @details Initializes the EInk display manager internal data.
         *
         * @param[in, out] pBtMgr The bluetooth manager that the manager will
         * use for data transfer.
         */
        EInkDisplayManager(BluetoothManager* pBtMgr);

        /**
         * @brief Initializes the EInk display.
         *
         * @details Following the internal initialization when the object is
         * constructed, this function initializes the EInk display.
         */
        void Init(void);

        /**
         * @brief Clears the EInk display.
         *
         * @details Clears the EInk display. The current image name is also
         * reset.
         *
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void Clear(SCommandResponse& rResponse);


        /**
         * @brief Removes and image.
         *
         * @details Removes and image. If the image is the current image, the
         * eInk display is cleared.
         *
         * @param[in] rkFilename The image to remove.
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void RemoveImage(const std::string& rkFilename,
                         SCommandResponse& rResponse);

        /**
         * @brief Selects and displays a stored image.
         *
         * @details Selects and displays a stored image. The image must be
         * stored in the SD card.
         *
         * @param[in] rkFilename The name of the image to display.
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void SetDisplayedImage(const std::string& rkFilename,
                               SCommandResponse&  rResponse);

        /**
         * @brief Downloads and displays a new image.
         *
         * @details Downloads and displays a new image. The image will be
         * stored in the SD card.
         *
         * @param[in] rkFilename The name of the image to display.
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void DisplayNewImage(const std::string& rkFilename,
                             SCommandResponse&  rResponse);

        /**
         * @brief Sends the currently displayed image.
         *
         * @details Sends the currently displayed image. The image will be read
         * from the SD card and sent.
         *
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void SendDisplayedImage(SCommandResponse&  rResponse) const;

        /**
         * @brief Gets the currently displayed image name.
         *
         * @details Gets the currently displayed image name.
         *
         * @param[out] rResponse The result of the action to be sent to the
         * client that requested the action.
         */
        void GetDisplayedImageName(SCommandResponse& rResponse) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the name of the currently displayed image. */
        std::string       currentImageName_;
        /** @brief Stores the storage singleton. */
        Storage*          pStore_;
        /** @brief Stores the EInk driver. */
        WaveshareDriver   eInkDriver_;
        /** @brief Stores the bluetooth manager. */
        BluetoothManager* pBtMgr_;
};

#endif /* #ifndef __DRIVERS_WAVESHARE_EINK_MGR_H_ */