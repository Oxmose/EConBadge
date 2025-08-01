package dev.olsontek.econbadge.ui.ledborder

class SceneItemListModel (
    var itemId: Int,
    var itemName: String,
    var itemDescription: String,
    var isAnimation: Boolean,
    var isUsed: Boolean,
    var startColorCode: Int,
    var endColorCode: Int,
    var isGradient: Boolean,
    var isCompatible: Boolean,
    var startLedIdx: Int,
    var endLedIdx: Int
) {
    override fun equals(other: Any?): Boolean {
        if (other !is SceneItemListModel) {
            return false
        }

        return itemId == other.itemId && isAnimation == other.isAnimation
    }

    override fun hashCode(): Int {
        return (itemId.hashCode() + isAnimation.hashCode()).hashCode()
    }
}