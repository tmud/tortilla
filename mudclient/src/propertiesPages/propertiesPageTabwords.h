#pragma once

class PropertyTabwords :  public CDialogImpl<PropertyTabwords>
{
    PropertiesList *propValues;
    PropertiesList m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_pattern;
    CButton m_add;
    CButton m_del;
    CButton m_replace;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
     enum { IDD = IDD_PROPERTY_TABWORDS };
     PropertyTabwords() : m_update_mode(false) {}
     void setParams(PropertiesList *values, PropertiesDlgPageState *state)
     {
         propValues = values;
         dlg_state = state;
     }

private:
    BEGIN_MSG_MAP(PropertyTabwords)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER, OnSetFocus)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_REPLACE, OnReplaceElement)
       COMMAND_HANDLER(IDC_EDIT_PATTERN, EN_CHANGE, OnPatternEditChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring pattern;
        getWindowText(m_pattern, &pattern);

        int index = m_list_values.find(pattern);
        m_list_values.add(index, pattern);

        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, pattern);
        }
        else
        {
            m_list.setItem(index, 0, pattern);
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
        for (int i = 0; i < items; ++i)
        {
            int index = selected[i];
            m_list.DeleteItem(index);
            m_list_values.del(index);
        }
        return 0;
    }

    LRESULT OnReplaceElement(WORD, WORD, HWND, BOOL&)
    {
        updateCurrentItem(true);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnPatternEditChanged(WORD, WORD, HWND, BOOL&)
    {
        if (!m_update_mode)
        {
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
                    return 0;
                }
                m_replace.EnableWindow(len > 0 && selected >= 0 && !currelement);
                m_add.EnableWindow(len == 0 ? FALSE : !currelement);
                if (currelement)
                    updateCurrentItem(false);
            }
        }
        return 0;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        m_update_mode = true;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected == 0)
        {
            m_del.EnableWindow(FALSE);
            m_pattern.SetWindowText(L"");
        }
        else if (items_selected == 1)
        {
            m_add.EnableWindow(FALSE);
            m_del.EnableWindow(TRUE);
            int item = m_list.getOnlySingleSelection();
            const tstring &v = m_list_values.get(item);
            m_pattern.SetWindowText( v.c_str() );
        }
        else
        {
            m_del.EnableWindow(TRUE);
            m_add.EnableWindow(FALSE);
            m_pattern.SetWindowText( L"" );
        }
        m_replace.EnableWindow(FALSE);
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
            m_update_mode = false;
            update();
            PostMessage(WM_USER); // OnSetFocus to list
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
        m_pattern.Attach(GetDlgItem(IDC_EDIT_PATTERN));
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_replace.Attach(GetDlgItem(IDC_BUTTON_REPLACE));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(L"Ключевые слова", 90);        
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);       
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_add.EnableWindow(FALSE);
        m_del.EnableWindow(FALSE);
        m_replace.EnableWindow(FALSE);
        m_state_helper.init(dlg_state, &m_list);
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
        m_list.DeleteAllItems();
        for (int i=0,e=m_list_values.size(); i<e; ++i)
        {
            const tstring& v = m_list_values.get(i);
            m_list.addItem(i, 0, v);
        }

        int index = -1;
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        if (!pattern.empty())
            index = m_list_values.find(pattern);
        m_state_helper.loadCursorAndTopPos(index);
    }

    void updateCurrentItem(bool update_key)
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;
        m_update_mode = true;
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        tstring& v = m_list_values.getw(item);
        if (v != pattern) 
        {
            if (!update_key) { m_update_mode = false; return; }
            v = pattern;
            m_list.setItem(item, 0, pattern);
        }
        m_update_mode = false;
    }

    void loadValues()
    {
        m_list_values.clear();
        for (int i=0,e=propValues->size(); i<e; ++i) {
            m_list_values.add(-1, propValues->get(i));
        }
    }

    void saveValues()
    {
        if (!m_state_helper.save(L"", false))
            return;
        propValues->clear();
        for (int i=0,e=m_list_values.size(); i<e; ++i) {
            propValues->add(-1, m_list_values.get(i));
        }
    }
};
