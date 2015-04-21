#include "stdafx.h"
#include "traymain.h"

TrayMainObject::~TrayMainObject()
{
    stopTimer();
    m_windows.insert(m_windows.end(), m_free_windows.begin(), m_free_windows.end());
    m_free_windows.clear();
    for (int i=0,e=m_windows.size(); i<e;++i)
    {
        PopupWindow *pw = m_windows[i];
        pw->DestroyWindow();
        delete pw;
    }
    m_windows.clear();
    DestroyWindow();
}

void TrayMainObject::create()
{
    Create(NULL, NULL, NULL, WS_POPUP);
}

void TrayMainObject::setFont(HFONT font)
{
    if (!m_font.IsNull())
        m_font.DeleteObject();
    CFontHandle fh(font);
    LOGFONT lf;
    fh.GetLogFont(&lf);
    m_font.CreateFontIndirect(&lf);
}

void TrayMainObject::setAlarmWnd(HWND wnd)
{
    m_alarmWnd = wnd;
}

 void TrayMainObject::setActivated(bool activated)
 {
    m_activated = activated;
    if (m_activated)
        stopTimer();
 }

bool TrayMainObject::showMessage(const u8string& msg)
{
    if (isHeightLimited())
    {
        m_cache.push_back(msg);
        return true;
    }

    PopupWindow *w = getFreeWindow();
    if (!w) 
        return false;

    POINT rb = { -1, -1 };
    Animation a; 
    if (m_windows.empty())
        rb = GetTaskbarRB();
    else
    {
        int last = m_windows.size() - 1;
        RECT pos;
        m_windows[last]->GetWindowRect(&pos);
        rb.x = GetSystemMetrics(SM_CXSCREEN);
        rb.y = pos.top-4;
    }

    w->setText(msg);
    const TraySettings &s = m_settings;
    COLORREF tcolor = (s.syscolor) ? GetSysColor(COLOR_INFOTEXT) : s.text;
    COLORREF bcolor = (s.syscolor) ? GetSysColor(COLOR_INFOBK) : s.background;
    SIZE sz = w->getSize();

    rb.x -= 2;
    rb.y -= sz.cy;
    rb.x -= sz.cx;
    a.pos = rb;
    a.speed = 0.5f;
    a.wait_sec = m_settings.timeout;
    a.bkgnd_color = bcolor;
    a.text_color = tcolor;
    a.notify_wnd = m_hWnd;
    a.notify_msg = WM_USER;
    a.notify_param = (WPARAM)w;
    if (m_windows.empty())
        m_point0 = rb;
    m_windows.push_back(w);
    w->startAnimation(a);
    if (!m_activated)
      startTimer();
   return true;
}

void TrayMainObject::onFinishedAnimation(PopupWindow *w)
{
    int index = -1;
    for (int i=0,e=m_windows.size(); i<e; ++i) {
        if (m_windows[i] == w) { index = i; break; }
    }
    if (index == -1) {  assert(false); return; }

    PopupWindow* window = m_windows[index];
    m_windows.erase(m_windows.begin() + index);
    freeWindow(window);
    if (m_windows.empty())
        return;

    for (int i=0,e=m_windows.size(); i<e; ++i)
    {
        int at = m_windows[i]->getAnimationType();
        if (at == PopupWindow::ANIMATION_TOSTART || at == PopupWindow::ANIMATION_TOEND)
             return;
    }

    POINT p = m_point0;
    for (int i=0,e=m_windows.size(); i<e; ++i)
    {
         PopupWindow* w = m_windows[i];
         SIZE sz = w->getSize();
         POINT w_pos = w->getAnimation().pos;
         if (w_pos.x != p.x || w_pos.y != p.y)
         {
             MoveAnimation ma;
             ma.pos = p;
             ma.speed = 0.003f;
             w->startMoveAnimation(ma);
         }
         p.y -= (sz.cy+4);
    }

}

PopupWindow* TrayMainObject::getFreeWindow()
{
    if (!m_free_windows.empty())
    {
        int last = m_free_windows.size() - 1;
        PopupWindow *wnd = m_free_windows[last];
        m_free_windows.pop_back();
        return wnd;
    }

   PopupWindow *wnd = new PopupWindow(&m_font);
   wnd->Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
   if (!wnd->IsWindow())
   {
       delete wnd;
       return NULL;
   }
   return wnd;
}

void TrayMainObject::freeWindow(PopupWindow *w)
{
    m_free_windows.push_back(w);
}

POINT TrayMainObject::GetTaskbarRB()
{
    POINT pt = { GetSystemMetrics(SM_CXSCREEN),  GetSystemMetrics(SM_CYSCREEN) };
    RECT rect;
    HWND taskBar = FindWindow(L"Shell_traywnd", NULL);
    if(taskBar && ::GetWindowRect(taskBar, &rect)) 
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

bool TrayMainObject::isHeightLimited()
{
    if (m_windows.empty())
        return false;
    int last = m_windows.size() - 1;
    const POINT &p = m_windows[last]->getAnimation().pos;
    int limit_height = (GetSystemMetrics(SM_CYSCREEN) * 2) / 10;
    return(p.y < limit_height) ? true : false;
}

TraySettings& TrayMainObject::traySettings()
{
   return m_settings;
}

void TrayMainObject::startTimer()
{
    if (m_timerStarted) return;
    const DWORD one_min = 60000;
    SetTimer(1, m_settings.interval*one_min);
    m_timerStarted = true;
}

void TrayMainObject::stopTimer()
{
    if (!m_timerStarted) return;
    KillTimer(1);
    m_timerStarted = false;
}

void TrayMainObject::onTimer()
{
    if (!::IsWindow(m_alarmWnd))
       return;
    FLASHWINFO fw;
    fw.cbSize = sizeof(FLASHWINFO);
    fw.hwnd = m_alarmWnd;
    fw.uCount = 5;
    fw.dwFlags = FLASHW_ALL;
    fw.dwTimeout = 0;
    ::FlashWindowEx(&fw);
}
