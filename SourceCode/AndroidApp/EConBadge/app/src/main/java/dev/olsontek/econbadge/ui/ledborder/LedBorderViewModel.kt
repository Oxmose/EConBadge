package dev.olsontek.econbadge.ui.ledborder

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import dev.olsontek.econbadge.data.LedBorderAnimation
import dev.olsontek.econbadge.data.LedBorderPattern
import dev.olsontek.econbadge.data.LedBorderScene

class LedBorderViewModel : ViewModel() {
    private val animationInternal = HashMap<Int, LedBorderAnimation>()
    private val animations = MutableLiveData<HashMap<Int, LedBorderAnimation>>()
    val animationValues: LiveData<HashMap<Int, LedBorderAnimation>> = animations

    private val patternInternal = HashMap<Int, LedBorderPattern>()
    private val patterns = MutableLiveData<HashMap<Int, LedBorderPattern>>()
    val patternValues: LiveData<HashMap<Int, LedBorderPattern>> = patterns

    private val sceneInternal = HashMap<Int, LedBorderScene>()
    private val scenes = MutableLiveData<HashMap<Int, LedBorderScene>>()
    val sceneValues: LiveData<HashMap<Int, LedBorderScene>> = scenes

    private val currentScene = MutableLiveData<LedBorderScene>()
    val currentSceneValue: LiveData<LedBorderScene> = currentScene

    fun addAnimation(animation: LedBorderAnimation) {
        animationInternal[animation.uniqueId] = animation
        animations.value = HashMap()
        animations.value = animationInternal
    }

    fun addAnimations(animationsTable: HashMap<Int, LedBorderAnimation>) {
        animationInternal.putAll(animationsTable)
        animations.value = HashMap()
        animations.value = animationInternal
    }

    fun removeAnimation(animationId: Int) {
        animationInternal.remove(animationId)
        animations.value = HashMap()
        animations.value = animationInternal
    }

    fun getAnimation(animationId: Int) : LedBorderAnimation? {
        return animationInternal[animationId]
    }

    fun addPattern(pattern: LedBorderPattern) {
        patternInternal[pattern.uniqueId] = pattern
        patterns.value = HashMap()
        patterns.value = patternInternal
    }

    fun addPatterns(patternsTable: HashMap<Int, LedBorderPattern>) {
        patternInternal.putAll(patternsTable)
        patterns.value = HashMap()
        patterns.value = patternInternal
    }

    fun removePattern(patternId: Int) {
        patternInternal.remove(patternId)
        patterns.value = HashMap()
        patterns.value = patternInternal
    }

    fun getPattern(patternId: Int) : LedBorderPattern? {
        return patternInternal[patternId]
    }

    fun addScene(scene: LedBorderScene) {
        sceneInternal[scene.uniqueId] = scene
        scenes.value = HashMap()
        scenes.value = sceneInternal
    }

    fun addScenes(sceneTable: HashMap<Int, LedBorderScene>) {
        sceneInternal.putAll(sceneTable)
        scenes.value = HashMap()
        scenes.value = sceneInternal
    }

    fun removeScene(sceneId: Int) {
        sceneInternal.remove(sceneId)
        scenes.value = HashMap()
        scenes.value = sceneInternal
    }

    fun getScene(sceneId: Int) : LedBorderScene? {
        return sceneInternal[sceneId]
    }
}