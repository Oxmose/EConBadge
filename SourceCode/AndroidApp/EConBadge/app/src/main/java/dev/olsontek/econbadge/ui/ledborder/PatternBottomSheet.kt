package dev.olsontek.econbadge.ui.ledborder

import android.graphics.Color
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.NumberPicker
import com.github.dhaval2404.colorpicker.ColorPickerDialog
import com.github.dhaval2404.colorpicker.MaterialColorPickerDialog
import com.github.dhaval2404.colorpicker.model.ColorShape
import com.github.dhaval2404.colorpicker.model.ColorSwatch
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetBehavior.STATE_EXPANDED
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import com.google.android.material.button.MaterialButton
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.textview.MaterialTextView
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.data.ECBManager
import dev.olsontek.econbadge.data.LedBorderPattern

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class PatternBottomSheet(
    private val pattern: LedBorderPattern,
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
    private lateinit var plainTypeButton: Button
    private lateinit var gradientTypeButton: Button

    private lateinit var typeButtonGroup: MaterialButtonToggleGroup

    private lateinit var patternColorEndTextView: MaterialTextView

    private lateinit var startColorButton: MaterialButton
    private lateinit var endColorButton: MaterialButton

    private lateinit var startLedIdxPicker: NumberPicker
    private lateinit var endLedIdxPicker: NumberPicker

    private lateinit var nameEditText: EditText

    private lateinit var updatePatternButton: MaterialButton
    private lateinit var removePatternButton: MaterialButton

    /* Current start / end colors */
    private var currentStartColor = pattern.startColorCode
    private var currentEndColor = pattern.endColorCode

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun setupType() {
        /* Setup preliminary selection */
        if (pattern.uniqueId != -1) {
            when (pattern.type) {
                LedBorderPattern.PatternType.PLAIN -> {
                    typeButtonGroup.check(R.id.plainPatternButton)
                    plainTypeButton.isSelected = true
                    gradientTypeButton.isSelected = false

                    endColorButton.visibility = View.GONE
                    patternColorEndTextView.visibility = View.GONE
                }
                else -> {
                    typeButtonGroup.check(R.id.gradientPatternButton)
                    gradientTypeButton.isSelected = true
                    plainTypeButton.isSelected = false

                    endColorButton.visibility = View.VISIBLE
                    patternColorEndTextView.visibility = View.VISIBLE
                }
            }
        }
        else {
            typeButtonGroup.check(R.id.plainPatternButton)
            typeButtonGroup.uncheck(R.id.gradientPatternButton)
            plainTypeButton.isSelected = true
            gradientTypeButton.isSelected = false

            endColorButton.visibility = View.GONE
            patternColorEndTextView.visibility = View.GONE
        }

        /* Setup action */
        plainTypeButton.setOnClickListener {
            typeButtonGroup.uncheck(R.id.gradientPatternButton)
            typeButtonGroup.check(R.id.plainPatternButton)

            plainTypeButton.isSelected = true
            gradientTypeButton.isSelected = false

            endColorButton.visibility = View.GONE
            patternColorEndTextView.visibility = View.GONE
        }
        gradientTypeButton.setOnClickListener {

            plainTypeButton.isSelected = false
            gradientTypeButton.isSelected = true

            typeButtonGroup.uncheck(R.id.plainPatternButton)
            typeButtonGroup.check(R.id.gradientPatternButton)

            endColorButton.visibility = View.VISIBLE
            patternColorEndTextView.visibility = View.VISIBLE
        }
    }

    private fun setupColors() {
        startColorButton.setBackgroundColor(currentStartColor or 0xFF000000.toInt())
        startColorButton.setOnClickListener {
            ColorPickerDialog.Builder(requireActivity())
                .setTitle("Choose a color")
                .setColorShape(ColorShape.CIRCLE)
                .setDefaultColor(currentStartColor or 0xFF000000.toInt())
                .setColorListener {
                    color, _ ->
                    currentStartColor = color
                    startColorButton.setBackgroundColor(currentStartColor or 0xFF000000.toInt())
                }
                .show()
        }

        endColorButton.setBackgroundColor(currentEndColor or 0xFF000000.toInt())
        endColorButton.setOnClickListener {
            ColorPickerDialog.Builder(requireActivity())
                    .setTitle("Choose a color")
                    .setColorShape(ColorShape.CIRCLE)
                    .setDefaultColor(currentEndColor or 0xFF000000.toInt())

                    .setColorListener {
                            color, _ ->
                        currentEndColor = color
                        endColorButton.setBackgroundColor(currentEndColor or 0xFF000000.toInt())
                    }
                    .show()
            }
    }

    private fun setupLEDIndices() {
        val maxLedCount = ecbManager.getLedBorderCount()

        val startIdx: Int
        val endIdx: Int
        if (pattern.startLedIdx >= pattern.endLedIdx) {
            startIdx = 0
            endIdx = maxLedCount - 1
        } else {
            startIdx = pattern.startLedIdx
            endIdx = pattern.endLedIdx
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
        if (pattern.uniqueId == -1) {
            nameEditText.setText("New Pattern")
        } else if (pattern.name.isEmpty()) {
            nameEditText.setText(pattern.uniqueId.toString())
        } else {
            nameEditText.setText(pattern.name)
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
                    pattern.name = nameEditText.text.toString()
                }
            }
        )

        updatePatternButton.setOnClickListener {
            pattern.name = nameEditText.text.toString()
            pattern.startColorCode = currentStartColor
            pattern.endColorCode = currentEndColor
            pattern.startLedIdx = startLedIdxPicker.value
            pattern.endLedIdx = endLedIdxPicker.value
            pattern.type = if (plainTypeButton.isSelected) {
                LedBorderPattern.PatternType.PLAIN
            } else {
                LedBorderPattern.PatternType.GRADIENT
            }

            ecbManager.addPattern(pattern)
            dismiss()
        }
        if (pattern.uniqueId == -1) {
            updatePatternButton.text = "Add pattern"
            removePatternButton.text = "Cancel"


            removePatternButton.setOnClickListener {
                dismiss()
            }
        } else {
            updatePatternButton.text = "Update pattern"
            removePatternButton.text = "Remove pattern"

            removePatternButton.setOnClickListener {
                ecbManager.removePattern(pattern)
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
            R.layout.pattern_bottom_sheet,
            container, false
        )

        val standardBottomSheet = v.findViewById<FrameLayout>(R.id.patternBottomSheetFrame)
        BottomSheetBehavior.from(standardBottomSheet).state = STATE_EXPANDED

        /* Get the components */
        typeButtonGroup = v.findViewById(R.id.patternTypeGroup)
        plainTypeButton = v.findViewById(R.id.plainPatternButton)
        gradientTypeButton = v.findViewById(R.id.gradientPatternButton)
        patternColorEndTextView = v.findViewById(R.id.patternColorEndTextView)
        startColorButton = v.findViewById(R.id.patternColorStartButton)
        endColorButton = v.findViewById(R.id.patternColorEndButton)
        startLedIdxPicker = v.findViewById(R.id.patternStartLedPicker)
        endLedIdxPicker = v.findViewById(R.id.patternEndLedPicker)
        nameEditText = v.findViewById(R.id.patternNameEditText)
        updatePatternButton = v.findViewById(R.id.setPatternButton)
        removePatternButton = v.findViewById(R.id.removePatternButton)

        /* Setup the components */
        setupType()
        setupColors()
        setupLEDIndices()
        setupButtons()

        return v
    }
}