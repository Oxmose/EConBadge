package dev.olsontek.econbadge;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import dev.olsontek.econbadge.btserial.BluetoothManager;
import dev.olsontek.econbadge.btserial.BluetoothSerialDevice;
import dev.olsontek.econbadge.btserial.SimpleBluetoothDeviceInterface;

import io.reactivex.Completable;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;

public class EConBadgeManager {
    private static final String LOG_TAG          = "EConBadge.EConBadgeManager";
    private static final Integer REQUEST_MAGIC   = 0xC0DE1ECB;
    private static final Integer RESPONSE_MAGIC  = 0xCB1EDEC0;
    private static final int TX_MESSAGE_SIZE     = 68;
    private static final int RX_MESSAGE_SIZE     = 8192;
    private static final int COMMAND_ID_POSITION = 4;

    private static final char COMMAND_ID_PING = 0;

    private static final int CONNECTION_WAIT_TIMEOUT = 10;
    private static final int RXTX_TIMEOUT = 10;

    private static EConBadgeManager instance_ = null;

    private BluetoothManager btManager_;
    private SimpleBluetoothDeviceInterface deviceInterface_;
    private String                         deviceName_;
    private String                         deviceMac_;
    private boolean                        isInit_;
    private boolean                        isConnected_;
    private ByteBuffer                     txBuffer_;
    private byte[]                         internalResponse_;
    private Semaphore                      responseAvailable_;
    private Semaphore                      rxReady_;
    private Semaphore                      connectedSem_;

    private EConBadgeManager() {
        isConnected_       = false;
        isInit_            = false;
        btManager_         = null;
        deviceInterface_   = null;
        txBuffer_          = ByteBuffer.allocate(TX_MESSAGE_SIZE);
        responseAvailable_ = new Semaphore(0, true);
        rxReady_           = new Semaphore(1, true);
        connectedSem_      = new Semaphore(0, true);

        txBuffer_.order(ByteOrder.BIG_ENDIAN);
    }


    private void OnConnected(BluetoothSerialDevice device) {
        txBuffer_.clear();

        txBuffer_.putInt(REQUEST_MAGIC);
        txBuffer_.put(COMMAND_ID_POSITION, (byte)COMMAND_ID_PING);

        deviceInterface_ = device.toSimpleDeviceInterface();
        deviceInterface_.setListeners(this::OnMessageRx, this::OnMessageTx, this::OnError);

        deviceInterface_.sendMessage(txBuffer_.array());
    }

    private void OnMessageRx(byte[] message) {
        ByteBuffer buffer;
        int        magic;
        int        size;

        buffer = ByteBuffer.wrap(message);
        magic  = buffer.getInt();
        size   = (int)buffer.get();

        if(RESPONSE_MAGIC.equals(magic)) {
            try {
                if(!rxReady_.tryAcquire(RXTX_TIMEOUT, TimeUnit.SECONDS)) {
                    return;
                }
            }
            catch (InterruptedException e) {
                throw new RuntimeException(e);
            }

            byte[] internalResponse_ = new byte[size];
            buffer.get(internalResponse_);
            Log.d(LOG_TAG, "Received Response of size " + size);
            Log.d(LOG_TAG, new String(internalResponse_));

            /* Check for PONG on connection */
            if(!isConnected_) {
                if(new String(internalResponse_).equals("PONG")){
                    Log.d(LOG_TAG, "New connected device");
                    isConnected_ = true;
                    connectedSem_.release();
                }

                rxReady_.release();
                return;
            }

            responseAvailable_.release();
        }
        else {
            Log.d(LOG_TAG, "Incorrect Magic " + Integer.toHexString(magic));
        }
    }

    private void OnMessageTx(byte[] message) {

    }

    private void OnError(Throwable error) {

    }

    private void HandleResponse(byte[] response) {

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

        isConnected_       = false;
        isInit_            = false;
        deviceInterface_   = null;
        responseAvailable_ = new Semaphore(0, true);
        rxReady_           = new Semaphore(1, true);
        connectedSem_      = new Semaphore(0, true);
    }
    public boolean TransmitData(byte[] data) {
        if(isInit_ && isConnected_) {
            deviceInterface_.sendMessage(data);
            return true;
        }
        else {
            return false;
        }
    }

    public byte[] ReceiveData() {
        byte[] data;

        if(!isInit_ || !isConnected_) {
            return null;
        }

        /* Wait on data available semaphore */
        try {
            if(!responseAvailable_.tryAcquire(RXTX_TIMEOUT, TimeUnit.SECONDS)) {
                return null;
            }
        }
        catch (InterruptedException e) {
            return null;
        }
        data = Arrays.copyOf(internalResponse_, internalResponse_.length);

        /* Release the rx ready semaphore */
        rxReady_.release();

        return data;
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
}
