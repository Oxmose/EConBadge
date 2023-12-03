package dev.olsontek.econbadge;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.google.android.material.navigation.NavigationView;

import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

import dev.olsontek.econbadge.databinding.ActivityMainDashboardBinding;

public class MainDashboardActivity extends AppCompatActivity {

    private AppBarConfiguration appBarConfiguration_;
    private ActivityMainDashboardBinding binding_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding_ = ActivityMainDashboardBinding.inflate(getLayoutInflater());
        setContentView(binding_.getRoot());

        setSupportActionBar(binding_.appBarMainDashboard.toolbar);

        DrawerLayout drawer = binding_.drawerLayout;
        NavigationView navigationView = binding_.navView;
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        appBarConfiguration_ = new AppBarConfiguration.Builder(R.id.navDashboard,
                                                               R.id.navLedBorder,
                                                               R.id.navImageUpdate,
                                                               R.id.navSettings,
                                                               R.id.navAbout)
                .setOpenableLayout(drawer)
                .build();

        Button btn = navigationView.getHeaderView(0).findViewById(R.id.btnDisconnectNavHeader);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                /* Clear the btManager */
                EConBadgeManager.GetInstance().CancelConnect();

                /* Kill the activity and present the main activity */
                Toast.makeText(MainDashboardActivity.this, "EConBadge Disconnected", Toast.LENGTH_LONG).show();
                Intent intent = new Intent(MainDashboardActivity.this, MainActivity.class);
                MainDashboardActivity.this.finish();
                startActivity(intent);
            }
        });

        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main_dashboard);
        NavigationUI.setupActionBarWithNavController(this, navController, appBarConfiguration_);
        NavigationUI.setupWithNavController(navigationView, navController);
    }

    @Override
    public boolean onSupportNavigateUp() {
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main_dashboard);
        return NavigationUI.navigateUp(navController, appBarConfiguration_)
                || super.onSupportNavigateUp();
    }
}