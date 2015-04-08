#pragma once

#include "popupWindow.h"

class TrayMainObject
{
public:
    void setFont(HFONT font);
    void showMessage(const u8string& msg);
    
private:
    void createWindow(const u8string& msg);
    void startAnimation(int window_index);
    void stopAnimation(int window_index);

private:
    CFont m_font;
    std::vector<PopupWindow*> m_popups;
    PopupWindow* getWindow(int index) const;
    POINT GetTaskbarRB();
};
