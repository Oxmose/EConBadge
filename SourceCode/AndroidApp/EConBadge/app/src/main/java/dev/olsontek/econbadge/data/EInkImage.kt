package dev.olsontek.econbadge.data

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Matrix
import dev.olsontek.econbadge.ui.eink.EInkImageGridModel
import java.io.File
import java.nio.ByteBuffer


/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Path to the stored images */
private const val ECB_IMAGES_PATH = "images/"

/* Image ECB data in bytes */
private const val ECB_DATA_SIZE = 134400

/* Image bitmap data in bytes */
private const val BITMAP_DATA_SIZE = 1075200

/* Image bitmap width in pixels */
private const val BITMAP_WIDTH = 600

/* Image bitmap height in pixels */
private const val BITMAP_HEIGHT = 448

/* ECB image color palette */
private val ECB_PALETTE = intArrayOf(
    0xFF000000.toInt(),
    0xFFFFFFFF.toInt(),
    0xFF4b6e54.toInt(),
    0xFF37436a.toInt(),
    0xFFa4504b.toInt(),
    0xFFdccc5f.toInt(),
    0xFFc06650.toInt(),
)

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class EInkImage private constructor(
    private val imageBitmap: Bitmap,
    private val imageECBData: ByteArray,
    private val imageName: String
) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* Image manager errors */
    enum class ImageStatus {
        SUCCESS,
        IMAGE_DOES_NO_EXISTS,
        IMAGE_IS_CORRUPTED,
        IMAGE_ALREADY_EXISTS,
        INVALID_NAME,
        TRANSMISSION_FAILED,
        INVALID_IMAGE,
        INVALID_IMAGE_SIZE,
    }

    companion object {
        fun getECBDataSize() : Int {
            return ECB_DATA_SIZE
        }

        fun isImageDownloaded(context: Context, imageName: String) : Boolean {
            val file = File(context.filesDir, ECB_IMAGES_PATH + imageName)
            return file.exists()
        }

        fun removeDownloadedImage(context: Context, imageName: String) {
            val file = File(context.filesDir, ECB_IMAGES_PATH + imageName)
            if (file.exists()) {
                file.delete()
            }
        }

        private fun createFromFileData(data: ByteArray, imageName: String) : Pair<EInkImage?, ImageStatus> {
            /* Check if file size is correct */
            if (data.size != ECB_DATA_SIZE) {
                return null to ImageStatus.IMAGE_IS_CORRUPTED
            }

            /* Create the bitmap */
            var x = 0
            var y = 0
            var loadedBitmap = Bitmap.createBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, Bitmap.Config.ARGB_8888)
            for (dataByte in data) {
                /* Four LSB are the first pixel */
                val firstPixel = (dataByte.toInt() shr 4)
                if (firstPixel >= ECB_PALETTE.size) {
                    return null to ImageStatus.IMAGE_IS_CORRUPTED
                }

                /* Four MSB are the second pixel */
                val secondPixel = (dataByte.toInt() and 0xF)
                if (secondPixel >= ECB_PALETTE.size) {
                    return null to ImageStatus.IMAGE_IS_CORRUPTED
                }

                /* Add the pixels */
                loadedBitmap.setPixel(x, y, ECB_PALETTE[firstPixel])

                if (x == BITMAP_WIDTH - 1) {
                    ++y
                    x = 0
                }
                else {
                    ++x
                }

                loadedBitmap.setPixel(x, y, ECB_PALETTE[secondPixel])
                if (x == BITMAP_WIDTH - 1) {
                    ++y
                    x = 0
                }
                else {
                    ++x
                }
            }

            val matrix = Matrix()
            matrix.postRotate(90.0f)

            loadedBitmap = Bitmap.createBitmap(
                loadedBitmap,
                0,
                0,
                loadedBitmap.width,
                loadedBitmap.height,
                matrix,
                true
            )
            return EInkImage(loadedBitmap, data, imageName) to ImageStatus.SUCCESS
        }

        fun loadFromName(context: Context, imageName: String) : Pair<EInkImage?, ImageStatus> {
            /* Check if file exists */
            val file = File(context.filesDir, ECB_IMAGES_PATH + imageName)

            if (!file.exists()) {
                return null to ImageStatus.IMAGE_DOES_NO_EXISTS
            }

            val data = file.inputStream().readBytes()

            return createFromFileData(data, imageName)
        }

        fun createFromBitmap(context: Context, imageBitmap: Bitmap, imageName: String) : Pair<EInkImage?, ImageStatus> {
            val size = imageBitmap.rowBytes * imageBitmap.height

            if(size != BITMAP_DATA_SIZE) {
                return null to ImageStatus.IMAGE_IS_CORRUPTED
            }

            /* Read the bytes into a buffer */
            val matrix = Matrix()
            matrix.postRotate(-90.0f)

            val ecbDataBuffer = ByteBuffer.allocate(ECB_DATA_SIZE)
            var pixelPair = false
            var colorByte = 0
            for (x in 0..< imageBitmap.width) {
                for (y in 0..< imageBitmap.height) {
                    val index = ECB_PALETTE.indexOf(imageBitmap.getPixel(x, y))
                    if (index == -1 || index >= ECB_PALETTE.size) {
                        return null to ImageStatus.IMAGE_IS_CORRUPTED
                    }
                    if (!pixelPair) {
                        colorByte = colorByte or index
                        ecbDataBuffer.put(colorByte.toByte())
                        pixelPair = true
                    }
                    else {
                        colorByte = (index shl 4)
                        pixelPair = false
                    }
                }
            }

            /* Save the image */
            val directory = File(context.filesDir, ECB_IMAGES_PATH)
            if (!directory.exists()) {
                directory.mkdirs()
            }
            val file = File(context.filesDir, ECB_IMAGES_PATH + imageName)
            if (!file.exists()) {
                if (file.createNewFile()) {
                    file.writeBytes(ecbDataBuffer.array())
                    return EInkImage(imageBitmap, ecbDataBuffer.array(), imageName) to ImageStatus.SUCCESS
                }
            }
            else {
                file.writeBytes(ecbDataBuffer.array())
                return EInkImage(imageBitmap, ecbDataBuffer.array(), imageName) to ImageStatus.SUCCESS
            }

            return null to ImageStatus.IMAGE_DOES_NO_EXISTS
        }

        fun createFromECBData(context: Context, data: ByteArray, imageName: String) : Pair<EInkImage?, ImageStatus> {

            val bitmap = getBitmapFromECBData(data) ?: return null to ImageStatus.IMAGE_IS_CORRUPTED

            /* Save the image */
            val directory = File(context.filesDir, ECB_IMAGES_PATH)
            if (!directory.exists()) {
                directory.mkdirs()
            }
            val file = File(context.filesDir, ECB_IMAGES_PATH + imageName)
            if (!file.exists()) {
                if (file.createNewFile()) {
                    file.writeBytes(data)
                    return EInkImage(bitmap, data, imageName) to ImageStatus.SUCCESS
                }
            }
            else {
                file.writeBytes(data)
                return EInkImage(bitmap, data, imageName) to ImageStatus.SUCCESS
            }

            return null to ImageStatus.IMAGE_DOES_NO_EXISTS
        }

        fun getStoredImages(context: Context): HashSet<EInkImageGridModel> {
            val imageSet = HashSet<EInkImageGridModel>()

            /* Get all files */
            val file = File(context.filesDir, ECB_IMAGES_PATH)
            if (!file.isDirectory) {
                return imageSet
            }
            val listFiles = file.listFiles() ?: return imageSet

            for (imageFile in listFiles) {
                val image = createFromFileData(imageFile.inputStream().readBytes(), imageFile.name)
                imageSet.add(
                    EInkImageGridModel(
                        localStore = true,
                        remoteStore = false,
                        imageName = imageFile.name,
                        imageData = image.first
                    )
                )
            }

            return imageSet
        }

        fun getBitmapFromECBData(data: ByteArray): Bitmap? {
            /* Check if file size is correct */
            if (data.size != ECB_DATA_SIZE) {
                return null
            }

            /* Create the bitmap */
            var x = 0
            var y = 0
            var loadedBitmap = Bitmap.createBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, Bitmap.Config.ARGB_8888)
            for (dataByte in data) {
                /* Four LSB are the first pixel */
                val firstPixel = (dataByte.toInt() shr 4)
                if (firstPixel >= ECB_PALETTE.size) {
                    return null
                }

                /* Four MSB are the second pixel */
                val secondPixel = (dataByte.toInt() and 0xF)
                if (secondPixel >= ECB_PALETTE.size) {
                    return null
                }

                /* Add the pixels */
                loadedBitmap.setPixel(x, y, ECB_PALETTE[firstPixel])

                if (x == BITMAP_WIDTH - 1) {
                    ++y
                    x = 0
                }
                else {
                    ++x
                }

                loadedBitmap.setPixel(x, y, ECB_PALETTE[secondPixel])
                if (x == BITMAP_WIDTH - 1) {
                    ++y
                    x = 0
                }
                else {
                    ++x
                }
            }

            val matrix = Matrix()
            matrix.postRotate(90.0f)

            loadedBitmap = Bitmap.createBitmap(
                loadedBitmap,
                0,
                0,
                loadedBitmap.width,
                loadedBitmap.height,
                matrix,
                true
            )

            return loadedBitmap
        }

        fun getImageWidth(): Int {
            /* Intentionally return height as we rotate the images by 90 deg */
            return BITMAP_HEIGHT
        }

        fun getImageHeight(): Int {
            /* Intentionally return width as we rotate the images by 90 deg */
            return BITMAP_WIDTH
        }

        fun getEInkPalette() : IntArray {
            return ECB_PALETTE
        }
    }

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    fun getBitmap() : Bitmap {
        return imageBitmap
    }

    fun getECBData() : ByteArray {
        return imageECBData
    }

    fun getName() : String {
        return imageName
    }
}