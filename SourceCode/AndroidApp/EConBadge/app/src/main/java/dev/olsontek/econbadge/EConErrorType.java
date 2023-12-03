package dev.olsontek.econbadge;

public class EConErrorType {
    public static final int NO_ERROR = 0;
    public static final int BT_CANNOT_CONNECT = 1;
    public static final int CANNOT_GET_UPDATE_INFO = 2;
    public static final int CANNOT_CONTACT_ECONBADGE = 3;
    public static final int CANNOT_GET_UPDATE_VERSION = 4;
    public static final int CANNOT_GET_UPDATE_CHECKSUM = 5;
    public static final int INCORRECT_UPDATE_CHECKSUM = 6;
    public static final int CANNOT_GET_UPDATE_BINARY = 7;
    public static final int UPDATE_IS_NOT_NEWER = 8;
    public static final int ECONBADGE_NOT_READY = 9;
}
