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
    NO_CONNECTION   = 4
} EErrorCode;

typedef enum
{
    SYS_IDLE                = 0,
    SYS_WAITING_WIFI_CLIENT = 1
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