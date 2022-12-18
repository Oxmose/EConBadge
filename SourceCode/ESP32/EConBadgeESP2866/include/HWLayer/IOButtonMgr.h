/*******************************************************************************
 * @file IOButtonMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 18/12/2022
 *
 * @version 1.0
 *
 * @brief This file contains the IO buttons manager.
 *
 * @details This file contains the IO buttons manager. The file provides the
 * services read input buttons and associate interrupts to the desired pins.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __HWLAYER_IOBUTTONMGR_H_
#define __HWLAYER_IOBUTTONMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */

#include <Types.h>            /* Defined Types */
#include <CommandControler.h> /* Command controller service */
#include <SystemState.h>      /* System State Service */

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

typedef void (*TInputBtnHandler)(void);

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

class CIOButtonMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CIOButtonMgr(void);

        nsCommon::EErrorCode SetupBtn(const nsCommon::EButtonID btnId,
                                      const uint8_t buttonPin);
        nsCommon::EErrorCode UpdateState(nsCore::CSystemState & sysState,
                                         const nsCore::CCommandControler & comControler);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        nsCommon::EErrorCode AddBtnListener(const nsCommon::EButtonID btnId,
                                            TInputBtnHandler listener);

        int8_t btnPins[nsCommon::BUTTON_MAX_ID];
        uint32_t btnLastPress[nsCommon::BUTTON_MAX_ID];
        nsCommon::EButtonState btnStates[nsCommon::BUTTON_MAX_ID];
        TInputBtnHandler btnHandlers[nsCommon::BUTTON_MAX_ID];
};

} /* namespace nsHWL */

#endif /* #ifndef __HWLAYER_IOBUTTONMGR_H_ */