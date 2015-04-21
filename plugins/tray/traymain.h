#pragma once

#include "popupWindow.h"
#include "traySettings.h"

class TrayMainObject : public CWindowImpl < TrayMainObject >
{
public:
    DECLARE_WND_CLASS(NULL)
    TrayMainObject() :  m_activated(false), m_timerStarted(false), m_alarmWnd(NULL) {}
    ~TrayMainObject();
    void create();
    void setFont(HFONT font);
    void setAlarmWnd(HWND wnd);
    bool showMessage(const u8string& msg);
    void setActivated(bool activated);
    TraySettings& traySettings();

private:
    BEGIN_MSG_MAP(TimeoutWindow)
       MESSAGE_HANDLER(WM_TIMER, OnTimer)
       MESSAGE_HANDLER(WM_USER, OnFinishedAnimation)
    END_MSG_MAP()
    LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&) { onTimer(); return 0; }
    LRESULT OnFinishedAnimation(UINT, WPARAM wparam, LPARAM, BOOL&) { onFinishedAnimation((PopupWindow*)wparam); return 0; }

    void startTimer();
    void stopTimer();
    void onTimer();
    void onFinishedAnimation(PopupWindow *w);
    PopupWindow* getFreeWindow();
    void freeWindow(PopupWindow *w);
    POINT GetTaskbarRB();
private:
    CFont m_font;
    std::vector<PopupWindow*> m_windows;
    std::vector<PopupWindow*> m_free_windows;
    TraySettings m_settings;
    bool m_activated;
    bool m_timerStarted;
    HWND m_alarmWnd;
    POINT m_point0;
};
