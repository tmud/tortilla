#include "stdafx.h"
#include "mainwnd.h"
extern HWND m_hwnd_client;

void ClickpadMainWnd::onCreate()
{

    createButton();
}

void ClickpadMainWnd::onSize()
{

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