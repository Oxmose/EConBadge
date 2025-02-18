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
 * @details This file defines the logging module. This comprises a set of
 * functions used to log at different verbose levels.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <HWMgr.h>   /* HW Layer */
#include <cstdint>   /* Standard Int Types */
#include <Arduino.h> /* Serial service */

/* Header file */
#include <Logger.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief The log buffer size in bytes. */
#define LOGGER_BUFFER_SIZE 256

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
/** @brief See Logger.h */
bool Logger::ISINIT_ = false;
/** @brief See Logger.h */
ELogLevel Logger::LOGLEVEL_ = ELogLevel::LOG_LEVEL_NONE;

/*******************************************************************************
 * STATIC FUNCTIONS DECLARATIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void Logger::Init(const ELogLevel kLoglevel)
{
    if(!ISINIT_)
    {
        Serial.begin(115200);

        ISINIT_   = true;
        LOGLEVEL_ = kLoglevel;
    }
}

void Logger::LogLevel(const ELogLevel kLevel,
                      const char*     pkFile,
                      const uint32_t  kLine,
                      const char*     pkStr,
                      ...)
{
    va_list argptr;
    size_t  len;
    char    pTag[32];
    char*   pBuffer;

    if(ISINIT_ && LOGLEVEL_ >= kLevel)
    {
        pBuffer = new char[LOGGER_BUFFER_SIZE];
        if(pBuffer == nullptr)
        {
            return;
        }

        if(kLevel == LOG_LEVEL_ERROR)
        {
            memcpy(pTag, "[ERROR - %16llu] %s:%d -\0", 26);
        }
        else if (kLevel == LOG_LEVEL_INFO)
        {
            memcpy(pTag, "[INFO - %16llu]\0", 17);
        }
        else if (kLevel == LOG_LEVEL_DEBUG)
        {
            memcpy(pTag, "[DBG -  %16llu] %s:%d -\0", 24);
        }
        else
        {
            memcpy(pTag, "[UNKN - %16llu] %s:%d -\0", 25);
        }

        len = snprintf(pBuffer,
                       LOGGER_BUFFER_SIZE,
                       pTag,
                       HWManager::GetTime(),
                       pkFile,
                       kLine);
        pBuffer[len++] = ' ';

        va_start(argptr, pkStr);
        len += vsnprintf(pBuffer + len,
                         LOGGER_BUFFER_SIZE - len,
                         pkStr,
                         argptr);
        va_end(argptr);

        pBuffer[len] = 0;

        Serial.printf("%s", pBuffer);

        delete[] pBuffer;
    }
}