package dev.olsontek.econbadge.ui.ledborder

class AnimationListModel (
    var animId: Int,
    var animName: String,
    var animDescription: String
) {
    override fun equals(other: Any?): Boolean {
        if (other !is AnimationListModel) {
            return false
        }

        return animId == other.animId
    }

    override fun hashCode(): Int {
        return animId
    }
}