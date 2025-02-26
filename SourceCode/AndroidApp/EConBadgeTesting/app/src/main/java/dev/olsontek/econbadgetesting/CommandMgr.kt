package dev.olsontek.econbadgetesting

import android.annotation.SuppressLint
import android.util.Log
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.lang.Integer.min
import java.math.BigInteger
import java.net.URL
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.security.MessageDigest
import javax.net.ssl.HttpsURLConnection

private const val ECB_TOKENT = "0000000000000011"
private const val ECB_UPDATE_SERVER = "https://olsontek.dev/EConBadge/"
private const val ECB_UPDATE_FIRMWARE = "firmware_latest.bin"
private const val ECB_UPDATE_VERSION = "firmware_latest.bin.version"
private const val ECB_UPDATE_HASH = "firmware_latest.bin.hash"
private const val ECB_UPDATE_SIG = "firmware_latest.bin.sig"

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
                bleManager_?.sendData(imageBytes)

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

    class UpdateData {

        override fun toString(): String {
            return  "Hash: " + serverHash +
                    "\nSw Version: " + serverSwVersion.toString() +
                    "\nSw Date: " + serverSwDate +
                    "\nHw Compat: " + serverHwCompat.toString() +
                    "\nCurrent Sw Version: " + currentSwVersion.toString() +
                    "\nCurrent Sw Date: " + currentSwDate +
                    "\nCurrent Hw: " + currentHwVersion +
                    "\nFirmware size: " + newFirmwareData.size +
                    "\nFirmware hash: " + serverHash.toHexString() +
                    "\nFirmware signature: " + serverSignature.toHexString()


        }

        var serverHash = ByteArray(0)
            get() = field
            set(value) {
                field = value
            }
        var serverSignature = ByteArray(0)
            get() = field
            set(value) {
                field = value
            }
        var serverSwVersion = ArrayList<String>()
            get() = field
            set(value) {
                field = value
            }
        var serverSwDate = ""
            get() = field
            set(value) {
                field = value
            }
        var serverHwCompat = ArrayList<String>()
            get() = field
            set(value) {
                field = value
            }
        var currentSwVersion = ArrayList<String>()
            get() = field
            set(value) {
                field = value
            }
        var currentSwDate = ""
            get() = field
            set(value) {
                field = value
            }
        var currentHwVersion = ""
            get() = field
            set(value) {
                field = value
            }
        var newFirmwareData = ByteArray(0)
            get() = field
            set(value) {
                field = value
            }
    }

    fun getUpdateInfo(data: UpdateData) {
        val networkThread = Thread({
            try {
                runBlocking {
                    launch {
                        val buffer = ByteArray(65536)

                        val url = URL(ECB_UPDATE_SERVER + "/" + ECB_UPDATE_VERSION)
                        val uc: HttpsURLConnection = url.openConnection() as HttpsURLConnection
                        val br = BufferedReader(InputStreamReader(uc.getInputStream()))
                        var line: String?
                        val lin2 = StringBuilder()
                        while (br.readLine().also { line = it } != null) {
                            lin2.append(line)
                        }

                        val splitInfo = lin2.split(' ')
                        data.serverSwVersion = splitInfo[0].split('.') as ArrayList<String>
                        data.serverSwDate = splitInfo[1] + " " + splitInfo[2]
                        for (i in 3..splitInfo.size - 1 step 2) {
                            data.serverHwCompat.add(splitInfo[i] + " " + splitInfo[i + 1])
                        }

                        val url2 = URL(ECB_UPDATE_SERVER + "/" + ECB_UPDATE_HASH)
                        val uc2: HttpsURLConnection = url2.openConnection() as HttpsURLConnection
                        val br2 = uc2.getInputStream()
                        data.serverHash = ByteArray(0)
                        while (true) {
                            val size = br2.read(buffer)
                            if(size <= 0) {
                                break
                            }
                            data.serverHash += buffer.slice(0..size - 1)
                        }

                        val url3 = URL(ECB_UPDATE_SERVER + "/" + ECB_UPDATE_SIG)
                        val uc3: HttpsURLConnection = url3.openConnection() as HttpsURLConnection
                        val br3 = uc3.getInputStream()
                        data.serverSignature = ByteArray(0)
                        while (true) {
                            val size = br3.read(buffer)
                            if(size <= 0) {
                                break
                            }
                            data.serverSignature += buffer.slice(0..size - 1)
                        }
                    }
                }

            } catch (e: IOException) {
                Log.d("texts", "onClick: " + e.getLocalizedMessage())
                e.printStackTrace()
            }
        })

        networkThread.start()
        networkThread.join()
    }

    suspend fun getCurrentInfo(data: UpdateData) {
        val response = ByteArray(256)

        /* Get the hardware version and check compatibility */
        bleManager_?.readCharacteristic(BLEManager.ECharacteristic.HW_VERSION)
        bleManager_?.waitCharacteristicRead(response)

        data.currentHwVersion = response.toString(Charsets.US_ASCII).substringBefore("\u0000")


        bleManager_?.readCharacteristic(BLEManager.ECharacteristic.SW_VERSION)
        bleManager_?.waitCharacteristicRead(response)

        val currentSw = response.toString(Charsets.US_ASCII).substringBefore("\u0000").split(' ')
        data.currentSwVersion = currentSw[0].split('.') as ArrayList<String>
        data.currentSwDate = currentSw[1] + " " + currentSw[2]
    }

    fun getFirmwareBinary(updateData: UpdateData) {
        val networkThread = Thread({
            try {
                runBlocking {
                    launch {
                        val url = URL(ECB_UPDATE_SERVER + "/" + ECB_UPDATE_FIRMWARE)
                        val uc: HttpsURLConnection = url.openConnection() as HttpsURLConnection
                        val br = uc.getInputStream()
                        val buffer = ByteArray(65536)
                        updateData.newFirmwareData = ByteArray(0)
                        while (true) {
                            val size = br.read(buffer)
                            if(size <= 0) {
                                break
                            }
                            updateData.newFirmwareData += buffer.slice(0..size - 1)
                        }
                    }
                }

            } catch (e: IOException) {
                Log.d("texts", "onClick: " + e.getLocalizedMessage())
                e.printStackTrace()
            }
        })

        networkThread.start()
        networkThread.join()
    }

    fun validateUpdate(updateData: UpdateData) : Boolean {
        /* Check HW compat */
        var found = false
        for(item in updateData.serverHwCompat) {
            if(item == updateData.currentHwVersion) {
                found = true
            }
        }
        if(!found) {
            return false
        }

        /* Check Upgrade / Downgrade */
        var upgrade = false
        for (i in 0..updateData.serverSwVersion.size - 1) {
            if(updateData.serverSwVersion[i] < updateData.serverSwVersion[i]) {
                upgrade = true
                break
            }
        }

        if(upgrade) {
            Log.d("Upgrade!", "Upgrade!")
        }
        else {
            Log.d("Downgrade!", "Downgrade!")
        }


        /* Check firmware checksum */
        val md = MessageDigest.getInstance("SHA-256")
        val hash = md.digest(updateData.newFirmwareData)
        Log.d("Update", "Comparing " + hash.toHexString() + " to " + updateData.serverHash.toHexString())


        return hash.contentEquals(updateData.serverHash)
    }

    suspend fun launchUpdate(updateData: UpdateData) {
        val response = ByteArray(256)
        val buffer = ByteBuffer.allocate(312)

        bleManager_?.sendCommand(11, ByteArray(0))

        bleManager_?.waitCommandResponse(response)
        mainAct?.setDebugText(response.toHexString());

        /* Send header */
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putInt(0xECB0C0DE.toInt())
        buffer.putInt(updateData.newFirmwareData.size)
        buffer.put(updateData.serverHash)
        buffer.put(updateData.serverSignature)
        buffer.put(updateData.currentHwVersion.toByteArray())
        buffer.put(ByteArray(buffer.remaining()))
        bleManager_?.sendData(buffer.array())

        /* Wait for data and read */
        bleManager_?.sendData(updateData.newFirmwareData)

        /* Wait for end of command */
        bleManager_?.waitCommandResponse(response)
        mainAct?.setDebugText(response.toHexString());
    }

    fun performUpdate() {
        /* Get the latest firmware info */
        Thread({

            val updateData = UpdateData()
            try {
                runBlocking {
                    launch {
                        getUpdateInfo(updateData)
                        getCurrentInfo(updateData)
                        getFirmwareBinary(updateData)
                        Log.d("UpdateInfo", updateData.toString())
                        if(validateUpdate(updateData)) {
                            launchUpdate(updateData)
                        }
                    }
                }

            } catch (e: IOException) {
                Log.d("texts", "onClick: " + e.getLocalizedMessage())
                e.printStackTrace()
            }
        }).start()
    }

    private var imageBytes = ByteArray(134400)
    private var bleManager_: BLEManager? = null
    private var mainAct: MainActivity? = null
}