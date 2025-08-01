package dev.olsontek.econbadge.ui.ledborder

import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.checkbox.MaterialCheckBox
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "SceneItemListAdapter"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class SceneItemListAdapter(
    private val itemList: List<SceneItemListModel>,
    private val selectedItems: HashSet<Int>,
    private val validateCompat: () -> Unit
) : RecyclerView.Adapter<SceneItemListAdapter.ViewHolder>() {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val itemLayout: ConstraintLayout = itemView.findViewById(R.id.sceneItemLayout)
        val itemTitle: MaterialTextView = itemView.findViewById(R.id.sceneItemTitle)
        val itemDescription: MaterialTextView = itemView.findViewById(R.id.sceneItemDescription)
        val itemCheckbox: MaterialCheckBox = itemView.findViewById(R.id.sceneItemCheckbox)
        val startColorBox: View = itemView.findViewById(R.id.startColorBox)
        val endColorBox: View = itemView.findViewById(R.id.endColorBox)
    }

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
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.scene_list_item, parent, false)

        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.startColorBox.visibility = View.GONE
        holder.endColorBox.visibility = View.GONE

        val item = itemList[position]

        holder.itemTitle.text = item.itemName
        holder.itemDescription.text = item.itemDescription
        holder.itemCheckbox.isChecked = item.isUsed

        if (!item.isAnimation) {
            holder.startColorBox.visibility = View.VISIBLE
            holder.startColorBox.setBackgroundColor(item.startColorCode or 0xFF000000.toInt())

            if (item.isGradient) {
                holder.endColorBox.visibility = View.VISIBLE
                holder.endColorBox.setBackgroundColor(item.endColorCode or 0xFF000000.toInt())
            } else {
                holder.endColorBox.visibility = View.GONE
            }

            if (item.isCompatible) {
                holder.itemCheckbox.visibility = View.VISIBLE
                holder.itemLayout.setOnClickListener {
                    if (holder.itemCheckbox.isChecked) {
                        selectedItems.add(item.itemId)
                        /* Manage compatibility between patterns */
                        validateCompat()
                    } else {
                        selectedItems.remove(item.itemId)
                        /* Manage compatibility between patterns */
                        validateCompat()
                    }
                }
                holder.itemCheckbox.setOnClickListener {
                    if (holder.itemCheckbox.isChecked) {
                        selectedItems.add(item.itemId)
                        /* Manage compatibility between patterns */
                        validateCompat()
                    } else {
                        selectedItems.remove(item.itemId)
                        /* Manage compatibility between patterns */
                        validateCompat()
                    }
                }
                holder.itemDescription.text = item.itemDescription
            }
            else {
                holder.itemDescription.text = "Incompatible"
                holder.itemCheckbox.visibility = View.GONE
            }
        }
        else {
            holder.startColorBox.visibility = View.GONE
            holder.endColorBox.visibility = View.GONE

            holder.itemCheckbox.setOnClickListener {
                if (holder.itemCheckbox.isChecked) {
                    selectedItems.add(item.itemId)
                }
                else {
                    selectedItems.remove(item.itemId)
                }
            }
            holder.itemLayout.setOnClickListener {
                if (holder.itemCheckbox.isChecked) {
                    selectedItems.add(item.itemId)
                }
                else {
                    selectedItems.remove(item.itemId)
                }
            }
        }
    }

    override fun getItemCount(): Int {
        return itemList.size
    }
}