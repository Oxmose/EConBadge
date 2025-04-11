package dev.olsontek.econbadge.data

import android.util.Log
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.connectivity.CommandManager.CommandStatus
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.net.URL
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.security.MessageDigest
import javax.net.ssl.HttpsURLConnection

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* Current module name */
private const val MODULE_NAME = "Updater"

/* Update server base URL */
private const val ECB_UPDATE_SERVER = "https://olsontek.dev/EConBadge/"

/* Latest firmware binary name */
private const val ECB_UPDATE_FIRMWARE = "firmware_latest.bin"

/* Latest firmware version name */
private const val ECB_UPDATE_VERSION = "firmware_latest.bin.version"

/* Latest firmware hash name */
private const val ECB_UPDATE_HASH = "firmware_latest.bin.hash"

/* Latest firmware Signature name */
private const val ECB_UPDATE_SIG = "firmware_latest.bin.sig"

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class Updater(private val commandManager: CommandManager) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/

    /* Gathers the update information */
    private class UpdateData {

        var serverHash = ByteArray(0)
        var serverSignature = ByteArray(0)
        var serverSwVersion = ArrayList<String>()
        var serverSwDate = ""
        var serverHwCompat = ArrayList<String>()
        var currentSwVersion = ArrayList<String>()
        var currentSwDate = ""
        var currentHwVersion = ""
        var newFirmwareData = ByteArray(0)

        @OptIn(ExperimentalStdlibApi::class)
        override fun toString(): String {
            return  "Latest Version: " + serverSwVersion.toString() +
                    "\nLatest Date: " + serverSwDate +
                    "\nLatest HW Compat: " + serverHwCompat.toString() +
                    "\nCurrent Version: " + currentSwVersion.toString() +
                    "\nCurrent Date: " + currentSwDate +
                    "\nCurrent HW Compat: " + currentHwVersion +
                    "\nFirmware size: " + newFirmwareData.size +
                    "\nFirmware hash: " + serverHash.toHexString() +
                    "\nFirmware signature: " + serverSignature.toHexString()
        }
    }

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* Update data information */
    private val updateData = UpdateData()

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/

    private fun performUpdate(
        progressCallback: (CommandStatus, Float) -> Unit,
        updateEndCallback: (CommandStatus) -> Unit,
        updateIdentifier: Int
    )
    {
        val header = ByteBuffer.allocate(312)

        /* Send the header */
        header.order(ByteOrder.LITTLE_ENDIAN)
        header.putInt(0xECB0C0DE.toInt())
        header.putInt(updateData.newFirmwareData.size)
        header.put(updateData.serverHash)
        header.put(updateData.serverSignature)
        header.put(updateData.currentHwVersion.toByteArray())
        header.put(ByteArray(header.remaining()))
        commandManager.sendUpdateHeader(header.array(), updateIdentifier) {
            commandStatus: CommandStatus ->

            if (commandStatus != CommandStatus.SUCCESS) {
                updateEndCallback(commandStatus)
                return@sendUpdateHeader
            }

            /* Send the update */
            commandManager.sendUpdateData(
                updateData.newFirmwareData,
                updateIdentifier,
                processCallback = {
                    progress: Float ->

                    /* Send progress */
                    progressCallback(
                        CommandStatus.SENDING_UPDATE,
                        0.5f + progress / 2.0f
                    )
                },
                sendEndCallback = updateEndCallback
            )
        }
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    fun checkForUpdate(
        swVersion: String,
        hwVersion: String,
        callback: (CommandStatus) -> Unit
    ) {
        /* Get the current version data */
        val splitSwVersion = swVersion.substringBefore("\u0000").split(' ')
        updateData.currentHwVersion = hwVersion
        updateData.currentSwVersion = splitSwVersion[0].split('.') as ArrayList<String>
        updateData.currentSwDate = splitSwVersion[1] + " " + splitSwVersion[2]

        val networkThread = Thread {
            try {
                val buffer = ByteArray(65536)

                /* Get the latest version information */
                var url = URL("$ECB_UPDATE_SERVER/$ECB_UPDATE_VERSION")
                var urlConn = url.openConnection() as HttpsURLConnection
                val buffReader = BufferedReader(InputStreamReader(urlConn.inputStream))
                var line: String?
                val content = StringBuilder()
                while (buffReader.readLine().also { line = it } != null) {
                    content.append(line)
                }

                /* Extract the version information */
                val splitInfo = content.split(' ')
                updateData.serverSwVersion = splitInfo[0].split('.') as ArrayList<String>
                updateData.serverSwDate = splitInfo[1] + " " + splitInfo[2]
                for (i in 3..< splitInfo.size step 2) {
                    updateData.serverHwCompat.add(splitInfo[i] + " " + splitInfo[i + 1])
                }

                /* Get the latest firmware hash */
                url = URL("$ECB_UPDATE_SERVER/$ECB_UPDATE_HASH")
                urlConn = url.openConnection() as HttpsURLConnection
                var inputStream = urlConn.inputStream
                updateData.serverHash = ByteArray(0)
                while (true) {
                    val size = inputStream.read(buffer)
                    if (size <= 0) {
                        break
                    }
                    updateData.serverHash += buffer.slice(0..< size)
                }

                /* Get the latest firmware signature */
                url = URL("$ECB_UPDATE_SERVER/$ECB_UPDATE_SIG")
                urlConn = url.openConnection() as HttpsURLConnection
                inputStream = urlConn.inputStream
                updateData.serverSignature = ByteArray(0)
                while (true) {
                    val size = inputStream.read(buffer)
                    if (size <= 0) {
                        break
                    }
                    updateData.serverSignature += buffer.slice(0..< size)
                }

                Log.d(MODULE_NAME, "Update package information\n $updateData")

                /* Check the hardware compatibility */
                var found = false
                for (item in updateData.serverHwCompat) {
                    if(item == updateData.currentHwVersion) {
                        found = true
                        break
                    }
                }

                if (!found) {
                    callback(CommandStatus.HW_NOT_COMPATIBLE)
                    return@Thread
                }

                /* Check for upgrade / downgrade */
                for (i in 0..< updateData.serverSwVersion.size) {
                    if(updateData.serverSwVersion[i] > updateData.currentSwVersion[i]) {
                        callback(CommandStatus.SUCCESS)
                        return@Thread
                    }
                }

                /* If we are here, not update is available */
                callback(CommandStatus.NO_UPDATE_AVAILABLE)
            }
            catch (e: IOException) {
                callback(CommandStatus.COMM_ERROR)
            }
        }

        /* Start the network thread */
        networkThread.start()
    }

    fun requestFirmwareUpdate(
        progressCallback: (CommandStatus, Float) -> Unit,
        updateEndCallback: (CommandStatus) -> Unit
    )
    {
        val networkThread = Thread {
            try {
                /* Download the file */
                val url = URL("$ECB_UPDATE_SERVER/$ECB_UPDATE_FIRMWARE")
                val urlConn = url.openConnection() as HttpsURLConnection
                val inputStream = urlConn.inputStream
                val totalSize = urlConn.contentLength
                val buffer = ByteArray(65536)
                updateData.newFirmwareData = ByteArray(0)
                while (true) {
                    val size = inputStream.read(buffer)
                    if (size <= 0) {
                        break
                    }
                    updateData.newFirmwareData += buffer.slice(0..<size)
                    progressCallback(
                        CommandStatus.DOWNLOADING_UPDATE,
                        updateData.newFirmwareData.size.toFloat() / (totalSize * 2.0f)
                    )
                }

                /* Check the file integrity */
                val md = MessageDigest.getInstance("SHA-256")
                val hash = md.digest(updateData.newFirmwareData)

                if (!hash.contentEquals(updateData.serverHash)) {
                    updateEndCallback(CommandStatus.CORRUPTED_DATA)
                    return@Thread
                }

                /* Send the file to the badge */
                commandManager.requestUpdate {commandStatus: CommandStatus,
                                              updateIdentifier: Int ->

                    if (commandStatus != CommandStatus.SUCCESS) {
                        updateEndCallback(commandStatus)
                    }
                    else {
                        performUpdate(progressCallback, updateEndCallback, updateIdentifier)
                    }
                }
            }
            catch (e: IOException) {
                updateEndCallback(CommandStatus.COMM_ERROR)
            }
        }

        /* Start the network thread */
        networkThread.start()
    }

    fun getLatestVersion(): String {
        var version = ""
        for (item in updateData.serverSwVersion) {
            version += "$item."
        }
        return version.substring(0, version.length - 1)
    }
}