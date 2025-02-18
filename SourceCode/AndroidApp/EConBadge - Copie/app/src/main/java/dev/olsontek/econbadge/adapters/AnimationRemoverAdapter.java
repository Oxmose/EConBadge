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

import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.models.AnimationModel;
import dev.olsontek.econbadge.ui.ledborder.LedBorderViewModel;

public class AnimationRemoverAdapter extends RecyclerView.Adapter<AnimationRemoverAdapter.AnimationHolder>{
    private Context context_;

    LedBorderViewModel viewModel_;

    public AnimationRemoverAdapter(Context context,
                                   LedBorderViewModel viewModel) {
        context_   = context;
        viewModel_ = viewModel;
    }
    @NonNull
    @Override
    public AnimationHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater layoutInflater = LayoutInflater.from(parent.getContext());

        View view = layoutInflater.inflate(R.layout.animation_list_item, parent, false);
        return new AnimationHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull AnimationHolder holder, int position) {
        final AnimationModel model = viewModel_.GetAnimations().get(position);

        // Set the data to the views here
        if(model.getType() == 0) {
            holder.setAnimType("Slider Animation");
            holder.setAnimParam("Speed: " + model.getParam());
        }
        else {
            holder.setAnimType("Breath Animation");
            holder.setAnimParam("Speed: " + model.getParam());
        }

        Log.d("REM", "Binder " + position);

        holder.setRemoveAction(view -> {
            ProgressDialog waitDataDialog = new ProgressDialog(context_);
            waitDataDialog.setMessage("Retrieving EConBadge Information");
            waitDataDialog.setCancelable(false);
            waitDataDialog.show();

            Executors.newSingleThreadExecutor().execute(() -> {
                boolean result;

                result = EConBadgeManager.GetInstance().RemoveAnimation(viewModel_, model.getIndex());

                new Handler(Looper.getMainLooper()).post(() -> {
                    waitDataDialog.cancel();

                    if(!result) {
                        /* On failure, close dialog and display error message */
                        new AlertDialog.Builder(context_)
                                .setTitle("Error")
                                .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                        "disconnect from the badge and reconnect.")
                                .setPositiveButton(android.R.string.ok, null)
                                .setIcon(android.R.drawable.ic_dialog_alert)
                                .show();
                    }
                    else {
                        notifyDataSetChanged();
                    }
                });
            });
        });
    }

    @Override
    public int getItemCount() {
        return viewModel_.GetAnimations().size();
    }

    public class AnimationHolder extends RecyclerView.ViewHolder {

        private TextView textViewAnimType_;
        private TextView textViewAnimParam_;
        private Button   buttonRemove_;

        public AnimationHolder(View itemView) {
            super(itemView);

            textViewAnimType_  = itemView.findViewById(R.id.textViewAnimTypeAnimationListItem);
            textViewAnimParam_ = itemView.findViewById(R.id.textViewAnimParamAnimationListItem);
            buttonRemove_      = itemView.findViewById(R.id.buttonRemoveAnimationListItem);
        }

        public void setAnimType(String type) {
            textViewAnimType_.setText(type);
        }

        public void setAnimParam(String param) {
            textViewAnimParam_.setText(param);
        }

        public void setRemoveAction(View.OnClickListener action) {
            buttonRemove_.setOnClickListener(action);
        }
    }
}
