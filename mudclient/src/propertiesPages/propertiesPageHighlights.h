#pragma once

#include "propertiesPagesElements.h"
#include "propertiesSaveHelper.h"
#include "propertiesGroupFilter.h"

class PropertyHighlights :  public CDialogImpl<PropertyHighlights>, public PropertyListCtrlHandler
{
    PropertiesValues *propValues;
    PropertiesValues *propGroups;
    HighlightValues m_list_values;
    std::vector<int> m_list_positions;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_pattern;
    CButton m_add;
    CButton m_del;
    CButton m_up;
    CButton m_down;
    CButton m_replace;
    CButton m_reset;
    CComboBox m_filter;
    CComboBox m_cbox;
    int m_filterMode;
    tstring m_currentGroup;
    tstring m_loadedGroup;
    HighlightSelectColor m_textColor;
    HighlightSelectColor m_bkgColor;
    HighlightsExampleWindow m_exampleWnd;
    CButton m_underline;
    CButton m_border;
    CButton m_italic;
    COLORREF m_windowColor;
    bool m_deleted;
    bool m_update_mode;
    PropertiesDlgPageState *dlg_state;
    PropertiesSaveHelper m_state_helper;

public:
     enum { IDD = IDD_PROPERTY_HIGHLIGHTS };
     PropertyHighlights(PropertiesData *data) : m_filterMode(0), 
         m_textColor(RGB(192,192,192), WM_USER), m_bkgColor(RGB(0,0,0), WM_USER+1),
         m_exampleWnd(data), m_deleted(false), m_update_mode(false), dlg_state(NULL)
     {
         propValues = &data->highlights;
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
             getWindowText(m_pattern, &pattern);
             const highlight_value& v = m_list_values.get(item);
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
    BEGIN_MSG_MAP(PropertyHighlights)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER+2, OnSetFocus)
       MESSAGE_HANDLER(WM_USER, OnTextColor)
       MESSAGE_HANDLER(WM_USER+1, OnBkgColor)
       MESSAGE_HANDLER(WM_USER+3, OnKeyDown)
       COMMAND_HANDLER(IDC_COMBO_FILTER, CBN_SELCHANGE, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged);
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_UNDERLINE, OnFontUnderlined)
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_FLASH, OnFontBorder)
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_ITALIC, OnFontItalic)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_REPLACE, OnReplaceElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_RESET, OnResetData)
       COMMAND_ID_HANDLER(IDC_BUTTON_UP, OnUpElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DOWN, OnDownElement)
       COMMAND_HANDLER(IDC_EDIT_HIGHLIGHT_TEXT, EN_CHANGE, OnPatternEditChanged)
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
        if (wparam == VK_INSERT)
        {
            BOOL b = FALSE;
            OnResetData(0, 0, 0, b);
            return 1;
        }
        return 0;
    }

    // handler for draw item
    bool drawPropertyListItem(PropertyListItemData *pData)
    {
        int subitem = pData->subitem;
        if (subitem == 3 || subitem == 2) // 2 - text color, 3 - background color
        {
            const highlight_value& hv = m_list_values.get(pData->item);
            const PropertiesHighlight& hl = hv.value;
            pData->bkg = (subitem == 2) ? hl.textcolor : hl.bkgcolor;
            return true;
        }
        return false;
    }

    LRESULT OnAddElement(WORD, WORD, HWND, BOOL&)
    {
        tstring pattern;
        getWindowText(m_pattern, &pattern);

        PropertiesHighlight hl;
        hl.textcolor = m_textColor.getColor();
        hl.bkgcolor = m_bkgColor.getColor();
        hl.underlined = m_underline.GetCheck() ? 1 : 0;
        hl.border = m_border.GetCheck() ? 1 : 0;
        hl.italic = m_italic.GetCheck() ? 1 : 0;

        tstring flags;
        hl.getFlags(&flags);
        int index = m_list_values.find(pattern, m_currentGroup);
        if (index == -1 && m_filterMode)
        {
            int index2 = propValues->find(pattern, m_currentGroup);
            if (index2 != -1)
                propValues->del(index2);
        }

        if (index == -1)
        {
            index = m_list.getOnlySingleSelection() + 1;
            m_list_values.insert(index, pattern, hl, m_currentGroup);
            m_list.addItem(index, 0, pattern);
            m_list.addItem(index, 1, flags);
            m_list.addItem(index, 4, m_currentGroup);
        }
        else
        {
            m_list_values.add(index, pattern, hl, m_currentGroup);
            m_list.setItem(index, 0, pattern);
            m_list.setItem(index, 1, flags);
            m_list.setItem(index, 4, m_currentGroup);
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
        updateCurrentItem(true);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnResetData(WORD, WORD, HWND, BOOL&)
    {
        m_pattern.SetWindowText(L"");
        m_list.SelectItem(-1);
        m_pattern.SetFocus();
        return 0;
    }

    LRESULT OnUpElement(WORD, WORD, HWND, BOOL&)
    {
        propertiesUpDown<PropertiesHighlight> ud(4);
        ud.up(m_list, m_list_values, false);
        return 0;
    }

    LRESULT OnDownElement(WORD, WORD, HWND, BOOL&)
    {
        propertiesUpDown<PropertiesHighlight> ud(4);
        ud.down(m_list, m_list_values, false);
        return 0;
    }

    LRESULT OnFontUnderlined(WORD, WORD, HWND, BOOL&)
    {
        bool checked = m_underline.GetCheck() ? true : false;
        m_exampleWnd.setUnderlined(checked);
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
            highlight_value& v = m_list_values.getw(item);
            if (v.group != m_currentGroup)
                return 0;
            if (updateCurrentItem(false))
                v.value.underlined = checked ? 1 : 0;
        }
        return 0;
    }

    LRESULT OnFontBorder(WORD, WORD, HWND, BOOL&)
    {
        bool checked = m_border.GetCheck() ? true : false;
        m_exampleWnd.setBorder(checked);
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
            highlight_value& v = m_list_values.getw(item);
            if (v.group != m_currentGroup)
                return 0;
            if (updateCurrentItem(false))
                v.value.border = checked ? 1 : 0;
        }
        return 0;
    }

    LRESULT OnFontItalic(WORD, WORD, HWND, BOOL&)
    {
        bool checked = m_italic.GetCheck() ? true : false;
        m_exampleWnd.setItalic(checked);
        int item = m_list.getOnlySingleSelection();
        if (item != -1)
        {
            highlight_value& v = m_list_values.getw(item);
            if (v.group != m_currentGroup)
                return 0;
            if (updateCurrentItem(false))
                v.value.italic = checked ? 1 : 0;
        }
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
        getWindowText(m_pattern, &pattern);
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
            int index = m_list_values.find(pattern, m_currentGroup);
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

    bool updateCurrentItem(bool update_key)
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return false;
        m_update_mode = true;
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        highlight_value& v = m_list_values.getw(item);
        if (v.key != pattern && !update_key)
            { m_update_mode = false; return false; }
        if (v.key != pattern)
        {
            v.key = pattern;
            m_list.setItem(item, 0, pattern);
        }
        PropertiesHighlight &hl = v.value;
        if (v.group != m_currentGroup)
        {
            v.group = m_currentGroup;
            m_list.setItem(item, 4, m_currentGroup);
        }
        COLORREF color = m_textColor.getColor();
        m_exampleWnd.setTextColor(color);
        hl.textcolor = color;
        color = m_bkgColor.getColor();
        m_exampleWnd.setBkgColor(color);
        hl.bkgcolor = color;

        hl.italic = m_italic.GetCheck() ? 1 : 0;
        hl.border = m_border.GetCheck() ? 1 : 0;
        hl.underlined = m_underline.GetCheck() ? 1 : 0;

        tstring flags;
        hl.getFlags(&flags);
        m_list.setItem(item, 1, flags);

        m_update_mode = false;
		return true;
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
                m_pattern.SetWindowText(L"");
        }
        else if (items_selected == 1)
        {
            int item = m_list.getOnlySingleSelection();
            const highlight_value& hv = m_list_values.get(item);
            const PropertiesHighlight& hl = hv.value;
            m_pattern.SetWindowText( hv.key.c_str() );
            m_textColor.setColor(hl.textcolor);
            m_bkgColor.setColor(hl.bkgcolor);
            m_underline.SetCheck(hl.underlined ? BST_CHECKED : BST_UNCHECKED);
            m_border.SetCheck(hl.border ? BST_CHECKED : BST_UNCHECKED);
            m_italic.SetCheck(hl.italic ? BST_CHECKED : BST_UNCHECKED);
            m_exampleWnd.setAllParameters(hl.textcolor, hl.bkgcolor, 
                hl.underlined ? true:false, 
                hl.border ? true:false,
                hl.italic ? true:false);
            int index = getGroupIndex(hv.group);
            m_cbox.SetCurSel(index);
            m_currentGroup = hv.group;
        }
        else
        {
            m_pattern.SetWindowText(L"");
        }
        updateButtons();
        m_update_mode = false;
        return 0;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_windowColor = GetSysColor(COLOR_WINDOW);
        m_pattern.Attach(GetDlgItem(IDC_EDIT_HIGHLIGHT_TEXT));

        RECT rc;
        CStatic tc(GetDlgItem(IDC_STATIC_TEXTCOLOR));
        tc.GetWindowRect(&rc);
        tc.ShowWindow(SW_HIDE);
        ScreenToClient(&rc);
        m_textColor.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP);

        CStatic bc(GetDlgItem(IDC_STATIC_BKGCOLOR));
        bc.GetWindowRect(&rc);
        bc.ShowWindow(SW_HIDE);
        ScreenToClient(&rc);
        m_bkgColor.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP);

        CStatic he(GetDlgItem(IDC_STATIC_HIGHLIGHTS_EXAMPLE));
        he.GetWindowRect(&rc);
        he.ShowWindow(SW_HIDE);
        ScreenToClient(&rc);
        m_exampleWnd.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE);
        m_add.Attach(GetDlgItem(IDC_BUTTON_ADD));
        m_del.Attach(GetDlgItem(IDC_BUTTON_DEL));
        m_replace.Attach(GetDlgItem(IDC_BUTTON_REPLACE));
        m_up.Attach(GetDlgItem(IDC_BUTTON_UP));
        m_down.Attach(GetDlgItem(IDC_BUTTON_DOWN));
        m_reset.Attach(GetDlgItem(IDC_BUTTON_RESET));
        m_filter.Attach(GetDlgItem(IDC_COMBO_FILTER));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));

        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.setHandler(this);
        m_list.addColumn(L"Шаблон", 40);
        m_list.addColumn(L"Флаги", 10);
        m_list.addColumn(L"Цвет текста", 15);
        m_list.addColumn(L"Цвет фона", 15);
        m_list.addColumn(L"Группа", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
        m_list.setKeyDownMessageHandler(m_hWnd, WM_USER+3);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_underline.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_UNDERLINE));
        m_border.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_FLASH));
        m_italic.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_ITALIC));
        m_state_helper.init(dlg_state, &m_list);

        m_state_helper.loadGroupAndFilter(m_currentGroup, m_filterMode);

        m_filter.AddString(L"Все группы");
        m_filter.AddString(L"Текущая группа");
        m_filter.AddString(L"Активные группы");
        m_filter.SetCurSel(m_filterMode);

        loadValues();
        updateButtons();
        enableColorControls(TRUE);
        return 0;
    }

    LRESULT OnCloseDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        saveValues();
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
            m_exampleWnd.updateProps();
            PostMessage(WM_USER+2); // OnSetFocus to list
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

    LRESULT OnTextColor(UINT, WPARAM, LPARAM, BOOL&)
    {
        COLORREF color = m_textColor.getColor();
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();
            m_textColor.setColor(color);
            m_exampleWnd.setTextColor(color);
            int item = m_list.getOnlySingleSelection();
            if (item != -1)
            {
                highlight_value& v = m_list_values.getw(item);
                if (v.group != m_currentGroup)
                    return 0;
                if (updateCurrentItem(false))
                    v.value.textcolor = color;
            }
        }
        return 0;
    }

    LRESULT OnBkgColor(UINT, WPARAM, LPARAM, BOOL&)
    {
        COLORREF color = m_bkgColor.getColor();
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();
            m_bkgColor.setColor(color);
            m_exampleWnd.setBkgColor(color);
            int item = m_list.getOnlySingleSelection();
            if (item != -1)
            {
                highlight_value& v = m_list_values.getw(item);
                if (v.group != m_currentGroup)
                    return 0;
                if (updateCurrentItem(false))
                    v.value.bkgcolor = color;
            }
        }
        return 0;
    }

    void enableColorControls(BOOL state)
    {
        m_textColor.EnableWindow(state);
        m_bkgColor.EnableWindow(state);
        m_underline.EnableWindow(state);
        m_border.EnableWindow(state);
        m_italic.EnableWindow(state);
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
            const highlight_value& hv = m_list_values.get(i);
            const PropertiesHighlight& hl = hv.value;

            tstring flags;
            hl.getFlags(&flags);
            m_list.addItem(i, 0, hv.key);
            m_list.addItem(i, 1, flags);
            m_list.addItem(i, 4, hv.group);
        }

        m_state_helper.loadCursorAndTopPos(4);
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
        bool pattern_empty = m_pattern.GetWindowTextLength() == 0;
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
                const highlight_value& v = m_list_values.get(selected);
                mode = TRUE;
                if (m_currentGroup == v.group)
                {
                    tstring pattern;
                    getWindowText(m_pattern, &pattern);
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

    void loadValues()
    {
        m_loadedGroup = m_currentGroup;
        PropertiesGroupFilter gf(propGroups);
        m_list_values.clear();
        m_list_positions.clear();
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v= propValues->get(i);
            if (m_filterMode == 1) {
                if (v.group != m_currentGroup)
                continue;
            }
            else if (m_filterMode == 2) {
                if (!gf.isGroupActive(v.group))
                    continue;
            }
            PropertiesHighlight hl;
            hl.convertFromString(v.value);
            m_list_values.add(-1, v.key, hl, v.group);
            if (m_filterMode)
                m_list_positions.push_back(i);
         }
    }

    void saveValues()
    {
        if (!m_state_helper.save(m_loadedGroup, m_filterMode))
            return;

        if (!m_filterMode)
        {
            propValues->clear();
            for (int i=0,e=m_list_values.size(); i<e; ++i)
            {
                const highlight_value &hv = m_list_values.get(i);
                tstring value;
                hv.value.convertToString(&value);
                propValues->add(-1, hv.key, value, hv.group);
            }
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
            const highlight_value& v = m_list_values.get(i);
            int pos = propValues->find(v.key, v.group);
            if (pos != -1 && pos != index)
                todelete.push_back(pos);
            tstring value;
            v.value.convertToString(&value);
            propValues->add(index, v.key, value, v.group);
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
