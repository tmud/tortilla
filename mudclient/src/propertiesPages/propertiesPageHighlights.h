#pragma once

#include "propertiesPagesElements.h"
#include "propertiesSaveHelper.h"

class PropertyHighlights :  public CDialogImpl<PropertyHighlights>, public PropertyListCtrlHandler
{
    PropertiesValues *propValues;
    PropertiesValues *propGroups;
    HighlightValues m_list_values;
    PropertyListCtrl m_list;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_pattern;
    CButton m_add;
    CButton m_del;
    CButton m_replace;
    CButton m_reset;
    CButton m_filter;
    CComboBox m_cbox;
    bool m_filterMode;
    tstring m_currentGroup;
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
     PropertyHighlights(PropertiesData *data) : m_filterMode(false), 
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

private:
    BEGIN_MSG_MAP(PropertyHighlights)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       MESSAGE_HANDLER(WM_DESTROY, OnCloseDialog)
       MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
       MESSAGE_HANDLER(WM_USER+2, OnSetFocus)
       MESSAGE_HANDLER(WM_USER, OnTextColor)
       MESSAGE_HANDLER(WM_USER+1, OnBkgColor)
       COMMAND_ID_HANDLER(IDC_CHECK_GROUP_FILTER, OnFilter)
       COMMAND_HANDLER(IDC_COMBO_GROUP, CBN_SELCHANGE, OnGroupChanged);
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_UNDERLINE, OnFontUnderlined)
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_FLASH, OnFontBorder)
       COMMAND_ID_HANDLER(IDC_CHECK_HIGHLIGHTS_ITALIC, OnFontItalic)
       COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnAddElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_DEL, OnDeleteElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_REPLACE, OnReplaceElement)
       COMMAND_ID_HANDLER(IDC_BUTTON_RESET, OnResetData)
       COMMAND_HANDLER(IDC_EDIT_HIGHLIGHT_TEXT, EN_CHANGE, OnPatternEditChanged)
       NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
       NOTIFY_HANDLER(IDC_LIST, NM_KILLFOCUS, OnListKillFocus)
       REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

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
        getFlags(hl, &flags);
        int index = m_list_values.find(pattern);
        if (index == -1 && m_filterMode)
        {
            int index2 = propValues->find(pattern);
            if (index2 != -1)
                propValues->del(index2);
        }
        m_list_values.add(index, pattern, hl, m_currentGroup);

        if (index == -1)
        {
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, pattern);
            m_list.addItem(pos, 1, flags);
            m_list.addItem(pos, 4, m_currentGroup);
        }
        else
        {
            m_list.setItem(index, 0, pattern);
            m_list.setItem(index, 1, flags);
            m_list.setItem(index, 4, m_currentGroup);
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
        m_list.SelectItem(-1);
        m_pattern.SetFocus();
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
            v.value.underlined = checked ? 1 : 0;
            updateCurrentItem(false);
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
            v.value.border = checked ? 1 : 0;
            updateCurrentItem(false);
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
            v.value.italic = checked ? 1 : 0;
            updateCurrentItem(false);
        }
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
            updateCurrentItem(false);
            return 0;
        }
        tstring old = m_currentGroup;
        m_currentGroup = group;
        updateCurrentItem(false);
        m_currentGroup = old;
        saveValues();
        m_currentGroup = group;
        loadValues();
        update();
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
                    updateCurrentItem(false);
                    return 0;
                }
            }
            m_replace.EnableWindow(len > 0 && selected >= 0 && !currelement);
            m_add.EnableWindow(len == 0 ? FALSE : !currelement);
            m_reset.EnableWindow(len == 0 ? FALSE : TRUE);
            if (currelement)
                updateCurrentItem(false);
        }
        return 0;
    }

    void updateCurrentItem(bool update_key)
    {
        int item = m_list.getOnlySingleSelection();
        if (item == -1) return;
        m_update_mode = true;
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        highlight_value& v = m_list_values.getw(item);
        if (v.key != pattern) 
        {
            if (!update_key) { m_update_mode = false; return; }
            v.key = pattern;
            m_list.setItem(item, 0, pattern);
        }
        PropertiesHighlight &hl = v.value;
        tstring flags;
        getFlags(hl, &flags);
        m_list.setItem(item, 1, flags);

        if (v.group != m_currentGroup)
        {
            v.group = m_currentGroup;
            m_list.setItem(item, 4, m_currentGroup);
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
            enableColorControls(TRUE);
            m_del.EnableWindow(FALSE);
            if (!m_deleted)
                m_pattern.SetWindowText(L"");
            m_reset.EnableWindow(FALSE);
        }
        else if (items_selected == 1)
        {
            m_add.EnableWindow(FALSE);
            enableColorControls(TRUE);
            m_del.EnableWindow(TRUE);
            m_reset.EnableWindow(TRUE);
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
            enableColorControls(FALSE);
            m_del.EnableWindow(TRUE);
            m_add.EnableWindow(FALSE);
            m_reset.EnableWindow(FALSE);
            m_pattern.SetWindowText(L"");
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
        m_reset.Attach(GetDlgItem(IDC_BUTTON_RESET));
        m_filter.Attach(GetDlgItem(IDC_CHECK_GROUP_FILTER));
        m_cbox.Attach(GetDlgItem(IDC_COMBO_GROUP));

        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.setHandler(this);
        m_list.addColumn(L"Шаблон", 40);
        m_list.addColumn(L"Флаги", 10);
        m_list.addColumn(L"Цвет текста", 15);
        m_list.addColumn(L"Цвет фона", 15);
        m_list.addColumn(L"Группа", 20);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_add.EnableWindow(FALSE);
        m_del.EnableWindow(FALSE);
        m_replace.EnableWindow(FALSE);
        m_reset.EnableWindow(FALSE);
        m_underline.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_UNDERLINE));
        m_border.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_FLASH));
        m_italic.Attach(GetDlgItem(IDC_CHECK_HIGHLIGHTS_ITALIC));
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
                v.value.textcolor = color;
                updateCurrentItem(false);
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
                v.value.bkgcolor = color;
                updateCurrentItem(false);
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
            const highlight_value& hv = m_list_values.get(i);
            const PropertiesHighlight& hl = hv.value;

            tstring flags;
            getFlags(hl, &flags);
            m_list.addItem(i, 0, hv.key);
            m_list.addItem(i, 1, flags);
            m_list.addItem(i, 4, hv.group);
        }

        int index = -1;
        tstring pattern;
        getWindowText(m_pattern, &pattern);
        if (!pattern.empty())
            index = m_list_values.find(pattern);
        m_state_helper.loadCursorAndTopPos(index);
    }

    void loadValues()
    {
        m_list_values.clear();
        for (int i=0,e=propValues->size(); i<e; ++i)
        {
            const property_value& v= propValues->get(i);
            if (m_filterMode && v.group != m_currentGroup)
                continue;
            PropertiesHighlight hl;
            hl.convertFromString(v.value);
            m_list_values.add(-1, v.key, hl, v.group);
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
                const highlight_value &hv = m_list_values.get(i);
                tstring value;
                hv.value.convertToString(&value);
                propValues->add(-1, hv.key, value, hv.group);
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

        int pos_count = positions.size();
        int elem_count = m_list_values.size();
        for (int i=0; i<elem_count; ++i)
        {
            const highlight_value& v = m_list_values.get(i);
            int index = (i < pos_count) ? positions[i] : -1;
            tstring value;
            v.value.convertToString(&value);
            propValues->add(index, v.key, value, v.group);
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

    void getFlags(const PropertiesHighlight& hl, tstring* flags)
    {
        if (hl.underlined) flags->append(L"П");
        if (hl.border) flags->append(L"Р");
        if (hl.italic) flags->append(L"К");
        if (flags->empty())
            flags->append(L"-");
    }
};
