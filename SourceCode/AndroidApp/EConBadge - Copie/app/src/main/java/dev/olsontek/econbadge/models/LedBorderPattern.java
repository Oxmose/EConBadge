package dev.olsontek.econbadge.models;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class LedBorderPattern {
    public static boolean CheckTypeIdx(int typeId) {
        return (typeId >= 0 && typeId <= 4);
    }

    public LedBorderPattern GetCopy() {
        int i ;

        LedBorderPattern newPattern = new LedBorderPattern();

        newPattern.type_ = type_;
        newPattern.startColorCode_ = new int[4];
        newPattern.endColorCode_ = new int[4];
        newPattern.gradientSize_ = new int[4];

        for(i = 0; i < 4; ++i) {
            newPattern.startColorCode_[i] = startColorCode_[i];
            newPattern.endColorCode_[i] = endColorCode_[i];
            newPattern.gradientSize_[i] = gradientSize_[i];
        }

        newPattern.plainColorCode_ = plainColorCode_;

        return newPattern;
    }

    public enum PATTERN_TYPE {
        PLAIN,
        GRAD1,
        GRAD2,
        GRAD3,
        GRAD4
    }

    private PATTERN_TYPE type_;
    private int[] startColorCode_ = new int[4];
    private int[] endColorCode_ = new int[4];
    private int[] gradientSize_ = new int[4];
    private int   plainColorCode_;

    public LedBorderPattern() {
        int i;

        for(i = 0; i < 4; ++i) {
            startColorCode_[i] = 0xFFFFFFFF;
            endColorCode_[i] = 0xFFFFFFFF;
            gradientSize_[i] = 5;
        }

        plainColorCode_ = 0xFFFFFFFF;
        type_ = PATTERN_TYPE.PLAIN;
    }

    public int GetTypeIdx() {
        switch(type_) {
            case GRAD1:
                return 1;
            case GRAD2:
                return 2;
            case GRAD3:
                return 3;
            case GRAD4:
                return 4;
            default:
                return 0;
        }
    }

    public void SetTypeIdx(int type) {
        switch(type) {
            case 1:
                type_ = PATTERN_TYPE.GRAD1;
                break;
            case 2:
                type_ = PATTERN_TYPE.GRAD2;
                break;
            case 3:
                type_ = PATTERN_TYPE.GRAD3;
                break;
            case 4:
                type_ = PATTERN_TYPE.GRAD4;
                break;
            default:
                type_ = PATTERN_TYPE.PLAIN;
        }
    }

    public ByteBuffer GetRawData() {
        ByteBuffer rawData;
        int        i;

        rawData = ByteBuffer.allocate(37);
        rawData.order(ByteOrder.LITTLE_ENDIAN);

        rawData.put((byte)GetTypeIdx());
        for(i = 0; i < 4; ++i) {
            rawData.putInt(startColorCode_[i]);
        }
        for(i = 0; i < 4; ++i) {
            rawData.putInt(endColorCode_[i]);
        }
        if(type_ == LedBorderPattern.PATTERN_TYPE.PLAIN) {
            rawData.putInt(plainColorCode_);
        }
        else {
            for(i = 0; i < 4; ++i) {
                rawData.put((byte)gradientSize_[i]);
            }
        }

        return rawData;
    }

    public PATTERN_TYPE GetType() {
        return type_;
    }

    public void SetType(PATTERN_TYPE type) {
        this.type_ = type;
    }

    public int GetStartColorCode(int idx) {
        if(idx >= 0 && idx < 4) {
            return startColorCode_[idx] | 0xFF000000;
        }
        return 0xFFFFFFFF;
    }

    public int GetEndColorCode(int idx) {
        if(idx >= 0 && idx < 4) {
            return endColorCode_[idx] | 0xFF000000;
        }
        return 0xFFFFFFFF;
    }

    public int GetGradientSize(int idx) {
        if(idx >= 0 && idx < 4) {
            return gradientSize_[idx];
        }
        return 0;
    }
    public int GetPlainColorCode() {
        return plainColorCode_ | 0xFF000000;
    }

    public void SetPlainColorCode(int code) {
        plainColorCode_ = code | 0xFF000000;
    }

    public void SetStartColorCode(int idx, int code) {
        if(idx >= 0 && idx < 4) {
            startColorCode_[idx] = code | 0xFF000000;
        }
    }

    public void SetEndColorCode(int idx, int code) {
        if(idx >= 0 && idx < 4) {
            endColorCode_[idx] = code | 0xFF000000;
        }
    }

    public void SetGradientSize(int idx, int size) {
        if(idx >= 0 && idx < 4) {
            gradientSize_[idx] = size;
        }
    }


}
