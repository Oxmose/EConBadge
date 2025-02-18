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
#include <SD.h>     /* SD Card driver */
#include <vector>   /* std::vector */
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
/* Forward declarations */
class ColorPattern;
class IColorAnimation;

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

        sdcard_type_t GetSdCardType(void) const;
        uint64_t GetSdCardSize(void) const;

        bool CreateDirectory(const std::string& rkPath);
        File Open(const std::string& rkFilename, const char* pkMode);
        bool Close(File& rFile);
        bool Remove(const std::string& rkFilename);
        bool FileExists(const std::string& rkFilename);

        void GetContent(const std::string& rkFilename,
                        const char*        pkDefaultContent,
                        std::string&       rContent,
                        const bool         kCacheable);
        bool SetContent(const std::string& rkFilename,
                        const std::string& rkContent,
                        const bool         kCacheable);

        /* TODO: Remove all that if possible */
        void GetOwner(std::string& rStr);
        bool SetOwner(const std::string& rkStr);
        void GetContact(std::string& rStr);
        bool SetContact(const std::string& rkStr);

        bool CreateImage(const std::string& rkFilename);
        bool RemoveImage(const std::string& rkFilename);
        bool SaveImagePart(const std::string& rkFilename,
                           const uint8_t*     pkBuffer,
                           const size_t       kSize);
        bool ReadImagePart(const std::string& rkFilename,
                           const size_t       kOffset,
                           uint8_t*           pBuffer,
                           size_t&            rSize);
        bool SetCurrentImageName(const std::string& rkFilename);
        void GetDisplayedImageName(std::string& rFilename);
        void GetImageListFrom(ImageList& rList,
                              const std::string& rkStartName,
                              const size_t       kCount);
        void GetImageList(ImageList&    rList,
                          const int32_t kStartIdx,
                          const size_t  kCount);
        void GetImageList(uint8_t*     pBuffer,
                          size_t&      rBuffSize,
                          const size_t kStartIdx,
                          const size_t kCount);

        bool SaveLEDBorderEnabled(const bool kEnabled);
        bool SaveLEDBorderBrightness(const uint8_t kBrightness);
        bool SaveLEDBorderPattern(const ColorPattern* pkPattern);
        bool SaveLEDBorderAnimation(const IColorAnimation* pkAnim,
                                    const uint8_t          kIndex);

        bool RemoveLEDBorderAnimation(const uint8_t kIndex);
        void RemoveLEDBorderAnimations(void);

        bool LoadLEDBorderSettings(bool&                          rEnabled,
                                   uint8_t&                       rBrightness,
                                   ColorPattern**                 ppPattern,
                                   std::vector<IColorAnimation*>& animations);

        void Format(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        Storage(void);

        void Init(void);



        bool LoadLEDBorderEnabled(bool& rEnabled);
        bool LoadLEDBorderBrightness(uint8_t& rBrightness);
        bool LoadLEDBorderPattern(ColorPattern** ppPattern);
        bool LoadLEDBorderAnimations(std::vector<IColorAnimation*>& rAnims);

        void RemoveDirectory(const std::string& rkDirName,
                             const std::string& rkRootDir);

        bool                               init_;
        bool                               faulty_;
        uint64_t                           storageSize_;
        sdcard_type_t                      sdType_;

        std::map<std::string, std::string> cache_;

        static Storage*                    PINSTANCE_;
};

#endif /* #ifndef __CORE_STORAGE_H_ */