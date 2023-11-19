package dev.olsontek.econbadge.ui.ledborder;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import dev.olsontek.econbadge.databinding.FragmentLedborderBinding;

public class LedBorderFragment extends Fragment {

    private FragmentLedborderBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        LedBorderViewModel galleryViewModel =
                new ViewModelProvider(this).get(LedBorderViewModel.class);

        binding = FragmentLedborderBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        final TextView textView = binding.textLedborder;
        galleryViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);
        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}