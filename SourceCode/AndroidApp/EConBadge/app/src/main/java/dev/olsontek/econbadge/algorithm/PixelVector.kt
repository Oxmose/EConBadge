package dev.olsontek.econbadge.algorithm

import android.graphics.Color
import kotlin.math.abs

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class PixelVector(private var r: Int, private var g: Int, private var b: Int) {
/***************************************************************************************************
 * PUBLIC TYPES AND ENUMERATIONS
 **************************************************************************************************/
    companion object {
        private fun red(argb: Int) : Int {
            return (argb shr 16) and 0xFF
        }

        private fun green(argb: Int) : Int {
            return (argb shr 8) and 0xFF
        }

        private fun blue(argb: Int) : Int {
            return argb and 0xFF
        }
    }

/***************************************************************************************************
 * PRIVATE TYPES AND ENUMERATIONS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * PRIVATE ATTRIBUTES
 **************************************************************************************************/
    /* Stores the ARGB values for the pixel */
    private var argbVal: Int = Color.argb(0xff, r, g, b)

/***************************************************************************************************
 * PRIVATE METHODS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * PUBLIC METHODS
 **************************************************************************************************/
    constructor(argb: Int) : this(red(argb), green(argb), blue(argb))

    fun toRGB() : Int {
        return argbVal
    }

    fun subtract(other: PixelVector) : PixelVector {
        return PixelVector(r - other.r, g - other.g, b - other.b)
    }

    fun add(other: PixelVector): PixelVector {
        return PixelVector(r + other.r, g + other.g, b + other.b)
    }

    fun fastDifferenceTo(other: PixelVector): Int {
        val difference = subtract(other)
        return abs(difference.r) + abs(difference.g) + abs(difference.b)
    }

    fun scalarMultiply(scalar: Float): PixelVector {
        return PixelVector(
            (r * scalar).toInt(),
            (g * scalar).toInt(),
            (b * scalar).toInt()
        )
    }

    fun clip(minimum: Int, maximum: Int): PixelVector {
        val clipped = PixelVector(r, g, b)
        if (clipped.r > maximum) {
            clipped.r = maximum
        } else if (clipped.r < minimum) {
            clipped.r = minimum
        }

        if (clipped.g > maximum) {
            clipped.g = maximum
        } else if (clipped.g < minimum) {
            clipped.g = minimum
        }

        if (clipped.b > maximum) {
            clipped.b = maximum
        } else if (clipped.b < minimum) {
            clipped.b = minimum
        }

        return clipped
    }
}