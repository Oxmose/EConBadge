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
 * @details This file defines the logging module. This comprises a set of
 * functions used to log at different verbose levels.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMON_LOGGER_H_
#define __COMMON_LOGGER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint> /* Standard Int Types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Initializes the logger.
 *
 * @details Initializes the logger with the requested log level.
 *
 * @param[in] LEVEL The level at which the logger shall be initialized.
 */
#define INIT_LOGGER(LEVEL) Logger::Init(LEVEL)

/**
 * @brief Logs an information to the log buffer.
 *
 * @details Logs an information to the log buffer. The INFO tag will be added to
 * the log.
 *
 * @param[in] FMT The format string used for the log.
 * @param[in] ... The format arguments.
 */
#define LOG_INFO(FMT, ...) {                                            \
    Logger::LogLevel(ECB_LOG_LEVEL_INFO,                                \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

/**
 * @brief Logs an error to the log buffer.
 *
 * @details Logs an error to the log buffer. The ERROR tag will be added to
 * the log.
 *
 * @param[in] FMT The format string used for the log.
 * @param[in] ... The format arguments.
 */
#define LOG_ERROR(FMT, ...) {                                           \
    Logger::LogLevel(ECB_LOG_LEVEL_ERROR,                               \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

#if LOGGER_DEBUG_ENABLED

/**
 * @brief Logs a debug to the log buffer.
 *
 * @details Logs a debug to the log buffer. The DEBUG tag will be added to
 * the log.
 *
 * @param[in] FMT The format string used for the log.
 * @param[in] ... The format arguments.
 */
#define LOG_DEBUG(FMT, ...) {                                           \
    Logger::LogLevel(ECB_LOG_LEVEL_DEBUG,                               \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     FMT,                                               \
                     ##__VA_ARGS__);                                    \
}

#else

#define LOG_DEBUG(FMT, ...)

#endif

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/** @brief Defines the log level. */
typedef enum
{
    /** @brief Logging level: no logging */
    ECB_LOG_LEVEL_NONE  = 0,
    /** @brief Logging level: log only errors */
    ECB_LOG_LEVEL_ERROR = 1,
    /** @brief Logging level: log previous levels and information */
    ECB_LOG_LEVEL_INFO  = 2,
    /** @brief Logging level: log previous levels and debug output */
    ECB_LOG_LEVEL_DEBUG = 3
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

/**
 * @brief The logger class.
 *
 * @details The logger class that provides the necessary functions to log
 * different log level messages to the serial port.
 */
class Logger
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Initializes the logger.
         *
         * @details Initializes the logger with the requested log level.
         *
         * @param[in] kLoglevel The log level to use.
         */
        static void Init(const ELogLevel kLoglevel);

        /**
         * @brief Logs a message to the logger.
         *
         * @details Logs a message to the logger. If the log level is above the
         * logger current level, the message is discarded.
         *
         * @param[in] kLevel The lovel of the message to log.
         * @param[in] pkFile The file where the log was generated.
         * @param[in] kLine The line where the log was generated.
         * @param[in] pkStr The format string used for the log
         * @param[in] ... The format arguments.
         */
        static void LogLevel(const ELogLevel kLevel,
                             const char*     pkFile,
                             const uint32_t  kLine,
                             const char*     kStr,
                             ...);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Tells if the logger is initialized. */
        static bool      ISINIT_;
        /** @brief The current log level. */
        static ELogLevel LOGLEVEL_;
};

#endif /* #ifndef __COMMON_LOGGER_H_ */