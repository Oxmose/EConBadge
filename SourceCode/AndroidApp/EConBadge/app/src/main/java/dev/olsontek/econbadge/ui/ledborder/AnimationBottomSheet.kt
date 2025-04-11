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
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetBehavior.STATE_EXPANDED
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.google.android.material.button.MaterialButton
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.materialswitch.MaterialSwitch
import com.google.android.material.slider.Slider
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorder
import dev.olsontek.econbadge.data.LedBorderAnimation
import kotlin.math.max

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class AnimationBottomSheet(
    private val animation: LedBorderAnimation,
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
    private lateinit var trailTypeButton: Button
    private lateinit var breathTypeButton: Button

    private lateinit var typeButtonGroup: MaterialButtonToggleGroup

    private lateinit var directionSwitch: MaterialSwitch

    private lateinit var animationSpeedSlider: Slider
    private lateinit var startLedIdxPicker: NumberPicker
    private lateinit var endLedIdxPicker: NumberPicker

    private lateinit var nameEditText: EditText

    private lateinit var updateAnimButton: MaterialButton
    private lateinit var removeAnimButton: MaterialButton

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun setupType() {
        /* Setup preliminary selection */
        if (animation.uniqueId != -1) {
            when (animation.type) {
                LedBorderAnimation.AnimationType.TRAIL -> {
                    typeButtonGroup.check(R.id.trailAnimationButton)
                    trailTypeButton.isSelected = true
                    breathTypeButton.isSelected = false

                    directionSwitch.visibility = View.VISIBLE
                }
                else -> {
                    typeButtonGroup.check(R.id.breathAnimationButton)
                    breathTypeButton.isSelected = true
                    trailTypeButton.isSelected = false

                    directionSwitch.visibility = View.GONE
                }
            }
        }
        else {
            typeButtonGroup.check(R.id.trailAnimationButton)
            typeButtonGroup.uncheck(R.id.breathAnimationButton)
            trailTypeButton.isSelected = true
            breathTypeButton.isSelected = false

            directionSwitch.visibility = View.VISIBLE
        }

        /* Setup action */
        trailTypeButton.setOnClickListener {
            typeButtonGroup.uncheck(R.id.breathAnimationButton)
            typeButtonGroup.check(R.id.trailAnimationButton)

            trailTypeButton.isSelected = true
            breathTypeButton.isSelected = false

            directionSwitch.visibility = View.VISIBLE
        }
        breathTypeButton.setOnClickListener {

            trailTypeButton.isSelected = false
            breathTypeButton.isSelected = true

            typeButtonGroup.uncheck(R.id.trailAnimationButton)
            typeButtonGroup.check(R.id.breathAnimationButton)

            directionSwitch.visibility = View.GONE
        }

        directionSwitch.isChecked = animation.direction != 0
    }

    private fun setupSpeed() {
        animationSpeedSlider.valueFrom = 1.0f
        animationSpeedSlider.valueTo = 100.0f
        if (animation.speed < 1.0f || animation.speed > 100.0f) {
            animation.speed = 1
        }
        animationSpeedSlider.value = animation.speed.toFloat()
    }

    private fun setupLEDIndices() {
        val maxLedCount = ecbManager.getLedBorderCount()

        val startIdx: Int
        val endIdx: Int
        if (animation.startLedIdx >= animation.endLedIdx) {
            startIdx = 0
            endIdx = maxLedCount - 1
        } else {
            startIdx = animation.startLedIdx
            endIdx = animation.endLedIdx
        }

        startLedIdxPicker.maxValue = endIdx
        startLedIdxPicker.minValue = 1
        startLedIdxPicker.value = startIdx + 1

        endLedIdxPicker.minValue = startIdx + 2
        endLedIdxPicker.maxValue = maxLedCount
        endLedIdxPicker.value = endIdx + 1

        startLedIdxPicker.setOnValueChangedListener {
                _, _, newVal ->
            if (endLedIdxPicker.value <= newVal) {
                endLedIdxPicker.value = newVal + 1
            }
            endLedIdxPicker.minValue = newVal + 1
        }

        endLedIdxPicker.setOnValueChangedListener {
                _, _, newVal ->
            if (startLedIdxPicker.value >= newVal) {
                startLedIdxPicker.value = newVal - 1
            }
            startLedIdxPicker.maxValue = newVal - 1
        }
    }

    private fun setupButtons() {
        /* Set the animation name */
        if (animation.uniqueId == -1) {
            nameEditText.setText("New Animation")
        } else if (animation.name.isEmpty()) {
            nameEditText.setText(animation.uniqueId.toString())
        } else {
            nameEditText.setText(animation.name)
        }

        nameEditText.addTextChangedListener (
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
                    animation.name = nameEditText.text.toString()
                }
            }
        )

        updateAnimButton.setOnClickListener {
            animation.name = nameEditText.text.toString()
            animation.speed = animationSpeedSlider.value.toInt()
            animation.startLedIdx = startLedIdxPicker.value
            animation.endLedIdx = endLedIdxPicker.value
            animation.direction = if (directionSwitch.isChecked) { 1 } else { 0 }
            animation.type = if (trailTypeButton.isSelected) {
                LedBorderAnimation.AnimationType.TRAIL
            } else {
                LedBorderAnimation.AnimationType.BREATH
            }

            ecbManager.addAnimation(animation)
            dismiss()
        }
        if (animation.uniqueId == -1) {
            updateAnimButton.text = "Add animation"
            removeAnimButton.text = "Cancel"


            removeAnimButton.setOnClickListener {
                dismiss()
            }
        } else {
            updateAnimButton.text = "Update animation"
            removeAnimButton.text = "Remove animation"

            removeAnimButton.setOnClickListener {
                ecbManager.removeAnimation(animation)
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
            R.layout.animation_bottom_sheet,
            container, false
        )

        val standardBottomSheet = v.findViewById<FrameLayout>(R.id.animationBottomSheetFrame)
        BottomSheetBehavior.from(standardBottomSheet).state = STATE_EXPANDED

        /* Get the components */
        typeButtonGroup = v.findViewById(R.id.animationTypeGroup)
        trailTypeButton = v.findViewById(R.id.trailAnimationButton)
        breathTypeButton = v.findViewById(R.id.breathAnimationButton)
        directionSwitch = v.findViewById(R.id.animationDirectionSwitch)
        animationSpeedSlider = v.findViewById(R.id.animationSpeedSlider)
        startLedIdxPicker = v.findViewById(R.id.animationStartLedPicker)
        endLedIdxPicker = v.findViewById(R.id.animationEndLedPicker)
        nameEditText = v.findViewById(R.id.animationNameEditText)
        updateAnimButton = v.findViewById(R.id.setAnimationButton)
        removeAnimButton = v.findViewById(R.id.removeAnimationButton)

        /* Setup the components */
        setupType()
        setupSpeed()
        setupLEDIndices()
        setupButtons()

        return v
    }
}