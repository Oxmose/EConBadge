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

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * CLASS METHODS
 ******************************************************************************/

EErrorCode CCTRL::ExecuteCommand(const uint32_t command,
                                 nsComm::ICommInterface * comm) const
{
    EErrorCode retCode;

    /* Check command bound */
    retCode = NO_ERROR;
    if(command < COMM_MAX_ID)
    {
        LOG_DEBUG("Executing Command %d\n", command);

        switch(command)
        {
            case COMM_PING_ID:
                CommPing();
                break;
            default:
                retCode = NO_ACTION;
        }
    }
    else
    {
        retCode = NO_ACTION;
    }

    return retCode;
}

void CCTRL::CommPing(void) const
{
    LOG_INFO("PONG\n");
    delay(2000);
}

#undef CCTRL