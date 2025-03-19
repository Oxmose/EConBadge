package dev.olsontek.econbadge.ui.eink

import android.app.AlertDialog
import android.content.DialogInterface
import android.graphics.Bitmap
import android.net.Uri
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.ImageView
import android.widget.LinearLayout
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.widget.addTextChangedListener
import androidx.lifecycle.Observer
import com.canhub.cropper.CropImageOptions
import com.canhub.cropper.CropImageView
import com.google.android.material.button.MaterialButton
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.checkbox.MaterialCheckBox
import com.google.android.material.slider.Slider
import com.google.android.material.slider.Slider.OnSliderTouchListener
import com.google.android.material.textfield.TextInputEditText
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.EInkImage
import dev.olsontek.econbadge.data.ImageManager
import dev.olsontek.econbadge.databinding.ActivityImageImportBinding

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

class ImageImportActivity : AppCompatActivity() {

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

    /* Main layout binding object */
    private lateinit var binding: ActivityImageImportBinding

    /* EInk viewmodel */
    private lateinit var einkViewModel: EInkViewModel

    /* Interface elements */
    private lateinit var cancelButton: Button

    private lateinit var selectButton: Button
    private lateinit var cropButton: Button
    private lateinit var ditherButton: Button
    private lateinit var importButton: Button

    private lateinit var stepsButtonGroup: MaterialButtonToggleGroup

    private lateinit var startImportTextView: MaterialTextView

    private lateinit var cropImageLayout: LinearLayout
    private lateinit var cropImageView: CropImageView
    private lateinit var cropImageValidateButton: Button

    private lateinit var ditherImageLayout: LinearLayout
    private lateinit var ditherImageView: ImageView
    private lateinit var ditherImageValidateButton: Button
    private lateinit var ditherImageDiffusionSlider: Slider
    private lateinit var ditherImageDiffusionTextView: MaterialTextView

    private lateinit var importImageView: ImageView
    private lateinit var importImageLayout: LinearLayout
    private lateinit var imageNameTextView: TextInputEditText
    private lateinit var sendToBadgeCheckbox: MaterialCheckBox
    private lateinit var validateButton: MaterialButton

    /* Data observers */
    private var importStarted = false

    private val initImageObserver = Observer<Bitmap?> {
        if (!importStarted) {
            return@Observer
        }
        if (it != null) {
            stepsButtonGroup.check(R.id.selectImageButton)
            stepsButtonGroup.check(R.id.cropImageButton)

            cropImageView.setImageBitmap(it)
            cropImageLayout.visibility = View.VISIBLE
            startImportTextView.visibility = View.GONE

            /* Check if the image is already cropped */
            if (imageManager.isEInkImageSizeReady(it)) {
                /* Set crop image directly */
                einkViewModel.setImportImageBitmapCropData(it)
            }

        } else {
            resetImageSelection()
        }
    }

    private val croppedImageObserver = Observer<Bitmap?> {
        if (!importStarted) {
            return@Observer
        }

        if (it != null) {
            stepsButtonGroup.check(R.id.ditherImageButton)

            cropImageLayout.visibility = View.GONE
            ditherImageLayout.visibility = View.VISIBLE

            /* Set first dithered image */
            einkViewModel.setImportImageBitmapTmpDitherData(
                imageManager.ditherImage(
                    ditherImageDiffusionSlider.value.toInt(),
                    it
                )
            )

            /* Check if the image is already dithered */
            if (imageManager.isEInkImageDithered(it)) {
                /* Set dithered image directly */
                einkViewModel.setImportImageBitmapDitherData(it)
            }
        } else {
            resetDitheringImage()
            cropImageLayout.visibility = View.VISIBLE
        }
    }

    private val tmpDitheredImageObserver = Observer<Bitmap?> {
        if (!importStarted) {
            return@Observer
        }

        ditherImageView.setImageBitmap(it)
    }

    private val ditheredImageObserver = Observer<Bitmap?> {

        if (!importStarted) {
            return@Observer
        }
        if (it != null) {
            stepsButtonGroup.check(R.id.importImageButton)

            importImageView.setImageBitmap(it)

            ditherImageLayout.visibility = View.GONE
            importImageLayout.visibility = View.VISIBLE
        } else {
            resetImportImage()
            ditherImageLayout.visibility = View.VISIBLE
        }
    }

    /* Image manager */
    private lateinit var imageManager: ImageManager

    /* eConBadge manager */
    private lateinit var ecbManager: ECBManager

    /* Activity manager */
    private var resultLauncher = registerForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        val imageOpenResult = imageManager.openEInkImage(uri)
        if (imageOpenResult.second != EInkImage.ImageStatus.SUCCESS) {
            resetImageSelection()
            einkViewModel.setImportImageBitmapBaseData(null)
            stepsButtonGroup.clearChecked()
            val strError: String
            when (imageOpenResult.second) {

                EInkImage.ImageStatus.IMAGE_DOES_NO_EXISTS -> {
                    strError = getString(R.string.image_does_not_exist)
                }

                EInkImage.ImageStatus.INVALID_IMAGE -> {
                    strError = getString(R.string.image_is_invalid)
                }

                EInkImage.ImageStatus.INVALID_IMAGE_SIZE -> {
                    strError =
                        getString(
                            R.string.image_too_small,
                            EInkImage.getImageWidth(),
                            EInkImage.getImageHeight()
                        )
                }

                else -> {
                    strError = getString(R.string.unknown_error)
                }
            }

            val infoDialog = AlertDialog.Builder(this)
                .setTitle(getString(R.string.error))
                .setMessage(getString(R.string.error_while_loading_image, strError))
                .setCancelable(false)
                .setNeutralButton(getString(R.string.ok)) { dialog: DialogInterface?, _: Int ->
                    dialog?.dismiss()
                }
                .create()
            infoDialog.show()
            infoDialog.getButton(DialogInterface.BUTTON_NEUTRAL)?.setTextColor(
                ContextCompat.getColor(this, R.color.grey_50)
            )
        } else {
            einkViewModel.setImportImageBitmapBaseData(imageOpenResult.first)
        }
    }

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun resetImageSelection() {
        /* Reset the crop image */
        resetCropImage()

        /* Reset UI */
        stepsButtonGroup.uncheck(R.id.selectImageButton)
        startImportTextView.visibility = View.VISIBLE
    }

    private fun resetCropImage() {
        /* Reset the dithering image */
        resetDitheringImage()

        /* Reset UI */
        stepsButtonGroup.uncheck(R.id.cropImageButton)
        cropImageLayout.visibility = View.GONE
    }

    private fun resetDitheringImage() {
        /* Reset the import */
        resetImportImage()

        /* Reset UI */
        stepsButtonGroup.uncheck(R.id.ditherImageButton)
        ditherImageLayout.visibility = View.GONE
    }

    private fun resetImportImage() {
        /* Reset UI */
        stepsButtonGroup.uncheck(R.id.importImageButton)
        importImageLayout.visibility = View.GONE
    }

    private fun selectImage() {
        /* Request file picker */
        resultLauncher.launch("*/*")
    }

    private fun getComponents() {
        cancelButton = binding.cancelImportButton

        stepsButtonGroup = binding.stepsButtonGroup
        selectButton = binding.selectImageButton
        cropButton = binding.cropImageButton
        ditherButton = binding.ditherImageButton
        importButton = binding.importImageButton

        startImportTextView = binding.startImportTextView

        cropImageLayout = binding.cropImageLayout
        cropImageView = binding.cropImageView
        cropImageValidateButton = binding.validateCropButton

        ditherImageLayout = binding.ditherImageLayout
        ditherImageView = binding.ditherImageView
        ditherImageValidateButton = binding.validateDitherButton
        ditherImageDiffusionSlider = binding.ditherImageDiffusionSlider
        ditherImageDiffusionTextView = binding.ditherImageDiffusionTextView

        importImageView = binding.importImageView
        importImageLayout = binding.importImageLayout
        validateButton = binding.validateImportButton
        imageNameTextView = binding.imageNameEditText
        sendToBadgeCheckbox = binding.sendToBadgeCheckbox
    }

    private fun setupSelectImage() {
        /* Setup the cancel button */
        cancelButton.setOnClickListener {
            /* Reset data */
            resetView()
            finish()
        }

        /* Setup the top buttons bar */
        selectButton.setOnClickListener {
            /* Start import */
            importStarted = true
            selectImage()
            resetImageSelection()
        }

        einkViewModel.importImageBitmapBaseData.observeForever(initImageObserver)
    }

    private fun setupCrop() {
        cropImageLayout.visibility = View.GONE

        /* Setup the top buttons bar */
        cropButton.setOnClickListener {
            if (!stepsButtonGroup.checkedButtonIds.contains(R.id.cropImageButton)) {
                stepsButtonGroup.check(R.id.cropImageButton)
                einkViewModel.setImportImageBitmapCropData(null)
            } else {
                stepsButtonGroup.uncheck(R.id.cropImageButton)
            }
        }

        /* Setup the crop image view */
        cropImageView.apply {
            val options = CropImageOptions()
            options.cropShape = CropImageView.CropShape.RECTANGLE
            options.guidelines = CropImageView.Guidelines.ON
            options.autoZoomEnabled = true
            options.centerMoveEnabled = true
            options.showCropOverlay = true
            options.showProgressBar = true
            options.minCropResultWidth = EInkImage.getImageWidth()
            options.minCropResultHeight = EInkImage.getImageHeight()
            options.cornerShape = CropImageView.CropCornerShape.RECTANGLE
            setImageCropOptions(options)
            setAspectRatio(EInkImage.getImageWidth(), EInkImage.getImageHeight())
        }

        /* Setup the validate button */
        cropImageValidateButton.setOnClickListener {
            /* Scale the image */
            var scaledImage = cropImageView.getCroppedImage()
                ?.copy(Bitmap.Config.ARGB_8888, true)
            scaledImage = imageManager.scaleImage(scaledImage!!)
            einkViewModel.setImportImageBitmapCropData(scaledImage)
        }

        einkViewModel.importImageBitmapCropData.observeForever(croppedImageObserver)
    }

    private fun setupDither() {

        ditherImageLayout.visibility = View.GONE
        ditherButton.setOnClickListener {
            if (!stepsButtonGroup.checkedButtonIds.contains(R.id.ditherImageButton)) {
                stepsButtonGroup.check(R.id.ditherImageButton)
                einkViewModel.setImportImageBitmapDitherData(null)
            } else {
                stepsButtonGroup.uncheck(R.id.ditherImageButton)
            }
        }

        /* Setup the validate button */
        ditherImageValidateButton.setOnClickListener {
            einkViewModel.setImportImageBitmapDitherData(
                einkViewModel.importImageBitmapDitherTmpData.value
            )
        }

        /* Setup the diffusion slider and textview */
        ditherImageDiffusionSlider.stepSize = 1.0f
        ditherImageDiffusionSlider.valueFrom = 0.0f
        ditherImageDiffusionSlider.valueTo = 100.0f
        ditherImageDiffusionSlider.value = 100.0f
        ditherImageDiffusionSlider.addOnChangeListener { _, value, _ ->
            ditherImageDiffusionTextView.text = getString(R.string.diffusion, value.toInt())


        }
        val touchListener = object : OnSliderTouchListener {
            override fun onStartTrackingTouch(slider: Slider) {

            }

            override fun onStopTrackingTouch(slider: Slider) {
                einkViewModel.setImportImageBitmapTmpDitherData(
                    imageManager.ditherImage(
                        ditherImageDiffusionSlider.value.toInt(),
                        einkViewModel.importImageBitmapCropData.value!!
                    )
                )
            }
        }
        ditherImageDiffusionSlider.addOnSliderTouchListener(touchListener)
        ditherImageDiffusionTextView.text = getString(
            R.string.diffusion,
            ditherImageDiffusionSlider.value.toInt()
        )


        einkViewModel.importImageBitmapDitherTmpData.observeForever(tmpDitheredImageObserver)
        einkViewModel.importImageBitmapDitherData.observeForever(ditheredImageObserver)
    }

    private fun setupImport() {
        importImageLayout.visibility = View.GONE
        importButton.setOnClickListener {
            if (!stepsButtonGroup.checkedButtonIds.contains(R.id.importImageButton)) {
                stepsButtonGroup.check(R.id.importImageButton)
            } else {
                stepsButtonGroup.uncheck(R.id.importImageButton)
            }
        }

        imageNameTextView.addTextChangedListener {
            validateImageName()
        }

        /* Setup the validate button */
        validateButton.isEnabled = false
        validateButton.setOnClickListener {
            val currentImageBitmap = einkViewModel.importImageBitmapDitherData.value
            if (currentImageBitmap == null) {
                val infoDialog = AlertDialog.Builder(this, R.style.DialogStyle)
                    .setCancelable(false)
                    .setTitle(getString(R.string.error))
                    .setMessage(
                        getString(R.string.an_error_occurred_when_saving_the_image)
                    )
                    .setNeutralButton(getString(R.string.ok)) { dialog: DialogInterface?, _: Int ->
                        dialog?.dismiss()
                    }
                    .create()

                infoDialog?.show()
                infoDialog?.getButton(DialogInterface.BUTTON_NEUTRAL)?.setTextColor(
                    ContextCompat.getColor(applicationContext, R.color.grey_50)
                )
                return@setOnClickListener
            }

            /* Validate image name */
            if (!imageManager.validateName(imageNameTextView.text.toString())) {
                validateImageName()
                return@setOnClickListener
            }

            sendEInkImage(currentImageBitmap, false)
        }
    }

    private fun validateImageName() {
        if (!imageManager.validateName(imageNameTextView.text.toString())) {
            imageNameTextView.error =
                "The image name must contain between 1 and 61 ASCII characters."
            validateButton.isEnabled = false
        } else {
            validateButton.isEnabled = true
            imageNameTextView.error = null
        }
    }

    private fun sendEInkImage(currentImageBitmap: Bitmap, forceOverWrite: Boolean) {

        var infoDialog: AlertDialog? = null
        if (sendToBadgeCheckbox.isChecked) {
            infoDialog = AlertDialog.Builder(this)
                .setTitle(getString(R.string.updating_image))
                .setMessage(getString(R.string.setting_new_image))
                .setCancelable(false)
                .create()
            infoDialog.show()
        }

        ecbManager.addEInkImage(
            currentImageBitmap,
            imageNameTextView.text.toString(),
            forceOverWrite,
            sendToBadgeCheckbox.isChecked
        ) { imageStatus: EInkImage.ImageStatus ->
            infoDialog?.dismiss()
            when (imageStatus) {
                EInkImage.ImageStatus.SUCCESS -> {
                    resetView()
                    finish()
                }

                EInkImage.ImageStatus.IMAGE_ALREADY_EXISTS -> {
                    infoDialog = AlertDialog.Builder(this, R.style.DialogStyle)
                        .setCancelable(false)
                        .setTitle(getString(R.string.image_overwrite))
                        .setMessage(
                            getString(R.string.the_image_name_you_specified_already_exists)
                        )
                        .setPositiveButton(getString(R.string.yes)) { dialog: DialogInterface?, _: Int ->
                            sendEInkImage(currentImageBitmap, true)
                            dialog?.dismiss()
                        }
                        .setNegativeButton(getString(R.string.no)) { dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        .create()

                    infoDialog?.show()
                    infoDialog?.getButton(DialogInterface.BUTTON_NEGATIVE)?.setTextColor(
                        ContextCompat.getColor(applicationContext, R.color.grey_50)
                    )
                    infoDialog?.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                        ContextCompat.getColor(applicationContext, R.color.grey_50)
                    )
                }

                else -> {
                    infoDialog = AlertDialog.Builder(this, R.style.DialogStyle)
                        .setCancelable(false)
                        .setTitle(getString(R.string.error))
                        .setMessage(
                            getString(R.string.an_error_occurred_when_saving_the_image)
                        )
                        .setNeutralButton(getString(R.string.ok)) { dialog: DialogInterface?, _: Int ->
                            dialog?.dismiss()
                        }
                        .create()

                    infoDialog?.show()
                    infoDialog?.getButton(DialogInterface.BUTTON_NEUTRAL)?.setTextColor(
                        ContextCompat.getColor(applicationContext, R.color.grey_50)
                    )
                }
            }
        }
    }

    private fun resetView() {
        einkViewModel.importImageBitmapBaseData.removeObserver(initImageObserver)
        einkViewModel.importImageBitmapCropData.removeObserver(croppedImageObserver)
        einkViewModel.importImageBitmapDitherData.removeObserver(ditheredImageObserver)
        einkViewModel.importImageBitmapDitherTmpData.removeObserver(tmpDitheredImageObserver)

        resetImageSelection()
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()

        /* Init managers */
        ecbManager = ECBManager.getInstance(null)
        imageManager = ImageManager(contentResolver)

        einkViewModel = ecbManager.getEInkViewModel()

        importStarted = false

        /* Bindings and content view */
        binding = ActivityImageImportBinding.inflate(layoutInflater)
        setContentView(binding.root)

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        /* Get the components */
        getComponents()

        /* Setup interface */
        setupSelectImage()
        setupCrop()
        setupDither()
        setupImport()
    }

    override fun onDestroy() {
        einkViewModel.importImageBitmapBaseData.removeObserver(initImageObserver)
        einkViewModel.importImageBitmapBaseData.removeObserver(croppedImageObserver)
        einkViewModel.importImageBitmapBaseData.removeObserver(ditheredImageObserver)
        einkViewModel.importImageBitmapBaseData.removeObserver(tmpDitheredImageObserver)

        super.onDestroy()
    }
}