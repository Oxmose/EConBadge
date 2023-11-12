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

#define RESPONSE_MAGIC    0xCB1EDEC0
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
    MAX_COMMAND_TYPE
} ECommandType;

typedef enum
{
    EINK_CLEAR  = 0,
    EINK_UPDATE = 1,
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

typedef struct
{
    uint8_t type;
    uint8_t commandData[COMMAND_DATA_SIZE];
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

class SystemState
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        SystemState(IOButtonMgr * buttonMgr, BluetoothManager * btMgr);

        EErrorCode   Update(void);

        ESystemState GetSystemState(void) const;
        uint8_t      GetDebugState(void) const;

        EMenuAction      ConsumeMenuAction(void);
        EEinkAction      ConsumeEInkAction(void);
        ELEDBorderAction ConsumeELEDBorderAction(uint8_t buffer[COMMAND_DATA_SIZE]);

        uint32_t     GetLastEventTime(void) const;
        EButtonState GetButtonState(const EButtonID btnId) const;
        uint32_t     GetButtonKeepTime(const EButtonID btnId) const;

        bool EnqueueResponse(const uint8_t * buffer, const uint8_t size);
        bool SendResponseNow(const uint8_t * buffer, const uint8_t size);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void SetSystemState(const ESystemState state);

        bool SendPendingTransmission(void);

        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);

        void UpdateButtonsState(void);

        void HandleCommand(SCBCommand * command);

        ESystemState currState_;
        ESystemState prevState_;
        uint32_t     lastEventTime_;

        EButtonState buttonsState_[EButtonID::BUTTON_MAX_ID];
        EButtonState prevButtonsState_[EButtonID::BUTTON_MAX_ID];
        uint32_t     buttonsKeepTime_[EButtonID::BUTTON_MAX_ID];

        uint8_t      currDebugState_;

        IOButtonMgr *      buttonMgr_;
        BluetoothManager * btMgr_;

        EMenuAction      nextMenuAction_;
        EEinkAction      nextEInkAction_;
        ELEDBorderAction nextLEDBorderAction_;
        uint8_t          nextLEDBorderMeta_[COMMAND_DATA_SIZE];

        uint8_t *        txQueue_;
};

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */