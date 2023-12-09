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

#define LOGGER_DEBUG_ENABLED 1

/*******************************************************************************
 * MACROS
 ******************************************************************************/
#define INIT_LOGGER(LEVEL, LOGFILE) { \
    Logger::Init(LEVEL, LOGFILE);     \
}

#define LOG_INFO(FMT, ...) {                                            \
    Logger::LogLevel(ELogLevel::LOG_LEVEL_INFO,                         \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

#define LOG_ERROR(FMT, ...) {                                           \
    Logger::LogLevel(ELogLevel::LOG_LEVEL_ERROR,                        \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

#if LOGGER_DEBUG_ENABLED

#define LOG_DEBUG(FMT, ...) {                                           \
    Logger::LogLevel(ELogLevel::LOG_LEVEL_DEBUG,                        \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

#else

#define LOG_DEBUG(FMT, ...)

#endif

#define LOGGER_FILE_STATE Logger::GetLogToFileState()

#define LOGGER_TOGGLE_FILE_LOG() Logger::ToggleLogToFileState()

#define LOGGER_BUFFER_SIZE 256

/*******************************************************************************
 * STRUCTURES AND TYPES
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
        static void Init        (const ELogLevel kLoglevel,
                                 const bool      kFileLog);

        static void LogLevel(const ELogLevel   kLevel,
                             const char      * pkFile,
                             const uint32_t    kLine,
                             const char      * pkStr,
                             ...);

        static bool GetLogToFileState    (void);
        static void ToggleLogToFileState (void);

    protected:

    private:
        static bool        ISINIT_;
        static bool        FILELOGGING_;
        static char        PBUFFER_[LOGGER_BUFFER_SIZE];

        static ELogLevel   LOGLEVEL_;
        static Storage   * PSTORAGE_;
};

#endif /* #ifndef __COMMON_LOGGER_H_ */