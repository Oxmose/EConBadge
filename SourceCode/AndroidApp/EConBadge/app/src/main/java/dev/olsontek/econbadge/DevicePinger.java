package dev.olsontek.econbadge;

import android.app.Activity;
import android.util.Log;
import android.widget.ArrayAdapter;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.Semaphore;

import dev.olsontek.econbadge.btserial.BluetoothManager;
import dev.olsontek.econbadge.btserial.BluetoothSerialDevice;
import dev.olsontek.econbadge.btserial.SimpleBluetoothDeviceInterface;
import dev.olsontek.econbadge.models.StringMetadataItem;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;

public class DevicePinger {

    private static final String LOG_TAG = "DEVICE_PINGER";

    private static final int PING_WAIT_SLEEP_MS   = 500;
    private static final int TX_COMMAND_SIZE      = 68;
    private static final int TXRX_RAW_PACKET_SIZE = 128;
    private static final Integer REQUEST_MAGIC    = 0xC0DE1ECB;
    private static final Integer RESPONSE_MAGIC   = 0xCB1EDEC0;
    private static final char COMMAND_ID_PING     = 0;

    private static final int RECV_STATE_WAIT_RESPONSE_MAGIC = 0;
    private static final int RECV_STATE_WAIT_RESPONSE_SIZE  = 1;
    private static final int RECV_STATE_RECOMPOSE_MESSAGE   = 2;
    private static final int RECV_STATE_COPY_BUFFER         = 3;

    private int    receiverState_;
    private int    receiverStateMagicIdx_;
    private int    receiverInternalBufferSize_;
    private int    receiverInternalBufferOffset_;
    private byte[] receiverInternalBuffer_;

    private final String deviceName_;
    private final String deviceMac_;

    private volatile boolean hasPing_;

    private volatile boolean isRunning_;

    private final Semaphore execSem_;
    private volatile Thread currentThread_;

    private BluetoothManager btManager_;
    private SimpleBluetoothDeviceInterface pingDevice_;

    private ArrayAdapter                  devicesListAdapter_;
    private ArrayList<StringMetadataItem> pingedDevices_;

    private Activity caller_;

    private void DevicePingerRoutine() {
        while(isRunning_ && !hasPing_) {
            try {
                execSem_.acquire();
                if (!isRunning_ || hasPing_) {
                    break;
                }

                Log.d(LOG_TAG, "Sending ping command to " + deviceName_);

                /* Set magic, command ID and data */
                ByteBuffer buffer = ByteBuffer.allocate(TX_COMMAND_SIZE);
                buffer.order(ByteOrder.BIG_ENDIAN);

                buffer.putInt(REQUEST_MAGIC);
                buffer.put((byte)COMMAND_ID_PING);

                /* Send data */
                pingDevice_.sendMessage(buffer.array());
                execSem_.release();

                /* Sleep */
                Thread.sleep(PING_WAIT_SLEEP_MS);
            }
            catch (InterruptedException e) {
                e.printStackTrace();
                break;
            }
        }
    }

    private void OnConnected(BluetoothSerialDevice device) {
        Runnable run;

        Log.d(LOG_TAG, "Device " + deviceName_ + " (" + deviceMac_ + ") : " + device.getMac() + " connected.");

        try {
            if (!isRunning_ || hasPing_) {
                return;
            }

            execSem_.acquire();
            pingDevice_ = device.toSimpleDeviceInterface();
            pingDevice_.setListeners(this::OnMessageRx, null, this::OnError);

            /* Start the send message thread */
            run = () -> DevicePingerRoutine();

            currentThread_ = new Thread(run);
            currentThread_.start();

            execSem_.release();
        }
        catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void OnError(Throwable error) {
        /* TODO: */
    }

    private void OnMessageRx(byte[] message) {
        ByteBuffer buffer;
        int        i;
        int        maxToCopy;
        byte       readByte;

        StringMetadataItem newItem;

        /* Check state and execute */
        try {
            buffer = ByteBuffer.wrap(message);
            buffer.order(ByteOrder.BIG_ENDIAN);

            if(receiverState_ == RECV_STATE_WAIT_RESPONSE_MAGIC) {
                Log.d(LOG_TAG, "Checking Magic from received message.");
                for(i = 0; i < message.length; ++i) {
                    if (receiverStateMagicIdx_ > 3) {
                        receiverStateMagicIdx_ = 0;
                        receiverState_ = RECV_STATE_WAIT_RESPONSE_SIZE;
                        break;
                    }
                    else {
                        readByte = buffer.get();
                        if(readByte == (byte)(RESPONSE_MAGIC >> (8 * (3 - receiverStateMagicIdx_)))) {
                            ++receiverStateMagicIdx_;
                        }
                        else {
                            Log.d(LOG_TAG, "Incorrect magic byte at position " +
                                    receiverStateMagicIdx_ + " : " + Integer.toHexString(readByte) +
                                    " expected " + Integer.toHexString((byte)(RESPONSE_MAGIC >> (8 * (3 - receiverStateMagicIdx_)))));
                            receiverStateMagicIdx_ = 0;
                        }
                    }
                }
            }
            if(receiverState_ == RECV_STATE_WAIT_RESPONSE_SIZE) {
                receiverInternalBufferSize_   = buffer.get();
                receiverInternalBufferOffset_ = 0;

                Log.d(LOG_TAG, "Checking Size from received message: " + receiverInternalBufferSize_);

                receiverState_ = RECV_STATE_RECOMPOSE_MESSAGE;
            }
            if(receiverState_ == RECV_STATE_RECOMPOSE_MESSAGE) {
                Log.d(LOG_TAG, "Recomposing received message, left: " + (receiverInternalBufferSize_ - receiverInternalBufferOffset_));
                maxToCopy = Math.min(buffer.remaining(), receiverInternalBufferSize_ - receiverInternalBufferOffset_);
                buffer.get(receiverInternalBuffer_, receiverInternalBufferOffset_, maxToCopy);

                receiverInternalBufferOffset_ += maxToCopy;

                Log.d(LOG_TAG, "Read: " + maxToCopy);

                /* Check for end of transmission */
                if(receiverInternalBufferOffset_ == receiverInternalBufferSize_) {
                    receiverState_ = RECV_STATE_COPY_BUFFER;
                }
            }
            if(receiverState_ == RECV_STATE_COPY_BUFFER) {
                Log.d(LOG_TAG, "Complete message recomposed of size " + receiverInternalBufferSize_);

                if(new String(Arrays.copyOfRange(receiverInternalBuffer_, 0, receiverInternalBufferSize_), StandardCharsets.US_ASCII).equals("PONG")) {
                    Log.d(LOG_TAG, "New pinged device");
                    hasPing_ = true;
                    newItem = new StringMetadataItem(deviceName_, deviceMac_);
                    if(!pingedDevices_.contains(newItem)) {
                        pingedDevices_.add(newItem);
                        caller_.runOnUiThread(() -> devicesListAdapter_.notifyDataSetChanged());
                    }

                    btManager_.closeDevice(deviceMac_);
                }

                /* Wait for next message */
                receiverState_ = RECV_STATE_WAIT_RESPONSE_MAGIC;
            }
        }
        catch (BufferUnderflowException e) {
            /* We read too much, stop here */
            return;
        }
    }

    public DevicePinger(Activity caller,
                        String deviceName,
                        String deviceMac,
                        ArrayAdapter devicesListAdapter,
                        ArrayList<StringMetadataItem> pingedDevices) {
        deviceName_         = deviceName;
        deviceMac_          = deviceMac;
        devicesListAdapter_ = devicesListAdapter;
        pingedDevices_      = pingedDevices;
        caller_             = caller;

        receiverInternalBuffer_ = new byte[TXRX_RAW_PACKET_SIZE];

        execSem_ = new Semaphore(1, true);

        hasPing_ = false;
    }

    public void Start() {
        /* Create device bluetooth manager */
        if(btManager_ == null) {
            btManager_ = BluetoothManager.getInstance();
            if(btManager_ == null) {
                return;
            }
        }

        if(currentThread_ != null) {
            Log.e(LOG_TAG, "Tried to reuse DevicePinger that was stopped");
            return;
        }

        /* Close the device connection is existed */
        btManager_.closeDevice(deviceMac_);

        Log.d(LOG_TAG, "Starting pinger for device " + deviceName_);

        hasPing_   = false;
        isRunning_ = true;

        btManager_.openSerialDevice(deviceMac_, StandardCharsets.ISO_8859_1)
                  .subscribeOn(Schedulers.io())
                  .observeOn(AndroidSchedulers.mainThread())
                  .subscribe(this::OnConnected, this::OnError);

        Log.d(LOG_TAG, "Started pinger for device " + deviceName_);
    }

    public void Stop() {
        isRunning_ = false;
        try {
            execSem_.acquire();

            /* Stop bluetooth device instance */
            btManager_.closeDevice(deviceMac_);

            execSem_.release();
            if(currentThread_ != null) {
                currentThread_.join();
            }
        }
        catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

    @Override
    public int hashCode() {
        return deviceMac_.hashCode();
    }
}
