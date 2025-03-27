package dev.olsontek.econbadge.data

import android.content.Context
import android.util.Log
import dev.olsontek.econbadge.ui.ledborder.LedBorderViewModel
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.ObjectInputStream
import java.io.ObjectOutputStream


private const val LED_COUNT = 120


/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Current module name */
private const val MODULE_NAME = "LedBorder"

/* Path to the stored animations */
private const val ECB_ANIMATIONS_PATH = "animations"

/* Path to the stored patterns */
private const val ECB_PATTERNS_PATH = "patterns"

/* Path to the stored scenes */
private const val ECB_SCENES_PATH = "scenes"

class LedBorder(private val context: Context, private val ledBorderViewModel: LedBorderViewModel) {

    private fun generateUniqueID(idSet: Set<Int>?) : Int {
        var value: Int
        do {
            value = (Int.MIN_VALUE ..Int.MAX_VALUE).random()
        } while ((idSet?.contains(value) == true) || value == -1)

        return value
    }

    private fun <T> saveData(path: String, objSave: T) {
        try {
            val file = File(context.filesDir, path)
            if (file.exists()) {
                file.delete()
            }
            if (!file.createNewFile()) {
                Log.e(MODULE_NAME, "Failed to create $path file.")
                return
            }

            val out = ObjectOutputStream(
                file.outputStream()
            )
            out.writeObject(objSave)
            out.close()
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    private fun loadAnimations() {
        try {
            val file = File(context.filesDir, ECB_ANIMATIONS_PATH)
            if (!file.exists()) {
                Log.d(MODULE_NAME, "No animations file.")
                ledBorderViewModel.addAnimations(HashMap())
                return
            }

            val inObj = ObjectInputStream(
                file.inputStream()
            )
            ledBorderViewModel.addAnimations(inObj.readObject() as HashMap<Int, LedBorderAnimation>)
            inObj.close()
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    private fun loadPatterns() {
        try {
            val file = File(context.filesDir, ECB_PATTERNS_PATH)
            if (!file.exists()) {
                Log.d(MODULE_NAME, "No patterns file.")
                ledBorderViewModel.addPatterns(HashMap())
                return
            }

            val inObj = ObjectInputStream(
                file.inputStream()
            )
            ledBorderViewModel.addPatterns(inObj.readObject() as HashMap<Int, LedBorderPattern>)
            inObj.close()
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    private fun loadScenes() {
        try {
            val file = File(context.filesDir, ECB_SCENES_PATH)
            if (!file.exists()) {
                Log.d(MODULE_NAME, "No scene file.")
                ledBorderViewModel.addScenes(HashMap())
                return
            }

            val inObj = ObjectInputStream(
                file.inputStream()
            )
            ledBorderViewModel.addScenes(inObj.readObject() as HashMap<Int, LedBorderScene>)
            inObj.close()
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    fun getLedCount() : Int {
        return LED_COUNT
    }

    fun loadData() {
        loadAnimations()
        loadPatterns()
        loadScenes()
    }

    fun addAnimation(animation: LedBorderAnimation) {
        /* Generate the unique animation ID */
        if (animation.uniqueId == -1) {
            animation.uniqueId = generateUniqueID(ledBorderViewModel.animationValues.value?.keys)
        }

        /* Add the animation */
        ledBorderViewModel.addAnimation(animation)

        /* Write the file */
        saveData(ECB_ANIMATIONS_PATH, ledBorderViewModel.animationValues.value)
    }

    fun removeAnimation(animationId: Int) {
        /* remove the animation */
        ledBorderViewModel.removeAnimation(animationId)

        /* Write the file */
        saveData(ECB_ANIMATIONS_PATH, ledBorderViewModel.animationValues.value)
    }

    fun getAnimation(animationId: Int): LedBorderAnimation? {
        return ledBorderViewModel.getAnimation(animationId)
    }

    fun addPattern(pattern: LedBorderPattern) {
        /* Generate the unique pattern ID */
        if (pattern.uniqueId == -1) {
            pattern.uniqueId = generateUniqueID(ledBorderViewModel.patternValues.value?.keys)
        }

        /* Add the pattern */
        ledBorderViewModel.addPattern(pattern)

        /* Write the file */
        saveData(ECB_PATTERNS_PATH, ledBorderViewModel.patternValues.value)
    }

    fun removePattern(patternId: Int) {
        /* remove the pattern */
        ledBorderViewModel.removePattern(patternId)

        /* Write the file */
        saveData(ECB_PATTERNS_PATH, ledBorderViewModel.patternValues.value)
    }

    fun getPattern(patternId: Int): LedBorderPattern? {
        return ledBorderViewModel.getPattern(patternId)
    }

    fun addScene(scene: LedBorderScene) {
        /* Generate the unique scene ID */
        if (scene.uniqueId == -1) {
            scene.uniqueId = generateUniqueID(ledBorderViewModel.sceneValues.value?.keys)
        }

        /* Add the sceneId */
        ledBorderViewModel.addScene(scene)

        /* Write the file */
        saveData(ECB_SCENES_PATH, ledBorderViewModel.sceneValues.value)
    }

    fun removeScene(sceneId: Int) {
        /* remove the scene */
        ledBorderViewModel.removeScene(sceneId)

        /* Write the file */
        saveData(ECB_PATTERNS_PATH, ledBorderViewModel.sceneValues.value)
    }

    fun getScene(sceneId: Int): LedBorderScene? {
        return ledBorderViewModel.getScene(sceneId)
    }
}