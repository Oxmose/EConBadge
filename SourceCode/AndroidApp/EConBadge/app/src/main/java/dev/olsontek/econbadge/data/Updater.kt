package dev.olsontek.econbadge.data

import android.util.Log
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.connectivity.CommandManager.CommandError
import dev.olsontek.econbadge.connectivity.CommandManager.ECBResponse
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader
import java.net.URL
import javax.net.ssl.HttpsURLConnection

private const val ECB_UPDATE_SERVER = "https://olsontek.dev/EConBadge/"
private const val ECB_UPDATE_FIRMWARE = "firmware_latest.bin"
private const val ECB_UPDATE_VERSION = "firmware_latest.bin.version"
private const val ECB_UPDATE_HASH = "firmware_latest.bin.hash"
private const val ECB_UPDATE_SIG = "firmware_latest.bin.sig"

private const val MODULE_NAME = "Updater"

class Updater(private val commandManager: CommandManager) {

    private val updateData = UpdateData()

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

    fun checkForUpdate(
        swVersion: String,
        hwVersion: String,
        callback: (CommandError) -> Unit
    ) {
        /* Get the current data */
        val splitSwVersion = swVersion.substringBefore("\u0000").split(' ')
        updateData.currentHwVersion = hwVersion
        updateData.currentSwVersion = splitSwVersion[0].split('.') as ArrayList<String>
        updateData.currentSwDate = splitSwVersion[1] + " " + splitSwVersion[2]

        val networkThread = Thread {
            try {
                runBlocking {
                    launch {
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
                    }
                }

                Log.d(MODULE_NAME, "Update information\n $updateData")

                /* Check the hardware compatibility */
                var found = false
                for (item in updateData.serverHwCompat) {
                    if(item == updateData.currentHwVersion) {
                        found = true
                        break
                    }
                }

                if (!found) {
                    callback(CommandError.HW_NOT_COMPATIBLE)
                    return@Thread
                }

                /* Check for upgrade / downgrade */
                for (i in 0..< updateData.serverSwVersion.size) {
                    if(updateData.serverSwVersion[i] > updateData.currentSwVersion[i]) {
                        callback(CommandError.SUCCESS)
                        return@Thread
                    }
                }

                /* If we are here, not update is available */
                callback(CommandError.NO_UPDATE_AVAILABLE)
            }
            catch (e: IOException) {
                callback(CommandError.COMM_ERROR)
            }
        }

        networkThread.start()
    }

    fun applyUpdate(callback: (CommandError, ECBResponse?) -> Unit) {

    }

    fun getLatestVersion(): String {
        var version = ""
        for (item in updateData.serverSwVersion) {
            version += "$item."
        }
        return version.substring(0, version.length - 1)
    }

}