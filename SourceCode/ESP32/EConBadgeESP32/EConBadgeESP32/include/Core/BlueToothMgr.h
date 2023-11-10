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

#include <BluetoothSerial.h> /* Bluetooth driver */
#include <SystemState.h>     /* System state manager */

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
        BluetoothManager(void);
        ~BluetoothManager(void);

        void Init(const char* hwName);

        bool ReceiveCommand(SCBCommand * command);

        void ReceiveData(uint8_t * buffer, size_t& size);
        void TransmitData(const uint8_t * buffer, size_t& size);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        BluetoothSerial btSerialIface_;
        uint8_t         magicStep_;
        bool            recomposingCommand_;
        SCBCommand      comm_;
        uint32_t        commCursor_;
};

#endif /* #ifndef __CORE_BLUETOOTH_MGR_H_ */