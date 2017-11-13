#pragma once

#include "propertiesSaveHelper.h"

class PropertyTimers:  public CDialogImpl<PropertyTimers>
{
    PropertiesValues *propValues;
    PropertiesValues *propGroups;
    TimerValues m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_number;
    CEdit m_pattern;
    CEdit m_text;
    CButton m_del;
    CButton m_filter;
    CComboBox m_cbox;
    bool m_filterMode;
    tstring m_currentGroup;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
     enum { IDD = IDD_PROPERTY_TIMERS };
     PropertyTimers() : propValues(NULL), propGroups(NULL), m_filterMode(false), m_update_mode(false), dlg_state(NULL) {}
     void setParams(PropertiesValues *values, PropertiesValues *groups, PropertiesDlgPageState *state)
     {
         propValues = values;
         propGroups = groups;
         dlg_state = state;
     }

private:
    BEGIN_MSG_MAP(PropertyTimers)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER, OnSetFocus)
       MESSAGE_HANDLER(WM_USER+1, OnKeyDown)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_CHECK_GROUP_FILTER, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged);
       COMMAND_HANDLER(IDC_EDIT_PATTERN, EN_CHANGE, OnDataEditChanged)
       COMMAND_HANDLER(IDC_EDIT_PATTERN_TEXT, EN_CHANGE, OnDataEditChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
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

    LRESULT OnDeleteElement(WORD, WORD, HWND, BOOL&)
    {
        tstring key;
        getWindowText(m_number, &key);
        int index = m_list_values.find(key);
        PropertiesTimer t;
        t.timer = L"0";
        m_list_values.add(index, key, t, L"");
        m_list.setItem(index, 0, key);
        m_list.setItem(index, 1, t.timer);
        m_list.setItem(index, 2, t.cmd);
        m_list.setItem(index, 3, L"");
        m_del.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnFilter(WORD, WORD, HWND, BOOL&)
    {
        saveValues();
        m_filterMode = m_filter.GetCheck() ? true : false;
        loadValues();
        update();
        m_state_helper.setCanSaveState();
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
            m_state_helper.setCanSaveState();
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
        m_state_helper.setCanSaveState();
        return 0;
    }

    LRESULT OnDataEditChanged(WORD, WORD, HWND, BOOL&)
    {
        if (m_update_mode)
            return 0;
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return 0;
        tstring timer, cmd;
        getWindowText(m_pattern, &timer);
        double delay = 0;
        w2double(timer, &delay);

        PropertiesTimer pt;
        pt.setTimer(delay);
        timer.assign(pt.timer);

        getWindowText(m_text, &cmd);
        timer_value& v = m_list_values.getw(item);
        PropertiesTimer& t = v.value;
        t.timer = timer;
        t.cmd = cmd;
        updateCurrentItem();
        return 0;
    }

    void updateCurrentItem()
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;
        timer_value& v = m_list_values.getw(item);
        PropertiesTimer& t = v.value;

        tstring timer, cmd;
        m_list.getItemText(item, 1, &timer);
        m_list.getItemText(item, 2, &cmd);
        if (t.timer != timer)
            m_list.setItem(item, 1, t.timer);
        if (t.cmd != cmd)
            m_list.setItem(item, 2, t.cmd);
        if (v.group != m_currentGroup)
        {
            v.group = m_currentGroup;
            m_list.setItem(item, 3, m_currentGroup);
        }
        else
        {
            tstring group;
            m_list.getItemText(item, 3, &group);
            if (group != m_currentGroup)
                m_list.setItem(item, 3, m_currentGroup);
        }
        m_del.EnableWindow(TRUE);
    }

    void disableControls()
    {
       m_update_mode = true;
       m_del.EnableWindow(FALSE);
       m_number.SetWindowText(L"");
       m_pattern.SetWindowText(L"");
       m_pattern.EnableWindow(FALSE);
       m_text.SetWindowText(L"");
       m_text.EnableWindow(FALSE);
       m_update_mode = false;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        m_update_mode = true;
        int items_selected = m_list.GetSelectedCount();
        if (items_selected != 1)
        {
            disableControls();
        }
        else
        {
            int item = m_list.getOnlySingleSelection();
            const timer_value& v = m_list_values.get(item);
            m_number.SetWindowText( v.key.c_str() );
            const PropertiesTimer& t = v.value;
            m_pattern.SetWindowText( t.timer.c_str() );
            m_pattern.EnableWindow(TRUE);
            m_text.SetWindowText( t.cmd.c_str() );
            m_text.EnableWindow(TRUE);
            if (!v.group.empty())
            {
                int index = getGroupIndex(v.group);
                m_cbox.SetCurSel(index);
                m_currentGroup = v.group;
            }
            m_del.EnableWindow( TRUE );
        }
        m_update_mode = false;
        return 0;
    } 

    LRESULT OnShowWindow(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        if (wparam)
        {
            loadValues();
            m_update_mode = true;
            m_pattern.SetWindowText(L"");
            m_text.SetWindowText(L"");
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
        m_number.Attach(GetDlgItem(IDC_EDIT_NUMBER));
        m_pattern.Attach(GetDlgItem(IDC_EDIT_PATTERN));
        m_pattern.SetLimitText(5);
        m_text.Attach(GetDlgItem(IDC_EDIT_PATTERN_TEXT));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));        
        m_filter.Attach(GetDlgItem(IDC_CHECK_GROUP_FILTER));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));
        m_list.Attach(GetDlgItem(IDC_LIST));

        m_list.addColumn(L"Номер", 20);
        m_list.addColumn(L"Интервал", 20);
        m_list.addColumn(L"Действие", 40);
        m_list.addColumn(L"Группа", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);       
        m_list.setKeyDownMessageHandler(m_hWnd, WM_USER+1);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_del.EnableWindow(FALSE);
        m_pattern.EnableWindow(FALSE);
        m_text.EnableWindow(FALSE);
        m_state_helper.init(dlg_state, &m_list);

        m_filterMode = false;
        int dummy = 0;
        m_state_helper.loadGroupAndFilter(m_currentGroup, dummy);
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
            const timer_value& v = m_list_values.get(i);
            const PropertiesTimer& t = v.value;
            m_list.addItem(i, 0, v.key);
            m_list.addItem(i, 1, t.timer);
            m_list.addItem(i, 2, t.cmd);
            m_list.addItem(i, 3, v.group);
        }

        m_state_helper.loadCursorAndTopPos(3);
    }

    void loadValues()
    {
        TimerValues tmp;
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v= propValues->get(i);
            if (m_filterMode && v.group != m_currentGroup)
                continue;
            PropertiesTimer t;
            t.convertFromString(v.value);
            tmp.add(-1, v.key, t, v.group);
        }

        m_list_values.clear();
        for (int i=1; i<=TIMERS_COUNT; ++i)
        {
            WCHAR buffer[8];
            _itow(i, buffer, 10);
            int pos = tmp.find(buffer);

            if (m_filterMode && pos == -1)
               continue;
            if (pos != -1)
            {
               const timer_value &t = tmp.get(pos);
               //if (!t.value.cmd.empty())
               {
                   m_list_values.add(-1, t.key, t.value, t.group);
                   continue;
               }
            }

            PropertiesTimer t;
            t.timer = L"0";
            m_list_values.add(-1, buffer, t, L"");
        }
    }

    void saveValues()
    {
        if (!m_state_helper.save(m_currentGroup, m_filterMode))
            return;

        if (!m_filterMode)
        {
            propValues->clear();
            for (int i=0,e=m_list_values.size(); i<e; ++i)
            {
                const timer_value &t = m_list_values.get(i);
                if (t.value.cmd.empty() || t.group.empty())
                    continue;
                tstring value;
                t.value.convertToString(&value);
                propValues->add(-1, t.key, value, t.group);
            }
            return;
        }

        std::vector<int> positions;
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v = propValues->get(i);
            if (v.group == m_currentGroup)
                positions.push_back(i);
        }

        std::vector<int> todelete;
        int elem_count = m_list_values.size();
        for (int i=0; i<elem_count; ++i)
        {
            int pos = positions[i];            
            const timer_value& t = m_list_values.get(i);
            if (t.value.cmd.empty() || t.group.empty())
            {
                todelete.push_back(pos);
                continue;
            }
            
            tstring value;
            t.value.convertToString(&value);
            propValues->add(pos, t.key, value, t.group);
        }

        int ts = todelete.size()-1;
        for (int i=ts; i>=0; --i)
        {            
            propValues->del(todelete[i]);
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
