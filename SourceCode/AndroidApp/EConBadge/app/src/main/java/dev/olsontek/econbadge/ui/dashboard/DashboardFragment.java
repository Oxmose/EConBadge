package dev.olsontek.econbadge.ui.dashboard;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import java.util.concurrent.Executors;

import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.databinding.FragmentDashboardBinding;

public class DashboardFragment extends Fragment {

    private FragmentDashboardBinding binding_;
    private EditText ownerTextEdit_;
    private EditText contactTextEdit_;
    private TextView bluetoothTextView_;
    private TextView ledBorderStateTextView_;
    private TextView currentEINKImageNameTextView_;
    private TextView softwareVersionTextView_;
    private TextView hardwareVersionTextView_;
    private Button   updateInfoButton_;


    private ProgressDialog waitDataDialog_;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        DashboardViewModel homeViewModel =
                new ViewModelProvider(this).get(DashboardViewModel.class);

        binding_ = FragmentDashboardBinding.inflate(inflater, container, false);
        View root = binding_.getRoot();

        ownerTextEdit_                = binding_.editTextOwnerFragmentDashboard;
        contactTextEdit_              = binding_.editTextContactFragmentDashboard;
        bluetoothTextView_            = binding_.textViewBluetoothNameFragmentDashboard;
        ledBorderStateTextView_       = binding_.textViewLedBorderStateFragmentDashboard;
        currentEINKImageNameTextView_ = binding_.textViewCurrentEINKImageNameFragmentDashboard;
        softwareVersionTextView_      = binding_.textViewSWVersionFragmentDashboard;
        hardwareVersionTextView_      = binding_.textViewHWVersionFragmentDashboard;
        updateInfoButton_             = binding_.buttonUpdateInformationFragmentDashboard;

        /* Set actions */
        updateInfoButton_.setOnClickListener(view -> {
            /* Get the information text */

            /* Open the progress dialog */
            waitDataDialog_ = new ProgressDialog(getContext());
            waitDataDialog_.setMessage("Sending EConBadge Information");
            waitDataDialog_.setCancelable(false);
            waitDataDialog_.show();

            Executors.newSingleThreadExecutor().execute(() -> {
                boolean result;

                result = homeViewModel.SetEConBadgeInformation(ownerTextEdit_.getText().toString(),
                                                               contactTextEdit_.getText().toString());

                new Handler(Looper.getMainLooper()).post(() -> {
                    waitDataDialog_.cancel();

                    ownerTextEdit_.setText(homeViewModel.GetOwner());
                    contactTextEdit_.setText(homeViewModel.GetContact());

                    /* On connection, finish this activity, start the manager activity */
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
                });
            });
        });

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
                    ownerTextEdit_.setText(homeViewModel.GetOwner());
                    contactTextEdit_.setText(homeViewModel.GetContact());
                    bluetoothTextView_.setText(getString(R.string.bt_name_string, homeViewModel.GetBluetoothName()));
                    ledBorderStateTextView_.setText(getString(R.string.led_border_state, homeViewModel.GetLedBorderState()));
                    currentEINKImageNameTextView_.setText(getString(R.string.current_eink_image, homeViewModel.GetCurrentEINKImageName()));
                    softwareVersionTextView_.setText(getString(R.string.firmware_version, homeViewModel.GetSoftwareVersion()));
                    hardwareVersionTextView_.setText(getString(R.string.hardware_version, homeViewModel.GetHardwareVersion()));
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

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding_ = null;
    }
}