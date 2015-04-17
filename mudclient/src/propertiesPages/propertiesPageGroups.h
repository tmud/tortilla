#pragma once

class PropertyGroups :  public CDialogImpl<PropertyGroups>
{
    PropertiesData *propData;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_group;
    CButton m_add;
    CButton m_del;
    CButton m_onoff;
    CButton m_rename;
    tstring m_OnStatus;
    bool m_deleted;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
     enum { IDD = IDD_PROPERTY_GROUPS };
     PropertyGroups(PropertiesData *data) : propData(data), m_OnStatus(L"Вкл"), m_deleted(false),dlg_state(NULL) {}
     void setParams(PropertiesDlgPageState *state)
     {
         dlg_state = state;
     }

private:
    BEGIN_MSG_MAP(PropertyGroups)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_USER, OnSetFocus)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_CHECK_GROUP_ON, OnGroupChecked)
       COMMAND_ID_HANDLER(IDC_BUTTON_RENAME, OnRenameElement)
       COMMAND_HANDLER(IDC_EDIT_GROUP, EN_CHANGE, OnEditChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring group;
        getWindowText(m_group, &group);

        int index = propData->groups.find(group);
        propData->groups.add(index, group, L"1", L"");

        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, group);
            m_list.addItem(pos, 1, m_OnStatus);
        }
        else
        {
            m_list.setItem(index, 0, group);
            m_list.setItem(index, 1, m_OnStatus);
        }
        if (index == -1)
            index = m_list.GetItemCount()-1;
        m_list.SelectItem(index);
        m_onoff.SetCheck(BST_CHECKED);
        m_rename.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnDeleteElement(WORD, WORD, HWND, BOOL&)
    {
        tstring title;
        loadString(IDR_MAINFRAME, &title);
        if (propData->groups.size() == 1)
        {
            MessageBox(L"Последнюю группу удалить нельзя!", title.c_str(), MB_OK|MB_ICONSTOP);
            return 0;
        }
        if (MessageBox(L"Вы уверены, что хотите удалить группу со всеми ее триггерами, заменами и другими элементами?", 
            title.c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            m_deleted = true;
            int index = m_list.GetSelectedIndex();
            m_list.DeleteItem(index);
            const property_value& g = propData->groups.get(index);
            propData->deleteGroup(g.key);
            m_deleted = false;
        }
        return 0;
    }

    LRESULT OnRenameElement(WORD, WORD, HWND, BOOL&)
    {
        tstring group;
        getWindowText(m_group, &group);

        int index = m_list.GetSelectedIndex();
        const property_value& g = propData->groups.get(index);
        tstring oldname(g.key);
        propData->renameGroup(oldname, group);

        m_list.setItem(index, 0, group);
        return 0;
    }

    LRESULT OnGroupChecked(WORD, WORD, HWND, BOOL&)
    {
        tstring state = (m_onoff.GetCheck() == BST_CHECKED) ? L"1" : L"0";
        int index = m_list.GetSelectedIndex();
        const property_value& g = propData->groups.get(index);
        tstring key(g.key);
        propData->groups.add(index, key, state, L"");
        setListEnableItem(index);
        return 0;
    }

    LRESULT OnEditChanged(WORD, WORD, HWND, BOOL&)
    {
        int string_len = m_group.GetWindowTextLength();
        BOOL flag = string_len == 0 ? FALSE : TRUE;
        m_add.EnableWindow(flag);
        if (flag)
        {
            int index = m_list.GetSelectedIndex();
            if (index == -1)
                flag = FALSE;
            else
            {
                tstring group;
                getWindowText(m_group, &group);
                for (int i=0,e=propData->groups.size(); i<e; ++i)
                {
                    //if ( i == index) continue;
                    const property_value& g = propData->groups.get(i);
                    if (g.key == group)
                        { flag = FALSE; break; }
                }
            }
            m_rename.EnableWindow(flag);
        }
        return 0;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected == -1)
        {
            m_del.EnableWindow(FALSE);
            m_onoff.SetCheck(BST_UNCHECKED);
            m_onoff.EnableWindow(FALSE);
            if (!m_deleted)
                m_group.SetWindowText(L"");
        }
        else
        {
            m_del.EnableWindow(TRUE);
            m_onoff.EnableWindow(TRUE);
            m_onoff.SetCheck(checkEnableItem(item_selected) ? BST_CHECKED : BST_UNCHECKED);
            const property_value& g = propData->groups.get(item_selected);
            m_group.SetWindowText(g.key.c_str());
        }
        m_rename.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnListKillFocus(int , LPNMHDR , BOOL&)
    {
        HWND focus = GetFocus();
        if (focus != m_del && focus != m_onoff)
            m_del.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnShowWindow(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        if (wparam)
        {
            PostMessage(WM_USER); // OnSetFocus to list
            m_state_helper.setCanSaveState();
            m_state_helper.loadCursorAndTopPos(-1);
        }
        return 0;
    }

    LRESULT OnCloseDialog(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        m_state_helper.save(L"", false);
        return 0;
    }

    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_group.Attach(GetDlgItem(IDC_EDIT_GROUP));
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_onoff.Attach(GetDlgItem(IDC_CHECK_GROUP_ON));
        m_rename.Attach(GetDlgItem(IDC_BUTTON_RENAME));
        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.addColumn(L"Группа", 50);
        m_list.addColumn(L"Статус", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_add.EnableWindow(FALSE);
        m_del.EnableWindow(FALSE);
        m_onoff.EnableWindow(FALSE);
        m_rename.EnableWindow(FALSE);
        m_state_helper.init(dlg_state, &m_list);
        for (int i=0,e=propData->groups.size(); i<e; ++i)
        {
            const property_value& g = propData->groups.get(i);
            m_list.addItem(i, 0, g.key);
            setListEnableItem(i);
        }
        return 0;
    }

    bool checkEnableItem(int index)
    {
        const property_value& g = propData->groups.get(index);
        const tstring& v = g.value;
        bool enable_group = true;
        if ( v.empty() || v == L"0" )
            enable_group = false;
        return enable_group;
    }

    void setListEnableItem(int index)
    {
        m_list.addItem(index, 1, checkEnableItem(index) ? m_OnStatus : L"");
    }
};
