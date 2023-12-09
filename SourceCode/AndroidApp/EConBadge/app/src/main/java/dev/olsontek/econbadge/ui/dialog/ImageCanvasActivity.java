package dev.olsontek.econbadge.ui.dialog;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.SeekBar;

import dev.olsontek.econbadge.EInkImage;
import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.ui.imgupdate.SimpleDrawingView;

public class ImageCanvasActivity extends Dialog {
    private EInkImage image_;
    private SimpleDrawingView canvas_;
    private Button btnExit_;

    public ImageCanvasActivity(@NonNull Context context, EInkImage image) {
        super(context);
        image_ = image;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_image_canvas);

        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);

        canvas_ = findViewById(R.id.canvasImageCanvasActivity);
        canvas_.setImage(image_);

        btnExit_ = findViewById(R.id.buttonExitImageCanvasActivity);
        btnExit_.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                dismiss();
            }
        });
    }
}