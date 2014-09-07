#pragma once

struct PropertyTwoConfig
{
    PropertyTwoConfig() : use_priority(false) {}
    tstring title;
    tstring label1;
    tstring label2;
    tstring list1;
    tstring list2;
    bool use_priority;
};

class PropertyTwoParams :  public CDialogImpl<PropertyTwoParams>
{
    PropertiesValues *propValues;
    PropertiesValues *propGroups;
    PropertiesValues m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_pattern;
    CEdit m_text;
    CButton m_add;
    CButton m_del;
    CButton m_up;
    CButton m_down;
    CButton m_filter;
    CComboBox m_cbox;
    PropertyTwoConfig m_config;
    bool m_filterMode;
    tstring m_currentGroup;
    bool m_deleted;
    bool m_update_mode;

public:
     enum { IDD = IDD_PROPERTY_TWOPARAMS };
     PropertyTwoParams() : propValues(NULL), propGroups(NULL), m_filterMode(false), m_deleted(false), m_update_mode(false) {}
     void setParams(PropertiesValues *values, PropertiesValues *groups, const PropertyTwoConfig& cfg)
     {
         propValues = values;
         propGroups = groups;
         m_config = cfg;
     }

private:
    BEGIN_MSG_MAP(PropertyTwoParams)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_UP, OnUpElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DOWN, OnDownElement)
       COMMAND_ID_HANDLER(IDC_CHECK_GROUP_FILTER, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged);
       COMMAND_HANDLER(IDC_EDIT_PATTERN, EN_CHANGE, OnPatternEditChanged)
       COMMAND_HANDLER(IDC_EDIT_PATTERN_TEXT, EN_CHANGE, OnPatternTextChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring pattern, text;
        getWindowText(m_pattern, &pattern);
        getWindowText(m_text, &text);        

        int index = m_list_values.find(pattern);
        if (index == -1 && m_filterMode)
        {
            int index2 = propValues->find(pattern);
            if (index2 != -1)
                propValues->del(index2);
        }

        m_list_values.add(index, pattern, text, m_currentGroup);
        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, pattern);
            m_list.addItem(pos, 1, text);
            m_list.addItem(pos, 2, m_currentGroup);
        }
        else
        {
            m_list.setItem(index, 0, pattern);
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
    
    LRESULT OnUpElement(WORD, WORD, HWND, BOOL&)
    {
        int index = m_list.getOnlySingleSelection();
        if (index > 0)
        {
            swapItems(index, index-1);
            m_list.SelectItem(index-1);
            m_list.SetFocus();
        }
        return 0;
    }

    LRESULT OnDownElement(WORD, WORD, HWND, BOOL&)
    {
        int index = m_list.getOnlySingleSelection();
        int last = m_list.GetItemCount() - 1;
        if (index >= 0 && index != last)
        {
            swapItems(index, index+1);
            m_list.SelectItem(index+1);
            m_list.SetFocus();
        }
        return 0;
    }

    LRESULT OnFilter(WORD, WORD, HWND, BOOL&)
    {
        saveValues();
        m_filterMode = m_filter.GetCheck() ? true : false;
        loadValues();
        m_up.EnableWindow(FALSE);
        m_down.EnableWindow(FALSE);
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
        m_up.EnableWindow(FALSE);
        m_down.EnableWindow(FALSE);        
        update();        
        return 0;
    }

    LRESULT OnPatternEditChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_update_mode)
        {
            int len = m_pattern.GetWindowTextLength();
            m_add.EnableWindow(len == 0 ? FALSE : TRUE);
            if (len > 0)
            {
                tstring pattern;
                getWindowText(m_pattern, &pattern);
                int index = m_list_values.find(pattern);
                if (index != -1)
                {
                    m_list.SelectItem(index);
                    m_pattern.SetSel(len, len);
                }
            }
        }
        return 0;
    }

    void updateCurrentItem()
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;      
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        property_value& v = m_list_values.getw(item);
        if (v.key != pattern) return;
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

    LRESULT OnPatternTextChanged(WORD, WORD, HWND, BOOL&)
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
            m_up.EnableWindow(FALSE);
            m_down.EnableWindow(FALSE);
            if (!m_deleted)
            {
                m_pattern.SetWindowText(L"");
                m_text.SetWindowText(L"");
            }
        }
        else if (items_selected == 1)
        {
            m_del.EnableWindow(TRUE);
            m_up.EnableWindow(TRUE);
            m_down.EnableWindow(TRUE);
            int item = m_list.getOnlySingleSelection();
            const property_value& v = m_list_values.get(item);
            m_pattern.SetWindowText( v.key.c_str() );
            m_text.SetWindowText( v.value.c_str() );
            int index = getGroupIndex(v.group);
            m_cbox.SetCurSel(index);
            m_currentGroup = v.group;            
        }
        else
        {
            m_del.EnableWindow(TRUE);
            m_add.EnableWindow(FALSE);
            m_up.EnableWindow(FALSE);
            m_down.EnableWindow(FALSE);
            m_pattern.SetWindowText(L"");
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
            m_pattern.SetWindowText(L"");
            m_text.SetWindowText(L"");
            update();            
        }
        else
        {
            m_del.EnableWindow(FALSE);
            saveValues();
        }
        return 0;
    } 
    
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        CStatic t(GetDlgItem(IDC_STATIC_TITLE)); t.SetWindowText(m_config.title.c_str());
        CStatic s1(GetDlgItem(IDC_STATIC_LABEL1)); s1.SetWindowText(m_config.label1.c_str());
        CStatic s2(GetDlgItem(IDC_STATIC_LABEL2)); s2.SetWindowText(m_config.label2.c_str());
        m_pattern.Attach(GetDlgItem(IDC_EDIT_PATTERN));
        m_text.Attach(GetDlgItem(IDC_EDIT_PATTERN_TEXT));
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_up.Attach(GetDlgItem(IDC_BUTTON_UP));
        m_down.Attach(GetDlgItem(IDC_BUTTON_DOWN));
        m_filter.Attach(GetDlgItem(IDC_CHECK_GROUP_FILTER));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(m_config.list1, 40);
        m_list.addColumn(m_config.list2, 40);
        m_list.addColumn(L"Ãðóïïà", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);       
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_add.EnableWindow(FALSE);
        m_del.EnableWindow(FALSE);
        m_up.EnableWindow(FALSE);
        m_down.EnableWindow(FALSE);
        if (!m_config.use_priority)
        {
            m_up.ShowWindow(SW_HIDE);
            m_down.ShowWindow(SW_HIDE);
        }
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
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        if (!pattern.empty())
            index = m_list_values.find(pattern);
        m_list.SelectItem(index);                    
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
