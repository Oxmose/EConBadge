/*******************************************************************************
 * @file OLEDScreenMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the OLED screen manager.
 *
 * @details This file contains the OLED screen manager. The file provides the
 * services to update the screen, enable and disable it.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __HWLAYER_OLEDSCREENMGR_H_
#define __HWLAYER_OLEDSCREENMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */
#include <Types.h> /* Defined Types */

#include <Adafruit_GFX.h>     /* OLED Screen Manipulation */
#include <Adafruit_SSD1306.h> /* OLED Screen Driver */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Hardware Layer Namespace
 * @details Hardware Layer Namespace used for definitions of hardware related
 * services.
 */
namespace nsHWL
{

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

class COLEDScreenMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        COLEDScreenMgr(void);
        ~COLEDScreenMgr(void);
        nsCommon::EErrorCode Init(void);
        nsCommon::EErrorCode UpdateState(nsCore::CSystemState & sysState,
                                         const nsCore::CCommandControler & comControler);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void DisplaySplash(void);
        void DisplayDebug(const nsCore::CSystemState & sysState);
        void DisplayMenu(const nsCore::CSystemState & sysState);

        Adafruit_SSD1306       * display;
        nsCommon::ESystemState   lastState;
};

} /* namespace nsHWL */

#endif /* #ifndef __HWLAYER_OLEDSCREENMGR_H_ */