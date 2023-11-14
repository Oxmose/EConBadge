/*******************************************************************************
 * @file Updater.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 12/11/2022
 *
 * @version 1.0
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

#include <cstdint> /* Generic Types */
#include <BlueToothMgr.h> /* Bluetooth manager */
#include <SystemState.h> /* System state service */
#include <Update.h>      /* ESP32 Update manager */

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

typedef enum
{
    WAITING_START    = 0,
    WAITING_VALID    = 1,
    APPLYING_UPDATE  = 2,
    IDLE             = 3
} EUpdateState;

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
        Updater(BluetoothManager * btMgr, SystemState * sysState);
        ~Updater(void);

        void Update(void);

        uint64_t     GetTimeoutLeft(void) const;
        EUpdateState GetStatus(void) const;

        void RequestUpdate(void);


    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void WaitUpdateStart(void);
        void WaitUpdateValidation(void);
        void ApplyUpdate(void);


        BluetoothManager * btMgr_;
        SystemState *      sysState_;
        uint64_t           timeout_;
        EUpdateState       state_;
        UpdateClass        update_;
};

#endif /* #ifndef __CORE_UPDATER_H_ */