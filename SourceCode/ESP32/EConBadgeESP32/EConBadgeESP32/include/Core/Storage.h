/*******************************************************************************
 * @file Storage.h
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

#ifndef __CORE_STORAGE_H_
#define __CORE_STORAGE_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <SD.h> /* SD Card driver */
#include <cstdint> /* Generic Int types */
#include <map> /* std::map */
#include <vector> /* std::vector */
#include <LEDBorder.h> /* Led Border types */
#include <Logger.h>    /* Logger service */

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

/* None*/

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

class Storage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        static Storage* GetInstance(void);

        ~Storage(void);

        sdcard_type_t GetSdCardType(void) const;
        uint64_t      GetSdCardSize(void) const;

        bool GetFileLoggingState(void);
        void LogToSdCard(const char * string, const ELogLevel level);

        void GetOwner(std::string& str);
        bool SetOwner(const std::string& str);
        void GetContact(std::string& str);
        bool SetContact(const std::string& str);
        void GetBluetoothName(std::string& str);
        bool SetBluetoothName(const std::string& str);
        void GetBluetoothPin(std::string& str);
        bool SetBluetoothPin(const std::string& str);

        bool CreateImage(const char * filename);
        bool RemoveImage(const char * filename);
        bool SaveImagePart(const char * filename,
                           const uint8_t * buffer,
                           const size_t size);
        bool ReadImagePart(const char * filename,
                           const size_t offset,
                           uint8_t * buffer,
                           size_t& size);
        bool SetCurrentImageName(const char* imageName);
        void GetCurrentImageName(std::string& imageName);
        void GetImageList(std::vector<std::string>* list);

        bool SaveLEDBorderEnabled(const bool enabled);
        bool SaveLEDBorderBrightness(const uint8_t brightness);
        bool SaveLEDBorderPattern(const ColorPattern * pattern);
        bool SaveLEDBorderAnimation(const IColorAnimation* anim,
                                    const uint8_t index);

        bool RemoveLEDBorderAnimation(const uint8_t index);
        bool RemoveLEDBorderAnimations(void);

        bool LoadLEDBorderSettings(bool& enabled,
                                   uint8_t& brightness,
                                   ColorPattern ** pattern,
                                   std::vector<IColorAnimation*>& animations);

        void Format(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        Storage(void);
        void Init(void);

        void GetContent(const char* fileName,
                        const char* defaultContent,
                        std::string& content,
                        bool cacheable);
        bool SetContent(const char* fileName,
                        const std::string& content,
                        bool cacheable);

        bool LoadLEDBorderEnabled(bool& enabled);
        bool LoadLEDBorderBrightness(uint8_t& brightness);
        bool LoadLEDBorderPattern(ColorPattern ** pattern);
        bool LoadLEDBorderAnimations(std::vector<IColorAnimation*>& animations);

        void RemoveDirectory(const char* dirName, const char* initDir);

        static Storage * instance_;

        bool          init_ = false;
        bool          faulty_ = false;

        sdcard_type_t sdType_;
        uint64_t      storageSize_;

        std::map<std::string, std::string> cache_;
};

#endif /* #ifndef __CORE_STORAGE_H_ */