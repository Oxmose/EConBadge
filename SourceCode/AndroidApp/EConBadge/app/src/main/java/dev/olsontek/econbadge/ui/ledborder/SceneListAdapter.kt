package dev.olsontek.econbadge.ui.ledborder

import android.content.Context
import android.util.Log
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.content.res.AppCompatResources
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.fragment.app.FragmentManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.button.MaterialButton
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderScene

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "SceneListAdapter"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class SceneListAdapter(
    private val context: Context,
    private val sceneList: List<SceneListModel>,
    private val fragmentManager: FragmentManager,
    private val ecbManager: ECBManager
) : RecyclerView.Adapter<SceneListAdapter.ViewHolder>() {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val sceneLayout: ConstraintLayout = itemView.findViewById(R.id.ledborderItemLayout)
        val sceneTitle: MaterialTextView = itemView.findViewById(R.id.ledborderItemTitle)
        val sceneDescription: MaterialTextView = itemView.findViewById(R.id.ledborderItemDescription)
        val sceneDeleteButton: MaterialButton = itemView.findViewById(R.id.ledborderItemDelete)
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
            val item = sceneList[position]
            val sceneData = ecbManager.getScene(item.sceneId)

            holder.sceneTitle.text = item.sceneName
            holder.sceneDescription.text = item.sceneDescription
            holder.sceneTitle.gravity = Gravity.LEFT
            holder.sceneDescription.gravity = Gravity.LEFT
            holder.sceneDescription.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.delete
                )
            )
            holder.sceneDeleteButton.background.setTint(
                context.resources.getColor(R.color.red_400)
            )
            holder.sceneDeleteButton.setOnClickListener {
                ecbManager.removeScene(item.sceneId)
            }
            holder.sceneLayout.setOnClickListener {
                if (sceneData != null) {
                    val sceneBottomSheet = SceneBottomSheet(sceneData, ecbManager)
                    sceneBottomSheet.show(fragmentManager, "")
                }
                else {
                    Log.e(MODULE_NAME, "Scene should not be null")
                }
            }
        } else {
            /* Add scene item */
            holder.sceneTitle.text = context.getString(R.string.add_scene)
            holder.sceneTitle.gravity = Gravity.CENTER
            holder.sceneDescription.text = context.getString(R.string.add_a_new_scene)
            holder.sceneDescription.gravity = Gravity.CENTER
            holder.sceneDescription.setBackgroundDrawable(
                AppCompatResources.getDrawable(
                    context,
                    R.drawable.add
                )
            )
            holder.sceneDeleteButton.background.setTintList(null)
            holder.sceneLayout.setOnClickListener {
                val sceneBottomSheet = SceneBottomSheet(LedBorderScene(), ecbManager)
                sceneBottomSheet.show(fragmentManager, "")
            }
        }
    }

    override fun getItemCount(): Int {
        return sceneList.size
    }
}