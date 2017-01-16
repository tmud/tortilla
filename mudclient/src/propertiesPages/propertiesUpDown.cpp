#include "stdafx.h"
#include "propertiesUpDown.h"

void propertiesUpDown::up(PropertyListCtrl &list, PropertiesValues& values, bool mode)
{
    std::vector<int> selected;
    list.getSelectedUpSorted(&selected);
    if (selected.empty())
        return;
    std::vector<int> not_selected;
    int begin = selected[0] - 1;
    if (begin < 0) begin = 0;
    int last_selected = selected.size() - 1;
    int end = selected[last_selected];
    for (int i = begin; i <= end; ++i) {
        if (std::find(selected.begin(), selected.end(), i) == selected.end())
            not_selected.push_back(i);
    }
    selected.insert(selected.end(), not_selected.begin(), not_selected.end());
    std::vector<property_value> values_list;
    for (int i = 0, e = selected.size(); i < e; ++i)
    {
        const property_value& v = values.get(selected[i]);
        values_list.push_back(values.get(selected[i]));
    }
    int index = 0;
    for (int i = begin; i <= end; ++i) {
        const property_value& v = values_list[index++];
        list.setItem(i, 0, v.key);
        if (!mode) {
            list.setItem(i, 1, v.value);
            list.setItem(i, 2, v.group);
        }
        else {
            list.setItem(i, 1, v.group);
        }
        values.add(i, v.key, v.value, v.group);
    }
    list.SelectItem(-1);
    int selected_count = selected.size() - not_selected.size();
    end = begin + selected_count - 1;
    for (int i = begin; i <= end; ++i)
        list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
    if (begin < list.getTopItem())
        list.setTopItem(begin);
    list.SetFocus();
}

void propertiesUpDown::down(PropertyListCtrl &list, PropertiesValues& values, bool mode)
{
    std::vector<int> selected;
    list.getSelectedUpSorted(&selected);
    if (selected.empty())
        return;
    std::vector<int> not_selected;
    int last_selected = selected.size() - 1;
    int last_item = list.GetItemCount() - 1;

    int end = selected[last_selected] + 1;
    if (end > last_item) end = last_item;
    
    int begin = selected[0];
    for (int i = begin; i <= end; ++i) {
        if (std::find(selected.begin(), selected.end(), i) == selected.end())
            not_selected.push_back(i);
    }
    selected.insert(selected.begin(), not_selected.begin(), not_selected.end());
    std::vector<property_value> values_list;
    for (int i = 0, e = selected.size(); i < e; ++i)
    {
        const property_value& v = values.get(selected[i]);
        values_list.push_back(values.get(selected[i]));
    }
    int index = 0;
    for (int i = begin; i <= end; ++i) {
        const property_value& v = values_list[index++];
        list.setItem(i, 0, v.key);
        if (!mode) {
            list.setItem(i, 1, v.value);
            list.setItem(i, 2, v.group);
        } else {
            list.setItem(i, 1, v.group);
        }
        values.add(i, v.key, v.value, v.group);
    }
    list.SelectItem(-1);
    int selected_count = selected.size() - not_selected.size();
    begin = end - selected_count + 1;
    for (int i = begin; i <= end; ++i)
        list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
    int on_screen = list.getItemsOnScreen() - 1;
    int visible_last = list.getTopItem() + on_screen;
    if (end > visible_last)
    {
        int top_visible = end - on_screen;
        list.setTopItem(top_visible);
    }
    list.SetFocus();
}
