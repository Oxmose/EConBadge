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

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Standard Int Types */
#include <Types.h> /* Defined types */

#include <OLEDScreenDriver.h> /* OLED driver */
#include <Menu.h>             /* Menu management */
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Core Namespace
 * @details Core Namespace used to gather the core services of the ESP32 module.
 */
namespace nsCore
{

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

class CSystemState
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        CSystemState(void);

        void Init(nsHWL::COLEDScreenMgr * oledDriver);

        nsCommon::ESystemState GetSystemState(void) const;
        void SetSystemState(const nsCommon::ESystemState state);

        uint32_t GetLastEventTime(void) const;

        uint8_t GetDebugState(void) const;

        nsCommon::EErrorCode ComputeState(void);

        nsCommon::EButtonState GetButtonState(const nsCommon::EButtonID btnId) const;
        void SetButtonState(const nsCommon::EButtonID btnId,
                            const nsCommon::EButtonState state);
        uint32_t GetButtonKeepTime(const nsCommon::EButtonID btnId) const;
        void SetButtonKeepTime(const nsCommon::EButtonID btnId,
                               const uint32_t keepTime);


    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);

        nsHWL::COLEDScreenMgr * oledDriver;

        nsCommon::ESystemState currState;
        nsCommon::ESystemState prevState;
        nsCommon::EButtonState buttonsState[nsCommon::BUTTON_MAX_ID];
        nsCommon::EButtonState prevButtonsState[nsCommon::BUTTON_MAX_ID];
        uint32_t               buttonsKeepTime[nsCommon::BUTTON_MAX_ID];
        uint32_t               lastEventTime;

        uint8_t currDebugState;

        CMenu menu;
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */