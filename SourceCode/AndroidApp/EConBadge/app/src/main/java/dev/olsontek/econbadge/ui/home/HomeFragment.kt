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
import dev.olsontek.econbadge.MainActivity
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.data.Updater
import dev.olsontek.econbadge.databinding.FragmentHomeBinding

class HomeFragment : Fragment() {

    private var binding: FragmentHomeBinding? = null

    private val bindingGet get() = binding!!

    private lateinit var homeViewModel: HomeViewModel

    private lateinit var mainActivity: MainActivity

    private lateinit var commandManager: CommandManager

    /* Get the activity components */
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

    /* Information snackbar */
    private var infoSnack: Snackbar? = null

    private fun validateToken() {
        val tokenValue = ecbTokenEditText.text.toString()
        updateEcbTokenButton.isEnabled = commandManager.validateToken(
            tokenValue
        )
        if (tokenValue.isNotEmpty() && !updateEcbTokenButton.isEnabled) {
            ecbTokenEditText.error = getString(R.string.token_size_requirement)
        }
    }

    private fun validateOwnerContact() {
        if (!commandManager.validateOwnerContact(ecbOwnerEditText.text.toString())) {
            ecbOwnerEditText.error = getString(R.string.owner_string_requirement)
            updateEcbOwnerContactButton.isEnabled = false
        }
        else if (!commandManager.validateOwnerContact(ecbContactEditText.text.toString())) {
            ecbContactEditText.error = getString(R.string.contact_string_requirement)
            updateEcbOwnerContactButton.isEnabled = false
        }
        else {
            updateEcbOwnerContactButton.isEnabled = true
        }
    }

    private fun requestUpdateToken() {

        val tokenValue = ecbTokenEditText.text.toString()

        /* Validate the current token */
        if (!commandManager.validateToken(tokenValue)) {
            ecbTokenEditText.error = getString(R.string.token_size_requirement)
            return
        }

        /* Request the token change */
        commandManager.requestTokenChange(tokenValue) {
            result: CommandManager.CommandError, response: CommandManager.ECBResponse? ->

           if (result == CommandManager.CommandError.SUCCESS) {
                /* Check the error code result */
                if (response?.status == CommandManager.ECBCommandStatus.NO_ERROR.value) {
                    commandManager.setECBToken(tokenValue)
                    activity?.runOnUiThread {
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
                    val errorString: String
                    if (response?.status == CommandManager.ECBCommandStatus.INVALID_COMMAND_SIZE.value ||
                        response?.status == CommandManager.ECBCommandStatus.INVALID_PARAM.value) {
                        errorString = getString(R.string.token_size_requirement)
                    }
                    else {
                        errorString = commandManager.getStringError(
                            CommandManager.ECBCommandStatus.fromByte(response!!.status)
                        )
                    }
                    activity?.runOnUiThread {
                        /* Set error on token edittext */
                        ecbTokenEditText.error =
                            getString(R.string.failed_to_update_token_snak, errorString)

                        /* Display snackbar */
                        infoSnack?.dismiss()
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(
                                R.string.failed_to_update_token_snak,
                                errorString
                            ),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                }
           }
           else {
                activity?.runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_token_snak,
                            commandManager.getStringError(result)
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
        if (!commandManager.validateOwnerContact(value)) {
            ecbOwnerEditText.error = getString(R.string.owner_string_requirement)
            return
        }

        /* Request the token change */
        commandManager.requestOwnerChange(value) {
            result: CommandManager.CommandError, response: CommandManager.ECBResponse? ->

            if (result == CommandManager.CommandError.SUCCESS) {
                /* Check the error code result */
                if (response?.status == CommandManager.ECBCommandStatus.NO_ERROR.value) {
                    activity?.runOnUiThread {
                        /* Display snackbar */
                        infoSnack?.dismiss()
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(R.string.owner_successfully_updated),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                }
                else {
                    val errorString: String
                    if (response?.status == CommandManager.ECBCommandStatus.INVALID_COMMAND_SIZE.value ||
                        response?.status == CommandManager.ECBCommandStatus.INVALID_PARAM.value) {
                        errorString = getString(R.string.owner_string_requirement)
                    }
                    else {
                        errorString = commandManager.getStringError(
                            CommandManager.ECBCommandStatus.fromByte(response!!.status)
                        )
                    }
                    activity?.runOnUiThread {
                        /* Set error on token edittext */
                        ecbOwnerEditText.error =
                            getString(R.string.failed_to_update_owner_snak, errorString)

                        /* Display snackbar */
                        infoSnack?.dismiss()
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(
                                R.string.failed_to_update_owner_snak,
                                errorString
                            ),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                }
            }
            else {
                activity?.runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_owner_snak,
                            commandManager.getStringError(result)
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
        if (!commandManager.validateOwnerContact(value)) {
            ecbContactEditText.error = getString(R.string.contact_string_requirement)
            return
        }

        /* Request the token change */
        commandManager.requestContactChange(value) {
                result: CommandManager.CommandError, response: CommandManager.ECBResponse? ->

            if (result == CommandManager.CommandError.SUCCESS) {
                /* Check the error code result */
                if (response?.status == CommandManager.ECBCommandStatus.NO_ERROR.value) {
                    activity?.runOnUiThread {
                        /* Display snackbar */
                        infoSnack?.dismiss()
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(R.string.contact_successfully_updated),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                }
                else {
                    val errorString: String
                    if (response?.status == CommandManager.ECBCommandStatus.INVALID_COMMAND_SIZE.value ||
                        response?.status == CommandManager.ECBCommandStatus.INVALID_PARAM.value) {
                        errorString = getString(R.string.contact_string_requirement)
                    }
                    else {
                        errorString = commandManager.getStringError(
                            CommandManager.ECBCommandStatus.fromByte(response!!.status)
                        )
                    }
                    activity?.runOnUiThread {
                        /* Set error on token edittext */
                        ecbOwnerEditText.error =
                            getString(R.string.failed_to_update_contact_snak, errorString)

                        /* Display snackbar */
                        infoSnack?.dismiss()
                        infoSnack = Snackbar.make(
                            requireActivity().findViewById(android.R.id.content),
                            getString(
                                R.string.failed_to_update_contact_snak,
                                errorString
                            ),
                            Snackbar.LENGTH_LONG
                        )
                        infoSnack?.show()
                    }
                }
            }
            else {
                activity?.runOnUiThread {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_update_contact_snak,
                            commandManager.getStringError(result)
                        ),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()
                }
            }
        }
    }

    private fun initializeUpdate() {
        val updater = Updater(commandManager)
        val builder = AlertDialog.Builder(mainActivity, R.style.DialogStyle)
        var connectDialog: AlertDialog? = null

        activity?.runOnUiThread {
            /* Open the dialog */
            builder.setTitle(getString(R.string.update))
            builder.setMessage(getString(R.string.checking_for_update))
            builder.setCancelable(false)
            connectDialog = builder.create()
            connectDialog?.show()
        }

        /* Start the updater */
        updater.checkForUpdate(
            homeViewModel.ecbSoftwareVersionText.value!!,
            homeViewModel.ecbHardwareVersionText.value!!,
        ) {
            result: CommandManager.CommandError ->
            activity?.runOnUiThread {
                connectDialog?.dismiss()
                when (result) {
                    CommandManager.CommandError.SUCCESS -> {
                        builder.setTitle("Update available")
                        builder.setMessage(
                            "A new firmware version is available:\n    - " +
                                    updater.getLatestVersion() +
                                    "\n\nDo you want to update?"
                        )
                        builder.setPositiveButton("Yes") { _: DialogInterface?, _: Int ->
                            connectDialog?.dismiss()
                        }
                        builder.setNegativeButton("No") { _: DialogInterface?, _: Int ->
                            connectDialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                        connectDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))
                        connectDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                            ContextCompat.getColor(requireActivity(), R.color.grey_50))

                    }

                    CommandManager.CommandError.NO_UPDATE_AVAILABLE -> {
                        builder.setTitle("Already up to date")
                        builder.setMessage(
                            "Your badge already has the latest firmware version."
                        )
                        builder.setPositiveButton("Ok") { _: DialogInterface?, _: Int ->
                            connectDialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                    }

                    CommandManager.CommandError.HW_NOT_COMPATIBLE -> {
                        builder.setTitle("Incompatible hardware")
                        builder.setMessage(
                            "The current hardware is incompatible with the new firmware update."
                        )
                        builder.setPositiveButton("Ok") { _: DialogInterface?, _: Int ->
                            connectDialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                    }

                    else -> {
                        builder.setTitle("Failed to get update information")
                        builder.setMessage(
                            "An error occurred while trying to retrieve firmware update information. Please try again later"
                        )
                        builder.setPositiveButton("Ok") { _: DialogInterface?, _: Int ->
                            connectDialog?.dismiss()
                        }
                        builder.setCancelable(false)
                        connectDialog = builder.create()
                        connectDialog?.show()
                    }
                }
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        mainActivity =  activity as MainActivity
        homeViewModel = mainActivity.getHomeViewModel()
        commandManager = mainActivity.getCommandManager()

        binding = FragmentHomeBinding.inflate(inflater, container, false)
        val root: View = bindingGet.root

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

        return root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }
}