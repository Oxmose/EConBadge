/*******************************************************************************
 * @file Logger.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file defines the logging module.
 *
 * @details This file defines the logging module. This comprises a set of MACRO
 * used to log at different verbose levels.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMON_LOGGER_H_
#define __COMMON_LOGGER_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>   /* Standard Int Types */
#include <Arduino.h> /* Serial service */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Logging level: no logging */
#define LOG_LEVEL_NONE  0
/** @brief Logging level: log only errors */
#define LOG_LEVEL_ERROR 1
/** @brief Logging level: log previous levels and warnings */
#define LOG_LEVEL_WARN  2
/** @brief Logging level: log previous levels and information */
#define LOG_LEVEL_INFO  3
/** @brief Logging level: log previous levels and debug output */
#define LOG_LEVEL_DEBUG 4

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define INIT_LOGGER(LOG_LEVEL) {            \
    nsCommon::loggingLevel = LOG_LEVEL;     \
    Serial.begin(115200);                   \
    Serial.printf("\n");                    \
}

#define LOG_INFO(FMT, ...) {                        \
    if(nsCommon::loggingLevel >= LOG_LEVEL_INFO)    \
    {                                               \
        Serial.printf("[INFO] ");                   \
        Serial.printf(FMT, ##__VA_ARGS__);          \
    }                                               \
}

#define LOG_WARNING(FMT, ...) {                     \
    if(nsCommon::loggingLevel >= LOG_LEVEL_WARNING) \
    {                                               \
        Serial.printf("[WARNING] ");                \
        Serial.printf(FMT, ##__VA_ARGS__);          \
    }                                               \
}

#define LOG_ERROR(FMT, ...) {                       \
    if(nsCommon::loggingLevel >= LOG_LEVEL_ERROR)   \
    {                                               \
        Serial.printf("[ERROR] ");                  \
        Serial.printf(FMT, ##__VA_ARGS__);          \
    }                                               \
}

#define LOG_DEBUG(FMT, ...) {                       \
    if(nsCommon::loggingLevel >= LOG_LEVEL_DEBUG)   \
    {                                               \
        Serial.printf("[DEBUG] ");                  \
        Serial.printf(FMT, ##__VA_ARGS__);          \
    }                                               \
}

#define LOG_CRITICAL(FMT, ...) {                    \
    Serial.printf("[CRITICAL] ");                   \
    Serial.printf(FMT, ##__VA_ARGS__);              \
    /* TODO: Put in sleep mode */                   \
}

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
/* None */

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
    /* None */

/************************* Exported global variables **************************/
extern uint8_t loggingLevel;

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