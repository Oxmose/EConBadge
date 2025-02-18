package dev.olsontek.econbadgetesting

import android.Manifest
import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.Activity
import android.app.AlertDialog
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
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Button
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.sync.Semaphore
import java.util.UUID

private const val PERMISSION_REQUEST_CODE = 1
private const val ECB_ADDRESS = "08:D1:F9:DF:B5:1A"
private const val CCC_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805f9b34fb"
private const val GATT_MAX_MTU_SIZE = 512
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        bleManager = BLEManager(this)

        /* Create button */
        findViewById<Button>(R.id.buttonPing).setOnClickListener {
            commandMgr.sendPing()
        }
        findViewById<Button>(R.id.buttonGetHW).setOnClickListener {
            commandMgr.getHWVersion()
        }
        findViewById<Button>(R.id.buttonGetSW).setOnClickListener {
            commandMgr.getSWVersion()
        }
        findViewById<Button>(R.id.buttonSetToken).setOnClickListener {
            commandMgr.setToken()
        }
        findViewById<Button>(R.id.buttonClearEINK).setOnClickListener {
            commandMgr.einkClear()
        }
        findViewById<Button>(R.id.buttonSendEINK).setOnClickListener {
            commandMgr.einkNewImage()
        }
        findViewById<Button>(R.id.buttonRemoveEINK).setOnClickListener {
            commandMgr.einkRemove()
        }
        findViewById<Button>(R.id.buttonSelectEINK).setOnClickListener {
            commandMgr.einkSelect()
        }
        findViewById<Button>(R.id.buttonGetImageNameEINK).setOnClickListener {
            commandMgr.einkGetImageName()
        }
        findViewById<Button>(R.id.buttonGetImageEINK).setOnClickListener {
            commandMgr.einkGetImage()
        }

        buttonStartScan = findViewById<Button>(R.id.buttonStartScan)
        buttonStartScan?.setOnClickListener {
            if (isScanning) {
                isScanning = bleManager?.stopBleScan() == true
            }
            else {
                isScanning = bleManager?.startBleScan() == true
            }
        }

        textviewDebug = findViewById(R.id.debugTextview)
    }

    override fun onResume() {
        super.onResume()
        if (!bluetoothAdapter.isEnabled) {
            promptEnableBluetooth()
        }
    }

    fun postBLEInit() {
        commandMgr.init(bleManager, this)
    }

    /**
     * Determine whether the current [Context] has been granted the relevant [Manifest.permission].
     */
    fun Context.hasPermission(permissionType: String): Boolean {
        return ContextCompat.checkSelfPermission(this, permissionType) ==
                PackageManager.PERMISSION_GRANTED
    }

    /**
     * Determine whether the current [Context] has been granted the relevant permissions to perform
     * Bluetooth operations depending on the mobile device's Android version.
     */
    fun Context.hasRequiredBluetoothPermissions(): Boolean {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            hasPermission(Manifest.permission.BLUETOOTH_SCAN) &&
                    hasPermission(Manifest.permission.BLUETOOTH_CONNECT)
        } else {
            hasPermission(Manifest.permission.ACCESS_FINE_LOCATION)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode != PERMISSION_REQUEST_CODE) return
        val containsPermanentDenial = permissions.zip(grantResults.toTypedArray()).any {
            it.second == PackageManager.PERMISSION_DENIED &&
                    !ActivityCompat.shouldShowRequestPermissionRationale(this, it.first)
        }
        val containsDenial = grantResults.any { it == PackageManager.PERMISSION_DENIED }
        val allGranted = grantResults.all { it == PackageManager.PERMISSION_GRANTED }
        when {
            containsPermanentDenial -> {
                // TODO: Handle permanent denial (e.g., show AlertDialog with justification)
                // Note: The user will need to navigate to App Settings and manually grant
                // permissions that were permanently denied
            }
            containsDenial -> {
                requestRelevantRuntimePermissions()
            }
            allGranted && hasRequiredBluetoothPermissions() -> {
                bleManager?.startBleScan()
            }
            else -> {
                // Unexpected scenario encountered when handling permissions
                recreate()
            }
        }
    }

    private fun Activity.requestRelevantRuntimePermissions() {
        if (hasRequiredBluetoothPermissions()) { return }
        when {
            Build.VERSION.SDK_INT < Build.VERSION_CODES.S -> {
                requestLocationPermission()
            }
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
                requestBluetoothPermissions()
            }
        }
    }

    private fun requestLocationPermission() = runOnUiThread {
        AlertDialog.Builder(this)
            .setTitle("Location permission required")
            .setMessage(
                "Starting from Android M (6.0), the system requires apps to be granted " +
                        "location access in order to scan for BLE devices."
            )
            .setCancelable(false)
            .setPositiveButton(android.R.string.ok) { _, _ ->
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(Manifest.permission.ACCESS_FINE_LOCATION),
                    PERMISSION_REQUEST_CODE
                )
            }
            .show()
    }

    @RequiresApi(Build.VERSION_CODES.S)
    private fun requestBluetoothPermissions() = runOnUiThread {
        AlertDialog.Builder(this)
            .setTitle("Bluetooth permission required")
            .setMessage(
                "Starting from Android 12, the system requires apps to be granted " +
                        "Bluetooth access in order to scan for and connect to BLE devices."
            )
            .setCancelable(false)
            .setPositiveButton(android.R.string.ok) { _, _ ->
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(
                        Manifest.permission.BLUETOOTH_SCAN,
                        Manifest.permission.BLUETOOTH_CONNECT
                    ),
                    PERMISSION_REQUEST_CODE
                )
            }
            .show()
    }

    /**
     * Prompts the user to enable Bluetooth via a system dialog.
     *
     * For Android 12+, [Manifest.permission.BLUETOOTH_CONNECT] is required to use
     * the [BluetoothAdapter.ACTION_REQUEST_ENABLE] intent.
     */
    private fun promptEnableBluetooth() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            !hasPermission(Manifest.permission.BLUETOOTH_CONNECT)
        ) {
            // Insufficient permission to prompt for Bluetooth enabling
            return
        }
        if (!bluetoothAdapter.isEnabled) {
            Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE).apply {
                bluetoothEnablingResult.launch(this)
            }
        }
    }

    private val bluetoothEnablingResult = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            // Bluetooth is enabled, good to go
        } else {
            // User dismissed or denied Bluetooth prompt
            promptEnableBluetooth()
        }
    }

    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }




    @SuppressLint("MissingPermission")
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
            writeDescriptor(cccDescriptor, payload)
        } ?: Log.e("ConnectionManager", "${characteristic.uuid} doesn't contain the CCC descriptor!")
    }

    @SuppressLint("MissingPermission")
    fun writeDescriptor(descriptor: BluetoothGattDescriptor, payload: ByteArray) {
        ecbGatt?.let { gatt ->
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                Log.d("ENABLE", "" + gatt.writeDescriptor(descriptor, payload).toString())
            } else {
                // Fall back to deprecated version of writeDescriptor for Android <13
                gatt.legacyDescriptorWrite(descriptor, payload)
            }
        } ?: error("Not connected to a BLE device!")
    }
    @SuppressLint("MissingPermission")
    @TargetApi(Build.VERSION_CODES.S)
    @Suppress("DEPRECATION")
    private fun BluetoothGatt.legacyDescriptorWrite(
        descriptor: BluetoothGattDescriptor,
        value: ByteArray
    ): Boolean {
        descriptor.value = value

        Log.d("ENABLE", "" +writeDescriptor(descriptor).toString())
        return true
    }

    fun getActivity() : MainActivity =  this

    @OptIn(ExperimentalStdlibApi::class)
    private val gattCallback = object : BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
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
            Handler(Looper.getMainLooper()).post {
                commandMgr.init(ecbGatt, getActivity())
            }
        }

        @SuppressLint("MissingPermission")
        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            with(characteristic) {
                val value = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) {
                    characteristic.value
                } else {
                    ecbGatt?.readCharacteristic(characteristic)
                }
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Wrote to characteristic $uuid | value: ${value}")
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
        }

        @Deprecated("Deprecated for Android 13+")
        @Suppress("DEPRECATION")
        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            with(characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Read characteristic $uuid:\n${value.toHexString()}")
                        Handler(Looper.getMainLooper()).post({textviewDebug?.setText(value.toString(Charsets.US_ASCII) + "\n" + value.toHexString())})

                    }
                    BluetoothGatt.GATT_READ_NOT_PERMITTED -> {
                        Log.e("BluetoothGattCallback", "Read not permitted for $uuid!")
                    }
                    else -> {
                        Log.e("BluetoothGattCallback", "Characteristic read failed for $uuid, error: $status")
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
            when (status) {
                BluetoothGatt.GATT_SUCCESS -> {
                    Log.i("BluetoothGattCallback", "Read characteristic $uuid:\n${value.toHexString()}")
                    if(uuid.toString() != "83670c18-0000-1000-8000-00805f9b34fb") {
                        Handler(Looper.getMainLooper()).post({
                            textviewDebug?.setText(
                                value.toString(
                                    Charsets.US_ASCII
                                ) + "\n" + value.toHexString()
                            )
                        })
                    }
                }
                BluetoothGatt.GATT_READ_NOT_PERMITTED -> {
                    Log.e("BluetoothGattCallback", "Read not permitted for $uuid!")
                }
                else -> {
                    Log.e("BluetoothGattCallback", "Characteristic read failed for $uuid, error: $status")
                }
            }
        }

        @Deprecated("Deprecated for Android 13+")
        @Suppress("DEPRECATION")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic
        ) {
            with(characteristic) {
                Log.i("BluetoothGattCallback", "Characteristic2 $uuid changed | value: ${value.toHexString()}")
                Handler(Looper.getMainLooper()).post({textviewDebug?.setText(value.toString(Charsets.US_ASCII) + "\n" + value.toHexString())})

            }

        }
        @SuppressLint("MissingPermission")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            val newValueHex = value.toString(Charsets.US_ASCII)
            with(characteristic) {
                Log.i("BluetoothGattCallback", "Characteristic $uuid changed | value: $newValueHex")
                if(uuid.toString() == "45fd43da-0000-1000-8000-00805f9b34fb") {
                    Log.d("BLE", "Unlocking defered wait")
                    waitDeferedResSem.release()
                }
                else if(uuid.toString() == "83670c18-0000-1000-8000-00805f9b34fb") {
                    Log.d("BLE", "Unlocking data wait")
                    runBlocking {
                        launch {
                            dataReceiveSemW.acquire()
                            Log.d("BLE", "DOING Unlocking data wait")
                            dataReceive = value
                            dataReceiveSemR.release()
                        }
                    }
                }
                else {
                    Handler(Looper.getMainLooper()).post({textviewDebug?.setText(value.toString(Charsets.US_ASCII) + "\n" + value.toHexString())})
                }
            }
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt?,
            descriptor: BluetoothGattDescriptor?,
            status: Int
        ) {

            super.onDescriptorWrite(gatt, descriptor, status)
            waitDescriptorWrittenSem.release()

        }
    }

    @SuppressLint("MissingPermission")
    suspend fun getData(dataBuffer: ByteArray, offset: Int) : Int {
        dataReceiveSemR.acquire()
        Log.d("BLE", "Unblocked!")
        dataReceive.copyInto(dataBuffer, offset)
        var size = dataReceive.size
        dataReceiveSemW.release()
        return size
    }

    suspend fun waitCommandResponse(endStr: String) {
        waitDeferedResSem.acquire()
        Handler(Looper.getMainLooper()).post({textviewDebug?.setText(endStr)})
    }
    suspend fun waitDeferedResponse(endStr: String) {
        waitDeferedResSem.acquire()
        Handler(Looper.getMainLooper()).post({textviewDebug?.setText(endStr)})
    }

    suspend fun waitForDescriptorReady() {
        waitDescriptorWrittenSem.acquire()
    }

    fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)
    fun BluetoothGattCharacteristic.isWritable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)
    fun BluetoothGattCharacteristic.isWritableWithoutResponse(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)
    fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }

    fun BluetoothGattCharacteristic.isIndicatable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_INDICATE)

    fun BluetoothGattCharacteristic.isNotifiable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_NOTIFY)
    private var isScanning = false
    private var waitDescriptorWrittenSem = Semaphore(1, 0)
    private var waitDeferedResSem = Semaphore(1, 1)
    private var dataReceiveSemR = Semaphore(1, 1)
    private var dataReceiveSemW = Semaphore(1, 0)
    private var dataReceive: ByteArray = ByteArray(1)
    private var bleManager: BLEManager? = null
    private var buttonStartScan: Button? = null
    private var textviewDebug: TextView? = null
    private var commandMgr = CommandMgr()
}