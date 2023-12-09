package dev.olsontek.econbadge.adapters;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.models.AnimationModel;
import dev.olsontek.econbadge.ui.ledborder.LedBorderViewModel;

public class ImageListAdapter extends RecyclerView.Adapter<ImageListAdapter.ImageHolder>{
    private Context context_;
    private ArrayList<String> images_;
    private ImageListAdapter.Updater updater_;

    public ImageListAdapter(Context context,
                            ArrayList<String> imageList,
                            ImageListAdapter.Updater updater) {
        context_ = context;
        images_  = imageList;
        updater_ = updater;
    }
    @NonNull
    @Override
    public ImageHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater layoutInflater = LayoutInflater.from(parent.getContext());

        View view = layoutInflater.inflate(R.layout.image_list_item, parent, false);
        return new ImageHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ImageHolder holder, int position) {
        final String imageName = images_.get(position);

        holder.setImageName(imageName);
        holder.setPosition(position);

        /* Update on last image show */
        if(position == images_.size() - 1) {
            updater_.updateList(false);
        }
    }

    @Override
    public int getItemCount() {
        return images_.size();
    }

    public class ImageHolder extends RecyclerView.ViewHolder {

        private TextView textViewImageName_;
        private Button   buttonRemove_;
        private Button   buttonViewImage_;
        private Button   buttonSelectImage_;
        private int      position_;

        public ImageHolder(View itemView) {
            super(itemView);

            textViewImageName_ = itemView.findViewById(R.id.textViewImageNameImageListItem);
            buttonRemove_      = itemView.findViewById(R.id.buttonRemoveImageListItem);
            buttonViewImage_   = itemView.findViewById(R.id.buttonViewImageListItem);
            buttonSelectImage_ = itemView.findViewById(R.id.buttonSelectImageListItem);

            buttonRemove_.setOnClickListener(view -> updater_.removeImage(position_, textViewImageName_.getText().toString()));
            buttonViewImage_.setOnClickListener(view -> updater_.viewImage(textViewImageName_.getText().toString()));
            buttonSelectImage_.setOnClickListener(view -> updater_.selectUpdate(textViewImageName_.getText().toString()));
        }

        public void setImageName(String imageName) {
            textViewImageName_.setText(imageName);
        }

        public void setPosition(int position) {
            position_ = position;
        }
    }

    public interface Updater {
        void updateList(boolean force);
        void removeImage(int position, String imageName);
        void viewImage(String imageName);
        void selectUpdate(String imageName);
    }
}
