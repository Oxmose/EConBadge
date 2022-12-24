/*******************************************************************************
 * @file Types.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file defines the types used in the ESP32 module.
 *
 * @details This file defines the types used in the ESP32 module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMON_TYPES_H_
#define __COMMON_TYPES_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Standard Int Types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SYSTEM_COMMAND_ARGS_LENGTH 64
#define SPLASH_TIME                5000
#define DEBUG_BTN_PRESS_TIME       3000
#define MENU_BTN_PRESS_TIME        1500

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Common Namespace
 * @details Common Namespace used for common definitions that are shared in all
 * the ESP32 module.
 */
namespace nsCommon
{

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Defines the error status type.
 */
typedef enum
{
    /** @brief No error occured. */
    NO_ERROR        = 0,
    /** @brief The requested action failed. */
    ACTION_FAILED   = 1,
    NOT_INITIALIZED = 2,
    NO_ACTION       = 3,
    NO_CONNECTION   = 4,
    INVALID_PARAM   = 5
} EErrorCode;

typedef enum
{
    SYS_IDLE         = 0,
    SYS_START_SPLASH = 1,
    SYS_MENU         = 2,
    SYS_MENU_WIFI    = 3
} ESystemState;

typedef enum
{
    COMM_PING_ID = 0,
    COMM_MAX_ID
} ESystemCommandId;

typedef struct
{
    uint32_t   command;
    uint8_t  * args[SYSTEM_COMMAND_ARGS_LENGTH];
} SSystemCommand;

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
    BUTTON_MAX_ID
} EButtonID;

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON  = 1
} ELEDState;

typedef enum
{
    LED_MAIN     = 0,
    LED_AUX      = 1,
    LED_MAX_ID
} ELEDID;

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

    /* None */

} /* namespace nsCommon */

#endif /* #ifndef __COMMON_TYPES_H_ */