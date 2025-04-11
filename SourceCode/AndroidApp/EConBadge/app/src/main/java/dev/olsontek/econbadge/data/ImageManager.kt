package dev.olsontek.econbadge.data

import android.content.ContentResolver
import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.net.Uri
import dev.olsontek.econbadge.algorithm.DitherFilter
import dev.olsontek.econbadge.algorithm.PixelVector

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class ImageManager(private val contentResolver: ContentResolver) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/

    /* Filter used to dither the image */
    private val ditherFilter: DitherFilter


    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun openECBImage(uri: Uri): Bitmap? {
        val imageData = contentResolver.openInputStream(uri)?.readBytes()
        if (imageData != null) {
            /* Get the image data */
            val imageBitMap = EInkImage.getBitmapFromECBData(imageData)
            if (imageBitMap != null) {
                return imageBitMap
            }
        }

        return null
    }

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    init {
        val einkPalette = EInkImage.getEInkPalette()
        val colorPalette = Array(einkPalette.size) { i ->
            PixelVector(einkPalette[i])
        }
        ditherFilter = DitherFilter(colorPalette)
    }

    fun scaleImage(image: Bitmap): Bitmap {
        /* Scale the image */
        val newImage = Bitmap.createScaledBitmap(
            image,
            EInkImage.getImageWidth(),
            EInkImage.getImageHeight(),
            true
        )

        return newImage
    }

    fun openEInkImage(uri: Uri?): Pair<Bitmap?, EInkImage.ImageStatus> {
        if (uri == null) {
            return null to EInkImage.ImageStatus.IMAGE_DOES_NO_EXISTS
        }

        /* Open the file */
        var bitmap = openECBImage(uri)
        if (bitmap == null) {
            try {
                val source = ImageDecoder.createSource(contentResolver, uri)
                bitmap = ImageDecoder.decodeBitmap(source)
            } catch (exc: Exception) {
                bitmap = null
            }
        }

        if (bitmap == null) {
            return null to EInkImage.ImageStatus.INVALID_IMAGE
        }

        /* Check the size */
        val validWidth = EInkImage.getImageWidth()
        val validHeight = EInkImage.getImageHeight()

        if (validWidth > bitmap.width || validHeight > bitmap.height) {
            return null to EInkImage.ImageStatus.INVALID_IMAGE_SIZE
        }

        return bitmap to EInkImage.ImageStatus.SUCCESS
    }

    fun isEInkImageSizeReady(bitmap: Bitmap): Boolean {
        return bitmap.width == EInkImage.getImageWidth() &&
                bitmap.height == EInkImage.getImageHeight()
    }

    fun isEInkImageDithered(bitmap: Bitmap): Boolean {
        val eInkPalette = EInkImage.getEInkPalette()

        /* Walk all pixels and check if they are in the EInk palette */
        for (x in 0..<bitmap.width) {
            for (y in 0..<bitmap.height) {
                if (!eInkPalette.contains(bitmap.getPixel(x, y))) {
                    return false
                }
            }
        }

        return true
    }

    fun validateName(imageName: String): Boolean {
        return imageName.isNotEmpty() && imageName.matches(Regex("\\A\\p{ASCII}*\\z"))
    }

    fun ditherImage(diffusion: Int, bitmap: Bitmap): Bitmap {
        return ditherFilter.applyFloydSteinberg(bitmap, diffusion)
    }
}