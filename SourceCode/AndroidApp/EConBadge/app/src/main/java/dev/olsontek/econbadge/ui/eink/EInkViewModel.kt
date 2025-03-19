package dev.olsontek.econbadge.ui.eink

import android.graphics.Bitmap
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import dev.olsontek.econbadge.data.EInkImage

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class EInkViewModel : ViewModel() {
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
    /* Current image name */
    private val currentImageName = MutableLiveData<String>()
    val currentImageNameText: LiveData<String> = currentImageName

    /* Current EInk image */
    private val currentImageData = MutableLiveData<EInkImage?>()
    val currentImageDataValue: LiveData<EInkImage?> = currentImageData

    /* Loading image text */
    private val loadingImage = MutableLiveData<String>()
    val loadingImageText: LiveData<String> = loadingImage

    /* List of image models */
    private var imageListInternal = HashSet<EInkImageGridModel>()
    private val imageList = MutableLiveData<HashSet<EInkImageGridModel>>()
    val imageListData: LiveData<HashSet<EInkImageGridModel>> = imageList

    /* Imported image base bitmap */
    private val importImageBitmapBase = MutableLiveData<Bitmap?>()
    val importImageBitmapBaseData: LiveData<Bitmap?> = importImageBitmapBase

    /* Imported image cropped bitmap */
    private val importImageBitmapCrop = MutableLiveData<Bitmap?>()
    val importImageBitmapCropData: LiveData<Bitmap?> = importImageBitmapCrop

    /* Imported image final dithered bitmap */
    private val importImageBitmapDither = MutableLiveData<Bitmap?>()
    val importImageBitmapDitherData: LiveData<Bitmap?> = importImageBitmapDither

    /* Imported image temporary dithered bitmap */
    private val importImageBitmapDitherTmp = MutableLiveData<Bitmap?>()
    val importImageBitmapDitherTmpData: LiveData<Bitmap?> = importImageBitmapDitherTmp

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PUBLIC METHODS
     **********************************************************************************************/
    fun setCurrentImageName(value: String) {
        currentImageName.value = value
    }

    fun setCurrentImageData(value: EInkImage?) {
        currentImageData.value = value
    }

    fun setLoadingImageListText(value: String) {
        loadingImage.value = value
    }

    fun addImageList(value: HashSet<EInkImageGridModel>) {
        imageListInternal = value
        updateImageList()
    }

    fun addImage(eInkImageGridModel: EInkImageGridModel) {
        imageListInternal.remove(eInkImageGridModel)
        imageListInternal.add(eInkImageGridModel)
        updateImageList()
    }

    fun updateImageList() {
        imageList.value = HashSet()
        imageList.value = imageListInternal
    }

    fun removeFromImageList(imageFileName: String) {
        imageListInternal.removeIf {
            it.imageName == imageFileName
        }
        updateImageList()
    }

    fun imageExists(imageName: String): Boolean {
        /* Image models are compared by their name, just create a dummy model and compare */
        return imageListInternal.contains(
            EInkImageGridModel(
                localStore = false, remoteStore = false, imageName = imageName, imageData = null
            )
        )
    }

    fun setImportImageBitmapBaseData(bitmap: Bitmap?) {
        importImageBitmapBase.value = bitmap
    }

    fun setImportImageBitmapCropData(bitmap: Bitmap?) {
        importImageBitmapCrop.value = bitmap
    }

    fun setImportImageBitmapDitherData(bitmap: Bitmap?) {
        importImageBitmapDither.value = bitmap
    }

    fun setImportImageBitmapTmpDitherData(bitmap: Bitmap?) {
        importImageBitmapDitherTmp.value = bitmap
    }
}