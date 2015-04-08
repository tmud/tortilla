#include "stdafx.h"
#include "traymain.h"
//#include "resource.h"

void TrayMainObject::setFont(HFONT font)
{
    if (!m_font.IsNull())
        m_font.DeleteObject();
    CFontHandle fh(font);
    LOGFONT lf;
    fh.GetLogFont(&lf);
    m_font.CreateFontIndirect(&lf);
}

void TrayMainObject::showMessage(const u8string& msg)
{
    createWindow(msg);
    startAnimation(0);
}

void TrayMainObject::createWindow(const u8string& msg)
{
   PopupWindow *wnd = new PopupWindow(msg, &m_font);
   wnd->Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_CHILD);
   m_popups.push_back(wnd);
}

void TrayMainObject::startAnimation(int window_index)
{
    PopupWindow *w = getWindow(window_index);
    if (!w || w->isAnimated()) { assert(false); return; }

    Animation a;
    SIZE sz = w->getSize();
    a.window_size = sz;

    int on_screen = 0; POINT rb = { -1, -1 };
    for (int i=0,e=m_popups.size();i<e;++i) {
        if (!m_popups[i]->isAnimated()) 
            continue;
        on_screen++;
        const Animation &a = w->getAnimation();
        POINT p = a.start_pos;
        if (rb.x < 0 || rb.y > p.y) rb = p;
    }

    if (on_screen == 0)
        rb = GetTaskbarRB();
    rb.y -= sz.cy;
    a.start_pos = rb;
    a.pos = rb;
    rb.x -= sz.cx;
    a.end_pos = rb;
    a.speed = 1.0f;
    w->startAnimation(a);
}

void TrayMainObject::stopAnimation(int window_index)
{
}

PopupWindow* TrayMainObject::getWindow(int index) const
{
    int windows = m_popups.size();
    return (index >= 0 && index < windows) ? m_popups[index] : NULL;
}

POINT TrayMainObject::GetTaskbarRB()
{
    POINT pt = { GetSystemMetrics(SM_CXSCREEN),  GetSystemMetrics(SM_CYSCREEN) };
    RECT rect;
    HWND taskBar = FindWindow(L"Shell_traywnd", NULL);
    if(taskBar && GetWindowRect(taskBar, &rect)) 
    {
        int height = rect.bottom - rect.top;
        int width = rect.right - rect.left;
        if (height < width && pt.y >= rect.top && pt.y <= rect.bottom)
        {
            pt.y -= height;
        }
    }
    return pt;
}
