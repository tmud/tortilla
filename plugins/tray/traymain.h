#pragma once
#include "popupWindow.h"
#include "traySettings.h"
#include "sharingController.h"

class TrayMainObject : public CWindowImpl < TrayMainObject >
{
public:
    DECLARE_WND_CLASS(NULL)
    TrayMainObject() :  m_activated(false), m_timerStarted(false), m_alarmWnd(NULL) {}
    ~TrayMainObject();
    bool create();
    void setFont(HFONT font);
    void setAlarmWnd(HWND wnd);
    void setActivated(bool activated);
    TraySettings& traySettings();
    void addMessage(const Msg& m);
private:
    BEGIN_MSG_MAP(TimeoutWindow)
       MESSAGE_HANDLER(WM_TIMER, OnTimer)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM wparam, LPARAM lparam, BOOL&) 
    {
        SetTimer(1, 200);
        return 0;
    }
    LRESULT OnDestroy(UINT, WPARAM wparam, LPARAM lparam, BOOL&) 
    {
        KillTimer(1);
        return 0;
    }
    LRESULT OnTimer(UINT, WPARAM wparam, LPARAM, BOOL&) {
        if (wparam==1) onTickPopups();
        if (wparam==2) onTimer(); 
        return 0; 
    }
    bool showMessage(const Msg& msg);
    void startTimer();
    void stopTimer();
    void onTimer();
    void onTickPopups();
    PopupWindow* getFreeWindow();
    void freeWindow(PopupWindow *w);
    POINT GetTaskbarRB();
    void tryShowQueue();
    void getWorkingArea(RECT *working_area);

    CFont m_font;
    std::vector<PopupWindow*> m_windows;
    std::vector<PopupWindow*> m_free_windows;
    TraySettings m_settings;
    bool m_activated;
    bool m_timerStarted;
    HWND m_alarmWnd;
    RECT m_workingarea;
private:
    std::deque<Msg> m_queue;
    SharingController m_shared;
};
