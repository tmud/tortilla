#pragma once

#include "popupWindow.h"

class TrayMainObject
{
public:
    TrayMainObject();
    ~TrayMainObject();
    void setFont(HFONT font);
    bool showMessage(const u8string& msg, COLORREF bkgnd, COLORREF text);
    
private:
    void startAnimation(int window_index, COLORREF bkgnd, COLORREF text);

private:
    CFont m_font;
    std::vector<PopupWindow*> m_popups;
    PopupWindow* getWindow(int index) const;
    POINT GetTaskbarRB();
};
