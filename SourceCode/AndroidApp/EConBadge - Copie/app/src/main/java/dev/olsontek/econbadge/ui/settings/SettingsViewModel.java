package dev.olsontek.econbadge.ui.settings;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import dev.olsontek.econbadge.EConBadgeManager;

public class SettingsViewModel extends ViewModel {

    private String bluetoothNameText_;
    private String bluetoothPinText_;
    private String swVersionText_;
    private String hwVersionText_;
    private final EConBadgeManager manager_;

    public SettingsViewModel() {
        bluetoothNameText_        = new String("Unknown");
        bluetoothNameText_        = new String("Unknown");
        swVersionText_            = new String("Unknown");
        hwVersionText_            = new String("Unknown");

        manager_ = EConBadgeManager.GetInstance();
    }

    public String GetBluetoothName() {
        return bluetoothNameText_;
    }

    public String GetBluetoothPin() {
        return bluetoothPinText_;
    }

    public boolean GetEConBadgeInformation() {
        if(!manager_.UpdateInformation()) {
            return false;
        }

        bluetoothNameText_ = manager_.GetDeviceName();
        if(manager_.GetDevicePin().equals("None")) {
            bluetoothPinText_ = "";
        }
        else {
            bluetoothPinText_  = manager_.GetDevicePin();
        }

        swVersionText_     = manager_.GetDeviceFirmwareVersion();
        hwVersionText_     = manager_.GetDeviceHardwareVersion();

        return true;
    }
}