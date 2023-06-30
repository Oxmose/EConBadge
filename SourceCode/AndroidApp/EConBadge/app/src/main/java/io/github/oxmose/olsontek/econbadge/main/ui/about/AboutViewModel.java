package io.github.oxmose.olsontek.econbadge.main.ui.about;

import android.content.Context;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import io.github.oxmose.olsontek.econbadge.R;

public class AboutViewModel extends ViewModel {

    private final MutableLiveData<Integer> mVersionTextView;

    public AboutViewModel() {
        mVersionTextView = new MutableLiveData<>();
        mVersionTextView.setValue(R.string.app_version);
    }

    public LiveData<Integer> getTextId() {
        return mVersionTextView;
    }
}