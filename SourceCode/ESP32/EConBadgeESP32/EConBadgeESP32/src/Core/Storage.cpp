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
#include <map>         /* std::map */
#include <SD.h>        /* SD Card driver */
#include <vector>      /* std::vector */
#include <cstdint>     /* Generic Int types */
#include <Logger.h>    /* Logger service */
#include <HWLayer.h>   /* HWLayer service */
#include <algorithm>   /* Sort vector */
#include <LEDBorder.h> /* Led Border types */

/* Header File */
#include <Storage.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSTOR Storage

/** @brief SD card CS pin */
#define CS_PIN 15

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define OWNER_FILE_PATH            "/owner"
#define CONTACT_FILE_PATH          "/contact"
#define BLUETOOTH_NAME_FILE_PATH   "/btname"
#define BLUETOOTH_PIN_FILE_PATH    "/btpin"
#define CURRENT_IMG_NAME_FILE_PATH "/currimg"
#define LOG_DIR_PATH               "/logs"
#define LOG_FILE_STATE             "/logtofile"

#define LEDBORDER_DIR_PATH             "/ledborder"
#define LEDBORDER_ENABLED_FILE_PATH    LEDBORDER_DIR_PATH "/enabled"
#define LEDBORDER_BRIGHTNESS_FILE_PATH LEDBORDER_DIR_PATH "/brightness"
#define LEDBORDER_PATTERN_FILE_PATH    LEDBORDER_DIR_PATH "/pattern"
#define LEDBORDER_ANIM_DIR_PATH        LEDBORDER_DIR_PATH "/anim"

#define IMAGE_DIR_PATH "/images"

#define DEFAULT_OWNER   "Unknown"
#define DEFAULT_CONTACT "No Contact"
#define DEFAULT_BT_PIN  ""

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
Storage * CSTOR::PINSTANCE_ = nullptr;

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

Storage * CSTOR::GetInstance(void)
{
    if (CSTOR::PINSTANCE_ == nullptr)
    {
        CSTOR::PINSTANCE_ = new Storage();
    }

    if(!CSTOR::PINSTANCE_->init_)
    {
        CSTOR::PINSTANCE_->Init();
    }

    return CSTOR::PINSTANCE_;
}

sdcard_type_t CSTOR::GetSdCardType(void) const
{
    if(init_ == false)
    {
        return CARD_NONE;
    }

    return sdType_;
}

uint64_t CSTOR::GetSdCardSize(void) const
{
    if(init_ == false)
    {
        return 0;
    }

    return storageSize_;
}

bool CSTOR::GetFileLoggingState(void)
{
    if(init_ == false)
    {
        return false;
    }

    return SD.exists(LOG_FILE_STATE);
}

void CSTOR::LogToSdCard(const char * pkStr, const ELogLevel kLevel)
{

    File        file;
    size_t      size;
    std::string filePath = std::string(LOG_DIR_PATH "/");

    if(init_ == false)
    {
        return;
    }

    filePath = std::string(LOG_DIR_PATH "/");

    switch(kLevel)
    {
        case LOG_LEVEL_ERROR:
            filePath += "error.log";
            break;
        case LOG_LEVEL_INFO:
            filePath += "info.log";
            break;
        case LOG_LEVEL_DEBUG:
            filePath += "debug.log";
            break;
        default:
            return;
    }

    file = SD.open(filePath.c_str(), FILE_WRITE);
    if(file)
    {
        size = file.size();
        if((int)size == -1)
        {
            size = 0;
        }
        /* Seek the end of the file */
        if(file.seek(size))
        {
            file.write((uint8_t*)pkStr, strlen(pkStr));
        }
        file.close();
    }
}

void CSTOR::GetOwner(std::string & rStr)
{
    GetContent(OWNER_FILE_PATH, DEFAULT_OWNER, rStr, true);
}

bool CSTOR::SetOwner(const std::string & rkStr)
{
    return SetContent(OWNER_FILE_PATH, rkStr, true);
}

void CSTOR::GetContact(std::string & rStr)
{
    GetContent(CONTACT_FILE_PATH, DEFAULT_CONTACT, rStr, true);
}

bool CSTOR::SetContact(const std::string & rkStr)
{
    return SetContent(CONTACT_FILE_PATH, rkStr, true);
}

void CSTOR::GetBluetoothName(std::string & rStr)
{
    GetContent(BLUETOOTH_NAME_FILE_PATH, HWManager::GetHWUID(), rStr, true);
}

bool CSTOR::SetBluetoothName(const std::string & rkStr)
{
    return SetContent(BLUETOOTH_NAME_FILE_PATH, rkStr, true);
}

void CSTOR::GetBluetoothPin(std::string & rStr)
{
    GetContent(BLUETOOTH_PIN_FILE_PATH, DEFAULT_BT_PIN, rStr, true);
}

bool CSTOR::SetBluetoothPin(const std::string & rkStr)
{
    return SetContent(BLUETOOTH_PIN_FILE_PATH, rkStr, true);
}

bool CSTOR::CreateImage(const std::string & rkFilename)
{
    File file;
    std::string filePath;

    filePath = std::string(IMAGE_DIR_PATH "/") + rkFilename;

    if(init_ == false)
    {
        LOG_ERROR("Failed to create %s. Need initialization\n",
                  filePath.c_str());
        return false;
    }

    if(rkFilename.size() == 0)
    {
        LOG_ERROR("Tried to create an empty-named image\n");
        return false;
    }

    /* Delete old image if exist */
    if(SD.exists(filePath.c_str()) == true)
    {
        if(SD.remove(filePath.c_str()) == false)
        {
            LOG_ERROR("Failed to remove %s\n", filePath.c_str());
            return false;
        }
    }

    file = SD.open(filePath.c_str(), FILE_WRITE);
    if(file)
    {
        file.close();
        return true;
    }
    else
    {
        LOG_ERROR("Faield to create %s\n", filePath.c_str());
        return false;
    }
}

bool CSTOR::RemoveImage(const std::string & rkFilename)
{
    File        file;
    std::string filePath;

    filePath = std::string(IMAGE_DIR_PATH "/") + rkFilename;
    if(init_ == false)
    {
        LOG_ERROR("Failed to remove %s. Need initialization\n",
                  filePath.c_str());
        return false;
    }

    if(rkFilename.size() == 0)
    {
        LOG_ERROR("Tried to remove an empty-named image\n");
        return false;
    }

    /* Delete old state */
    if(SD.exists(filePath.c_str()) == true)
    {
        if(SD.remove(filePath.c_str()) == false)
        {
            LOG_ERROR("Failed to remove %s\n", filePath.c_str());
            return false;
        }
    }

    return true;
}

bool CSTOR::SaveImagePart(const std::string & rkFilename,
                          const uint8_t     * pkBuffer,
                          const size_t        kSize)
{
    File        file;
    size_t      sizeSeek;
    std::string filePath;

    filePath = std::string(IMAGE_DIR_PATH "/") + rkFilename;

    if(init_ == false)
    {
        LOG_ERROR("Failed to save %s. Need initialization\n",
                  filePath.c_str());
        return false;
    }

    if(rkFilename.size() == 0)
    {
        LOG_ERROR("Tried to save an empty-named image\n");
        return false;
    }

    /* Delete old state */
    if(SD.exists(filePath.c_str()) == false)
    {
        LOG_ERROR("Failed to save %s. File not found\n", filePath.c_str());
        return false;
    }

    file = SD.open(filePath.c_str(), FILE_WRITE);
    if(file)
    {
        /* Seek the end of the file */
        sizeSeek = file.size();
        if((int)sizeSeek == -1)
        {
            sizeSeek = 0;
        }
        if(file.seek(sizeSeek) == true)
        {
            if(file.write(pkBuffer, kSize) != kSize)
            {
                file.close();
                LOG_ERROR("Error while saving %s\n", filePath.c_str());
                return false;
            }
        }
        else
        {
            file.close();
            LOG_ERROR("Cannot seek end of %s\n", filePath.c_str());
            return false;
        }
        file.close();
        return true;
    }
    else
    {
        LOG_ERROR("Could not update %s file\n", filePath.c_str());
        return false;
    }
}

bool CSTOR::ReadImagePart(const std::string & rkFilename,
                          const size_t        kOffset,
                          uint8_t           * pBuffer,
                          size_t            & rSize)
{
    File        file;
    std::string filePath;

    filePath = std::string(IMAGE_DIR_PATH "/") + rkFilename;
    if(init_ == false)
    {
        LOG_ERROR("Failed to read %s. Need initialization\n",
                  filePath.c_str());
        return false;
    }

    if(rkFilename.size() == 0)
    {
        LOG_ERROR("Tried to read an empty-named image\n");
        return false;
    }

    /* Delete old state */
    if(SD.exists(filePath.c_str()) == false)
    {
        LOG_ERROR("Failed to read %s. File not found\n", filePath.c_str());
    }

    file = SD.open(filePath.c_str(), FILE_READ);
    if(file)
    {
        if(file.seek(kOffset) == false)
        {
            file.close();
            LOG_ERROR("failed to seek %d in file %s\n",
                      kOffset,
                      rkFilename.c_str());
            return false;
        }

        if(file.readBytes((char*)pBuffer, rSize) == 0)
        {
            file.close();
            LOG_ERROR("Eror while reading %s\n", filePath.c_str());
            return false;
        }
        file.close();
        return true;
    }
    else
    {
        LOG_ERROR("Could not read %s file\n", filePath.c_str());
        return false;
    }
}

bool CSTOR::SetCurrentImageName(const std::string & rkFilename)
{
    return SetContent(CURRENT_IMG_NAME_FILE_PATH, rkFilename, true);
}

void CSTOR::GetCurrentImageName(std::string & rFilename)
{
    GetContent(CURRENT_IMG_NAME_FILE_PATH, "", rFilename, true);
}

void CSTOR::GetImageListFrom(ImageList         & rList,
                             const std::string & rkStartName,
                             const size_t        kCount)
{
    File   file;
    File   root;
    size_t i;
    size_t idxSinceFound;
    size_t fileIdx;

    rList.clear();

    if(init_ == false)
    {
        LOG_ERROR("Failed to get image list. Need initialization\n");
        return;
    }

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(IMAGE_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Failed to open %s\n", IMAGE_DIR_PATH);
        return;
    }
    if(root.isDirectory() == false)
    {
        LOG_ERROR("Failed to open %s. Not a directory\n", IMAGE_DIR_PATH);
        return;
    }

    /* List the files */
    idxSinceFound = 0;
    file          = root.openNextFile();
    fileIdx       = 0;
    while(file)
    {
        if(file.isDirectory() == false)
        {
            if((file.name() == rkStartName || rkStartName.size() == 0) &&
               idxSinceFound == 0)
            {
                idxSinceFound = 1;
            }
            else if(idxSinceFound != 0)
            {
                ++idxSinceFound;
            }

            /* Fill the list before replacing items in it */
            if(rList.size() < kCount)
            {
                rList.push_back(std::make_pair(file.name(), fileIdx));
            }
            else
            {
                for(i = 1; i < kCount; ++i)
                {
                    rList[i - 1] = rList[i];
                }
                rList[kCount - 1] = std::make_pair(file.name(), fileIdx);
            }

            ++fileIdx;
        }
        file.close();

        /* If we read enough files */
        if(idxSinceFound == kCount)
        {
            break;
        }
        file = root.openNextFile();
    }
}

void CSTOR::GetImageList(ImageList     & rList,
                         const int32_t   kStartIdx,
                         const size_t    kCount)
{
    File        file;
    File        root;
    size_t      i;
    size_t      copiedNames;
    size_t      tmpIdx;
    std::string tmpName;

    rList.clear();

    if(init_ == false)
    {
        LOG_ERROR("Failed to get image list. Need initialization\n");
        return;
    }

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(IMAGE_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Faield to open %s\n", IMAGE_DIR_PATH);
        return;
    }
    if(root.isDirectory() == false)
    {
        LOG_ERROR("Failed to open %s. Not a directory\n", IMAGE_DIR_PATH);
        return;
    }

    /* List the files */
    copiedNames = 0;
    i           = 0;
    file        = root.openNextFile();
    while(file)
    {
        if(file.isDirectory() == false)
        {
            /* Check if we should start from the end */
            if(kStartIdx >= 0)
            {
                if(i >= kStartIdx)
                {
                    rList.push_back(std::make_pair(file.name(), i));
                    ++copiedNames;
                }
            }
            else
            {
                tmpName = file.name();
                tmpIdx = i;
            }
            ++i;
        }
        file.close();

        /* If we read enough files */
        if(copiedNames == kCount)
        {
            break;
        }
        file = root.openNextFile();

        /* If this was the last file, and we wanted to get it, copy */
        if(kStartIdx < 0 && !file)
        {
            rList.push_back(std::make_pair(tmpName, tmpIdx));
        }
    }
}

void CSTOR::GetImageList(uint8_t      * pBuffer,
                         size_t       & rBuffSize,
                         const size_t   kStartIdx,
                         const size_t   kCount)
{
    File    file;
    File    root;
    size_t  i;
    size_t  copiedNames;
    size_t  nameSize;

    rBuffSize = 0;

    if(init_ == false)
    {
        LOG_ERROR("Failed to get image list. Need initialization\n");
        return;
    }

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(IMAGE_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Failed to open %s\n", IMAGE_DIR_PATH);
        return;
    }
    if(root.isDirectory() == false)
    {
        LOG_ERROR("Failed to open %s. Not a directory\n", IMAGE_DIR_PATH);
        return;
    }

    /* List the files */
    copiedNames = 0;
    i           = 0;
    file        = root.openNextFile();
    while(file)
    {
        if(file.isDirectory() == false)
        {
            if(i >= kStartIdx)
            {
                nameSize = strlen(file.name());
                memcpy(pBuffer, file.name(), nameSize);
                pBuffer += nameSize;
                *pBuffer = 0;
                ++pBuffer;
                rBuffSize += nameSize + 1;
                ++copiedNames;
            }
            ++i;
        }
        file.close();

        /* If we read enough files */
        if(copiedNames == kCount)
        {
            break;
        }
        file = root.openNextFile();
    }
}

bool CSTOR::SaveLEDBorderEnabled(const bool kEnabled)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Failed to open %s. Need initialization\n",
                  LEDBORDER_ENABLED_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_ENABLED_FILE_PATH) == true)
    {
        if(SD.remove(LEDBORDER_ENABLED_FILE_PATH) == false)
        {
            LOG_ERROR("Failed to remove %s\n", LEDBORDER_ENABLED_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_ENABLED_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the state */
        if(file.write(kEnabled) != 1)
        {
            file.close();

            LOG_ERROR("Failed to save LEDBorder state\n");

            if(!SD.remove(LEDBORDER_ENABLED_FILE_PATH))
            {
                LOG_ERROR("Failed to remove %s\n", LEDBORDER_ENABLED_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border state\n");
        return true;
    }
    else
    {
        LOG_ERROR("Failed to open %s\n", LEDBORDER_ENABLED_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderBrightness(const uint8_t kBrightness)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Could not open %d. Need initialization\n",
                  LEDBORDER_BRIGHTNESS_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_BRIGHTNESS_FILE_PATH) == true)
    {
        if(SD.remove(LEDBORDER_BRIGHTNESS_FILE_PATH) == false)
        {
            LOG_ERROR("Failed to remove %s\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_BRIGHTNESS_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the brightness */
        if(file.write(kBrightness) != 1)
        {
            file.close();

            LOG_ERROR("Failed to save LEDBorder brightness\n");
            if(SD.remove(LEDBORDER_BRIGHTNESS_FILE_PATH) == false)
            {
                LOG_ERROR("Failed to remove %s\n",
                          LEDBORDER_BRIGHTNESS_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border brightness\n");
        return true;
    }
    else
    {
        LOG_ERROR("Failed to open %s\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderPattern(const ColorPattern * pkPattern)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Could not open %d. Need initialization\n",
                  LEDBORDER_PATTERN_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_PATTERN_FILE_PATH) == true)
    {
        if(SD.remove(LEDBORDER_PATTERN_FILE_PATH) == false)
        {
            LOG_ERROR("Failed to remove %s\n", LEDBORDER_PATTERN_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_PATTERN_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the pattern */
        if(file.print(*pkPattern) <= 0)
        {
            file.close();

            LOG_ERROR("Could not save LEDBorder pattern\n");
            if(SD.remove(LEDBORDER_PATTERN_FILE_PATH) == false)
            {
                LOG_ERROR("Failed to remove %s\n", LEDBORDER_PATTERN_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border pattern\n");
        return true;
    }
    else
    {
        LOG_ERROR("Failed to open %s\n", LEDBORDER_PATTERN_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderAnimation(const IColorAnimation * pkAnim,
                                   const uint8_t           kIndex)
{
    File        file;
    std::string filename;

    filename = LEDBORDER_ANIM_DIR_PATH +
               std::string("/") +
               std::to_string(kIndex);

    if(init_ == false)
    {
        LOG_ERROR("Could not open %d. Need initialization\n",
                  filename.c_str());
        return false;
    }

    /* Delete old state */
    if(SD.exists(filename.c_str()) == true)
    {
        if(SD.remove(filename.c_str()) == false)
        {
            LOG_ERROR("Failed to remove %s\n", filename.c_str());
            return false;
        }
    }

    file = SD.open(filename.c_str(), FILE_WRITE);
    if(file)
    {
        if(file.print(*pkAnim) <= 0)
        {
            file.close();

            LOG_ERROR("Could not save LEDBorder animation\n");
            if(SD.remove(filename.c_str()) == false)
            {
                LOG_ERROR("Failed to remove %s\n", filename.c_str());
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border animation\n");
        return true;
    }
    else
    {
        LOG_ERROR("Failed to open %s\n", filename.c_str());
        return false;
    }
}

bool CSTOR::RemoveLEDBorderAnimation(const uint8_t kIndex)
{
    File    file;
    File    root;
    uint8_t nameInt;
    uint8_t i;
    bool    status;

    std::vector<uint8_t> anims;
    std::string          filename;
    std::string          newName;

    status = true;

    filename = LEDBORDER_ANIM_DIR_PATH +
               std::string("/") +
               std::to_string(kIndex);

    if(init_ == false)
    {
        LOG_ERROR("Could not open %d. Need initialization\n",
                  filename.c_str());
        return false;
    }

    /* Delete old state */
    if(SD.exists(filename.c_str()) == false)
    {
        LOG_ERROR("Failed to remove %s: File not found\n", filename.c_str());
        return false;
    }

    if(SD.remove(filename.c_str()) == false)
    {
        LOG_ERROR("Failed to remove %s\n", filename.c_str());
        return false;
    }

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(LEDBORDER_ANIM_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Could not open %s\n", LEDBORDER_ANIM_DIR_PATH);
        return false;
    }
    if(root.isDirectory() == false)
    {
        root.close();

        LOG_ERROR("Could not open %s. Not a directory\n",
                  LEDBORDER_ANIM_DIR_PATH);
        return false;
    }

    /* List the files */
    file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory() == false)
        {
            try
            {
                /* Only when the animation index is bigger */
                nameInt = std::stoi(file.name());
                if(nameInt > kIndex)
                {
                    anims.push_back(nameInt);
                }
            }
            catch(std::exception& e)
            {
                (void)e;
            }
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();

    /* Sort the vector */
    std::sort(anims.begin(), anims.end());

    /* Rename all files */
    for(i = 0; i < anims.size(); ++i)
    {
        nameInt = anims[i];
        filename = LEDBORDER_ANIM_DIR_PATH +
                   std::string("/") +
                   std::to_string(nameInt);
        newName  = LEDBORDER_ANIM_DIR_PATH +
                   std::string("/") +
                   std::to_string(nameInt - 1);

        if(SD.rename(filename.c_str(), newName.c_str()) == false)
        {
            LOG_ERROR("Failed to rename %s to %s\n",
                      filename.c_str(),
                      newName.c_str());
            status = false;
        }
    }

    return status;
}

void CSTOR::RemoveLEDBorderAnimations(void)
{
    RemoveDirectory(LEDBORDER_ANIM_DIR_PATH, LEDBORDER_ANIM_DIR_PATH);
}

bool CSTOR::LoadLEDBorderSettings(bool                          &  rEnabled,
                                  uint8_t                       &  rBrightness,
                                  ColorPattern                  ** ppPattern,
                                  std::vector<IColorAnimation*> &  animations)
{
    if(init_ == false)
    {
        LOG_ERROR("Faield to open configuration. Need initialization\n");
        return false;
    }

    if(LoadLEDBorderEnabled(rEnabled) == false)
    {
        return false;
    }
    if(LoadLEDBorderBrightness(rBrightness) == false)
    {
        return false;
    }
    if(LoadLEDBorderPattern(ppPattern) == false)
    {
        return false;
    }
    if(LoadLEDBorderAnimations(animations) == false)
    {
        return false;
    }

    return true;
}

void CSTOR::Format(void)
{
    LOG_DEBUG("Format requested\n");
    RemoveDirectory("/", "/");
}

CSTOR::Storage(void)
{
    init_        = false;
    faulty_      = false;
    storageSize_ = 0;
    sdType_      = CARD_UNKNOWN;
}

void CSTOR::Init(void)
{
    if(faulty_ == false)
    {
        init_ = SD.begin(CS_PIN, GENERAL_SPI);
        if(init_ == true)
        {
            sdType_ = SD.cardType();
            if(sdType_ != sdcard_type_t::CARD_NONE)
            {
                storageSize_ = SD.cardSize();

                SD.mkdir(LEDBORDER_DIR_PATH);
                SD.mkdir(LEDBORDER_ANIM_DIR_PATH);
                SD.mkdir(IMAGE_DIR_PATH);
                SD.mkdir(LOG_DIR_PATH);

                init_ = true;
                LOG_DEBUG("SD card detected: %d (%lluB)\n", sdType_, storageSize_);
            }
            else
            {
                init_ = false;
                faulty_ = true;
                LOG_ERROR("No SD card detected\n");
            }
        }
        else
        {
            LOG_ERROR("Failed to initalize SD card\n");
            faulty_ = true;
        }
    }
}

void CSTOR::GetContent(const std::string & rkFilename,
                       const char        * pkDefaultContent,
                       std::string       & rContent,
                       const bool          kCacheable)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Could not open %s. Need initialization\n",
                  rkFilename.c_str());
        rContent = "ERROR";
        return;
    }

    /* Check cache */
    if(kCacheable == true && cache_.count(rkFilename) != 0)
    {
        rContent = cache_[rkFilename];
        LOG_DEBUG("Read file %s from cache\n", rkFilename.c_str());
        return;
    }

    if(SD.exists(rkFilename.c_str()) == true)
    {
        file = SD.open(rkFilename.c_str(), FILE_READ);
        if(file)
        {
            /* Read owner */
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
            LOG_ERROR("Failed to open %s\n", rkFilename.c_str());
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

bool CSTOR::SetContent(const std::string & rkFilename,
                       const std::string & rkContent,
                       const bool          kCacheable)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Could not open %d. Need initialization\n",
                  rkFilename.c_str());
        return false;
    }

    /* First we remove the file */
    if(SD.exists(rkFilename.c_str()) == true)
    {
        if(SD.remove(rkFilename.c_str()) == false)
        {
            LOG_ERROR("Failed to remove %s\n", rkFilename.c_str());
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
        LOG_ERROR("Failed to open %s\n", rkFilename.c_str());
        return false;
    }

    return true;
}

bool CSTOR::LoadLEDBorderEnabled(bool & rEnabled)
{
    File    file;
    uint8_t buffer;
    size_t  readCount;

    if(init_ == false)
    {
        LOG_ERROR("Failed to get LEDBorder state. Need initialization\n");
        return false;
    }

    if(SD.exists(LEDBORDER_ENABLED_FILE_PATH) == true)
    {
        file = SD.open(LEDBORDER_ENABLED_FILE_PATH, FILE_READ);
        if(file)
        {
            /* Load the state */
            readCount = file.readBytes((char*)&buffer, sizeof(buffer));
            if(readCount != sizeof(buffer))
            {
                file.close();
                LOG_ERROR("Could not load LEDBorder enable state\n");
                return false;
            }

            rEnabled = buffer;
            return true;
        }
        else
        {
            LOG_ERROR("Failed to open %s\n", LEDBORDER_ENABLED_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderBrightness(uint8_t & rBrightness)
{
    File    file;
    uint8_t buffer;
    size_t  readCount;

    if(init_ == false)
    {
        LOG_ERROR("Failed to get LEDBorder state. Need initialization\n");
        return false;
    }

    if(SD.exists(LEDBORDER_BRIGHTNESS_FILE_PATH) == true)
    {
        file = SD.open(LEDBORDER_BRIGHTNESS_FILE_PATH, FILE_READ);
        if(file)
        {
            /* Load the brightness */
            readCount = file.readBytes((char*)&buffer, sizeof(buffer));
            if(readCount != sizeof(buffer))
            {
                file.close();
                LOG_ERROR("Could not load LEDBorder brightness\n");
                return false;
            }

            rBrightness = buffer;
            return true;
        }
        else
        {
            LOG_ERROR("Failed to open %s\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderPattern(ColorPattern ** ppPattern)
{
    File file;

    if(init_ == false)
    {
        LOG_ERROR("Failed to get LEDBorder state. Need initialization\n");
        return false;
    }

    if(SD.exists(LEDBORDER_PATTERN_FILE_PATH) == true)
    {
        file = SD.open(LEDBORDER_PATTERN_FILE_PATH, FILE_READ);
        if(file)
        {
            *ppPattern = LedBorderBuilder::DeserializePattern(file);
            file.close();

            if(*ppPattern == nullptr)
            {
                LOG_ERROR("Could not load LEDBorder pattern\n");
                return false;
            }
            return true;
        }
        else
        {
            LOG_ERROR("Failed to open %s\n", LEDBORDER_PATTERN_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderAnimations(std::vector<IColorAnimation*> & rAnims)
{
    File    file;
    File    root;
    uint8_t nameInt;
    bool    status;

    std::string filename;

    IColorAnimation * pNewAnim;

    if(init_ == false)
    {
        LOG_ERROR("Failed to get LEDBorder state. Need initialization\n");
        return false;
    }

    status = true;

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(LEDBORDER_ANIM_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Could not open %s\n", LEDBORDER_ANIM_DIR_PATH);
        return false;
    }
    if(!root.isDirectory())
    {
        root.close();

        LOG_ERROR("Could not open %s: Not a directory\n",
                  LEDBORDER_ANIM_DIR_PATH);
        return false;
    }

    /* List the files */
    file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory() == false)
        {
            try
            {
                /* Only when the animation index is less than 255, also filters
                 * potential non annimation files (not a number)
                 */
                nameInt = std::stoi(file.name());
                if(nameInt < 255)
                {
                    pNewAnim = LedBorderBuilder::DeserializeAnimation(file);
                    if(pNewAnim == nullptr)
                    {
                        LOG_ERROR("Could not load LEDBorder animation\n");
                        status = false;
                    }
                    else
                    {
                        rAnims.push_back(pNewAnim);
                    }
                }
            }
            catch(std::exception& e)
            {
                (void)e;
            }
        }
        file.close();
        file = root.openNextFile();
    }

    root.close();

    return status;
}

void CSTOR::RemoveDirectory(const std::string & rkDirName,
                            const std::string & rkRootDir)
{
    File        file;
    File        root;
    std::string filename;

    if(init_ == false)
    {
        LOG_ERROR("Could remove directory, SD card not initialized\n");
        return;
    }

    LOG_DEBUG("Opening directory %s\n", rkDirName.c_str());

    root = SD.open(rkDirName.c_str());
    if(!root)
    {
        LOG_ERROR("Could not open %s\n", rkDirName.c_str());
        return;
    }
    if(root.isDirectory() == false)
    {
        LOG_ERROR("Could not open %s: Not a directory\n",
                  rkDirName.c_str());
        return;
    }

    /* List the files to ermove */
    file = root.openNextFile();
    while(file)
    {
        filename = file.path();

        if(file.isDirectory() == false)
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
}

#undef CSTOR
