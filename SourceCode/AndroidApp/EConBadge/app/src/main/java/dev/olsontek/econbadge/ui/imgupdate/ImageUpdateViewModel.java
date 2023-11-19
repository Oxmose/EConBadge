package dev.olsontek.econbadge.ui.imgupdate;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class ImageUpdateViewModel extends ViewModel {

    private final MutableLiveData<String> mText;

    public ImageUpdateViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is slideshow fragment");
    }

    public LiveData<String> getText() {
        return mText;
    }
}