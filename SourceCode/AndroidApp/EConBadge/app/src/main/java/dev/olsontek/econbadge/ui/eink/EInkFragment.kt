package dev.olsontek.econbadge.ui.eink

import android.app.AlertDialog
import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.GridView
import android.widget.ImageView
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.graphics.drawable.toBitmap
import androidx.fragment.app.Fragment
import com.google.android.material.snackbar.Snackbar
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.databinding.FragmentEinkBinding

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class EInkFragment : Fragment(), ECBManager.ECBEventHandler {

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
    private var binding: FragmentEinkBinding? = null

    /* Binding getter */
    private val bindingGet get() = binding!!

    /* Fragment view model that contains the data to display */
    private lateinit var einkViewModel: EInkViewModel

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* Fragment components */
    private lateinit var currentImageView: ImageView
    private lateinit var currentImageNameTextView: MaterialTextView
    private lateinit var clearImageButton: Button
    private lateinit var loadingImageListTextView: MaterialTextView
    private lateinit var einkImagesGridView: GridView
    private lateinit var addNewImageButton: Button

    /* Grid adapter */
    private lateinit var gridViewAdapter: EInkImageGridAdapter

    /* Fragment inflater and container */
    private lateinit var savedInflater: LayoutInflater
    private var savedContainer: ViewGroup? = null

    /* Information retrieval dialog */
    private lateinit var infoRetrieveDialog: AlertDialog

    /* Information snackbar */
    private var infoSnack: Snackbar? = null

    /* List of image models for the gridview */
    private val imageList = ArrayList<EInkImageGridModel>()

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun displayInfoDialog(title: String, content: String) {
        val builder = AlertDialog.Builder(requireActivity())
        builder.setTitle(title)
        builder.setMessage(content)
        builder.setCancelable(false)

        infoRetrieveDialog = builder.create()
        infoRetrieveDialog.show()
    }

    private fun hideInfoDialog() {
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
        einkViewModel = ecbManager.getEInkViewModel()
        ecbManager.addEventListener(this)

        binding = FragmentEinkBinding.inflate(savedInflater, savedContainer, false)

        /* Get the activity components */
        currentImageView = bindingGet.currentImageView
        currentImageNameTextView = bindingGet.currentImageTextView
        clearImageButton = bindingGet.clearImageButton
        loadingImageListTextView = bindingGet.loadingImageListTextView
        einkImagesGridView = bindingGet.ecbImagesGridView
        addNewImageButton = bindingGet.addNewImageButton

        /* Setup the viewmodel interaction */
        einkViewModel.currentImageDataValue.observe(viewLifecycleOwner) {
            if (it == null) {
                /* No image, display default */
                currentImageView.setImageBitmap(
                    AppCompatResources.getDrawable(
                        requireActivity(),
                        R.drawable.ic_menu_gallery
                    )?.toBitmap()
                )
            } else {
                currentImageView.setImageBitmap(it.getBitmap())
            }
        }
        einkViewModel.currentImageNameText.observe(viewLifecycleOwner) {
            if (it.isEmpty()) {
                currentImageNameTextView.text = getString(R.string.no_image_loaded)
            } else {
                currentImageNameTextView.text = it
            }
        }

        einkViewModel.loadingImageText.observe(viewLifecycleOwner) {
            if (it == "") {
                loadingImageListTextView.visibility = View.INVISIBLE
            } else {
                loadingImageListTextView.visibility = View.VISIBLE
            }
            loadingImageListTextView.text = it
        }

        gridViewAdapter = EInkImageGridAdapter(
            requireActivity(),
            imageList,
            requireActivity().supportFragmentManager,
            ecbManager
        )
        einkImagesGridView.adapter = gridViewAdapter
        einkViewModel.imageListData.observe(viewLifecycleOwner) {
            imageList.clear()
            imageList.addAll(it)
            gridViewAdapter.notifyDataSetChanged()
        }

        /* Setup the clear image button interaction */
        clearImageButton.setOnClickListener {
            displayInfoDialog(
                getString(R.string.eink_display),
                getString(R.string.clearing_eink_display)
            )
            ecbManager.clearEInkDisplay { result: CommandManager.CommandStatus ->
                hideInfoDialog()
                if (result != CommandManager.CommandStatus.SUCCESS) {
                    /* Display snackbar */
                    infoSnack?.dismiss()
                    infoSnack = Snackbar.make(
                        requireActivity().findViewById(android.R.id.content),
                        getString(
                            R.string.failed_to_clear_eink_display,
                            ecbManager.getStringError(result)
                        ),
                        Snackbar.LENGTH_LONG
                    )
                    infoSnack?.show()
                }
            }
        }

        /* Get the data */
        displayInfoDialog(
            getString(R.string.retrieving_data),
            getString(R.string.retrieving_data_please_wait)
        )
        ecbManager.retrieveCurrentImage {
            hideInfoDialog()
        }
        loadingImageListTextView.visibility = View.VISIBLE
        ecbManager.retrieveImageList {
            /* Nothing to do */
        }

        /* Add the new image button actions */
        addNewImageButton.setOnClickListener {
            /* Start the image add activity */
            startActivity(Intent(requireActivity(), ImageImportActivity::class.java))
        }

        return bindingGet.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }

    override fun onConnect(status: ECBBluetoothManager.ECBBleError) {
        /* Get data */
        requireActivity().runOnUiThread {
            displayInfoDialog(
                getString(R.string.retrieving_data),
                getString(R.string.retrieving_data_please_wait)
            )
            ecbManager.retrieveCurrentImage {
                hideInfoDialog()
            }
            loadingImageListTextView.visibility = View.VISIBLE
            ecbManager.retrieveImageList {
                /* Nothing to do */
            }
        }
    }

    override fun onDisconnect(status: ECBBluetoothManager.ECBBleError) {
        /* Nothing to so */
    }
}