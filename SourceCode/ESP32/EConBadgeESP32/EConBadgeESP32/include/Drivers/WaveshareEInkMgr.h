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

#include <cstdint> /* Generic Types */
#include <string>  /* String */
#include <Types.h> /* Defined Types */
#include <SystemState.h> /* System state manager */
#include <epd5in65f.h>        /* EInk Driver */
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
        EInkDisplayManager(SystemState * systemState, BluetoothManager * btMgr);
        ~EInkDisplayManager(void);

        void Init(void);

        void Update(void);
        void RequestClear(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void Clear(void);
        void UpdateDisplay(void);

        Epd eInkDriver_;

        SystemState *      systemState_;
        BluetoothManager * btMgr_;
};

#endif /* #ifndef __DRIVERS_WAVESHARE_EINK_MGR_H_ */