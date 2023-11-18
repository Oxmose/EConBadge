/*******************************************************************************
 * @file Logger.cpp
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>   /* Standard Int Types */
#include <Arduino.h> /* Serial service */
#include <HWLayer.h> /* HW Layer */
#include <Storage.h> /* Storage service */

/* Header file */
#include <Logger.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

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
ELogLevel Logger::loggingLevel = ELogLevel::LOG_LEVEL_NONE;
bool      Logger::fileLogging  = false;
bool      Logger::isInit       = false;
Storage * Logger::storage      = nullptr;
char      Logger::buffer[LOGGER_BUFFER_SIZE];

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void Logger::Init(const ELogLevel loglevel, const bool fileLog)
{
    if(!Logger::isInit)
    {
        Serial.begin(115200);
        Serial.printf("\n");

        Logger::isInit       = true;
        Logger::loggingLevel = loglevel;

        storage = Storage::GetInstance();

        if(!fileLog)
        {
            Logger::fileLogging = storage->GetFileLoggingState();
        }
        else
        {
            Logger::fileLogging = true;
        }
    }
}

void Logger::LogInfo(const char * str, ...)
{
    va_list argptr;
    size_t  len;

    if(Logger::isInit && Logger::loggingLevel >= ELogLevel::LOG_LEVEL_INFO)
    {
        snprintf(Logger::buffer, 20, "[INFO - %llu]", HWManager::GetTime());
        len = strlen(Logger::buffer);
        Logger::buffer[len++] = ' ';

        va_start(argptr, str);
        vsnprintf(Logger::buffer + len, LOGGER_BUFFER_SIZE - len, str, argptr);
        va_end(argptr);

        Logger::buffer[LOGGER_BUFFER_SIZE - 1] = 0;

        Serial.printf("%s", Logger::buffer);
        if(Logger::fileLogging)
        {
            storage->LogToSdCard(Logger::buffer, ELogLevel::LOG_LEVEL_INFO);
        }
    }
}

void Logger::LogError(const char * str, ...)
{
    va_list argptr;
    size_t  len;

    if(Logger::isInit && Logger::loggingLevel >= ELogLevel::LOG_LEVEL_ERROR)
    {
        snprintf(Logger::buffer, 21, "[ERROR - %llu]", HWManager::GetTime());
        len = strlen(Logger::buffer);
        Logger::buffer[len++] = ' ';

        va_start(argptr, str);
        vsnprintf(Logger::buffer + len, LOGGER_BUFFER_SIZE - len, str, argptr);
        va_end(argptr);

        Logger::buffer[LOGGER_BUFFER_SIZE - 1] = 0;

        Serial.printf("%s", Logger::buffer);
        if(Logger::fileLogging)
        {
            storage->LogToSdCard(Logger::buffer, ELogLevel::LOG_LEVEL_ERROR);
        }
    }
}

void Logger::LogDebug(const char * str, ...)
{
    va_list argptr;
    size_t  len;

    if(Logger::isInit && Logger::loggingLevel >= ELogLevel::LOG_LEVEL_DEBUG)
    {
        snprintf(Logger::buffer, 21, "[DEBUG - %llu]", HWManager::GetTime());
        len = strlen(Logger::buffer);
        Logger::buffer[len++] = ' ';

        va_start(argptr, str);
        vsnprintf(Logger::buffer + len, LOGGER_BUFFER_SIZE - len, str, argptr);
        va_end(argptr);

        Logger::buffer[LOGGER_BUFFER_SIZE - 1] = 0;

        Serial.printf("%s", Logger::buffer);
        if(Logger::fileLogging)
        {
            storage->LogToSdCard(Logger::buffer, ELogLevel::LOG_LEVEL_DEBUG);
        }
    }
}

void Logger::LogCritical(const char * str, ...)
{
    va_list argptr;
    size_t  len;

    snprintf(Logger::buffer, 20, "[CRIT - %llu]", HWManager::GetTime());
    len = strlen(Logger::buffer);
    Logger::buffer[len++] = ' ';

    va_start(argptr, str);
    vsnprintf(Logger::buffer + len, LOGGER_BUFFER_SIZE - len, str, argptr);
    va_end(argptr);

    Logger::buffer[LOGGER_BUFFER_SIZE - 1] = 0;

    Serial.printf("%s", Logger::buffer);
    if(Logger::fileLogging)
    {
        storage->LogToSdCard(Logger::buffer, ELogLevel::LOG_LEVEL_ERROR);
    }

    delay(5000);
    ESP.restart();
}


bool Logger::GetLogToFileState(void)
{
    return Logger::fileLogging;
}

void Logger::ToggleLogToFileState(void)
{
    Logger::fileLogging = !Logger::fileLogging;
}
