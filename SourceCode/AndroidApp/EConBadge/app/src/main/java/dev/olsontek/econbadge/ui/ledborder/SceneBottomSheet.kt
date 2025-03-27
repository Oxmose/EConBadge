package dev.olsontek.econbadge.ui.ledborder

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.NumberPicker
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetBehavior.STATE_EXPANDED
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.google.android.material.button.MaterialButton
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.materialswitch.MaterialSwitch
import com.google.android.material.slider.Slider
import com.google.android.material.textfield.TextInputEditText
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorder
import dev.olsontek.econbadge.data.LedBorderAnimation
import dev.olsontek.econbadge.data.LedBorderScene
import kotlin.math.max

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class SceneBottomSheet(
    private val scene: LedBorderScene,
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
    private lateinit var animationsListView: RecyclerView
    private lateinit var patternsListView: RecyclerView
    private lateinit var ecoBrightnessButton: Button
    private lateinit var mediumBrightnessButton: Button
    private lateinit var highBrightnessButton: Button
    private lateinit var customBrightnessButton: Button
    private lateinit var brightnessButtonGroup: MaterialButtonToggleGroup
    private lateinit var brightnessSlider: Slider
    private lateinit var sceneNameEditText: TextInputEditText
    private lateinit var setSceneButton: MaterialButton
    private lateinit var removeSceneButton: MaterialButton

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/


    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val v: View = inflater.inflate(
            R.layout.scene_bottom_sheet,
            container, false
        )

        val standardBottomSheet = v.findViewById<FrameLayout>(R.id.sceneBottomSheetFrame)
        BottomSheetBehavior.from(standardBottomSheet).state = STATE_EXPANDED

        /* Get the components */
        animationsListView = v.findViewById(R.id.animationList)
        patternsListView = v.findViewById(R.id.patternList)
        ecoBrightnessButton = v.findViewById(R.id.brightnessEcoButton)
        mediumBrightnessButton = v.findViewById(R.id.brightnessMedButton)
        highBrightnessButton = v.findViewById(R.id.brightnessHigButton)
        customBrightnessButton = v.findViewById(R.id.brightnessCusButton)
        brightnessButtonGroup = v.findViewById(R.id.brightnessGroup)
        brightnessSlider = v.findViewById(R.id.brightnessSlider)
        sceneNameEditText = v.findViewById(R.id.sceneNameEditText)
        setSceneButton = v.findViewById(R.id.setSceneButton)
        removeSceneButton = v.findViewById(R.id.removeSceneButton)

        /* Setup the components */

        return v
    }
}