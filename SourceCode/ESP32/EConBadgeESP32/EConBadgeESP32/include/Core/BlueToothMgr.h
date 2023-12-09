/*******************************************************************************
 * @file BlueToothMgr.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/11/2023
 *
 * @version 1.0
 *
 * @brief This file provides the BlueTooth service
 *
 * @details This file provides the BlueTooth service. This files defines
 * the different features embedded for the BT service.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CORE_BLUETOOTH_MGR_H_
#define __CORE_BLUETOOTH_MGR_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <SystemState.h>     /* System state manager */
#include <BluetoothSerial.h> /* Bluetooth driver */

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

class BluetoothManager
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        BluetoothManager  (void);

        void Init (void);

        bool ReceiveCommand (SCBCommand * pCommand);

        void ReceiveData  (uint8_t * pBuffer, size_t & rSize);
        void TransmitData (const uint8_t * pkBuffer, size_t & rSize);

        bool UpdateSettings (const uint8_t * pkBuffer);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        bool              recomposingCommand_;
        uint8_t           magicStep_;
        uint32_t          comCursor_;

        std::string       pin_;
        std::string       name_;

        SCBCommand        comm_;
        BluetoothSerial   btIface_;
        Storage         * pStorage_;
};

#endif /* #ifndef __CORE_BLUETOOTH_MGR_H_ */