package dev.olsontek.econbadge.algorithm

import android.graphics.Bitmap

/***************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/
/* None */

/***************************************************************************************************
 * MAIN CLASS
 **************************************************************************************************/
class DitherFilter(private var palette: Array<PixelVector>) {

    /***********************************************************************************************
     * PUBLIC TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE TYPES AND ENUMERATIONS
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE ATTRIBUTES
     **********************************************************************************************/
    /* None */

    /***********************************************************************************************
     * PRIVATE METHODS
     **********************************************************************************************/
    private fun getClosestMatch(color: PixelVector): PixelVector {
        var minIdx = 0
        var minDiff = palette[0].fastDifferenceTo(color)

        for (i in 1 until palette.size) {
            val currDiff: Int = palette[i].fastDifferenceTo(color)

            if (currDiff < minDiff) {
                minDiff = currDiff
                minIdx = i
            }
        }

        return palette[minIdx]
    }

    /***************************************************************************************************
     * PUBLIC METHODS
     **************************************************************************************************/
    fun applyFloydSteinberg(bitmap: Bitmap, diffusion: Int): Bitmap {
        val newBitmap = bitmap.copy(Bitmap.Config.ARGB_8888, true)

        val diffusionLow = ((100 - diffusion) / 2)
        val diffusionHigh = ((100 - diffusion) / 2 + diffusion)

        for (y in 0..<newBitmap.height) {
            for (x in 0..<newBitmap.width) {
                val currColor = PixelVector(newBitmap.getPixel(x, y))
                val match = getClosestMatch(currColor)
                val error = currColor.subtract(match)

                val difference: Int = error.fastDifferenceTo(currColor) * 100 / (255 * 3)

                newBitmap.setPixel(x, y, match.toRGB())

                /* Manage error diffusion */
                if (difference < diffusionLow || difference > diffusionHigh) {
                    continue
                }

                /* Apply Floyd-Steinberg
                 *     X   7
                 * 3   5   1
                 *
                 * (1/16)
                 */
                if (x != newBitmap.width - 1) {
                    newBitmap.setPixel(
                        x + 1,
                        y,
                        PixelVector(
                            newBitmap.getPixel(
                                x + 1,
                                y
                            )
                        ).add(error.scalarMultiply(7.0f / 16.0f)).clip(0, 255).toRGB()
                    )

                    if (y != newBitmap.height - 1) {
                        newBitmap.setPixel(
                            x + 1,
                            y + 1,
                            PixelVector(newBitmap.getPixel(x + 1, y + 1)).add(
                                error.scalarMultiply(1.0f / 16.0f)
                            ).clip(0, 255).toRGB()
                        )
                    }
                }

                if (y != newBitmap.height - 1) {
                    newBitmap.setPixel(
                        x,
                        y + 1,
                        PixelVector(
                            newBitmap.getPixel(
                                x,
                                y + 1
                            )
                        ).add(error.scalarMultiply(5.0f / 16.0f)).clip(0, 255).toRGB()
                    )

                    if (x != 0) {
                        newBitmap.setPixel(
                            x - 1,
                            y + 1,
                            PixelVector(newBitmap.getPixel(x - 1, y + 1)).add(
                                error.scalarMultiply(3.0f / 16.0f)
                            ).clip(0, 255).toRGB()
                        )
                    }
                }
            }
        }

        return newBitmap
    }
}