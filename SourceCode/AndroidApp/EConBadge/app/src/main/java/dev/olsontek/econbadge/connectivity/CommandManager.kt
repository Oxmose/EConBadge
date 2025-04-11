package dev.olsontek.econbadge.connectivity

import android.content.Context
import android.util.Log
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.EInkImage
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.sync.Semaphore
import java.nio.ByteBuffer

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "CommandManager"

/* Timeout in milliseconds of the update BLE operations */
private const val UPDATE_TIMEOUT_MS = 500000

/* General command timeout in milliseconds */
private const val COMMAND_TIMEOUT_MS = 5000

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class CommandManager(private val bleManager: ECBBluetoothManager, private val context: Context) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     ***********************************************************************************************/

    /* Command manager errors */
    enum class CommandStatus {
        SUCCESS,
        TIMED_OUT,
        COMM_ERROR,
        NOT_CONNECTED,
        INVALID_TOKEN,
        UNKNOWN,
        HW_NOT_COMPATIBLE,
        NO_UPDATE_AVAILABLE,
        DOWNLOADING_UPDATE,
        SENDING_UPDATE,
        CORRUPTED_DATA,
        INVALID_PARAM,
        ACTION_FAILED,
        NOT_INITIALIZED,
        NO_ACTION,
        INVALID_COMMAND_SIZE,
        FILE_NOT_FOUND,
        NO_MORE_MEMORY,
        INVALID_COMMAND_REQ,
        MAX_COMMAND_REACHED,
        IMG_NAME_UPDATE_FAIL,
        OPEN_FILE_FAILED,
        WRITE_FILE_FAILED,
        READ_FILE_FAILED,
        TRANS_SEND_FAILED,
        TRANS_RECV_FAILED,
        DATA_TOO_LONG,
        INVALID_INDEX,
        OVERLAPPING_PATTERNS;
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
                return if (status < MAX_ERROR.value) {
                    entries[status.toInt()]
                } else {
                    MAX_ERROR
                }
            }
        }
    }

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* Defines the different commands that can be sent to the ECB */
    private enum class CommandType(val value: Int) {
        CMD_PING(0),
        CMD_SET_BT_TOKEN(1),

        CMD_EINK_CLEAR(2),
        CMD_EINK_NEW_IMAGE(3),
        CMD_EINK_REMOVE_IMAGE(4),
        CMD_EINK_SELECT_IMAGE(5),
        CMD_EINK_GET_CURRENT_IMG_NAME(6),
        CMD_EINK_GET_IMAGE_DATA(7),
        CMD_EINK_GET_IMAGE_LIST(8),

        CMD_FACTORY_RESET(9),

        CMD_SET_OWNER(10),
        CMD_SET_CONTACT(11),
        CMD_GET_OWNER(12),
        CMD_GET_CONTACT(13),

        CMD_FIRMWARE_UPDATE(14),

        CMD_LEDBORDER_SET_ENABLE(15),
        CMD_LEDBORDER_GET_ENABLE(16),
        CMD_LEDBORDER_INC_BRIGHTNESS(17),
        CMD_LEDBORDER_DEC_BRIGHTNESS(18),
        CMD_LEDBORDER_SET_BRIGHTNESS(19),
        CMD_LEDBORDER_GET_BRIGHTNESS(20),
        CMD_LEDBORDER_CLEAR(21),
        CMD_LEDBORDER_ADD_PATTERN(22),
        CMD_LEDBORDER_REMOVE_PATTERN(23),
        CMD_LEDBORDER_CLEAR_PATTERNS(24),
        CMD_LEDBORDER_GET_PATTERNS(25),
        CMD_LEDBORDER_ADD_ANIMATION(26),
        CMD_LEDBORDER_REMOVE_ANIMATION(27),
        CMD_LEDBORDER_CLEAR_ANIMATIONS(28),
        CMD_LEDBORDER_GET_ANIMATIONS(29),

        MAX_COMMAND_TYPE(30);
    }

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun manageBleError(error: ECBBluetoothManager.ECBBleError): CommandStatus {
        /* Maps the BLE errors to the command errors */
        when (error) {
            ECBBluetoothManager.ECBBleError.TIMED_OUT -> {
                return CommandStatus.TIMED_OUT
            }

            ECBBluetoothManager.ECBBleError.WRITE_FAILED,
            ECBBluetoothManager.ECBBleError.READ_FAILED -> {
                return CommandStatus.COMM_ERROR
            }

            ECBBluetoothManager.ECBBleError.NOT_CONNECTED -> {
                return CommandStatus.NOT_CONNECTED
            }

            ECBBluetoothManager.ECBBleError.INVALID_TOKEN -> {
                return CommandStatus.INVALID_TOKEN
            }

            else -> {
                return CommandStatus.UNKNOWN
            }
        }
    }

    private fun getECBStatus(response: ByteArray?): CommandStatus {
        if (response == null || response.isEmpty()) {
            return CommandStatus.COMM_ERROR
        }

        when (ECBCommandStatus.fromByte(response[0])) {
            ECBCommandStatus.NO_ERROR -> {
                return CommandStatus.SUCCESS
            }

            ECBCommandStatus.INVALID_PARAM -> {
                return CommandStatus.INVALID_PARAM
            }

            ECBCommandStatus.ACTION_FAILED -> {
                return CommandStatus.ACTION_FAILED
            }

            ECBCommandStatus.NOT_INITIALIZED -> {
                return CommandStatus.NOT_INITIALIZED
            }

            ECBCommandStatus.NO_ACTION -> {
                return CommandStatus.NO_ACTION
            }

            ECBCommandStatus.INVALID_TOKEN -> {
                return CommandStatus.INVALID_TOKEN
            }

            ECBCommandStatus.INVALID_COMMAND_SIZE -> {
                return CommandStatus.INVALID_COMMAND_SIZE
            }

            ECBCommandStatus.FILE_NOT_FOUND -> {
                return CommandStatus.FILE_NOT_FOUND
            }

            ECBCommandStatus.NO_MORE_MEMORY -> {
                return CommandStatus.NO_MORE_MEMORY
            }

            ECBCommandStatus.INVALID_COMMAND_REQ -> {
                return CommandStatus.INVALID_COMMAND_REQ
            }

            ECBCommandStatus.MAX_COMMAND_REACHED -> {
                return CommandStatus.MAX_COMMAND_REACHED
            }

            ECBCommandStatus.IMG_NAME_UPDATE_FAIL -> {
                return CommandStatus.IMG_NAME_UPDATE_FAIL
            }

            ECBCommandStatus.OPEN_FILE_FAILED -> {
                return CommandStatus.OPEN_FILE_FAILED
            }

            ECBCommandStatus.WRITE_FILE_FAILED -> {
                return CommandStatus.WRITE_FILE_FAILED
            }

            ECBCommandStatus.READ_FILE_FAILED -> {
                return CommandStatus.READ_FILE_FAILED
            }

            ECBCommandStatus.TRANS_SEND_FAILED -> {
                return CommandStatus.TRANS_SEND_FAILED
            }

            ECBCommandStatus.TRANS_RECV_FAILED -> {
                return CommandStatus.TRANS_RECV_FAILED
            }

            ECBCommandStatus.DATA_TOO_LONG -> {
                return CommandStatus.DATA_TOO_LONG
            }

            ECBCommandStatus.CORRUPTED_DATA -> {
                return CommandStatus.CORRUPTED_DATA
            }

            ECBCommandStatus.INVALID_INDEX -> {
                return CommandStatus.INVALID_INDEX
            }

            ECBCommandStatus.OVERLAPPING_PATTERNS -> {
                return CommandStatus.OVERLAPPING_PATTERNS
            }

            else -> {
                return CommandStatus.UNKNOWN
            }
        }
    }

    private fun baseWaitCommand(
        status: ECBBluetoothManager.ECBBleError,
        response: ByteArray?,
        commandId: Int,
        callback: (CommandStatus, ByteArray, Int) -> Unit
    ) {
        when (status) {
            ECBBluetoothManager.ECBBleError.SUCCESS -> {
                when (val commandStatus = getECBStatus(response)) {
                    CommandStatus.SUCCESS -> {
                        /* Check the response size */
                        if (response!!.size < 2 || response[1].toInt() != response.size - 2) {
                            callback(CommandStatus.COMM_ERROR, ByteArray(0), commandId)
                        } else {
                            callback(
                                commandStatus,
                                response.sliceArray(2..<response.size),
                                commandId
                            )
                        }
                    }

                    else -> {
                        callback(commandStatus, ByteArray(0), commandId)
                    }
                }
            }

            else -> {
                callback(manageBleError(status), ByteArray(0), commandId)
            }
        }
    }

    private fun baseSendCommand(
        commandType: CommandType,
        commandData: ByteArray,
        callback: (CommandStatus, ByteArray, Int) -> Unit
    ) {
        Log.d(MODULE_NAME, "Sending command $commandType.")
        bleManager.sendCommand(
            commandType.value,
            commandData,
            callback = { status: ECBBluetoothManager.ECBBleError, response: ByteArray?, commandId: Int ->
                Log.d(
                    MODULE_NAME,
                    "Command $commandType response status: $status | Command $commandId."
                )
                when (status) {
                    ECBBluetoothManager.ECBBleError.SUCCESS -> {
                        when (val commandStatus = getECBStatus(response)) {
                            CommandStatus.SUCCESS -> {
                                /* Check the response size */
                                if (response!!.size < 2 || response[1].toInt() != response.size - 2) {
                                    callback(CommandStatus.COMM_ERROR, ByteArray(0), commandId)
                                } else {
                                    callback(
                                        commandStatus,
                                        response.sliceArray(2..<response.size),
                                        commandId
                                    )
                                }
                            }

                            else -> {
                                callback(commandStatus, ByteArray(0), commandId)
                            }
                        }
                    }

                    else -> {
                        callback(manageBleError(status), ByteArray(0), commandId)
                    }
                }
            },
            COMMAND_TIMEOUT_MS
        )
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    fun sendPing(callback: (CommandStatus, String) -> Unit) {
        baseSendCommand(
            CommandType.CMD_PING,
            ByteArray(0)
        ) { commandStatus: CommandStatus, response: ByteArray?, _: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    callback(commandStatus, response!!.toString(Charsets.US_ASCII))
                }

                else -> {
                    callback(commandStatus, "")
                }
            }
        }
    }

    fun requestTokenChange(token: String, callback: (CommandStatus) -> Unit) {
        baseSendCommand(
            CommandType.CMD_SET_BT_TOKEN,
            token.toByteArray(Charsets.US_ASCII)
        ) { commandStatus: CommandStatus, _: ByteArray?, _: Int ->
            callback(commandStatus)
        }
    }

    fun getSoftwareVersion(callback: (CommandStatus, String) -> Unit) {
        Log.d(MODULE_NAME, "Getting software version")
        bleManager.readSoftwareVersion(
            callback = { status: ECBBluetoothManager.ECBBleError, response: ByteArray?, commandId: Int ->
                Log.d(MODULE_NAME, "Software version status: $status | Command $commandId.")
                when (status) {
                    ECBBluetoothManager.ECBBleError.SUCCESS -> {
                        callback(CommandStatus.SUCCESS, response!!.toString(Charsets.US_ASCII))
                    }

                    else -> {
                        callback(manageBleError(status), "")
                    }
                }
            },
            COMMAND_TIMEOUT_MS
        )
    }

    fun getHardwareVersion(callback: (CommandStatus, String) -> Unit) {
        Log.d(MODULE_NAME, "Getting hardware version")
        bleManager.readHardwareVersion(
            callback = { status: ECBBluetoothManager.ECBBleError, response: ByteArray?, commandId: Int ->
                Log.d(MODULE_NAME, "Hardware version status: $status | Command $commandId.")
                when (status) {
                    ECBBluetoothManager.ECBBleError.SUCCESS -> {
                        callback(CommandStatus.SUCCESS, response!!.toString(Charsets.US_ASCII))
                    }

                    else -> {
                        callback(manageBleError(status), "")
                    }
                }
            },
            COMMAND_TIMEOUT_MS
        )
    }

    fun requestOwnerChange(value: String, callback: (CommandStatus) -> Unit) {
        baseSendCommand(
            CommandType.CMD_SET_OWNER,
            value.toByteArray(Charsets.US_ASCII)
        ) { commandStatus: CommandStatus, _: ByteArray?, _: Int ->
            callback(commandStatus)
        }
    }

    fun requestContactChange(value: String, callback: (CommandStatus) -> Unit) {
        baseSendCommand(
            CommandType.CMD_SET_CONTACT,
            value.toByteArray(Charsets.US_ASCII)
        ) { commandStatus: CommandStatus, _: ByteArray?, _: Int ->
            callback(commandStatus)
        }
    }

    fun getOwnerInfo(callback: (CommandStatus, String) -> Unit) {
        baseSendCommand(
            CommandType.CMD_GET_OWNER,
            ByteArray(0)
        ) { commandStatus: CommandStatus, response: ByteArray?, _: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    callback(commandStatus, response!!.toString(Charsets.US_ASCII))
                }

                else -> {
                    callback(commandStatus, "")
                }
            }
        }
    }

    fun getContactInfo(callback: (CommandStatus, String) -> Unit) {
        baseSendCommand(
            CommandType.CMD_GET_CONTACT,
            ByteArray(0)
        ) { commandStatus: CommandStatus, response: ByteArray?, _: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    callback(commandStatus, response!!.toString(Charsets.US_ASCII))
                }

                else -> {
                    callback(commandStatus, "")
                }
            }
        }
    }

    fun requestFactoryReset(callback: (CommandStatus) -> Unit) {
        baseSendCommand(
            CommandType.CMD_FACTORY_RESET,
            ByteArray(0)
        ) { commandStatus: CommandStatus, _: ByteArray?, _: Int ->
            callback(commandStatus)
        }
    }

    fun requestUpdate(callback: (CommandStatus, Int) -> Unit) {
        baseSendCommand(
            CommandType.CMD_FIRMWARE_UPDATE,
            ByteArray(0)
        ) { commandStatus: CommandStatus, _: ByteArray?, commandId: Int ->
            callback(commandStatus, commandId)
        }
    }

    fun sendUpdateHeader(
        header: ByteArray,
        updateIdentifier: Int,
        callback: (CommandStatus) -> Unit
    ) {
        Log.d(MODULE_NAME, "Sending update header")

        val dataSendSem = Semaphore(1, 1)

        /* Wait for end of update command response */
        bleManager.waitCommandResponse(
            updateIdentifier,
            callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                         response: ByteArray?,
                         commandId: Int ->

                baseWaitCommand(responseStatus, response, commandId) { finalStatus: CommandStatus,
                                                                       _: ByteArray?,
                                                                       _: Int ->

                    Log.d(MODULE_NAME, "Sending update header status wait: $finalStatus.")

                    /* Ensure data send is done */
                    if (finalStatus == CommandStatus.SUCCESS) {
                        runBlocking {
                            launch {
                                dataSendSem.acquire()
                            }
                        }
                    }

                    callback(finalStatus)
                }
            },
            UPDATE_TIMEOUT_MS
        )

        /* Send header */
        bleManager.sendData(
            header,
            progressCallback = null,
            sendDataEndCallback = { status: ECBBluetoothManager.ECBBleError ->
                Log.d(MODULE_NAME, "Sending update header status: $status.")

                if (status != ECBBluetoothManager.ECBBleError.SUCCESS) {
                    bleManager.cancelWaitCommandResponse(updateIdentifier)
                    callback(manageBleError(status))
                } else {
                    dataSendSem.release()
                }
            },
            UPDATE_TIMEOUT_MS
        )

    }

    fun sendUpdateData(
        firmwareBinary: ByteArray,
        updateIdentifier: Int,
        processCallback: (Float) -> Unit,
        sendEndCallback: (CommandStatus) -> Unit
    ) {
        Log.d(MODULE_NAME, "Sending firmware update")

        val dataSendSem = Semaphore(1, 1)

        /* Wait for end of update command response */
        bleManager.waitCommandResponse(
            updateIdentifier,
            callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                         response: ByteArray?,
                         commandId: Int ->

                baseWaitCommand(responseStatus, response, commandId) { finalStatus: CommandStatus,
                                                                       _: ByteArray?,
                                                                       _: Int ->

                    /* Ensure data send is done */
                    if (finalStatus == CommandStatus.SUCCESS) {
                        runBlocking {
                            launch {
                                dataSendSem.acquire()
                            }
                        }
                    }

                    sendEndCallback(finalStatus)
                }
            },
            UPDATE_TIMEOUT_MS
        )

        /* Send firmware data */
        bleManager.sendData(
            firmwareBinary,
            progressCallback = processCallback,
            sendDataEndCallback = { status: ECBBluetoothManager.ECBBleError ->
                Log.d(MODULE_NAME, "Sending update status: $status.")

                if (status != ECBBluetoothManager.ECBBleError.SUCCESS) {
                    bleManager.cancelWaitCommandResponse(updateIdentifier)
                    sendEndCallback(manageBleError(status))
                } else {
                    dataSendSem.release()
                }
            },
            UPDATE_TIMEOUT_MS
        )
    }

    fun getCurrentImageName(callback: (CommandStatus, String) -> Unit) {
        baseSendCommand(
            CommandType.CMD_EINK_GET_CURRENT_IMG_NAME,
            ByteArray(0)
        ) { commandStatus: CommandStatus, response: ByteArray?, _: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    callback(commandStatus, response!!.toString(Charsets.US_ASCII))
                }

                else -> {
                    callback(commandStatus, "")
                }
            }
        }
    }

    fun getImageData(imageName: String, callback: (CommandStatus, ByteArray?) -> Unit) {

        Log.d(MODULE_NAME, "Getting image data")

        val dataReceiveSem = Semaphore(1, 1)
        var receivedData: ByteArray? = null

        baseSendCommand(
            CommandType.CMD_EINK_GET_IMAGE_DATA,
            imageName.toByteArray(Charsets.US_ASCII)
        ) { commandStatus: CommandStatus, _: ByteArray?, dataGetIdentifier: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    /* Wait for end of data receive command response */
                    bleManager.waitCommandResponse(
                        dataGetIdentifier,
                        callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                     dataResponse: ByteArray?,
                                     commandId: Int ->

                            baseWaitCommand(
                                responseStatus,
                                dataResponse,
                                commandId
                            ) { finalStatus: CommandStatus,
                                _: ByteArray?,
                                _: Int ->

                                /* Ensure data send is done */
                                if (finalStatus == CommandStatus.SUCCESS) {
                                    runBlocking {
                                        launch {
                                            dataReceiveSem.acquire()
                                        }
                                    }
                                }

                                callback(finalStatus, receivedData)
                            }
                        },
                        UPDATE_TIMEOUT_MS
                    )

                    /* Receive the data */
                    bleManager.receiveData(
                        EInkImage.getECBDataSize(),
                        progressCallback = null,
                        receiveDataEndCallback = { statusRecv: ECBBluetoothManager.ECBBleError, dataRecv: ByteArray? ->
                            when (statusRecv) {
                                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                                    receivedData = dataRecv
                                    dataReceiveSem.release()
                                }

                                else -> {
                                    bleManager.cancelWaitCommandResponse(dataGetIdentifier)
                                    callback(manageBleError(statusRecv), null)
                                }
                            }
                        },
                        UPDATE_TIMEOUT_MS,
                    )
                }

                else -> {
                    callback(commandStatus, null)
                }
            }
        }
    }

    fun clearEInkDisplay(callback: (CommandStatus) -> Unit) {
        baseSendCommand(
            CommandType.CMD_EINK_CLEAR,
            ByteArray(0)
        ) { commandStatus: CommandStatus, _: ByteArray?, commandId: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    bleManager.waitCommandResponse(
                        commandId,
                        callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                     waitResponse: ByteArray?,
                                     waitCommandId: Int ->
                            baseWaitCommand(
                                responseStatus,
                                waitResponse,
                                waitCommandId
                            ) { finalStatus: CommandStatus,
                                _: ByteArray?,
                                _: Int ->

                                callback(finalStatus)
                            }
                        },
                        UPDATE_TIMEOUT_MS
                    )
                }

                else -> {
                    callback(commandStatus)
                }
            }
        }
    }

    fun retrieveImageList(callback: (CommandStatus, List<String>) -> Unit) {
        Log.d(MODULE_NAME, "Getting images list")

        val dataReceiveSem = Semaphore(1, 1)
        var receivedData: ByteArray? = null

        baseSendCommand(
            CommandType.CMD_EINK_GET_IMAGE_LIST,
            ByteArray(0)
        ) { commandStatus: CommandStatus, imageCountRaw: ByteArray?, dataGetIdentifier: Int ->
            when (commandStatus) {
                CommandStatus.SUCCESS -> {
                    if (imageCountRaw == null) {
                        callback(CommandStatus.COMM_ERROR, ArrayList())
                        return@baseSendCommand
                    }

                    val imagesCount = ByteBuffer.wrap(imageCountRaw).getInt()
                    if (imagesCount <= 0) {
                        callback(commandStatus, ArrayList())
                        return@baseSendCommand
                    }

                    /* Wait for end of data receive command response */
                    bleManager.waitCommandResponse(
                        dataGetIdentifier,
                        callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                     dataResponse: ByteArray?,
                                     commandId: Int ->

                            baseWaitCommand(
                                responseStatus,
                                dataResponse,
                                commandId
                            ) { finalStatus: CommandStatus,
                                _: ByteArray?,
                                _: Int ->

                                val imageList = ArrayList<String>()

                                /* Ensure data send is done */
                                if (finalStatus == CommandStatus.SUCCESS) {
                                    runBlocking {
                                        launch {
                                            dataReceiveSem.acquire()

                                            if (receivedData == null) {
                                                return@launch
                                            }

                                            /* Transform the data */
                                            var imageName = ByteArray(0)
                                            for (dataByte: Byte in receivedData!!) {
                                                if (dataByte.compareTo(0) == 0) {
                                                    imageList.add(
                                                        imageName.toString(Charsets.US_ASCII)
                                                    )
                                                    imageName = ByteArray(0)
                                                } else {
                                                    imageName += dataByte
                                                }
                                            }
                                        }
                                    }
                                }

                                callback(finalStatus, imageList)
                            }
                        },
                        UPDATE_TIMEOUT_MS
                    )

                    /* Receive the data */
                    bleManager.receiveData(
                        -1,
                        progressCallback = null,
                        receiveDataEndCallback = { statusRecv: ECBBluetoothManager.ECBBleError, dataRecv: ByteArray? ->
                            when (statusRecv) {
                                ECBBluetoothManager.ECBBleError.SUCCESS -> {
                                    receivedData = dataRecv
                                    dataReceiveSem.release()
                                }

                                else -> {
                                    bleManager.cancelWaitCommandResponse(dataGetIdentifier)
                                    callback(manageBleError(statusRecv), ArrayList())
                                }
                            }
                        },
                        UPDATE_TIMEOUT_MS,
                    )
                }

                else -> {
                    callback(commandStatus, ArrayList())
                }
            }
        }
    }

    fun setEInkImage(imageFileName: String, callback: (CommandStatus) -> Boolean) {
        baseSendCommand(
            CommandType.CMD_EINK_SELECT_IMAGE,
            imageFileName.toByteArray(Charsets.US_ASCII) + 0x00
        ) { commandStatus: CommandStatus, _: ByteArray?, commandId: Int ->
            if (commandStatus == CommandStatus.SUCCESS) {
                bleManager.waitCommandResponse(
                    commandId,
                    callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                 waitResponse: ByteArray?,
                                 waitCommandId: Int ->
                        baseWaitCommand(
                            responseStatus,
                            waitResponse,
                            waitCommandId
                        ) { finalStatus: CommandStatus,
                            _: ByteArray?,
                            _: Int ->

                            callback(finalStatus)
                        }
                    },
                    UPDATE_TIMEOUT_MS
                )
            } else {
                callback(commandStatus)
            }
        }
    }

    fun removeEInkImage(
        imageFileName: String,
        isCurrent: Boolean,
        callback: (CommandStatus) -> Boolean
    ) {
        baseSendCommand(
            CommandType.CMD_EINK_REMOVE_IMAGE,
            imageFileName.toByteArray(Charsets.US_ASCII) + 0x00
        ) { commandStatus: CommandStatus, _: ByteArray?, commandId: Int ->
            if (commandStatus == CommandStatus.SUCCESS) {
                /* Wait clear if current */
                if (isCurrent) {
                    bleManager.waitCommandResponse(
                        commandId,
                        callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                     waitResponse: ByteArray?,
                                     waitCommandId: Int ->
                            baseWaitCommand(
                                responseStatus,
                                waitResponse,
                                waitCommandId
                            ) { finalStatus: CommandStatus,
                                _: ByteArray?,
                                _: Int ->

                                callback(finalStatus)
                            }
                        },
                        UPDATE_TIMEOUT_MS
                    )
                } else {
                    callback(commandStatus)
                }
            } else {
                callback(commandStatus)
            }
        }
    }

    fun sendEInkImage(
        imageFileName: String,
        imageData: ByteArray,
        callback: (CommandStatus) -> Boolean
    ) {

        val dataSendSem = Semaphore(1, 1)
        baseSendCommand(
            CommandType.CMD_EINK_NEW_IMAGE,
            imageFileName.toByteArray(Charsets.US_ASCII) + 0x00
        ) { commandStatus: CommandStatus, _: ByteArray?, commandId: Int ->
            if (commandStatus == CommandStatus.SUCCESS) {
                bleManager.waitCommandResponse(
                    commandId,
                    callback = { responseStatus: ECBBluetoothManager.ECBBleError,
                                 waitResponse: ByteArray?,
                                 waitCommandId: Int ->
                        baseWaitCommand(
                            responseStatus,
                            waitResponse,
                            waitCommandId
                        ) { finalStatus: CommandStatus,
                            _: ByteArray?,
                            _: Int ->
                            /* Wait for data to be sent and callback */
                            runBlocking {
                                launch {
                                    dataSendSem.acquire()
                                }
                            }
                            callback(finalStatus)
                        }
                    },
                    UPDATE_TIMEOUT_MS
                )

                bleManager.sendData(
                    imageData,
                    progressCallback = null,
                    sendDataEndCallback = { status: ECBBluetoothManager.ECBBleError ->
                        Log.d(MODULE_NAME, "Sending image: $status.")

                        if (status != ECBBluetoothManager.ECBBleError.SUCCESS) {
                            bleManager.cancelWaitCommandResponse(commandId)
                            callback(manageBleError(status))
                        } else {
                            dataSendSem.release()
                        }
                    },
                    UPDATE_TIMEOUT_MS
                )
            } else {
                callback(commandStatus)
            }
        }
    }

    fun getStringError(error: CommandStatus): String {
        when (error) {
            CommandStatus.SUCCESS -> {
                return context.getString(R.string.success)
            }

            CommandStatus.TIMED_OUT -> {
                return context.getString(R.string.request_timed_out)
            }

            CommandStatus.COMM_ERROR -> {
                return context.getString(R.string.communication_error)
            }

            CommandStatus.NOT_CONNECTED -> {
                return context.getString(R.string.device_not_connected)
            }

            CommandStatus.INVALID_TOKEN -> {
                return context.getString(R.string.invalid_token_provided)
            }

            CommandStatus.INVALID_PARAM -> {
                return context.getString(R.string.invalid_command_parameter)
            }

            CommandStatus.ACTION_FAILED -> {
                return context.getString(R.string.action_failed)
            }

            CommandStatus.NOT_INITIALIZED -> {
                return context.getString(R.string.missing_initialization)
            }

            CommandStatus.NO_ACTION -> {
                return context.getString(R.string.no_action)
            }

            CommandStatus.INVALID_COMMAND_SIZE -> {
                return context.getString(R.string.invalid_command_size)
            }

            CommandStatus.FILE_NOT_FOUND -> {
                return context.getString(R.string.file_not_found)
            }

            CommandStatus.NO_MORE_MEMORY -> {
                return context.getString(R.string.no_more_memory)
            }

            CommandStatus.INVALID_COMMAND_REQ -> {
                return context.getString(R.string.invalid_command_request)
            }

            CommandStatus.MAX_COMMAND_REACHED -> {
                return context.getString(R.string.maximal_number_of_command_reached)
            }

            CommandStatus.IMG_NAME_UPDATE_FAIL -> {
                return context.getString(R.string.failed_to_update_the_image_name)
            }

            CommandStatus.OPEN_FILE_FAILED -> {
                return context.getString(R.string.failed_to_open_file)
            }

            CommandStatus.WRITE_FILE_FAILED -> {
                return context.getString(R.string.failed_to_write_file)
            }

            CommandStatus.READ_FILE_FAILED -> {
                return context.getString(R.string.failed_to_read_file)
            }

            CommandStatus.TRANS_SEND_FAILED -> {
                return context.getString(R.string.sending_failed)
            }

            CommandStatus.TRANS_RECV_FAILED -> {
                return context.getString(R.string.receiving_failed)
            }

            CommandStatus.DATA_TOO_LONG -> {
                return context.getString(R.string.data_length_too_big)
            }

            CommandStatus.CORRUPTED_DATA -> {
                return context.getString(R.string.corrupted_data)
            }

            CommandStatus.INVALID_INDEX -> {
                return context.getString(R.string.invalid_index)
            }

            CommandStatus.OVERLAPPING_PATTERNS -> {
                return context.getString(R.string.patterns_are_overlapping)
            }

            else -> {
                return context.getString(R.string.unknown_error)
            }
        }
    }
}