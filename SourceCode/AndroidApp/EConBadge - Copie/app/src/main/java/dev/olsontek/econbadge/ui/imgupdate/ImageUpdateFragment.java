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
import androidx.recyclerview.widget.LinearLayoutManager;
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
import dev.olsontek.econbadge.adapters.AnimationRemoverAdapter;
import dev.olsontek.econbadge.adapters.ImageListAdapter;
import dev.olsontek.econbadge.databinding.FragmentImageupdateBinding;
import dev.olsontek.econbadge.ui.dialog.ImageCanvasActivity;
import dev.olsontek.econbadge.ui.dialog.RemoveAnimationActivity;

public class ImageUpdateFragment extends Fragment {
    private static final String LOG_TAG = "IMG_UPDATE";

    private static final int IMAGE_LIST_UPDATE_SIZE = 5;

    private Button btnAddImage_;
    private Button btnAddPreparedImage_;
    private Button btnClearEInkDisplay_;
    private RecyclerView recyclerImageList_;

    private FragmentImageupdateBinding binding;
    private ProgressDialog waitDataDialog_;
    private boolean listCanUpdate_;
    private ArrayList<String> imagesList_;
    private ImageListAdapter listAdapter_;

    private final ActivityResultLauncher<Intent> captureImageResultLauncher =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(),
                    new ActivityResultCallback<ActivityResult>() {
                        @Override
                        public void onActivityResult(ActivityResult result) {
                            waitDataDialog_.setMessage("Updating EInk Image...");
                            waitDataDialog_.setCancelable(false);
                            waitDataDialog_.show();


                            if (result != null && result.getResultCode() == Activity.RESULT_OK) {
                                /* Get progress dialog */

                                EInkImage newEinkImage = EInkImage.getFileMetaData(getContext(), result.getData().getData());
                                if (newEinkImage.isValid()) {
                                    Executors.newSingleThreadExecutor().execute(() -> {
                                        boolean updateRes;

                                        updateRes = EConBadgeManager.GetInstance()
                                                .SendEInkImage(newEinkImage.getDisplayName(),
                                                        newEinkImage.getImageData(),
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
                                            else {
                                                updateImageList(true);
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

    private void updateImageList(boolean force) {
        if(force || listCanUpdate_) {
            Executors.newSingleThreadExecutor().execute(() -> {
                boolean updateRes;
                int startIdx;

                startIdx = imagesList_.size();
                updateRes = EConBadgeManager.GetInstance().GetImagesName(imagesList_,
                                                                         IMAGE_LIST_UPDATE_SIZE);
                new Handler(Looper.getMainLooper()).post(() -> {
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
                    else {
                        if (imagesList_.size() != startIdx) {
                            listAdapter_.notifyItemRangeInserted(startIdx,
                                                        imagesList_.size() - startIdx);
                        }
                        else {
                            /* There was nothing to add, do not update */
                            listCanUpdate_ = false;
                        }
                    }
                });
            });
        }
    }

    private void removeImageFromBadge(int position, String imageName) {
        Executors.newSingleThreadExecutor().execute(() -> {
            boolean updateRes;
            updateRes = EConBadgeManager.GetInstance().RemoveImage(imageName);
            new Handler(Looper.getMainLooper()).post(() -> {
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
                else {
                    imagesList_.remove(position);
                    listAdapter_.notifyItemRemoved(position);
                }
            });
        });
    }

    private void viewImageFromBadge(String imageName) {
        waitDataDialog_.setMessage("Receiving image...");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();

        Executors.newSingleThreadExecutor().execute(() -> {
            boolean   updateRes;
            EInkImage image = new EInkImage();
            updateRes = EConBadgeManager.GetInstance().GetImageData(imageName, image);
            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.dismiss();
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
                else {
                    ImageCanvasActivity activity = new ImageCanvasActivity(getContext(), image);
                    activity.show();
                }
            });
        });
    }
    private void selectImageFromBadge(String imageName) {
        waitDataDialog_.setMessage("Updating selected image...");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();
        Executors.newSingleThreadExecutor().execute(() -> {
            boolean updateRes;
            updateRes = EConBadgeManager.GetInstance().SelectLoadedImage(imageName);
            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.dismiss();
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
    }

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
        binding = FragmentImageupdateBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        listCanUpdate_ = true;
        imagesList_    = new ArrayList<>();

        waitDataDialog_ = new ProgressDialog(getContext());

        /* Bind views */
        btnAddImage_         = binding.buttonAddImageImageUpdateFragment;
        btnAddPreparedImage_ = binding.buttonAddPreparedImageImageUpdateFragment;
        btnClearEInkDisplay_ = binding.buttonClearEINKUpdateFragment;
        recyclerImageList_   = binding.recycleViewImageListImageUpdateFragment;

        listAdapter_ = new ImageListAdapter(getContext(), imagesList_, new ImageListAdapter.Updater() {
            @Override
            public void updateList(boolean force) {
                updateImageList(force);
            }

            @Override
            public void removeImage(int position, String imageName) {
                removeImageFromBadge(position, imageName);
            }

            @Override
            public void viewImage(String imageName) {
                viewImageFromBadge(imageName);
            }

            @Override
            public void selectUpdate(String imageName) {
                selectImageFromBadge(imageName);
            }

        });
        LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
        recyclerImageList_.setLayoutManager(layoutManager);
        recyclerImageList_.setAdapter(listAdapter_);

        /* Setup button actions */
        setupButtonActions();

        updateImageList(true);

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}