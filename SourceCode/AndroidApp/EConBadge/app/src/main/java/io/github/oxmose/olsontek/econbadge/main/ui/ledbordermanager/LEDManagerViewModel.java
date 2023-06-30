package io.github.oxmose.olsontek.econbadge.main.ui.ledbordermanager;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class LEDManagerViewModel extends ViewModel {

    private final MutableLiveData<String> mText;

    public LEDManagerViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is slideshow fragment");
    }

    public LiveData<String> getText() {
        return mText;
    }
}