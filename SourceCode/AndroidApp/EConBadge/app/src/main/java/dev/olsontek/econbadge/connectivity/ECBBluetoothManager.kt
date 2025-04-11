package dev.olsontek.econbadge.connectivity

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothStatusCodes
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import dev.olsontek.econbadge.data.SharedData
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.Semaphore
import kotlinx.coroutines.sync.withLock
import java.util.UUID
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger
import kotlin.math.min

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "ECBBluetoothManager"

/* CCC Descriptor UUID */
private const val CCC_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805f9b34fb"

/* ECB main service UUID */
private const val ECB_SERVICE_UUID = "d3e63261-0000-1000-8000-00805f9b34fb"

/* ECB characteristics UUIDs */
private const val ECB_COMMAND_UUID = "2d3a8ac3-0000-1000-8000-00805f9b34fb"
private const val ECB_DATA_UUID = "83670c18-0000-1000-8000-00805f9b34fb"
private const val ECB_HW_VERSION_UUID = "997ca8f9-0000-1000-8000-00805f9b34fb"
private const val ECB_SW_VERSION_UUID = "20a14f57-0000-1000-8000-00805f9b34fb"

/* Service read special read Ids */
private const val SW_VERSION_READ_ID = Long.MAX_VALUE
private const val HW_VERSION_READ_ID = Long.MAX_VALUE - 1

/* Requested MTU for the BLE communication */
private const val GATT_MAX_MTU_SIZE = 517

/* Time in milliseconds before the BLE scan should timeout */
private const val BLE_SCAN_DURATION = 15000L

/* ECB response identifier size in bytes */
private const val RESPONSE_ID_SIZE = 4

/* The size of the token in bytes */
private const val TOKEN_SIZE = 16

/* Time in millisecond before BLE operations are checked for timeout */
private const val BLE_OPERATION_TIMEOUT = 1000L

/* Maximal number of times a data send must be reissued in case of error */
private const val WRITE_DATA_RETRY_COUNT = 3

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ECBBluetoothManager private constructor(context: Context, handler: EventCallbackHandler) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* Defines the interface for the device events handling */
    interface EventCallbackHandler {
        /* Called when a device is connected */
        fun onDeviceConnected(status: ECBBleError)

        /* Called when a device is disconnected */
        fun onDeviceDisconnected(status: ECBBleError)
    }

    /* Defines the list of errors that can be raised by the bluetooth manager */
    enum class ECBBleError {
        SUCCESS,
        TIMED_OUT,
        WRITE_FAILED,
        READ_FAILED,
        NOT_CONNECTED,
        INVALID_TOKEN,
        INVALID_IDENTIFIER
    }

    /* Singleton object for the bluetooth manager */
    companion object {
        /* Singleton instance */
        @Volatile
        private var instance: ECBBluetoothManager? = null

        /* Create the singleton instance */
        fun getInstance(context: Context, handler: EventCallbackHandler) =
            instance ?: synchronized(this) {
                instance ?: ECBBluetoothManager(context, handler).also {
                    instance = it
                }
            }
    }

    /***************************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **************************************************************************************************/
    /* Defines the applicable command types */
    private enum class CommandType {
        COMMAND,
        SW_VERSION_READ,
        HW_VERSION_READ
    }

    /* Defines a command response */
    private class ECBCommandResponse {
        var commandType = CommandType.COMMAND
        var data = ByteArray(0)
        var bleStatus = ECBBleError.SUCCESS
    }

    /* Defines the internal data for an ECB command, with its callback and time out value */
    private class ECBCommand {
        var commandType = CommandType.COMMAND
        var commandId = 0
        var command = ByteArray(0)
        var callback: ((ECBBleError, ByteArray?, Int) -> Unit)? = null
        var timeout = 0L
    }

    /* Defines a sender */
    private class DataSender {
        var data = ByteArray(0)
        var progressCallback: ((Float) -> Unit)? = null
        var sendDataEndCallback: ((ECBBleError) -> Unit)? = null
        var timeout = 0L
    }

    /* Defines a receiver */
    private class DataReceiver {
        var dataSize = 0
        var progressCallback: ((Float) -> Unit)? = null
        var receiveDataEndCallback: ((ECBBleError, ByteArray?) -> Unit)? = null
        var timeout = 0L
    }

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* Data end nimble */
    private val endDataNimble = byteArrayOf(
        0xFE.toByte(), 0xDE.toByte(), 0xAD.toByte(), 0xC0.toByte(),
        0xDE.toByte(), 0xEC.toByte(), 0xBB.toByte(), 0xAD.toByte(),
        0x0E.toByte(), 0x12.toByte(), 0x34.toByte(), 0x56.toByte(),
        0x78.toByte(), 0x90.toByte(), 0xAA.toByte(), 0xBB.toByte()
    )

    /* Application context */
    private val appContext: Context = context

    /* Even handler */
    private val eventHandler: EventCallbackHandler = handler

    /* Runner thread scheduler */
    private val runnerScheduler = Executors.newScheduledThreadPool(1)

    /* BLE adapter */
    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager =
            appContext.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    /* BLE scanner instance */
    private val bleScanner by lazy {
        bluetoothAdapter.bluetoothLeScanner
    }

    /* BLE scan settings */
    private val scanSettings = ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
        .build()

    /* Stores the result of the BLE scan */
    private val scanResults = mutableListOf<ScanResult>()

    /* Stores the ECB Identifier and Token */
    private lateinit var ecbIdentifier: String
    private lateinit var ecbToken: String

    /* Stores the ECB BLE device */
    private var ecbDevice: ScanResult? = null

    /* Stores the current ECB GATT instance */
    private var ecbGatt: BluetoothGatt? = null

    /* Descriptor write semaphore */
    private var waitDescriptorSem = Semaphore(1, 0)

    /* BLE operation semaphore */
    private var bleOperationSem = Semaphore(1, 0)

    /* BLE command waiting response map lock */
    private var waitingResponseCommandLock = Mutex(false)

    /* BLE command waiting response map */
    private val waitingResponseCommands: HashMap<Long, ECBCommand> = HashMap()

    /* BLE command responses channel */
    private var responseCommandsChannel = Channel<ECBCommandResponse>(capacity = Channel.UNLIMITED)

    /* BLE command send thread */
    private var commandResponseThread: Thread

    /* BLE command ID */
    private var lastCommandId = AtomicInteger(0)

    /* BLE pending commands channel */
    private var pendingCommandsChannel = Channel<ECBCommand>(capacity = Channel.UNLIMITED)

    /* BLE command send thread */
    private var commandSenderThread: Thread

    /* BLE last data operation status */
    private var dataOperationWriteStatus = false

    /* BLE last data send operation status semaphore */
    private var dataOperationWriteStatusSem = Semaphore(1, 1)

    /* BLE data sender channel */
    private var dataSenderChannel = Channel<DataSender>(capacity = Channel.UNLIMITED)

    /* BLE data send thread */
    private var dataSenderThread: Thread

    /* BLE data receiver channel */
    private var dataReceiverChannel = Channel<DataReceiver>(capacity = Channel.UNLIMITED)

    /* BLE data received channel */
    private var dataReceiveChannel = Channel<ByteArray>(capacity = Channel.UNLIMITED)

    /* BLE data receive thread */
    private var dataReceiverThread: Thread

    /* Shared data instance */
    private val sharedData = SharedData.getInstance()

    /* Connection state */
    private var wasConnected = false

    /* GATT callback object */
    @SuppressLint("MissingPermission")
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.i(MODULE_NAME, "Successfully connected to $gatt.device.address")

                    wasConnected = true
                    ecbGatt = gatt
                    ecbGatt?.discoverServices()
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.i(MODULE_NAME, "Successfully disconnected from $gatt.device.address")
                    gatt.close()
                    ecbGatt = null
                    ecbDevice = null

                    eventHandler.onDeviceDisconnected(ECBBleError.SUCCESS)
                }
            } else {
                Log.e(MODULE_NAME, "Error $status encountered for $gatt.device.address!")
                gatt.close()
                ecbGatt = null
                ecbDevice = null

                eventHandler.onDeviceDisconnected(ECBBleError.NOT_CONNECTED)
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            super.onServicesDiscovered(gatt, status)

            Handler(Looper.getMainLooper()).post {
                runBlocking {
                    launch {
                        /* Request MTU */
                        waitDescriptorSem.acquire()
                        ecbGatt?.requestMtu(GATT_MAX_MTU_SIZE)

                        /* Subscribe to command notification */
                        val commandChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                            ?.getCharacteristic(UUID.fromString(ECB_COMMAND_UUID))
                        if (commandChar == null) {
                            Log.e(MODULE_NAME, "Failed to get command characteristic.")
                            disconnect()
                            return@launch
                        }
                        waitDescriptorSem.acquire()
                        enableNotifications(commandChar)

                        /* Subscribe to data notification */
                        val dataChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                            ?.getCharacteristic(UUID.fromString(ECB_DATA_UUID))
                        if (dataChar == null) {
                            Log.e(MODULE_NAME, "Failed to get data characteristic.")
                            disconnect()
                            return@launch
                        }
                        waitDescriptorSem.acquire()
                        enableNotifications(dataChar)


                        /* Notify end of setup and connection success */
                        waitDescriptorSem.acquire()
                        waitDescriptorSem.release()
                        eventHandler.onDeviceConnected(ECBBleError.SUCCESS)
                    }
                }
            }
        }

        override fun onMtuChanged(gatt: BluetoothGatt?, mtu: Int, status: Int) {
            super.onMtuChanged(gatt, mtu, status)

            if (waitDescriptorSem.availablePermits < 1) {
                waitDescriptorSem.release()
            }
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            val uuid = characteristic.uuid
            val response = ECBCommandResponse()

            Log.d(MODULE_NAME, "Characteristic $uuid write status: $status")

            /* Release the ble operation */
            bleOperationSem.release()

            /* Get the response type */
            when (uuid.toString()) {
                ECB_COMMAND_UUID -> {
                    response.commandType = CommandType.COMMAND
                }

                ECB_DATA_UUID -> {
                    dataOperationWriteStatus = status == BluetoothGatt.GATT_SUCCESS
                    dataOperationWriteStatusSem.release()
                    return
                }

                else -> {
                    Log.e(MODULE_NAME, "Received write from $uuid")
                    return
                }
            }

            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    response.bleStatus = ECBBleError.SUCCESS
                }

                else -> {
                    response.bleStatus = ECBBleError.WRITE_FAILED

                    /* Send the response */
                    runBlocking {
                        launch {
                            responseCommandsChannel.send(response)
                        }
                    }
                }
            }
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
            status: Int
        ) {
            val uuid = characteristic.uuid
            val response = ECBCommandResponse()

            /* Release the ble operation */
            bleOperationSem.release()

            /* Get the data */
            response.data = value

            /* Get the response type */
            when (uuid.toString()) {
                ECB_COMMAND_UUID -> {
                    response.commandType = CommandType.COMMAND
                }

                ECB_SW_VERSION_UUID -> {
                    response.commandType = CommandType.SW_VERSION_READ
                }

                ECB_HW_VERSION_UUID -> {
                    response.commandType = CommandType.HW_VERSION_READ
                }

                else -> {
                    Log.e(MODULE_NAME, "Received read from $uuid")
                    return
                }
            }

            /* Get the status */
            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    response.bleStatus = ECBBleError.SUCCESS

                    /* Get the data */
                    response.data = value
                }

                else -> {
                    response.bleStatus = ECBBleError.READ_FAILED
                }
            }

            /* Send the response */
            runBlocking {
                launch {
                    responseCommandsChannel.send(response)
                }
            }
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            super.onCharacteristicChanged(gatt, characteristic, value)

            val uuid = characteristic.uuid
            val response = ECBCommandResponse()

            Log.d(MODULE_NAME, "Characteristic $uuid changed with data size ${value.size}")

            /* Get the response type */
            when (uuid.toString()) {
                ECB_COMMAND_UUID -> {
                    response.commandType = CommandType.COMMAND
                }

                ECB_DATA_UUID -> {
                    runBlocking {
                        launch {
                            dataReceiveChannel.send(value)
                        }
                    }
                    return
                }

                else -> {
                    Log.e(MODULE_NAME, "Received notify from $uuid")
                    return
                }
            }


            /* Get the data */
            response.data = value
            response.bleStatus = ECBBleError.SUCCESS

            /* Send the response */
            runBlocking {
                launch {
                    responseCommandsChannel.send(response)
                }
            }
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt?,
            descriptor: BluetoothGattDescriptor?,
            status: Int
        ) {
            super.onDescriptorWrite(gatt, descriptor, status)

            /* Unlock BLE operations */
            waitDescriptorSem.release()
        }
    }

    @SuppressLint("MissingPermission")
    private val scanCallback = object : ScanCallback() {

        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val indexQuery = scanResults.indexOfFirst {
                it.device.address == result.device.address
            }
            if (indexQuery != -1) {
                /* A scan result already exists with the same address */
                scanResults[indexQuery] = result
            } else {
                with(result.device) {
                    Log.i(MODULE_NAME, "Scanned BLE device ${name ?: "Unnamed"} at $address")
                    if (name == ecbIdentifier && result.isConnectable) {
                        ecbDevice = result
                        ecbDevice!!.device.connectGatt(appContext, true, gattCallback)
                    }
                }
                scanResults.add(result)
            }

            /* Stop scanning if we were able to connect */
            if (ecbDevice != null) {
                bleScanner.stopScan(this)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e(MODULE_NAME, "BLE scan failed with code $errorCode")
            ecbDevice = null
            eventHandler.onDeviceConnected(ECBBleError.NOT_CONNECTED)
        }
    }

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    @SuppressLint("MissingPermission")
    private fun startBLEScan() {

        /* Clear the previous results */
        scanResults.clear()
        wasConnected = false

        /* Start scanning with a timeout */
        bleScanner.startScan(null, scanSettings, scanCallback)
        Handler(Looper.getMainLooper()).postDelayed({
            bleScanner.stopScan(scanCallback)

            /* Check if we were able to connect */
            if (ecbGatt == null && !wasConnected) {
                eventHandler.onDeviceConnected(ECBBleError.INVALID_IDENTIFIER)
            }
        }, BLE_SCAN_DURATION)
    }

    @SuppressLint("MissingPermission")
    private fun enableNotifications(characteristic: BluetoothGattCharacteristic) {
        val cccdUuid = UUID.fromString(CCC_DESCRIPTOR_UUID)
        val payload = when {
            characteristic.isNotifiable() -> BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            else -> {
                Log.e(MODULE_NAME, "${characteristic.uuid} doesn't support notifications")
                return
            }
        }

        characteristic.getDescriptor(cccdUuid)?.let { cccDescriptor ->
            if (ecbGatt?.setCharacteristicNotification(characteristic, true) == false) {
                Log.e(MODULE_NAME, "Enabling notification failed for ${characteristic.uuid}")
                return
            } else {
                Log.d(MODULE_NAME, "Enabling notification for ${characteristic.uuid}")
            }
            ecbGatt?.writeDescriptor(cccDescriptor, payload)
        } ?: Log.e(MODULE_NAME, "${characteristic.uuid} doesn't contain the CCC descriptor!")
    }

    private fun bleManagerRoutine() {
        runBlocking {
            launch {
                val commands = ArrayList<ECBCommand>()

                waitingResponseCommandLock.withLock {
                    val iterator = waitingResponseCommands.iterator()
                    while (iterator.hasNext()) {
                        val command = iterator.next()
                        if (command.value.timeout < System.currentTimeMillis()) {
                            commands.add(command.value)
                            iterator.remove()
                        }
                    }
                }

                /* Execute the timed out callbacks */
                for (ecbCommand in commands) {
                    ecbCommand.callback?.invoke(ECBBleError.TIMED_OUT, null, ecbCommand.commandId)
                }
            }
        }
    }

    @SuppressLint("MissingPermission")
    private suspend fun bleCommandSenderRoutine() {
        while (true) {
            /* Get the next command  */
            val command = pendingCommandsChannel.receive()
            val characteristic: BluetoothGattCharacteristic?
            val commandId: Long

            when (command.commandType) {
                CommandType.COMMAND -> {
                    characteristic = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                        ?.getCharacteristic(UUID.fromString(ECB_COMMAND_UUID))

                    commandId = command.commandId.toLong()

                    if (characteristic?.isWritable() == true) {
                        /* Perform the write */
                        bleOperationSem.acquire()

                        /* Add the command */
                        waitingResponseCommandLock.withLock {
                            waitingResponseCommands[commandId] = command
                        }

                        val writeStatus = ecbGatt?.writeCharacteristic(
                            characteristic,
                            command.command,
                            BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        )

                        /* Write */
                        if (writeStatus != BluetoothStatusCodes.SUCCESS) {
                            bleOperationSem.release()

                            waitingResponseCommandLock.withLock {
                                waitingResponseCommands.remove(commandId)
                            }

                            Log.e(MODULE_NAME, "Error while sending command: $writeStatus")

                            /* Call the callback and update */
                            command.callback?.invoke(
                                ECBBleError.WRITE_FAILED,
                                null,
                                commandId.toInt()
                            )
                        }
                    } else {
                        command.callback?.invoke(ECBBleError.WRITE_FAILED, null, commandId.toInt())
                    }
                }

                CommandType.HW_VERSION_READ -> {
                    characteristic = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                        ?.getCharacteristic(UUID.fromString(ECB_HW_VERSION_UUID))
                    commandId = HW_VERSION_READ_ID

                    if (characteristic?.isReadable() == true) {
                        /* Perform the write */
                        bleOperationSem.acquire()

                        /* Add the command */
                        waitingResponseCommandLock.withLock {
                            waitingResponseCommands[commandId] = command
                        }

                        val readStatus = ecbGatt?.readCharacteristic(characteristic)

                        /* Read */
                        if (readStatus != true) {
                            bleOperationSem.release()

                            waitingResponseCommandLock.withLock {
                                waitingResponseCommands.remove(commandId)
                            }

                            Log.e(MODULE_NAME, "Error while reading HW characteristic")

                            /* Call the callback and update */
                            command.callback?.invoke(
                                ECBBleError.READ_FAILED,
                                null,
                                commandId.toInt()
                            )
                        }
                    } else {
                        command.callback?.invoke(ECBBleError.READ_FAILED, null, commandId.toInt())
                    }
                }

                CommandType.SW_VERSION_READ -> {
                    characteristic = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                        ?.getCharacteristic(UUID.fromString(ECB_SW_VERSION_UUID))
                    commandId = SW_VERSION_READ_ID

                    if (characteristic?.isReadable() == true) {
                        /* Perform the write */
                        bleOperationSem.acquire()

                        /* Add the command */
                        waitingResponseCommandLock.withLock {
                            waitingResponseCommands[commandId] = command
                        }

                        val readStatus = ecbGatt?.readCharacteristic(characteristic)

                        /* Read */
                        if (readStatus != true) {
                            bleOperationSem.release()

                            waitingResponseCommandLock.withLock {
                                waitingResponseCommands.remove(commandId)
                            }

                            Log.e(MODULE_NAME, "Error while reading SW characteristic")

                            /* Call the callback and update */
                            command.callback?.invoke(
                                ECBBleError.READ_FAILED,
                                null,
                                commandId.toInt()
                            )
                        }
                    } else {
                        command.callback?.invoke(ECBBleError.READ_FAILED, null, commandId.toInt())
                    }
                }
            }
        }
    }

    private suspend fun bleCommandReceiverRoutine() {
        while (true) {
            /* Get the next command response */
            val response = responseCommandsChannel.receive()

            /* Check the response type */
            var commandId = 0L
            when (response.commandType) {
                CommandType.COMMAND -> {
                    if (response.bleStatus != ECBBleError.SUCCESS) {
                        /* We cannot know the response ID */
                        Log.d(
                            MODULE_NAME,
                            "Received command response BLE error $response.bleStatus"
                        )
                        continue
                    }

                    /* Check the response ID */
                    val responseId = ByteArray(4)
                    response.data.copyInto(responseId, 0, 0, RESPONSE_ID_SIZE)

                    commandId = read4BytesFromBuffer(responseId).toLong()
                }

                CommandType.SW_VERSION_READ -> {
                    commandId = SW_VERSION_READ_ID
                }

                CommandType.HW_VERSION_READ -> {
                    commandId = HW_VERSION_READ_ID
                }
            }

            /* Search for a pending command */
            var command: ECBCommand? = null
            waitingResponseCommandLock.withLock {
                val iterator = waitingResponseCommands.iterator()
                while (iterator.hasNext()) {
                    val item = iterator.next()
                    if (item.key == commandId) {
                        command = item.value
                        iterator.remove()
                        break
                    }
                }
            }

            if (command != null) {
                if (command!!.commandType == CommandType.COMMAND) {
                    val responseToken = ByteArray(TOKEN_SIZE)

                    /* Check the response token */
                    response.data.copyInto(
                        responseToken,
                        0,
                        RESPONSE_ID_SIZE,
                        RESPONSE_ID_SIZE + TOKEN_SIZE
                    )
                    if (responseToken.toString(Charsets.US_ASCII) == ecbToken) {
                        /* Copy the response buffer */
                        val offset = RESPONSE_ID_SIZE + TOKEN_SIZE
                        val buffer = ByteArray(response.data.size - offset)
                        response.data.copyInto(
                            buffer,
                            0,
                            offset,
                            response.data.size
                        )
                        command!!.callback?.invoke(ECBBleError.SUCCESS, buffer, commandId.toInt())
                    } else {
                        command!!.callback?.invoke(
                            ECBBleError.INVALID_TOKEN,
                            null,
                            commandId.toInt()
                        )
                    }
                } else {
                    /* No need to check token and response status for reads */
                    command!!.callback?.invoke(
                        ECBBleError.SUCCESS,
                        response.data,
                        commandId.toInt()
                    )
                }
            }
        }
    }


    @SuppressLint("MissingPermission")
    private suspend fun bleSenderRoutine() {
        val mtuBase = 503
        val buffer = ByteArray(mtuBase)

        while (true) {
            /* Get the next sender */
            val sender = dataSenderChannel.receive()

            val dataChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
                ?.getCharacteristic(UUID.fromString(ECB_DATA_UUID))

            if (dataChar?.isWritable() == true) {
                var toSend = sender.data.size
                var sent = 0

                /* Try to get a better priority */
                bleOperationSem.acquire()
                ecbGatt?.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH)
                bleOperationSem.release()

                var retryCount = 0

                while (toSend > 0) {
                    val toWrite = min(sender.data.size - sent, mtuBase)
                    sender.data.copyInto(buffer, 0, sent, sent + toWrite)

                    /* Perform the write */
                    bleOperationSem.acquire()

                    val actualBuffer = if (toWrite < mtuBase) {
                        buffer.sliceArray(0..<toWrite)
                    } else {
                        buffer
                    }

                    val writeStatus = ecbGatt?.writeCharacteristic(
                        dataChar,
                        actualBuffer,
                        BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                    )

                    if (writeStatus == BluetoothStatusCodes.SUCCESS) {
                        /* Wait for the write status */
                        dataOperationWriteStatusSem.acquire()

                        if (!dataOperationWriteStatus) {
                            ++retryCount

                            /* Check if we reached the maximal retry count */
                            if (retryCount == WRITE_DATA_RETRY_COUNT) {
                                sender.sendDataEndCallback?.invoke(ECBBleError.WRITE_FAILED)
                                break
                            }
                        } else {
                            retryCount = 0
                            sent += toWrite
                            toSend -= toWrite

                            sender.progressCallback?.invoke(
                                sent.toFloat() / sender.data.size.toFloat()
                            )
                        }
                    } else {
                        bleOperationSem.release()
                        sender.sendDataEndCallback?.invoke(ECBBleError.WRITE_FAILED)
                        break
                    }
                }

                /* Set back old priority */
                bleOperationSem.acquire()
                ecbGatt?.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_BALANCED)
                bleOperationSem.release()

                /* End of transmission callback */
                if (toSend == 0) {
                    sender.sendDataEndCallback?.invoke(ECBBleError.SUCCESS)
                }
            } else {
                Log.e(MODULE_NAME, "Error while sending data: characteristic is not writeable")
                sender.sendDataEndCallback?.invoke(ECBBleError.WRITE_FAILED)
            }
        }
    }

    private suspend fun bleReceiverRoutine() {
        while (true) {
            /* Get the next receiver */
            val receiver = dataReceiverChannel.receive()

            var receivedData = ByteArray(0)
            if (receiver.dataSize >= 0) {
                receivedData = ByteArray(receiver.dataSize)
            }

            /* Get the data */
            var received = 0

            /* Wait until we received everything or the data size is not known */
            while (received < receiver.dataSize || receiver.dataSize == -1) {
                val receivedChunk = dataReceiveChannel.receive()

                /* Copy */
                if (receiver.dataSize >= 0) {
                    val toCopy: Int = if (receivedChunk.size > receiver.dataSize - received) {
                        receiver.dataSize - received
                    } else {
                        receivedChunk.size
                    }
                    receivedChunk.copyInto(receivedData, received, 0, toCopy)
                    received += toCopy

                    /* Call progress */
                    if (receiver.progressCallback != null) {
                        receiver.progressCallback?.invoke(
                            received.toFloat() / receiver.dataSize.toFloat()
                        )
                    }
                } else {


                    /* Check if we received the end data nimble */
                    if (receivedChunk.size >= endDataNimble.size) {
                        val nimble = ByteArray(endDataNimble.size)
                        receivedChunk.copyInto(
                            nimble,
                            0,
                            receivedChunk.size - endDataNimble.size,
                            receivedChunk.size
                        )

                        if (nimble.contentEquals(endDataNimble)) {

                            val msgChunk = ByteArray(receivedChunk.size - endDataNimble.size)
                            receivedChunk.copyInto(
                                nimble,
                                0,
                                0,
                                receivedChunk.size - endDataNimble.size
                            )
                            receivedData += msgChunk
                            break
                        } else {
                            receivedData += receivedChunk
                        }
                    } else {
                        receivedData += receivedChunk
                    }
                }
            }

            /* Send the result */
            receiver.receiveDataEndCallback?.invoke(ECBBleError.SUCCESS, receivedData)
        }
    }

    private fun read4BytesFromBuffer(buffer: ByteArray): Int {
        return (buffer[3].toInt() shl 24) or
                (buffer[2].toInt() and 0xff shl 16) or
                (buffer[1].toInt() and 0xff shl 8) or
                (buffer[0].toInt() and 0xff)
    }

    private fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)

    private fun BluetoothGattCharacteristic.isWritable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)

    private fun BluetoothGattCharacteristic.isNotifiable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_NOTIFY)

    private fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    init {
        /* Start the runner thread */
        runnerScheduler.scheduleWithFixedDelay({
            bleManagerRoutine()
        }, 0, BLE_OPERATION_TIMEOUT, TimeUnit.MILLISECONDS)


        /* Start the command sender thread */
        commandSenderThread = Thread {
            runBlocking {
                bleCommandSenderRoutine()
            }
        }
        commandSenderThread.start()

        /* Start the command response thread */
        commandResponseThread = Thread {
            runBlocking {
                bleCommandReceiverRoutine()
            }
        }
        commandResponseThread.start()

        /* Start the data sender thread */
        dataSenderThread = Thread {
            runBlocking {
                bleSenderRoutine()
            }
        }
        dataSenderThread.start()

        /* Start the data receiver thread */
        dataReceiverThread = Thread {
            runBlocking {
                bleReceiverRoutine()
            }
        }
        dataReceiverThread.start()
    }

    fun connect() {
        runBlocking {
            launch {
                /* Get the HWUID and the bluetooth token */
                ecbIdentifier = sharedData.getECBIdentifier(appContext)
                if (ecbIdentifier.isEmpty()) {
                    eventHandler.onDeviceConnected(ECBBleError.INVALID_IDENTIFIER)
                    return@launch
                }
                ecbToken = sharedData.getECBToken(appContext)
                if (ecbToken.isEmpty()) {
                    eventHandler.onDeviceConnected(ECBBleError.INVALID_TOKEN)
                    return@launch
                }

                /* Check if we are connected */
                if (ecbGatt == null) {
                    /* Start the BLE scan */
                    startBLEScan()
                } else {
                    eventHandler.onDeviceConnected(ECBBleError.SUCCESS)
                }
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        if (ecbGatt != null) {
            ecbGatt?.disconnect()
        } else {
            eventHandler.onDeviceDisconnected(ECBBleError.NOT_CONNECTED)
        }
    }

    fun sendCommand(
        commType: Int,
        commBuffer: ByteArray,
        callback: (ECBBleError, ByteArray?, Int) -> Unit,
        timeoutMs: Int
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null, 0)
            return
        }

        /* Prepare the command */
        val ecbCommand = ECBCommand()

        ecbCommand.commandType = CommandType.COMMAND
        ecbCommand.commandId = lastCommandId.getAndIncrement()
        val commandIdBytes = ByteArray(RESPONSE_ID_SIZE)
        for (i in 0..3) {
            commandIdBytes[i] = (ecbCommand.commandId shr (i * 8)).toByte()
        }
        ecbCommand.command = commandIdBytes +
                ecbToken.toByteArray(Charsets.US_ASCII) +
                commType.toByte() +
                commBuffer.size.toByte() +
                commBuffer
        ecbCommand.callback = callback
        ecbCommand.timeout = System.currentTimeMillis() + timeoutMs

        /* Add the command */
        runBlocking {
            launch {
                pendingCommandsChannel.send(ecbCommand)
            }
        }
    }

    fun waitCommandResponse(
        commandId: Int,
        callback: (ECBBleError, ByteArray?, Int) -> Unit,
        timeoutMs: Int,
    ) {
        /* Prepare the command */
        val ecbCommand = ECBCommand()
        ecbCommand.commandType = CommandType.COMMAND
        ecbCommand.commandId = commandId
        ecbCommand.timeout = System.currentTimeMillis() + timeoutMs
        ecbCommand.callback = callback

        runBlocking {
            launch {
                /* Add the command */
                waitingResponseCommandLock.withLock {
                    waitingResponseCommands[commandId.toLong()] = ecbCommand
                }
            }
        }
    }

    fun cancelWaitCommandResponse(commandId: Int) {
        runBlocking {
            launch {
                /* Remove the command from the queue */
                waitingResponseCommandLock.withLock {
                    waitingResponseCommands.remove(commandId.toLong())
                }
            }
        }
    }

    fun sendData(
        dataBytes: ByteArray,
        progressCallback: ((Float) -> Unit)?,
        sendDataEndCallback: (ECBBleError) -> Unit,
        timeoutMs: Int
    ) {

        /* Check if we are connected */
        if (ecbGatt == null) {
            sendDataEndCallback(ECBBleError.NOT_CONNECTED)
            return
        }

        /* Add sender to the queue */
        val sender = DataSender()
        sender.data = dataBytes
        sender.progressCallback = progressCallback
        sender.sendDataEndCallback = sendDataEndCallback
        sender.timeout = System.currentTimeMillis() + timeoutMs

        runBlocking {
            launch {
                dataSenderChannel.send(sender)
            }
        }
    }

    fun receiveData(
        dataSize: Int,
        progressCallback: ((Float) -> Unit)?,
        receiveDataEndCallback: (ECBBleError, ByteArray?) -> Unit,
        timeoutMs: Int
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            receiveDataEndCallback(ECBBleError.NOT_CONNECTED, null)
            return
        }

        /* Add receiver to the queue */
        val receiver = DataReceiver()
        receiver.dataSize = dataSize
        receiver.progressCallback = progressCallback
        receiver.receiveDataEndCallback = receiveDataEndCallback
        receiver.timeout = System.currentTimeMillis() + timeoutMs

        runBlocking {
            launch {
                dataReceiverChannel.send(receiver)
            }
        }
    }

    fun readSoftwareVersion(
        callback: (ECBBleError, ByteArray?, Int) -> Unit,
        timeoutMs: Int
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null, 0)
            return
        }

        /* Prepare the command */
        val ecbCommand = ECBCommand()

        ecbCommand.commandType = CommandType.SW_VERSION_READ
        ecbCommand.commandId = SW_VERSION_READ_ID.toInt()
        ecbCommand.callback = callback
        ecbCommand.timeout = System.currentTimeMillis() + timeoutMs

        /* Add the command */
        runBlocking {
            launch {
                pendingCommandsChannel.send(ecbCommand)
            }
        }
    }

    fun readHardwareVersion(
        callback: (ECBBleError, ByteArray?, Int) -> Unit,
        timeoutMs: Int
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null, 0)
            return
        }

        /* Prepare the command */
        val ecbCommand = ECBCommand()

        ecbCommand.commandType = CommandType.HW_VERSION_READ
        ecbCommand.commandId = HW_VERSION_READ_ID.toInt()
        ecbCommand.callback = callback
        ecbCommand.timeout = System.currentTimeMillis() + timeoutMs

        /* Add the command */
        runBlocking {
            launch {
                pendingCommandsChannel.send(ecbCommand)
            }
        }
    }

    fun getTokenSize(): Int {
        return TOKEN_SIZE
    }

    fun setECBToken(token: String) {
        ecbToken = token

        runBlocking {
            launch {
                /* Set the new token locally */
                sharedData.setECBToken(appContext, token)
            }
        }
    }
}