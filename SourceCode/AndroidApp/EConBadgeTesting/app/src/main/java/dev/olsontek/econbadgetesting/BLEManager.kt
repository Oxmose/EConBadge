package dev.olsontek.econbadgetesting

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.Semaphore
import java.util.UUID

private const val ECB_ADDRESS = "08:D1:F9:DF:B5:1A"
private const val CCC_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805f9b34fb"
private const val ECB_SERVICE_UUID = "d3e63261-0000-1000-8000-00805f9b34fb"
private const val ECB_COMMAND_UUID = "2d3a8ac3-0000-1000-8000-00805f9b34fb"
private const val ECB_COMMAND_DEFER_UUID = "45fd43da-0000-1000-8000-00805f9b34fb"
private const val ECB_DATA_UUID = "83670c18-0000-1000-8000-00805f9b34fb"
private const val ECB_HW_VERSION_UUID = "997ca8f9-0000-1000-8000-00805f9b34fb"
private const val ECB_SW_VERSION_UUID = "20a14f57-0000-1000-8000-00805f9b34fb"
private const val GATT_MAX_MTU_SIZE = 512


class BLEManager(activity: MainActivity) {
    enum class ECharacteristic {
        HW_VERSION,
        SW_VERSION
    }

    val mainActivity = activity

    @SuppressLint("MissingPermission")
    fun startBleScan(): Boolean {
        Log.d("BLE", "Scan started")
        scanResults.clear()
        isScanning = true
        bleScanner.startScan(null, scanSettings, scanCallback)
        return true
    }

    @SuppressLint("MissingPermission")
    fun stopBleScan() : Boolean {
        Log.d("BLE", "Scan stoped")
        bleScanner.stopScan(scanCallback)
        isScanning = false
        return false
    }

    @SuppressLint("MissingPermission", "NewApi")
    fun enableNotifications(characteristic: BluetoothGattCharacteristic) {
        val cccdUuid = UUID.fromString(CCC_DESCRIPTOR_UUID)
        val payload = when {
            characteristic.isIndicatable() -> BluetoothGattDescriptor.ENABLE_INDICATION_VALUE
            characteristic.isNotifiable() -> BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            else -> {
                Log.e("ConnectionManager", "${characteristic.uuid} doesn't support notifications/indications")
                return
            }
        }

        characteristic.getDescriptor(cccdUuid)?.let { cccDescriptor ->
            if (ecbGatt?.setCharacteristicNotification(characteristic, true) == false) {
                Log.e("ConnectionManager", "setCharacteristicNotification failed for ${characteristic.uuid}")
                return
            }
            else {
                Log.d("ConnectionManager", "Enabling notification for ${characteristic.uuid}")
            }
            ecbGatt?.writeDescriptor(cccDescriptor, payload)
        } ?: Log.e("ConnectionManager", "${characteristic.uuid} doesn't contain the CCC descriptor!")
    }

    suspend fun readCharacteristic(dataBuffer: ByteArray, characteristic: ECharacteristic) {

    }

    suspend fun sendData(dataBuffer: ByteArray) {

        waitDataSendSem.acquire()
    }
    suspend fun receiveData(dataBuffer: ByteArray, offset: Int) : Int {
        dataReceiveSemR.acquire()
        dataReceive.copyInto(dataBuffer, offset)
        var size = dataReceive.size
        dataReceiveSemW.release()
        return size
    }

    suspend fun sendCommand(dataBuffer: ByteArray) {

    }
    suspend fun waitCommandResponse(dataBuffer: ByteArray) {
        waitCommandRedSem.acquire()
    }

    var commandResponse: ByteArray? = null
    var commandResponseLock: Mutex = Mutex(false);


    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val indexQuery = scanResults.indexOfFirst { it.device.address == result.device.address }
            if (indexQuery != -1) { // A scan result already exists with the same address
                scanResults[indexQuery] = result
            } else {
                with (result.device) {
                    Log.i("ScanCallback", "Found BLE device! Name: ${name ?: "Unnamed"}, address: $address")
                    if(address == ECB_ADDRESS) {
                        stopBleScan()
                        connectGatt(mainAct?.applicationContext, false, gattCallback)
                    }
                }
                scanResults.add(result)
            }
        }
        override fun onScanFailed(errorCode: Int) {
            Log.e("ScanCallback", "onScanFailed: code $errorCode")
        }
    }

    private val bleScanner by lazy {
        bluetoothAdapter.bluetoothLeScanner
    }

    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = mainAct?.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    @SuppressLint("MissingPermission")
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            val deviceAddress = gatt.device.address
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.w("BluetoothGattCallback", "Successfully connected to $deviceAddress")
                    ecbGatt = gatt

                    Handler(Looper.getMainLooper()).post {
                        gatt.requestMtu(GATT_MAX_MTU_SIZE)
                        ecbGatt?.discoverServices()
                    }

                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.w("BluetoothGattCallback", "Successfully disconnected from $deviceAddress")
                    gatt.close()
                }
            } else {
                Log.w("BluetoothGattCallback", "Error $status encountered for $deviceAddress! Disconnecting...")
                gatt.close()
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            super.onServicesDiscovered(gatt, status)
            mainAct?.postBLEInit()
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            with(characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Wrote characteristic $uuid:\n")

                        if(uuid.toString() == ECB_DATA_UUID) {
                            waitDataSendSem.release()
                            Log.d("BluetoothGattCallback", "Unlocking DATA SEND")
                        } else {
                            Log.d("BluetoothGattCallback", "Nothing to do")
                        }
                    }
                    BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH -> {
                        Log.e("BluetoothGattCallback", "Write exceeded connection ATT MTU!")
                    }
                    BluetoothGatt.GATT_WRITE_NOT_PERMITTED -> {
                        Log.e("BluetoothGattCallback", "Write not permitted for $uuid!")
                    }
                    else -> {
                        Log.e("BluetoothGattCallback", "Characteristic write failed for $uuid, error: $status")
                    }
                }
            }
            /* Unlock BLE operations */
            bleOperationSem.release()
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
            status: Int
        ) {
            val uuid = characteristic.uuid
            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    Log.i("BluetoothGattCallback", "Read characteristic $uuid:\n")
                }
                BluetoothGatt.GATT_READ_NOT_PERMITTED -> {
                    Log.e("BluetoothGattCallback", "Read not permitted for $uuid!")
                }
                else -> {
                    Log.e("BluetoothGattCallback", "Characteristic read failed for $uuid, error: $status")
                }
            }
            /* Unlock BLE operations */
            bleOperationSem.release()
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            with(characteristic) {
                Log.i("BluetoothGattCallback", "Characteristic $uuid notify")
                if(uuid.toString() == ECB_COMMAND_DEFER_UUID) {
                    Log.d("BluetoothGattCallback", "Unlocking DEFERRED COMMAND")
                    waitDeferedResSem.release()
                }
                else if(uuid.toString() == ECB_COMMAND_UUID) {
                    Log.d("BluetoothGattCallback", "Unlocking COMMEND RESPONSE")
                    waitCommandRedSem.release()
                }
                else if(uuid.toString() == ECB_DATA_UUID) {
                    Log.d("BluetoothGattCallback", "Unlocking DATA RECEIVE")
                    runBlocking {
                        launch {
                            dataReceiveSemW.acquire()
                            dataReceive = value
                            dataReceiveSemR.release()
                        }
                    }
                }
            }
            /* Unlock BLE operations */
            bleOperationSem.release()
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt?,
            descriptor: BluetoothGattDescriptor?,
            status: Int
        ) {
            super.onDescriptorWrite(gatt, descriptor, status)

            /* Unlock BLE operations */
            bleOperationSem.release()
        }
    }

    private fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)
    private fun BluetoothGattCharacteristic.isWritable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)
    private fun BluetoothGattCharacteristic.isWritableWithoutResponse(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)
    private fun BluetoothGattCharacteristic.isIndicatable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_INDICATE)
    private fun BluetoothGattCharacteristic.isNotifiable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_NOTIFY)
    private fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }

    private val scanSettings = ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
        .build()

    private val scanResults = mutableListOf<ScanResult>()

    private var isScanning = false
    private var bleOperationSem = Semaphore(1, 0)
    private var waitDataSendSem = Semaphore(1, 0)
    private var waitCommandRedSem = Semaphore(1, 1)
    private var waitDeferedResSem = Semaphore(1, 1)
    private var dataReceiveSemR = Semaphore(1, 1)
    private var dataReceiveSemW = Semaphore(1, 0)
    private var dataReceive: ByteArray = ByteArray(1)
    private var ecbGatt: BluetoothGatt? = null
    private var mainAct: MainActivity? = null
}