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
    HWND m_currentPage;

public:
   enum { IDD = IDD_PROPERTY_BASE };
   PropertiesDlg(PropertiesData* props) : propData(props),
       m_highlights(props), m_hotkeys(props), m_groups(props), m_colors(props), m_common(props),
       m_width(0), m_heigth(0), m_bar_width(0), m_additional_border(4), m_currentPage(NULL)
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

    LRESULT onAliases(WORD, WORD id, HWND, BOOL&) { selectPage(m_aliases); return 0; }
    LRESULT onActions(WORD, WORD id, HWND, BOOL&) { selectPage(m_actions); return 0; }
    LRESULT onHightlights(WORD, WORD id, HWND, BOOL&) { selectPage(m_highlights); return 0; }
    LRESULT onHotkeys(WORD, WORD id, HWND, BOOL&) { selectPage(m_hotkeys); return 0; }
    LRESULT onSubs(WORD, WORD id, HWND, BOOL&) { selectPage(m_subs); return 0; }
    LRESULT onAntisubs(WORD, WORD id, HWND, BOOL&) { selectPage(m_antisubs); return 0; }
    LRESULT onGags(WORD, WORD id, HWND, BOOL&) { selectPage(m_gags); return 0; }
    LRESULT onGroups(WORD, WORD id, HWND, BOOL&) { selectPage(m_groups); return 0; }
    LRESULT onColors(WORD, WORD id, HWND, BOOL&) { selectPage(m_colors); return 0; }
    LRESULT onCommon(WORD, WORD id, HWND, BOOL&) { selectPage(m_common); return 0; }
    LRESULT onTabwords(WORD, WORD id, HWND, BOOL&) { selectPage(m_tabwords); return 0; }
    LRESULT onTimers(WORD, WORD id, HWND, BOOL&) { selectPage(m_timers); return 0; }

    void selectPage(HWND page)
    {
        if (page == m_currentPage)
            return;
        if (m_currentPage != NULL)
            ::ShowWindow(m_currentPage, SW_HIDE);
        m_currentPage = page;
        ::ShowWindow(page, SW_SHOW);
        ::SetFocus(page);
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
        // add pages
        PropertyTwoConfig c;
        c.label1 = L"Макрос(alias)"; c.label2 = L"Текст"; c.list1 = L"Alias"; c.list2 = L"Текст"; c.title = L"Макросы (Aliases)";
        m_aliases.setParams(&propData->aliases, &propData->groups, c);
        m_aliases.Create(m_hWnd);
        c.label1 = L"Шаблон"; c.label2 = L"Триггер(action)"; c.list1 = L"Шаблон"; c.list2 = L"Action"; c.title = L"Триггеры (Actions)"; c.use_priority = true;
        m_actions.setParams(&propData->actions, &propData->groups, c);
        m_actions.Create(m_hWnd);
        m_highlights.Create(m_hWnd);
        m_hotkeys.Create(m_hWnd);
        c.label1 = L"Шаблон"; c.label2 = L"Замена"; c.list1 = L"Шаблон"; c.list2 = L"Замена"; c.title = L"Замены (Subs)"; c.use_priority = true;
        m_subs.setParams(&propData->subs, &propData->groups, c);
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

        selectPage(m_aliases);
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};
