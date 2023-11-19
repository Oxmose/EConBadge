package dev.olsontek.econbadge.ui.imgupdate;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import dev.olsontek.econbadge.databinding.FragmentImageupdateBinding;

public class ImageUpdateFragment extends Fragment {

    private FragmentImageupdateBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        ImageUpdateViewModel slideshowViewModel =
                new ViewModelProvider(this).get(ImageUpdateViewModel.class);

        binding = FragmentImageupdateBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        final TextView textView = binding.textImageupdate;
        slideshowViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);
        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}