package dev.olsontek.econbadge;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import dev.olsontek.econbadge.btserial.BluetoothManager;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;

import dev.olsontek.econbadge.adapters.StringMetadataAdapter;
import dev.olsontek.econbadge.models.StringMetadataItem;

public class MainActivity extends AppCompatActivity {
    private static final String LOG_TAG = "EConBadge.MainActivity";

    private ListView devicesListView_;
    private ArrayAdapter devicesListAdapter_;

    BluetoothManager bluetoothManager_;

    private ArrayList<StringMetadataItem> pairedDevices_;
    private Toast currentToast_ = null;
    private ProgressDialog connectDialog_;


    @SuppressLint("MissingPermission")
    private void InitBluetoothService() {
        List<BluetoothDevice> pairedDevices;

        bluetoothManager_ = BluetoothManager.getInstance();
        if(bluetoothManager_ == null) {
            if (currentToast_ != null) {
                currentToast_.cancel();
            }
            currentToast_ = Toast.makeText(this, "Bluetooth could not be initialized.", Toast.LENGTH_LONG);
            currentToast_.show();

            return;
        }

        pairedDevices = bluetoothManager_.getPairedDevicesList();
        for (BluetoothDevice device : pairedDevices) {
            pairedDevices_.add(new StringMetadataItem(device.getName(), device.getAddress()));
        }
        devicesListAdapter_.notifyDataSetChanged();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        pairedDevices_      = new ArrayList<>();
        devicesListAdapter_ = new StringMetadataAdapter(this, pairedDevices_);

        devicesListView_    = findViewById(R.id.btListViewMainActivity);

        devicesListView_.setAdapter(devicesListAdapter_);
        devicesListView_.setOnItemClickListener((adapterView, view, position, id) -> {
            EConBadgeManager manager = EConBadgeManager.GetInstance();
            StringMetadataItem item  = pairedDevices_.get(position);

            /* Try to ping the EConBadge */
            manager.ConnectTo(item.metadata_, item.displayString_);
            connectDialog_ = new ProgressDialog(this);
            connectDialog_.setMessage("Connecting to " + item.displayString_);
            connectDialog_.setCancelable(false);
            connectDialog_.show();


            Executors.newSingleThreadExecutor().execute(() -> {
                boolean result;

                result = manager.WaitForConnection();

                new Handler(Looper.getMainLooper()).post(() -> {
                    connectDialog_.cancel();

                    /* On connection, finish this activity, start the manager activity */
                    if(result) {
                        Toast.makeText(this, "Connected to " + item.displayString_, Toast.LENGTH_LONG).show();
                        Intent i = new Intent(MainActivity.this, MainDashboardActivity.class);
                        finish();
                        startActivity(i);
                    }
                    else {
                        manager.CancelConnect();

                        /* On failure, close dialog and display error message */
                        new AlertDialog.Builder(this)
                            .setTitle("Error")
                            .setMessage("Cannot connect to " + item.displayString_ + "\n" +
                                        "Ensure your device is tuned ON and paired with your " +
                                        "Android device.")
                            .setPositiveButton(android.R.string.ok, null)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .show();
                    }
                });
            });
        });

        /* Initialize the bluetooth manager */
        InitBluetoothService();
    }
}