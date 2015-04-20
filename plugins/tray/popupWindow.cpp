#include "stdafx.h"
#include "popupWindow.h"

void PopupWindow::onCreate()
{
    long l = GetWindowLong(GWL_EXSTYLE);
    l |= WS_EX_LAYERED;
    SetWindowLong(GWL_EXSTYLE, l);
    setAlpha(0);
}

void PopupWindow::onTimer()
{
    DWORD dt = m_ticker.getDiff();
    m_ticker.sync();
    if (m_move_animation_state == ANIMATION_MOVE)
    {    
    }

    if (m_animation_state == ANIMATION_NONE)
    {
        assert(false);
        return;
    }

    if (m_animation_state == ANIMATION_WAIT)
    {        
        wait_timer += dt;
        int end_timer = m_animation.wait_sec * 1000;
        if (wait_timer >= end_timer)
            setState(ANIMATION_TOSTART);
        return;
    }    
    float da = static_cast<float>(dt) * m_animation.speed;
    if (m_animation_state == ANIMATION_TOEND)
    {
        alpha = min(alpha+da, 255);
        setAlpha(alpha);
        if (alpha == 255)
            setState(ANIMATION_WAIT);
    }
    if (m_animation_state == ANIMATION_TOSTART)
    {
        alpha = max(alpha-da, 0);
        setAlpha(alpha);
        if (alpha == 0)
            setState(ANIMATION_NONE);
    }
}

void PopupWindow::startAnimation(const Animation& a)
{
    m_animation = a;
    setState(ANIMATION_TOEND);    
}

void PopupWindow::startMoveAnimation(POINT newpos)
{
    m_moveanimation = newpos;
    m_move_animation_state = ANIMATION_MOVE;
}

void PopupWindow::setState(int newstate)
{
    const Animation &a = m_animation;
    m_animation_state = newstate;
    switch(m_animation_state)
    {
    case ANIMATION_TOEND:
    {
        const SIZE &sz = getSize();
        RECT pos = { a.pos.x, a.pos.y, a.pos.x + sz.cx, a.pos.y + sz.cy };
        MoveWindow(&pos);
        ShowWindow(SW_SHOWNOACTIVATE);
        SetTimer(1, 10);
    }
    break;
    case ANIMATION_NONE:
        setAlpha(0);
        ShowWindow(SW_HIDE);
        KillTimer(1);
        wait_timer = 0;
        alpha = 0;
        if (m_animation.notify_wnd)
            ::PostMessage(m_animation.notify_wnd, m_animation.notify_msg, m_animation.notify_param, 0);
    break;
    }
    m_ticker.sync();
}

void PopupWindow::calcDCSize()
{
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(*m_font);
    GetTextExtentPoint32(dc, m_text.c_str(), m_text.length(), &m_dc_size);
    dc.SelectFont(oldfont);
}

void PopupWindow::onPaint(HDC dc)
{
    CDCHandle pdc(dc);
    RECT rc;
    GetClientRect(&rc);
    pdc.FillSolidRect(&rc, m_animation.bkgnd_color);
    HFONT old_font = pdc.SelectFont(*m_font);
    pdc.SetBkColor(m_animation.bkgnd_color);
    pdc.SetTextColor(m_animation.text_color);
    pdc.DrawText(m_text.c_str(), m_text.length(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    pdc.SelectFont(old_font);

    CPen p;
    p.CreatePen(PS_SOLID, 2, m_animation.text_color);
    HPEN old = pdc.SelectPen(p);
    pdc.MoveTo(rc.left+1, rc.top+1);
    pdc.LineTo(rc.right-1, rc.top+1);
    pdc.LineTo(rc.right-1, rc.bottom-1);
    pdc.LineTo(rc.left+1, rc.bottom-1);
    pdc.LineTo(rc.left+1, rc.top);
}

void PopupWindow::setAlpha(float a)
{
    int va = static_cast<int>(a);
    SetLayeredWindowAttributes(m_hWnd, 0, va, LWA_ALPHA);
}

void PopupWindow::onClickButton()
{
    setState(ANIMATION_TOSTART);
}
