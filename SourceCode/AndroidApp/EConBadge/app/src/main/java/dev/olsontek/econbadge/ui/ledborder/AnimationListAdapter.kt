package dev.olsontek.econbadge.ui.ledborder

import android.content.Context
import android.util.Log
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.content.res.AppCompatResources
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.graphics.drawable.toBitmap
import androidx.fragment.app.FragmentManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.button.MaterialButton
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderAnimation

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "AnimationListAdapter"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class AnimationListAdapter(
    private val context: Context,
    private val animationList: List<AnimationListModel>,
    private val fragmentManager: FragmentManager,
    private val ecbManager: ECBManager
) : RecyclerView.Adapter<AnimationListAdapter.ViewHolder>() {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val animationLayout: ConstraintLayout = itemView.findViewById(R.id.ledborderItemLayout)
        val animationTitle: MaterialTextView = itemView.findViewById(R.id.ledborderItemTitle)
        val animationDescription: MaterialTextView = itemView.findViewById(R.id.ledborderItemDescription)
        val animationDeleteButton: MaterialButton = itemView.findViewById(R.id.ledborderItemDelete)

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
        holder.startColorBox.visibility = View.GONE
        holder.endColorBox.visibility = View.GONE

        if (position != 0) {
            val item = animationList[position]

            holder.animationTitle.text = item.animName
            holder.animationDescription.text = item.animDescription
            holder.animationTitle.gravity = Gravity.LEFT
            holder.animationDescription.gravity = Gravity.LEFT
            holder.animationDeleteButton.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.delete
                )
            )
            holder.animationDeleteButton.background.setTint(
                context.resources.getColor(R.color.red_400)
            )
            holder.animationDeleteButton.setOnClickListener {
                ecbManager.removeAnimation(item.animId)
            }
            holder.animationLayout.setOnClickListener {
                val animationData = ecbManager.getAnimation(item.animId)
                if (animationData != null) {
                    val animationBottomSheet = AnimationBottomSheet(animationData, ecbManager)
                    animationBottomSheet.show(fragmentManager, "")
                }
                else {
                    Log.e(MODULE_NAME, "Animation should not be null")
                }
            }
        } else {
            /* Add animation item */
            holder.animationTitle.text = context.getString(R.string.add_animation)
            holder.animationTitle.gravity = Gravity.CENTER
            holder.animationDescription.text = context.getString(R.string.add_a_new_animation)
            holder.animationDescription.gravity = Gravity.CENTER
            holder.animationDeleteButton.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.add
                )
            )
            holder.animationDeleteButton.background.setTintList(null)
            holder.animationLayout.setOnClickListener {
                val animationBottomSheet = AnimationBottomSheet(LedBorderAnimation(), ecbManager)
                animationBottomSheet.show(fragmentManager, "")
            }
        }
    }

    override fun getItemCount(): Int {
        return animationList.size
    }
}