package dev.olsontek.econbadge.ui.eink

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.ImageView
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import androidx.core.graphics.drawable.toBitmap
import androidx.fragment.app.FragmentManager
import com.google.android.material.card.MaterialCardView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class EInkImageGridAdapter(
    context: Context,
    imageList: List<EInkImageGridModel>,
    private val fragmentManager: FragmentManager,
    private val ecbManager: ECBManager
) : ArrayAdapter<EInkImageGridModel?>(context, 0, imageList as List<EInkImageGridModel?>) {

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
    /* None */

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun getView(position: Int, view: View?, parent: ViewGroup): View {
        var itemView = view
        if (itemView == null) {
            itemView = LayoutInflater.from(context).inflate(
                R.layout.image_grid_item,
                parent,
                false
            )
        }

        val model: EInkImageGridModel? = getItem(position)
        val imageView = itemView!!.findViewById<ImageView>(R.id.ecbImageGridItemImageView)
        val itemLayout = itemView.findViewById<MaterialCardView>(R.id.ecbImageGridItemLayout)
        val remoteStoreIcon = itemView.findViewById<ImageView>(R.id.remoteStoreImageView)
        val localStoreIcon = itemView.findViewById<ImageView>(R.id.localStoreImageView)

        if (model!!.imageData != null) {
            imageView.setImageBitmap(model.imageData!!.getBitmap())
        } else {
            imageView.setImageBitmap(
                AppCompatResources.getDrawable(context, R.drawable.ic_menu_gallery)?.toBitmap()
            )
        }

        itemLayout.setOnClickListener {
            val aboutBottomSheet = ImageBottomSheet(model, ecbManager)
            aboutBottomSheet.show(fragmentManager, model.imageName)
        }

        /* Check if this is the current selected image */
        if (ecbManager.getCurrentImageName() == model.imageName) {
            itemLayout.strokeWidth = 6
            itemLayout.strokeColor = ContextCompat.getColor(context, R.color.teal_200)
        } else {
            itemLayout.strokeWidth = 3
            itemLayout.strokeColor = ContextCompat.getColor(context, R.color.grey_800)
        }

        if (model.remoteStore) {
            val drawable = remoteStoreIcon.drawable
            drawable.setTint(ContextCompat.getColor(context, R.color.teal_200))
        } else {
            val drawable = remoteStoreIcon.drawable
            drawable.setTint(ContextCompat.getColor(context, R.color.red_400))
        }

        if (model.localStore) {
            val drawable = localStoreIcon.drawable
            drawable.setTint(ContextCompat.getColor(context, R.color.teal_200))
        } else {
            val drawable = localStoreIcon.drawable
            drawable.setTint(ContextCompat.getColor(context, R.color.red_400))
        }

        return itemView
    }
}