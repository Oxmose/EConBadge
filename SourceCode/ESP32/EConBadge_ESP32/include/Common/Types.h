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
#include <cstdint>          /* Standard Int Types */
#include <esp32-hal-gpio.h> /* GPIO contants*/

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

#if ECB_ROOTING_1_F
/** @brief Defines the GPIO routing for the EConBadge */
typedef enum
{
    /** @brief Up button GPIO */
    GPIO_BTN_UP     = GPIO_NUM_0,
    /** @brief Back button GPIO */
    GPIO_BTN_DOWN   = GPIO_NUM_2,
    /** @brief Enter button GPIO */
    GPIO_BTN_ENTER  = GPIO_NUM_4,
    /** @brief Back button GPIO */
    GPIO_BTN_BACK   = GPIO_NUM_15,

    /** @brief SD card MISO GPIO */
    GPIO_SD_MISO    = GPIO_NUM_13,
    /** @brief SD card CLK GPIO */
    GPIO_SD_CLK     = GPIO_NUM_14,
    /** @brief SD card CS GPIO */
    GPIO_SD_CS      = GPIO_NUM_26,
    /** @brief SD card MOSI GPIO */
    GPIO_SD_MOSI    = GPIO_NUM_27,
    
    /** @brief EInk CLK GPIO */
    GPIO_EINK_DC    = GPIO_NUM_5,
    /** @brief EInk BUSY GPIO */
    GPIO_EINK_BUSY  = GPIO_NUM_16,
    /** @brief EInk reset GPIO */
    GPIO_EINK_RESET = GPIO_NUM_17,
    /** @brief EInk DIN GPIO */
    GPIO_EINK_CS    = GPIO_NUM_18,
    /** @brief EInk DC GPIO */
    GPIO_EINK_CLK   = GPIO_NUM_19,
    /** @brief EInk CS GPIO */
    GPIO_EINK_DIN   = GPIO_NUM_21,
    
    /** @brief OLED I2C SCL GPIO */
    GPIO_OLED_SCL   = GPIO_NUM_22,
    /** @brief OLED I2C SDA GPIO */
    GPIO_OLED_SDA   = GPIO_NUM_23,
    
    /** @brief LED border ENABLE GPIO */
    GPIO_LED_ENABLE = GPIO_NUM_32,
    /** @brief LED border DATA GPIO */
    GPIO_LED_DATA   = GPIO_NUM_33,
    
    /** @brief ADC battery GPIO */
    GPIO_ADC_BAT    = GPIO_NUM_34,
} EGPIORouting;

/** Define the GPIO pinmux behavior */
typedef enum 
{
    /** @brief Up button GPIO */
    GPIO_BTN_UP_MUX     = INPUT_PULLUP,
    /** @brief Back button GPIO */
    GPIO_BTN_DOWN_MUX   = INPUT_PULLDOWN,
    /** @brief Enter button GPIO */
    GPIO_BTN_ENTER_MUX  = INPUT_PULLDOWN,
    /** @brief Back button GPIO */
    GPIO_BTN_BACK_MUX   = INPUT_PULLDOWN,
} EGPIOPull;
#else
#warning "Compiling for debug board only!!!"
/** @brief Defines the GPIO routing for the EConBadge */
typedef enum
{
    /** @brief Back button GPIO */
    GPIO_BTN_BACK   = GPIO_NUM_0,
    /** @brief Down button GPIO */
    GPIO_BTN_DOWN   = GPIO_NUM_2,
    /** @brief Enter button GPIO */
    GPIO_BTN_ENTER  = GPIO_NUM_4,
    /** @brief Up button GPIO */
    GPIO_BTN_UP     = GPIO_NUM_5,
    /** @brief SD card MISO GPIO */
    GPIO_SD_MISO    = GPIO_NUM_13,
    /** @brief SD card CS GPIO */
    GPIO_SD_CS      = GPIO_NUM_14,
    /** @brief EInk DIN GPIO */
    GPIO_EINK_DIN   = GPIO_NUM_15,
    /** @brief EInk CLK GPIO */
    GPIO_EINK_CLK   = GPIO_NUM_16,
    /** @brief EInk CS GPIO */
    GPIO_EINK_CS    = GPIO_NUM_17,
    /** @brief EInk DC GPIO */
    GPIO_EINK_DC    = GPIO_NUM_18,
    /** @brief EInk reset GPIO */
    GPIO_EINK_RESET = GPIO_NUM_19,
    /** @brief OLED I2C SCL GPIO */
    GPIO_OLED_SCL   = GPIO_NUM_21,
    /** @brief OLED I2C SDA GPIO */
    GPIO_OLED_SDA   = GPIO_NUM_22,
    /** @brief EInk BUSY GPIO */
    GPIO_EINK_BUSY  = GPIO_NUM_23,
    /** @brief LED border ENABLE GPIO */
    GPIO_LED_ENABLE = GPIO_NUM_25,
    /** @brief SD card MOSI GPIO */
    GPIO_SD_MOSI    = GPIO_NUM_26,
    /** @brief SD card CLK GPIO */
    GPIO_SD_CLK     = GPIO_NUM_27,
    /** @brief LED border DATA GPIO */
    GPIO_LED_DATA   = GPIO_NUM_32,
    /** @brief ADC battery GPIO */
    GPIO_ADC_BAT    = GPIO_NUM_34
} EGPIORouting;

/** Define the GPIO pinmux behavior */
typedef enum 
{
    /** @brief Up button GPIO */
    GPIO_BTN_UP_MUX     = INPUT_PULLDOWN,
    /** @brief Back button GPIO */
    GPIO_BTN_DOWN_MUX   = INPUT_PULLDOWN,
    /** @brief Enter button GPIO */
    GPIO_BTN_ENTER_MUX  = INPUT_PULLDOWN,
    /** @brief Back button GPIO */
    GPIO_BTN_BACK_MUX   = INPUT_PULLDOWN,
} EGPIOPull;
#endif

typedef enum
{
   CMD_PING                       = 0,
   CMD_SET_BT_TOKEN               = 1,

   CMD_EINK_CLEAR                 = 2,
   CMD_EINK_NEW_IMAGE             = 3,
   CMD_EINK_REMOVE_IMAGE          = 4,
   CMD_EINK_SELECT_IMAGE          = 5,
   CMD_EINK_GET_CURRENT_IMG_NAME  = 6,
   CMD_EINK_GET_IMAGE_DATA        = 7,
   CMD_EINK_GET_IMAGE_LIST        = 8,

   CMD_FACTORY_RESET              = 9,

   CMD_SET_OWNER                  = 10,
   CMD_SET_CONTACT                = 11,
   CMD_GET_OWNER                  = 12,
   CMD_GET_CONTACT                = 13,

   CMD_FIRMWARE_UPDATE            = 14,

   CMD_LEDBORDER_SET_ENABLE       = 15,
   CMD_LEDBORDER_GET_ENABLE       = 16,
   CMD_LEDBORDER_INC_BRIGHTNESS   = 17,
   CMD_LEDBORDER_DEC_BRIGHTNESS   = 18,
   CMD_LEDBORDER_SET_BRIGHTNESS   = 19,
   CMD_LEDBORDER_GET_BRIGHTNESS   = 20,
   CMD_LEDBORDER_CLEAR            = 21,
   CMD_LEDBORDER_ADD_PATTERN      = 22,
   CMD_LEDBORDER_REMOVE_PATTERN   = 23,
   CMD_LEDBORDER_CLEAR_PATTERNS   = 24,
   CMD_LEDBORDER_GET_PATTERNS     = 25,
   CMD_LEDBORDER_ADD_ANIMATION    = 26,
   CMD_LEDBORDER_REMOVE_ANIMATION = 27,
   CMD_LEDBORDER_CLEAR_ANIMATIONS = 28,
   CMD_LEDBORDER_GET_ANIMATIONS   = 29,

   MAX_COMMAND_TYPE               = 30,
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