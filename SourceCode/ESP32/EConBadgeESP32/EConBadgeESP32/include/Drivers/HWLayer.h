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

#ifndef __DRIVERS_HWLAYER_H_
#define __DRIVERS_HWLAYER_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <SPI.h>   /* SPI manager */
#include <cstdint> /* Generic Types */
#include <string>  /* std::string */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define PROTO_REV "HW R.1D"

#define HW_ID_LENGTH 13

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#define GENERAL_SPI GEN_SPI
#define EINK_SPI    SPI

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
extern SPIClass GEN_SPI;

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
class HWManager
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
         * @return The ESP32 unique hardware ID.
         */
        static const char * GetHWUID(void);

        static const char * GetMacAddress(void);

        static uint64_t GetTime(void);

        static void Init(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        static uint64_t    TIME_;

        static std::string HWUID_;
        static std::string MACADDR_;
};

#endif /* #ifndef __DRIVERS_HWLAYER_H_ */