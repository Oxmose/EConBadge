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
#include <Menu.h>             /* Menu manager */
#include <Types.h>            /* Defined types */
#include <Storage.h>          /* Storage manager */
#include <IOButtonMgr.h>      /* Button manager */
#include <BlueToothMgr.h>     /* Bluetooth manager */
#include <DisplayInterface.h> /* Display interface */
#include <WaveshareEInkMgr.h> /* EInk display manager */

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

 /** @brief Defines the different state of the system */
typedef enum
{
    /** @brief System is idling */
    SYS_IDLE = 0,
    /** @brief System display the menu */
    SYS_MENU = 1,
    /** @brief System display the splash screen */
    SYS_SPLASH = 2,
} ESystemState;

/** @brief Command queue definition */
typedef std::queue<std::pair<SCommandRequest, bool>> TCommandQueue;

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
class SystemState: public CommandHandler
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        SystemState(IOButtonMgr*        pButtonMgr,
                    DisplayInterface*   pDisplayInterface,
                    BluetoothManager*   pBlueToothManager,
                    EInkDisplayManager* pEinkManager);

        virtual ~SystemState(void);

        void Update(void);


        virtual EErrorCode EnqueueCommand(SCommandRequest command);
        EErrorCode EnqueueLocalCommand(SCommandRequest command);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetSystemState(const ESystemState kState);
        void UpdateButtonsState(void);
        void ExecuteCommands(void);

        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);

        void SetOwner(const char* kpOwner, SCommandResponse& rReponse);
        void SetContact(const char* kpContact, SCommandResponse& rReponse);

        TCommandQueue commandsQueue_;

        SemaphoreHandle_t           commandsQueueLock_;
        Menu*                       pMenu_;
        Storage*                    pStore_;
        IOButtonMgr*                pButtonMgr_;
        DisplayInterface*           pDisplayInterface_;
        BluetoothManager*           pBlueToothManager_;
        EInkDisplayManager*         pEinkManager_;

        uint8_t             currDebugState_;
        uint64_t            lastEventTime_;
        ESystemState        currState_;
        ESystemState        prevState_;

        uint64_t                    pButtonsKeepTime_[BUTTON_MAX_ID];
        EButtonState                pButtonsState_[BUTTON_MAX_ID];
        EButtonState                pPrevButtonsState_[BUTTON_MAX_ID];
};

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */