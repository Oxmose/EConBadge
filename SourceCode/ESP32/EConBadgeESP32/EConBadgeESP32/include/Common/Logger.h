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
 * @details This file defines the logging module. This comprises a set of func
 * used to log at different verbose levels.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMON_LOGGER_H_
#define __COMMON_LOGGER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>   /* Standard Int Types */
#include <Arduino.h> /* Serial service */
#include <HWLayer.h> /* HW Layer */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

typedef enum
{
    /** @brief Logging level: no logging */
    LOG_LEVEL_NONE  = 0,
    /** @brief Logging level: log only errors */
    LOG_LEVEL_ERROR = 1,
    /** @brief Logging level: log previous levels and information */
    LOG_LEVEL_INFO  = 2,
    /** @brief Logging level: log previous levels and debug output */
    LOG_LEVEL_DEBUG = 3
} ELogLevel;

/*******************************************************************************
 * MACROS
 ******************************************************************************/
#define INIT_LOGGER(LEVEL, LOGFILE) { \
    Logger::Init(LEVEL, LOGFILE);     \
}

#define LOG_INFO(FMT, ...) {                \
    Logger::LogInfo(FMT, ##__VA_ARGS__);    \
}

#define LOG_ERROR(FMT, ...) {               \
    Logger::LogError(FMT, ##__VA_ARGS__);   \
}

#define LOG_DEBUG(FMT, ...) {               \
    Logger::LogDebug(FMT, ##__VA_ARGS__);   \
}

#define LOG_CRITICAL(FMT, ...) {                \
    Logger::LogCritical(FMT, ##__VA_ARGS__);    \
}

#define LOGGER_FILE_STATE Logger::GetLogToFileState()

#define LOGGER_TOGGLE_FILE_LOG() Logger::ToggleLogToFileState()

#define LOGGER_BUFFER_SIZE 256

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
/* Forward declaration */
class Storage;

class Logger
{
    public:
        static void Init(const ELogLevel loglevel, const bool fileLog);
        static void LogInfo(const char * str, ...);
        static void LogError(const char * str, ...);
        static void LogDebug(const char * str, ...);
        static void LogCritical(const char * str, ...);

        static bool GetLogToFileState(void);
        static void ToggleLogToFileState(void);

    protected:

    private:
        static bool      isInit;
        static bool      fileLogging;
        static ELogLevel loggingLevel;
        static Storage * storage;
        static char      buffer[LOGGER_BUFFER_SIZE];
};

#endif /* #ifndef __COMMON_LOGGER_H_ */