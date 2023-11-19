package dev.olsontek.econbadge.ui.ledborder;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class LedBorderViewModel extends ViewModel {

    private final MutableLiveData<String> mText;

    public LedBorderViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is gallery fragment");
    }

    public LiveData<String> getText() {
        return mText;
    }
}