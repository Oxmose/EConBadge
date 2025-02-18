package dev.olsontek.econbadge.ui.settings;

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
import android.widget.EditText;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.EConErrorType;
import dev.olsontek.econbadge.MainActivity;
import dev.olsontek.econbadge.databinding.FragmentSettingsBinding;
import dev.olsontek.econbadge.models.UpdaterStruct;

public class SettingsFragment extends Fragment {

    private FragmentSettingsBinding binding_;
    private EditText bluetoothNameTextEdit_;
    private EditText bluetoothPinTextEdit_;
    private Button   updateBluetoothSettingsButton_;
    private Button   updateFirmwareButton_;
    private Button   factoryResetButton_;

    private ProgressDialog waitDataDialog_;

    private void SetupUpdateBluetoothSettingsButton() {
        updateBluetoothSettingsButton_.setOnClickListener(view -> {

            if(bluetoothNameTextEdit_.getText().length() == 0) {
                new AlertDialog.Builder(getContext())
                        .setTitle("Error")
                        .setMessage("The bluetooth name cannot be empty.")
                        .setPositiveButton(android.R.string.ok, null)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .show();
                return;
            }

            /* Set the new values */
            if(EConBadgeManager.GetInstance()
                    .SetBluetoothSettings(bluetoothNameTextEdit_.getText().toString(),
                            bluetoothPinTextEdit_.getText().toString())) {
                /* Display info that the app will be disconnected */
                /* On failure, close dialog and display error message */
                new AlertDialog.Builder(getContext())
                        .setTitle("Bluetooth Settings Updated")
                        .setMessage("The bluetooth settings were update, please reconnect to the badge.")
                        .setPositiveButton(android.R.string.ok, (dialogInterface, i) -> {
                            /* Clear the btManager */
                            EConBadgeManager.GetInstance().CancelConnect();

                            /* Kill the activity and present the main activity */
                            Toast.makeText(getActivity(), "EConBadge Disconnected", Toast.LENGTH_LONG).show();
                            Intent intent = new Intent(getActivity(), MainActivity.class);
                            getActivity().finish();
                            startActivity(intent);
                        })
                        .setIcon(android.R.drawable.ic_dialog_info)
                        .setCancelable(false)
                        .show();
            }
            else {
                /* On failure, close dialog and display error message */
                new AlertDialog.Builder(getContext())
                        .setTitle("Error")
                        .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                "disconnect from the badge and reconnect.")
                        .setPositiveButton(android.R.string.ok, null)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .show();
            }
        });
    }
    private void SetupFactoryResetButton() {
        factoryResetButton_.setOnClickListener(view -> {
            /* Open the progress dialog */
            waitDataDialog_ = new ProgressDialog(getContext());
            waitDataDialog_.setMessage("Sending EConBadge Information");
            waitDataDialog_.setCancelable(false);
            waitDataDialog_.show();

            Executors.newSingleThreadExecutor().execute(() -> {
                boolean result;

                result = EConBadgeManager.GetInstance().RequestFactoryReset();

                new Handler(Looper.getMainLooper()).post(() -> {
                    waitDataDialog_.cancel();

                    /* On connection, finish this activity, start the manager activity */
                    if(result) {
                        new AlertDialog.Builder(getContext())
                                .setTitle("Validate and Reconnect")
                                .setMessage("To finish the factory reset, please confirm the action" +
                                        " on your EConBadge. You will be disconnected after that.")
                                .setPositiveButton(android.R.string.ok, (dialogInterface, i) -> {
                                    /* Clear the btManager */
                                    EConBadgeManager.GetInstance().CancelConnect();

                                    /* Kill the activity and present the main activity */
                                    Toast.makeText(getActivity(), "EConBadge Disconnected", Toast.LENGTH_LONG).show();
                                    Intent intent = new Intent(getActivity(), MainActivity.class);
                                    getActivity().finish();
                                    startActivity(intent);
                                })
                                .setIcon(android.R.drawable.ic_dialog_info)
                                .setCancelable(false)
                                .show();
                    }
                    else {
                        /* On failure, close dialog and display error message */
                        new AlertDialog.Builder(getContext())
                                .setTitle("Error")
                                .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                        "disconnect from the badge and reconnect.")
                                .setPositiveButton(android.R.string.ok, null)
                                .setIcon(android.R.drawable.ic_dialog_alert)
                                .show();
                    }
                });
            });
        });
    }
    private void SetupUpdateCheckButton() {
        updateFirmwareButton_.setOnClickListener(view -> {
            /* Open the progress dialog */
            waitDataDialog_ = new ProgressDialog(getContext());
            waitDataDialog_.setMessage("Checking for Update...");
            waitDataDialog_.setCancelable(false);
            waitDataDialog_.show();

            Executors.newSingleThreadExecutor().execute(() -> {
                UpdaterStruct updaterStruct = new UpdaterStruct();
                int           status;

                status = EConBadgeManager.GetInstance().RequestUpdateFirmware(updaterStruct);

                new Handler(Looper.getMainLooper()).post(() -> {
                    waitDataDialog_.cancel();

                    /* On connection, finish this activity, start the manager activity */
                    if(status == EConErrorType.NO_ERROR) {
                        new AlertDialog.Builder(getContext())
                                .setTitle("New firmware available!")
                                .setMessage("To proceed to the installation of the new firmware click \"Update\"." +
                                        " If you want to cancel the update click \"Cancel\".\n\n" +
                                        "New firmware: " + updaterStruct.newVersion + "\n" +
                                        "Current firmware: " + updaterStruct.oldVersion)
                                .setPositiveButton("Update", (dialogInterface, i) -> {
                                    ApplyUpdateAction(updaterStruct);
                                })
                                .setNegativeButton("Cancel", (dialogInterface, i) -> {
                                    /* Clear the btManager */
                                    EConBadgeManager.GetInstance().CancelUpdate();
                                })
                                .setIcon(android.R.drawable.ic_dialog_info)
                                .setCancelable(false)
                                .show();
                    }
                    else if(status == EConErrorType.UPDATE_IS_NOT_NEWER) {
                        new AlertDialog.Builder(getContext())
                                .setTitle("No new firmware available")
                                .setMessage("There is no new update.\nCurrent firmware: " + updaterStruct.oldVersion)
                                .setPositiveButton(android.R.string.ok, null)
                                .setIcon(android.R.drawable.ic_dialog_info)
                                .show();
                    }
                    else {
                        HandleUpdateError(status);
                    }
                });
            });
        });
    }

    private void ApplyUpdateAction(UpdaterStruct updaterStruct) {
        /* Open the progress dialog */
        waitDataDialog_ = new ProgressDialog(getContext());
        waitDataDialog_.setMessage("Applying Update...");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();

        Executors.newSingleThreadExecutor().execute(() -> {
            int status;

            status = EConBadgeManager.GetInstance().ApplyUpdate(updaterStruct, waitDataDialog_);

            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.cancel();

                /* On connection, finish this activity, start the manager activity */
                if(status == EConErrorType.NO_ERROR) {
                    new AlertDialog.Builder(getContext())
                            .setTitle("Update Successful")
                            .setMessage("EConBadge firmware updated to version " + updaterStruct.newVersion)
                            .setPositiveButton(android.R.string.ok, (dialogInterface, i) -> {
                                /* Clear the btManager */
                                EConBadgeManager.GetInstance().CancelConnect();

                                /* Kill the activity and present the main activity */
                                Toast.makeText(getActivity(), "EConBadge Disconnected", Toast.LENGTH_LONG).show();
                                Intent intent = new Intent(getActivity(), MainActivity.class);
                                getActivity().finish();
                                startActivity(intent);
                            })
                            .setIcon(android.R.drawable.ic_dialog_info)
                            .setCancelable(false)
                            .show();
                }
                else {
                    HandleUpdateError(status);
                }
            });
        });
    }

    private void HandleUpdateError(int error) {
        String errorInfo = "Unknown error.";

        if(error == EConErrorType.CANNOT_GET_UPDATE_INFO) {
            errorInfo = "Could not get the update information.";
        }
        else if(error == EConErrorType.CANNOT_CONTACT_ECONBADGE) {
            errorInfo = "Could not contact the EConBadge to get update information.";
        }
        else if(error == EConErrorType.CANNOT_GET_UPDATE_VERSION) {
            errorInfo = "Could not get the updated version information.";
        }
        else if(error == EConErrorType.CANNOT_GET_UPDATE_CHECKSUM) {
            errorInfo = "Could not get the update checksum information.";
        }
        else if(error == EConErrorType.INCORRECT_UPDATE_CHECKSUM) {
            errorInfo = "Incorrect checksum detected when downloading the new firmware.";
        }
        else if(error == EConErrorType.CANNOT_GET_UPDATE_BINARY) {
            errorInfo = "Cannot download the new firmware binaries, make sure you are connected to internet.";
        }
        else if(error == EConErrorType.ECONBADGE_NOT_READY) {
            errorInfo = "EConBadge is not ready for update.";
        }
        /* On failure, close dialog and display error message */
        new AlertDialog.Builder(getContext())
                .setTitle("Error")
                .setMessage(errorInfo)
                .setPositiveButton(android.R.string.ok, null)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        SettingsViewModel homeViewModel =
                new ViewModelProvider(this).get(SettingsViewModel.class);

        binding_ = FragmentSettingsBinding.inflate(inflater, container, false);
        View root = binding_.getRoot();

        bluetoothNameTextEdit_         = binding_.editTextBluetoothNameFragmentSettings;
        bluetoothPinTextEdit_          = binding_.editTextBluetoothPinFragmentSettings;
        updateBluetoothSettingsButton_ = binding_.buttonUpdateBluetoothFragmentDashboard;
        updateFirmwareButton_          = binding_.buttonUpdateFragmentDashboard;
        factoryResetButton_            = binding_.buttonFactoryResetFragmentDashboard;

        /* Open progress dialog */
        waitDataDialog_ = new ProgressDialog(getContext());
        waitDataDialog_.setMessage("Retrieving EConBadge Information");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();

        Executors.newSingleThreadExecutor().execute(() -> {
            boolean result;

            result = homeViewModel.GetEConBadgeInformation();

            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.cancel();

                /* On connection, finish this activity, start the manager activity */
                if(result) {
                    /* Get the information */
                    bluetoothNameTextEdit_.setText(homeViewModel.GetBluetoothName());
                    bluetoothPinTextEdit_.setText(homeViewModel.GetBluetoothPin());
                }
                else {
                    /* On failure, close dialog and display error message */
                    new AlertDialog.Builder(getContext())
                            .setTitle("Error")
                            .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                        "disconnect from the badge and reconnect.")
                            .setPositiveButton(android.R.string.ok, null)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .show();
                }
            });
        });

        SetupUpdateBluetoothSettingsButton();
        SetupFactoryResetButton();
        SetupUpdateCheckButton();

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding_ = null;
    }
}