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
bool      Logger::ISINIT_      = false;
bool      Logger::FILELOGGING_ = false;

char      Logger::PBUFFER_[LOGGER_BUFFER_SIZE];

ELogLevel Logger::LOGLEVEL_    = ELogLevel::LOG_LEVEL_NONE;
Storage * Logger::PSTORAGE_    = nullptr;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void Logger::Init(const ELogLevel kLoglevel, const bool kFileLog)
{
    if(!Logger::ISINIT_)
    {
        Serial.begin(115200);

        Logger::ISINIT_   = true;
        Logger::LOGLEVEL_ = kLoglevel;
        Logger::PSTORAGE_ = Storage::GetInstance();

        if(kFileLog == false)
        {
            Logger::FILELOGGING_ = Logger::PSTORAGE_->GetFileLoggingState();
        }
        else
        {
            Logger::FILELOGGING_ = true;
        }
    }
}

void Logger::LogLevel(const ELogLevel   kLevel,
                      const char      * pkFile,
                      const uint32_t    kLine,
                      const char      * pkStr,
                      ...)
{
    va_list argptr;
    size_t  len;
    char    pTag[24];

    if(Logger::ISINIT_ == true &&
       Logger::LOGLEVEL_ >= kLevel)
    {
        if(kLevel == LOG_LEVEL_ERROR)
        {
            memcpy(pTag, "[ERROR - %llu] %s:%d -\0", 23);
        }
        else if (kLevel == LOG_LEVEL_INFO)
        {
            memcpy(pTag, "[INFO - %llu]\0", 14);
        }
        else if (kLevel == LOG_LEVEL_DEBUG)
        {
            memcpy(pTag, "[DBG - %llu] %s:%d -\0", 21);
        }
        else
        {
            memcpy(pTag, "[UNKN - %llu] %s:%d -\0", 22);
        }

        len = snprintf(Logger::PBUFFER_,
                       LOGGER_BUFFER_SIZE,
                       pTag,
                       HWManager::GetTime(),
                       pkFile,
                       kLine);
        Logger::PBUFFER_[len++] = ' ';

        va_start(argptr, pkStr);
        len += vsnprintf(Logger::PBUFFER_ + len,
                         LOGGER_BUFFER_SIZE - len,
                         pkStr,
                         argptr);
        va_end(argptr);

        Logger::PBUFFER_[len] = 0;

        Serial.printf("%s", Logger::PBUFFER_);
        if(Logger::FILELOGGING_ == true)
        {
            PSTORAGE_->LogToSdCard(Logger::PBUFFER_, kLevel);
        }
    }
}

bool Logger::GetLogToFileState(void)
{
    return Logger::FILELOGGING_;
}

void Logger::ToggleLogToFileState(void)
{
    Logger::FILELOGGING_ = !Logger::FILELOGGING_;
}
