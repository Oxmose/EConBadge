package dev.olsontek.econbadge.ui.ledborder;

import androidx.lifecycle.ViewModel;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import dev.olsontek.econbadge.models.LedBorderPattern;
import dev.olsontek.econbadge.models.AnimationModel;

public class LedBorderViewModel extends ViewModel {

    private boolean isEnabled_;
    private int brightness_;
    private ArrayList<AnimationModel> animations_;

    LedBorderPattern currentPattern_;

    public LedBorderViewModel() {
        isEnabled_          = false;
        brightness_         = 100;

        animations_     = new ArrayList<>();
        currentPattern_ = new LedBorderPattern();
    }

    public boolean IsEnabled() {
        return isEnabled_;
    }

    public void SetEnabled(boolean enabled) {
        isEnabled_ = enabled;
    }

    public int GetPlainColor() {

        return currentPattern_.GetPlainColorCode();
    }

    public int GetBrightness() {
        return brightness_;
    }

    public int GetSelectedPatternIdx() {
        return currentPattern_.GetTypeIdx();
    }

    public int GetGradStartColor(int i) {
        return currentPattern_.GetStartColorCode(i);
    }

    public int GetGradEndColor(int i) {
        return currentPattern_.GetEndColorCode(i);
    }

    public void ClearAnimations() {
        animations_.clear();
    }

    public void SetBrightness(int brightness) {
        brightness_ = brightness;
    }

    public LedBorderPattern.PATTERN_TYPE GetSelectedPattern() {
        return currentPattern_.GetType();
    }

    public int GetGradSize(int i) {
        return currentPattern_.GetGradientSize(i);
    }

    public LedBorderPattern GetColorPattern() {
        LedBorderPattern newPattern = currentPattern_.GetCopy();

        return newPattern;
    }
    public void SetColorPattern(LedBorderPattern ledBorderPattern) {
        currentPattern_ = ledBorderPattern;
    }

    public void AddAnimation(int idx, byte animationType, byte animationParam) {
        animations_.add(new AnimationModel(idx, animationType, animationParam));
        animations_.sort((animationModel, t1) -> {
            if (animationModel.getIndex() < t1.getIndex()) {
                return 1;
            } else if (animationModel.getIndex() > t1.getIndex()) {
                return -1;
            } else {
                return 0;
            }
        });
    }

    public ArrayList<AnimationModel> GetAnimations() {

        return animations_;
    }
}