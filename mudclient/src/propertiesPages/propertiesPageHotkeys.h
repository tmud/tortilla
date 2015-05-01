#pragma once

#include "hotkeyTable.h"
#include "propertiesSaveHelper.h"

class HotkeyBox :  public CWindowImpl<HotkeyBox, CEdit>
{
    HotkeyTable m_table;

    BEGIN_MSG_MAP(HotkeyBox)
       MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
       MESSAGE_HANDLER(WM_SYSKEYUP, OnKeyUp)
       MESSAGE_HANDLER(WM_KEYDOWN, OnBlock)
       MESSAGE_HANDLER(WM_SYSKEYDOWN, OnBlock)
       MESSAGE_HANDLER(WM_CHAR, OnBlock)
    END_MSG_MAP()

    LRESULT OnKeyUp(UINT, WPARAM wparam, LPARAM lparam, BOOL& bHandled)
    {
        tstring key;
        m_table.recognize(wparam, lparam, &key);
        if (!key.empty())
        {
            SetWindowText(key.c_str());
            ::SendMessage(GetParent(), WM_USER, 0, 0); 
        }
        else
            bHandled = FALSE;
        return 0;
    }

    LRESULT OnBlock(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        return 0;
    }
};

class PropertyHotkeys :  public CDialogImpl<PropertyHotkeys>
{
    PropertiesValues *propValues;
    PropertiesValues *propGroups;
    PropertiesValues m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    HotkeyBox m_hotkey;
    CEdit m_text;
    CButton m_add;
    CButton m_del;
    CButton m_reset;
    CButton m_filter;
    CComboBox m_cbox;
    bool m_filterMode;
    tstring m_currentGroup;
    bool m_deleted;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
    enum { IDD = IDD_PROPERTY_HOTKEYS };
    PropertyHotkeys(PropertiesData *data) : propValues(NULL), propGroups(NULL), m_filterMode(false), m_deleted(false), m_update_mode(false), dlg_state(NULL) 
    {
        propValues = &data->hotkeys;
        propGroups = &data->groups;
    }
    void setParams( PropertiesDlgPageState *state)
    {
        dlg_state = state;
    }

private:
    BEGIN_MSG_MAP(PropertyHotkeys)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER+1, OnSetFocus)
       MESSAGE_HANDLER(WM_USER, OnHotkeyEditChanged)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_RESET, OnResetData)
       COMMAND_ID_HANDLER(IDC_CHECK_GROUP_FILTER, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged)
       COMMAND_HANDLER(IDC_EDIT_HOTKEY_TEXT, EN_CHANGE, OnHotkeyTextChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring hotkey, text;
        getWindowText(m_hotkey, &hotkey);
        getWindowText(m_text, &text);

        int index = m_list_values.find(hotkey);
        if (index == -1 && m_filterMode)
        {
            int index2 = propValues->find(hotkey);
            if (index2 != -1)
                propValues->del(index2);
        }

        m_list_values.add(index, hotkey, text, m_currentGroup);

        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, hotkey);
            m_list.addItem(pos, 1, text);
            m_list.addItem(pos, 2, m_currentGroup);
        }
        else
        {
            m_list.setItem(index, 0, hotkey);
            m_list.setItem(index, 1, text);
            m_list.setItem(index, 2, m_currentGroup);
        }

        if (index == -1)
            index = m_list.GetItemCount()-1;
        m_list.SelectItem(index);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnDeleteElement(WORD, WORD, HWND, BOOL&)
    {
        std::vector<int> selected;
        m_list.getSelected(&selected);
        int items = selected.size();
        if (items == 1)
            m_deleted = true;
        for (int i = 0; i < items; ++i)
        {
            int index = selected[i];
            m_list.DeleteItem(index);
            m_list_values.del(index);
        }
        m_deleted = false;
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnResetData(WORD, WORD, HWND, BOOL&)
    {
        int sel = m_list.getOnlySingleSelection();
        m_list.SelectItem(-1);
        m_text.SetWindowText(L"");
        m_hotkey.SetWindowText(L"");
        if (sel == -1)
        {
            m_add.EnableWindow(FALSE);
            m_reset.EnableWindow(FALSE);
        }
        m_hotkey.SetFocus();
        return 0;
    }

    LRESULT OnFilter(WORD, WORD, HWND, BOOL&)
    {
        saveValues();
        m_filterMode = m_filter.GetCheck() ? true : false;
        loadValues();
        update();
        return 0;
    }

    LRESULT OnGroupChanged(WORD, WORD, HWND, BOOL&)
    {
        tstring group;
        getCurrentGroup(&group);
        if (!m_filterMode)
        {
            m_currentGroup = group;
            updateCurrentItem();
            return 0;
        }
        tstring old = m_currentGroup;
        m_currentGroup = group;
        updateCurrentItem();
        m_currentGroup = old;
        saveValues();
        m_currentGroup = group;
        loadValues();
        update();
        return 0;
    }

    LRESULT OnHotkeyEditChanged(UINT, WPARAM, LPARAM, BOOL&)
    {
        if (!m_update_mode)
        {
             int len = m_hotkey.GetWindowTextLength();
             m_add.EnableWindow(len == 0 ? FALSE : TRUE);
             m_reset.EnableWindow(len == 0 ? FALSE : TRUE);
             if (len > 0)
             {
                tstring hotkey;
                getWindowText(m_hotkey, &hotkey);
                int index = m_list_values.find(hotkey);
                if (index != -1)
                {
                    m_list.SelectItem(index);
                    m_hotkey.SetSel(len, len);
                }
              }
        }
        return 0;
    }

    void updateCurrentItem()
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;
        tstring hotkey;
        getWindowText(m_hotkey, &hotkey);
        property_value& v = m_list_values.getw(item);
        if (v.key != hotkey) return;
        tstring text;
        getWindowText(m_text, &text);
        if (v.value != text)
        {
            v.value = text;
            m_list.setItem(item, 1, text);
        }
        if (v.group != m_currentGroup)
        {
            v.group = m_currentGroup;
            m_list.setItem(item, 2, m_currentGroup);
        }
    }

    LRESULT OnHotkeyTextChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_update_mode)
            updateCurrentItem();
        return 0;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        m_update_mode = true;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected == 0)
        {
            m_del.EnableWindow(FALSE);
            m_add.EnableWindow(FALSE);
            m_reset.EnableWindow(FALSE);
            if (!m_deleted) 
            {
              m_hotkey.SetWindowText(L"");
              m_text.SetWindowText(L"");
            }
        }
        else if (items_selected == 1)
        {
            m_del.EnableWindow(TRUE);
            m_reset.EnableWindow(TRUE);
            int item = m_list.getOnlySingleSelection();
            const property_value& v = m_list_values.get(item);
            m_hotkey.SetWindowText( v.key.c_str() );
            m_text.SetWindowText( v.value.c_str() );
            int index = getGroupIndex(v.group);
            m_cbox.SetCurSel(index);
            m_currentGroup = v.group;
        }
        else
        {
            m_del.EnableWindow(TRUE);
            m_add.EnableWindow(FALSE);
            m_reset.EnableWindow(FALSE);
            m_hotkey.SetWindowText(L"");
            m_text.SetWindowText(L"");
        }
        m_update_mode = false;
        return 0;
    }

    LRESULT OnListKillFocus(int , LPNMHDR , BOOL&)
    {
        if (GetFocus() != m_del && m_list.GetSelectedCount() > 1)
            m_list.SelectItem(-1);
        return 0;
    }

    LRESULT OnShowWindow(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        if (wparam)
        {
            loadValues();
            m_update_mode = true;
            m_hotkey.SetWindowText(L"");
            m_text.SetWindowText(L"");
            m_update_mode = false;
            update();
            PostMessage(WM_USER+1); // OnSetFocus to list
            m_state_helper.setCanSaveState();
        }
        else
        {
            m_del.EnableWindow(FALSE);
            saveValues();
        }
        return 0;
    }

    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_hotkey.SubclassWindow(GetDlgItem(IDC_EDIT_HOTKEY));
        m_text.Attach(GetDlgItem(IDC_EDIT_HOTKEY_TEXT));
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_reset.Attach(GetDlgItem(IDC_BUTTON_RESET));
        m_filter.Attach(GetDlgItem(IDC_CHECK_GROUP_FILTER));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(L"Hotkey", 20);
        m_list.addColumn(L"Текст", 60);
        m_list.addColumn(L"Группа", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);        
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));
        m_add.EnableWindow(FALSE);
        m_del.EnableWindow(FALSE);
        m_reset.EnableWindow(FALSE);
        m_state_helper.init(dlg_state, &m_list);
        m_state_helper.loadGroupAndFilter(m_currentGroup, m_filterMode);
        if (m_filterMode)
            m_filter.SetCheck(BST_CHECKED);
        loadValues();
        return 0;
    }

    LRESULT OnCloseDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        saveValues();
        return 0;
    }

    void update()
    {
        int current_index = 0;
        m_cbox.ResetContent();
        for (int i=0,e=propGroups->size(); i<e; ++i)
        {
            const property_value& g = propGroups->get(i);
            m_cbox.AddString(g.key.c_str());
            if (g.key == m_currentGroup)
                { current_index = i; }
        }
        m_cbox.SetCurSel(current_index);
        const property_value& g = propGroups->get(current_index);
        m_currentGroup = g.key;

        m_list.DeleteAllItems();
        for (int i=0,e=m_list_values.size(); i<e; ++i)
        {
            const property_value& v = m_list_values.get(i);
            m_list.addItem(i, 0, v.key);
            m_list.addItem(i, 1, v.value);
            m_list.addItem(i, 2, v.group);
        }

        int index = -1;
        tstring hotkey;
        getWindowText(m_hotkey, &hotkey);
        if (!hotkey.empty())
            index = m_list_values.find(hotkey);
        m_state_helper.loadCursorAndTopPos(index);
    }

    void loadValues()
    {
        if (!m_filterMode)
        {
            m_list_values = *propValues;
            return;
        }

        m_list_values.clear();
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v = propValues->get(i);
            if (v.group != m_currentGroup)
                continue;
            m_list_values.add(-1, v.key, v.value, v.group);
        }
    }

    void saveValues()
    {
        if (!m_state_helper.save(m_currentGroup, m_filterMode))
            return;

        if (!m_filterMode)
        {
            *propValues = m_list_values;
            return;
        }

        std::vector<int> positions;
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v = propValues->get(i);
            if (v.group == m_currentGroup)
                positions.push_back(i);
        }

        int pos_count = positions.size();
        int elem_count = m_list_values.size();
        for (int i=0; i<elem_count; ++i)
        {
            const property_value& v = m_list_values.get(i);
            int index = (i < pos_count) ? positions[i] : -1;
            propValues->add(index, v.key, v.value, v.group);
        }

        int todelete = pos_count - elem_count;
        for (int i=0; i<todelete; ++i)
        {
            int pos = pos_count-(i+1);
            propValues->del(pos);
        }
    }

    int getGroupIndex(const tstring& group)
    {
        int index = -1;
        for (int i=0,e=propGroups->size(); i<e; ++i)
        {
            const property_value& g = propGroups->get(i);
            if (g.key == group)
                { index = i; break; }
        }
        return index;
    }

    void getCurrentGroup(tstring *group)
    {
        int index = m_cbox.GetCurSel();
        int len = m_cbox.GetLBTextLen(index) + 1;
        WCHAR *buffer = new WCHAR[len];
        m_cbox.GetLBText(index, buffer);
        group->assign(buffer);
        delete[]buffer;
    }
};
