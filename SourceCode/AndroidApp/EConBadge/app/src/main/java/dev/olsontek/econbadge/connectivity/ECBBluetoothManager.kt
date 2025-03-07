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
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.Semaphore
import java.util.UUID
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "ECBBluetoothManager"

/* CCC Descriptor UUID */
private const val CCC_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805f9b34fb"

/* ECB main service UUID */
private const val ECB_SERVICE_UUID    = "d3e63261-0000-1000-8000-00805f9b34fb"

/* ECB characteristics UUIDs */
private const val ECB_COMMAND_UUID    = "2d3a8ac3-0000-1000-8000-00805f9b34fb"
private const val ECB_DATA_UUID       = "83670c18-0000-1000-8000-00805f9b34fb"
private const val ECB_HW_VERSION_UUID = "997ca8f9-0000-1000-8000-00805f9b34fb"
private const val ECB_SW_VERSION_UUID = "20a14f57-0000-1000-8000-00805f9b34fb"

/* Requested MTU for the BLE communication */
private const val GATT_MAX_MTU_SIZE   = 517

/* Time in milliseconds before the BLE scan should timeout */
private const val BLE_SCAN_DURATION   = 15000L

/* ECB response identifier size in bytes */
private const val RESPONSE_ID_SIZE    = 4

/* The size of the token in bytes */
private const val TOKEN_SIZE          = 16

/* Time in millisecond before a command should timeout */
private const val COMMAND_TIMEOUT     = 10000L

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ECBBluetoothManager private constructor(context: Context, handler: EventCallbackHandler) {

/***************************************************************************************************
 * PUBLIC TYPES AND ENUMERATIONS
 **************************************************************************************************/
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
        INVALID_IDENTIFIER,
        INVALID_RESPONSE,
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
    /* Defines the internal data for an ECB command, with its callback and time out value */
    private class ECBCommand {
        var commandId = 0
        var command = ByteArray(0)
        var callback: ((ECBBleError, CommandManager.ECBResponse?) -> Unit)? = null
        var timeout = 0L
    }

/***************************************************************************************************
 * PRIVATE ATTRIBUTES
 **************************************************************************************************/

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
    private val waitDescriptorSem = Semaphore(1, 0)

    /* BLE operation semaphore */
    private val bleOperationSem = Semaphore(1, 0)

    /* BLE command operation lock */
    private val commandOperationLock = Mutex(false)

    /* BLE command pending map */
    private val pendingCommands: HashMap<Int, ECBCommand> = HashMap()

    /* BLE last issued command */
    private var lastPendingCommand = 0

    /* BLE read operation lock */
    private val readOperationLock = Mutex(false)

    /* BLE read pending map */
    private val pendingReads: HashMap<String, ECBCommand> = HashMap()

    /* BLE last issued read */
    private var lastPendingRead = ""

    /* BLE command ID */
    private val lastCommandId = AtomicInteger(0)

    /* Shared data instance */
    private val sharedData = SharedData.getInstance()

    /* GATT callback object */
    @SuppressLint("MissingPermission")
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.i(MODULE_NAME, "Successfully connected to $gatt.device.address")
                    ecbGatt = gatt
                    ecbGatt?.discoverServices()
                }
                else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.i(MODULE_NAME, "Successfully disconnected from $gatt.device.address")
                    gatt.close()
                    ecbGatt = null
                    ecbDevice = null

                    Handler(Looper.getMainLooper()).post {
                        eventHandler.onDeviceDisconnected(ECBBleError.SUCCESS)
                    }
                }
            }
            else {
                Log.e(MODULE_NAME, "Error $status encountered for $gatt.device.address!")
                gatt.close()
                ecbGatt = null
                ecbDevice = null

                Handler(Looper.getMainLooper()).post {
                    eventHandler.onDeviceDisconnected(ECBBleError.NOT_CONNECTED)
                }
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
            var callback: ((ECBBleError, CommandManager.ECBResponse?) -> Unit)? = null
            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    Log.d(MODULE_NAME, "Characteristic write succeeded for $uuid")
                }
                else -> {
                    if (uuid == UUID.fromString(ECB_COMMAND_UUID)) {
                        runBlocking {
                            launch {

                                commandOperationLock.lock()

                                /* Remove last pending command */
                                callback = pendingCommands[lastPendingCommand]?.callback
                                pendingCommands.remove(lastPendingCommand)

                                commandOperationLock.unlock()
                            }
                        }
                    }
                    Log.e(MODULE_NAME, "Characteristic write failed for $uuid: $status")
                }
            }

            /* Release the operation */
            bleOperationSem.release()

            /* Callback */
            callback?.invoke(ECBBleError.WRITE_FAILED, null)
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
            status: Int
        ) {
            val uuid = characteristic.uuid
            var callback: ((ECBBleError, CommandManager.ECBResponse?) -> Unit)? = null
            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    Log.d(MODULE_NAME, "Read characteristic $uuid:\n")
                    runBlocking {
                        launch {
                            readOperationLock.lock()
                            callback = pendingReads[uuid.toString()]?.callback
                            pendingReads.remove(uuid.toString())
                            readOperationLock.unlock()
                            if (callback != null) {
                                callback?.invoke(
                                    ECBBleError.SUCCESS,
                                    CommandManager.ECBResponse(0, value)
                                )

                                callback = null
                            }
                        }
                    }

                }
                else -> {
                    runBlocking {
                        launch {
                            readOperationLock.lock()

                            /* Remove last pending command */
                            callback = pendingReads[lastPendingRead]?.callback
                            pendingReads.remove(lastPendingRead)

                            readOperationLock.unlock()
                        }
                    }
                    Log.e(MODULE_NAME, "Characteristic read failed for $uuid, error: $status")
                }
            }

            /* Release the operation */
            bleOperationSem.release()

            /* Callback */
            callback?.invoke(ECBBleError.READ_FAILED, null)
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            super.onCharacteristicChanged(gatt, characteristic, value)
            Log.d(MODULE_NAME, "Characteristic " + characteristic.uuid + " notify")
            if (characteristic.uuid == UUID.fromString(ECB_COMMAND_UUID)) {
                runBlocking {
                    launch {
                        handleCommandResponse(value)
                    }
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

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val indexQuery = scanResults.indexOfFirst { it.device.address == result.device.address }
            if (indexQuery != -1) {
                /* A scan result already exists with the same address */
                scanResults[indexQuery] = result
            }
            else {
                with (result.device) {
                    Log.i(MODULE_NAME,"Scanned BLE device ${name ?: "Unnamed"} at $address")
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

/***************************************************************************************************
 * PRIVATE METHODS
 **************************************************************************************************/
    @SuppressLint("MissingPermission")
    private fun startBLEScan() {

        /* Clear the previous results */
        scanResults.clear()

        /* Start scanning with a timeout */
        bleScanner.startScan(null, scanSettings, scanCallback)
        Handler(Looper.getMainLooper()).postDelayed({
            bleScanner.stopScan(scanCallback)

            /* Check if we were able to connect */
            if (ecbGatt == null) {
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

        characteristic.getDescriptor(cccdUuid)?.let {
            cccDescriptor ->
            if (ecbGatt?.setCharacteristicNotification(characteristic, true) == false) {
                Log.e(MODULE_NAME, "Enabling notification failed for ${characteristic.uuid}")
                return
            }
            else {
                Log.d(MODULE_NAME, "Enabling notification for ${characteristic.uuid}")
            }
            ecbGatt?.writeDescriptor(cccDescriptor, payload)
        } ?: Log.e(MODULE_NAME, "${characteristic.uuid} doesn't contain the CCC descriptor!")
    }

    private fun bleManagerRoutine() {
        runBlocking {
            launch {
                val commands = ArrayList<ECBCommand>()

                /* Clean the pending commands */
                commandOperationLock.lock()

                val iterator = pendingCommands.iterator()
                while(iterator.hasNext()) {
                    val command = iterator.next()
                    if (command.value.timeout < System.currentTimeMillis()) {
                        commands.add(command.value)
                        iterator.remove()
                    }
                }

                commandOperationLock.unlock()

                /* Clean the pending reads */
                readOperationLock.lock()

                val iteratorRead = pendingReads.iterator()
                while(iteratorRead.hasNext()) {
                    val command = iteratorRead.next()
                    if (command.value.timeout < System.currentTimeMillis()) {
                        commands.add(command.value)
                        iteratorRead.remove()
                    }
                }

                readOperationLock.unlock()

                Log.d(MODULE_NAME, "Cleaning " + commands.size + " timed out command.")

                /* Execute the timed out callbacks */
                for(ecbCommand in commands) {
                    ecbCommand.callback?.invoke(ECBBleError.TIMED_OUT, null)
                }
            }
        }
    }

    private suspend fun handleCommandResponse(value: ByteArray) {
        var callback: ((ECBBleError, CommandManager.ECBResponse?) -> Unit)? = null
        val responseId = ByteArray(RESPONSE_ID_SIZE)
        val responseToken = ByteArray(TOKEN_SIZE)

        /* Check if we have a correct response */
        if (value.size > RESPONSE_ID_SIZE + TOKEN_SIZE) {
            /* Check the response ID */
            value.copyInto(responseId, 0, 0, RESPONSE_ID_SIZE)

            /* Search for a callback */
            commandOperationLock.lock()
            val iterator = pendingCommands.iterator()
            while (iterator.hasNext()) {
                val item = iterator.next()
                if (item.key == read4BytesFromBuffer(responseId)) {
                    callback = item.value.callback
                    iterator.remove()
                    break
                }
            }
            commandOperationLock.unlock()
        }

        if (callback != null) {
            /* Check the response token */
            value.copyInto(
                responseToken,
                0,
                RESPONSE_ID_SIZE,
                RESPONSE_ID_SIZE + TOKEN_SIZE
            )
            if (responseToken.toString(Charsets.US_ASCII) == ecbToken) {

                /* Copy the response buffer */
                var offset = RESPONSE_ID_SIZE + TOKEN_SIZE
                val responseStatus = value[offset]
                val responseSize = value[offset + 1]

                offset += 2

                val buffer: ByteArray
                if (responseSize.toInt() == (value.size - offset)) {
                    buffer = ByteArray(responseSize.toInt())
                    value.copyInto(
                        buffer,
                        0,
                        offset,
                        value.size
                    )
                    callback.invoke(
                        ECBBleError.SUCCESS,
                        CommandManager.ECBResponse(responseStatus, buffer)
                    )
                }
                else {
                    callback.invoke(ECBBleError.INVALID_RESPONSE, null)
                }
            }
            else {
                callback.invoke(ECBBleError.INVALID_TOKEN, null)
            }
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

/***************************************************************************************************
 * PUBLIC METHODS
 **************************************************************************************************/
    init {
        /* Start the runner thread */
        runnerScheduler.scheduleWithFixedDelay({
            bleManagerRoutine()
        }, 0, COMMAND_TIMEOUT, TimeUnit.MILLISECONDS)
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
                }
                else {
                    eventHandler.onDeviceConnected(ECBBleError.SUCCESS)
                }
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        if (ecbGatt != null) {
            ecbGatt?.disconnect()
        }
        else {
            eventHandler.onDeviceDisconnected(ECBBleError.NOT_CONNECTED)
        }
    }

    @SuppressLint("MissingPermission")
    fun sendCommand(
        commType: Int,
        commBuffer: ByteArray,
        callback: (ECBBleError, CommandManager.ECBResponse?) -> Unit
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null)
            return
        }

        val commandChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
            ?.getCharacteristic(UUID.fromString(ECB_COMMAND_UUID))

        if (commandChar?.isWritable() == true) {
            /* Prepare the command */
            val ecbCommand = ECBCommand()

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
            ecbCommand.timeout = System.currentTimeMillis() + COMMAND_TIMEOUT

            runBlocking {
                launch {
                    /* Perform the write */
                    bleOperationSem.acquire()

                    commandOperationLock.lock()

                    val writeStatus = ecbGatt?.writeCharacteristic(
                        commandChar,
                        ecbCommand.command,
                        BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                    )
                    if (writeStatus == BluetoothStatusCodes.SUCCESS) {
                        /* Add to the command queue */
                        lastPendingCommand = ecbCommand.commandId
                        pendingCommands[lastPendingCommand] = ecbCommand
                    }
                    else {
                        bleOperationSem.release()

                        Log.e(MODULE_NAME, "Error while sending command: $writeStatus")

                        /* Call the callback and update */
                        callback(ECBBleError.WRITE_FAILED, null)
                    }
                    commandOperationLock.unlock()
                }
            }
        }
        else {
            Log.e(MODULE_NAME, "Error while sending command: characteristic is not writeable")
            callback(ECBBleError.WRITE_FAILED, null)
            return
        }
    }

    @SuppressLint("MissingPermission")
    fun readSoftwareVersion(
        callback: (ECBBleError, CommandManager.ECBResponse?) -> Unit
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null)
            return
        }

        val swVersionChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
            ?.getCharacteristic(UUID.fromString(ECB_SW_VERSION_UUID))

        if (swVersionChar?.isReadable() == true) {
            val ecbCommand = ECBCommand()

            ecbCommand.callback = callback
            ecbCommand.timeout = System.currentTimeMillis() + COMMAND_TIMEOUT
            runBlocking {
                launch {
                    bleOperationSem.acquire()

                    /* Perform the write */
                    readOperationLock.lock()

                    val readStatus = ecbGatt?.readCharacteristic(swVersionChar)
                    if (readStatus == true) {
                        /* Add to the command queue */
                        lastPendingRead = ECB_SW_VERSION_UUID
                        pendingReads[lastPendingRead] = ecbCommand
                    }
                    else {
                        bleOperationSem.release()

                        Log.e(MODULE_NAME, "Error while reading characteristic")

                        /* Call the callback and update */
                        callback(ECBBleError.READ_FAILED, null)
                    }
                    readOperationLock.unlock()
                }
            }
        }
        else {
            Log.e(MODULE_NAME, "Error while getting SW version: not readable")
            callback(ECBBleError.READ_FAILED, null)
            return
        }
    }

    @SuppressLint("MissingPermission")
    fun readHardwareVersion(
        callback: (ECBBleError, CommandManager.ECBResponse?) -> Unit
    ) {
        /* Check if we are connected */
        if (ecbGatt == null) {
            callback(ECBBleError.NOT_CONNECTED, null)
            return
        }

        val swVersionChar = ecbGatt?.getService(UUID.fromString(ECB_SERVICE_UUID))
            ?.getCharacteristic(UUID.fromString(ECB_HW_VERSION_UUID))

        if (swVersionChar?.isReadable() == true) {
            val ecbCommand = ECBCommand()

            ecbCommand.callback = callback
            ecbCommand.timeout = System.currentTimeMillis() + COMMAND_TIMEOUT
            runBlocking {
                launch {
                    /* Perform the write */
                    bleOperationSem.acquire()

                    readOperationLock.lock()
                    val readStatus = ecbGatt?.readCharacteristic(swVersionChar)
                    if (readStatus == true) {
                        /* Add to the command queue */
                        lastPendingRead = ECB_HW_VERSION_UUID
                        pendingReads[lastPendingRead] = ecbCommand
                    }
                    else {
                        bleOperationSem.release()

                        Log.e(MODULE_NAME, "Error while reading characteristic")

                        /* Call the callback and update */
                        callback(ECBBleError.READ_FAILED, null)
                    }
                    readOperationLock.unlock()
                }
            }
        }
        else {
            Log.e(MODULE_NAME, "Error while getting HW version: not readable")
            callback(ECBBleError.READ_FAILED, null)
            return
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