package io.github.oxmose.olsontek.econbadge.data;

import java.net.Socket;

public class AppConfig {

    private String badgeIPAddress    = "192.168.0.1";
    private int badgeIPPort = 5000;

    private Socket badgeSocket = null;

    private static AppConfig instance = null;
    private AppConfig() {

    }

    public static AppConfig getInstance() {
        if(instance == null) {
            instance = new AppConfig();
        }

        return instance;
    }


    public String getBadgeIPAddress() {
        return badgeIPAddress;
    }

    public int getBadgeIPPort() {
        return badgeIPPort;
    }

    public Socket getBadgeSocket() {
        return badgeSocket;
    }

    public void setBadgeSocket(Socket socket) {
        badgeSocket = socket;
    }
}
