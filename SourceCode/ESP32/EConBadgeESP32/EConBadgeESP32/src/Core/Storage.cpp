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
#include <SD.h> /* SD Card driver */
#include <cstdint> /* Generic Int types */
#include <Logger.h> /* Logger service */
#include <map> /* std::map */
#include <vector> /* std::vector */
#include <algorithm> /* Sort vector */
#include <LEDBorder.h> /* Led Border types */
#include <HWLayer.h> /* HWLayer service */

/* Header File */
#include <Storage.h>

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CSTOR Storage

#define CS_PIN 15

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define OWNER_FILE_PATH            "/owner"
#define CONTACT_FILE_PATH          "/contact"
#define BLUETOOTH_NAME_FILE_PATH   "/btname"
#define BLUETOOTH_PIN_FILE_PATH    "/btpin"
#define CURRENT_IMG_NAME_FILE_PATH "/currimg"

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
Storage* CSTOR::instance_ = nullptr;

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

CSTOR::~Storage(void)
{

}

Storage* CSTOR::GetInstance(void)
{
    if (CSTOR::instance_ == nullptr)
    {
        CSTOR::instance_ = new Storage();
    }

    if(!CSTOR::instance_->init_)
    {
        CSTOR::instance_->Init();
    }

    return CSTOR::instance_;
}

void CSTOR::GetOwner(std::string& str)
{
    GetContent(OWNER_FILE_PATH, DEFAULT_OWNER, str, true);
}

bool CSTOR::SetOwner(const std::string& str)
{
    return SetContent(OWNER_FILE_PATH, str, true);
}

void CSTOR::GetContact(std::string& str)
{
    GetContent(CONTACT_FILE_PATH, DEFAULT_CONTACT, str, true);
}

bool CSTOR::SetContact(const std::string& str)
{
    return SetContent(CONTACT_FILE_PATH, str, true);
}

void CSTOR::GetBluetoothName(std::string& str)
{
    GetContent(BLUETOOTH_NAME_FILE_PATH, HWManager::GetHWUID(), str, true);
}

bool CSTOR::SetBluetoothName(const std::string& str)
{
    return SetContent(BLUETOOTH_NAME_FILE_PATH, str, true);
}

void CSTOR::GetBluetoothPin(std::string& str)
{
    GetContent(BLUETOOTH_PIN_FILE_PATH, DEFAULT_BT_PIN, str, true);
}

bool CSTOR::SetBluetoothPin(const std::string& str)
{
    return SetContent(BLUETOOTH_PIN_FILE_PATH, str, true);
}

bool CSTOR::CreateImage(const char * filename)
{
    File file;
    std::string filePath = std::string(IMAGE_DIR_PATH "/") +
                           std::string(filename);

    if(!init_)
    {
        LOG_ERROR("Could not create %s file, SD card not initialized\n",
                  filePath.c_str());
        return false;
    }

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Tried to create an empty-named image.\n");
        return false;
    }

    /* Delete old image if exist */
    if(SD.exists(filePath.c_str()))
    {
        if(!SD.remove(filePath.c_str()))
        {
            LOG_ERROR("Could not remove %s file\n", filePath.c_str());
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
        LOG_ERROR("Could not create %s file\n", filePath.c_str());
        return false;
    }
}

bool CSTOR::RemoveImage(const char * filename)
{
    File file;
    std::string filePath = std::string(IMAGE_DIR_PATH "/") +
                           std::string(filename);

    if(!init_)
    {
        LOG_ERROR("Could not remove %s file, SD card not initialized\n",
                  filePath.c_str());
        return false;
    }

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Tried to remove an empty-named image.\n");
        return false;
    }

    /* Delete old state */
    if(SD.exists(filePath.c_str()))
    {
        if(!SD.remove(filePath.c_str()))
        {
            LOG_ERROR("Could not remove %s file\n", filePath.c_str());
            return false;
        }
    }

    return true;
}

bool CSTOR::SaveImagePart(const char * filename,
                          const uint8_t * buffer,
                          const size_t size)
{
    File file;
    std::string filePath = std::string(IMAGE_DIR_PATH "/") +
                           std::string(filename);

    if(!init_)
    {
        LOG_ERROR("Could not update %s file, SD card not initialized\n",
                  filePath.c_str());
        return false;
    }

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Tried to save an empty-named image.\n");
        return false;
    }

    /* Delete old state */
    if(!SD.exists(filePath.c_str()))
    {
        LOG_ERROR("Could not update %s file, file not found\n",
                  filePath.c_str());
    }

    file = SD.open(filePath.c_str(), FILE_WRITE);
    if(file)
    {
        if(file.write(buffer, size) != size)
        {
            LOG_ERROR("Error while updating image %s\n", filePath.c_str());
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

bool CSTOR::ReadImagePart(const char * filename,
                          const size_t offset,
                          uint8_t * buffer,
                          size_t& size)
{
    File file;
    std::string filePath = std::string(IMAGE_DIR_PATH "/") +
                           std::string(filename);

    if(!init_)
    {
        LOG_ERROR("Could not read %s file, SD card not initialized\n",
                  filePath.c_str());
        return false;
    }

    if(strlen(filename) == 0)
    {
        LOG_ERROR("Tried to read an empty-named image.\n");
        return false;
    }

    /* Delete old state */
    if(!SD.exists(filePath.c_str()))
    {
        LOG_ERROR("Could not read %s file, file not found\n",
                  filePath.c_str());
    }

    file = SD.open(filePath.c_str(), FILE_READ);
    if(file)
    {
        if(!file.seek(offset))
        {
            LOG_ERROR("Could not seek %d to file %s\n", offset, filename);
        }

        if(file.readBytes((char*)buffer, size) == 0)
        {
            LOG_ERROR("Eror while reading image %s\n", filePath.c_str());
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

bool CSTOR::SetCurrentImageName(const char* imageName)
{
    return SetContent(CURRENT_IMG_NAME_FILE_PATH, std::string(imageName), true);
}

void CSTOR::GetCurrentImageName(std::string& imageName)
{
    GetContent(CURRENT_IMG_NAME_FILE_PATH, "", imageName, true);
}

void CSTOR::GetImageList(std::vector<std::string>* list)
{
    File    file;
    File    root;
    uint8_t nameInt;
    bool    status;

    status = true;

    list->clear();

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(IMAGE_DIR_PATH);
    if(!root)
    {
        LOG_ERROR("Could not open %s\n", IMAGE_DIR_PATH);
        return;
    }
    if(!root.isDirectory())
    {
        LOG_ERROR("Could not open %s: Not a directory\n",
                  IMAGE_DIR_PATH);
        return;
    }

    /* List the files to rename */
    file = root.openNextFile();
    while(file)
    {
        if(!file.isDirectory())
        {
            list->push_back(file.name());
        }
        file.close();
        file = root.openNextFile();
    }
}

bool CSTOR::SaveLEDBorderEnabled(const bool enabled)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n",
                  LEDBORDER_ENABLED_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_ENABLED_FILE_PATH))
    {
        if(!SD.remove(LEDBORDER_ENABLED_FILE_PATH))
        {
            LOG_ERROR("Could not remove %s file\n", LEDBORDER_ENABLED_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_ENABLED_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the state */
        if(file.write(enabled) != 1)
        {
            LOG_ERROR("Could not save LEDBorder state\n");
            if(!SD.remove(LEDBORDER_ENABLED_FILE_PATH))
            {
                LOG_ERROR("Could not remove %s file\n", LEDBORDER_ENABLED_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border state\n");
        return true;
    }
    else
    {
        LOG_ERROR("Could not open %s file\n", LEDBORDER_ENABLED_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderBrightness(const uint8_t brightness)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n",
                  LEDBORDER_BRIGHTNESS_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_BRIGHTNESS_FILE_PATH))
    {
        if(!SD.remove(LEDBORDER_BRIGHTNESS_FILE_PATH))
        {
            LOG_ERROR("Could not remove %s file\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_BRIGHTNESS_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the brightness */
        if(file.write(brightness) != 1)
        {
            LOG_ERROR("Could not save LEDBorder brightness\n");
            if(!SD.remove(LEDBORDER_BRIGHTNESS_FILE_PATH))
            {
                LOG_ERROR("Could not remove %s file\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border brightness\n");
        return true;
    }
    else
    {
        LOG_ERROR("Could not open %s file\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderPattern(const ColorPattern * pattern)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n",
                  LEDBORDER_PATTERN_FILE_PATH);
        return false;
    }

    /* Delete old state */
    if(SD.exists(LEDBORDER_PATTERN_FILE_PATH))
    {
        if(!SD.remove(LEDBORDER_PATTERN_FILE_PATH))
        {
            LOG_ERROR("Could not remove %s file\n", LEDBORDER_PATTERN_FILE_PATH);
            return false;
        }
    }

    file = SD.open(LEDBORDER_PATTERN_FILE_PATH, FILE_WRITE);
    if(file)
    {
        /* Save the pattern */
        if(file.print(*pattern) <= 0)
        {
            LOG_ERROR("Could not save LEDBorder pattern\n");
            if(!SD.remove(LEDBORDER_PATTERN_FILE_PATH))
            {
                LOG_ERROR("Could not remove %s file\n", LEDBORDER_PATTERN_FILE_PATH);
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border pattern\n");
        return true;
    }
    else
    {
        LOG_ERROR("Could not open %s file\n", LEDBORDER_PATTERN_FILE_PATH);
        return false;
    }
}

bool CSTOR::SaveLEDBorderAnimation(const IColorAnimation* anim,
                                   const uint8_t index)
{
    File    file;
    uint8_t i;

    std::string filename;

    filename = LEDBORDER_ANIM_DIR_PATH + std::string("/") + std::to_string(index);

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n",
                  filename.c_str());
        return false;
    }

    /* Delete old state */
    if(SD.exists(filename.c_str()))
    {
        if(!SD.remove(filename.c_str()))
        {
            LOG_ERROR("Could not remove %s file\n", filename.c_str());
            return false;
        }
    }

    file = SD.open(filename.c_str(), FILE_WRITE);
    if(file)
    {
        if(file.print(*anim) <= 0)
        {
            LOG_ERROR("Could not save LEDBorder animation\n");
            if(!SD.remove(filename.c_str()))
            {
                LOG_ERROR("Could not remove %s file\n", filename.c_str());
            }
            return false;
        }

        file.close();

        LOG_DEBUG("Saved LED Border animation\n");
        return true;
    }
    else
    {
        LOG_ERROR("Could not open %s file\n", filename.c_str());
        return false;
    }
}

bool CSTOR::RemoveLEDBorderAnimation(const uint8_t index)
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

    filename = LEDBORDER_ANIM_DIR_PATH + std::string("/") + std::to_string(index);

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n",
                  filename.c_str());
        return false;
    }

    /* Delete old state */
    if(!SD.exists(filename.c_str()))
    {
        LOG_ERROR("Could not remove %s file: File not found\n", filename.c_str());
        return false;
    }

    if(!SD.remove(filename.c_str()))
    {
        LOG_ERROR("Could not remove %s file\n", filename.c_str());
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
    if(!root.isDirectory())
    {
        LOG_ERROR("Could not open %s: Not a directory\n",
                  LEDBORDER_ANIM_DIR_PATH);
        return false;
    }

    /* List the files to rename */
    file = root.openNextFile();
    while(file)
    {
        if(!file.isDirectory())
        {
            try
            {
                /* Only when the animation index is bigger */
                nameInt = std::stoi(file.name());
                if(nameInt > index)
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

    /* Sort the vector */
    std::sort(anims.begin(), anims.end());

    /* Rename all files */
    for(i = 0; i < anims.size(); ++i)
    {
        nameInt = anims[i];
        filename = LEDBORDER_ANIM_DIR_PATH +
                   std::string("/") + std::to_string(nameInt);
        newName  = LEDBORDER_ANIM_DIR_PATH +
                   std::string("/") + std::to_string(nameInt - 1);

        if(!SD.rename(filename.c_str(), newName.c_str()))
        {
            LOG_ERROR("Could not rename file %s to %s\n",
                      filename.c_str(),
                      newName.c_str());
            status = false;
        }
    }

    return status;
}

bool CSTOR::RemoveLEDBorderAnimations(void)
{
    RemoveDirectory(LEDBORDER_ANIM_DIR_PATH, LEDBORDER_ANIM_DIR_PATH);
    return true;
}

bool CSTOR::LoadLEDBorderSettings(bool& enabled,
                                  uint8_t& brightness,
                                  ColorPattern ** pattern,
                                  std::vector<IColorAnimation*>& animations)
{
    if(pattern == nullptr)
    {
        LOG_ERROR("NULL Pattern pointer\n");
        return false;
    }

    if(!init_)
    {
        LOG_ERROR("Could not open configuration, SD card not initialized\n");
        return false;
    }

    if(!LoadLEDBorderEnabled(enabled))
    {
        return false;
    }
    if(!LoadLEDBorderBrightness(brightness))
    {
        return false;
    }
    if(!LoadLEDBorderPattern(pattern))
    {
        return false;
    }
    if(!LoadLEDBorderAnimations(animations))
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

}

void CSTOR::Init(void)
{
    init_ = SD.begin(CS_PIN, GENERAL_SPI);
    if(init_)
    {
        sdType_ = SD.cardType();
        if(sdType_ != sdcard_type_t::CARD_NONE)
        {
            storageSize_ = SD.cardSize();

            SD.mkdir(LEDBORDER_DIR_PATH);
            SD.mkdir(LEDBORDER_ANIM_DIR_PATH);
            SD.mkdir(IMAGE_DIR_PATH);

            init_        = true;
            LOG_DEBUG("SD card detected: %d (%lluB)\n", sdType_, storageSize_);
        }
        else
        {
            init_ = false;
            LOG_ERROR("No SD card detected\n");
        }
    }
    else
    {
        LOG_ERROR("Could not initalize SD card\n");
    }
}

void CSTOR::GetContent(const char* fileName,
                       const char* defaultContent,
                       std::string& content,
                       bool cacheable)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n", fileName);
        content = "ERROR";
        return;
    }

    /* Check cache */
    if(cacheable && cache_.count(fileName) != 0)
    {
        content = cache_[fileName];
        LOG_DEBUG("Read file %s from cache\n", fileName);
        return;
    }

    if(SD.exists(fileName))
    {
        file = SD.open(fileName, FILE_READ);
        if(file)
        {
            /* Read owner */
            content = "";
            while(file.available())
            {
                content += file.readString().c_str();
            }

            file.close();

            LOG_DEBUG("Read file %s\n", fileName);
        }
        else
        {
            LOG_ERROR("Could not open %s file\n", fileName);
            content = "ERROR";
        }

        if(cacheable)
        {
            cache_[fileName] = content;
        }
    }
    else
    {
        content = defaultContent;
        SetContent(fileName, defaultContent, cacheable);
    }
}

bool CSTOR::SetContent(const char* fileName,
                       const std::string& content,
                       bool cacheable)
{
    File file;

    if(!init_)
    {
        LOG_ERROR("Could not open %s file, SD card not initialized\n", fileName);
        return false;
    }

    /* First we remove the file */
    if(SD.exists(fileName))
    {
        if(!SD.remove(fileName))
        {
            LOG_ERROR("Could not remove %s file\n", fileName);
            return false;
        }
    }

    /* Create file and write */
    file = SD.open(fileName, FILE_WRITE);
    if(file)
    {
        file.print(content.c_str());
        file.close();

        if(cacheable)
        {
            cache_[fileName] = content;
        }

        LOG_DEBUG("Wrote file %s\n", fileName);
    }
    else
    {
        LOG_ERROR("Could not open %s file\n", fileName);
        return false;
    }

    return true;
}

bool CSTOR::LoadLEDBorderEnabled(bool& enabled)
{
    File    file;
    uint8_t buffer;
    size_t  readCount;

    if(SD.exists(LEDBORDER_ENABLED_FILE_PATH))
    {
        file = SD.open(LEDBORDER_ENABLED_FILE_PATH, FILE_READ);
        if(file)
        {
            /* Load the state */
            readCount = file.readBytes((char*)&buffer, sizeof(buffer));
            if(readCount != sizeof(buffer))
            {
                LOG_ERROR("Could not load LED Border enable state\n");
                return false;
            }

            enabled = buffer;
            return true;
        }
        else
        {
            LOG_ERROR("Could not open %s file\n", LEDBORDER_ENABLED_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderBrightness(uint8_t& brightness)
{
    File    file;
    uint8_t buffer;
    size_t  readCount;

    if(SD.exists(LEDBORDER_BRIGHTNESS_FILE_PATH))
    {
        file = SD.open(LEDBORDER_BRIGHTNESS_FILE_PATH, FILE_READ);
        if(file)
        {
            /* Load the brightness */
            readCount = file.readBytes((char*)&buffer, sizeof(buffer));
            if(readCount != sizeof(buffer))
            {
                LOG_ERROR("Could not load LED Border brightness\n");
                return false;
            }

            brightness = buffer;
            return true;
        }
        else
        {
            LOG_ERROR("Could not open %s file\n", LEDBORDER_BRIGHTNESS_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderPattern(ColorPattern ** pattern)
{
    File file;

    if(SD.exists(LEDBORDER_PATTERN_FILE_PATH))
    {
        file = SD.open(LEDBORDER_PATTERN_FILE_PATH, FILE_READ);
        if(file)
        {
            *pattern = LedBorderBuilder::DeserializePattern(file);
            if(*pattern == nullptr)
            {
                LOG_ERROR("Could not load LEDBorder pattern\n");
                return false;
            }
            return true;
        }
        else
        {
            LOG_ERROR("Could not open %s file\n", LEDBORDER_PATTERN_FILE_PATH);
            return false;
        }
    }
    else
    {
        LOG_DEBUG("No LED Border state file available\n");
        return false;
    }
}

bool CSTOR::LoadLEDBorderAnimations(std::vector<IColorAnimation*>& animations)
{
    File    file;
    File    root;
    uint8_t nameInt;
    bool    status;

    std::string filename;

    IColorAnimation * newAnim;

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
        LOG_ERROR("Could not open %s: Not a directory\n",
                  LEDBORDER_ANIM_DIR_PATH);
        return false;
    }

    /* List the files to rename */
    file = root.openNextFile();
    while(file)
    {
        if(!file.isDirectory())
        {
            try
            {
                /* Only when the animation index is less than 255, also filters
                 * potential non annimation files (not a number)
                 */
                nameInt = std::stoi(file.name());
                if(nameInt < 255)
                {
                    newAnim = LedBorderBuilder::DeserializeAnimation(file);
                    if(newAnim == nullptr)
                    {
                        LOG_ERROR("Could not load LEDBorder animation\n");
                        status = false;
                    }
                    else
                    {
                        animations.push_back(newAnim);
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

    return status;
}

void CSTOR::RemoveDirectory(const char* dirName, const char* initDir)
{
    File    file;
    File    root;
    uint8_t nameInt;
    bool    status;

    std::string filename;

    IColorAnimation * newAnim;

    status = true;

    LOG_DEBUG("Opening directory %s\n", dirName);

    /* Rename files that are bigger, effectively overwriting the animation and
     * setting the corret order
     */
    root = SD.open(dirName);
    if(!root)
    {
        LOG_ERROR("Could not open %s\n", dirName);
        return;
    }
    if(!root.isDirectory())
    {
        LOG_ERROR("Could not open %s: Not a directory\n",
                  dirName);
        return;
    }

    /* List the files to rename */
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
            RemoveDirectory(filename.c_str(), filename.c_str());
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
    if(strcmp(dirName, initDir) != 0)
    {
        LOG_DEBUG("Removing directory %s\n", dirName);
        SD.rmdir(dirName);
    }
}

#undef CSTOR
