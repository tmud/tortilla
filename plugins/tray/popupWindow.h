#pragma once
#include "common.h"
#include "sharingData.h"

struct Msg {
    Msg() : textcolor(0), bkgndcolor(0) {}
    std::wstring text;
    COLORREF textcolor;
    COLORREF bkgndcolor;
};

/*struct NotifyParams {
    NotifyParams() : wnd(0), msg(0), wparam(0){}
    HWND wnd;
    UINT msg;
    WPARAM wparam;
    //LPARAM - parameter
};*/

struct Animation
{
    enum AnimationState { ANIMATION_NONE, ANIMATION_FADE_UP, ANIMATION_FADE_DOWN, ANIMATION_MOVE, ANIMATION_WAIT };
    Animation() : state(ANIMATION_NONE), speed(0), move_speed(0), wait_sec(0), bkgnd_color(0), text_color(0) {}
    SharingWindow pos;
    AnimationState state;
    float speed;
    float move_speed;
    int   wait_sec;
    COLORREF bkgnd_color;
    COLORREF text_color;
    //NotifyParams notify;
};

class PopupWindow : public CWindowImpl<PopupWindow>
{
    Ticker m_ticker;    
    std::wstring m_original_text;
    std::vector<std::wstring> m_text;
    CFont *m_font;
    TempDC m_src_dc;
    Animation m_animation;
    SharingWindow m_destination;
    int wait_timer;
    float alpha, m_move_dx, m_move_dy;

public:
    DECLARE_WND_CLASS(NULL)
    PopupWindow() : m_font(NULL),
        wait_timer(0), alpha(0), m_move_dx(0), m_move_dy(0) {}
    ~PopupWindow() { if (IsWindow()) DestroyWindow();  }
    const SharingWindow* getPosition() { return &m_animation.pos; }
    const SharingWindow* getDestination() { return &m_destination; }
    bool create(CFont *font);
    void startAnimation(const SharingWindow& startpos);
    void setText(const Msg& msg, int timeout);
    void tick(int id);
    bool canMove() const;
    void moveTo(const SharingWindow& pos);
    void wait();
    bool isAnimated() const;

private:
    void onTimer();
    void fillSrcDC();
    SIZE calcDCSize();
    void setAlpha(float a);
    void onClickButton();
    //void sendNotify(int state);
    void trimleft(std::wstring* s);
    SIZE getSize() const;
private:
    BEGIN_MSG_MAP(PopupWindow)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnClick)
    END_MSG_MAP()
    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&) { onClickButton(); return 0;  }
};
