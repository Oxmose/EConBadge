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
#include <queue>              /* std:: queue */
#include <Types.h>            /* Defined types */
#include <Storage.h>          /* Storage manager */
#include <IOButtonMgr.h>      /* Button manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <OLEDScreenMgr.h>    /* OLED screen manager */
#include <WaveshareEInkMgr.h> /* EInk manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

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

class SystemState: public CommandHandler
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        SystemState(OLEDScreenMgr*      pOLEDMgr,
                    IOButtonMgr*        pButtonMgr,
                    BluetoothManager*   pBtMgr,
                    EInkDisplayManager* pEInkMgr);
        virtual ~SystemState(void){}

        EErrorCode Update(void);
        void Ping(void);

        void SetLedBorder(LEDBorder* pLedBorder);
        void SetUpdater(Updater* pUpdater);

        ESystemState GetSystemState(void) const;
        uint8_t GetDebugState(void) const;

        EMenuAction ConsumeMenuAction(void);
        ELEDBorderAction ConsumeELEDBorderAction(uint8_t pBuffer[COMMAND_DATA_SIZE]);
        EUpdaterAction ConsumeUpdateAction(void);

        uint64_t GetLastEventTime(void) const;
        EButtonState GetButtonState(const EButtonID kBtnId) const;
        uint64_t GetButtonKeepTime(const EButtonID kBtnId) const;

        virtual EErrorCode EnqueueCommand(SCommandRequest command);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetSystemState(const ESystemState kState);

        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);

        void UpdateButtonsState(void);

        void SendBadgeInfo(void);
        void SendLedBorderInfo(void);
        void SendEInkImagesName(const uint32_t kStartIdx, uint32_t count);

        uint8_t             currDebugState_;
        uint64_t            lastEventTime_;
        ESystemState        currState_;
        ESystemState        prevState_;

        uint8_t             pNextLEDBorderMeta_[COMMAND_DATA_SIZE];
        uint64_t            pButtonsKeepTime_[EButtonID::BUTTON_MAX_ID];
        EButtonState        pButtonsState_[EButtonID::BUTTON_MAX_ID];
        EButtonState        pPrevButtonsState_[EButtonID::BUTTON_MAX_ID];


        Updater*            pUpdater_;
        Storage*            pStore_;
        LEDBorder*          pLedBorderMgr_;
        IOButtonMgr*        pButtonMgr_;
        OLEDScreenMgr*      pOLEDMgr_;
        BluetoothManager*   pBtMgr_;
        EInkDisplayManager* pEInkMgr_;

        EMenuAction         nextMenuAction_;
        EUpdaterAction      nextUpdateAction_;
        ELEDBorderAction    nextLEDBorderAction_;

        std::queue<SCommandRequest> commandsQueue_;
        SemaphoreHandle_t           commandsQueueLock_;
};

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */