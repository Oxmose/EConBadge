package dev.olsontek.econbadgetesting

import android.annotation.SuppressLint
import android.util.Log
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.lang.Integer.min

private const val ECB_TOKENT = "0000000000000011"

@OptIn(ExperimentalStdlibApi::class)
class CommandMgr {
    fun init(bleManager: BLEManager, activity: MainActivity) {
        bleManager_ = bleManager
        mainAct = activity
    }
    @SuppressLint("MissingPermission")
    fun sendPing() {
        val response = ByteArray(256)
        bleManager_?.sendCommand(0, ByteArray(0))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun getSWVersion() {
        val response = ByteArray(256)

        bleManager_?.readCharacteristic(BLEManager.ECharacteristic.SW_VERSION)

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCharacteristicRead(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun getHWVersion() {
        val response = ByteArray(256)

        bleManager_?.readCharacteristic(BLEManager.ECharacteristic.HW_VERSION)

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCharacteristicRead(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun setToken() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(1, ECB_TOKENT.toByteArray(Charsets.US_ASCII))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkClear() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(2, ByteArray(0))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkNewImage() {
        val response = ByteArray(256)

        mainAct?.setDebugText("Starting Upload");
        bleManager_?.sendCommand(3, "NewImageTest.img".toByteArray(Charsets.US_ASCII))


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

                    imageBytes.copyInto(buffer, 0, offset, offset + readBytes)
                    bleManager_?.sendData(buffer)


                    Log.d("BLE", "Send " + readBytes.toString())
                    offset += readBytes
                    toRead -= readBytes
                }

                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkRemove() {
        val response = ByteArray(256)

        mainAct?.setDebugText("Starting Removal")
        bleManager_?.sendCommand(4, "NewImageTest.img".toByteArray(Charsets.US_ASCII))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkSelect() {
        val response = ByteArray(256)

        mainAct?.setDebugText("Starting Image selection")
        bleManager_?.sendCommand(5, "NewImageTest.img".toByteArray(Charsets.US_ASCII))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkGetImageName() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(6, ByteArray(0))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun einkGetImage() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(7, ByteArray(0))

        runBlocking {
            launch {
                /* Wait for data and read */
                var offset = 0
                var toRead = 134400
                while (toRead > 0) {
                    val readBytes = bleManager_?.receiveData(imageBytes, offset);
                    if (readBytes != null) {
                        offset += readBytes
                        toRead -= readBytes
                    }
                    Log.d("BLE", "Left to read " + toRead.toString())
                }

                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    fun factoryReset() {
        bleManager_?.sendCommand(8, ByteArray(0))
    }

    @SuppressLint("MissingPermission")
    fun setOwner() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(9, "TestOwner".toByteArray(Charsets.US_ASCII))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun setContact() {
        val response = ByteArray(256)

        bleManager_?.sendCommand(10, "TestContact".toByteArray(Charsets.US_ASCII))

        runBlocking {
            launch {
                /* Wait for end of command */
                bleManager_?.waitCommandResponse(response)
                mainAct?.setDebugText(response.toHexString());
            }
        }
    }

    private var imageBytes = ByteArray(134400)
    private var bleManager_: BLEManager? = null
    private var mainAct: MainActivity? = null
}