/*******************************************************************************
 * @file Battery.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 15/02/2024
 *
 * @version 1.0
 *
 * @brief This file contains the battery manager.
 *
 * @details This file contains the battery manager. The file provides the
 * services to get the battery status such as its charching state and its
 * current capacity.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __DRIVERS_BATTERYMGR_H_
#define __DRIVERS_BATTERYMGR_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint>     /* Generic Types */
#include <Types.h>     /* Defined Types */
#include <LEDBorder.h> /* LED border manager */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* None */

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/

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

class BatteryManager
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        BatteryManager(LEDBorder* pLEDBorder);

        void Update(void);

        uint32_t GetPercentage(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        uint32_t   percentage_;
        uint64_t   lastUpdate_;

        LEDBorder* pLEDBorderMgr_;
};


#endif /* #ifndef __DRIVERS_BATTERYMGR_H_ */