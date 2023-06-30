package io.github.oxmose.olsontek.econbadge.main.ui.ledbordermanager;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import io.github.oxmose.olsontek.econbadge.databinding.FragmentLedmanagerBinding;

public class LEDManagerFragment extends Fragment {

    private FragmentLedmanagerBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        LEDManagerViewModel slideshowViewModel =
                new ViewModelProvider(this).get(LEDManagerViewModel.class);

        binding = FragmentLedmanagerBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}