package dev.olsontek.econbadge.ui.ledborder

class PatternListModel (
    var patternId: Int,
    var patternName: String,
    var patternDescription: String
) {
    override fun equals(other: Any?): Boolean {
        if (other !is PatternListModel) {
            return false
        }

        return patternId == other.patternId
    }

    override fun hashCode(): Int {
        return patternId
    }
}