package dev.olsontek.econbadgetesting

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

private const val PERMISSION_REQUEST_CODE = 1
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
        findViewById<Button>(R.id.buttonFactoryReset).setOnClickListener {
            commandMgr.factoryReset()
        }
        findViewById<Button>(R.id.buttonSetOwner).setOnClickListener {
            commandMgr.setOwner()
        }
        findViewById<Button>(R.id.buttonSetContact).setOnClickListener {
            commandMgr.setContact()
        }
        findViewById<Button>(R.id.buttonUpdate).setOnClickListener {
            commandMgr.performUpdate()
        }
        findViewById<Button>(R.id.buttonEnableLed).setOnClickListener {
            commandMgr.setLedBorderEnable(true)
        }
        findViewById<Button>(R.id.buttonDisableLed).setOnClickListener {
            commandMgr.setLedBorderEnable(false)
        }
        findViewById<Button>(R.id.buttonGetLedState).setOnClickListener {
            commandMgr.getLedBorderEnable()
        }
        findViewById<Button>(R.id.buttonIncBrightness).setOnClickListener {
            commandMgr.increaseLedBrightness()
        }
        findViewById<Button>(R.id.buttonDecBrightness).setOnClickListener {
            commandMgr.decreaseLedBrightness()
        }
        findViewById<Button>(R.id.buttonSetBrightness).setOnClickListener {
            commandMgr.setLedBrightness()
        }
        findViewById<Button>(R.id.buttonGetBrightness).setOnClickListener {
            commandMgr.getLedBrightness()
        }
        findViewById<Button>(R.id.buttonLedClear).setOnClickListener {
            commandMgr.ledClear()
        }
        findViewById<Button>(R.id.buttonLedAddPattern).setOnClickListener {
            commandMgr.ledAddPattern()
        }
        findViewById<Button>(R.id.buttonLedRemPattern).setOnClickListener {
            commandMgr.ledRemPattern()
        }
        findViewById<Button>(R.id.buttonLedGetPatterns).setOnClickListener {
            commandMgr.ledGetPatterns()
        }
        findViewById<Button>(R.id.buttonLedClearPatterns).setOnClickListener {
            commandMgr.ledClearPatterns()
        }
        findViewById<Button>(R.id.buttonLedAddAnim).setOnClickListener {
            commandMgr.ledAddAnim()
        }
        findViewById<Button>(R.id.buttonLedRemAnim).setOnClickListener {
            commandMgr.ledRemAnim()
        }
        findViewById<Button>(R.id.buttonLedGetAnims).setOnClickListener {
            commandMgr.ledGetAnims()
        }
        findViewById<Button>(R.id.buttonLedClearAnims).setOnClickListener {
            commandMgr.ledClearAnims()
        }


        buttonStartScan = findViewById(R.id.buttonStartScan)
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
        bleManager?.let { commandMgr.init(it, this) }
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

    fun Context.hasRequiredWifiPermissions(): Boolean {
        return hasPermission(Manifest.permission.INTERNET)
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
            allGranted && hasRequiredBluetoothPermissions() && hasRequiredWifiPermissions() -> {
                bleManager?.startBleScan()
            }
            else -> {
                // Unexpected scenario encountered when handling permissions
                recreate()
            }
        }
    }

    @RequiresApi(Build.VERSION_CODES.S)
    private fun Activity.requestRelevantRuntimePermissions() {
        if (hasRequiredBluetoothPermissions() && hasRequiredWifiPermissions()) { return }
        when {
            Build.VERSION.SDK_INT < Build.VERSION_CODES.S -> {
                requestLocationPermission()
                requestInternetPermissions()
            }
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
                requestBluetoothPermissions()
                requestInternetPermissions()
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

    @RequiresApi(Build.VERSION_CODES.S)
    private fun requestInternetPermissions() = runOnUiThread {
        AlertDialog.Builder(this)
            .setTitle("Internet permission required")
            .setMessage(
                "Internet permissions are required for firmware update."
            )
            .setCancelable(false)
            .setPositiveButton(android.R.string.ok) { _, _ ->
                ActivityCompat.requestPermissions(
                    this,
                    arrayOf(
                        Manifest.permission.INTERNET,
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

    fun setDebugText(text: String) {
        runOnUiThread {
            textviewDebug?.text = text
        }
    }

    private var isScanning = false
    private var bleManager: BLEManager? = null
    private var buttonStartScan: Button? = null
    private var textviewDebug: TextView? = null
    private var commandMgr = CommandMgr()
}