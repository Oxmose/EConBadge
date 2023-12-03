package dev.olsontek.econbadge.ui.imgupdate;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.RecyclerView;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;

import com.nareshchocha.filepickerlibrary.models.DocumentFilePickerConfig;
import com.nareshchocha.filepickerlibrary.ui.FilePicker;

import dev.olsontek.econbadge.EInkImage;
import dev.olsontek.econbadge.databinding.FragmentImageupdateBinding;

public class ImageUpdateFragment extends Fragment {
    private static final String LOG_TAG = "IMG_UPDATE";

    private Button btnAddImage_;
    private Button btnAddPreparedImage_;
    private Button btnClearEInkDisplay_;
    private RecyclerView recyclerImageList_;

    private FragmentImageupdateBinding binding;
    private ProgressDialog waitDataDialog_;

    private final ActivityResultLauncher<Intent> captureImageResultLauncher =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(),
                    new ActivityResultCallback<ActivityResult>() {
                        @Override
                        public void onActivityResult(ActivityResult result) {


                            if (result != null && result.getResultCode() == Activity.RESULT_OK) {
                                /* Get progress dialog */
                                waitDataDialog_ = new ProgressDialog(getContext());
                                waitDataDialog_.setMessage("Updating EInk Image...");
                                waitDataDialog_.setCancelable(false);
                                waitDataDialog_.show();

                                EInkImage newEinkImage = EInkImage.getFileMetaData(getContext(), result.getData().getData());
                                if (newEinkImage.isValid()) {
                                    Executors.newSingleThreadExecutor().execute(() -> {
                                        boolean updateRes;

                                        updateRes = EConBadgeManager.GetInstance()
                                                .SendEInkImage(newEinkImage.displayName_,
                                                        newEinkImage.imageData_,
                                                        waitDataDialog_) == 0;

                                        new Handler(Looper.getMainLooper()).post(() -> {
                                            waitDataDialog_.cancel();

                                            if (!updateRes) {
                                                /* On failure, close dialog and display error message */
                                                new AlertDialog.Builder(getContext())
                                                        .setTitle("Error")
                                                        .setMessage("Cannot update the the EConBadge display, is the error persists, " +
                                                                "disconnect from the badge and reconnect.")
                                                        .setPositiveButton(android.R.string.ok, null)
                                                        .setIcon(android.R.drawable.ic_dialog_alert)
                                                        .show();
                                            }
                                        });
                                    });
                                } else {
                                    waitDataDialog_.dismiss();
                                    new AlertDialog.Builder(getContext())
                                            .setTitle("Error")
                                            .setMessage("The selected image cannot be loaded in the EConBadge")
                                            .setPositiveButton(android.R.string.ok, null)
                                            .setIcon(android.R.drawable.ic_dialog_alert)
                                            .show();
                                }
                            }
                        }
                    });


    private void setupButtonActions() {
        btnClearEInkDisplay_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                btnClearEInkDisplay_.setEnabled(false);
                Executors.newSingleThreadExecutor().execute(() -> {
                    boolean result;

                    result = EConBadgeManager.GetInstance().ClearEInk();

                    new Handler(Looper.getMainLooper()).post(() -> {

                        btnClearEInkDisplay_.setEnabled(true);
                        if(!result) {
                            /* On failure, close dialog and display error message */
                            new AlertDialog.Builder(getContext())
                                    .setTitle("Error")
                                    .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                            "disconnect from the badge and reconnect.")
                                    .setPositiveButton(android.R.string.ok, null)
                                    .setIcon(android.R.drawable.ic_dialog_alert)
                                    .show();
                        }
                        else {
                            new AlertDialog.Builder(getContext())
                                    .setTitle("EInk Clearing")
                                    .setMessage("The EInk clearing command was sent, the display " +
                                                "should flicker and reset to its blank state.")
                                    .setPositiveButton(android.R.string.ok, null)
                                    .setIcon(android.R.drawable.ic_dialog_info)
                                    .show();
                        }
                    });
                });
            }
        });

        btnAddPreparedImage_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AlertDialog.Builder(getContext())
                        .setTitle("Upload a prepared image")
                        .setMessage("A prepared image is a BMP file with the following parameters:" +
                                "\n  - 600x448 pixels" +
                                "\n  - 8 bits-per-pixel palette" +
                                "\n  - 7 colors palette" +
                                "\n  - No compression")
                        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {

                                ArrayList<String> mMimeTypesList = new ArrayList<>();
                                mMimeTypesList.add("image/bmp");
                                captureImageResultLauncher.launch(
                                        new FilePicker.Builder(getContext())
                                                .pickDocumentFileBuild(new DocumentFilePickerConfig(
                                                                null, // DrawableRes Id
                                                                null,// Title for pop item
                                                                false, // set Multiple pick file
                                                                null, // max files working only in android latest version
                                                                mMimeTypesList, // added Multiple MimeTypes
                                                                null,  // set Permission ask Title
                                                                null, // set Permission ask Message
                                                                null, // set Permission setting Title
                                                                null // set Permission setting Messages
                                                        )
                                                )
                                );
                            }
                        })
                        .setNegativeButton(android.R.string.cancel, null)
                        .setIcon(android.R.drawable.ic_dialog_info)
                        .show();
            }
        });
    }

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        ImageUpdateViewModel slideshowViewModel =
                new ViewModelProvider(this).get(ImageUpdateViewModel.class);

        binding = FragmentImageupdateBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        /* Bind views */
        btnAddImage_         = binding.buttonAddImageImageUpdateFragment;
        btnAddPreparedImage_ = binding.buttonAddPreparedImageImageUpdateFragment;
        btnClearEInkDisplay_ = binding.buttonClearEINKUpdateFragment;
        recyclerImageList_   = binding.recycleViewImageListImageUpdateFragment;


        /* Setup button actions */
        setupButtonActions();

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}