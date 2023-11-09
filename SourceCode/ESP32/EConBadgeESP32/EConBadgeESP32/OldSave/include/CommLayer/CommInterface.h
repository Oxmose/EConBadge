/*******************************************************************************
 * @file CommInterface.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 26/12/2022
 *
 * @version 1.0
 *
 * @brief Communication Layer Internal.
 *
 * @details Communication Layer Internal. This file defines the interface for
 * a communication driver (e.g wifi, bt, etc.).
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMLAYER_COMMINTERFACE_H_
#define __COMMLAYER_COMMINTERFACE_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <Types.h>            /* Common module's types */

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
 * @brief Communication Services Namespace
 * @details Communication Services Namespace. This namespace gathers the
 * declarations and definitions related to the communication services.
 */
namespace nsComm
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

class ICommInterface
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        virtual ~ICommInterface(void) {};
        virtual nsCommon::EErrorCode ReadBytes(uint32_t * readSize,
                                               void * buffer) = 0;
        virtual nsCommon::EErrorCode WriteBytes(uint32_t * writeSize,
                                                const void * buffer) = 0;

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:

};

} /* namespace nsComm */

#endif /* #ifndef __COMMLAYER_COMMINTERFACE_H_ */