package dev.olsontek.econbadge.ui.dialog;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.google.android.material.chip.Chip;

import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.databinding.FragmentDashboardBinding;

public class AddAnimationActivity extends Dialog {

    private Chip chipBreath_;
    private Chip chipSlider_;
    private SeekBar seekAnimSpeed_;
    private Button buttonCancel_;
    private Button buttonValidate_;
    private TextView textViewAnimSpeed_;

    private View.OnClickListener onValidateClickListener_;

    public AddAnimationActivity(@NonNull Context context) {
        super(context);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_add_animation);

        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);

        chipBreath_        = findViewById(R.id.chipBreathActivityAddAnimation);
        chipSlider_        = findViewById(R.id.chipSliderActivityAddAnimation);
        seekAnimSpeed_     = findViewById(R.id.seekBarSpeedActivityAddAnimation);
        buttonCancel_      = findViewById(R.id.buttonCancelActivityAddAnimation);
        buttonValidate_    = findViewById(R.id.buttonValidateActivityAddAnimation);
        textViewAnimSpeed_ = findViewById(R.id.textViewAnimationSpeedValueActivityAddAnimation);

        seekAnimSpeed_.setProgress(5);
        seekAnimSpeed_.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                textViewAnimSpeed_.setText("" + seekAnimSpeed_.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                textViewAnimSpeed_.setText("" + seekAnimSpeed_.getProgress());
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                textViewAnimSpeed_.setText("" + seekAnimSpeed_.getProgress());
            }
        });
        textViewAnimSpeed_.setText("" + seekAnimSpeed_.getProgress());

        buttonCancel_.setOnClickListener(view -> dismiss());
        buttonValidate_.setOnClickListener(onValidateClickListener_);
    }

    public void setOnValidListener(View.OnClickListener onClickListener) {
        onValidateClickListener_ = onClickListener;
    }

    public int getAnimationSpeed() {
        return seekAnimSpeed_.getProgress();
    }

    public int getAnimationType() {
        if(chipBreath_.isChecked()) {
            return 1;
        }
        else {
            return 0;
        }
    }
}