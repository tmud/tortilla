#pragma once

class TimeoutWindow : public CWindowImpl < TimeoutWindow >
{ 
    HWND m_alarm_wnd;
    bool m_started;
public:
    DECLARE_WND_CLASS(NULL)
    TimeoutWindow() : m_alarm_wnd(NULL), m_started(false) {}
    ~TimeoutWindow() { stop(); if (IsWindow()) DestroyWindow(); }
    void setAlarmWnd(HWND alarm_wnd) 
    {
        m_alarm_wnd = alarm_wnd; 
        createMe();
    }

    void start(int interval)
    {
        if (m_started) return;
        createMe();
        const DWORD one_min = 60000;
        SetTimer(1, interval*one_min);
        m_started = true;
    }

    void stop()
    {
        if (!m_started) return;
        KillTimer(1);
        m_started = false;
    }

private:
    void createMe()
    {
        if (!IsWindow())
            Create(NULL, NULL, NULL, WS_POPUP);
    }
    void onTimer()
    {
        if (!::IsWindow(m_alarm_wnd))
            return;
        FLASHWINFO fw;
        fw.cbSize = sizeof(FLASHWINFO);
        fw.hwnd = m_alarm_wnd;
        fw.uCount = 5;
        fw.dwFlags = FLASHW_ALL;
        fw.dwTimeout = 0;
        ::FlashWindowEx(&fw);
    }

private:
    BEGIN_MSG_MAP(TimeoutWindow)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
    END_MSG_MAP()
    LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&) { onTimer(); return 0; }
};
