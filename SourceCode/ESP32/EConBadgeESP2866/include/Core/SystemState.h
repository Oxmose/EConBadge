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

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define MENU_PAGE_COUNT 4

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

/* None*/

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

        nsCommon::ESystemState GetSystemState(void) const;
        void SetSystemState(const nsCommon::ESystemState state);

        uint8_t GetDebugState(void) const;

        nsCommon::EErrorCode ComputeState(void);

        nsCommon::EButtonState GetButtonState(const nsCommon::EButtonID btnId) const;
        void SetButtonState(const nsCommon::EButtonID btnId,
                            const nsCommon::EButtonState state);
        uint32_t GetButtonKeepTime(const nsCommon::EButtonID btnId) const;
        void SetButtonKeepTime(const nsCommon::EButtonID btnId,
                            const uint32_t keepTime);

        uint8_t GetMenuPage(void) const;
        void GetCurrentMenu(const char *** pMenuItem,
                            const char **  pMenuTitle,
                            uint8_t * pSelectedItemIdx,
                            uint8_t * pItemsCount) const;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        void ManageDebugState(void);
        void ManageIdleState(void);
        void ManageMenuState(void);
        void ManageMenuAction(void);

        nsCommon::ESystemState currState;
        nsCommon::ESystemState prevState;
        nsCommon::EButtonState buttonsState[nsCommon::BUTTON_MAX_ID];
        nsCommon::EButtonState prevButtonsState[nsCommon::BUTTON_MAX_ID];
        uint32_t               buttonsKeepTime[nsCommon::BUTTON_MAX_ID];
        uint32_t               lastEventTime;

        uint8_t currDebugState;

        uint8_t currMenuPage;
        uint8_t currMenuItem[MENU_PAGE_COUNT];
};

} /* nsCore nsCore */

#endif /* #ifndef __CORE_SYSTEM_STATE_H_ */