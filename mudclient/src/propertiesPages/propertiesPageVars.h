#pragma once

class PropertyVars :  public CDialogImpl<PropertyVars>
{
    PropertiesValues *propValues;
    PropertiesValues m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_pattern;
    CEdit m_value;
    CButton m_add;
    CButton m_del;
    CButton m_replace;
    CButton m_reset;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;
    bool m_deleted;

public:
     enum { IDD = IDD_PROPERTY_VARS };
     PropertyVars() : m_update_mode(false), m_deleted(false) {}
     void setParams(PropertiesValues *values, PropertiesDlgPageState *state)
     {
         propValues = values;
         dlg_state = state;
     }

     bool updateChangedTemplate(bool check)
     {
         int item = m_list.getOnlySingleSelection();
         if (item != -1)
         {
             tstring pattern;
             getWindowText(m_pattern, &pattern);
             const property_value& v = m_list_values.get(item);
             if (v.key != pattern && !pattern.empty())
             {
                 if (!check)
                    updateCurrentItem(true);
                 return true;
             }
         }
         return false;
     }

private:
    BEGIN_MSG_MAP(PropertyTabwords)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER, OnSetFocus)
       MESSAGE_HANDLER(WM_USER+1, OnKeyDown)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_REPLACE, OnReplaceElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_RESET, OnResetData)
       COMMAND_HANDLER(IDC_EDIT_PATTERN, EN_CHANGE, OnPatternEditChanged)
       COMMAND_HANDLER(IDC_EDIT_VARVALUE, EN_CHANGE, OnPatternValueChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
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
        if (wparam == VK_INSERT)
        {
            BOOL b = FALSE;
            OnResetData(0, 0, 0, b);
            return 1;
        }
        return 0;
    }

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring pattern, value;
        getWindowText(m_pattern, &pattern);
        getWindowText(m_value, &value);

        int index = m_list_values.find(pattern);
        m_list_values.add(index, pattern, value, L"");

        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, pattern);
            m_list.setItem(pos, 1, value);
        }
        else
        {
            m_list.setItem(index, 0, pattern);
            m_list.setItem(index, 1, value);
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

    LRESULT OnReplaceElement(WORD, WORD, HWND, BOOL&)
    {
        updateCurrentItem(true);
        m_list.SetFocus();
        return 0;
    }
    
    LRESULT OnResetData(WORD, WORD, HWND, BOOL&)
    {
        m_pattern.SetWindowText(L"");
        m_value.SetWindowText(L"");
        m_list.SelectItem(-1);
        m_pattern.SetFocus();
        return 0;
    }

    LRESULT OnPatternEditChanged(WORD, WORD, HWND, BOOL&)
    {
        if (m_update_mode)
            return 0;

        BOOL currelement = FALSE;
        int len = m_pattern.GetWindowTextLength();
        int selected = m_list.getOnlySingleSelection();
        if (len > 0)
        {
            tstring pattern;
            getWindowText(m_pattern, &pattern);
            int index = m_list_values.find(pattern);
            currelement = (index != -1 && index == selected) ? TRUE : FALSE;
            if (index != -1 && !currelement)
            {
                m_list.SelectItem(index);
                m_pattern.SetSel(len, len);
            }
        }
        updateButtons();
        return 0;
    }

    LRESULT OnPatternValueChanged(WORD, WORD, HWND, BOOL&)
    {        
        if (m_update_mode)
           return 0;
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
          updateCurrentItem(false);
        }        
        return 0;
    }

    void updateCurrentItem(bool update_key)
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;
        m_update_mode = true;
        tstring pattern, value;
        getWindowText(m_pattern, &pattern);
        getWindowText(m_value, &value);
        property_value& v = m_list_values.getw(item);
        if (v.key != pattern) 
        {
            if (!update_key) { m_update_mode = false; return; }
            v.key = pattern;
            v.value = value;
            m_list.setItem(item, 0, pattern);
        }
        if (v.value != value)
        {
            v.value = value;
            m_list.setItem(item, 1, value);
        }
        m_update_mode = false;
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
               m_pattern.SetWindowText(L"");
               m_value.SetWindowText(L"");
            }
        }
        else if (items_selected == 1)
        {
            int item = m_list.getOnlySingleSelection();
            const property_value &v = m_list_values.get(item);
            m_pattern.SetWindowText( v.key.c_str() );
            m_value.SetWindowText( v.value.c_str() );
        }
        else
        {
            m_pattern.SetWindowText( L"" );
            m_value.SetWindowText( L"" );
        }
        updateButtons();
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
            m_pattern.SetWindowText(L"");
            m_value.SetWindowText(L"");
            m_update_mode = false;
            update();
            PostMessage(WM_USER); // OnSetFocus to list
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
        m_pattern.Attach(GetDlgItem(IDC_EDIT_PATTERN));
        m_value.Attach(GetDlgItem(IDC_EDIT_VARVALUE));
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_reset.Attach(GetDlgItem(IDC_BUTTON_RESET));
        m_replace.Attach(GetDlgItem(IDC_BUTTON_REPLACE));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(L"Имя", 30);
        m_list.addColumn(L"Значение", 70);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);       
        m_list.setKeyDownMessageHandler(m_hWnd, WM_USER+1);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_state_helper.init(dlg_state, &m_list);
        loadValues();
        updateButtons();
        m_reset.EnableWindow(TRUE);
        return 0;
    }

    LRESULT OnCloseDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        saveValues();
        return 0;
    }

    void update()
    {
        m_list.DeleteAllItems();
        for (int i=0,e=m_list_values.size(); i<e; ++i)
        {
            const property_value& v = m_list_values.get(i);
            m_list.addItem(i, 0, v.key);
            m_list.setItem(i, 1, v.value);
        }

        m_state_helper.loadCursorAndTopPos(-1);
    }

    void updateButtons()
    {
        bool pattern_empty = m_pattern.GetWindowTextLength() == 0;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected == 0)
        {
            m_add.EnableWindow(pattern_empty ? FALSE : TRUE);
            m_del.EnableWindow(FALSE);
            m_replace.EnableWindow(FALSE);
        }
        else if(items_selected == 1)
        {
            m_del.EnableWindow(TRUE);
            bool mode = FALSE;
            if (!pattern_empty)
            {
                tstring pattern;
                getWindowText(m_pattern, &pattern);
                int selected = m_list.getOnlySingleSelection();
                const property_value& v = m_list_values.get(selected);
                mode = (pattern == v.key) ? FALSE : TRUE;
            }
            m_replace.EnableWindow(mode);
            m_add.EnableWindow(mode);
        }
        else
        {
            m_add.EnableWindow(FALSE);
            m_del.EnableWindow(TRUE);
            m_replace.EnableWindow(FALSE);
        }
    }

    void loadValues()
    {
        std::map<tstring,int> sort;
        for (int i=0,e=propValues->size(); i<e; ++i) {
            const property_value& v = propValues->get(i);
            sort[v.key] = i;
        }
        m_list_values.clear();
        std::map<tstring,int>::iterator it = sort.begin(), it_end = sort.end();
        for (; it!=it_end; ++it) {
            const property_value& v = propValues->get(it->second);
            m_list_values.add(-1, v.key, v.value, L"");
        }
    }

    void saveValues()
    {
        if (!m_state_helper.save(L"", false))
            return;
        propValues->clear();
        for (int i=0,e=m_list_values.size(); i<e; ++i) {
            const property_value& v = m_list_values.get(i);
            propValues->add(-1, v.key, v.value, L"");
        }
    }
};
