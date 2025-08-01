package dev.olsontek.econbadge.ui.ledborder

import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.FrameLayout
import androidx.recyclerview.widget.DividerItemDecoration
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetBehavior.STATE_EXPANDED
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.google.android.material.button.MaterialButton
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.divider.MaterialDividerItemDecoration
import com.google.android.material.slider.Slider
import com.google.android.material.textfield.TextInputEditText
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderAnimation
import dev.olsontek.econbadge.data.LedBorderPattern
import dev.olsontek.econbadge.data.LedBorderScene

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

    /* List adapters */
    private lateinit var patternsListAdapter: SceneItemListAdapter
    private lateinit var animationsListAdapter: SceneItemListAdapter

    /* List of items models for the recyclerviews */
    private val patternsList = ArrayList<SceneItemListModel>()
    private val animationsList = ArrayList<SceneItemListModel>()

    /* List of selected items */
    private val selectedPatterns = HashSet<Int>()
    private val selectedAnimations = HashSet<Int>()

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun validateCompat(update: Boolean) {
        val ledIdsSet = HashSet<Int>()
        val patterns = ecbManager.getLedBorderViewModel().patternValues.value!!
        /* Build the used leds range */
        for (patternId in selectedPatterns) {
            val pattern = patterns[patternId]!!
            for (i in pattern.startLedIdx..pattern.endLedIdx) {
                ledIdsSet.add(i)
            }
        }

        /* Update the pattern models accordingly */
        for (patternModel in patternsList) {
            if (!selectedPatterns.contains(patternModel.itemId)) {
                patternModel.isUsed = false
                var found = false
                for (i in patternModel.startLedIdx..patternModel.endLedIdx) {
                    if (ledIdsSet.contains(i)) {
                        patternModel.isCompatible = false
                        found = true
                        break
                    }
                }
                if (!found) {
                    patternModel.isCompatible = true
                }
            }
            else {
                patternModel.isUsed = true
            }
        }
        if (update) {
            patternsListAdapter.notifyDataSetChanged()
        }
    }

    private fun setupPatterns() {
        val ledBorderViewModel = ecbManager.getLedBorderViewModel()

        /* Add all patterns to list */
        selectedPatterns.clear()
        patternsList.clear()
        for ((id, pattern) in ledBorderViewModel.patternValues.value!!){
            var description = if (pattern.type == LedBorderPattern.PatternType.PLAIN) {
                "Plain"
            } else {
                "Gradient"
            }

            description += " | " +
                    pattern.startLedIdx.toString() +
                    " -> " +
                    pattern.endLedIdx.toString() +
                    " |"

            val isUsed = scene.patterns.contains(id)
            if (isUsed) {
                selectedPatterns.add(id)
            }

            patternsList.add(
                SceneItemListModel(
                    id,
                    pattern.name,
                    description,
                    false,
                    isUsed,
                    pattern.startColorCode,
                    pattern.endColorCode,
                    pattern.type == LedBorderPattern.PatternType.GRADIENT,
                    true,
                    pattern.startLedIdx,
                    pattern.endLedIdx)
            )
        }

        /* Setup UI */
        patternsListAdapter = SceneItemListAdapter(
            patternsList,
            selectedPatterns) {
            validateCompat(true)
        }

        patternsListView.adapter = patternsListAdapter
        patternsListView.setLayoutManager(LinearLayoutManager(context));
        patternsListView.addItemDecoration(
            MaterialDividerItemDecoration(
                requireContext(),
                DividerItemDecoration.VERTICAL
            ).apply {
                dividerInsetEnd = 0
                dividerInsetStart = 0
            }
        )

        /* Manage compatible patterns */
        validateCompat(true)
    }

    private fun setupAnimations() {
        val ledBorderViewModel = ecbManager.getLedBorderViewModel()

        /* Add all animations to list */
        animationsList.clear()
        for ((id, animation) in ledBorderViewModel.animationValues.value!!){
            var description = if (animation.type == LedBorderAnimation.AnimationType.TRAIL) {
                "Trail"
            } else {
                "Breath"
            }

            description += " | " +
                    animation.startLedIdx.toString() +
                    " -> " +
                    animation.endLedIdx.toString() +
                    " | Speed: " +
                    animation.speed.toString()

            animationsList.add(
                SceneItemListModel(
                    id,
                    animation.name,
                    description,
                    true,
                    scene.animations.contains(id),
                    0,
                    0,
                    false,
                    true,
                    0,
                    0)
            )
        }

        /* Setup UI */
        selectedAnimations.clear()
        animationsListAdapter = SceneItemListAdapter(
            animationsList,
            selectedAnimations){

        }

        animationsListView.adapter = animationsListAdapter
        animationsListView.setLayoutManager(LinearLayoutManager(context));
        animationsListView.addItemDecoration(
            MaterialDividerItemDecoration(
                requireContext(),
                DividerItemDecoration.VERTICAL
            ).apply {
                dividerInsetEnd = 0
                dividerInsetStart = 0
            }
        )
    }

    private fun setupBrightness() {
        brightnessSlider.valueFrom = 0.0f
        brightnessSlider.valueTo = 100.0f

        when (scene.brightness) {
            25 -> {
                brightnessButtonGroup.check(R.id.brightnessEcoButton)
                brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
                brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
                brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
                brightnessSlider.visibility = View.GONE
            }
            50 -> {
                brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
                brightnessButtonGroup.check(R.id.brightnessMedButton)
                brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
                brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
                brightnessSlider.visibility = View.GONE
            }
            100 -> {
                brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
                brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
                brightnessButtonGroup.check(R.id.brightnessHigButton)
                brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
                brightnessSlider.visibility = View.GONE
            }
            else -> {
                brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
                brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
                brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
                brightnessButtonGroup.check(R.id.brightnessCusButton)
                brightnessSlider.visibility = View.VISIBLE

                brightnessSlider.value = scene.brightness.toFloat()
            }
        }

        ecoBrightnessButton.setOnClickListener {
            brightnessButtonGroup.check(R.id.brightnessEcoButton)
            brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
            brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
            brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
            brightnessSlider.visibility = View.GONE
            brightnessSlider.value = 25.0f
        }

        mediumBrightnessButton.setOnClickListener {
            brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
            brightnessButtonGroup.check(R.id.brightnessMedButton)
            brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
            brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
            brightnessSlider.visibility = View.GONE
            brightnessSlider.value = 50.0f
        }

        highBrightnessButton.setOnClickListener {
            brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
            brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
            brightnessButtonGroup.check(R.id.brightnessHigButton)
            brightnessButtonGroup.uncheck(R.id.brightnessCusButton)
            brightnessSlider.visibility = View.GONE
            brightnessSlider.value = 100.0f
        }

        customBrightnessButton.setOnClickListener {
            brightnessButtonGroup.uncheck(R.id.brightnessEcoButton)
            brightnessButtonGroup.uncheck(R.id.brightnessMedButton)
            brightnessButtonGroup.uncheck(R.id.brightnessHigButton)
            brightnessButtonGroup.check(R.id.brightnessCusButton)
            brightnessSlider.visibility = View.VISIBLE

            brightnessSlider.value = scene.brightness.toFloat()
        }
    }

    private fun setupButtons() {
        /* Set the animation name */
        if (scene.uniqueId == -1) {
            sceneNameEditText.setText("New Scene")
        } else if (scene.name.isEmpty()) {
            sceneNameEditText.setText(scene.uniqueId.toString())
        } else {
            sceneNameEditText.setText(scene.name)
        }

        sceneNameEditText.addTextChangedListener (
            object : TextWatcher {
                override fun afterTextChanged(s: Editable?) {
                }

                override fun beforeTextChanged(
                    s: CharSequence?,
                    start: Int,
                    count: Int,
                    after: Int
                ) {
                }

                override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                    scene.name = sceneNameEditText.text.toString()
                }
            }
        )

        setSceneButton.setOnClickListener {
            scene.name = sceneNameEditText.text.toString()

            /* Update selected animations and patters */
            scene.patterns.clear()
            scene.patterns.addAll(selectedPatterns.toList())
            scene.animations.clear()
            scene.animations.addAll(selectedAnimations.toList())

            /* Set brightness */
            scene.brightness = brightnessSlider.value.toInt()

            ecbManager.addScene(scene)

            dismiss()
        }
        if (scene.uniqueId == -1) {
            setSceneButton.text = "Add scene"
            removeSceneButton.text = "Cancel"


            removeSceneButton.setOnClickListener {
                dismiss()
            }
        } else {
            setSceneButton.text = "Update scene"
            removeSceneButton.text = "Remove scene"

            removeSceneButton.setOnClickListener {
                ecbManager.removeScene(scene)
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
            R.layout.scene_bottom_sheet,
            container, false
        )

        val standardBottomSheet = v.findViewById<FrameLayout>(R.id.sceneBottomSheetFrame)
        BottomSheetBehavior.from(standardBottomSheet).state = STATE_EXPANDED

        /* Get the components */
        animationsListView = v.findViewById(R.id.animationList)
        patternsListView = v.findViewById(R.id.patternsList)
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
        setupPatterns()
        setupAnimations()
        setupBrightness()
        setupButtons()

        return v
    }
}