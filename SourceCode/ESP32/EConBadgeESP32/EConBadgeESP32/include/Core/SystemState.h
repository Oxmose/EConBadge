/*******************************************************************************
 * @file SystemState.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the system state service.
 *
 * @details This file provides the system state service. This files defines
 * the different features embedded in the system state.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_SYSTEM_STATE_H_
#define __CORE_SYSTEM_STATE_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>         /* Standard Int Types */
#include <Types.h>         /* Defined types */
#include <IOButtonMgr.h>   /* Button manager */
#include <OLEDScreenMgr.h> /* OLED screen manager */
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define RESPONSE_MAGIC    0xCB1EDEC0
#define COMMAND_MAGIC     0xC0DE1ECB
#define COMMAND_DATA_SIZE 63

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    SYS_IDLE         = 0,
    SYS_START_SPLASH = 1,
    SYS_MENU         = 2
} ESystemState;

typedef enum
{
    SELECT_NEXT            = 0,
    SELECT_PREV            = 1,
    EXECUTE_SEL            = 2,
    BACK_MENU              = 3,
    REFRESH_LEDB_STATE     = 4,
    REFRESH_MYINFO         = 5,
    REFRESH_BT_INFO        = 6,
    VALIDATE_FACTORY_RESET = 7,
    NONE
} EMenuAction;

typedef enum
{
    PING                  = 0,
    CLEAR_EINK            = 1,
    UPDATE_EINK           = 2,
    ENABLE_LEDB           = 3,
    DISABLE_LEDB          = 4,
    ADD_ANIMATION_LEDB    = 5,
    REMOVE_ANIMATION_LEDB = 6,
    SET_PATTERN_LEDB      = 7,
    CLEAR_ANIMATION_LEDB  = 8,
    SET_BRIGHTNESS_LEDB   = 9,
    SET_OWNER_VALUE       = 10,
    SET_CONTACT_VALUE     = 11,
    SET_BT_SETTINGS       = 12,
    REQUEST_FACTORY_RESET = 13,
    START_UPDATE          = 14,
    VALIDATE_UPDATE       = 15,
    CANCEL_UPDATE         = 16,
    START_TRANS_UPDATE    = 17,
    GET_INFO              = 18,
    REQUEST_UPDATE        = 19,
    GET_INFO_LEDBORDER    = 20,
    GET_IMAGES_NAME       = 21,
    REMOVE_IMAGE          = 22,
    SELECT_LOADED_IMAGE   = 23,
    GET_CURRENT_IMAGE     = 24,
    MAX_COMMAND_TYPE
} ECommandType;

typedef enum
{
    EINK_CLEAR              = 0,
    EINK_UPDATE             = 1,
    EINK_SELECT_LOADED      = 2,
    EINK_SEND_CURRENT_IMAGE = 3,
    EINK_NONE
} EEinkAction;

typedef enum
{
    ENABLE_LEDB_ACTION           = 0,
    DISABLE_LEDB_ACTION          = 1,
    ADD_ANIMATION_LEDB_ACTION    = 2,
    REMOVE_ANIMATION_LEDB_ACTION = 3,
    SET_PATTERN_LEDB_ACTION      = 4,
    CLEAR_ANIMATION_LEDB_ACTION  = 5,
    SET_BRIGHTNESS_LEDB_ACTION   = 6,
    ELEDBORDER_NONE
} ELEDBorderAction;

typedef enum
{
    START_UPDATE_ACTION   = 0,
    VALIDATION_ACTION     = 1,
    CANCEL_ACTION         = 2,
    START_TRANSFER_ACTION = 3,
    EUPDATER_NONE
} EUpdaterAction;

typedef struct
{
    uint8_t type;
    uint8_t pCommandData[COMMAND_DATA_SIZE];
} SCBCommand;

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

/* Forward Declatations */
class BluetoothManager;
class LEDBorder;
class Updater;

class SystemState
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        SystemState (OLEDScreenMgr    * pOLEDMgr,
                     IOButtonMgr      * pButtonMgr,
                     BluetoothManager * pBtMgr);

        EErrorCode Update (void);
        void       Ping   (void);

        void SetLedBorder (LEDBorder * pLedBorder);
        void SetUpdater   (Updater   * pUpdater);

        ESystemState GetSystemState (void) const;
        uint8_t      GetDebugState  (void) const;

        EMenuAction      ConsumeMenuAction       (void);
        EEinkAction      ConsumeEInkAction       (uint8_t pBuffer[COMMAND_DATA_SIZE]);
        ELEDBorderAction ConsumeELEDBorderAction (uint8_t pBuffer[COMMAND_DATA_SIZE]);
        EUpdaterAction   ConsumeUpdateAction     (void);

        uint64_t     GetLastEventTime  (void) const;
        EButtonState GetButtonState    (const EButtonID kBtnId) const;
        uint64_t     GetButtonKeepTime (const EButtonID kBtnId) const;

        void EnqueueResponse (const uint8_t * pkBuffer, const uint8_t kSize);
        bool SendResponseNow (const uint8_t * pkBuffer, const uint8_t kSize);

        void ClearTransmissionQueue (void);

        void ManageBoot (void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetSystemState (const ESystemState kState);

        bool SendPendingTransmission (void);

        void ManageDebugState (void);
        void ManageIdleState  (void);
        void ManageMenuState  (void);

        void UpdateButtonsState (void);

        void HandleCommand (SCBCommand * pCommand);

        void SendBadgeInfo      (void);
        void SendLedBorderInfo  (void);
        void SendEInkImagesName (const uint32_t kStartIdx, uint32_t count);

        void Hibernate (const bool kDisplay);

        uint8_t            currDebugState_;
        uint64_t           lastEventTime_;
        uint64_t           startTime_;
        ESystemState       currState_;
        ESystemState       prevState_;
        uint8_t          * pTxQueue_;

        uint8_t            pNextEInkMeta_[COMMAND_DATA_SIZE];
        uint8_t            pNextLEDBorderMeta_[COMMAND_DATA_SIZE];
        uint64_t           pButtonsKeepTime_[EButtonID::BUTTON_MAX_ID];
        EButtonState       pButtonsState_[EButtonID::BUTTON_MAX_ID];
        EButtonState       pPrevButtonsState_[EButtonID::BUTTON_MAX_ID];


        Updater          * pUpdater_;
        Storage          * pStore_;
        LEDBorder        * pLedBorderMgr_;
        IOButtonMgr      * pButtonMgr_;
        OLEDScreenMgr    * pOLEDMgr_;
        BluetoothManager * pBtMgr_;

        EMenuAction        nextMenuAction_;
        EEinkAction        nextEInkAction_;
        EUpdaterAction     nextUpdateAction_;
        ELEDBorderAction   nextLEDBorderAction_;
};

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */