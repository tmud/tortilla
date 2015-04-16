#pragma once

#include "propertyList.h"
#include "propertiesData.h"

#include "propertiesData.h"
#include "propertiesPageOneParam.h"
#include "propertiesPageTwoParams.h"
#include "propertiesPageHighlights.h"
#include "propertiesPageHotkeys.h"
#include "propertiesPageGroups.h"
#include "propertiesPageColors.h"
#include "propertiesPageCommon.h"
#include "propertiesPageTabwords.h"
#include "propertiesPageTimers.h"

class PropertiesDlg :  public CDialogImpl<PropertiesDlg>
{
    PropertiesData *propData; 
    PropertyTwoParams m_aliases;
    PropertyTwoParams m_actions;
    PropertyHighlights m_highlights;
    PropertyHotkeys m_hotkeys;
    PropertyTwoParams m_subs;
    PropertyOneParam m_antisubs;
    PropertyOneParam m_gags;
    PropertyGroups m_groups;
    PropertyColors m_colors;
    PropertyCommon m_common;
    PropertyTabwords m_tabwords;
    PropertyTimers m_timers;

    int m_width;
    int m_heigth;
    int m_bar_width;
    int m_additional_border;
    CBevelLine m_line;
    int m_currentPage;

public:
   enum { IDD = IDD_PROPERTY_BASE };
   PropertiesDlg(PropertiesData* props) : propData(props),
       m_highlights(props), m_hotkeys(props), m_groups(props), m_colors(props), m_common(props),
       m_width(0), m_heigth(0), m_bar_width(0), m_additional_border(4), m_currentPage(-1)
   {
       assert(props);
   }

private:
    BEGIN_MSG_MAP(PropertiesDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_ALIASES, onAliases)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_ACTIONS, onActions)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_HIGHLIGHTS, onHightlights)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_HOTKEYS, onHotkeys)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROPS_SUBS, onSubs)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROPS_ANTISUBS, onAntisubs)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROPS_GAGS, onGags)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_GROUPS, onGroups)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_COLORS, onColors)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_COMMON, onCommon)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_TABWORDS, onTabwords)
        COMMAND_ID_HANDLER(IDC_BUTTON_PROP_TIMERS, onTimers)
    END_MSG_MAP()

    LRESULT onAliases(WORD, WORD id, HWND, BOOL&) { selectPage(0); return 0; }
    LRESULT onActions(WORD, WORD id, HWND, BOOL&) { selectPage(1); return 0; }
    LRESULT onHightlights(WORD, WORD id, HWND, BOOL&) { selectPage(2); return 0; }
    LRESULT onHotkeys(WORD, WORD id, HWND, BOOL&) { selectPage(3); return 0; }
    LRESULT onSubs(WORD, WORD id, HWND, BOOL&) { selectPage(4); return 0; }
    LRESULT onAntisubs(WORD, WORD id, HWND, BOOL&) { selectPage(5); return 0; }
    LRESULT onGags(WORD, WORD id, HWND, BOOL&) { selectPage(6); return 0; }
    LRESULT onGroups(WORD, WORD id, HWND, BOOL&) { selectPage(7); return 0; }
    LRESULT onColors(WORD, WORD id, HWND, BOOL&) { selectPage(8); return 0; }
    LRESULT onCommon(WORD, WORD id, HWND, BOOL&) { selectPage(9); return 0; }
    LRESULT onTabwords(WORD, WORD id, HWND, BOOL&) { selectPage(10); return 0; }
    LRESULT onTimers(WORD, WORD id, HWND, BOOL&) { selectPage(11); return 0; }

    void selectPage(int page)
    {
        if (page == m_currentPage)
            return;
        HWND newpage = getPage(page);
        if (!newpage)
            return;
        if (m_currentPage != -1)
            ::ShowWindow(getPage(m_currentPage), SW_HIDE);
        m_currentPage = page;        
        PropertiesDlgData& d = propData->dlg;
        d.current_page = page;
        ::ShowWindow(newpage, SW_SHOW);
        ::SetFocus(newpage);
    }

    HWND getPage(int page)
    {
        switch(page) {
        case 0: return m_aliases;
        case 1: return m_actions;
        case 2: return m_highlights;
        case 3: return m_hotkeys;
        case 4: return m_subs;
        case 5: return m_antisubs;
        case 6: return m_gags;
        case 7: return m_groups;
        case 8: return m_colors;
        case 9: return m_common;
        case 10: return m_tabwords;
        case 11: return m_timers;
        }
        return NULL;
    }

    void setPosition(HWND page)
    {
        RECT pos;
        ::GetClientRect(page, &pos);
        int width = pos.right;
        int heigth = pos.bottom;

        pos.left = (m_width - width) / 2;
        pos.top = 0;
        pos.right = pos.left + width;
        pos.bottom = pos.top + heigth;
        ::MoveWindow(page, pos.left, pos.top, width, heigth, FALSE);
    }

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        PropertiesDlgData& d = propData->dlg;
        d.pages.resize(12);

        // add pages
        PropertyTwoConfig c;
        c.label1 = L"Макрос(alias)"; c.label2 = L"Текст"; c.list1 = L"Alias"; c.list2 = L"Текст"; c.title = L"Макросы (Aliases)";
        m_aliases.setParams(&propData->aliases, &propData->groups, &d.pages[0], c);
        m_aliases.Create(m_hWnd);
        c.label1 = L"Шаблон"; c.label2 = L"Триггер(action)"; c.list1 = L"Шаблон"; c.list2 = L"Action"; c.title = L"Триггеры (Actions)"; c.use_priority = true;
        m_actions.setParams(&propData->actions, &propData->groups, &d.pages[1], c);
        m_actions.Create(m_hWnd);
        m_highlights.Create(m_hWnd);
        m_hotkeys.Create(m_hWnd);
        c.label1 = L"Шаблон"; c.label2 = L"Замена"; c.list1 = L"Шаблон"; c.list2 = L"Замена"; c.title = L"Замены (Subs)"; c.use_priority = true;
        m_subs.setParams(&propData->subs, &propData->groups, &d.pages[2], c);
        m_subs.Create(m_hWnd);

        PropertyOneConfig c0; c0.title = L"Антизамены (Antisubs)"; c0.label = L"Шаблон"; c0.list = L"Шаблон";
        m_antisubs.setParams(&propData->antisubs, &propData->groups, c0);
        m_antisubs.Create(m_hWnd);

        c0.title = L"Фильтры (Gags)";
        m_gags.setParams(&propData->gags, &propData->groups, c0);
        m_gags.Create(m_hWnd);

        m_groups.Create(m_hWnd);
        m_colors.Create(m_hWnd);
        m_common.Create(m_hWnd);
        m_tabwords.setParams(&propData->tabwords);
        m_tabwords.Create(m_hWnd);

        m_timers.setParams(&propData->timers, &propData->groups);
        m_timers.Create(m_hWnd);

        // line delimeter
        m_line.SubclassWindow(GetDlgItem(IDC_STATIC_BLLINE));

        // calc size of freespace for pages
        RECT pos;
        GetClientRect(&pos);
        m_heigth = pos.bottom;
        m_width = pos.right;

        // set position of vertical delimeter
        GetWindowRect(&pos);
        int left_pos = pos.left;
        m_line.GetWindowRect(&pos);
        left_pos = pos.left - left_pos;
        m_width = left_pos;

        // calc main window size + move it
        GetWindowRect(&pos);
        int width = pos.right - pos.left;
        int heigth = pos.bottom - pos.top;

        DWORD screen_width = GetSystemMetrics(SM_CXSCREEN);
        DWORD screen_heigth = GetSystemMetrics(SM_CYSCREEN);
        pos.left = (screen_width - width) / 2;
        pos.top = (screen_heigth - heigth) / 2;
        pos.right = pos.left + width;
        pos.bottom = pos.top + heigth;
        MoveWindow(&pos);
        CenterWindow(GetParent());

        setPosition(m_aliases);
        setPosition(m_actions);
        setPosition(m_highlights);
        setPosition(m_hotkeys);
        setPosition(m_subs);
        setPosition(m_antisubs);
        setPosition(m_gags);
        setPosition(m_groups);
        setPosition(m_colors);
        setPosition(m_common);
        setPosition(m_tabwords);
        setPosition(m_timers);
        selectPage(d.current_page);
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};
