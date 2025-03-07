package dev.olsontek.econbadge.connectivity

import android.content.Context
import android.content.res.Resources
import android.util.Log
import dev.olsontek.econbadge.R

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "CommandManager"

private const val MAX_OWNER_CONTACT_SIZE = 41

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class CommandManager(private val bleManager: ECBBluetoothManager, private val context: Context) {

/***************************************************************************************************
 * PUBLIC TYPES AND ENUMERATIONS
 **************************************************************************************************/

    /* Command manager errors */
    enum class CommandError {
        SUCCESS,
        TIMED_OUT,
        COMM_ERROR,
        NOT_CONNECTED,
        INVALID_TOKEN,
        UNKNOWN,
        HW_NOT_COMPATIBLE,
        NO_UPDATE_AVAILABLE;
    }

    /* ECB command result status */
    enum class ECBCommandStatus(val value: Byte) {
        NO_ERROR(0),
        INVALID_PARAM(1),
        ACTION_FAILED(2),
        NOT_INITIALIZED(3),
        NO_ACTION(4),
        INVALID_TOKEN(5),
        INVALID_COMMAND_SIZE(6),
        FILE_NOT_FOUND(7),
        NO_MORE_MEMORY(8),
        INVALID_COMMAND_REQ(9),
        MAX_COMMAND_REACHED(10),
        IMG_NAME_UPDATE_FAIL(11),
        OPEN_FILE_FAILED(12),
        WRITE_FILE_FAILED(13),
        READ_FILE_FAILED(14),
        TRANS_SEND_FAILED(15),
        TRANS_RECV_FAILED(16),
        DATA_TOO_LONG(17),
        CORRUPTED_DATA(18),
        INVALID_INDEX(19),
        OVERLAPPING_PATTERNS(20),
        MAX_ERROR(21);

        companion object {
            fun fromByte(status: Byte): ECBCommandStatus {
                if (status < MAX_ERROR.value) {
                    return entries[status.toInt()]
                }
                else {
                    return MAX_ERROR
                }
            }
        }
    }

    /* eConBadge response formated for further usage */
    class ECBResponse(responseStatus: Byte, responseValue: ByteArray) {
        var status: Byte
            private set

        var response: ByteArray
            private set

        init {
            status = responseStatus
            response = responseValue
        }
    }

    /***************************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **************************************************************************************************/
    /* Defines the different commands that can be sent to the ECB */
    private enum class CommandType(val value: Int) {
        CMD_PING(0),
        CMD_SET_BT_TOKEN(1),

        CMD_EINK_CLEAR(2),
        CMD_EINK_NEW_IMAGE(3),
        CMD_EINK_REMOVE_IMAGE(4),
        CMD_EINK_SELECT_IMAGE(5),
        CMD_EINK_GET_CURRENT_IMG_NAME(6),
        CMD_EINK_GET_CURRENT_IMG(7),

        CMD_FACTORY_RESET(8),

        CMD_SET_OWNER(9),
        CMD_SET_CONTACT(10),
        CMD_GET_OWNER(11),
        CMD_GET_CONTACT(12),

        CMD_FIRMWARE_UPDATE(13),

        CMD_LEDBORDER_SET_ENABLE(14),
        CMD_LEDBORDER_GET_ENABLE(15),
        CMD_LEDBORDER_INC_BRIGHTNESS(16),
        CMD_LEDBORDER_DEC_BRIGHTNESS(17),
        CMD_LEDBORDER_SET_BRIGHTNESS(18),
        CMD_LEDBORDER_GET_BRIGHTNESS(19),
        CMD_LEDBORDER_CLEAR(20),
        CMD_LEDBORDER_ADD_PATTERN(21),
        CMD_LEDBORDER_REMOVE_PATTERN(22),
        CMD_LEDBORDER_CLEAR_PATTERNS(23),
        CMD_LEDBORDER_GET_PATTERNS(24),
        CMD_LEDBORDER_ADD_ANIMATION(25),
        CMD_LEDBORDER_REMOVE_ANIMATION(26),
        CMD_LEDBORDER_CLEAR_ANIMATIONS(27),
        CMD_LEDBORDER_GET_ANIMATIONS(28),

        MAX_COMMAND_TYPE(29);
    }

    /***************************************************************************************************
     * PRIVATE ATTRIBUTES
     **************************************************************************************************/
    /* None */

    /***************************************************************************************************
     * PRIVATE METHODS
     **************************************************************************************************/
    private fun manageBleError(error: ECBBluetoothManager.ECBBleError): CommandError {
        Log.d(MODULE_NAME, "Error: $error")

        /* Maps the BLE errors to the command errors */
        when (error) {
            ECBBluetoothManager.ECBBleError.TIMED_OUT -> {
                return CommandError.TIMED_OUT
            }

            ECBBluetoothManager.ECBBleError.WRITE_FAILED,
            ECBBluetoothManager.ECBBleError.READ_FAILED -> {
                return CommandError.COMM_ERROR
            }

            ECBBluetoothManager.ECBBleError.NOT_CONNECTED -> {
                return CommandError.NOT_CONNECTED
            }

            ECBBluetoothManager.ECBBleError.INVALID_TOKEN -> {
                return CommandError.INVALID_TOKEN
            }

            else -> {
                return CommandError.UNKNOWN
            }
        }
    }

    /***************************************************************************************************
     * PUBLIC METHODS
     **************************************************************************************************/
    fun sendPing(callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Sending PING command.")
        /* Send the command and manage the return value */
        bleManager.sendCommand(
            CommandType.CMD_PING.value,
            ByteArray(0)
        ) {
            status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "PING command status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }

                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun requestTokenChange(token: String, callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Sending new token command.")
        /* Send the command and manage the return value */
        bleManager.sendCommand(
            CommandType.CMD_SET_BT_TOKEN.value,
            token.toByteArray(Charsets.US_ASCII)
        ) {
            status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Set new token command status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun getSoftwareVersion(callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Getting software version")
        bleManager.readSoftwareVersion() {
            status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Software version status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun getHardwareVersion(callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Getting hardware version")
        bleManager.readHardwareVersion() {
                status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Hardware version status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun requestOwnerChange(value: String, callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Setting owner info")
        bleManager.sendCommand(
            CommandType.CMD_SET_OWNER.value,
            value.toByteArray(Charsets.US_ASCII)
        ) {
                status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Owner info set status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun requestContactChange(value: String, callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Setting contact info")
        bleManager.sendCommand(
            CommandType.CMD_SET_CONTACT.value,
            value.toByteArray(Charsets.US_ASCII)
        ) {
                status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Contact info set status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun getOwnerInfo(callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Getting owner info")
        bleManager.sendCommand(
            CommandType.CMD_GET_OWNER.value,
            ByteArray(0)
        ) {
                status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Owner info status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun getContactInfo(callback: (CommandError, ECBResponse?) -> Unit) {
        Log.d(MODULE_NAME, "Getting contact info")
        bleManager.sendCommand(
            CommandType.CMD_GET_CONTACT.value,
            ByteArray(0)
        ) {
                status: ECBBluetoothManager.ECBBleError, response: ECBResponse? ->
            Log.d(MODULE_NAME, "Owner contact status: $status.")
            when (status) {
                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                    callback(CommandError.SUCCESS, response)
                }
                else -> {
                    callback(manageBleError(status), response)
                }
            }
        }
    }

    fun validateToken(tokenString: String): Boolean {
        /* Check the token size */
        if (tokenString.length != bleManager.getTokenSize()) {
            return false
        }

        /* Check for ASCII characters only */
        if(!tokenString.matches(Regex("\\A\\p{ASCII}*\\z"))) {
            return false
        }

        return true
    }

    fun validateOwnerContact(value: String): Boolean {
        /* Check the token size */
        if (value.length > MAX_OWNER_CONTACT_SIZE) {
            return false
        }

        /* Check for ASCII characters only */
        if(!value.matches(Regex("\\A\\p{ASCII}*\\z"))) {
            return false
        }

        return true
    }

    fun getStringError(error: CommandError): String {
        when (error) {
            CommandError.SUCCESS -> {
                return context.getString(R.string.success)
            }
            CommandError.TIMED_OUT -> {
                return context.getString(R.string.request_timed_out)
            }
            CommandError.COMM_ERROR -> {
                return context.getString(R.string.communication_error)
            }
            CommandError.NOT_CONNECTED -> {
                return context.getString(R.string.device_not_connected)
            }
            CommandError.INVALID_TOKEN -> {
                return context.getString(R.string.invalid_token_provided)
            }
            else -> {
                return context.getString(R.string.unknown_error)
            }
        }
    }

    fun getStringError(error: ECBCommandStatus): String {
        when (error) {
            ECBCommandStatus.NO_ERROR -> {
                return context.getString(R.string.no_error)
            }
            ECBCommandStatus.INVALID_PARAM -> {
                return context.getString(R.string.invalid_command_parameter)
            }
            ECBCommandStatus.ACTION_FAILED -> {
                return context.getString(R.string.action_failed)
            }
            ECBCommandStatus.NOT_INITIALIZED -> {
                return context.getString(R.string.missing_initialization)
            }
            ECBCommandStatus.NO_ACTION -> {
                return context.getString(R.string.no_action)
            }
            ECBCommandStatus.INVALID_TOKEN -> {
                return context.getString(R.string.invalid_token_provided)
            }
            ECBCommandStatus.INVALID_COMMAND_SIZE -> {
                return context.getString(R.string.invalid_command_size)
            }
            ECBCommandStatus.FILE_NOT_FOUND -> {
                return context.getString(R.string.file_not_found)
            }
            ECBCommandStatus.NO_MORE_MEMORY -> {
                return context.getString(R.string.no_more_memory)
            }
            ECBCommandStatus.INVALID_COMMAND_REQ -> {
                return context.getString(R.string.invalid_command_request)
            }
            ECBCommandStatus.MAX_COMMAND_REACHED -> {
                return context.getString(R.string.maximal_number_of_command_reached)
            }
            ECBCommandStatus.IMG_NAME_UPDATE_FAIL -> {
                return context.getString(R.string.failed_to_update_the_image_name)
            }
            ECBCommandStatus.OPEN_FILE_FAILED -> {
                return context.getString(R.string.failed_to_open_file)
            }
            ECBCommandStatus.WRITE_FILE_FAILED -> {
                return context.getString(R.string.failed_to_write_file)
            }
            ECBCommandStatus.READ_FILE_FAILED -> {
                return context.getString(R.string.failed_to_read_file)
            }
            ECBCommandStatus.TRANS_SEND_FAILED -> {
                return context.getString(R.string.sending_failed)
            }
            ECBCommandStatus.TRANS_RECV_FAILED -> {
                return context.getString(R.string.receiving_failed)
            }
            ECBCommandStatus.DATA_TOO_LONG -> {
                return context.getString(R.string.data_length_too_big)
            }
            ECBCommandStatus.CORRUPTED_DATA -> {
                return context.getString(R.string.corrupted_data)
            }
            ECBCommandStatus.INVALID_INDEX -> {
                return context.getString(R.string.invalid_index)
            }
            ECBCommandStatus.OVERLAPPING_PATTERNS -> {
                return context.getString(R.string.patterns_are_overlapping)
            }
            else -> {
                return context.getString(R.string.unknown_error)
            }
        }
    }

    fun setECBToken(tokenValue: String) {
        bleManager.setECBToken(tokenValue)
    }
}