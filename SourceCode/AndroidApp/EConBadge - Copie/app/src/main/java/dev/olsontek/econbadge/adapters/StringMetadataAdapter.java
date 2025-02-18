package dev.olsontek.econbadge.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.ArrayList;

import dev.olsontek.econbadge.R;
import dev.olsontek.econbadge.models.StringMetadataItem;

public class StringMetadataAdapter extends ArrayAdapter {
    public StringMetadataAdapter(Context context, ArrayList<StringMetadataItem> items) {
        super(context, 0, items);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        StringMetadataItem itemModel;
        TextView           itemView;

        itemModel = (StringMetadataItem) getItem(position);

        if (convertView == null) {
            convertView = LayoutInflater.from(getContext()).inflate(R.layout.bt_device_list_item,
                                                                    parent,
                                                                    false);
        }

        itemView = convertView.findViewById(R.id.itemTextViewBtDeviceListItem);
        itemView.setText(itemModel.displayString_);

        return convertView;
    }
}
