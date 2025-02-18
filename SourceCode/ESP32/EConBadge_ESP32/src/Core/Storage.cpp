/*******************************************************************************
 * @file Storage.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 12/11/2023
 *
 * @version 1.0
 *
 * @brief This file provides the storage management service.
 *
 * @details This file provides the storage management service. This files
 * defines the different features embedded for the storafe service.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <map>      /* std::map */
#include <SD.h>     /* SD Card driver */
#include <HWMgr.h>  /* Hardware manager */
#include <cstdint>  /* Generic Int types */
#include <Logger.h> /* Logger service */

/* Header File */
#include <Storage.h>

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
/** @brief The storage singleton instance. */
Storage* Storage::PINSTANCE_ = nullptr;

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
 * CLASS METHODS
 ******************************************************************************/

Storage* Storage::GetInstance(void)
{
    if (Storage::PINSTANCE_ == nullptr)
    {
        Storage::PINSTANCE_ = new Storage();
    }

    return Storage::PINSTANCE_;
}

sdcard_type_t Storage::GetSdCardType(void) const
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return CARD_NONE;
    }

    return SD.cardType();
}

uint64_t Storage::GetSdCardSize(void) const
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return 0;
    }

    return SD.cardSize();
}

bool Storage::CreateDirectory(const std::string& rkPath)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }

    if(SD.exists(rkPath.c_str()))
    {
        return true;
    }
    return SD.mkdir(rkPath.c_str());
}

File Storage::Open(const std::string& rkFilename, const char* pkMode)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return File(nullptr);
    }

    return SD.open(rkFilename.c_str(), pkMode, true);
}

bool Storage::Remove(const std::string& rkFilename)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }
    return SD.remove(rkFilename.c_str());
}

bool Storage::FileExists(const std::string& rkFilename)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }
    return SD.exists(rkFilename.c_str());
}

void Storage::GetContent(const std::string& rkFilename,
                       const char*          pkDefaultContent,
                       std::string&         rContent,
                       const bool           kCacheable)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        rContent = "ERROR";
        return;
    }

    /* Check cache */
    if(kCacheable && cache_.count(rkFilename) != 0)
    {
        LOG_DEBUG("Read file %s from cache\n", rkFilename.c_str());
        rContent = cache_[rkFilename];
        return;
    }

    if(SD.exists(rkFilename.c_str()))
    {
        file = SD.open(rkFilename.c_str(), FILE_READ);
        if(file)
        {
            /* Read data */
            rContent = "";
            while(file.available())
            {
                rContent += file.readString().c_str();
            }

            file.close();

            LOG_DEBUG("Read file %s\n", rkFilename.c_str());
        }
        else
        {
            LOG_ERROR("Failed to open file %s\n", rkFilename.c_str());
            rContent = "ERROR";
        }

        if(kCacheable)
        {
            cache_[rkFilename] = rContent;
        }
    }
    else
    {
        rContent = pkDefaultContent;
        SetContent(rkFilename, pkDefaultContent, kCacheable);
    }
}

bool Storage::SetContent(const std::string& rkFilename,
                         const std::string& rkContent,
                         const bool         kCacheable)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");

        return false;
    }

    /* First we remove the file */
    if(SD.exists(rkFilename.c_str()))
    {
        if(!SD.remove(rkFilename.c_str()))
        {
            LOG_ERROR("Failed to remove file %s\n", rkFilename.c_str());
            return false;
        }
    }

    /* Create file and write */
    file = SD.open(rkFilename.c_str(), FILE_WRITE);
    if(file)
    {
        file.print(rkContent.c_str());
        file.close();

        if(kCacheable)
        {
            cache_[rkFilename] = rkContent;
        }

        LOG_DEBUG("Wrote file %s\n", rkFilename.c_str());
    }
    else
    {
        LOG_ERROR("Failed to open file %s\n", rkFilename.c_str());
        return false;
    }

    return true;
}

void Storage::Format(void)
{
    LOG_DEBUG("Format requested\n");
    RemoveDirectory("/", "/");
}

Storage::Storage(void)
{
    init_ = SD.begin(GPIO_SD_CS, GENERAL_SPI);

    if(init_)
    {
        if(SD.cardType() != CARD_NONE)
        {
            init_ = true;
            LOG_DEBUG(
                "SD card detected: %d (%lluB)\n",
                SD.cardType(),
                SD.cardSize()
            );
        }
        else
        {
            init_ = false;
            LOG_ERROR("No SD card detected\n");
        }
    }
}

bool Storage::RemoveDirectory(const std::string& rkDirName,
                              const std::string& rkRootDir)
{
    File        file;
    File        root;
    std::string filename;

    if(!init_)
    {
        LOG_ERROR("Could remove directory, SD card not initialized\n");
        return false;
    }

    LOG_DEBUG("Opening directory %s\n", rkDirName.c_str());

    root = SD.open(rkDirName.c_str());
    if(!root)
    {
        LOG_ERROR("Could not open file %s\n", rkDirName.c_str());
        return false;
    }
    if(!root.isDirectory())
    {
        LOG_ERROR("Could not open %s: Not a directory\n", rkDirName.c_str());
        return false;
    }

    /* List the files to ermove */
    file = root.openNextFile();
    while(file)
    {
        filename = file.path();

        if(!file.isDirectory())
        {
            file.close();
            LOG_DEBUG("Removing file %s\n", filename.c_str());
            SD.remove(filename.c_str());
        }
        else
        {
            file.close();
            RemoveDirectory(filename, filename);
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();

    if(rkDirName != rkRootDir)
    {
        LOG_DEBUG("Removing directory %s\n", rkDirName.c_str());
        SD.rmdir(rkDirName.c_str());
    }

    return true;
}
