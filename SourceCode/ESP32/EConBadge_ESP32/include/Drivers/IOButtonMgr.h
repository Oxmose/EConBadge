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

/** @brief Defines the button states. */
typedef enum
{
    /** @brief Button is released */
    BTN_STATE_UP   = 0,
    /** @brief Button is pressed */
    BTN_STATE_DOWN = 1,
    /** @brief Button has been pressed for more than a specified time */
    BTN_STATE_KEEP = 2
} EButtonState;

/** @brief Defines the button functionalities. */
typedef enum
{
    /** @brief Button function: up */
    BUTTON_UP    = 0,
    /** @brief Button function: down */
    BUTTON_DOWN  = 1,
    /** @brief Button function: enter */
    BUTTON_ENTER = 2,
    /** @brief Button function: back */
    BUTTON_BACK  = 3,
    /** @brief Max ID */
    BUTTON_MAX_ID
} EButtonID;

/** @brief Defines a button action */
typedef struct SButtonAction
{
    /** @brief The button identifier */
    EButtonID    id;
    /** @brief The corresponding state.  */
    EButtonState state;

    /** @brief If the button state is KEEP, the time since it entered this
     * state.
     */
    uint64_t     keepTime;
} SButtonAction;


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

/**
 * @brief Button manager class.
 *
 * @details Button manager class. This class provides the services
 * to retrieve the button states.
 */
class IOButtonMgr
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Constructs a new IOButtonMgr object.
         *
         * @details Constructs a new IOButtonMgr object.
         * The constructor initializes the buttons and their GPIOs.
         */
        IOButtonMgr(void);

        /**
         * @brief Updates the button states.
         *
         * @details Updates the button states. This function also calculate the
         * time a button has been pressed to setup the KEEP state.
         */
        void Update(void);

        /**
         * @brief Get the Button State.
         *
         * @param[in] kBtnId The button for which the state shall be returned.
         *
         * @return The function returns the button state.
         */
        EButtonState GetButtonState(const EButtonID kBtnId) const;

        /**
         * @brief Get the Button Keep Time.
         *
         * @param[in] kBtnId The button for which the keep state shall be
         * returned.
         * @return The function returns the time the button has been in the
         * KEPP state.
         */
        uint64_t GetButtonKeepTime(const EButtonID kBtnId) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the buttons GPIO pins */
        int8_t       pBtnPins_[BUTTON_MAX_ID];
        /** @brief Stores the button GPIO pin mux */
        int8_t       pBtnPinsMux_[BUTTON_MAX_ID];
        /** @brief Stores the time as which the buttons were pressed last. */
        uint64_t     pBtnLastPress_[BUTTON_MAX_ID];
        /** @brief Stores the buttons state. */
        EButtonState pBtnStates_[BUTTON_MAX_ID];
};

#endif /* #ifndef __DRIVERS_IOBUTTONMGR_H_ */