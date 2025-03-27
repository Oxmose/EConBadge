package dev.olsontek.econbadge.data

import java.io.Serializable


class LedBorderAnimation : Serializable {
    enum class AnimationType {
        TRAIL,
        BREATH
    }

    var uniqueId: Int = -1
    var name: String = ""
    var type: AnimationType = AnimationType.TRAIL
    var startLedIdx: Int = 0
    var endLedIdx: Int = 0
    var direction: Int = 0
    var speed: Int = 0


    override fun equals(other: Any?): Boolean {
        if (other !is LedBorderAnimation) {
            return false
        }

        return uniqueId == other.uniqueId
    }

    override fun hashCode(): Int {
        return uniqueId
    }
}