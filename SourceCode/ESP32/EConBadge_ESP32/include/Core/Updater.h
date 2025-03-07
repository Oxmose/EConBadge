/*******************************************************************************
 * @file Updater.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 24/02/2025
 *
 * @version 2.0
 *
 * @brief This file defines updater service.
 *
 * @details This file defines updater service. The updater allows to update the
 * EConBadge over the air (OTA).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_UPDATER_H_
#define __CORE_UPDATER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>        /* Generic Types */
#include <Update.h>       /* ESP32 Update manager */
#include <BlueToothMgr.h> /* Bluetooth manager */

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

typedef struct __attribute__((packed))
{
    uint32_t magic;
    uint32_t size;
    uint8_t  checksum[32];
    uint8_t  signature[256];
    char     compatHw[16];
} SUpdateHeader;

typedef enum
{
    UPDATE_DOWNLOAD,
    UPDATE_VERIFY,
    UPDATE_APPLY
} EUpdateStep;

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
 * @brief Updater service class.
 *
 * @details Updater service class. This class provides the services
 * to update the ESP32 with a new firmware. The update is safe as a shadow
 * firmware is used in case the update fails or is stopped mid-process.
 */
class Updater
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        Updater(BluetoothManager* pBtMgr);

        uint8_t GetProgress(void) const;

        void RequestUpdate(const uint8_t* kpData, SCommandResponse& rReponse);


    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        bool DownloadUpdateFile(void);
        bool CheckUpdateFile(void);
        void ApplyUpdate(void);

        void SetProgress(const EUpdateStep kStep, const uint8_t kProgress);

        static void UpdateRoutine(void* pUpdaterParam);


        BluetoothManager* pBtMgr_;
        UpdateClass       update_;
        volatile uint8_t  progress_;
        SUpdateHeader     updateHeader_;
        SCommandResponse* pCommandResponse_;
        TaskHandle_t      updateThread_;
};

#endif /* #ifndef __CORE_UPDATER_H_ */