package dev.olsontek.econbadge.ui.dashboard;

import android.content.Context;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.R;

public class DashboardViewModel extends ViewModel {

    private String ownerText_;
    private String contactText_;
    private String bluetoothNameText_;
    private String ledStateText_;
    private String currentEINKImageNameText_;
    private String swVersionText_;
    private String hwVersionText_;
    private final EConBadgeManager manager_;


    public DashboardViewModel() {
        ownerText_                = new String("Unknown");
        contactText_              = new String("Unknown");
        bluetoothNameText_        = new String("Unknown");
        ledStateText_             = new String("Unknown");
        currentEINKImageNameText_ = new String("Unknown");
        swVersionText_            = new String("Unknown");
        hwVersionText_            = new String("Unknown");

        manager_ = EConBadgeManager.GetInstance();
    }

    public String GetOwner() {
        return ownerText_;
    }

    public String GetContact() {
        return contactText_;
    }

    public String GetBluetoothName() {
        return bluetoothNameText_;
    }

    public String GetLedBorderState() {
        return ledStateText_;
    }

    public String GetCurrentEINKImageName() {
        return currentEINKImageNameText_;
    }

    public String GetSoftwareVersion() {
        return swVersionText_;
    }

    public String GetHardwareVersion() {
        return hwVersionText_;
    }

    public boolean GetEConBadgeInformation() {
        if(!manager_.UpdateInformation()) {
            return false;
        }

        ownerText_                = manager_.GetOwner();
        contactText_              = manager_.GetContact();
        bluetoothNameText_        = manager_.GetDeviceName();
        ledStateText_             = manager_.GetLedBorderState();
        currentEINKImageNameText_ = manager_.GetCurrentEInkImageName();
        swVersionText_            = manager_.GetDeviceFirmwareVersion();
        hwVersionText_            = manager_.GetDeviceHardwareVersion();

        return true;
    }

    public boolean SetEConBadgeInformation(String newOwner, String newContact) {
        boolean status;

        status = manager_.SetInformation(newOwner, newContact);

        ownerText_   = manager_.GetOwner();
        contactText_ = manager_.GetContact();

        return status;
    }
}