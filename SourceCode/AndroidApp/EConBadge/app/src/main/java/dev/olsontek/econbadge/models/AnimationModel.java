package dev.olsontek.econbadge.models;

public class AnimationModel {
    public void setIndex_(Integer index_) {
        this.index_ = index_;
    }

    private Integer index_;

    private byte type_;

    private byte param_;

    public AnimationModel(Integer index, byte type, byte param) {
        index_ = index;
        type_  = type;
        param_ = param;
    }

    public Integer getIndex() {
        return index_;
    }

    public byte getType() {
        return type_;
    }

    public byte getParam() {
        return param_;
    }

    public void setIndex(Integer index) {
        index_ = index;
    }

    public void setType(byte type) {
        type_ = type;
    }

    public void setParam(byte param) {
        param_ = param;
    }

}
