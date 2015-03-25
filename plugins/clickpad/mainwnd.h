#include "padbutton.h"

class ClickpadMainWnd : public CWindowImpl < ClickpadMainWnd >
{
public:
    ClickpadMainWnd() {}
    ~ClickpadMainWnd() {}
    
private:
    BEGIN_MSG_MAP(ClickpadMainWnd)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_USER, OnParentSetFocus)
        //MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) { onCreate(); return 0; }
    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) { m_hWnd = NULL; return 0; }
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    LRESULT OnParentSetFocus(UINT, WPARAM, LPARAM, BOOL&) { onSetParentFocus(); return 0; }
    

    void onCreate();
    void onSize();
    void createButton();
    void onSetParentFocus();
    


private:
    std::deque<PadButton*> m_buttons;
};