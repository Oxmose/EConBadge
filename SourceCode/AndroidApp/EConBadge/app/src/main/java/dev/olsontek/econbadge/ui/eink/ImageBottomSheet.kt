package dev.olsontek.econbadge.ui.eink

import android.app.AlertDialog
import android.content.DialogInterface
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.core.graphics.drawable.toBitmap
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetBehavior.STATE_EXPANDED
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.google.android.material.button.MaterialButton
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.EInkImage

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ImageBottomSheet(
    private val imageModel: EInkImageGridModel,
    private val ecbManager: ECBManager
) : BottomSheetDialogFragment() {
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

    /* Layout components */
    private lateinit var imageView: ImageView
    private lateinit var imageTitle: MaterialTextView
    private lateinit var retrieveImageButton: MaterialButton
    private lateinit var removeImageButton: MaterialButton
    private lateinit var setImageButton: MaterialButton

    /* Currently displayed eInk image */
    private var einkImage: EInkImage? = null

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/

    private fun sendSelectedImage() {
        var dialog = AlertDialog.Builder(context)
            .setTitle(getString(R.string.updating_image))
            .setMessage(getString(R.string.setting_new_image))
            .setCancelable(false)
            .create()
        dialog.show()

        ecbManager.sendEInkImage(einkImage!!) { commandStatus: CommandManager.CommandStatus ->
            dialog.dismiss()
            if (commandStatus != CommandManager.CommandStatus.SUCCESS) {
                dialog = AlertDialog.Builder(context)
                    .setTitle(getString(R.string.failed_to_set_image))
                    .setMessage(
                        getString(
                            R.string.an_error_occurred_while_setting_image_template,
                            ecbManager.getStringError(
                                commandStatus
                            )
                        )
                    )
                    .setCancelable(true)
                    .setPositiveButton(android.R.string.ok) { dialog: DialogInterface, _ ->
                        dialog.dismiss()
                    }
                    .create()
                dialog.show()
                dialog.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                    ContextCompat.getColor(requireActivity(), R.color.grey_50)
                )
            }
        }
    }

    private fun setSelectedImage() {
        var dialog = AlertDialog.Builder(context)
            .setTitle(getString(R.string.updating_image))
            .setMessage(getString(R.string.setting_new_image))
            .setCancelable(false)
            .create()
        dialog.show()

        ecbManager.setEInkImage(imageModel.imageName) { commandStatus: CommandManager.CommandStatus ->
            dialog.dismiss()
            if (commandStatus != CommandManager.CommandStatus.SUCCESS) {
                dialog = AlertDialog.Builder(context)
                    .setTitle(getString(R.string.failed_to_set_image))
                    .setMessage(
                        getString(
                            R.string.an_error_occurred_while_setting_image_template,
                            ecbManager.getStringError(
                                commandStatus
                            )
                        )
                    )
                    .setCancelable(true)
                    .setPositiveButton(android.R.string.ok) { dialog: DialogInterface, _ ->
                        dialog.dismiss()
                    }
                    .create()
                dialog.show()
                dialog.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                    ContextCompat.getColor(requireActivity(), R.color.grey_50)
                )
            }
        }
    }

    private fun retrieveImage() {
        var dialog = AlertDialog.Builder(context)
            .setTitle(getString(R.string.downloading_image_preview))
            .setMessage(getString(R.string.downloading_image_preview_please_wait))
            .setCancelable(false)
            .create()
        dialog.show()

        ecbManager.retrieveImage(imageModel.imageName) { commandStatus: CommandManager.CommandStatus ->
            dialog.dismiss()
            if (commandStatus != CommandManager.CommandStatus.SUCCESS) {
                dialog = AlertDialog.Builder(context)
                    .setTitle(getString(R.string.failed_to_retrieve_image))
                    .setMessage(
                        getString(
                            R.string.an_error_occurred_while_getting_image_template,
                            ecbManager.getStringError(
                                commandStatus
                            )
                        )
                    )
                    .setCancelable(true)
                    .setPositiveButton(android.R.string.ok) { dialog: DialogInterface, _ ->
                        dialog.dismiss()
                    }
                    .create()
                dialog.show()
                dialog.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                    ContextCompat.getColor(requireActivity(), R.color.grey_50)
                )
            } else {
                /* Update the image */
                einkImage = EInkImage.loadFromName(requireActivity(), imageModel.imageName).first
                if (einkImage == null) {
                    imageView.setImageBitmap(
                        AppCompatResources.getDrawable(
                            requireActivity(),
                            R.drawable.ic_menu_gallery
                        )?.toBitmap()
                    )
                } else {
                    imageView.setImageBitmap(einkImage!!.getBitmap())
                    retrieveImageButton.visibility = View.INVISIBLE
                }
            }
        }
    }

    private fun removeImage() {
        var dialog = AlertDialog.Builder(context)
            .setTitle(getString(R.string.removing_image))
            .setMessage(getString(R.string.removing_image_please_wait))
            .setCancelable(false)
            .create()
        dialog.show()

        ecbManager.removeImage(imageModel) { commandStatus: CommandManager.CommandStatus ->
            dialog.dismiss()
            if (commandStatus != CommandManager.CommandStatus.SUCCESS) {
                dialog = AlertDialog.Builder(context)
                    .setTitle(getString(R.string.failed_to_remove_image))
                    .setMessage(
                        getString(
                            R.string.an_error_occurred_while_removing_image_template,
                            ecbManager.getStringError(
                                commandStatus
                            )
                        )
                    )
                    .setCancelable(true)
                    .setPositiveButton(android.R.string.ok) { dialog: DialogInterface, _ ->
                        dialog.dismiss()
                    }
                    .create()
                dialog.show()
                dialog.getButton(DialogInterface.BUTTON_POSITIVE)?.setTextColor(
                    ContextCompat.getColor(requireActivity(), R.color.grey_50)
                )
            } else {
                /* Close the sheet */
                dismiss()
            }
        }
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val v: View = inflater.inflate(
            R.layout.image_bottom_sheet,
            container, false
        )

        val standardBottomSheet = v.findViewById<FrameLayout>(R.id.imageBottomSheetFrame)
        BottomSheetBehavior.from(standardBottomSheet).state = STATE_EXPANDED

        /* Retrieve the sheet elements */
        imageView = v.findViewById(R.id.imageBottomSheetImageView)
        imageTitle = v.findViewById(R.id.imageTitleTextview)
        retrieveImageButton = v.findViewById(R.id.downloadImageButton)
        removeImageButton = v.findViewById(R.id.removeImageButton)
        setImageButton = v.findViewById(R.id.setImageButton)

        /* Set the components */
        imageTitle.text = imageModel.imageName

        einkImage = imageModel.imageData
        if (einkImage == null) {
            imageView.setImageBitmap(
                AppCompatResources.getDrawable(
                    requireActivity(),
                    R.drawable.ic_menu_gallery
                )?.toBitmap()
            )
        } else {
            imageView.setImageBitmap(einkImage!!.getBitmap())
            retrieveImageButton.visibility = View.INVISIBLE
        }

        retrieveImageButton.setOnClickListener {
            retrieveImage()
        }

        setImageButton.setOnClickListener {
            if (imageModel.remoteStore) {
                setSelectedImage()
            } else if (imageModel.localStore) {
                sendSelectedImage()
            } else {
                throw Exception()
            }
        }

        removeImageButton.setOnClickListener {
            removeImage()
        }

        return v
    }
}