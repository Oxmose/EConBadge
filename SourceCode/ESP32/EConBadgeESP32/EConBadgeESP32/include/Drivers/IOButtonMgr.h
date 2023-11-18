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

#ifndef __DRIVERS_IOBUTTONMGR_H_
#define __DRIVERS_IOBUTTONMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */

#include <Types.h>            /* Defined Types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    BTN_STATE_UP   = 0,
    BTN_STATE_DOWN = 1,
    BTN_STATE_KEEP = 2
} EButtonState;

typedef enum
{
    BUTTON_UP    = 0,
    BUTTON_DOWN  = 1,
    BUTTON_ENTER = 2,
    BUTTON_BACK  = 3,
    BUTTON_MAX_ID
} EButtonID;

typedef enum
{
    ENTER_PIN = 0,
    DOWN_PIN  = 2,
    UP_PIN    = 4,
    BACK_PIN  = 32
} EButtonPin;

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

class IOButtonMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        IOButtonMgr(void);

        EErrorCode Init(void);

        EErrorCode SetupBtn(const EButtonID btnId,
                            const EButtonPin buttonPin);
        EErrorCode UpdateState(void);

        EButtonState GetButtonState(const EButtonID btnId) const;
        uint64_t     GetButtonKeepTime(const EButtonID btnId) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        int8_t       btnPins_[BUTTON_MAX_ID];
        uint64_t     btnLastPress_[BUTTON_MAX_ID];
        EButtonState btnStates_[BUTTON_MAX_ID];
};


#endif /* #ifndef __DRIVERS_IOBUTTONMGR_H_ */