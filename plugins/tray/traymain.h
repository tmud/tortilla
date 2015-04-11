#pragma once

#include "popupWindow.h"
#include "timeoutWindow.h"

class TrayMainObject
{
public:
    TrayMainObject();
    ~TrayMainObject();
    void setFont(HFONT font);
    void setAlarmWnd(HWND wnd);
    bool showMessage(const u8string& msg, COLORREF bkgnd, COLORREF text);
    void setActivated(bool activated);

private:
    void startAnimation(int window_index, const u8string&msg, COLORREF bkgnd, COLORREF text);

private:
    CFont m_font;
    std::vector<PopupWindow*> m_popups;
    TimeoutWindow m_timeout_wnd;
    bool m_activated;
    PopupWindow* getWindow(int index) const;
    POINT GetTaskbarRB();
};
