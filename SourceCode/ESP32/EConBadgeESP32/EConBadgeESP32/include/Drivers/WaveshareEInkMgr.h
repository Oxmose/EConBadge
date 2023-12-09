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

#include <string>         /* std::string */
#include <cstdint>        /* Generic Types */
#include <Types.h>        /* Defined Types */
#include <EPD_5in65f.h>   /* EInk Driver */
#include <SystemState.h>  /* System state manager */
#include <BlueToothMgr.h> /* Bluetooth Manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define INTERNAL_BUFFER_SIZE 8192

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
        EInkDisplayManager  (SystemState      * pSystemState,
                             BluetoothManager * pBtMgr);

        void Init (void);

        void Update                (void);
        void RequestClear          (void);
        void SetDisplayedImage     (const std::string & rkFilename);
        void GetDisplayedImageName (std::string & rFileName) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void Clear                  (void);
        void DownloadAndUpdateImage (const std::string & rkFilename);
        void SendImage              (const std::string & rkFilename);
        void FormatFilename         (char * pFilename);

        uint8_t            pInternalBuffer_[INTERNAL_BUFFER_SIZE];

        std::string        currentImageName_;

        Epd                eInkDriver_;
        SystemState      * pSystemState_;
        BluetoothManager * pBtMgr_;
        Storage          * pStore_;
};

#endif /* #ifndef __DRIVERS_WAVESHARE_EINK_MGR_H_ */