package dev.olsontek.econbadge.ui.ledborder;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.google.android.material.switchmaterial.SwitchMaterial;
import com.skydoves.colorpickerview.ColorPickerDialog;
import com.skydoves.colorpickerview.listeners.ColorEnvelopeListener;

import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.databinding.FragmentLedborderBinding;
import dev.olsontek.econbadge.models.LedBorderPattern;
import dev.olsontek.econbadge.ui.dialog.AddAnimationActivity;
import dev.olsontek.econbadge.ui.dialog.RemoveAnimationActivity;

public class LedBorderFragment extends Fragment {

    private FragmentLedborderBinding binding_;

    private SwitchMaterial switchState_;
    private Spinner spinnerPattern_;
    private Button btnPlainColor_;
    private Button[] btnGradColorStart_;
    private Button[] btnGradColorEnd_;
    private SeekBar[] seekGradSize_;
    private TextView[] textiewGradSize_;
    private Button   btnNewAnimation_;
    private Button   btnRemoveAnimation_;
    private Button   btnClearAnimations_;
    private SeekBar seekBrightness_;

    private TextView colorPatternTitleTextView_;
    private TextView plainColorTitleTextView_;
    private TextView[] gradientTitleTextView_;
    private TextView brightnessTitleTextView_;
    private TextView brightnessValueTextView_;
    private TextView animationsTitleTextView_;

    private EConBadgeManager eConBadgeManager_;
    private ProgressDialog waitDataDialog_;
    private LedBorderViewModel ledBorderViewModel_;


    private void SetColorButtonsState(boolean enabled) {
        int i;

        btnPlainColor_.setEnabled(enabled);

        for(i = 0; i < 4; ++i) {
            btnGradColorStart_[i].setEnabled(enabled);
            btnGradColorEnd_[i].setEnabled(enabled);
        }
    }

    private void SetPatternColor(int colorIdx, int color) {
        /* Disable all color buttons */
        SetColorButtonsState(false);
        Executors.newSingleThreadExecutor().execute(() -> {
            boolean result;

            result = eConBadgeManager_.SetLedBorderPatternValues(ledBorderViewModel_,
                                                                 colorIdx,
                                                                 color);

            new Handler(Looper.getMainLooper()).post(() -> {
                UpdateLayout();
                SetColorButtonsState(true);
                if(!result) {
                    /* On failure, close dialog and display error message */
                    new AlertDialog.Builder(getContext())
                            .setTitle("Error")
                            .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                    "disconnect from the badge and reconnect.")
                            .setPositiveButton(android.R.string.ok, null)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .show();
                }
            });
        });
    }

    private void OpenColorPicker(int colorIdx, int initColor) {
        new ColorPickerDialog.Builder(getContext(), AlertDialog.THEME_DEVICE_DEFAULT_DARK)
                .setTitle("Pick a new color")
                .setPreferenceName("MyColorPickerDialog")
                .setPositiveButton("Validate",
                        (ColorEnvelopeListener) (envelope, fromUser) -> SetPatternColor(colorIdx, envelope.getColor()))
                .setNegativeButton("Cancel",
                        (dialogInterface, i) -> dialogInterface.dismiss())
                .attachAlphaSlideBar(false)
                .attachBrightnessSlideBar(true)
                .show();
    }

    private void InitLayout() {
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
                getContext(),
                R.array.color_pattern_choices,
                android.R.layout.simple_spinner_item
        );
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        /* Setup layout items */
        switchState_        = binding_.switchStateFragmentLedBorder;
        spinnerPattern_     = binding_.spinnerColorPatternFragmentLedBorder;
        btnPlainColor_      = binding_.btnPlainColorFragmentLedBorder;
        seekBrightness_     = binding_.seekBrightnessFragmentLedBorder;
        btnNewAnimation_    = binding_.btnNewAnimationFragmentLedBorder;
        btnRemoveAnimation_ = binding_.btnRemoveAnimationFragmentLedBorder;
        btnClearAnimations_ = binding_.btnClearAnimationFragmentLedBorder;

        btnGradColorStart_    = new Button[4];
        btnGradColorStart_[0] = binding_.btnGradColor0StartFragmentLedBorder;
        btnGradColorStart_[1] = binding_.btnGradColor1StartFragmentLedBorder;
        btnGradColorStart_[2] = binding_.btnGradColor2StartFragmentLedBorder;
        btnGradColorStart_[3] = binding_.btnGradColor3StartFragmentLedBorder;

        btnGradColorEnd_    = new Button[4];
        btnGradColorEnd_[0] = binding_.btnGradColor0EndFragmentLedBorder;
        btnGradColorEnd_[1] = binding_.btnGradColor1EndFragmentLedBorder;
        btnGradColorEnd_[2] = binding_.btnGradColor2EndFragmentLedBorder;
        btnGradColorEnd_[3] = binding_.btnGradColor3EndFragmentLedBorder;

        seekGradSize_    = new SeekBar[4];
        seekGradSize_[0] = binding_.seekGrad0SizeFragmentLedBorder;
        seekGradSize_[1] = binding_.seekGrad1SizeFragmentLedBorder;
        seekGradSize_[2] = binding_.seekGrad2SizeFragmentLedBorder;
        seekGradSize_[3] = binding_.seekGrad3SizeFragmentLedBorder;

        textiewGradSize_    = new TextView[4];
        textiewGradSize_[0] = binding_.textViewGrad0SizeValueFragmentLedBorder;
        textiewGradSize_[1] = binding_.textViewGrad1SizeValueFragmentLedBorder;
        textiewGradSize_[2] = binding_.textViewGrad2SizeValueFragmentLedBorder;
        textiewGradSize_[3] = binding_.textViewGrad3SizeValueFragmentLedBorder;

        colorPatternTitleTextView_ = binding_.textViewPatternSelectionFragmentLedBorder;
        plainColorTitleTextView_   = binding_.textViewPlainColorFragmentLedBorder;
        gradientTitleTextView_     = new TextView[4];
        gradientTitleTextView_[0]  = binding_.textViewGrad0FragmentLedBorder;
        gradientTitleTextView_[1]  = binding_.textViewGrad1FragmentLedBorder;
        gradientTitleTextView_[2]  = binding_.textViewGrad2FragmentLedBorder;
        gradientTitleTextView_[3]  = binding_.textViewGrad3FragmentLedBorder;
        brightnessTitleTextView_   = binding_.textViewBrightnessTitleFragmentLedBorder;
        brightnessValueTextView_   = binding_.textViewBrightnessValueFragmentLedBorder;
        animationsTitleTextView_   = binding_.textViewAnimationFragmentLedBorder;

        spinnerPattern_.setAdapter(adapter);
        switchState_.setChecked(false);

        UpdateLayout();
    }

    private void UpdateLayout() {
        int spinnerPosition;
        int i;
        LedBorderPattern.PATTERN_TYPE pattern;

        switchState_.setChecked(ledBorderViewModel_.IsEnabled());
        seekBrightness_.setProgress(ledBorderViewModel_.GetBrightness());
        brightnessValueTextView_.setText(ledBorderViewModel_.GetBrightness() + "%");
        spinnerPattern_.setSelection(ledBorderViewModel_.GetSelectedPatternIdx());

        for(i = 0; i < 4; ++i) {
            seekGradSize_[i].setProgress(ledBorderViewModel_.GetGradSize(i));
            textiewGradSize_[i].setText("Gradient size: " + ledBorderViewModel_.GetGradSize(i) + " LEDs");
        }

        /* Set the max seekbar grad size values based on the pattern idx */
        pattern = ledBorderViewModel_.GetSelectedPattern();
        if(pattern == LedBorderPattern.PATTERN_TYPE.GRAD1) {
            seekGradSize_[0].setMax(120);
        }
        else if(pattern == LedBorderPattern.PATTERN_TYPE.GRAD2) {
            seekGradSize_[0].setMax(60);
            seekGradSize_[1].setMax(60);
        }
        else if(pattern == LedBorderPattern.PATTERN_TYPE.GRAD3) {
            seekGradSize_[0].setMax(40);
            seekGradSize_[1].setMax(40);
            seekGradSize_[2].setMax(40);
        }
        else if(pattern == LedBorderPattern.PATTERN_TYPE.GRAD4) {
            seekGradSize_[0].setMax(30);
            seekGradSize_[1].setMax(30);
            seekGradSize_[2].setMax(30);
            seekGradSize_[3].setMax(30);
        }

        if(switchState_.isChecked()) {
            /* Enable buttons */
            btnNewAnimation_.setVisibility(View.VISIBLE);
            btnRemoveAnimation_.setVisibility(View.VISIBLE);
            btnClearAnimations_.setVisibility(View.VISIBLE);
            seekBrightness_.setVisibility(View.VISIBLE);
            spinnerPattern_.setVisibility(View.VISIBLE);
            colorPatternTitleTextView_.setVisibility(View.VISIBLE);
            brightnessTitleTextView_.setVisibility(View.VISIBLE);
            brightnessValueTextView_.setVisibility(View.VISIBLE);
            animationsTitleTextView_.setVisibility(View.VISIBLE);

            /* Enable pattern depending on the selected pattern */
            spinnerPosition = spinnerPattern_.getSelectedItemPosition();
            if(spinnerPosition != 0) {
                for(i = 0; i < spinnerPosition; ++i) {
                    btnGradColorStart_[i].setVisibility(View.VISIBLE);
                    btnGradColorEnd_[i].setVisibility(View.VISIBLE);
                    gradientTitleTextView_[i].setVisibility(View.VISIBLE);
                    seekGradSize_[i].setVisibility(View.VISIBLE);
                    textiewGradSize_[i].setVisibility(View.VISIBLE);
                }
                for(; i < 4; ++i) {
                    btnGradColorStart_[i].setVisibility(View.GONE);
                    btnGradColorEnd_[i].setVisibility(View.GONE);
                    gradientTitleTextView_[i].setVisibility(View.GONE);
                    seekGradSize_[i].setVisibility(View.GONE);
                    textiewGradSize_[i].setVisibility(View.GONE);
                }

                plainColorTitleTextView_.setVisibility(View.GONE);
                btnPlainColor_.setVisibility(View.GONE);
            }
            else {
                for(i = 0; i < 4; ++i) {
                    btnGradColorStart_[i].setVisibility(View.GONE);
                    btnGradColorEnd_[i].setVisibility(View.GONE);
                    gradientTitleTextView_[i].setVisibility(View.GONE);
                    seekGradSize_[i].setVisibility(View.GONE);
                    textiewGradSize_[i].setVisibility(View.GONE);
                }

                plainColorTitleTextView_.setVisibility(View.VISIBLE);
                btnPlainColor_.setVisibility(View.VISIBLE);
            }
        }
        else {
            /* Disable everything */
            spinnerPattern_.setVisibility(View.GONE);
            btnPlainColor_.setVisibility(View.GONE);
            seekBrightness_.setVisibility(View.GONE);
            btnNewAnimation_.setVisibility(View.GONE);
            btnRemoveAnimation_.setVisibility(View.GONE);
            btnClearAnimations_.setVisibility(View.GONE);

            for(i = 0; i < 4; ++i) {
                btnGradColorStart_[i].setVisibility(View.GONE);
                btnGradColorEnd_[i].setVisibility(View.GONE);
                gradientTitleTextView_[i].setVisibility(View.GONE);
                seekGradSize_[i].setVisibility(View.GONE);
                textiewGradSize_[i].setVisibility(View.GONE);
            }

            colorPatternTitleTextView_.setVisibility(View.GONE);
            plainColorTitleTextView_.setVisibility(View.GONE);
            brightnessTitleTextView_.setVisibility(View.GONE);
            brightnessValueTextView_.setVisibility(View.GONE);
            animationsTitleTextView_.setVisibility(View.GONE);
        }
    }

    private void SetupSwitchState() {
        switchState_.setOnClickListener(view -> {
            boolean isEnabled = switchState_.isChecked();
            switchState_.setEnabled(false);


            Executors.newSingleThreadExecutor().execute(() -> {
                boolean result;

                result = eConBadgeManager_.SetLedBorderState(ledBorderViewModel_, isEnabled);

                new Handler(Looper.getMainLooper()).post(() -> {
                    UpdateLayout();
                    switchState_.setEnabled(true);

                    /* On connection, finish this activity, start the manager activity */
                    if (!result) {
                        /* On failure, close dialog and display error message */
                        new AlertDialog.Builder(getContext())
                                .setTitle("Error")
                                .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                        "disconnect from the badge and reconnect.")
                                .setPositiveButton(android.R.string.ok, null)
                                .setIcon(android.R.drawable.ic_dialog_alert)
                                .show();
                    }
                });
            });
        });
    }

    private void SetupSpinnerPattern() {
        spinnerPattern_.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                /* Only update when a different is set */
                if(ledBorderViewModel_.GetSelectedPatternIdx() == i) {
                    return;
                }

                spinnerPattern_.setEnabled(false);
                Executors.newSingleThreadExecutor().execute(() -> {
                    boolean result;

                    result = eConBadgeManager_.SetLedBorderPatternType(ledBorderViewModel_, i);

                    new Handler(Looper.getMainLooper()).post(() -> {
                        UpdateLayout();
                        spinnerPattern_.setEnabled(true);
                        if(!result) {
                            /* On failure, close dialog and display error message */
                            new AlertDialog.Builder(getContext())
                                    .setTitle("Error")
                                    .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                            "disconnect from the badge and reconnect.")
                                    .setPositiveButton(android.R.string.ok, null)
                                    .setIcon(android.R.drawable.ic_dialog_alert)
                                    .show();
                        }
                    });
                });
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });
    }

    private void SetupSeekBrightness() {
        seekBrightness_.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                /* Change the brightness value */
                brightnessValueTextView_.setText(i + "%");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                /* Nothing */
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                seekBrightness_.setEnabled(false);
                Executors.newSingleThreadExecutor().execute(() -> {
                    boolean result;

                    result = eConBadgeManager_.SetBrightness(ledBorderViewModel_, seekBar.getProgress());

                    new Handler(Looper.getMainLooper()).post(() -> {
                        seekBrightness_.setEnabled(true);
                        UpdateLayout();
                        if(!result) {
                            /* On failure, close dialog and display error message */
                            new AlertDialog.Builder(getContext())
                                    .setTitle("Error")
                                    .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                            "disconnect from the badge and reconnect.")
                                    .setPositiveButton(android.R.string.ok, null)
                                    .setIcon(android.R.drawable.ic_dialog_alert)
                                    .show();
                        }
                    });
                });
            }
        });
    }

    private void SetupButtonPattern() {
        int i;

        btnPlainColor_.setOnClickListener(view -> OpenColorPicker(0, ledBorderViewModel_.GetPlainColor()));

        for(i = 0; i < 4; ++i) {
            int finalI = i;
            btnGradColorStart_[i].setOnClickListener(view -> OpenColorPicker(finalI + 1, ledBorderViewModel_.GetGradStartColor(finalI)));
            btnGradColorEnd_[i].setOnClickListener(view -> OpenColorPicker(finalI + 4 + 1, ledBorderViewModel_.GetGradEndColor(finalI)));
        }
    }

    private void SetupSeekPattern() {
        int i;

        for(i = 0; i < 4; ++i) {
            int finalI = i;
            seekGradSize_[finalI].setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int value, boolean b) {
                    /* Change the brightness value */
                    textiewGradSize_[finalI].setText("Gradient size: " + value + " LEDs");
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                    /* Nothing */
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                    seekBar.setEnabled(false);
                    Executors.newSingleThreadExecutor().execute(() -> {
                        boolean result;

                        result = eConBadgeManager_.SetLedBorderPatternSize(ledBorderViewModel_,
                                                                           seekBar.getProgress(),
                                                                           finalI);

                        new Handler(Looper.getMainLooper()).post(() -> {
                            UpdateLayout();
                            seekBar.setEnabled(true);
                            if(!result) {
                                /* On failure, close dialog and display error message */
                                new AlertDialog.Builder(getContext())
                                        .setTitle("Error")
                                        .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                                "disconnect from the badge and reconnect.")
                                        .setPositiveButton(android.R.string.ok, null)
                                        .setIcon(android.R.drawable.ic_dialog_alert)
                                        .show();
                            }
                        });
                    });
                }
            });
        }
    }

    private void SetupAnimationButtons() {
        btnNewAnimation_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AddAnimationActivity activity = new AddAnimationActivity(getContext());

                activity.setOnValidListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        waitDataDialog_ = new ProgressDialog(getContext());
                        waitDataDialog_.setMessage("Retrieving EConBadge Information");
                        waitDataDialog_.setCancelable(false);
                        waitDataDialog_.show();

                        Executors.newSingleThreadExecutor().execute(() -> {
                            boolean result;

                            result = eConBadgeManager_.AddAnimation(ledBorderViewModel_,
                                                                    activity.getAnimationType(),
                                                                    activity.getAnimationSpeed());

                            new Handler(Looper.getMainLooper()).post(() -> {
                                waitDataDialog_.cancel();

                                if(!result) {
                                    /* On failure, close dialog and display error message */
                                    new AlertDialog.Builder(getContext())
                                            .setTitle("Error")
                                            .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                                    "disconnect from the badge and reconnect.")
                                            .setPositiveButton(android.R.string.ok, null)
                                            .setIcon(android.R.drawable.ic_dialog_alert)
                                            .show();
                                }
                                else {
                                    activity.dismiss();
                                }
                            });
                        });
                    }
                });
                activity.show();
            }
        });
        btnRemoveAnimation_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                RemoveAnimationActivity activity = new RemoveAnimationActivity(getContext(), ledBorderViewModel_);
                activity.show();
            }
        });
        btnClearAnimations_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                btnClearAnimations_.setEnabled(false);
                Executors.newSingleThreadExecutor().execute(() -> {
                    boolean result;

                    result = eConBadgeManager_.ClearAnimations(ledBorderViewModel_);

                    new Handler(Looper.getMainLooper()).post(() -> {
                        UpdateLayout();

                        btnClearAnimations_.setEnabled(true);
                        if(!result) {
                            /* On failure, close dialog and display error message */
                            new AlertDialog.Builder(getContext())
                                    .setTitle("Error")
                                    .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                            "disconnect from the badge and reconnect.")
                                    .setPositiveButton(android.R.string.ok, null)
                                    .setIcon(android.R.drawable.ic_dialog_alert)
                                    .show();
                        }
                    });
                });
            }
        });
    }

    private void SetupActions() {
        SetupSwitchState();
        SetupSpinnerPattern();
        SetupSeekBrightness();
        SetupButtonPattern();
        SetupSeekPattern();
        SetupAnimationButtons();
    }


    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {

        ledBorderViewModel_ = new ViewModelProvider(this).get(LedBorderViewModel.class);

        binding_ = FragmentLedborderBinding.inflate(inflater, container, false);
        View root = binding_.getRoot();

        eConBadgeManager_ = EConBadgeManager.GetInstance();

        InitLayout();

        /* Start process to get led border information */
        waitDataDialog_ = new ProgressDialog(getContext());
        waitDataDialog_.setMessage("Retrieving EConBadge Information");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();

        Executors.newSingleThreadExecutor().execute(() -> {
            boolean result;

            result = eConBadgeManager_.GetLedBorderInformation(ledBorderViewModel_);

            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.cancel();

                UpdateLayout();
                SetupActions();

                if(!result) {
                    /* On failure, close dialog and display error message */
                    new AlertDialog.Builder(getContext())
                            .setTitle("Error")
                            .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                    "disconnect from the badge and reconnect.")
                            .setPositiveButton(android.R.string.ok, null)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .show();
                }
            });
        });


        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding_ = null;
    }
}