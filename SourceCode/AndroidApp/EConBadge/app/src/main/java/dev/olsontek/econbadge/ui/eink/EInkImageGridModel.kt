package dev.olsontek.econbadge.ui.eink

import dev.olsontek.econbadge.data.EInkImage

class EInkImageGridModel (
    var localStore: Boolean,
    var remoteStore: Boolean,
    var imageName: String,
    var imageData: EInkImage?
) {
    override fun equals(other: Any?): Boolean {
        if (other !is EInkImageGridModel) {
            return false
        }

        return imageName == other.imageName
    }

    override fun hashCode(): Int {
        return imageName.hashCode()
    }
}