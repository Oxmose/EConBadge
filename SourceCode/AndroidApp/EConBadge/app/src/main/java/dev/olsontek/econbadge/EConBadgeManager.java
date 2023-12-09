package dev.olsontek.econbadge;

import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Formatter;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.HttpsURLConnection;

import dev.olsontek.econbadge.btserial.BluetoothManager;
import dev.olsontek.econbadge.btserial.BluetoothSerialDevice;
import dev.olsontek.econbadge.btserial.SimpleBluetoothDeviceInterface;
import dev.olsontek.econbadge.models.LedBorderPattern;
import dev.olsontek.econbadge.models.UpdaterStruct;
import dev.olsontek.econbadge.ui.ledborder.LedBorderViewModel;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;

public class EConBadgeManager {
    private static final String LOG_TAG            = "EConBadge.EConBadgeManager";

    private static final String FIRMWARE_SERVER_URL       = "https://olsontek.dev/EConBadge/updates/";
    private static final String FIRMWARE_SERVER_INFO_FILE = "update.txt";
    private static final String FIRMWARE_SERVER_BIN_FILE  = "firmware.bin";
    private static final int    MAX_FIRMWARE_SIZE         = 2097152; /* 2MB */
    private static final int    UPDATE_PACKET_MARKER_END  = 0xE0E0A0A0;

    private static final Integer REQUEST_MAGIC              = 0xC0DE1ECB;
    private static final Integer RESPONSE_MAGIC             = 0xCB1EDEC0;
    private static final int RECV_STATE_WAIT_RESPONSE_MAGIC = 0;
    private static final int RECV_STATE_WAIT_RESPONSE_SIZE  = 1;
    private static final int RECV_STATE_RECOMPOSE_MESSAGE   = 2;
    private static final int RECV_STATE_COPY_BUFFER         = 3;

    private static final int RECV_MODE_NORMAL = 0;
    private static final int RECV_MODE_RAW    = 1;

    private static final char COMMAND_ID_PING                        = 0;
    private static final char COMMAND_ID_CLEAR_EINK                  = 1;
    private static final char COMMAND_ID_UPDATE_EINK                 = 2;
    private static final char COMMAND_ID_ENABLE_LED_BORDER           = 3;
    private static final char COMMAND_ID_DISABLE_LED_BORDER          = 4;
    private static final char COMMAND_ID_ADD_ANIM_LED_BORDER         = 5;
    private static final char COMMAND_ID_REM_ANIM_LED_BORDER         = 6;
    private static final char COMMAND_ID_SET_PATTERN_LED_BORDER      = 7;
    private static final char COMMAND_ID_CLEAR_ANIM_LED_BORDER       = 8;
    private static final char COMMAND_ID_SET_BRIGHTNESS_LED_BORDER   = 9;
    private static final char COMMAND_ID_SET_OWNER                   = 10;
    private static final char COMMAND_ID_SET_CONTACT                 = 11;
    private static final char COMMAND_ID_SET_BT_SETTINGS             = 12;
    private static final char COMMAND_ID_REQ_FACTORY_RESET           = 13;
    private static final char COMMAND_ID_START_UPDATE                = 14;
    private static final char COMMAND_ID_VALIDATE_UPDATE             = 15;
    private static final char COMMAND_ID_CANCEL_UPDATE               = 16;
    private static final char COMMAND_ID_START_TRANS_UPDATE          = 17;
    private final static char COMMAND_ID_GETINFO                     = 18;
    private final static char COMMAND_ID_REQUEST_UPDATE              = 19;
    private final static char COMMAND_ID_GET_INFO_LED_BORDER         = 20;
    private final static char COMMAND_ID_GET_IMAGES_NAME             = 21;
    private final static char COMMAND_ID_REMOVE_IMAGE                = 22;
    private final static char COMMAND_ID_SELECT_LOADED_IMAGE         = 23;
    private final static char COMMAND_ID_GET_CURRENT_IMAGE           = 24;

    private final static int GETINFO_RESPONSE_DATA_PARTS = 7;

    private static final int  CONNECTION_WAIT_TIMEOUT = 10;
    private static final int  RX_LONG_TIMEOUT         = 60;
    private static final int  RXTX_TIMEOUT            = 10;
    private static final int  TX_COMMAND_SIZE         = 68;
    private static final int  TXRX_RAW_PACKET_SIZE    = 8192;
    private static final byte MESSAGE_LIST_DIVIDER    = 6;

    private static EConBadgeManager instance_ = null;

    private BluetoothManager               btManager_;
    private SimpleBluetoothDeviceInterface deviceInterface_;
    private String                         deviceName_;
    private String                         devicePin_;
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

    private int    receiverMode_;

    private int    receiverState_;
    private int    receiverStateMagicIdx_;
    private int    receiverInternalBufferSize_;
    private int    receiverInternalBufferOffset_;
    private byte[] receiverInternalBuffer_;

    private int       receiverRawLeftToRecv_;
    private byte[]    receiverRawBuffer_;
    private Semaphore receiverRawWaitSem_;


    private EConBadgeManager() {
        isConnected_            = false;
        isInit_                 = false;
        btManager_              = null;
        deviceInterface_        = null;
        deviceName_             = new String();
        devicePin_              = new String();
        owner_                  = new String();
        contact_                = new String();
        deviceSWVersion_        = new String();
        deviceHWVersion_        = new String();
        ledBorderState_         = false;
        currentEInkImageName_   = new String();
        receiverState_          = RECV_STATE_WAIT_RESPONSE_MAGIC;
        receiverMode_           = RECV_MODE_NORMAL;
        receiverStateMagicIdx_  = 0;
        receiverInternalBuffer_ = new byte[TXRX_RAW_PACKET_SIZE];
        txBuffer_               = ByteBuffer.allocate(TX_COMMAND_SIZE);
        responseAvailable_      = new Semaphore(0, true);
        rxReady_                = new Semaphore(1, true);
        connectedSem_           = new Semaphore(0, true);

        txBuffer_.order(ByteOrder.BIG_ENDIAN);
    }


    private void OnConnected(BluetoothSerialDevice device) {
        deviceInterface_ = device.toSimpleDeviceInterface();
        deviceInterface_.setListeners(this::OnMessageRx, this::OnMessageTx, this::OnError);

        /* Send the PING command */
        SendCommand(COMMAND_ID_PING, null);
    }

    private void OnMessageRx(byte[] message) {

        if(receiverMode_ == RECV_MODE_NORMAL) {
            handleRecvModeNormal(message);
        }
        else if(receiverMode_ == RECV_MODE_RAW) {
            handleRecvModeRaw(message);
        }
    }

    private void handleRecvModeNormal(byte[] message) {
        ByteBuffer buffer;
        int        i;
        int        maxToCopy;
        byte       readByte;

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

                if(!rxReady_.tryAcquire(RXTX_TIMEOUT, TimeUnit.SECONDS)) {
                    return;
                }

                Log.d(LOG_TAG, "Complete message recomposed of size " + receiverInternalBufferSize_);

                internalResponse_       = Arrays.copyOfRange(receiverInternalBuffer_, 0, receiverInternalBufferSize_);
                internalResponseLength_ = receiverInternalBufferSize_;

                if(isConnected_) {
                    Log.d(LOG_TAG, "Received recomposed response of size " + internalResponseLength_);
                    Log.d(LOG_TAG, new String(internalResponse_));

                    responseAvailable_.release();
                }
                /* Check for PONG on connection */
                else if(new String(internalResponse_, StandardCharsets.US_ASCII).equals("PONG")) {
                    Log.d(LOG_TAG, "New connected device");
                    isConnected_ = true;
                    connectedSem_.release();
                    rxReady_.release();
                }

                /* Wait for next message */
                receiverState_ = RECV_STATE_WAIT_RESPONSE_MAGIC;
            }
        }
        catch (BufferUnderflowException e) {
            /* We read too much, stop here */
            return;
        }
        catch (InterruptedException e){
            return;
        }
    }

    private void handleRecvModeRaw(byte[] message) {
        ByteBuffer buffer;
        byte[]     okMsg = new byte[1];

        /* Check state and execute */
        try {
            buffer = ByteBuffer.wrap(message);
            buffer.order(ByteOrder.LITTLE_ENDIAN);

            /* First check how much we should receive */
            if(receiverRawLeftToRecv_ == -1) {
                receiverRawLeftToRecv_ = buffer.getInt();
                receiverRawBuffer_ = new byte[receiverRawLeftToRecv_];

                Log.d(LOG_TAG, "RAW DATA RECV INIT " + receiverRawLeftToRecv_);
            }

            /* Now copy to the buffer */
            while(buffer.hasRemaining() && receiverRawLeftToRecv_ > 0) {
                receiverRawBuffer_[receiverRawBuffer_.length - receiverRawLeftToRecv_] = buffer.get();
                --receiverRawLeftToRecv_;
            }

            /* If we finished the transfer, notify the waiting semaphore */
            if(receiverRawLeftToRecv_ == 0) {
                receiverRawLeftToRecv_ = -2;
                receiverRawWaitSem_.release();
            }
            else {
                Thread.sleep(10);
                Log.d(LOG_TAG, "SEND ACK");
                /* Send ok */
                okMsg[0] = 1;
                deviceInterface_.sendMessage(okMsg);
            }
        }
        catch (BufferUnderflowException e) {
            /* We read too much, stop here */
            return;
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    private void OnMessageTx(byte[] message) {
        /* TODO: */
    }

    private void OnError(Throwable error) {
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

    private void SetReceivingMode(int recvMode) {
        receiverMode_ = recvMode;

        receiverRawLeftToRecv_ = -1;
        receiverRawBuffer_     = null;
        receiverRawWaitSem_    = new Semaphore(0);
    }

    private byte[] ReceiveData(int timeout) {
        byte[] data = null;

        if(!isInit_ || !isConnected_) {
            return null;
        }

        Log.d(LOG_TAG, "Waiting for received data");

        if(receiverMode_ == RECV_MODE_NORMAL) {
            /* Wait on data available semaphore */
            try {
                if (!responseAvailable_.tryAcquire(timeout, TimeUnit.SECONDS)) {
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
        }
        else {
            try {
                if (!receiverRawWaitSem_.tryAcquire(timeout, TimeUnit.SECONDS)) {
                    return null;
                }
            }
            catch (InterruptedException e) {
                return null;
            }

            data = Arrays.copyOf(receiverRawBuffer_, receiverRawBuffer_.length);

            /* Reinit state */
            receiverRawLeftToRecv_ = -1;
            receiverRawBuffer_     = null;
            receiverRawWaitSem_    = new Semaphore(0);
        }

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
        if(lastDivider < message.length) {
            byte[] tmpBytes;
            tmpBytes = Arrays.copyOfRange(message, lastDivider, i);
            splitList.add(tmpBytes);
        }

        return splitList;
    }

    private int GetFirmwareBinary(UpdaterStruct updaterStruct) {
        /* Get the last firmware binary and info */
        InputStream        in = null;
        ByteBuffer         binaryFile;
        byte[]             tmpBuffer;
        Formatter          formatter;
        byte[]             newFirmwareBin;
        HttpsURLConnection httpConn;
        URLConnection      conn;
        URL                currUrl;
        int                firmwareSize;
        int                bytesRead;
        int                responseCode;
        MessageDigest      md;

        /* Init the update */
        try {
            /* Make the request */
            currUrl = new URL(FIRMWARE_SERVER_URL + FIRMWARE_SERVER_BIN_FILE);
            conn    = currUrl.openConnection();
            if(!(conn instanceof HttpsURLConnection)) {
                return EConErrorType.CANNOT_GET_UPDATE_BINARY;
            }

            httpConn = (HttpsURLConnection)conn;
            httpConn.setAllowUserInteraction(false);
            httpConn.setInstanceFollowRedirects(true);
            httpConn.setRequestMethod("GET");
            httpConn.connect();
            responseCode = httpConn.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                return EConErrorType.CANNOT_GET_UPDATE_BINARY;
            }
            in = httpConn.getInputStream();

            /* Get the data */
            binaryFile = ByteBuffer.allocate(MAX_FIRMWARE_SIZE);

            firmwareSize = 0;
            tmpBuffer    = new byte[MAX_FIRMWARE_SIZE];
            while((bytesRead = in.read(tmpBuffer)) != -1) {
                binaryFile.put(tmpBuffer, 0, bytesRead);
                firmwareSize += bytesRead;
            }
            Log.d(LOG_TAG, "Firmware length: " + firmwareSize);
            in.close();

            newFirmwareBin = Arrays.copyOf(binaryFile.array(), firmwareSize);

            /* Get the checksum and check it */
            md = MessageDigest.getInstance("SHA-1");
            formatter = new Formatter();
            for (byte b : md.digest(newFirmwareBin)) {
                formatter.format("%02x", b);
            }
            Log.d(LOG_TAG, "Downloaded firmware checksum: " + formatter);

            if(!updaterStruct.checksum.equals(formatter.toString())) {
                return EConErrorType.INCORRECT_UPDATE_CHECKSUM;
            }
        }
        catch (IOException | NoSuchAlgorithmException e) {
            try {
                if(in != null) {
                    in.close();
                }
            }
            catch (IOException ex) {
                throw new RuntimeException(ex);
            }
            return EConErrorType.CANNOT_GET_UPDATE_BINARY;
        }

        updaterStruct.newFirmwareBinary = newFirmwareBin;
        return EConErrorType.NO_ERROR;
    }

    private int GetUpdateInfo(UpdaterStruct updaterStruct) {
        /* Get the last firmware binary and info */
        InputStream        in = null;
        BufferedReader     buffReader;
        HttpsURLConnection httpConn;
        URLConnection      conn;
        URL                currUrl;
        String             version;
        String             checksum;
        int                responseCode;

        /* Init the update */
        try {
            /* Make the request */
            currUrl = new URL(FIRMWARE_SERVER_URL + FIRMWARE_SERVER_INFO_FILE);
            conn    = currUrl.openConnection();
            if(!(conn instanceof HttpsURLConnection)) {
                return EConErrorType.CANNOT_GET_UPDATE_INFO;
            }
            httpConn = (HttpsURLConnection)conn;
            httpConn.setAllowUserInteraction(false);
            httpConn.setInstanceFollowRedirects(true);
            httpConn.setRequestMethod("GET");
            httpConn.connect();
            responseCode = httpConn.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                return EConErrorType.CANNOT_GET_UPDATE_INFO;
            }
            in = httpConn.getInputStream();
            buffReader = new BufferedReader(new InputStreamReader(in));

            /* Get the data version */
            version = buffReader.readLine();
            if(version == null) {
                in.close();
                return EConErrorType.CANNOT_GET_UPDATE_VERSION;
            }
            Log.d(LOG_TAG, "Server SW version: " + version);
            Log.d(LOG_TAG, "Badge SW version: " + updaterStruct.oldVersion);

            if(!IsVersionNewer(version, updaterStruct.oldVersion)) {
                in.close();
                return EConErrorType.UPDATE_IS_NOT_NEWER;
            }

            /* Get the checksum */
            checksum = buffReader.readLine();
            if(checksum == null) {
                in.close();
                return EConErrorType.CANNOT_GET_UPDATE_CHECKSUM;
            }
            Log.d(LOG_TAG, "Server SW checksum: " + checksum);

            in.close();
        }
        catch (IOException e) {
            try {
                if(in != null) {
                    in.close();
                }
            }
            catch (IOException ex) {
                throw new RuntimeException(ex);
            }
            return EConErrorType.CANNOT_GET_UPDATE_INFO;
        }

        updaterStruct.newVersion = version;
        updaterStruct.checksum   = checksum;

        return EConErrorType.NO_ERROR;
    }

    private boolean IsVersionNewer(String newVersion, String oldVersion) {
        String[] newVersionSplit;
        String[] oldVersionSplit;

        int i;

        /* Version is of format vMAJ.MIN.BUILD */
        if(newVersion.charAt(0) != 'v') {
            return false;
        }
        if(oldVersion.charAt(0) != 'v') {
            return false;
        }

        newVersion = newVersion.substring(1);
        oldVersion = oldVersion.substring(1);

        /* Check split size */
        newVersionSplit = newVersion.split("\\.");
        if(newVersionSplit.length != 3) {
            return false;
        }
        oldVersionSplit = oldVersion.split("\\.");
        if(oldVersionSplit.length != 3) {
            return false;
        }

        try {
            /* Check all component */
            for (i = 0; i < 3; ++i) {
                if (Integer.parseInt(newVersionSplit[i]) > Integer.parseInt(oldVersionSplit[i])) {
                    return true;
                }
            }
        }
        catch (NumberFormatException e) {
            return false;
        }

        return false;
    }

    public static EConBadgeManager GetInstance() {
        if(instance_ == null) {
            instance_ = new EConBadgeManager();
        }

        return instance_;
    }

    @SuppressLint("CheckResult")
    public int ConnectTo(String macAddress, String name) {
        if(!isInit_) {
            if(btManager_ == null) {
                btManager_ = BluetoothManager.getInstance();
                if(btManager_ == null) {
                    return EConErrorType.BT_CANNOT_CONNECT;
                }
            }

            CancelConnect();

            btManager_.openSerialDevice(macAddress, StandardCharsets.ISO_8859_1)
                    .subscribeOn(Schedulers.io())
                    .observeOn(AndroidSchedulers.mainThread())
                    .subscribe(this::OnConnected, this::OnError);

            isInit_     = true;
            deviceName_ = name;
        }

        return EConErrorType.NO_ERROR;
    }

    public void CancelConnect() {
        if(btManager_ != null) {
            btManager_.close();
        }

        isConnected_           = false;
        isInit_                = false;
        deviceInterface_       = null;
        deviceName_            = new String();
        devicePin_             = new String();
        owner_                 = new String();
        contact_               = new String();
        deviceSWVersion_       = new String();
        deviceHWVersion_       = new String();
        ledBorderState_        = false;
        currentEInkImageName_  = new String();
        receiverState_         = RECV_STATE_WAIT_RESPONSE_MAGIC;
        receiverMode_          = RECV_MODE_NORMAL;
        receiverStateMagicIdx_ = 0;
        responseAvailable_     = new Semaphore(0, true);
        rxReady_               = new Semaphore(1, true);
        connectedSem_          = new Semaphore(0, true);
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

        response = ReceiveData(RXTX_TIMEOUT);
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
            devicePin_            = new String(splitResponse.get(6), StandardCharsets.US_ASCII);

            ledBorderState_ = ByteBuffer.wrap(splitResponse.get(4)).get() != (byte)0;

            Log.d(LOG_TAG, "Owner Info: " + owner_);
            Log.d(LOG_TAG, "Contact Info: " + contact_);
            Log.d(LOG_TAG, "SW version Info: " + deviceSWVersion_);
            Log.d(LOG_TAG, "HW version Info: " + deviceHWVersion_);
            Log.d(LOG_TAG, "LED Border State Info: " + ledBorderState_);
            Log.d(LOG_TAG, "Current EINK Image Info: " + currentEInkImageName_);
            Log.d(LOG_TAG, "Bluetooth PIN: " + devicePin_);
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

    public String GetDevicePin() {
        if(!isConnected_) {
            return "EConBadge Not Connected";
        }
        return devicePin_;
    }

    public boolean SetInformation(String newOwner, String newContact) {
        byte[] response;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_SET_OWNER, newOwner.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                owner_ = newOwner;
                Log.d(LOG_TAG, "Updated Owner Info");
            }
            else {
                Log.e(LOG_TAG, "Error on owner update: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_OWNER response.");
            return false;
        }

        SendCommand(COMMAND_ID_SET_CONTACT, newContact.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                contact_ = newContact;
                Log.d(LOG_TAG, "Updated Contact Info");
            }
            else {
                Log.e(LOG_TAG, "Error on contact update: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_CONTACT response.");
            return false;
        }

        return true;
    }

    public boolean SetBluetoothSettings(String newBtName, String newBtPin) {
        ByteBuffer commandData;
        int        minSize;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        commandData = ByteBuffer.allocate(25);

        minSize = Math.min(21, newBtName.getBytes(StandardCharsets.US_ASCII).length);
        commandData.put(newBtName.getBytes(StandardCharsets.US_ASCII), 0, minSize);
        /* Add padding */
        if(minSize < 21) {
            commandData.put(new byte[21 - minSize]);
        }

        minSize = Math.min(4, newBtPin.getBytes(StandardCharsets.US_ASCII).length);
        commandData.put(newBtPin.getBytes(StandardCharsets.US_ASCII), 0, minSize);
        /* Add padding */
        if(minSize < 4) {
            commandData.put(new byte[4 - minSize]);
        }

        Log.d(LOG_TAG, "Sending new BT Name and PIN " + new String(commandData.array(), StandardCharsets.US_ASCII));

        SendCommand(COMMAND_ID_SET_BT_SETTINGS, commandData.array());

        return true;
    }

    public boolean RequestFactoryReset() {
        byte[] response;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_REQ_FACTORY_RESET, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on factory reset request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get FACTORY_RESET response.");
            return false;
        }

        return true;
    }

    public int RequestUpdateFirmware(UpdaterStruct updaterStruct) {
        int    result;
        byte[] response;

        if(!isConnected_) {
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }

        /* Send the request to update the badge */
        SendCommand(COMMAND_ID_REQUEST_UPDATE, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Could not get START_UPDATE response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        else if(!new String(response).equals("OK")) {
            Log.e(LOG_TAG, "Could not request update: " + new String(response));
        }

        /* Get the last firmware binary and info */
        SendCommand(COMMAND_ID_START_UPDATE, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get START_UPDATE response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        updaterStruct.oldVersion = new String(response, StandardCharsets.US_ASCII);

        /* Check the version and get the checksum */
        result = GetUpdateInfo(updaterStruct);
        if(result != EConErrorType.NO_ERROR) {
            SendCommand(COMMAND_ID_CANCEL_UPDATE, null);
            return result;
        }

        /* Get the binary and compare to checksum */
        result = GetFirmwareBinary(updaterStruct);
        if(result!= EConErrorType.NO_ERROR) {
            SendCommand(COMMAND_ID_CANCEL_UPDATE, null);
            return result;
        }

        return EConErrorType.NO_ERROR;
    }

    public int ApplyUpdate(UpdaterStruct updaterStruct, ProgressDialog pd) {
        int         i;
        ByteBuffer  byteBuffer;
        int         toSend;
        byte[]      response;

        /* Apply update */
        SendCommand(COMMAND_ID_VALIDATE_UPDATE, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get VALIDATE_UPDATE response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        if(!new String(response).equals("READY")) {
            return EConErrorType.ECONBADGE_NOT_READY;
        }
        SendCommand(COMMAND_ID_START_TRANS_UPDATE, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get START_TRANS_UPDATE response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        if(!new String(response).equals("READY_TRANS")) {
            return EConErrorType.ECONBADGE_NOT_READY;
        }

        for(i = 0; i < updaterStruct.newFirmwareBinary.length; i += toSend) {
            /* Set the packet header */
            toSend     = Math.min(updaterStruct.newFirmwareBinary.length - i, TXRX_RAW_PACKET_SIZE - 8);
            byteBuffer = ByteBuffer.allocate(toSend + 8);
            byteBuffer.order(ByteOrder.LITTLE_ENDIAN);

            if(updaterStruct.newFirmwareBinary.length - i > TXRX_RAW_PACKET_SIZE - 8) {
                byteBuffer.putInt(0);
            }
            else {
                byteBuffer.putInt(UPDATE_PACKET_MARKER_END);
            }
            byteBuffer.putInt(toSend + 8);
            byteBuffer.put(updaterStruct.newFirmwareBinary, i, toSend);

            Log.d(LOG_TAG, "Update: transmitting " + toSend + 8 + " bytes");
            if(!TransmitData(byteBuffer.array())) {
                return EConErrorType.BT_CANNOT_CONNECT;
            }

            if(toSend == TXRX_RAW_PACKET_SIZE - 8) {
                pd.setMessage("Applying update: " + (i * 100 / updaterStruct.newFirmwareBinary.length) + "%");
            }
            else {
                pd.setMessage("Waiting for update cleanup...");
            }

            /* Wait for response */
            response = ReceiveData(RXTX_TIMEOUT);
            if(response == null) {
                Log.e(LOG_TAG, "Count not get packet update OK response.");
                return EConErrorType.CANNOT_CONTACT_ECONBADGE;
            }
            if(!new String(response).equals("OK")) {
                return EConErrorType.ECONBADGE_NOT_READY;
            }
        }

        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get END_UPDATE response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        if(!new String(response).equals("UPDATE_SUCCESS")) {
            return EConErrorType.ECONBADGE_NOT_READY;
        }

        return EConErrorType.NO_ERROR;
    }

    public int CancelUpdate() {
        /* Cancel update */
        SendCommand(COMMAND_ID_CANCEL_UPDATE, null);

        return EConErrorType.NO_ERROR;
    }

    public boolean SetLedBorderPatternValues(LedBorderViewModel ledBorderViewModel,
                                             int colorIdx,
                                             int color) {
        byte[]           response;
        ByteBuffer       commandData;
        LedBorderPattern newPattern;
        int              i;

        if(!isConnected_) {
            return false;
        }

        /* Check bounds */
        if(colorIdx > 8) {
            return false;
        }

        /* Copy the current pattern */
        newPattern = ledBorderViewModel.GetColorPattern();

        /* Update the value */
        if(colorIdx > 4) {
            newPattern.SetEndColorCode(colorIdx - 5, color);
        }
        else if(colorIdx > 0) {
            newPattern.SetStartColorCode(colorIdx - 1, color);
        }
        else {
            newPattern.SetPlainColorCode(color);
        }

        /* Prepare the command */
        commandData = newPattern.GetRawData();

        SendCommand(COMMAND_ID_SET_PATTERN_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set pattern request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_PATTERN response.");
            return false;
        }

        ledBorderViewModel.SetColorPattern(newPattern);
        return true;
    }

    public boolean SetLedBorderState(LedBorderViewModel ledBorderViewModel, boolean enabled) {
        byte[] response;

        if(!isConnected_) {
            return false;
        }
        if(enabled) {
            SendCommand(COMMAND_ID_ENABLE_LED_BORDER, null);
        }
        else {
            SendCommand(COMMAND_ID_DISABLE_LED_BORDER, null);
        }
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set led border state request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET LED BORDER STATE response.");
            return false;
        }

        ledBorderViewModel.SetEnabled(enabled);
        return true;
    }

    public boolean SetLedBorderPatternType(LedBorderViewModel ledBorderViewModel, int typeId) {
        byte[]           response;
        ByteBuffer       commandData;
        LedBorderPattern newPattern;
        int              i;

        if(!isConnected_) {
            return false;
        }

        /* Check bounds */
        if(!LedBorderPattern.CheckTypeIdx(typeId)) {
            return false;
        }

        /* Copy the current pattern */
        newPattern = ledBorderViewModel.GetColorPattern();

        /* Update the value */
        newPattern.SetTypeIdx(typeId);

        /* Prepare the command */
        commandData = newPattern.GetRawData();

        SendCommand(COMMAND_ID_SET_PATTERN_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set pattern request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_PATTERN response.");
            return false;
        }

        return GetLedBorderInformation(ledBorderViewModel);
    }

    public boolean SetBrightness(LedBorderViewModel ledBorderViewModel, int brightness) {
        byte[]     response;
        ByteBuffer commandData;
        int        fixedBrightness;

        if(!isConnected_) {
            return false;
        }

        /* Set base to 100 and compute on 255 base */
        if(brightness > 100) {
            fixedBrightness = 100;
        }
        else {
            fixedBrightness = brightness;
        }
        fixedBrightness = fixedBrightness * 255 / 100;

        /* Prepare the command */
        commandData = ByteBuffer.allocate(1);
        commandData.put((byte)fixedBrightness);

        Log.d("HERE", "New brightness: " + fixedBrightness);

        SendCommand(COMMAND_ID_SET_BRIGHTNESS_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set brightness request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_BRIGHTNESS response.");
            return false;
        }

        ledBorderViewModel.SetBrightness(brightness);
        return true;
    }

    public boolean ClearAnimations(LedBorderViewModel ledBorderViewModel) {
        byte[] response;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_CLEAR_ANIM_LED_BORDER, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on clear animation request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get CLEAR_ANIMATION response.");
            return false;
        }

        ledBorderViewModel.ClearAnimations();
        return true;
    }

    public boolean GetLedBorderInformation(LedBorderViewModel ledBorderViewModel) {
        byte[]           response;
        LedBorderPattern newPattern;
        int              responseSize;
        ByteBuffer       buffer;
        int              i;
        int              animCount;
        int              fixedBrightness;

        if(!isConnected_) {
            return false;
        }

        SendCommand(COMMAND_ID_GET_INFO_LED_BORDER, null);
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            responseSize = response.length;
            if(responseSize < 40) {
                Log.e(LOG_TAG, "Error on get ledborder request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get GET_INFO_LEDBORDER response.");
            return false;
        }

        newPattern = new LedBorderPattern();
        buffer = ByteBuffer.wrap(response);
        buffer.order(ByteOrder.LITTLE_ENDIAN);

        /* Set the state */
        ledBorderViewModel.SetEnabled(buffer.get() != 0);

        /* Set the pattern */
        newPattern.SetTypeIdx(buffer.get());
        for(i = 0; i < 4; ++i) {
            newPattern.SetStartColorCode(i, buffer.getInt());
        }
        for(i = 0; i < 4; ++i) {
            newPattern.SetEndColorCode(i, buffer.getInt());
        }

        if(newPattern.GetType() != LedBorderPattern.PATTERN_TYPE.PLAIN) {
            for(i = 0; i < 4; ++i) {
                newPattern.SetGradientSize(i, buffer.get());
            }
        }
        else {
            newPattern.SetPlainColorCode(buffer.getInt());
        }

        ledBorderViewModel.SetColorPattern(newPattern);

        /* Get the brightness */
        fixedBrightness = buffer.get() & 0xFF;
        fixedBrightness = fixedBrightness * 100 / 255;
        ledBorderViewModel.SetBrightness(fixedBrightness);

        /* Get the animations */
        animCount = buffer.get();

        /* Check we read enough */
        if(animCount * 2 + 40 <= responseSize) {
            ledBorderViewModel.ClearAnimations();
            for(i = 0; i < animCount; ++i) {
                ledBorderViewModel.AddAnimation(i, buffer.get(), buffer.get());
            }
        }
        else {
            Log.e(LOG_TAG, "Cannot read animation: response size is " + responseSize +
                           ", expected at least " + animCount * 2 + 40);
        }

        return true;
    }

    public boolean SetLedBorderPatternSize(LedBorderViewModel ledBorderViewModel,
                                           int size,
                                           int gradId) {
        byte[]           response;
        ByteBuffer       commandData;
        LedBorderPattern newPattern;
        int              i;

        if(!isConnected_) {
            return false;
        }

        /* Copy the current pattern */
        newPattern = ledBorderViewModel.GetColorPattern();

        /* Update the value */
        newPattern.SetGradientSize(gradId, size);

        /* Prepare the command */
        commandData = newPattern.GetRawData();

        SendCommand(COMMAND_ID_SET_PATTERN_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set pattern request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get SET_PATTERN response.");
            return false;
        }

        ledBorderViewModel.SetColorPattern(newPattern);
        return true;
    }

    public boolean AddAnimation(LedBorderViewModel ledBorderViewModel,
                                int animationType,
                                int animationSpeed) {
        byte[]      response;
        ByteBuffer  commandData;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        commandData = ByteBuffer.allocate(2);
        commandData.put((byte)animationType);
        commandData.put((byte)animationSpeed);

        SendCommand(COMMAND_ID_ADD_ANIM_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(Arrays.copyOf(response, 2), StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set pattern request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get ADD_ANIMATION response.");
            return false;
        }

        return true;
    }

    public boolean RemoveAnimation(LedBorderViewModel viewModel, int animationIndex) {
        byte[]      response;
        ByteBuffer  commandData;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        commandData = ByteBuffer.allocate(1);
        commandData.put((byte)animationIndex);

        SendCommand(COMMAND_ID_REM_ANIM_LED_BORDER, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(Arrays.copyOf(response, 2), StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on set pattern request: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get REMOVE_ANIMATION response.");
            return false;
        }

        return GetLedBorderInformation(viewModel);
    }

    public boolean ClearEInk() {
        if(!isConnected_) {
            return false;
        }
        SendCommand(COMMAND_ID_CLEAR_EINK, null);
        return true;
    }

    public int SendEInkImage(String imageName, byte[] imageData, ProgressDialog pd) {
        byte[]      response;
        int         i;
        int         toSend;
        ByteBuffer  commandData;

        if(!isConnected_) {
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }

        /* Reduce the image name */
        if(imageName.length() > 63) {
            imageName = imageName.substring(0, 63);
        }

        /* Prepare the command */
        SendCommand(COMMAND_ID_UPDATE_EINK, imageName.getBytes());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("READY")) {
                Log.e(LOG_TAG, "Error on set EINK Image request: " + new String(response, StandardCharsets.US_ASCII));
                return EConErrorType.ECONBADGE_NOT_READY;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get UPDATE EINK response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }

        /* Send the image */
        for(i = 0; i < imageData.length; i += toSend) {
            /* Set the packet header */
            toSend      = Math.min(imageData.length - i, TXRX_RAW_PACKET_SIZE);
            commandData = ByteBuffer.allocate(toSend);
            commandData.order(ByteOrder.BIG_ENDIAN);
            commandData.put(imageData, i, toSend);

            Log.d(LOG_TAG, "IMAGE: transmitting " + toSend + 8 + " bytes");
            if(!TransmitData(commandData.array())) {
                return EConErrorType.BT_CANNOT_CONNECT;
            }

            if(toSend == TXRX_RAW_PACKET_SIZE) {
                pd.setMessage("Updating image: " + (i * 100 / imageData.length) + "%");
            }
            else {
                pd.setMessage("Waiting for update cleanup...");
            }

            /* Wait for response */
            response = ReceiveData(RXTX_TIMEOUT);
            if(response == null) {
                Log.e(LOG_TAG, "Count not get packet update OK response.");
                return EConErrorType.CANNOT_CONTACT_ECONBADGE;
            }
            if(!new String(response).equals("OK")) {
                return EConErrorType.ECONBADGE_NOT_READY;
            }
        }

        /* Wait for acknowledge */
        response = ReceiveData(RX_LONG_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get packet EINK Updated UPDATED response.");
            return EConErrorType.CANNOT_CONTACT_ECONBADGE;
        }
        if(!new String(response).equals("UPDATED")) {
            return EConErrorType.ECONBADGE_NOT_READY;
        }

        return EConErrorType.NO_ERROR;
    }

    public boolean GetImagesName(List<String> imageNames, int numberOfNamesToGet) {
        byte[]      response;
        ByteBuffer  commandData;
        StringBuilder newName;
        int         i;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        commandData = ByteBuffer.allocate(8);
        commandData.order(ByteOrder.LITTLE_ENDIAN);
        commandData.putInt(imageNames.size());
        commandData.putInt(numberOfNamesToGet);

        SendCommand(COMMAND_ID_GET_IMAGES_NAME, commandData.array());
        response = ReceiveData(RXTX_TIMEOUT);
        if(response == null) {
            Log.e(LOG_TAG, "Count not get COMMAND_ID_GET_IMAGES_NAME response.");
            return false;
        }

        /* Recompose the new images name */
        newName = new StringBuilder("");
        for(i = 0; i < response.length; ++i) {
            /* Split with separator */
            if(response[i] == 0) {
                if(newName.length() > 0) {
                    imageNames.add(newName.toString());
                    newName = new StringBuilder("");
                }
            }
            else {
                newName.append((char)response[i]);
            }
        }
        if(newName.length() > 0) {
            imageNames.add(newName.toString());
        }

        return true;
    }

    public boolean RemoveImage(String imageName) {
        byte[]     response;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        SendCommand(COMMAND_ID_REMOVE_IMAGE, imageName.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData(RXTX_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on image removal: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get COMMAND_ID_REMOVE_IMAGE response.");
            return false;
        }

        return true;
    }

    public boolean SelectLoadedImage(String imageName) {
        byte[]     response;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command */
        SendCommand(COMMAND_ID_SELECT_LOADED_IMAGE, imageName.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData(RX_LONG_TIMEOUT);
        if(response != null) {
            if(!new String(response, StandardCharsets.US_ASCII).equals("OK")) {
                Log.e(LOG_TAG, "Error on image removal: " + new String(response, StandardCharsets.US_ASCII));
                return false;
            }
        }
        else {
            Log.e(LOG_TAG, "Count not get COMMAND_ID_SELECT_LOADED_IMAGE response.");
            return false;
        }

        return true;
    }

    public boolean GetImageData(String imageName, EInkImage image) {
        byte[] response;

        if(!isConnected_) {
            return false;
        }

        /* Prepare the command and set raw receiving mode */
        SetReceivingMode(RECV_MODE_RAW);
        SendCommand(COMMAND_ID_GET_CURRENT_IMAGE, imageName.getBytes(StandardCharsets.US_ASCII));
        response = ReceiveData(RX_LONG_TIMEOUT);
        SetReceivingMode(RECV_MODE_NORMAL);

        if(response != null) {
            image.setImageData(response);
        }
        else {
            return false;
        }

        return true;
    }
}
