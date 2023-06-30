package io.github.oxmose.olsontek.econbadge.main;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import io.github.oxmose.olsontek.econbadge.R;
import io.github.oxmose.olsontek.econbadge.data.NetworkLinker;

public class SplashActivity extends AppCompatActivity {
    private Toast infoToast = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Button step1Button;
        Button step2Button;
        Button step3Button;

        Button connectButton;

        AlertDialog step1Dialog;
        AlertDialog step2Dialog;
        AlertDialog step3Dialog;

        AlertDialog.Builder diagBuilder;

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        /* Creates the dialogs */
        diagBuilder = new AlertDialog.Builder(this);
        diagBuilder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                dialogInterface.dismiss();
            }
        });

        diagBuilder.setMessage(R.string.step_1_detailed_text)
                   .setTitle(R.string.step_1_title);
        step1Dialog = diagBuilder.create();

        diagBuilder.setMessage(R.string.step_2_detailed_text)
                .setTitle(R.string.step_2_title);
        step2Dialog = diagBuilder.create();

        diagBuilder.setMessage(R.string.step_3_detailed_text)
                .setTitle(R.string.step_3_title);
        step3Dialog = diagBuilder.create();


        /* Setup the buttons' dialog box */
        step1Button = findViewById(R.id.activity_splash_step1_button);
        step1Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                step1Dialog.show();
            }
        });
        step2Button = findViewById(R.id.activity_splash_step2_button);
        step2Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                step2Dialog.show();
            }
        });
        step3Button = findViewById(R.id.activity_splash_step3_button);
        step3Button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                step3Dialog.show();
            }
        });

        /* Setup the connect button */
        connectButton = findViewById(R.id.activity_splash_connect_button);
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NetworkLinker linker = new NetworkLinker();
                linker.TryLink(new NetworkLinker.LinkerCallback() {
                    @Override
                    public void postExecCallback(boolean isLinked) {
                        if (isLinked) {
                            /* We are successfully linked */
                            if (infoToast != null) {
                                infoToast.cancel();
                            }

                            infoToast = Toast.makeText(getApplicationContext(),
                                    R.string.link_success_toast,
                                    Toast.LENGTH_LONG);
                            infoToast.show();

                            /* Launch main activity */
                            Intent i = new Intent(SplashActivity.this, MainActivity.class);
                            finish();
                            startActivity(i);
                        } else {
                            /* Error while linking, display error toast */
                            if (infoToast != null) {
                                infoToast.cancel();
                            }

                            infoToast = Toast.makeText(getApplicationContext(),
                                    getString(R.string.cannot_link_toast),
                                    Toast.LENGTH_LONG);
                            infoToast.show();
                        }
                    }
                },
                getApplicationContext());
            }
        });
    }
}