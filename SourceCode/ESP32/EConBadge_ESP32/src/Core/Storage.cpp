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
#include <vector>   /* std::vector */
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
    File file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return File(nullptr);
    }

    file = SD.open(rkFilename.c_str(), pkMode, true);
    if(file)
    {
        /* Remove from file list */
        fileLists_.clear();
    }
    return file;
}

bool Storage::Remove(const std::string& rkFilename)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }

    file = SD.open(rkFilename.c_str(), FILE_READ, false);
    if(!file || file.isDirectory())
    {
        LOG_ERROR("Cannot remove file %s\n", rkFilename.c_str());
        return false;
    }

    if(SD.remove(rkFilename.c_str()))
    {
        /* Remove from file list */
        fileLists_.clear();

        return true;
    }
    else
    {
        return false;
    }
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
                         const char*        pkDefaultContent,
                         std::string&       rContent,
                         const bool         kCacheable)
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
    /* Remove from file list */
    fileLists_.clear();
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

    std::map<std::string, std::vector<std::string>>::iterator it;

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

    /* List the files to remove */
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

    /* Remove from file list */
    it = fileLists_.find(rkDirName);
    if(fileLists_.find(rkDirName) != fileLists_.end())
    {
        fileLists_.erase(it);
    }

    return true;
}


void Storage::GetFilesListFrom(const std::string&        krDirectory,
                               std::vector<std::string>& rList,
                               const std::string&        rkStartName,
                               const size_t              kPrev,
                               const size_t              kCount)
{
    File                     file;
    File                     root;
    size_t                   i;
    size_t                   idxSinceFound;
    size_t                   searchSize;
    std::vector<std::string> files;
    std::pair<std::map<std::string, std::vector<std::string>>::iterator, bool> it;

    rList.clear();

    if(!init_)
    {
        LOG_ERROR("Failed to get image list. SD card not initialized\n");
        return;
    }

    /* Start caching if needed */
    it.first = fileLists_.find(krDirectory);
    if(it.first == fileLists_.end())
    {
        root = SD.open(krDirectory.c_str());
        if(!root)
        {
            LOG_ERROR("Failed to open %s\n", krDirectory.c_str());
            return;
        }
        if(!root.isDirectory())
        {
            LOG_ERROR("Failed to open %s. Not a directory\n", krDirectory.c_str());
            return;
        }

        /* List the files */
        file = root.openNextFile();
        while(file)
        {
            if(!file.isDirectory())
            {
                files.push_back(file.name());
            }
            file.close();
            file = root.openNextFile();
        }

        it = fileLists_.emplace(krDirectory, files);
    }
    else
    {
        it.second = true;
    }

    /* Search in the list */
    if(!it.second)
    {
        LOG_ERROR("Failed to load files list for %s.\n", krDirectory.c_str());
        return;
    }

    /* Find the file */
    searchSize = it.first->second.size();
    if(searchSize == 0)
    {
        return;
    }

    i = 0;
    for(i = 0; i < it.first->second.size(); ++i)
    {
        if(it.first->second[i] == rkStartName || rkStartName.size() == 0)
        {
            break;
        }
    }

    /* Get the correct amount of items */
    if(kPrev > i)
    {
        idxSinceFound = searchSize - (kPrev - i);
    }
    else
    {
        idxSinceFound = i - kPrev;
    }

    for(i = 0; i < kCount && i < searchSize; ++i)
    {
        rList.push_back(it.first->second[idxSinceFound]);
        idxSinceFound = (idxSinceFound + 1) % searchSize;
    }
}
