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

#include <string>  /* std::string */
#include <cstdint> /* Generic Types */
#include <Types.h> /* Defined Types */

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

        EErrorCode Update(void);

        EButtonState GetButtonState(const EButtonID kBtnId) const;
        uint64_t GetButtonKeepTime(const EButtonID kBtnId) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        EErrorCode SetupBtn(const EButtonID kBtnId,
                            const uint8_t   kBtnPin);

        int8_t       pBtnPins_[BUTTON_MAX_ID];
        uint64_t     pBtnLastPress_[BUTTON_MAX_ID];
        EButtonState pBtnStates_[BUTTON_MAX_ID];
};


#endif /* #ifndef __DRIVERS_IOBUTTONMGR_H_ */