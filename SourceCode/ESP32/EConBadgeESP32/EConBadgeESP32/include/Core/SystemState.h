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

#include <cstdint> /* Standard Int Types */
#include <Types.h> /* Defined types */

#include <IOButtonMgr.h>  /* Button manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SYSTEM_IDLE_TIME 150000 /* NS : 15 sec*/

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define COMMAND_MAGIC     0xC0DE1ECB
#define COMMAND_DATA_SIZE 63

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

typedef enum
{
    SYS_IDLE                = 0,
    SYS_START_SPLASH        = 1,
    SYS_MENU                = 2
} ESystemState;

typedef enum
{
    SELECT_NEXT = 0,
    SELECT_PREV = 1,
    EXECUTE_SEL = 2,
    NONE
} EMenuAction;

typedef enum
{
    PING        = 0,
    CLEAR_EINK  = 1,
    UPDATE_EINK = 2,
    MAX_COMMAND_TYPE
} ECommandType;

typedef enum
{
    EINK_CLEAR  = 0,
    EINK_UPDATE = 1,
    EINK_NONE
} EEinkAction;

typedef struct
{
    uint8_t type;
    uint8_t commandData[COMMAND_DATA_SIZE];
} ECBCommand;

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

class SystemState
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        SystemState(IOButtonMgr * buttonMgr, BluetoothManager * btMgr);

        ESystemState GetSystemState(void) const;
        uint8_t      GetDebugState(void) const;
        EMenuAction  ConsumeMenuAction(void);
        EEinkAction  ConsumeEInkAction(void);
        EErrorCode   Update(void);

        uint32_t     GetLastEventTime(void) const;
        EButtonState GetButtonState(const EButtonID btnId) const;
        uint32_t     GetButtonKeepTime(const EButtonID btnId) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetSystemState(const ESystemState state);

        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);

        void UpdateButtonsState(void);

        void HandleCommand(ECBCommand * command);

        ESystemState currState_;
        ESystemState prevState_;
        uint32_t     lastEventTime_;

        EButtonState buttonsState_[EButtonID::BUTTON_MAX_ID];
        EButtonState prevButtonsState_[EButtonID::BUTTON_MAX_ID];
        uint32_t     buttonsKeepTime_[EButtonID::BUTTON_MAX_ID];

        uint8_t      currDebugState_;

        IOButtonMgr *      buttonMgr_;
        BluetoothManager * btMgr_;

        EMenuAction nextMenuAction_;
        EEinkAction nextEInkAction_;
};

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */