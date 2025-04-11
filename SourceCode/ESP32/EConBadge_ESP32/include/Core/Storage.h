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
#include <map>      /* std::map */
#include <vector>   /* std::vector */
#include <SdFat.h>  /* SD Card driver */
#include <cstdint>  /* Generic Int types */
#include <Types.h>  /* ECB Types */
#include <Logger.h> /* Logger service */

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
 * @brief Storage class.
 *
 * @details The Storage class provides functionalities to use files on the SD
 * card present on the ECB. This is a singleton class.
 */
class Storage
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Get the Storage instance object.
         *
         * @details Get the Storage instance object. The singleton is created
         * on the first call.
         *
         * @return The function returns the Storage singleton.
         */
        static Storage* GetInstance(void);

        /**
         * @brief Get the Sd Card Type.
         *
         * @details Get the Sd Card Type.
         *
         * @return The SD Card type is returned.
         */
        uint8_t GetSdCardType(void);

        /**
         * @brief Get the SD card size in bytes.
         *
         * @details Get the SD card size in bytes.
         *
         * @return The SD card size in bytes is returned.
         */
        uint64_t GetSdCardSize(void) const;

        /**
         * @brief Creates a new directory.
         *
         * @details Creates a new directory is not already existant.
         *
         * @param[in] rkPath The path of the directory to create
         * @return true is returned on success, otherwise false.
         */
        bool CreateDirectory(const std::string& rkPath);

        /**
         * @brief Opens a file in the SD card.
         *
         * @details Opens a file in the SD card. If the file is opened for
         * writting and does not exists, it will be created.
         *
         * @param[in] rkFilename The path to the file to open.
         * @param[in] kOpenMode The open mode (FILE_READ or FILE_WRITE).
         *
         * @return The new file is returned.
         */
        FsFile Open(const std::string& rkFilename, const oflag_t kOpenMode);

        /**
         * @brief Removes a file from the SD card.
         *
         * @details Removes a file from the SD card. If the file does not exists
         * no action is taken.
         *
         * @param[in] rkFilename The path to the file to remove.
         *
         * @return true on success, false otherwise.
         */
        bool Remove(const std::string& rkFilename);

        /**
         * @brief Checks if a file exists.
         *
         * @details Checks if a file exists.
         *
         * @param[in] rkFilename The path to the file to check.
         *
         * @return true if the file exists, false otherwise.
         */
        bool FileExists(const std::string& rkFilename);

        /**
         * @brief Gets the content of a file.
         *
         * @details Gets the full content of a file and stores it in the string
         * buffer. If the file does not exists it will be created and the
         * default content stored. The function provides a caching function to
         * avoid mutiple reads to the SD card.
         *
         * @param[in] rkFilename The file to get the content from.
         * @param[in] pkDefaultContent The default content to store if the file
         * does not exists.
         * @param[out] rContent The buffer that receives the content of the
         * file.
         * @param[in] kCacheable Defines if the content shall be cached.
         */
        void GetContent(const std::string& rkFilename,
                        const char*        pkDefaultContent,
                        std::string&       rContent,
                        const bool         kCacheable);

        /**
         * @brief Sets the content of a file.
         *
         * @details Sets the full content of a file and stores it in the file,
         * If the file does not exists it will be created and the content
         * stored. The function provides a caching function to
         * avoid mutiple writes to the SD card.
         *
         * @param[in] rkFilename The file to write the content to.
         * @param[out] rkContent The buffer that contains the content to store
         * to the file.
         * @param[in] kCacheable Defines if the content shall be cached.
         */
        bool SetContent(const std::string& rkFilename,
                        const std::string& rkContent,
                        const bool         kCacheable);

        /**
         * @brief Formats the SD card.
         *
         * @details Formats the SD card. This will remove all files and
         * directories from the card.
         */
        void Format(void);

        void GetFilesListFrom(const std::string&        krDirectory,
                              std::vector<std::string>& rList,
                              const std::string&        rkStartName,
                              const size_t              kPrev,
                              const size_t              kCount);

        size_t GetFilesCount(const std::string& krDirectory);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /**
         * @brief Construct a new Storage object.
         *
         */
        Storage(void);

        /** @brief Stores the initialization state. */
        bool init_;

        /** @brief Cache map for cached content. */
        std::map<std::string, std::string> cache_;

        /** @brief Cache the file lists. */
        std::map<std::string, std::vector<std::string>> fileLists_;

        /** @brief Stores the singleton instance. */
        static Storage* PINSTANCE_;

        /** @brief Stores the SD card instance */
        SdFs sdCard_;

        /** @brief Stores the CID info */
        cid_t cid_;
        /** @brief Stores the CSD info */
        csd_t csd_;
        /** @brief Stores the SCR info */
        scr_t scr_;
        /** @brief Stores the OCR info */
        uint32_t ocr_;

        /** @brief Stores the SD card configuration */
        SdSpiConfig* pConfig_;
};

#endif /* #ifndef __CORE_STORAGE_H_ */