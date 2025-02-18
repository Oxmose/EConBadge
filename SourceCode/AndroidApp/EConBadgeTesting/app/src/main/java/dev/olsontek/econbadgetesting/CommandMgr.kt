package dev.olsontek.econbadgetesting

import android.annotation.SuppressLint
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.util.Log
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.lang.Integer.min
import java.util.UUID

private const val ECB_SERVICE_UUID = "d3e63261-0000-1000-8000-00805f9b34fb"
private const val ECB_COMMAND_UUID = "2d3a8ac3-0000-1000-8000-00805f9b34fb"
private const val ECB_COMMAND_DEFER_UUID = "45fd43da-0000-1000-8000-00805f9b34fb"
private const val ECB_DATA_UUID = "83670c18-0000-1000-8000-00805f9b34fb"
private const val ECB_HW_VERSION_UUID = "997ca8f9-0000-1000-8000-00805f9b34fb"
private const val ECB_SW_VERSION_UUID = "20a14f57-0000-1000-8000-00805f9b34fb"
private const val ECB_SET_TOKEN__UUID = "02b31496-0000-1000-8000-00805f9b34fb"
private const val ECB_TOKEN = "0000000000000000"
private const val ECB_TOKENT = "0000000000000011"
class CommandMgr {
    fun init(bleManager: BLEManager, activity: MainActivity) {
        bleManager_ = bleManager
        mainAct = activity

        runBlocking {
            launch {
                Log.d("CMD", "Init command manager")

                val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
                val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
                val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

                if (commandChar != null) {
                    mainAct!!.waitForDescriptorReady()
                    mainAct!!.enableNotifications(commandChar)

                }

                val commandDefCharUUID = UUID.fromString(ECB_COMMAND_DEFER_UUID)
                val commandDefChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandDefCharUUID)

                if (commandDefChar != null) {
                    mainAct!!.waitForDescriptorReady()
                    mainAct!!.enableNotifications(commandDefChar)
                }

                val dataCharUUID = UUID.fromString(ECB_DATA_UUID)
                val dataChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(dataCharUUID)

                if (dataChar != null) {
                    mainAct!!.waitForDescriptorReady()
                    mainAct!!.enableNotifications(dataChar)
                }
            }
        }

    }
    @SuppressLint("MissingPermission")
    fun sendPing() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(0)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }

        Log.d("BLE", "Sent Ping to Char")
    }

    @SuppressLint("MissingPermission")
    fun getSWVersion() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val charUUID = UUID.fromString(ECB_SW_VERSION_UUID)
        val charObj = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(charUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII)
        if (charObj != null) {
            ecbGatt?.readCharacteristic(charObj)
        }
    }

    @SuppressLint("MissingPermission")
    fun getHWVersion() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val charUUID = UUID.fromString(ECB_HW_VERSION_UUID)
        val charObj = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(charUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII)
        if (charObj != null) {
            ecbGatt?.readCharacteristic(charObj)
        }
    }

    @SuppressLint("MissingPermission")
    fun setToken() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(1) + ECB_TOKENT.toByteArray(Charsets.US_ASCII)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }
    }

    @SuppressLint("MissingPermission")
    fun einkClear() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(2)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }
    }

    @SuppressLint("MissingPermission")
    fun einkNewImage() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(3) + "NewImageTest.img".toByteArray(Charsets.US_ASCII)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }

        runBlocking {
            launch {
                /* Wait for ack */
                mainAct?.waitCommandResponse("Starting upload")

                /* Send image by chunk while waiting for ACK */
                sendImage()

                /* Wait for end of command */
                mainAct?.waitDeferedResponse("Updloaded New Image")
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkRemove() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(4) + "NewImageTest.img".toByteArray(Charsets.US_ASCII)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }
    }

    @SuppressLint("MissingPermission")
    fun einkSelect() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(5) + "Olson2.bmp".toByteArray(Charsets.US_ASCII)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }

        runBlocking {
            launch {

                mainAct?.waitDeferedResponse("Selected Image")
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkGetImageName() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(6)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }
    }

    @SuppressLint("MissingPermission")
    fun einkGetImage() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_COMMAND_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        val byteArray = ECB_TOKEN.toByteArray(Charsets.US_ASCII) + byteArrayOf(7)
        if (commandChar != null) {
            ecbGatt?.writeCharacteristic(commandChar, byteArray, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
        }

        runBlocking {
            launch {
                /* Wait for data and read */
                var offset = 0
                var toRead = 134400
                while (toRead > 0) {
                    val readBytes = mainAct?.getData(imageBytes, offset);
                    if (readBytes != null) {
                        offset += readBytes
                        toRead -= readBytes
                    }
                    Log.d("BLE", "Left to read " + toRead.toString())
                }

                mainAct?.waitDeferedResponse("Downloaded Image")
            }
        }
    }

    @SuppressLint("MissingPermission", "NewApi")
    fun sendImage() {
        val ecbMainServiceUUID = UUID.fromString(ECB_SERVICE_UUID)
        val commandCharUUID = UUID.fromString(ECB_DATA_UUID)
        val commandChar = ecbGatt?.getService(ecbMainServiceUUID)?.getCharacteristic(commandCharUUID)

        runBlocking {
            launch {
                /* Wait for data and read */
                var offset = 0
                var readBytes = 0
                var toRead = 134400
                var buffer = ByteArray(512)
                while (toRead > 0) {
                    readBytes = min(toRead, 512)

                    Log.d("BLE", "Left to send " + toRead.toString())

                    mainAct?.waitForDataSent()
                    imageBytes.copyInto(buffer, 0, offset, offset + readBytes)
                    if (commandChar != null) {
                        Log.d(
                            "BLE",
                            ecbGatt?.writeCharacteristic(
                                commandChar,
                                buffer,
                                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                            ).toString()
                        )
                    }

                    Log.d("BLE", "Send " + readBytes.toString() + " to " + commandChar)
                    offset += readBytes
                    toRead -= readBytes
                }

                mainAct?.waitDeferedResponse("Uploaded Image")
            }
        }
    }


    private var imageBytes = ByteArray(134400)
    private var bleManager_: BLEManager? = null
    private var mainAct: MainActivity? = null
}