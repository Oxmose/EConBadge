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
import android.view.MenuItem
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.navigation.findNavController
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.NavigationUI
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import com.google.android.material.navigation.NavigationView
import com.google.android.material.shape.CornerFamily
import com.google.android.material.shape.MaterialShapeDrawable
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.databinding.ActivityMainBinding
import dev.olsontek.econbadge.ui.AboutBottomSheet
import dev.olsontek.econbadge.ui.login.ConnectActivity

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
class MainActivity : AppCompatActivity(), ECBManager.ECBEventHandler {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* App bar configuration object */
    private lateinit var appBarConfiguration: AppBarConfiguration

    /* Main layout binding object */
    private lateinit var binding: ActivityMainBinding

    /* Connection information dialog */
    private var connectDialog: AlertDialog? = null

    /* Bluetooth enable result handler */
    private val bluetoothEnablingResult = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            Log.i(MODULE_NAME, "Bluetooth is enabled")
        } else {
            Log.i(MODULE_NAME, "Bluetooth is not enabled")
            enableBluetooth()
        }
    }

    /* Disconnection request flag */
    private var disconnectRequested = false

    /* eConBadge manager center */
    private lateinit var manager: ECBManager

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
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

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onConnect(status: ECBBluetoothManager.ECBBleError) {
        runOnUiThread {
            hideConnectDialog()

            if (status != ECBBluetoothManager.ECBBleError.SUCCESS) {
                /* We were not able to connect, launch the connect activity */
                Toast.makeText(
                    applicationContext,
                    getString(R.string.unable_to_connect_to_econbadge),
                    Toast.LENGTH_SHORT
                ).show()

                /* Start the connect activity */
                startActivity(Intent(applicationContext, ConnectActivity::class.java).apply {
                    putExtra("ecbIdentifierIncorrect", true)
                })

                finish()
            }
        }
    }

    override fun onDisconnect(status: ECBBluetoothManager.ECBBleError) {
        runOnUiThread {
            hideConnectDialog()

            /* Try reconnecting if disconnect was not requested */
            if (!disconnectRequested) {
                displayConnectDialog()
                manager.ecbConnect()
            } else {
                /* Start the connect activity */
                startActivity(Intent(applicationContext, ConnectActivity::class.java))
                finish()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        disconnectRequested = false

        /* Init the manager */
        manager = ECBManager.getInstance(this)

        /* Check permissions */
        enableBluetooth()

        /* Check if we can connect to the ECB */
        displayConnectDialog()
        manager.addEventListener(this)
        manager.ecbConnect()

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
        val navHostFragment =
            supportFragmentManager.findFragmentById(R.id.navHostFragmentContentMain) as NavHostFragment
        val navController = navHostFragment.navController

        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.navHome,
                R.id.navEInk,
                R.id.navBorder,
                R.id.navDisconnect
            ), drawerLayout
        )
        setupActionBarWithNavController(navController, appBarConfiguration)
        navView.setupWithNavController(navController)

        /* Setup the navigation header */
        val navHeaderTextview = navView.getHeaderView(0)
            .findViewById<TextView>(R.id.navHeaderTitleTextview)

        manager.getHomeViewModel().ecbIdentifierText.observe(this) {
            navHeaderTextview.text = getString(R.string.econbadge_hwuid, it)
        }

        /* Setup the disconnect button */
        navView.setNavigationItemSelectedListener { item: MenuItem ->
            if (item.itemId == R.id.navDisconnect) {
                disconnectRequested = true
                manager.ecbDisconnect()
            } else {
                NavigationUI.onNavDestinationSelected(item, navController)
            }
            drawerLayout.closeDrawer(GravityCompat.START)
            return@setNavigationItemSelectedListener true
        }
    }

    override fun onDestroy() {
        hideConnectDialog()

        super.onDestroy()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menuAboutButton -> {
                val aboutBottomSheet = AboutBottomSheet()
                aboutBottomSheet.show(supportFragmentManager, getString(R.string.about))
            }

            else -> {
                /* Nothing to do */
            }
        }

        return super.onOptionsItemSelected(item)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.navHostFragmentContentMain)
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
}