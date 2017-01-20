#pragma once
#include "common.h"
#include "sharingData.h"

struct Msg {
    Msg() : textcolor(0), bkgndcolor(0) {}
    std::wstring text;
    COLORREF textcolor;
    COLORREF bkgndcolor;
};

struct NotifyParams {
    NotifyParams() : wnd(0), msg(0), wparam(0){}
    HWND wnd;
    UINT msg;
    WPARAM wparam;
    //LPARAM - parameter
};

struct Animation
{
    //enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    //enum { ANIMATION_FINISHED = 0, MOVEANIMATION_FINISHED, STARTANIMATION_FINISHED, CLICK_EVENT };
    enum AnimationState { ANIMATION_NONE, ANIMATION_FADE_UP, ANIMATION_FADE_DOWN, ANIMATION_MOVE };    
    Animation() : state(ANIMATION_NONE), speed(0), wait_sec(0), bkgnd_color(0), text_color(0) {}
    SharingWindow pos;
    AnimationState state;
    float speed;
    int   wait_sec;
    COLORREF bkgnd_color;
    COLORREF text_color;
    NotifyParams notify;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    Ticker m_ticker;    
    std::wstring m_original_text;
    std::vector<std::wstring> m_text;
    CFont *m_font;
    CSize m_dc_size;
    TempDC m_src_dc;
    Animation m_animation;
    int wait_timer;
    float alpha, m_move_dx, m_move_dy;    
public:
    DECLARE_WND_CLASS(NULL)
    //enum { ANIMATION_NONE = 0, ANIMATION_TOEND, ANIMATION_TOSTART, ANIMATION_WAIT, ANIMATION_MOVE };
    //enum { ANIMATION_FINISHED = 0, MOVEANIMATION_FINISHED, STARTANIMATION_FINISHED, CLICK_EVENT };

    PopupWindow() : m_font(NULL),
        wait_timer(0), alpha(0), m_move_dx(0), m_move_dy(0)
    {
    }
    ~PopupWindow() {
        if (IsWindow()) DestroyWindow(); 
    }

    bool create(CFont *font)
    {
        m_font = font;
        Create(GetDesktopWindow(), CWindow::rcDefault, NULL, WS_POPUP, WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_NOACTIVATE);
        return (IsWindow()) ? true : false;
    }

    void setText(const Msg& msg, const NotifyParams& notify, int timeout);

    const SharingWindow& getPosition() { 
        return m_animation.pos;
    }

    /*const SIZE& getSize() const
    {
        CSize sz(m_dc_size);
        sz += CSize(16,12);
        return sz;
    }*/



    /*bool isAnimated() const {  return (m_animation_state==ANIMATION_NONE) ? false : true; }
    int  getAnimationState() const { return m_animation_state; }
    const Animation& getAnimation() const { return m_animation; }
    const MoveAnimation& getMoveAnimation() const { return m_move_animation; }*/
    //void startAnimation(const Animation& a);
    //void startMoveAnimation(const MoveAnimation& a);
    void onTick();
private:
    void onTimer();
    void fillSrcDC();
    void setState(int newstate);
    void calcDCSize();
    void setAlpha(float a);
    void onClickButton();
    void sendNotify(int state);
    void trimleft(std::wstring* s);
private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
    END_MSG_MAP()
    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&) { onClickButton(); return 0;  }
};
