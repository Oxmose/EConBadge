package dev.olsontek.econbadge.ui.ledborder

class SceneListModel (
    var sceneId: Int,
    var sceneName: String,
    var sceneDescription: String
) {
    override fun equals(other: Any?): Boolean {
        if (other !is SceneListModel) {
            return false
        }

        return sceneId == other.sceneId
    }

    override fun hashCode(): Int {
        return sceneId
    }
}