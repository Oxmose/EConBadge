package dev.olsontek.econbadge.ui.dialog;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.ViewGroup;
import android.widget.Button;

import java.util.concurrent.Executors;

import dev.olsontek.econbadge.EConBadgeManager;
import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.adapters.AnimationRemoverAdapter;
import dev.olsontek.econbadge.ui.ledborder.LedBorderViewModel;

public class RemoveAnimationActivity extends Dialog {
    private Button buttonCancel_;

    private ProgressDialog waitDataDialog_;

    private LedBorderViewModel ledBorderViewModel_;

    private RecyclerView recycler_;
    private AnimationRemoverAdapter listAdapter_;

    public RemoveAnimationActivity(@NonNull Context context, LedBorderViewModel viewModel) {
        super(context);

        ledBorderViewModel_ = viewModel;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remove_animation);

        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

        buttonCancel_      = findViewById(R.id.buttonExitRemoveAnimationActivity);
        buttonCancel_.setOnClickListener(view -> dismiss());

        recycler_ = findViewById(R.id.recycleViewRemoveAnimationActivity);
        listAdapter_ = new AnimationRemoverAdapter(getContext(),
                                                   ledBorderViewModel_);
        LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
        recycler_.setLayoutManager(layoutManager);
        recycler_.setAdapter(listAdapter_);

        /* Open progress dialog */
        waitDataDialog_ = new ProgressDialog(getContext());
        waitDataDialog_.setMessage("Retrieving EConBadge Information");
        waitDataDialog_.setCancelable(false);
        waitDataDialog_.show();

        Executors.newSingleThreadExecutor().execute(() -> {
            boolean result;

            result = EConBadgeManager.GetInstance().GetLedBorderInformation(ledBorderViewModel_);

            new Handler(Looper.getMainLooper()).post(() -> {
                waitDataDialog_.cancel();

                /* On connection, finish this activity, start the manager activity */
                if(result) {
                    /* Get the information */
                    listAdapter_.notifyDataSetChanged();
                }
                else {
                    /* On failure, close dialog and display error message */
                    new AlertDialog.Builder(getContext())
                            .setTitle("Error")
                            .setMessage("Cannot contact the EConBadge, is the error persists, " +
                                    "disconnect from the badge and reconnect.")
                            .setPositiveButton(android.R.string.ok, null)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .show();
                }
            });
        });
    }
}