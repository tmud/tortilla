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
        delete pw;
    }
    m_windows.clear();
    if (IsWindow())
        DestroyWindow();
}

bool TrayMainObject::create()
{
    if (!m_shared.init())
        return false;
    Create(HWND_MESSAGE);
    getWorkingArea(&m_workingarea);
	return true;
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

 void TrayMainObject::addMessage(const Msg& m)
 {
     m_queue.push_back(m);
     tryShowQueue();
 }
 
void TrayMainObject::tryShowQueue()
{
    while (!m_queue.empty())
    {        
        SharingWindow sw;
        if (m_shared.getLastPostion(&sw) && (sw.y < (m_workingarea.bottom/5)) )
            break;
        Msg msg(*m_queue.begin());
        if (showMessage(msg))
            m_queue.pop_front();
    }
}

bool TrayMainObject::showMessage(const Msg& msg)
{
#ifndef _DEBUG
    if (m_activated && !m_settings.showactive)
       return true;
#endif
    PopupWindow *w = getFreeWindow();
    if (!w)
        return false;

    NotifyParams np; 
    np.wnd = m_hWnd;
    np.msg = WM_USER;
    np.wparam = (WPARAM)w;
    w->setText(msg, np, m_settings.timeout);

    SharingWindow sw = w->getPosition();
    if (!m_shared.tryAddWindow(&sw, m_workingarea, 4))
    {
        freeWindow(w);
        return false;
    }
    w->startAnimation(sw.x, sw.y);
    m_windows.push_back(w);

    if (!m_activated)
      startTimer();
    return true;
}

void TrayMainObject::onTickPopups()
{
    for (int i=0,e=m_windows.size();i<e;++i)
    {
        PopupWindow *w = m_windows[i];
        if (!w->canMove()) continue;
        SharingWindow sw = w->getPosition();
        if (m_shared.tryMoveWindow(&sw, m_workingarea, 4))
            w->moveTo(sw);
        else
            w->wait();
    }

    std::vector<PopupWindow*> next;
    for (int i=0,e=m_windows.size();i<e;++i)
    {
        PopupWindow *w = m_windows[i];
        w->tick(i); //todo!
        if (!w->isAnimated()) {
            const SharingWindow *sw = &w->getPosition(); 
            m_shared.deleteWindow(sw);
            freeWindow(w);
        } else {
          next.push_back(w);
        }
    }
    m_windows.swap(next);
}


/*void TrayMainObject::onFinishedAnimation(PopupWindow *w)
{
    int index = -1;
    for (int i=0,e=m_windows.size(); i<e; ++i) {
        if (m_windows[i] == w) { index = i; break; }
    }
    if (index == -1) { assert(false); }
    else { m_windows.erase(m_windows.begin() + index); }
    freeWindow(w);
    if (m_windows.empty())
        tryShowQueue();
    else
        tryRunMoveAnimation();
}

void TrayMainObject::onFinishedMoveAnimation(PopupWindow *w)
{
    int last = m_windows.size() - 1;
    if (m_windows[last] != w)
        return;
    tryShowQueue();
}

void TrayMainObject::onFinishedStartAnimation(PopupWindow *w)
{
    tryRunMoveAnimation();
}

void TrayMainObject::tryRunMoveAnimation()
{
    for (int i=0,e=m_windows.size(); i<e; ++i)
    {
        int at = m_windows[i]->getAnimationState();
        if (at == PopupWindow::ANIMATION_TOSTART || at == PopupWindow::ANIMATION_TOEND)
             return;
    }
    //POINT p = { m_workingarea.right, m_workingarea.bottom };


    for (int i=0,e=m_windows.size(); i<e; ++i)
    {
         PopupWindow* w = m_windows[i];
         //SIZE sz = w->getSize();
         //POINT w_pos = w->getAnimation().pos;
         
         const Animation &a = w->getAnimation();





         p.y -= (sz.cy+4);
         if (w_pos.x != p.x || w_pos.y != p.y)
         {
             MoveAnimation ma;
             ma.pos.y = p.y;
             ma.pos.x = w_pos.x;
             ma.speed = 0.002f;
             w->startMoveAnimation(ma);
         }
    }
}*/

PopupWindow* TrayMainObject::getFreeWindow()
{
   if (!m_free_windows.empty())
   {
       int last = m_free_windows.size() - 1;
       PopupWindow *wnd = m_free_windows[last];
       m_free_windows.pop_back();
       return wnd;
   }
   PopupWindow *wnd = new PopupWindow();
   if (!wnd->create(&m_font))
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

void TrayMainObject::getWorkingArea(RECT *working_area)
{
    POINT rb =  GetTaskbarRB();
    working_area->left = 0;
    working_area->right = rb.x;
    working_area->top = GetSystemMetrics(SM_CYSCREEN) / 6;
    working_area->bottom = rb.y;
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

/*bool TrayMainObject::isHeightLimited() const
{
    if (m_windows.empty())
        return false;
    int last = m_windows.size() - 1;
    const POINT &p = m_windows[last]->getAnimation().pos;
    return(p.y < getHeightLimit()) ? true : false;
    
    return false;
}

int TrayMainObject::getHeightLimit() const
{
    return (GetSystemMetrics(SM_CYSCREEN) * 2) / 10;
}*/

TraySettings& TrayMainObject::traySettings()
{
   return m_settings;
}

void TrayMainObject::startTimer()
{
    if (m_timerStarted) return;
    const DWORD one_min = 60000;
    SetTimer(2, m_settings.interval*one_min);
    m_timerStarted = true;
}

void TrayMainObject::stopTimer()
{
    if (!m_timerStarted) return;
    KillTimer(2);
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
