/*******************************************************************************
 * @file Types.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2022
 *
 * @version 1.0
 *
 * @brief This file defines the types used in the ESP32 module.
 *
 * @details This file defines the types used in the ESP32 module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __COMMON_TYPES_H_
#define __COMMON_TYPES_H_

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <cstdint> /* Standard Int Types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief The ECB logo width in pixels. */
#define LOGO_WIDTH  57
/** @brief The ECB logo height in pixels. */
#define LOGO_HEIGHT 41

/** @brief Size of a command in bytes. */
#define COMMAND_DATA_SIZE 64
/** @brief Defines the command response length. */
#define COMMAND_RESPONSE_LENGTH 64

/** @brief Size of the communication token */
#define COMM_TOKEN_SIZE 16

#define OWNER_FILE_PATH            "/owner"
#define CONTACT_FILE_PATH          "/contact"
#define BLUETOOTH_TOKEN_FILE_PATH  "/bttoken"
#define CURRENT_IMG_NAME_FILE_PATH "/currimg"
#define UPDATE_FILE_PATH           "/firmware_update"
#define TMP_DIR_PATH               "/tmp"

#define LEDBORDER_DIR_PATH             "/ledborder"
#define LEDBORDER_ENABLED_FILE_PATH    LEDBORDER_DIR_PATH "/enabled"
#define LEDBORDER_BRIGHTNESS_FILE_PATH LEDBORDER_DIR_PATH "/brightness"
#define LEDBORDER_PATTERN_FILE_PATH    LEDBORDER_DIR_PATH "/pattern"
#define LEDBORDER_ANIM_FILE_PATH       LEDBORDER_DIR_PATH "/anim"
#define LEDBORDER_PATTERN_TMP_FILE     TMP_DIR_PATH "/tmp_pattern"
#define LEDBORDER_ANIM_TMP_FILE        TMP_DIR_PATH "/tmp_anim"

#define IMAGE_DIR_PATH "/images"

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/**
 * @brief Gets the minimal value between two.
 *
 * @param[in] X The first value.
 * @param[in] Y The second value.
 */
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

/**
 * @brief Gets the maximal value between two.
 *
 * @param[in] X The first value.
 * @param[in] Y The second value.
 */
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

/*******************************************************************************
 * STRUCTURES AND TYPES
 ******************************************************************************/

/**
 * @brief Defines the error status type.
 */
typedef enum
{
    /** @brief No error occured. */
    NO_ERROR             = 0,
    /** @brief An invalid parameter was used */
    INVALID_PARAM        = 1,
    /** @brief The Action failed */
    ACTION_FAILED        = 2,
    /** @brief Component was not initalialized */
    NOT_INITIALIZED      = 3,
    /** @brief No action to be done */
    NO_ACTION            = 4,
    /** @brief Invalid Bluetooth token */
    INVALID_TOKEN        = 5,
    /** @brief Invalid command size */
    INVALID_COMMAND_SIZE = 6,
    /** @brief File was not found */
    FILE_NOT_FOUND       = 7,
    /** @brief No more memory in the ECB */
    NO_MORE_MEMORY       = 8,
    /** @brief Invalid command request */
    INVALID_COMMAND_REQ  = 9,
    /** @brief The maximal number of commands has been reached. */
    MAX_COMMAND_REACHED  = 10,
    /** @brief Failed to update the current image name. */
    IMG_NAME_UPDATE_FAIL = 11,
    /** @brief Failed to open file */
    OPEN_FILE_FAILED     = 12,
    /** @brief Failed to write file */
    WRITE_FILE_FAILED    = 13,
    /** @brief Failed to read file */
    READ_FILE_FAILED     = 14,
    /** @brief Transation send failed. */
    TRANS_SEND_FAILED    = 15,
    /** @brief Transation receive failed. */
    TRANS_RECV_FAILED    = 16,
    /** @brief Data too long */
    DATA_TOO_LONG        = 17,
    /** @brief Corrupted data */
    CORRUPTED_DATA       = 18,
    /** @brief Invalid index. */
    INVALID_INDEX        = 19,
    /** @brief LED patterns are overlapping */
    OVERLAPPING_PATTERNS = 20,

} EErrorCode;

/** @brief Defines the GPIO routing for the EConBadge */
typedef enum
{
    /** @brief Back button GPIO */
    GPIO_BTN_BACK   = 0,
    /** @brief Down button GPIO */
    GPIO_BTN_DOWN   = 2,
    /** @brief Enter button GPIO */
    GPIO_BTN_ENTER  = 4,
    /** @brief Up button GPIO */
    GPIO_BTN_UP     = 5,
    /** @brief SD card MISO GPIO */
    GPIO_SD_MISO    = 13,
    /** @brief SD card CS GPIO */
    GPIO_SD_CS      = 14,
    /** @brief EInk DIN GPIO */
    GPIO_EINK_DIN   = 15,
    /** @brief EInk CLK GPIO */
    GPIO_EINK_CLK   = 16,
    /** @brief EInk CS GPIO */
    GPIO_EINK_CS    = 17,
    /** @brief EInk DC GPIO */
    GPIO_EINK_DC    = 18,
    /** @brief EInk reset GPIO */
    GPIO_EINK_RESET = 19,
    /** @brief OLED I2C SCL GPIO */
    GPIO_OLED_SCL   = 21,
    /** @brief OLED I2C SDA GPIO */
    GPIO_OLED_SDA   = 22,
    /** @brief EInk BUSY GPIO */
    GPIO_EINK_BUSY  = 23,
    /** @brief LED border ENABLE GPIO */
    GPIO_LED_ENABLE = 25,
    /** @brief SD card MOSI GPIO */
    GPIO_SD_MOSI    = 26,
    /** @brief SD card CLK GPIO */
    GPIO_SD_CLK     = 27,
    /** @brief LED border DATA GPIO */
    GPIO_LED_DATA   = 32,
    /** @brief ADC battery GPIO */
    GPIO_ADC_BAT    = 34
} EGPIORouting;

typedef enum
{
   CMD_PING                       = 0,
   CMD_SET_BT_TOKEN               = 1,

   CMD_EINK_CLEAR                 = 2,
   CMD_EINK_NEW_IMAGE             = 3,
   CMD_EINK_REMOVE_IMAGE          = 4,
   CMD_EINK_SELECT_IMAGE          = 5,
   CMD_EINK_GET_CURRENT_IMG_NAME  = 6,
   CMD_EINK_GET_CURRENT_IMG       = 7,

   CMD_FACTORY_RESET              = 8,

   CMD_SET_OWNER                  = 9,
   CMD_SET_CONTACT                = 10,
   CMD_GET_OWNER                  = 11,
   CMD_GET_CONTACT                = 12,

   CMD_FIRMWARE_UPDATE            = 13,

   CMD_LEDBORDER_SET_ENABLE       = 14,
   CMD_LEDBORDER_GET_ENABLE       = 15,
   CMD_LEDBORDER_INC_BRIGHTNESS   = 16,
   CMD_LEDBORDER_DEC_BRIGHTNESS   = 17,
   CMD_LEDBORDER_SET_BRIGHTNESS   = 18,
   CMD_LEDBORDER_GET_BRIGHTNESS   = 19,
   CMD_LEDBORDER_CLEAR            = 20,
   CMD_LEDBORDER_ADD_PATTERN      = 21,
   CMD_LEDBORDER_REMOVE_PATTERN   = 22,
   CMD_LEDBORDER_CLEAR_PATTERNS   = 23,
   CMD_LEDBORDER_GET_PATTERNS     = 24,
   CMD_LEDBORDER_ADD_ANIMATION    = 25,
   CMD_LEDBORDER_REMOVE_ANIMATION = 26,
   CMD_LEDBORDER_CLEAR_ANIMATIONS = 27,
   CMD_LEDBORDER_GET_ANIMATIONS   = 28,

   MAX_COMMAND_TYPE               = 29,
} ECommandType;

/** @brief Defines the command header */
typedef struct __attribute__((packed))
{
    /** @brief The command id */
    uint32_t identifier;

    /** @brief The token used for the communication */
    uint8_t pToken[COMM_TOKEN_SIZE];

    union {
        /** @brief Command type */
        uint8_t type;

        /** @brief The command return code. */
        uint8_t errorCode;
    };

    /** @brief Data size */
    uint8_t size;

} SCommandHeader;

/** @brief Defines the request of a command. */
typedef struct __attribute__((packed))
{
    /** @brief The command header */
    SCommandHeader header;

    /** @brief The command data */
    uint8_t pCommand[COMMAND_DATA_SIZE];
} SCommandRequest;

/** @brief Defines the response to a command. */
typedef struct __attribute__((packed))
{
    /** @brief The command header */
    SCommandHeader header;

    /** @brief The command additional data. */
    uint8_t pResponse[COMMAND_RESPONSE_LENGTH];
} SCommandResponse;

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/************************* Imported global variables **************************/
/* None */

/************************* Exported global variables **************************/
/** @brief The logo bitmap. */
extern const unsigned char PKLOGO_BITMAP[];

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

/* None */

#endif /* #ifndef __COMMON_TYPES_H_ */