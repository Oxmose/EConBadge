package dev.olsontek.econbadge.ui.imgupdate;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.nio.IntBuffer;
import java.util.List;

import dev.olsontek.econbadge.EInkImage;

public class SimpleDrawingView extends View {
    private Paint drawPaint_;

    private EInkImage image_;
    private int width_;
    private int height_;

    // Setup paint with color and stroke styles
    private void setupPaint() {
        drawPaint_ = new Paint();
        drawPaint_.setAntiAlias(true);
        drawPaint_.setStrokeWidth(1);
        drawPaint_.setStyle(Paint.Style.STROKE);
        drawPaint_.setStrokeJoin(Paint.Join.ROUND);
        drawPaint_.setStrokeCap(Paint.Cap.ROUND);
    }

    public SimpleDrawingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setupPaint();
    }

    public void setImage(EInkImage image) {
        image_ = image;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        width_ = w;
        height_ = h;
        super.onSizeChanged(w, h, oldw, oldh);
    }
    protected void onDraw(Canvas canvas) {
        int width;
        int height;
        Matrix matrix;
        int[] pixelArray;
        List<Integer> pixels;
        Bitmap bmp;

        /* Get the original image */
        pixels = image_.getRawPixels();
        pixelArray = pixels.stream().mapToInt(m->m).toArray();
        bmp = Bitmap.createBitmap(600, 448, Bitmap.Config.ARGB_8888);
        bmp.copyPixelsFromBuffer(IntBuffer.wrap(pixelArray));

        /* Compute how much we should resize it */
        width = width_;
        height = (int)(448.0 * (float)width_ / 600.0);

        bmp = Bitmap.createScaledBitmap(bmp, width, height, true);

        /* Rotate */
        matrix = new Matrix();
        matrix.postRotate(90);
        bmp = Bitmap.createBitmap(bmp, 0, 0, width, height, matrix, false);

        canvas.drawBitmap(bmp, 16, 16, drawPaint_);
    }
}
