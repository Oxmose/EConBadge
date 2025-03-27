package dev.olsontek.econbadge.ui.ledborder

import android.content.Context
import android.util.Log
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.appcompat.content.res.AppCompatResources
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.graphics.drawable.toBitmap
import androidx.fragment.app.FragmentManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.button.MaterialButton
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderPattern

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "PatternListAdapter"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class PatternListAdapter(
    private val context: Context,
    private val patternList: List<PatternListModel>,
    private val fragmentManager: FragmentManager,
    private val ecbManager: ECBManager
) : RecyclerView.Adapter<PatternListAdapter.ViewHolder>() {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val patternLayout: ConstraintLayout = itemView.findViewById(R.id.ledborderItemLayout)
        val patternTitle: MaterialTextView = itemView.findViewById(R.id.ledborderItemTitle)
        val patternDescription: MaterialTextView = itemView.findViewById(R.id.ledborderItemDescription)
        val patternDeleteButton: MaterialButton = itemView.findViewById(R.id.ledborderItemDelete)
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
            .inflate(R.layout.ledborder_list_item, parent, false)

        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        if (position != 0) {
            val item = patternList[position]
            val patternData = ecbManager.getPattern(item.patternId)

            if (patternData != null) {
                holder.startColorBox.visibility = View.VISIBLE
                holder.startColorBox.setBackgroundColor(patternData.startColorCode or 0xFF000000.toInt())

                if (patternData.type == LedBorderPattern.PatternType.GRADIENT) {
                    holder.endColorBox.visibility = View.VISIBLE
                    holder.endColorBox.setBackgroundColor(patternData.endColorCode or 0xFF000000.toInt())
                } else {
                    holder.endColorBox.visibility = View.GONE
                }
            }

            holder.patternTitle.text = item.patternName
            holder.patternDescription.text = item.patternDescription
            holder.patternTitle.gravity = Gravity.LEFT
            holder.patternDescription.gravity = Gravity.LEFT
            holder.patternDeleteButton.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.delete
                )
            )
            holder.patternDeleteButton.background.setTint(
                context.resources.getColor(R.color.red_400)
            )
            holder.patternDeleteButton.setOnClickListener {
                ecbManager.removePattern(item.patternId)
            }
            holder.patternLayout.setOnClickListener {
                if (patternData != null) {
                    val patternBottomSheet = PatternBottomSheet(patternData, ecbManager)
                    patternBottomSheet.show(fragmentManager, "")
                }
                else {
                    Log.e(MODULE_NAME, "Pattern should not be null")
                }
            }
        } else {

            holder.startColorBox.visibility = View.GONE
            holder.endColorBox.visibility = View.GONE

            /* Add pattern item */
            holder.patternTitle.text = context.getString(R.string.add_pattern)
            holder.patternTitle.gravity = Gravity.CENTER
            holder.patternDescription.text = context.getString(R.string.add_a_new_pattern)
            holder.patternDescription.gravity = Gravity.CENTER
            holder.patternDeleteButton.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.add
                )
            )
            holder.patternDeleteButton.background.setTintList(null)
            holder.patternLayout.setOnClickListener {
                val patternBottomSheet = PatternBottomSheet(LedBorderPattern(), ecbManager)
                patternBottomSheet.show(fragmentManager, "")
            }
        }
    }

    override fun getItemCount(): Int {
        return patternList.size
    }
}