package dev.olsontek.econbadge;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Array;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class EInkImage
{
    private static final Map<Integer, Byte> USED_PALETTE;
    static {
        Map<Integer, Byte> aMap = new HashMap<>();
        aMap.put(0x000000, (byte)0);
        aMap.put(0xFFFFFF, (byte)1);
        aMap.put(0x4b6e54, (byte)2);
        aMap.put(0x37436a, (byte)3);
        aMap.put(0xa4504b, (byte)4);
        aMap.put(0xdccc5f, (byte)5);
        aMap.put(0xc06650, (byte)6);
        USED_PALETTE = Collections.unmodifiableMap(aMap);
    }
    private static final Map<Byte, Integer> REVERT_PALETTE;
    static {
        Map<Byte, Integer> aMap = new HashMap<>();
        aMap.put((byte)0, 0xFF000000);
        aMap.put((byte)1, 0xFFFFFFFF);
        aMap.put((byte)2, 0xFF546e4b);
        aMap.put((byte)3, 0xFF6a4337);
        aMap.put((byte)4, 0xFF4b50a4);
        aMap.put((byte)5, 0xFF5fccdc);
        aMap.put((byte)6, 0xFF5066c0);
        REVERT_PALETTE = Collections.unmodifiableMap(aMap);
    }
    private static List<Byte> DYN_PALETTE = new ArrayList<>();

    private String displayName_;
    private String path_;

    private byte[] imageData_;

    private boolean isValid_;

    public EInkImage() {
        isValid_ = false;
    }

    private boolean generateImageData(int startBitmap, int startPalette) {
        int  i;
        int  j;
        int  color;
        byte currByte;
        byte[] imageData;
        ByteBuffer buffer = ByteBuffer.wrap(imageData_);
        buffer.order(ByteOrder.LITTLE_ENDIAN);

        /* Construct the palette */
        DYN_PALETTE.clear();
        for(i = 0; i < 7; ++i) {
            color = buffer.getInt(startPalette + i * 4);

            if(!USED_PALETTE.containsKey(color)) {
                Log.e("EINK_IMAGE", "Unknown color " + color + " at position " + i + ", please update the palette in the python script.");
                return false;
            }

            DYN_PALETTE.add(USED_PALETTE.get(color));
        }

        /* Create the actual image data */
        imageData = new byte[448 * 600 / 2];
        currByte  = 0;
        for(i = 0; i < 448; ++i) {
            for(j = 0; j < 600; ++j) {
                color = imageData_[startBitmap + (447 - i) * 600 + j];
                if(j % 2 == 0) {
                    currByte = (byte)((DYN_PALETTE.get(color) << 4) & 0xF0);
                }
                else {
                    currByte |= (byte)(DYN_PALETTE.get(color) & 0x0F);
                    imageData[(i * 600 + j) / 2] = currByte;
                }
            }
        }

        imageData_ = imageData;

        return true;
    }

    public boolean isValid() {
        ByteBuffer buffer;
        int        dataInt;
        int        startBitmap;
        int        startPalette;
        DataInputStream dis;

        if(isValid_) {
            return true;
        }

        if(null == path_) {
            return false;
        }

        /* Load the file */
        File file = new File(path_);
        imageData_ = new byte[(int) file.length()];
        try {
            dis = new DataInputStream(new FileInputStream(file));
            dis.readFully(imageData_);
            dis.close();
        }
        catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        if(imageData_ != null) {
            buffer = ByteBuffer.wrap(imageData_);
            buffer.order(ByteOrder.LITTLE_ENDIAN);

            /* Check the magic */
            if(!new String(Arrays.copyOfRange(imageData_, 0, 2), StandardCharsets.US_ASCII).equals("BM")) {
                Log.e("EINK_IMAGE", "Wrong magic");
                return false;
            }

            /* Check the size */
            dataInt = buffer.getInt(18);
            if(dataInt != 600) {
                Log.e("EINK_IMAGE", "Wrong width: " + dataInt);
                return false;
            }
            dataInt = buffer.getInt(22);
            if(dataInt != 448) {
                Log.e("EINK_IMAGE", "Wrong height: " + dataInt);
                return false;
            }

            /* Check the bpp */
            dataInt = buffer.getShort(28);
            if(dataInt != 8) {
                Log.e("EINK_IMAGE", "Wrong BPP: " + dataInt);
                return false;
            }

            /* Check the compression */
            dataInt = buffer.getInt(30);
            if(dataInt != 0) {
                Log.e("EINK_IMAGE", "Wrong Compression: " + dataInt);
                return false;
            }

            /* Check the palette size */
            dataInt = buffer.getInt(46);
            if(dataInt != 7) {
                Log.e("EINK_IMAGE", "Wrong palette size: " + dataInt);
                return false;
            }

            /* Copy the actual bitmap data */
            startBitmap  = buffer.getInt(10);
            startPalette = startBitmap - 7 * 4;

            if(!generateImageData(startBitmap, startPalette)) {
                Log.e("EINK_IMAGE", "Cannot generate data");
                return false;
            }

            isValid_ = true;

            return true;
        }

        return false;
    }

    public void setImageData(byte[] imageData) {
        imageData_ = imageData;
    }

    public List<Integer> getRawPixels() {
        List<Integer> pixels = new ArrayList<>();
        byte splitData;

        if(imageData_ != null) {
            for(byte data : imageData_) {
                splitData = (byte)(data & 0x0F);
                if(REVERT_PALETTE.containsKey(splitData)) {
                    pixels.add(REVERT_PALETTE.get(splitData));
                }
                else {
                    Log.d("UNKNOWN", " " + splitData);
                    pixels.add(0);
                }
                splitData = (byte)(data >> 4);
                if(REVERT_PALETTE.containsKey(splitData)) {
                    pixels.add(REVERT_PALETTE.get(splitData));
                }
                else {
                    Log.d("UNKNOWN", " " + splitData);
                    pixels.add(0xFF000000);
                }
            }
        }

        return pixels;
    }

    @SuppressLint("Range")
    public static EInkImage getFileMetaData(Context context, Uri uri) {
        EInkImage fileMetaData = new EInkImage();

        fileMetaData.displayName_ = FileHelper.getContentName(context.getContentResolver(), uri);
        fileMetaData.path_ = FileHelper.getRealPathFromURI(context, uri);

        return  fileMetaData;
    }

    public String getDisplayName() {
        return displayName_;
    }

    public byte[] getImageData() {
        return imageData_;
    }
}