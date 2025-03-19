package dev.olsontek.econbadge.ui.home

import android.app.AlertDialog
import android.content.DialogInterface
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import androidx.core.content.ContextCompat
import androidx.core.widget.addTextChangedListener
import androidx.fragment.app.Fragment
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.textview.MaterialTextView
import com.techiness.progressdialoglibrary.ProgressDialog
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.databinding.FragmentHomeBinding

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class HomeFragment : Fragment(), ECBManager.ECBEventHandler {
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

    /* Fragment bindings */
    private var binding: FragmentHomeBinding? = null

    /* Binding getter */
    private val bindingGet get() = binding!!

    /* Fragment view model that contains the data to display */
    private lateinit var homeViewModel: HomeViewModel

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* Fragment components */
    private lateinit var ecbIdentifierTextView: MaterialTextView
    private lateinit var ecbSoftwareVersionTextView: MaterialTextView
    private lateinit var ecbHardWareVersionTextView: MaterialTextView
    private lateinit var ecbTokenEditText: EditText
    private lateinit var updateEcbTokenButton: Button
    private lateinit var ecbOwnerEditText: EditText
    private lateinit var ecbContactEditText: EditText
    private lateinit var updateEcbOwnerContactButton: Button
    private lateinit var updateFirmwareButton: Button
    private lateinit var factoryResetButton: Button

    /* Fragment inflater and container */
    private lateinit var savedInflater: LayoutInflater
    private var savedContainer: ViewGroup? = null

    /* Information snackbar */
    private var infoSnack: Snackbar? = null

    /* Information retrieval dialog */
    private lateinit var infoRetrieveDialog: AlertDialog

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun validateToken() {
        val tokenValue = ecbTokenEditText.text.toString()
        updateEcbTokenButton.isEnabled = ecbManager.validateToken(
            tokenValue
        )
        if (tokenValue.isNotEmpty() && !updateEcbTokenButton.isEnabled) {
            ecbTokenEditText.error = getString(R.string.token_size_requirement)
        }
    }

    private fun validateOwnerContact() {
        if (!ecbManager.validateOwnerContact(ecbOwnerEditText.text.toString())) {
            ecbOwnerEditText.error = getString(R.string.owner_string_requirement)
            updateEcbOwnerContactButton.isEnabled = false
        }
        else if (!ecbManager.validateOwnerContact(ecbContactEditText.text.toString())) {
            ecbContactEditText.error = getString(R.string.contact_string_requirement)
            updateEcbOwnerContactButton.isEnabled = false
        }
        else {
            updateEcbOwnerContactButton.isEnabled = true
            ecbOwnerEditText.error = null
            ecbContactEditText.error = null
        }
    }

    private fun requestUpdateToken() {

        val tokenValue = ecbTokenEditText.text.toString()

        /* Validate the current token */
        if (!ecbManager.validateToken(tokenValue)) {
            ecbTokenEditText.error = getString(R.string.token_size_requirement)
            return
        }

        /* Request the token change */
        ecbManager.requestTokenChange(tokenValue) {
                result: CommandManager.CommandStatus ->

           if (result == CommandManager.CommandStatus.SUCCESS) {
               requireActivity().runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(R.string.token_updated_successfully),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()

                    /* Clear token */
                    ecbTokenEditText.text.clear()
                }
           }
           else {
               requireActivity().runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_token_snack,
                            ecbManager.getStringError(result)
                        ),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()
                }
           }
        }
    }

    private fun requestUpdateOwner() {
        val value = ecbOwnerEditText.text.toString()

        /* Validate the current token */
        if (!ecbManager.validateOwnerContact(value)) {
            ecbOwnerEditText.error = getString(R.string.owner_string_requirement)
            return
        }

        /* Request the token change */
        ecbManager.requestOwnerChange(value) {
                result: CommandManager.CommandStatus ->

            if (result != CommandManager.CommandStatus.SUCCESS) {
                requireActivity().runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_owner_snack,
                            ecbManager.getStringError(result)
                        ),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()
                }
            }
        }
    }

    private fun requestUpdateContact() {

        val value = ecbContactEditText.text.toString()

        /* Validate the current token */
        if (!ecbManager.validateOwnerContact(value)) {
            ecbContactEditText.error = getString(R.string.contact_string_requirement)
            return
        }

        /* Request the token change */
        ecbManager.requestContactChange(value) {
                result: CommandManager.CommandStatus ->

            if (result != CommandManager.CommandStatus.SUCCESS) {
                requireActivity().runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_contact_snack,
                            ecbManager.getStringError(result)
                        ),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()
                }
            }
        }
    }

    private fun performUpdate() {
        val progressDialog = ProgressDialog(requireActivity())
        progressDialog.isCancelable = false
        progressDialog.mode = ProgressDialog.MODE_DETERMINATE
        progressDialog.maxValue = 10000
        ecbManager.performUpdate(
            progressCallback = {
                    status: CommandManager.CommandStatus,
                    progress: Float ->
                requireActivity().runOnUiThread {
                    when (status) {
                        CommandManager.CommandStatus.DOWNLOADING_UPDATE -> {
                            progressDialog.setMessage(getString(R.string.downloading_firmware))
                        }

                        CommandManager.CommandStatus.SENDING_UPDATE -> {
                            progressDialog.setMessage(getString(R.string.uploading_the_firmware))
                        }

                        else -> {
                            progressDialog.setMessage(getString(R.string.update))
                        }
                    }

                    progressDialog.progress = (progress * 10000).toInt()
                    progressDialog.show()
                }
            },
            updateEndCallback = {
                    status: CommandManager.CommandStatus ->
                requireActivity().runOnUiThread {
                    progressDialog.dismiss()

                    var strError: String? = null
                    infoSnack?.dismiss()
                    if (status == CommandManager.CommandStatus.SUCCESS) {
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(R.string.awaiting_for_the_econbadge_to_reboot),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                    else {
                        strError = getString(R.string.update_failed_with_error) +
                                ecbManager.getStringError(status)
                    }

                    if (strError != null) {
                        val builder = AlertDialog.Builder(requireActivity(), R.style.DialogStyle)

                        /* Open the dialog */
                        builder.setTitle(getString(R.string.update))
                        builder.setMessage(strError)
                        builder.setCancelable(true)
                        builder.setPositiveButton(getString(R.string.ok)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        val errorDialog: AlertDialog? = builder.create()
                        errorDialog?.show()
                    }

                }
            }
        )
    }

    private fun initializeUpdate() {
        val builder = AlertDialog.Builder(requireActivity(), R.style.DialogStyle)
        var connectDialog: AlertDialog? = null

        requireActivity().runOnUiThread {
            /* Open the dialog */
            builder.setTitle(getString(R.string.update))
            builder.setMessage(getString(R.string.checking_for_update))
            builder.setCancelable(false)
            connectDialog = builder.create()
            connectDialog?.show()
        }

        /* Start the updater */
        ecbManager.requestUpdate {
            result: CommandManager.CommandStatus, lastVersion: String ->
            requireActivity().runOnUiThread {
                connectDialog?.dismiss()
                when (result) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        builder.setTitle(getString(R.string.update_available))
                        builder.setMessage(
                            getString(R.string.a_new_firmware_version_is_available) +
                                    lastVersion +
                            getString(R.string.do_you_want_to_update)
                        )
                        builder.setPositiveButton(getString(R.string.yes)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                            performUpdate()
                        }
                        builder.setNegativeButton(getString(R.string.no)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                        connectDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                        connectDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))

                    }

                    CommandManager.CommandStatus.NO_UPDATE_AVAILABLE -> {
                        builder.setTitle(getString(R.string.already_up_to_date))
                        builder.setMessage(
                            getString(R.string.your_badge_already_has_the_latest_firmware_version)
                        )
                        builder.setPositiveButton(getString(R.string.ok)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                        connectDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                        connectDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                    }

                    CommandManager.CommandStatus.HW_NOT_COMPATIBLE -> {
                        builder.setTitle(getString(R.string.incompatible_hardware))
                        builder.setMessage(
                            getString(R.string.the_current_hardware_is_incompatible)
                        )
                        builder.setPositiveButton(getString(R.string.ok)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                        connectDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                        connectDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                    }

                    else -> {
                        builder.setTitle(getString(R.string.failed_to_get_update_information))
                        builder.setMessage(
                            getString(R.string.an_error_occurred_while_trying_to)
                        )
                        builder.setPositiveButton(getString(R.string.ok)) {
                            dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                        connectDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                        connectDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                    }
                }
            }
        }
    }

    private fun requestFactoryReset() {
        val builder = AlertDialog.Builder(requireActivity(), R.style.DialogStyle)
        var validateDialog: AlertDialog?

        builder.setTitle(getString(R.string.factory_reset))
        builder.setMessage(getString(R.string.confirm_factory_reset))
        builder.setPositiveButton(getString(R.string.yes)) {
            dialog: DialogInterface?, _: Int ->
            dialog?.dismiss()

            val infoBuilder = AlertDialog.Builder(requireActivity(), R.style.DialogStyle)

            infoBuilder.setCancelable(false)
            infoBuilder.setTitle(getString(R.string.factory_reset))
            infoBuilder.setMessage(getString(R.string.factory_reset_in_progress))
            validateDialog = infoBuilder.create()
            validateDialog?.show()

            /* Request the token change */
            ecbManager.requestFactoryReset {
                    result: CommandManager.CommandStatus ->
                requireActivity().runOnUiThread {
                    validateDialog?.dismiss()
                    infoSnack?.dismiss()
                    if (result == CommandManager.CommandStatus.SUCCESS) {
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(R.string.factory_reset_success),
                            Snackbar.LENGTH_LONG
                        )
                    }
                    else {
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(
                                R.string.factory_reset_failed,
                                ecbManager.getStringError(result)
                            ),
                            Snackbar.LENGTH_LONG
                        )
                    }
                    infoSnack?.show()
                }
            }
        }
        builder.setNegativeButton(getString(R.string.no)) {
            dialog: DialogInterface?, _: Int ->
            dialog?.dismiss()
        }
        builder.setCancelable(false)
        validateDialog = builder.create()
        validateDialog?.show()
        validateDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
            ContextCompat.getColor(requireActivity(), R.color.grey_50))
        validateDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
            ContextCompat.getColor(requireActivity(), R.color.grey_50))
    }

    private fun displayConnectDialog() {
        val builder = AlertDialog.Builder(requireActivity())
        builder.setTitle(getString(R.string.retrieving_data))
        builder.setMessage(getString(R.string.retrieving_data_please_wait))
        builder.setCancelable(false)

        infoRetrieveDialog = builder.create()
        infoRetrieveDialog.show()
    }

    private fun hideConnectDialog() {
        infoRetrieveDialog.dismiss()
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        savedInflater = inflater
        savedContainer = container

        ecbManager = ECBManager.getInstance(null)
        homeViewModel = ecbManager.getHomeViewModel()
        ecbManager.addEventListener(this)

        binding = FragmentHomeBinding.inflate(savedInflater, savedContainer, false)

        /* Get the activity components */
        ecbIdentifierTextView = bindingGet.ecbIdentifierInfoMaterialTextView
        ecbSoftwareVersionTextView = bindingGet.ecbSwVersionInfoMaterialTextView
        ecbHardWareVersionTextView = bindingGet.ecbHwVersionInfoMaterialTextView

        ecbTokenEditText = bindingGet.ecbTokenEditText
        updateEcbTokenButton = bindingGet.updateTokenButton

        ecbOwnerEditText = bindingGet.ecbOwnerEditText
        ecbContactEditText = bindingGet.ecbContactEditText
        updateEcbOwnerContactButton = bindingGet.updateOwnerContactButton

        updateFirmwareButton = bindingGet.checkUpdateButton
        factoryResetButton = bindingGet.factoryResetButton

        /* Setup the viewmodel interactions */
        homeViewModel.ecbIdentifierText.observe(viewLifecycleOwner) {
            ecbIdentifierTextView.text = it
        }
        homeViewModel.ecbSoftwareVersionText.observe(viewLifecycleOwner) {
            ecbSoftwareVersionTextView.text = it
        }
        homeViewModel.ecbHardwareVersionText.observe(viewLifecycleOwner) {
            ecbHardWareVersionTextView.text = it
        }
        homeViewModel.ecbTokenText.observe(viewLifecycleOwner) {
            ecbTokenEditText.setText(it)
        }
        homeViewModel.ecbOwnerText.observe(viewLifecycleOwner) {
            ecbOwnerEditText.setText(it)
        }
        homeViewModel.ecbContactText.observe(viewLifecycleOwner) {
            ecbContactEditText.setText(it)
        }

        /* Setup the token interaction */
        ecbTokenEditText.addTextChangedListener {
            validateToken()
        }
        updateEcbTokenButton.setOnClickListener {
            requestUpdateToken()
        }

        /* Setup the contact and owner interaction */
        ecbOwnerEditText.addTextChangedListener {
            validateOwnerContact()
        }
        ecbContactEditText.addTextChangedListener {
            validateOwnerContact()
        }
        updateEcbOwnerContactButton.setOnClickListener {
            requestUpdateOwner()
            requestUpdateContact()
        }

        /* Setup update interaction */
        updateFirmwareButton.setOnClickListener {
            initializeUpdate()
        }

        /* Setup factory reset interaction */
        factoryResetButton.setOnClickListener {
            requestFactoryReset()
        }

        return bindingGet.root
    }

    override fun onConnect(status: ECBBluetoothManager.ECBBleError) {
        /* Get data */
        requireActivity().runOnUiThread {
            displayConnectDialog()
            ecbManager.retrieveHomeFragmentData {
                hideConnectDialog()
            }
        }
    }

    override fun onDisconnect(status: ECBBluetoothManager.ECBBleError) {
        /* Nothing to so */
    }

    override fun onDestroyView() {
        super.onDestroyView()
        hideConnectDialog()
        binding = null
    }
}