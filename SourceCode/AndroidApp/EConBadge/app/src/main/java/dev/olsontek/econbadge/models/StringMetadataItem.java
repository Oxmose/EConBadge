package dev.olsontek.econbadge.models;

public class StringMetadataItem {
    public String displayString_;
    public String metadata_;

    public StringMetadataItem(String displayString, String metadata) {
        displayString_ = displayString;
        metadata_      = metadata;
    }

    @Override
    public int hashCode() {
        return metadata_.hashCode();
    }

    @Override
    public boolean equals(Object item) {
        if(!(item instanceof StringMetadataItem)) {
            return false;
        }
        return ((StringMetadataItem)item).metadata_ == metadata_;
    }
}
