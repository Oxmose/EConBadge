package io.github.oxmose.olsontek.econbadge.data;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.github.oxmose.olsontek.econbadge.data.AppConfig;

public class NetworkLinker {

    private static final String LOG_LINKER = "ECON_LINKER";
    private boolean isLinked;

    public interface LinkerCallback {
        public void postExecCallback(boolean isLinked);
    }

    public void TryLink(LinkerCallback callbackFunction, Context context) {
        ExecutorService executor;
        Handler         handler;
        AppConfig config;

        /* Creates executor and handler */
        executor = Executors.newSingleThreadExecutor();
        handler = new Handler(Looper.getMainLooper());

        /* Execute in background */
        config = AppConfig.getInstance();
        executor.execute(() -> {
            Socket         linkerSocket;
            BufferedReader inputReader;
            BufferedWriter outputWriter;
            char[]         buffer;
            int            readCount;
            /* Connect to socket */
            isLinked     = false;
            linkerSocket = null;
            try {
                Log.d(LOG_LINKER, "Trying to link badge | IP: " +
                                 config.getBadgeIPAddress() + " Port: " +
                                 new Integer(config.getBadgeIPPort()));

                ConnectivityManager connectivity = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
                if (connectivity != null){
                    for (Network network : connectivity.getAllNetworks()){
                        NetworkInfo networkInfo = connectivity.getNetworkInfo(network);

                        if (networkInfo != null &&
                            networkInfo.getType() == ConnectivityManager.TYPE_WIFI){
                            if (networkInfo.isConnected()){
                                linkerSocket =  network.getSocketFactory().createSocket();
                            }
                        }
                    }
                }

                if(linkerSocket != null) {
                    linkerSocket.connect(new InetSocketAddress(config.getBadgeIPAddress(),
                                                               config.getBadgeIPPort()));

                    Log.d(LOG_LINKER, "Socket connected");

                    inputReader = new BufferedReader(new InputStreamReader(linkerSocket.getInputStream()));
                    outputWriter = new BufferedWriter(new OutputStreamWriter(linkerSocket.getOutputStream()));

                    buffer = new char[4];

                    /* Write the ping command */
                    outputWriter.write("\0\0\0\0", 0, 4);
                    outputWriter.flush();

                    Log.d(LOG_LINKER, "Sent PING command");

                    /* Wait for pong response */
                    readCount = inputReader.read(buffer, 0, 4);
                    Log.d(LOG_LINKER, "Linker response: " + String.valueOf(buffer));
                    if (readCount == 4 && String.valueOf(buffer).equals("PONG")) {
                        isLinked = true;
                        config.setBadgeSocket(linkerSocket);
                    }
                }
            }
            catch (IOException e) {
                Log.e("NetworkLinker", "Could not link with badge.");
                e.printStackTrace();
            }
            finally {
                if(linkerSocket != null) {
                    try {
                        linkerSocket.close();
                    } catch (IOException e) {
                        Log.e("NetworkLinker", "Could not close socket.");
                        e.printStackTrace();
                    }
                }
            }

            handler.post(() -> {
                /* Call callback function */
                callbackFunction.postExecCallback(isLinked);
            });
        });
    }
}
