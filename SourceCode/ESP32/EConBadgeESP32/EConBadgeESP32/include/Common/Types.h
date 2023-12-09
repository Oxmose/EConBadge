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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Standard Int Types */
#include <vector>  /* std::vector */
#include <string>  /* std::string */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SPLASH_TIME                3000
#define DEBUG_BTN_PRESS_TIME       3000
#define MENU_BTN_PRESS_TIME        1000

#define LOGO_WIDTH    57
#define LOGO_HEIGHT   41

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

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
    /** @brief An invalid parameter was used */
    INVALID_PARAM   = 1,
    /** @brief The Action failed */
    ACTION_FAILED   = 2,
    /** @brief Component was not initalialized */
    NOT_INITIALIZED = 3,
    /** @brief No action to be done */
    NO_ACTION       = 4,
} EErrorCode;

typedef std::vector<std::pair<std::string, uint32_t>> ImageList;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
extern const unsigned char PKLOGO_BITMAP [];

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


#endif /* #ifndef __COMMON_TYPES_H_ */