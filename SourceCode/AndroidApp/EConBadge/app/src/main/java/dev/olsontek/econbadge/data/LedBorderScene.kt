package dev.olsontek.econbadge.data

import java.io.Serializable


class LedBorderScene : Serializable {
    var uniqueId: Int = -1
    var name: String = ""
    var animations = HashSet<Int>()
    var patterns = HashSet<Int>()


    override fun equals(other: Any?): Boolean {
        if (other !is LedBorderScene) {
            return false
        }

        return uniqueId == other.uniqueId
    }

    override fun hashCode(): Int {
        return uniqueId
    }
}