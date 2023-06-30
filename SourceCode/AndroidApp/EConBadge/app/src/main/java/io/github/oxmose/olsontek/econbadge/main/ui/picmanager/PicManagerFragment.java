package io.github.oxmose.olsontek.econbadge.main.ui.picmanager;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import io.github.oxmose.olsontek.econbadge.databinding.FragmentPicmanagerBinding;

public class PicManagerFragment extends Fragment {

    private FragmentPicmanagerBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        PicManagerViewModel galleryViewModel =
                new ViewModelProvider(this).get(PicManagerViewModel.class);

        binding = FragmentPicmanagerBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}