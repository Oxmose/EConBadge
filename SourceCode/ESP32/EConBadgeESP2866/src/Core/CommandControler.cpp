/*******************************************************************************
 * @file CommandControler.cpp
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file prvides the command rontroler service.
 *
 * @details This file prvides the command rontroler service. This files defines
 * the different features embedded in the command controller. It is used to
 * service IO input (buttons) and network (WIFI / Bluetooth) commands.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstring>        /* String manipulation*/
#include <Types.h>        /* Defined Types */
#include <Logger.h>       /* Logger Service */

/* Header File */
#include <CommandControler.h>

using namespace nsCommon;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Class namespace shortcut. */
#define CCTRL nsCore::CCommandControler

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

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

static nsCommon::EErrorCode CommandPing(const CCTRL * commCtrl, void * args);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static nsCommon::EErrorCode CommandPing(const CCTRL * commCtrl, void * args)
{
    LOG_INFO("PING Command\n");

    return NO_ERROR;
}

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

CCTRL::CCommandControler(void)
{
    this->handlers[COMM_PING_ID] = CommandPing;
}

EErrorCode CCTRL::ParseCommand(const SSystemCommand & command) const
{
    EErrorCode retCode;

    /* Check command bound */
    if(command.command < COMM_MAX_ID)
    {
        LOG_DEBUG("Executing Command %d\n", command.command);
        /* Call the function handler */
        retCode = this->handlers[command.command](this, (void*)command.args);
    }
    else
    {
        retCode = NO_ACTION;
    }

    return retCode;
}

#undef CSYSSTATE