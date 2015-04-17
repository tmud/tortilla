#pragma once

class PropertiesSaveHelper
{
    PropertiesDlgPageState* dlg_state;
    PropertyListCtrl* plist;
public:
    PropertiesSaveHelper() : dlg_state(NULL), plist(NULL)
    {
    }

    void init(PropertiesDlgPageState* state, PropertyListCtrl* list)
    {
        assert(state && list);
        dlg_state = state;
        plist = list;
    }

    void loadGroupAndFilter(tstring &current_group, bool &filter_mode)
    {
        const tstring& cgroup = dlg_state->group;
        if (!cgroup.empty())
        {
            current_group = cgroup;
            filter_mode = dlg_state->filtermode;
        }
    }

    void loadCursorAndTopPos(int index)
    {
        int count = plist->GetItemCount();
        if (index == -1 && dlg_state->item != -1)
        {
            index = dlg_state->item;
            if (index < 0 || index >= count)
                index = -1;
            dlg_state->item = -1;
        }
        if (dlg_state->topitem != -1) 
        {
            int top = dlg_state->topitem;
            dlg_state->topitem = -1;
            if (top >= 0 && top < count)
                plist->setTopItem(top);
        }
        plist->SelectItem(index);
    }

    void setCanSaveState()
    {
        dlg_state->cansave = true;
    }

    bool save(const tstring &current_group, bool filter_mode)
    {
        if (!dlg_state->cansave)
            return false;
        dlg_state->group = current_group;
        dlg_state->filtermode = filter_mode;
        dlg_state->item = plist->getOnlySingleSelection();
        dlg_state->topitem = plist->getTopItem();
        dlg_state->cansave = false;
        return true;
    }
};
