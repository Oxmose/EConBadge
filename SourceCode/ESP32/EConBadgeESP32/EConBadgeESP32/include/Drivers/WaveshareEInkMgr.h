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
#include <BlueToothMgr.h> /* Bluetooth Manager */

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

typedef enum
{
    EINK_CLEAR                = 0,
    EINK_SET_DISPLAYED_IMAGE  = 1,
    EINK_DISPLAY_NEW_IMAGE    = 2,
    EINK_SEND_DISPLAYED_IMAGE = 3,
    EINK_NO_ACTION
} EEInkAction;

typedef struct
{
    EEInkAction action;
    std::string parameter;
} SUpdateAction;

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
        EInkDisplayManager(BluetoothManager* pBtMgr);

        void Init(void);

        void Update(void);

        EErrorCode Clear(void);
        EErrorCode SetDisplayedImage(const std::string& rkFilename);
        EErrorCode DisplayNewImage(const std::string& rkFilename);
        EErrorCode SendDisplayedImage(void);

        void GetDisplayedImageName(std::string& rFileName) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        EErrorCode InternalClear(SCommandResponse* pResponse);
        EErrorCode InternalSetDisplayedImage(const std::string& rkFilename,
                                             SCommandResponse* pResponse);
        EErrorCode InternalDisplayNewImage(const std::string& rkFilename,
                                           SCommandResponse* pResponse);
        EErrorCode InternalSendDisplayedImage(SCommandResponse* pResponse);

        std::string       currentImageName_;

        Epd               eInkDriver_;
        Storage*          pStore_;
        BluetoothManager* pBtMgr_;
        SUpdateAction     updateAction_;
};

#endif /* #ifndef __DRIVERS_WAVESHARE_EINK_MGR_H_ */