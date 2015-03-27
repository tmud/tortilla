#include "stdafx.h"
#include "mainwnd.h"
extern luaT_window m_parent_window;
extern HWND m_hwnd_client;

ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_rows(1), m_columns(10), m_button_width(64), m_button_height(64)
{
}

ClickpadMainWnd::~ClickpadMainWnd()
{
}

void ClickpadMainWnd::createRow()
{
    int rows = m_buttons.size();
    int columns = (rows > 0) ? m_buttons[0].size() : 0;
}

void ClickpadMainWnd::createColumn()
{
}

void ClickpadMainWnd::onCreate()
{    
    m_settings_wnd.Create(m_hWnd, rcDefault, WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    




    //turnonEditMode(); // todo

    //createButton();
}

void ClickpadMainWnd::onSize()
{
    RECT pos;
    GetClientRect(&pos);
    if (!m_editmode)
        return;
    pos.top = pos.bottom / 2;
    m_settings_wnd.MoveWindow(&pos);
}

void ClickpadMainWnd::switchEditMode()
{
    if (!m_editmode)
    {
        CWindow floatwnd(m_parent_window.floathwnd());
        RECT pos;
        floatwnd.GetClientRect(&pos);
        int height = pos.bottom;
        floatwnd.GetWindowRect(&pos);
        
        RECT settings_pos;
        m_settings_wnd.GetClientRect(&settings_pos);
        pos.bottom += settings_pos.bottom;
        floatwnd.MoveWindow(&pos);

        settings_pos.top += height;
        settings_pos.bottom += height;
        m_settings_wnd.MoveWindow(&settings_pos);
        m_settings_wnd.ShowWindow(SW_SHOW);
        m_editmode = true;
    }
    else
    {


        m_editmode = false;
    }
}

void ClickpadMainWnd::onSetParentFocus()
{
    ::SetFocus(m_hwnd_client);
}

void ClickpadMainWnd::createButton()
{
    PadButton *b = new PadButton();
    RECT pos = { 0, 0, 32, 32 };
    b->Create(m_hWnd, pos, L"test", WS_CHILD | WS_VISIBLE);
    m_buttons.push_back(b);
}
