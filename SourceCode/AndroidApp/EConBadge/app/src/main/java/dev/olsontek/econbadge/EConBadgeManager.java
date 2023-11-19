package dev.olsontek.econbadge;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.service.controls.actions.BooleanAction;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import dev.olsontek.econbadge.btserial.BluetoothManager;
import dev.olsontek.econbadge.btserial.BluetoothSerialDevice;
import dev.olsontek.econbadge.btserial.SimpleBluetoothDeviceInterface;

import io.reactivex.Completable;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;

public class EConBadgeManager {
    private static final String LOG_TAG            = "EConBadge.EConBadgeManager";

    private static final Integer REQUEST_MAGIC  = 0xC0DE1ECB;
    private static final Integer RESPONSE_MAGIC = 0xCB1EDEC0;

    private static final int COMMAND_ID_POSITION   = 4;
    private static final int COMMAND_DATA_POSITION = 5;

    private static final char COMMAND_ID_PING        = 0;
    private static final char COMMAND_ID_SET_OWNER   = 10;
    private static final char COMMAND_ID_SET_CONTACT = 11;
    private final static char COMMAND_ID_GETINFO     = 18;

    private final static int GETINFO_RESPONSE_DATA_PARTS = 6;

    private static final int CONNECTION_WAIT_TIMEOUT = 10;
    private static final int RXTX_TIMEOUT             = 10;
    private static final int TX_MESSAGE_SIZE          = 68;
    private static final int RX_MESSAGE_SIZE          = 8192;
    private static final byte MESSAGE_LIST_DIVIDER    = 6;

    private static EConBadgeManager instance_ = null;

    private BluetoothManager               btManager_;
    private SimpleBluetoothDeviceInterface deviceInterface_;
    private String                         deviceName_;
    private String                         deviceMac_;
    private boolean                        isInit_;
    private boolean                        isConnected_;
    private ByteBuffer                     txBuffer_;
    private byte[]                         internalResponse_;
    private int                            internalResponseLength_;
    private Semaphore                      responseAvailable_;
    private Semaphore                      rxReady_;
    private Semaphore                      connectedSem_;
    private String                         owner_;
    private String                         contact_;
    private String                         deviceSWVersion_;
    private String                         deviceHWVersion_;
    private boolean                        ledBorderState_;
    private String                         currentEInkImageName_;

    private EConBadgeManager() {
        isConnected_          = false;
        isInit_               = false;
        btManager_            = null;
        deviceInterface_      = null;
        deviceName_           = new String();
        deviceMac_            = new String();
        owner_                = new String();
        contact_              = new String();
        deviceSWVersion_      = new String();
        deviceHWVersion_      = new String();
        ledBorderState_       = false;
        currentEInkImageName_ = new String();
        txBuffer_             = ByteBuffer.allocate(TX_MESSAGE_SIZE);
        responseAvailable_    = new Semaphore(0, true);
        rxReady_              = new Semaphore(1, true);
        connectedSem_         = new Semaphore(0, true);

        txBuffer_.order(ByteOrder.BIG_ENDIAN);
    }


    private void OnConnected(BluetoothSerialDevice device) {
        deviceInterface_ = device.toSimpleDeviceInterface();
        deviceInterface_.setListeners(this::OnMessageRx, this::OnMessageTx, this::OnError);

        /* Send the PING command */
        SendCommand(COMMAND_ID_PING, null);
    }

    private void OnMessageRx(byte[] message) {
        ByteBuffer buffer;
        int        magic;
        int        size;

        buffer = ByteBuffer.wrap(message);
        magic  = buffer.getInt();
        size   = (int)buffer.get();

        /* TODO: Recompose lengthy messages */

        if(RESPONSE_MAGIC.equals(magic)) {
            try {
                if(!rxReady_.tryAcquire(RXTX_TIMEOUT, TimeUnit.SECONDS)) {
                    return;
                }
            }
            catch (InterruptedException e) {
                throw new RuntimeException(e);
            }

            internalResponse_ = new byte[size];
            buffer.get(internalResponse_);
            internalResponseLength_ = size;

            Log.d(LOG_TAG, "Received Response of size " + internalResponseLength_);
            Log.d(LOG_TAG, new String(internalResponse_));

            /* Check for PONG on connection */
            if(!isConnected_) {
                if(new String(internalResponse_).equals("PONG")){
                    Log.d(LOG_TAG, "New connected device");
                    isConnected_ = true;
                    connectedSem_.release();
                    rxReady_.release();
                }
            }
            else {
                responseAvailable_.release();
            }
        }
        else {
            Log.d(LOG_TAG, "Incorrect Magic " + Integer.toHexString(magic));
        }
    }

    private void OnMessageTx(byte[] message) {
        /* TODO: */
    }

    private void OnError(Throwable error) {
        /* TODO: */
    }

    private void HandleResponse(byte[] response) {
        /* TODO: */
    }

    private boolean TransmitData(byte[] data) {
        if(isInit_ && isConnected_) {
            deviceInterface_.sendMessage(data);
            return true;
        }
        else {
            return false;
        }
    }

    private byte[] ReceiveData() {
        byte[] data = null;

        if(!isInit_ || !isConnected_) {
            return null;
        }

        Log.d(LOG_TAG, "Waiting for received data");

        /* Wait on data available semaphore */
        try {
            if(!responseAvailable_.tryAcquire(RXTX_TIMEOUT, TimeUnit.SECONDS)) {
                return null;
            }
        }
        catch (InterruptedException e) {
            return null;
        }

        Log.d(LOG_TAG, "Receive Data: " + new String(internalResponse_, StandardCharsets.ISO_8859_1));

        data = Arrays.copyOf(internalResponse_, internalResponseLength_);

        /* Release the rx ready semaphore */
        rxReady_.release();

        return data;
    }

    private void SendCommand(char commandId, byte[] commandData) {
        /* Reset the TX buffer */
        Arrays.fill(txBuffer_.array(), (byte)0);
        txBuffer_.position(0);

        /* Set magic, command ID and data */
        txBuffer_.putInt(REQUEST_MAGIC);
        txBuffer_.put((byte)commandId);
        if(commandData != null && commandData.length != 0) {
            txBuffer_.put(commandData);
        }

        /* Send data */
        deviceInterface_.sendMessage(txBuffer_.array());
    }

    private List<byte[]> SplitMessage(byte[] message) {
        int          i;
        int          lastDivider;
        List<byte[]> splitList = new ArrayList<>();

        if(message == null) {
            return splitList;
        }

        lastDivider = 0;
        for(i = 0; i < message.length; ++i) {
            /* On new divider detected, create a new split */
            if(message[i] == MESSAGE_LIST_DIVIDER && lastDivider < i) {
                byte[] tmpBytes;
                tmpBytes = Arrays.copyOfRange(message, lastDivider, i);
                splitList.add(tmpBytes);
                lastDivider = i + 1;
            }
        }

        /* Add the last split */
        if(lastDivider < message.length && lastDivider <= i) {
            byte[] tmpBytes;
            tmpBytes = Arrays.copyOfRange(message, lastDivider, i);
            splitList.add(tmpBytes);
        }

        return splitList;
    }

    public static EConBadgeManager GetInstance() {
        if(instance_ == null) {
            instance_ = new EConBadgeManager();
        }

        return instance_;
    }

    @SuppressLint("CheckResult")
    public ErrorType ConnectTo(String macAddress, String name) {
        if(!isInit_) {
            if(btManager_ == null) {
                btManager_ = BluetoothManager.getInstance();
                if(btManager_ == null) {
                    return ErrorType.BT_CANNOT_CONNECT;
                }
            }

            btManager_.openSerialDevice(macAddress, StandardCharsets.ISO_8859_1)
                    .subscribeOn(Schedulers.io())
                    .observeOn(AndroidSchedulers.mainThread())
                    .subscribe(this::OnConnected, this::OnError);

            isInit_     = true;
            deviceMac_  = macAddress;
            deviceName_ = name;
        }

        return ErrorType.NO_ERROR;
    }

    public void CancelConnect() {
        if(isInit_) {
            btManager_.close();
        }

        isConnected_          = false;
        isInit_               = false;
        deviceInterface_      = null;
        deviceName_           = new String();
        deviceMac_            = new String();
        owner_                = new String();
        contact_              = new String();
        deviceSWVersion_      = new String();
        deviceHWVersion_      = new String();
        ledBorderState_       = false;
        currentEInkImageName_ = new String();
        responseAvailable_    = new Semaphore(0, true);
        rxReady_              = new Semaphore(1, true);
        connectedSem_         = new Semaphore(0, true);
    }

    public boolean WaitForConnection() {
        boolean status;

        if(isConnected_) {
            return true;
        }

        try {
            status = connectedSem_.tryAcquire(CONNECTION_WAIT_TIMEOUT, TimeUnit.SECONDS);
        }
        catch (InterruptedException e) {
            return false;
        }
        connectedSem_.release();
        return status;
    }

    public boolean UpdateInformation() {
        byte[]       response;
        List<byte[]> splitResponse;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_GETINFO, null);

        response = ReceiveData();
        if(response != null) {
            splitResponse = SplitMessage(response);

            if(splitResponse.size() != GETINFO_RESPONSE_DATA_PARTS) {
                Log.e(LOG_TAG, "GETINFO returned " + splitResponse.size() + " but expected " + GETINFO_RESPONSE_DATA_PARTS);
                return false;
            }

            /* Set the values */
            owner_                = new String(splitResponse.get(0), StandardCharsets.US_ASCII);
            contact_              = new String(splitResponse.get(1), StandardCharsets.US_ASCII);
            deviceSWVersion_      = new String(splitResponse.get(2), StandardCharsets.US_ASCII);
            deviceHWVersion_      = new String(splitResponse.get(3), StandardCharsets.US_ASCII);
            currentEInkImageName_ = new String(splitResponse.get(5), StandardCharsets.US_ASCII);

            ledBorderState_ = ByteBuffer.wrap(splitResponse.get(4)).get() != (byte)0;

            Log.d(LOG_TAG, "Owner Info: " + owner_);
            Log.d(LOG_TAG, "Contact Info: " + contact_);
            Log.d(LOG_TAG, "SW version Info: " + deviceSWVersion_);
            Log.d(LOG_TAG, "HW version Info: " + deviceHWVersion_);
            Log.d(LOG_TAG, "LED Border State Info: " + ledBorderState_);
            Log.d(LOG_TAG, "Current EINK Image Info: " + currentEInkImageName_);
        }
        else {
            Log.e(LOG_TAG, "Count not get GETINFO response.");
            return false;
        }

        return true;
    }

    public String GetOwner() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return owner_;
    }

    public String GetContact() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return contact_;
    }

    public String GetDeviceName() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return deviceName_;
    }

    public String GetDeviceFirmwareVersion() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return deviceSWVersion_;
    }

    public String GetDeviceHardwareVersion() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return deviceHWVersion_;
    }

    public String GetLedBorderState() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        if(ledBorderState_) {
            return "On";
        }
        else {
            return "Off";
        }
    }

    public String GetCurrentEInkImageName() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return currentEInkImageName_;
    }

    public boolean SetInformation(String newOwner, String newContact) {
        byte[] response;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_SET_OWNER, newOwner.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData();
        if(response != null) {
            if(new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                owner_ = newOwner;
                Log.d(LOG_TAG, "Updated Owner Info");
            }
            else {
                Log.e(LOG_TAG, "Error on owner update: " + new String(response, StandardCharsets.US_ASCII));
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_OWNER response.");
            return false;
        }

        SendCommand(COMMAND_ID_SET_CONTACT, newContact.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData();
        if(response != null) {
            if(new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                contact_ = newContact;
                Log.d(LOG_TAG, "Updated Contact Info");
            }
            else {
                Log.e(LOG_TAG, "Error on contact update: " + new String(response, StandardCharsets.US_ASCII));
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_CONTACT response.");
            return false;
        }

        return true;
    }
}
