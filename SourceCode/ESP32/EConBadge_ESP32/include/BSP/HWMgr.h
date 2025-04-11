/*******************************************************************************
 * @file HWMgr.h
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

#ifndef __BSP_HWMGR_H_
#define __BSP_HWMGR_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <SPI.h>   /* SPI manager */
#include <string>  /* std::string */
#include <cstdint> /* Generic Types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief Hardware version */
#define PROTO_REV "HW R.1F"

/** @brief Hardware versioning length */
#define HW_ID_LENGTH 13

/** @brief General SPI bus */
#define GENERAL_SPI GEN_SPI

/** @brief EINK SPI bus */
#define EINK_SPI SPI

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
/** @brief General SPI bus declaration */
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
        static const char* GetHWUID(void);

        /**
         * @brief Returns the ESP32 MAC address.
         *
         * @details Returns the ESP32 MAC address. The bluetooth MAC address is
         * used.
         *
         * @return The ESP32 MAC address.
         */
        static const char* GetMacAddress(void);

        /**
         * @brief Returns the time in microseconds.
         *
         * @details Returns the time in microseconds. The 64bit value is
         * software managed as the ESP32 only has 32bits resolution for the
         * time.
         *
         * @return The execution time is returned.
         */
        static uint64_t GetTime(void);

        /**
         * @brief Delays the calling thread.
         *
         * @details Delays the calling thread, if possible the passive option
         * will be used. If not, active wait will be made.
         *
         * @param[in] kDelayUs The details to wait in microseconds.
         */
        static void DelayExecUs(const uint64_t kDelayUs);

        /**
         * @brief Initializes the hadrware manager.
         *
         * @details Initializes the hadrware manager. This function setups the
         * different GPIO and peripherals needed at startup.
         */
        static void Init(void);

    /******************* PROTECTED METHODS AND ATTRIBUTES *********************/
    protected:
        /* None */

    /********************* PRIVATE METHODS AND ATTRIBUTES *********************/
    private:
        /** @brief Stores the current execution time. */
        static uint64_t    TIME_;
        /** @brief Stores the hwardware unique ID. */
        static std::string HWUID_;
        /** @brief Stores the MAC address. */
        static std::string MACADDR_;
};

#endif /* #ifndef __BSP_HWMGR_H_ */