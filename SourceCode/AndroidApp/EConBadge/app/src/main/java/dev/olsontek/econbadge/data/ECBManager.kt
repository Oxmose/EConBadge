package dev.olsontek.econbadge.data

import android.graphics.Bitmap
import android.os.Handler
import android.os.Looper
import androidx.lifecycle.ViewModelProvider
import dev.olsontek.econbadge.MainActivity
import dev.olsontek.econbadge.R
import dev.olsontek.econbadge.connectivity.CommandManager
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager.ECBBleError
import dev.olsontek.econbadge.connectivity.ECBBluetoothManager.EventCallbackHandler
import dev.olsontek.econbadge.ui.eink.EInkImageGridModel
import dev.olsontek.econbadge.ui.eink.EInkViewModel
import dev.olsontek.econbadge.ui.home.HomeViewModel
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.util.concurrent.atomic.AtomicInteger

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Maximal length of the owner and contact string */
private const val MAX_OWNER_CONTACT_SIZE = 41

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ECBManager private constructor(activity: MainActivity) : EventCallbackHandler {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* Singleton object for the ECB manager */
    companion object {
        /* Singleton instance */
        @Volatile
        private var instance: ECBManager? = null

        /* Create the singleton instance */
        fun getInstance(activity: MainActivity?) =
            instance ?: synchronized(this) {
                instance ?: ECBManager(activity!!).also {
                    instance = it
                }
            }
    }

    /* Defines the interface for the device events handling */
    interface ECBEventHandler {
        /* Called when a device is connected */
        fun onConnect(status: ECBBleError)

        /* Called when a device is disconnected */
        fun onDisconnect(status: ECBBleError)
    }

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* Main activity in which the manager is used */
    private val applicationContext = activity.applicationContext

    /* Bluetooth manager used to contact the ECB */
    private val bleManager = ECBBluetoothManager.getInstance(applicationContext, this)

    /* Command manager used to send and receive commands */
    private val commandManager = CommandManager(bleManager, applicationContext)

    /* List of even handlers to notify on events */
    private val listeners = ArrayList<ECBEventHandler>()

    /* ECB updater */
    private val updater = Updater(commandManager)

    /* Stores the fragments view models */
    private val homeViewModel = ViewModelProvider(activity)[HomeViewModel::class.java]
    private val einkViewModel = ViewModelProvider(activity)[EInkViewModel::class.java]

    /* Shared data singleton */
    private val sharedData = SharedData.getInstance()

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    override fun onDeviceConnected(status: ECBBleError) {
        var finalStatus = status
        if (finalStatus == ECBBleError.SUCCESS) {
            /* Send Ping command */
            commandManager.sendPing { result: CommandManager.CommandStatus, response: String ->
                if (result != CommandManager.CommandStatus.SUCCESS || response != "PONG") {
                    finalStatus = ECBBleError.NOT_CONNECTED
                    ecbDisconnect()
                }
                for (listener in listeners) {
                    listener.onConnect(finalStatus)
                }
            }
        } else {
            for (listener in listeners) {
                listener.onConnect(finalStatus)
            }
        }
    }

    override fun onDeviceDisconnected(status: ECBBleError) {
        for (listener in listeners) {
            listener.onDisconnect(status)
        }
    }

    fun addEventListener(listener: ECBEventHandler) {
        listeners.add(listener)
    }

    fun ecbConnect() {
        bleManager.connect()
    }

    fun ecbDisconnect() {
        bleManager.disconnect()
    }

    fun retrieveHomeFragmentData(callback: () -> Unit) {
        val barrier = AtomicInteger(4)
        val mainHandler = Handler(Looper.getMainLooper())

        /* Get the software version */
        commandManager.getSoftwareVersion { commandStatus: CommandManager.CommandStatus,
                                            response: String ->
            mainHandler.post {
                when (commandStatus) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        homeViewModel.setSoftwareVersion(response)
                    }

                    else -> {
                        homeViewModel.setSoftwareVersion(
                            applicationContext.getString(R.string.unknown_software_version)
                        )
                    }
                }
                if (barrier.decrementAndGet() == 0) {
                    callback()
                }
            }
        }

        /* Get the hardware version */
        commandManager.getHardwareVersion { commandStatus: CommandManager.CommandStatus,
                                            response: String ->
            mainHandler.post {
                when (commandStatus) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        homeViewModel.setHardwareVersion(response)
                    }

                    else -> {
                        homeViewModel.setHardwareVersion(
                            applicationContext.getString(R.string.unknown_hardware_version)
                        )
                    }
                }
                if (barrier.decrementAndGet() == 0) {
                    callback()
                }
            }
        }

        /* Get the owner info */
        commandManager.getOwnerInfo { commandStatus: CommandManager.CommandStatus,
                                      response: String ->
            mainHandler.post {
                when (commandStatus) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        homeViewModel.setOwner(response)
                    }

                    else -> {
                        homeViewModel.setOwner(
                            applicationContext.getString(R.string.failed_to_retrieve_owner)
                        )
                    }
                }
                if (barrier.decrementAndGet() == 0) {
                    callback()
                }
            }
        }

        /* Get the contact info */
        commandManager.getContactInfo { commandStatus: CommandManager.CommandStatus,
                                        response: String ->
            mainHandler.post {
                when (commandStatus) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        homeViewModel.setContact(response)
                    }

                    else -> {
                        homeViewModel.setContact(
                            applicationContext.getString(R.string.failed_to_retrieve_contact)
                        )
                    }
                }
                if (barrier.decrementAndGet() == 0) {
                    callback()
                }
            }
        }

        runBlocking {
            launch {
                homeViewModel.setIdentifier(sharedData.getECBIdentifier(applicationContext))
            }
        }
    }

    fun retrieveCurrentImage(callback: () -> Unit) {
        commandManager.getCurrentImageName { commandStatus: CommandManager.CommandStatus,
                                             response: String ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                when (commandStatus) {
                    CommandManager.CommandStatus.SUCCESS -> {
                        if (response.isNotEmpty()) {
                            einkViewModel.setCurrentImageName(response)
                            if (!EInkImage.isImageDownloaded(applicationContext, response)) {
                                /* Now actually load the image */
                                if (einkViewModel.currentImageNameText.value?.isNotEmpty() == true) {
                                    commandManager.getImageData(response) { status: CommandManager.CommandStatus,
                                                                            responseData: ByteArray? ->
                                        mainHandler.post {
                                            when (status) {
                                                CommandManager.CommandStatus.SUCCESS -> {
                                                    val imagePair =
                                                        EInkImage.createFromECBData(
                                                            applicationContext,
                                                            responseData!!,
                                                            response
                                                        )
                                                    if (imagePair.second == EInkImage.ImageStatus.SUCCESS) {
                                                        einkViewModel.setCurrentImageData(
                                                            imagePair.first
                                                        )
                                                        einkViewModel.updateImageList()
                                                    } else {
                                                        einkViewModel.setCurrentImageData(null)
                                                    }
                                                }

                                                else -> {
                                                    einkViewModel.setCurrentImageData(null)
                                                }
                                            }
                                            callback()
                                        }
                                    }
                                }
                            } else {
                                val imagePair = EInkImage.loadFromName(applicationContext, response)
                                if (imagePair.second == EInkImage.ImageStatus.SUCCESS) {
                                    einkViewModel.setCurrentImageData(imagePair.first!!)
                                    einkViewModel.updateImageList()
                                } else {
                                    einkViewModel.setCurrentImageData(null)
                                }
                                callback()

                            }
                        } else {
                            einkViewModel.setCurrentImageName(applicationContext.getString(R.string.no_image_loaded))
                            einkViewModel.setCurrentImageData(null)
                            callback()
                        }
                    }

                    else -> {
                        einkViewModel.setCurrentImageName(
                            applicationContext.getString(R.string.failed_to_retrieve_current_image)
                        )
                        einkViewModel.setCurrentImageData(null)
                        callback()
                    }
                }
            }
        }
    }

    fun retrieveImage(imageFileName: String, callback: (CommandManager.CommandStatus) -> Unit?) {
        if (!EInkImage.isImageDownloaded(applicationContext, imageFileName)) {
            /* Now actually load the image */
            if (imageFileName.isNotEmpty()) {
                val mainHandler = Handler(Looper.getMainLooper())
                commandManager.getImageData(imageFileName) { status: CommandManager.CommandStatus,
                                                             responseData: ByteArray? ->
                    mainHandler.post {
                        when (status) {
                            CommandManager.CommandStatus.SUCCESS -> {
                                val imagePair =
                                    EInkImage.createFromECBData(
                                        applicationContext,
                                        responseData!!,
                                        imageFileName
                                    )
                                if (imagePair.second == EInkImage.ImageStatus.SUCCESS) {
                                    /* Update the current image if needed */
                                    if (einkViewModel.currentImageNameText.value.toString() == imageFileName) {
                                        einkViewModel.setCurrentImageData(
                                            imagePair.first
                                        )
                                    }
                                    /* Update the gridview data */
                                    einkViewModel.addImage(
                                        EInkImageGridModel(
                                            localStore = true,
                                            remoteStore = true,
                                            imageName = imageFileName,
                                            imageData = imagePair.first
                                        )
                                    )
                                    einkViewModel.updateImageList()
                                    callback(status)
                                } else {
                                    callback(CommandManager.CommandStatus.COMM_ERROR)
                                }
                            }

                            else -> {
                                callback(status)
                            }
                        }
                    }
                }
            }
        }
    }

    fun getHomeViewModel(): HomeViewModel {
        return homeViewModel
    }

    fun getEInkViewModel(): EInkViewModel {
        return einkViewModel
    }

    fun validateToken(tokenString: String): Boolean {
        /* Check the token size */
        if (tokenString.length != bleManager.getTokenSize()) {
            return false
        }

        /* Check for ASCII characters only */
        if (!tokenString.matches(Regex("\\A\\p{ASCII}*\\z"))) {
            return false
        }

        return true
    }

    fun validateOwnerContact(value: String): Boolean {
        /* Check the token size */
        if (value.length > MAX_OWNER_CONTACT_SIZE) {
            return false
        }

        /* Check for ASCII characters only */
        if (!value.matches(Regex("\\A\\p{ASCII}*\\z"))) {
            return false
        }

        return true
    }

    fun requestTokenChange(token: String, callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.requestTokenChange(token) { result: CommandManager.CommandStatus ->

            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                /* Set token on result */
                if (result == CommandManager.CommandStatus.SUCCESS) {
                    bleManager.setECBToken(token)
                }

                callback(result)
            }
        }
    }

    fun requestOwnerChange(owner: String, callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.requestOwnerChange(owner) { result: CommandManager.CommandStatus ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                /* Set the owner result */
                if (result == CommandManager.CommandStatus.SUCCESS) {
                    homeViewModel.setOwner(owner)
                }

                callback(result)
            }
        }
    }

    fun requestContactChange(contact: String, callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.requestContactChange(contact) { result: CommandManager.CommandStatus ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                /* Set the contact result */
                if (result == CommandManager.CommandStatus.SUCCESS) {
                    homeViewModel.setContact(contact)
                }

                callback(result)
            }
        }
    }

    fun requestUpdate(callback: (CommandManager.CommandStatus, String) -> Unit) {
        if (homeViewModel.ecbSoftwareVersionText.value != null &&
            homeViewModel.ecbHardwareVersionText.value != null
        ) {
            updater.checkForUpdate(
                homeViewModel.ecbSoftwareVersionText.value!!,
                homeViewModel.ecbHardwareVersionText.value!!
            ) { result: CommandManager.CommandStatus ->
                callback(result, updater.getLatestVersion())
            }
        } else {
            callback(CommandManager.CommandStatus.ACTION_FAILED, "")
        }
    }

    fun performUpdate(
        progressCallback: (CommandManager.CommandStatus, Float) -> Unit,
        updateEndCallback: (CommandManager.CommandStatus) -> Unit
    ) {
        updater.requestFirmwareUpdate(progressCallback, updateEndCallback)
    }

    fun clearEInkDisplay(callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.clearEInkDisplay { status: CommandManager.CommandStatus ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                if (status == CommandManager.CommandStatus.SUCCESS) {
                    /* Clear the preview */
                    einkViewModel.setCurrentImageName("")
                    einkViewModel.setCurrentImageData(null)
                    einkViewModel.updateImageList()
                }
                callback(status)
            }
        }
    }

    fun requestFactoryReset(callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.requestFactoryReset(callback)
    }

    fun retrieveImageList(callback: () -> Unit) {
        /* Load local images */
        val imageDataList = EInkImage.getStoredImages(applicationContext)

        commandManager.retrieveImageList { result: CommandManager.CommandStatus,
                                           imageList: List<String> ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                /* Set the contact result */
                if (result == CommandManager.CommandStatus.SUCCESS) {
                    einkViewModel.setLoadingImageListText("")
                    if (imageList.isNotEmpty()) {
                        /* Create the models */
                        for (imageName in imageList) {

                            var found = false
                            for (image in imageDataList) {
                                if (image.imageName == imageName) {
                                    image.remoteStore = true
                                    found = true
                                    break
                                }
                            }
                            if (!found) {
                                imageDataList.add(
                                    EInkImageGridModel(
                                        localStore = false,
                                        remoteStore = true,
                                        imageName = imageName,
                                        imageData = null
                                    )
                                )
                            }
                        }
                    }

                    einkViewModel.addImageList(imageDataList)
                } else {
                    einkViewModel.setLoadingImageListText(
                        applicationContext.getString(
                            R.string.error_with_info,
                            commandManager.getStringError(result)
                        )
                    )
                }

                callback()
            }
        }
    }

    fun setEInkImage(imageFileName: String, callback: (CommandManager.CommandStatus) -> Unit) {
        commandManager.setEInkImage(imageFileName) { status: CommandManager.CommandStatus ->
            val mainHandler = Handler(Looper.getMainLooper())

            mainHandler.post {
                if (status == CommandManager.CommandStatus.SUCCESS) {
                    /* Clear the preview */
                    einkViewModel.setCurrentImageName(imageFileName)

                    val eInkImage = EInkImage.loadFromName(applicationContext, imageFileName)
                    einkViewModel.setCurrentImageData(eInkImage.first)

                    einkViewModel.updateImageList()
                }
                callback(status)
            }
        }
    }

    fun removeImage(
        imageModel: EInkImageGridModel,
        callback: (CommandManager.CommandStatus) -> Unit?
    ) {

        val imageFileName = imageModel.imageName
        if (imageModel.remoteStore) {
            commandManager.removeEInkImage(
                imageFileName,
                einkViewModel.currentImageNameText.value.toString() == imageFileName
            ) { status: CommandManager.CommandStatus ->
                val mainHandler = Handler(Looper.getMainLooper())

                mainHandler.post {
                    if (status == CommandManager.CommandStatus.SUCCESS) {
                        /* If current image */
                        if (einkViewModel.currentImageNameText.value.toString() == imageFileName) {
                            /* Clear the preview */
                            einkViewModel.setCurrentImageName("")
                            einkViewModel.setCurrentImageData(null)
                        }

                        if (imageModel.localStore) {
                            /* Remove from EInkImages */
                            EInkImage.removeDownloadedImage(applicationContext, imageFileName)
                        }
                        /* Update grid */
                        einkViewModel.removeFromImageList(imageFileName)
                    }
                    callback(status)
                }
            }
        } else if (imageModel.localStore) {
            /* Remove from EInkImages */
            EInkImage.removeDownloadedImage(applicationContext, imageFileName)

            /* Update grid */
            einkViewModel.removeFromImageList(imageFileName)

            callback(CommandManager.CommandStatus.SUCCESS)
        }
    }

    fun sendEInkImage(
        eInkImage: EInkImage,
        callback: (CommandManager.CommandStatus) -> Unit
    ) {
        commandManager.sendEInkImage(
            eInkImage.getName(),
            eInkImage.getECBData()
        ) { status: CommandManager.CommandStatus ->
            val mainHandler = Handler(Looper.getMainLooper())
            var sent: Boolean
            mainHandler.post {
                if (status == CommandManager.CommandStatus.SUCCESS) {
                    sent = true

                    /* Update the preview */
                    einkViewModel.setCurrentImageName(eInkImage.getName())
                    einkViewModel.setCurrentImageData(eInkImage)
                } else {
                    sent = false
                }

                /* Add the image to the image list */
                einkViewModel.addImage(
                    EInkImageGridModel(
                        localStore = true,
                        remoteStore = sent,
                        imageName = eInkImage.getName(),
                        imageData = eInkImage
                    )
                )

                callback(status)
            }
        }
    }

    fun addEInkImage(
        image: Bitmap,
        imageName: String,
        forceOverwrite: Boolean,
        sendToBadge: Boolean,
        callback: (EInkImage.ImageStatus) -> Unit
    ) {

        /* Validate the name */
        if (imageName.isEmpty() || !imageName.matches(Regex("\\A\\p{ASCII}*\\z"))) {
            callback(EInkImage.ImageStatus.INVALID_NAME)
            return
        }

        /* Check if the name already exists */
        if (einkViewModel.imageExists(imageName) && !forceOverwrite) {
            callback(EInkImage.ImageStatus.IMAGE_ALREADY_EXISTS)
            return
        }

        /* Save the image */
        val imagePair = EInkImage.createFromBitmap(applicationContext, image, imageName)

        /* Check the creation */
        var currentStatus = imagePair.second
        if (currentStatus == EInkImage.ImageStatus.SUCCESS) {
            /* Send to badge */
            if (sendToBadge) {
                sendEInkImage(imagePair.first!!) {

                        status: CommandManager.CommandStatus ->

                    val mainHandler = Handler(Looper.getMainLooper())

                    mainHandler.post {
                        if (status != CommandManager.CommandStatus.SUCCESS) {
                            currentStatus = EInkImage.ImageStatus.TRANSMISSION_FAILED
                        }
                        callback(currentStatus)
                    }
                }
            } else {
                /* Add the image to the image list */
                einkViewModel.addImage(
                    EInkImageGridModel(
                        localStore = true,
                        remoteStore = false,
                        imageName = imageName,
                        imageData = imagePair.first
                    )
                )
                callback(currentStatus)
            }
        } else {
            callback(currentStatus)
        }
    }

    fun getStringError(result: CommandManager.CommandStatus): String {
        return commandManager.getStringError(result)
    }

    fun getCurrentImageName(): String {
        return einkViewModel.currentImageNameText.value.toString()
    }
}