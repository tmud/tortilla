#include "stdafx.h"
#include "traymain.h"

TrayMainObject::~TrayMainObject()
{
    stopTimer();
    for (int i=0,e=m_popups.size(); i<e;++i)
    {
        PopupWindow *pw = m_popups[i];
        pw->DestroyWindow();
        delete pw;
    }
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
    const TraySettings &s = m_settings;
    COLORREF tcolor = (s.syscolor) ? GetSysColor(COLOR_INFOTEXT) : s.text;
    COLORREF bcolor = (s.syscolor) ? GetSysColor(COLOR_INFOBK) : s.background;
    for (int i=0,e=m_popups.size();i<e;++i)
    {
      if (!m_popups[i]->isAnimated())
      {
          startAnimation(i, msg, bcolor, tcolor);
          return true;
      }
    }

   PopupWindow *wnd = new PopupWindow(&m_font);
   wnd->Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
   if (!wnd->IsWindow())
   {
       delete wnd;
       return false;
   }
   m_popups.push_back(wnd);
   int index = m_popups.size() - 1;
   startAnimation(index, msg, bcolor, tcolor);
   if (!m_activated)
      startTimer();
   return true;
}

void TrayMainObject::startAnimation(int window_index, const u8string&msg, COLORREF bkgnd, COLORREF text)
{
    PopupWindow *w = getWindow(window_index);
    if (!w || w->isAnimated()) { assert(false); return; }

    w->setText(msg);

    Animation a;
    SIZE sz = w->getSize();

    int on_screen = 0; POINT rb = { -1, -1 };
    for (int i=0,e=m_popups.size();i<e;++i) {
        if (!m_popups[i]->isAnimated()) 
            continue;
        on_screen++;
        RECT pos;
        m_popups[i]->GetWindowRect(&pos);
        POINT p = { GetSystemMetrics(SM_CXSCREEN), pos.top-4 };
        if (rb.x < 0 || rb.y > p.y) rb = p;
    }

    if (on_screen == 0)
        rb = GetTaskbarRB();
    rb.x -= 2;
    rb.y -= sz.cy;
    rb.x -= sz.cx;
    a.pos = rb;
    a.speed = 0.5f;
    a.wait_sec = m_settings.timeout;
    a.bkgnd_color = bkgnd;
    a.text_color = text;
    a.notify_wnd = m_hWnd;
    a.notify_msg = WM_USER;
    a.notify_param = window_index;
    w->startAnimation(a);
}

void TrayMainObject::onFinishedAnimation(int id)
{
    int x = 1;
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
