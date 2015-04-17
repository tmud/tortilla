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
        if (index == -1 && dlg_state->item != -1)
        {
            index = dlg_state->item;
            dlg_state->item = -1; 
        }
        if (dlg_state->topitem != -1) {
            plist->setTopItem(dlg_state->topitem);
            dlg_state->topitem = -1;
        }
        plist->SelectItem(index);
    }

    void setCanSaveState()
    {
        dlg_state->cansave = true;
    }

    void save(const tstring &current_group, bool filter_mode)
    {
        if (!dlg_state->cansave)
            return;
        dlg_state->group = current_group;
        dlg_state->filtermode = filter_mode;
        dlg_state->item = plist->getOnlySingleSelection();
        dlg_state->topitem = plist->getTopItem();
        dlg_state->cansave = false;
    }
};
