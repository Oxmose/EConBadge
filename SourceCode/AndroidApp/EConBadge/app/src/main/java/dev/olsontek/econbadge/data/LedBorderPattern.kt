package dev.olsontek.econbadge.data

import java.io.Serializable


class LedBorderPattern : Serializable {
    enum class PatternType {
        PLAIN,
        GRADIENT
    }

    var uniqueId: Int = -1
    var name: String = ""
    var type: PatternType = PatternType.PLAIN
    var startLedIdx: Int = 0
    var endLedIdx: Int = 0
    var startColorCode: Int = 0
    var endColorCode: Int = 0


    override fun equals(other: Any?): Boolean {
        if (other !is LedBorderPattern) {
            return false
        }

        return uniqueId == other.uniqueId
    }

    override fun hashCode(): Int {
        return uniqueId
    }
}