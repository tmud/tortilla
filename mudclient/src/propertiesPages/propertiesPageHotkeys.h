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
    PropertiesValuesT<tstring> m_list_values;
    std::vector<int> m_list_positions;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    HotkeyBox m_hotkey;
    CEdit m_text;
    CButton m_add;
    CButton m_del;
    CButton m_replace;
    CButton m_reset;
    CButton m_up;
    CButton m_down;
    CComboBox m_filter;
    CComboBox m_cbox;
    int m_filterMode;
    tstring m_currentGroup;
    tstring m_loadedGroup;
    bool m_deleted;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
    enum { IDD = IDD_PROPERTY_HOTKEYS };
    PropertyHotkeys(PropertiesData *data) : propValues(NULL), propGroups(NULL), m_filterMode(0), m_deleted(false), m_update_mode(false), dlg_state(NULL) 
    {
        propValues = &data->hotkeys;
        propGroups = &data->groups;
    }
    void setParams( PropertiesDlgPageState *state)
    {
        dlg_state = state;
    }
    bool updateChangedTemplate(bool check)
    {
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
            tstring pattern;
            getWindowText(m_hotkey, &pattern);
            property_value& v = m_list_values.getw(item);
            if (v.key != pattern && !pattern.empty())
            { 
                if (!check) {
                tstring text;
                getWindowText(m_text, &text);
                v.key = pattern; v.value = text; v.group = m_currentGroup;
                }
                return true;
            }
        }
        return false;
    }
private:
    BEGIN_MSG_MAP(PropertyHotkeys)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER+1, OnSetFocus)
       MESSAGE_HANDLER(WM_USER, OnHotkeyEditChanged)
       MESSAGE_HANDLER(WM_USER+2, OnKeyDown)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_REPLACE, OnReplaceElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_RESET, OnResetData)
       COMMAND_ID_HANDLER(IDC_BUTTON_UP, OnUpElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DOWN, OnDownElement)
       COMMAND_HANDLER(IDC_COMBO_FILTER, CBN_SELCHANGE, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged)
       COMMAND_HANDLER(IDC_EDIT_HOTKEY_TEXT, EN_CHANGE, OnHotkeyTextChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       //NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnKeyDown(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        if (wparam == VK_DELETE)
        {
            if (m_del.IsWindowEnabled()) {
                BOOL b = FALSE;
                OnDeleteElement(0, 0, 0, b);
            }
            return 1;
        }
        return 0;
    }

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring hotkey, text;
        getWindowText(m_hotkey, &hotkey);
        getWindowText(m_text, &text);

        int index = m_list_values.find(hotkey, m_currentGroup);
        if (index == -1 && m_filterMode)
        {
            int index2 = propValues->find(hotkey, m_currentGroup);
            if (index2 != -1)
                propValues->del(index2);
        }

        if (index == -1)
        {
            index = m_list.getOnlySingleSelection() + 1;
            m_list_values.insert(index, hotkey, text, m_currentGroup);
            m_list.addItem(index, 0, hotkey);
            m_list.addItem(index, 1, text);
            m_list.addItem(index, 2, m_currentGroup);
        }
        else
        {
            m_list_values.add(index, hotkey, text, m_currentGroup);
            m_list.setItem(index, 0, hotkey);
            m_list.setItem(index, 1, text);
            m_list.setItem(index, 2, m_currentGroup);
        }

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
            if (m_filterMode)
                m_list_positions.erase(m_list_positions.begin()+index);
        }
        m_deleted = false;
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnReplaceElement(WORD, WORD, HWND, BOOL&)
    {
        tstring hotkey, text;
        getWindowText(m_hotkey, &hotkey);
        getWindowText(m_text, &text);

        int index = m_list.getOnlySingleSelection();
        m_list_values.add(index, hotkey, text, m_currentGroup);
        m_list.setItem(index, 0, hotkey);
        m_list.setItem(index, 1, text);
        m_list.setItem(index, 2, m_currentGroup);
        m_list.SelectItem(index);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnResetData(WORD, WORD, HWND, BOOL&)
    {
        m_update_mode = true;
        m_text.SetWindowText(L"");
        m_hotkey.SetWindowText(L"");
        m_list.SelectItem(-1);
        updateButtons();
        m_hotkey.SetFocus();
        m_update_mode = false;
        return 0;
    }

    LRESULT OnUpElement(WORD, WORD, HWND, BOOL&)
    {
        propertiesUpDown<tstring> ud;
        ud.up(m_list, m_list_values, false);
        return 0;
    }

    LRESULT OnDownElement(WORD, WORD, HWND, BOOL&)
    {
        propertiesUpDown<tstring> ud;
        ud.down(m_list, m_list_values, false);
        return 0;
    }

    LRESULT OnFilter(WORD, WORD, HWND, BOOL&)
    {
        saveValues();
        m_filterMode = m_filter.GetCurSel();
        loadValues();
        update();
        updateButtons();
        m_state_helper.setCanSaveState();
        return 0;
    }

    LRESULT OnGroupChanged(WORD, WORD, HWND, BOOL&)
    {
        tstring group;
        getCurrentGroup(&group);
        if (m_currentGroup == group) return 0;
        tstring pattern;
        getWindowText(m_hotkey, &pattern);
        m_currentGroup = group;
        int index = m_list_values.find(pattern, group);
        if (index != -1)
        {
            m_list.SelectItem(index);
            updateButtons();
            return 0;
        }
        if (m_filterMode && m_list.GetSelectedCount() == 0) {
            loadValues();
            update();
        }
        updateButtons();
        return 0;
    }

    LRESULT OnHotkeyEditChanged(UINT, WPARAM, LPARAM, BOOL&)
    {
        if (m_update_mode)
            return 0;
 
        BOOL currelement = FALSE;
        int len = m_hotkey.GetWindowTextLength();
        int selected = m_list.getOnlySingleSelection();
        if (len > 0)
        {
            tstring hotkey;
            getWindowText(m_hotkey, &hotkey);
            int index = m_list_values.find(hotkey, m_currentGroup);
            if (index != -1 && !currelement)
            {
                m_list.SelectItem(index);
                m_hotkey.SetSel(len, len);
            }
        }
        updateButtons();
        return 0;
    }

    bool updateCurrentItem()
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return false;
        m_update_mode = true;
        tstring hotkey;
        getWindowText(m_hotkey, &hotkey);
        property_value& v = m_list_values.getw(item);
        if (v.key != hotkey) return false;
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
        m_update_mode = false;
		return true;
    }

    LRESULT OnHotkeyTextChanged(WORD, WORD, HWND, BOOL&)
    {
        if (m_update_mode)
           return 0;
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
          const property_value& v = m_list_values.get(item);
          if (v.group != m_currentGroup)
              return 0;
        }
        updateCurrentItem();
        return 0;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        if (m_update_mode)
            return 0;
        m_update_mode = true;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected == 0)
        {
            if (!m_deleted) 
            {
              m_hotkey.SetWindowText(L"");
              m_text.SetWindowText(L"");
            }
        }
        else if (items_selected == 1)
        {
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
            m_hotkey.SetWindowText(L"");
            m_text.SetWindowText(L"");
        }
        updateButtons();
        m_update_mode = false;
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
        m_replace.Attach(GetDlgItem(IDC_BUTTON_REPLACE));
        m_reset.Attach(GetDlgItem(IDC_BUTTON_RESET));
        m_up.Attach(GetDlgItem(IDC_BUTTON_UP));
        m_down.Attach(GetDlgItem(IDC_BUTTON_DOWN));
        m_filter.Attach(GetDlgItem(IDC_COMBO_FILTER));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(L"Hotkey", 20);
        m_list.addColumn(L"Текст", 60);
        m_list.addColumn(L"Группа", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);        
        m_list.setKeyDownMessageHandler(m_hWnd, WM_USER+2);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));
        m_state_helper.init(dlg_state, &m_list);

        m_state_helper.loadGroupAndFilter(m_currentGroup, m_filterMode);

        m_filter.AddString(L"Все группы");
        m_filter.AddString(L"Текущая группа");
        m_filter.AddString(L"Активные группы");
        m_filter.SetCurSel(m_filterMode);

        loadValues();
        updateButtons();
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
            int pos = m_cbox.AddString(g.key.c_str());
            if (g.key == m_currentGroup)
                { current_index = pos; }
        }
        m_cbox.SetCurSel(current_index);
        getCurrentGroup(&m_currentGroup);

        m_list.DeleteAllItems();
        for (int i=0,e=m_list_values.size(); i<e; ++i)
        {
            const property_value& v = m_list_values.get(i);
            m_list.addItem(i, 0, v.key);
            m_list.addItem(i, 1, v.value);
            m_list.addItem(i, 2, v.group);
        }

        m_state_helper.loadCursorAndTopPos(2);
    }

    void updateButtons()
    {
         if (m_filterMode == 2 && m_cbox.GetCount() == 0) {
            m_add.EnableWindow(FALSE);
            m_del.EnableWindow(FALSE);
            m_up.EnableWindow(FALSE);
            m_down.EnableWindow(FALSE);
            m_replace.EnableWindow(FALSE);
            m_reset.EnableWindow(FALSE);
            return;
        }

        m_reset.EnableWindow(TRUE);
        bool pattern_empty = m_hotkey.GetWindowTextLength() == 0;
        bool text_empty = m_text.GetWindowTextLength() == 0;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected == 0)
        {
            m_add.EnableWindow(pattern_empty ? FALSE : TRUE);
            m_del.EnableWindow(FALSE);
            m_up.EnableWindow(FALSE);
            m_down.EnableWindow(FALSE);
            m_replace.EnableWindow(FALSE);
        }
        else if(items_selected == 1)
        {
            m_del.EnableWindow(TRUE);
            m_up.EnableWindow(TRUE);
            m_down.EnableWindow(TRUE);
            bool mode = FALSE;
            if (!pattern_empty)
            {
                int selected = m_list.getOnlySingleSelection();
                const property_value& v = m_list_values.get(selected);
                mode = TRUE;
                if (m_currentGroup == v.group)
                {
                    tstring pattern;
                    getWindowText(m_hotkey, &pattern);
                    mode = (pattern == v.key) ? FALSE : TRUE;
                }
            }
            m_replace.EnableWindow(mode);
            m_add.EnableWindow(mode);
        }
        else
        {
            m_add.EnableWindow(FALSE);
            m_del.EnableWindow(TRUE);
            m_up.EnableWindow(TRUE);
            m_down.EnableWindow(TRUE);
            m_replace.EnableWindow(FALSE);
        }
    }

    void swapItems(int index1, int index2)
    {
        const property_value& i1 = m_list_values.get(index1);
        const property_value& i2 = m_list_values.get(index2);
        m_list.setItem(index1, 0, i2.key);
        m_list.setItem(index1, 1, i2.value);
        m_list.setItem(index1, 2, i2.group);
        m_list.setItem(index2, 0, i1.key);
        m_list.setItem(index2, 1, i1.value);
        m_list.setItem(index2, 2, i1.group);
        m_list_values.swap(index1, index2);
    }

    void loadValues()
    {
        m_loadedGroup = m_currentGroup;
        if (!m_filterMode)
        {
            m_list_values = *propValues;
            return;
        }

        PropertiesGroupFilter gf(propGroups);
        m_list_values.clear();
        m_list_positions.clear();
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v = propValues->get(i);
            if (m_filterMode == 1) {
                if (v.group != m_currentGroup)
                continue;
            }
            else if (m_filterMode == 2) {
                if (!gf.isGroupActive(v.group))
                    continue;
            }
            else {
                assert(false);
            }
            m_list_values.add(-1, v.key, v.value, v.group);
            m_list_positions.push_back(i);
        }
    }

    void saveValues()
    {
        if (!m_state_helper.save(m_loadedGroup, m_filterMode))
            return;

        if (!m_filterMode)
        {
            *propValues = m_list_values;
            return;
        }

        PropertiesGroupFilter gf(propGroups);
        std::vector<int> todelete;
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v = propValues->get(i);
            bool filter = false;
            if (m_filterMode == 1) {
                filter = (v.group == m_loadedGroup);
            } else if (m_filterMode == 2) {
                filter = gf.isGroupActive(v.group);
            } else {
                assert(false);
            }
            if (filter)
            {
                bool exist = std::find(m_list_positions.begin(), m_list_positions.end(), i) != m_list_positions.end();
                if (!exist)
                    todelete.push_back(i);
            }
        }
        for (int i=todelete.size()-1; i>=0; --i)
        {
            int pos = todelete[i];
            propValues->del(pos);
            for (int j=0,je=m_list_positions.size();j<je;++j) {
                if (m_list_positions[j] > pos)
                    m_list_positions[j]--;
            }
        }

        todelete.clear();
        int pos_count = m_list_positions.size();
        int elem_count = m_list_values.size();
        for (int i=0; i<elem_count; ++i)
        {
            int index = (i < pos_count) ? m_list_positions[i] : -1;
            const property_value& v = m_list_values.get(i);
            int pos = propValues->find(v.key, v.group);
            if (pos != -1 && pos != index)
                todelete.push_back(pos);
            propValues->add(index, v.key, v.value, v.group);
        }
        for (int i=todelete.size()-1; i>=0; --i)
        {
            int pos = todelete[i];
            propValues->del(pos);
        }
    }

    int getGroupIndex(const tstring& group)
    {
        int count = m_cbox.GetCount();
        MemoryBuffer mb;
        for (int i=0; i<count; ++i)
        {
            int len = m_cbox.GetLBTextLen(i) + 1;
            mb.alloc(len * sizeof(tchar));
            tchar* buffer = reinterpret_cast<tchar*>(mb.getData());
            m_cbox.GetLBText(i, buffer);
            tstring name(buffer);
            if (group == name)
                return i;
        }
        return -1;
    }

    void getCurrentGroup(tstring *group)
    {
        int index = m_cbox.GetCurSel();
        if (index == -1) return;
        int len = m_cbox.GetLBTextLen(index) + 1;
        WCHAR *buffer = new WCHAR[len];
        m_cbox.GetLBText(index, buffer);
        group->assign(buffer);
        delete[]buffer;
    }
};
