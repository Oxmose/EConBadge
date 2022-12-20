/*******************************************************************************
 * @file HWLayer.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file defines the hardware layer.
 *
 * @details This file defines the hardware layer. This layer provides services
 * to interact with the ESP32 module hardware.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __HWLAYER_HWLAYER_H_
#define __HWLAYER_HWLAYER_H_

/****************************** OUTER NAMESPACE *******************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <cstdint> /* Generic Types */
#include <string>  /* String */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define BUTTON_ENTER_PIN D5
#define BUTTON_UP_PIN    D3
#define BUTTON_DOWN_PIN  D6

#define LED_MAIN_PIN     D4

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/* None */

/****************************** INNER NAMESPACE *******************************/
/**
 * @brief Hardware Layer Namespace
 * @details Hardware Layer Namespace used for definitions of hardware related
 * services.
 */
namespace nsHWL
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

/**
 * @brief Hardware manager class.
 *
 * @details Hardware manager class. This class provides the services
 * to access hwardware information, features and interract directly with the
 * different components of the ESP32 module.
 */
class CHWManager
{
    /********************* PUBLIC METHODS AND ATTRIBUTES **********************/
    public:
        /**
         * @brief Returns the ESP32 unique hardware ID.
         *
         * @details Returns the ESP32 unique hardware ID. The unique ID is
         * composed of a basic string (defined directly in the source code) and
         * a part of the ESP32 mac address.
         *
         * @param[out] pBuffer The buffer used to receive the unique ID.
         * @param[in] maxLength The maximal length of the buffer.
         */
        void GetHWUID(char * pBuffer, const uint32_t maxLength);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        String HWUID;
};

} /* namespace nsHWL */

#endif /* #ifndef __HWLAYER_HWLAYER_H_ */