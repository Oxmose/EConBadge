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
#include <SdFat.h>  /* SD Card driver */
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

uint8_t Storage::GetSdCardType(void)
{
    const SdCard* pSdCard;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return -1;
    }

    pSdCard = sdCard_.card();

    return pSdCard->type();
}

uint64_t Storage::GetSdCardSize(void) const
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return 0;
    }

    return (uint64_t)csd_.capacity() * 512ULL;
}

bool Storage::CreateDirectory(const std::string& rkPath)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }

    if(sdCard_.exists(rkPath.c_str()))
    {
        return true;
    }
    return sdCard_.mkdir(rkPath.c_str());
}

FsFile Storage::Open(const std::string& rkFilename, const oflag_t kOpenMode)
{
    FsFile file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return file;
    }

    if(file.open(rkFilename.c_str(), kOpenMode))
    {
        /* Remove from file list */
        fileLists_.clear();
    }
    return file;
}

bool Storage::Remove(const std::string& rkFilename)
{
    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");
        return false;
    }

    if(!sdCard_.exists(rkFilename.c_str()))
    {
        return false;
    }

    if(sdCard_.remove(rkFilename.c_str()))
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
    return sdCard_.exists(rkFilename.c_str());
}

void Storage::GetContent(const std::string& rkFilename,
                         const char*        pkDefaultContent,
                         std::string&       rContent,
                         const bool         kCacheable)
{
    FsFile file;

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

    if(sdCard_.exists(rkFilename.c_str()))
    {
        if(file.open(rkFilename.c_str(), FILE_READ))
        {
            /* Read data */
            rContent = "";
            while(file.available())
            {
                rContent += std::string(file.readString().c_str());
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
    FsFile file;

    if(!init_)
    {
        LOG_ERROR("SD Card not initialized.\n");

        return false;
    }

    /* First we remove the file */
    if(sdCard_.exists(rkFilename.c_str()))
    {
        if(!sdCard_.remove(rkFilename.c_str()))
        {
            LOG_ERROR("Failed to remove file %s\n", rkFilename.c_str());
            return false;
        }
    }

    /* Create file and write */
    if(file.open(rkFilename.c_str(), FILE_WRITE))
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
    if(sdCard_.format())
    {
        /* Remove from file list */
        fileLists_.clear();
    }
}

void Storage::GetFilesListFrom(const std::string&        krDirectory,
                               std::vector<std::string>& rList,
                               const std::string&        rkStartName,
                               const size_t              kPrev,
                               const size_t              kCount)
{
    FsFile                   file;
    FsFile                   root;
    size_t                   i;
    size_t                   idxSinceFound;
    size_t                   searchSize;
    std::vector<std::string> files;
    char                     baseName[128];
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
        if(!root.open(krDirectory.c_str()))
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
                file.getName(baseName, 128);
                files.push_back(baseName);
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

size_t Storage::GetFilesCount(const std::string& krDirectory)
{
    size_t fileCount;
    FsFile file;
    FsFile root;

    if(!init_)
    {
        LOG_ERROR("Failed to get image list. SD card not initialized\n");
        return 0;
    }

    if(!root.open(krDirectory.c_str()))
    {
        LOG_ERROR("Failed to open %s\n", krDirectory.c_str());
        return 0;
    }
    if(!root.isDirectory())
    {
        LOG_ERROR("Failed to open %s. Not a directory\n", krDirectory.c_str());
        return 0;
    }

    fileCount = 0;

    /* List the files */
    file = root.openNextFile();
    while(file)
    {
        if(!file.isDirectory())
        {
            ++fileCount;
        }
        file.close();
        file = root.openNextFile();
    }

    return fileCount;
}

Storage::Storage(void)
{
    pConfig_ = new SdSpiConfig(
        (uint8_t)GPIO_SD_CS,
        DEDICATED_SPI,
        SD_SCK_MHZ(8),
        &GENERAL_SPI
    );
    init_ = sdCard_.begin(*pConfig_);
    if(init_)
    {
        /* Init information */
        if (!sdCard_.card()->readCID(&cid_) ||
            !sdCard_.card()->readCSD(&csd_) ||
            !sdCard_.card()->readOCR(&ocr_) ||
            !sdCard_.card()->readSCR(&scr_))
        {
            init_ = false;
            LOG_ERROR("Failed to get SD card information\n");

            return;
        }

        if(GetSdCardType() != (uint8_t)-1)
        {
            init_ = true;
            LOG_DEBUG(
                "SD card detected: %d (%lluB)\n",
                GetSdCardType(),
                GetSdCardSize()
            );

            /* Create the TMP directory */
            sdCard_.rmdir(TMP_DIR_PATH);
            CreateDirectory(TMP_DIR_PATH);
        }
        else
        {
            init_ = false;
            LOG_ERROR("No SD card detected\n");
        }
    }
    else
    {
        init_ = false;
        LOG_ERROR("Failed to init SD card\n");
        sdCard_.initErrorHalt(&Serial);
    }
}
