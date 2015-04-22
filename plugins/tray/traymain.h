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
       MESSAGE_HANDLER(WM_USER, OnPopupEvent)
    END_MSG_MAP()
    LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&) { onTimer(); return 0; }
    LRESULT OnPopupEvent(UINT, WPARAM wparam, LPARAM lparam, BOOL&) 
    {
        PopupWindow *w = (PopupWindow*)wparam;
        if (lparam == PopupWindow::ANIMATION_FINISHED)
            onFinishedAnimation(w);
        if (lparam == PopupWindow::MOVEANIMATION_FINISHED)
            onFinishedMoveAnimation(w);
        return 0; 
    }
    void startTimer();
    void stopTimer();
    void onTimer();
    void onFinishedAnimation(PopupWindow *w);
    void onFinishedMoveAnimation(PopupWindow *w);
    PopupWindow* getFreeWindow();
    void freeWindow(PopupWindow *w);
    POINT GetTaskbarRB();
    bool isHeightLimited() const;
    int  getHeightLimit() const;
private:
    CFont m_font;
    std::vector<PopupWindow*> m_windows;
    std::vector<PopupWindow*> m_free_windows;
    TraySettings m_settings;
    bool m_activated;
    bool m_timerStarted;
    HWND m_alarmWnd;
    POINT m_point0;
    std::deque<u8string> m_cache;
};
