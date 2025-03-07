package dev.olsontek.econbadge

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.BluetoothAdapter
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.Menu
import android.view.Window
import android.view.WindowManager
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import com.google.android.material.navigation.NavigationView
import com.google.android.material.shape.CornerFamily
import com.google.android.material.shape.MaterialShapeDrawable
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.data.SharedData
import dev.olsontek.econbadge.databinding.ActivityMainBinding
import dev.olsontek.econbadge.ui.home.HomeViewModel
import dev.olsontek.econbadge.ui.login.ConnectActivity
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "MainActivity"

/* Permission request code for internet/bluetooth permissions */
private const val PERMISSION_REQUEST_CODE = 1

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class MainActivity : AppCompatActivity(), ECBBluetoothManager.EventCallbackHandler {

/***************************************************************************************************
 * PUBLIC TYPES AND ENUMERATIONS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * PRIVATE TYPES AND ENUMERATIONS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * PRIVATE ATTRIBUTES
 **************************************************************************************************/
    /* Bluetooth manager instance */
    private var bleManager: ECBBluetoothManager? = null

    /* Command manager instance */
    private var commandManager: CommandManager? = null

    /* App bar configuration object */
    private lateinit var appBarConfiguration: AppBarConfiguration

    /* Main layout binding object */
    private lateinit var binding: ActivityMainBinding

    /* Connection information dialog */
    private var connectDialog: AlertDialog? = null

    /* Bluetooth enable result handler */
    private val bluetoothEnablingResult = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) {
        result ->
        if (result.resultCode == Activity.RESULT_OK) {
            Log.i(MODULE_NAME,"Bluetooth is enabled")
        }
        else {
            Log.i(MODULE_NAME,"Bluetooth is not enabled")
            enableBluetooth()
        }
    }

    /* Disconnection request flag */
    private var disconnectRequested = false

    /* Stores the fragments vew models */
    private lateinit var homeViewModel: HomeViewModel

    /* Shared data singleton */
    private val sharedData = SharedData.getInstance()

/***************************************************************************************************
 * PRIVATE METHODS
 **************************************************************************************************/
    private fun enableBluetooth() {
        Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE).apply {
            bluetoothEnablingResult.launch(this)
        }
    }

    private fun Context.hasPermission(permissionType: String): Boolean {
        return ContextCompat.checkSelfPermission(this, permissionType) ==
               PackageManager.PERMISSION_GRANTED
    }

    private fun Context.hasRequiredBluetoothPermissions(): Boolean {
        return hasPermission(Manifest.permission.BLUETOOTH_SCAN) &&
               hasPermission(Manifest.permission.BLUETOOTH_CONNECT)
    }

    private fun Context.hasRequiredWifiPermissions(): Boolean {
        return hasPermission(Manifest.permission.INTERNET)
    }

    private fun Activity.requestRelevantRuntimePermissions() {
        if (hasRequiredBluetoothPermissions() && hasRequiredWifiPermissions()) {
            return
        }
        requestBluetoothPermissions()
        requestInternetPermissions()
    }

    private fun requestBluetoothPermissions() = runOnUiThread {
        AlertDialog.Builder(this)
            .setTitle(getString(R.string.bluetooth_permission_required))
            .setMessage(getString(R.string.request_bluetooth_msg))
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

    private fun requestInternetPermissions() = runOnUiThread {
        AlertDialog.Builder(this)
            .setTitle(getString(R.string.internet_permission_required))
            .setMessage(getString(R.string.request_internet_msg))
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

    private fun displayConnectDialog() {
        connectDialog?.dismiss()

        val builder = AlertDialog.Builder(this)
        builder.setTitle(getString(R.string.connecting_to_econbadge))
        builder.setMessage(getString(R.string.connecting_please_wait))
        builder.setCancelable(false)

        connectDialog = builder.create()
        connectDialog?.show()
    }

    private fun hideConnectDialog() {
        connectDialog?.dismiss()
    }


    private fun retrieveSoftwareVersion(){
        commandManager?.getSoftwareVersion {
                commandError: CommandManager.CommandError,
                response: CommandManager.ECBResponse? ->
            runOnUiThread {
                when (commandError) {
                    CommandManager.CommandError.SUCCESS -> {
                        if (response?.response != null) {
                            homeViewModel.setSoftwareVersion(
                                response.response.toString(Charsets.US_ASCII)
                            )
                        } else {
                            homeViewModel.setSoftwareVersion(
                                getString(R.string.unknown_software_version)
                            )
                        }
                    }

                    else -> {
                        homeViewModel.setSoftwareVersion(
                            getString(R.string.unknown_software_version)
                        )
                    }
                }
            }
        }
    }

    private fun retrieveHardwareVersion(){
        commandManager?.getHardwareVersion {
                commandError: CommandManager.CommandError,
                response: CommandManager.ECBResponse? ->
            runOnUiThread {
                when (commandError) {
                    CommandManager.CommandError.SUCCESS -> {
                        if (response?.response != null) {
                            homeViewModel.setHardwareVersion(
                                response.response.toString(Charsets.US_ASCII)
                            )
                        } else {
                            homeViewModel.setHardwareVersion(
                                getString(R.string.unknown_hardware_version)
                            )
                        }
                    }

                    else -> {
                        homeViewModel.setHardwareVersion(
                            getString(R.string.unknown_hardware_version)
                        )
                    }
                }
            }
        }
    }

    private fun retrieveOwnerInfo(){
        commandManager?.getOwnerInfo {
                commandError: CommandManager.CommandError,
                response: CommandManager.ECBResponse? ->
            runOnUiThread {
                when (commandError) {
                    CommandManager.CommandError.SUCCESS -> {
                        if (response?.response != null) {
                            homeViewModel.setOwner(
                                response.response.toString(Charsets.US_ASCII)
                            )
                        } else {
                            homeViewModel.setOwner(
                                getString(R.string.failed_to_retrieve_owner)
                            )
                        }
                    }

                    else -> {
                        homeViewModel.setOwner(
                            getString(R.string.failed_to_retrieve_owner)
                        )
                    }
                }
            }
        }
    }

    private fun retrieveContactInfo(){
        commandManager?.getContactInfo {
                commandError: CommandManager.CommandError,
                response: CommandManager.ECBResponse? ->
            runOnUiThread {
                when (commandError) {
                    CommandManager.CommandError.SUCCESS -> {
                        if (response?.response != null) {
                            homeViewModel.setContact(
                                response.response.toString(Charsets.US_ASCII)
                            )
                        } else {
                            homeViewModel.setContact(
                                getString(R.string.failed_to_retrieve_contact)
                            )
                        }
                    }

                    else -> {
                        homeViewModel.setContact(
                            getString(R.string.failed_to_retrieve_contact)
                        )
                    }
                }
            }
        }
    }

    private fun retrieveHomeFragmentData() {
        runBlocking {
            launch {
                /* Get the ecb identifier */
                homeViewModel.setIdentifier(
                    sharedData.getECBIdentifier(applicationContext)
                )

                /* Get the software version */
                retrieveSoftwareVersion()
                retrieveHardwareVersion()

                /* Get the owner and contact */
                retrieveOwnerInfo()
                retrieveContactInfo()
            }
        }
    }

    override fun onDeviceConnected(status: ECBBluetoothManager.ECBBleError) {

        runOnUiThread {
            hideConnectDialog()
            if (status == ECBBluetoothManager.ECBBleError.SUCCESS) {
                /* Send Ping command */
                commandManager?.sendPing {
                    result: CommandManager.CommandError, response: CommandManager.ECBResponse? ->

                    runOnUiThread {

                        if (result != CommandManager.CommandError.SUCCESS ||
                            response == null ||
                            response.response.toString(Charsets.US_ASCII) != "PONG"
                        ) {
                            /* We were not able to connect, launch the connect activity */
                            Toast.makeText(
                                applicationContext,
                                getString(R.string.unable_to_connect_to_econbadge),
                                Toast.LENGTH_SHORT
                            ).show()

                            /* Start the connect activity */
                            startActivity(
                                Intent(applicationContext, ConnectActivity::class.java).apply {
                                    putExtra("ecbTokenIncorrect", true)
                                }
                            )

                            finish()
                        }
                        else {

                            /* Update the current fragment */
                            retrieveHomeFragmentData()
                        }
                    }
                }
            }
            else {
                /* We were not able to connect, launch the connect activity */
                Toast.makeText(
                    applicationContext,
                    getString(R.string.unable_to_connect_to_econbadge),
                    Toast.LENGTH_SHORT
                ).show()

                /* Start the connect activity */
                startActivity(Intent(applicationContext, ConnectActivity::class.java).apply {
                    putExtra("ecbIdentifierIncorrect", true)
                }
                )

                finish()
            }
        }
    }

    override fun onDeviceDisconnected(status: ECBBluetoothManager.ECBBleError) {
        Log.d(MODULE_NAME, "Device disconnected with status $status")

        runOnUiThread {

            hideConnectDialog()

            /* Try reconnecting if disconnect was not requested */
            if (!disconnectRequested) {
                displayConnectDialog()
                bleManager?.connect()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        /* Close any initial dialog */
        connectDialog?.dismiss()

        /* Get the view models */
        homeViewModel = ViewModelProvider(this)[HomeViewModel::class.java]

        /* Check permissions */
        enableBluetooth()

        /* Check if we can connect to the ECB */
        displayConnectDialog()
        bleManager = ECBBluetoothManager.getInstance(applicationContext, this)
        bleManager?.connect()
        commandManager = CommandManager(bleManager!!, applicationContext)

        /* Bindings and content view */
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        /* Setup toolbar */
        setSupportActionBar(binding.appBarMain.toolbar)
        val materialShapeDrawable = binding.appBarMain.toolbar.background as MaterialShapeDrawable
        materialShapeDrawable.shapeAppearanceModel = materialShapeDrawable.shapeAppearanceModel
            .toBuilder()
            .setAllCorners(CornerFamily.ROUNDED, 120f)
            .build()

        /* Setup navigation */
        val drawerLayout: DrawerLayout = binding.drawerLayout
        val navView: NavigationView = binding.navView
        val navController = findNavController(R.id.nav_host_fragment_content_main)

        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.nav_home,
                R.id.nav_gallery,
                R.id.nav_slideshow
            ), drawerLayout
        )
        setupActionBarWithNavController(navController, appBarConfiguration)
        navView.setupWithNavController(navController)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment_content_main)
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
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
        when {
            containsPermanentDenial -> {
                requestRelevantRuntimePermissions()
            }
            containsDenial -> {
                requestRelevantRuntimePermissions()
            }
            else -> {
                // Unexpected scenario encountered when handling permissions
                recreate()
            }
        }
    }

    fun getCommandManager() : CommandManager {
        return commandManager!!
    }

    fun getHomeViewModel() : HomeViewModel {
        return homeViewModel
    }
}