#include "stdafx.h"
#include "popupWindow.h"

void PopupWindow::onCreate()
{
    calcDCSize();
}

void PopupWindow::onTimer()
{
    if (m_animation_state == ANIMATION_NONE)
    {
        assert(false);
        return;
    }

    DWORD dt = m_ticker.getDiff();
    m_ticker.sync();
    if (m_animation_state == ANIMATION_WAIT)
    {        
        wait_timer += dt;        
        int end_timer = m_animation.wait_sec * 1000;
        if (wait_timer >= end_timer)
            setState(ANIMATION_TOSTART);
        return;
    }

    const Animation &a = m_animation;
    pos_x += dx * dt;
    pos_y += dy * dt;

    LONG x = static_cast<LONG>(pos_x);
    LONG y = static_cast<LONG>(pos_y);
    
    bool finished = false;
    if (m_animation_state == ANIMATION_TOEND)
    {
        if ((a.start_pos.x < a.end_pos.x && x >= a.end_pos.x) ||
            (a.start_pos.x > a.end_pos.x && x <= a.end_pos.x))
        {
            pos_x = static_cast<float>(a.end_pos.x);
            finished = true;
        }
        if ((a.start_pos.y < a.end_pos.y && y >= a.end_pos.y) ||
            (a.start_pos.y > a.end_pos.y && y <= a.end_pos.y))
        {
            pos_y = static_cast<float>(a.end_pos.y);
            finished = true;
        }
    }
    
    if (m_animation_state == ANIMATION_TOSTART)
    {
        if ((a.start_pos.x < a.end_pos.x && x <= a.start_pos.x) ||
            (a.start_pos.x > a.end_pos.x && x >= a.start_pos.x))
        {
            pos_x = static_cast<float>(a.start_pos.x);
            finished = true;
        }
        if ((a.start_pos.y < a.end_pos.y && y <= a.start_pos.y) ||
            (a.start_pos.y > a.end_pos.y && y >= a.start_pos.y))
        {
            pos_y = static_cast<float>(a.start_pos.y);
            finished = true;
        }    
    }

    moveWindow();
    if (finished)
    {
        if (m_animation_state == ANIMATION_TOEND)
            setState(ANIMATION_WAIT);
        else if (m_animation_state == ANIMATION_TOSTART)
            setState(ANIMATION_NONE);
    }
}

void PopupWindow::onPaint(HDC dc)
{
    CDCHandle pdc(dc);
    RECT rc;
    GetClientRect(&rc);
    pdc.FillSolidRect(&rc, m_animation.bkgnd);
    HFONT old_font = pdc.SelectFont(*m_font);
    pdc.SetBkColor(m_animation.bkgnd);
    pdc.SetTextColor(m_animation.text);
    pdc.DrawText(m_text.c_str(), m_text.length(), &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    pdc.SelectFont(old_font);
}

void PopupWindow::startAnimation(const Animation& a)
{
    m_animation = a;
    setState(ANIMATION_TOEND);
}

void PopupWindow::setState(int newstate)
{
    const Animation &a = m_animation;
    m_animation_state = newstate;
    switch(m_animation_state)
    {
    case ANIMATION_TOEND:
        initAnimationVals();
        ShowWindow(SW_SHOWNOACTIVATE);
        SetTimer(1, a.timer_msec);
    break;
    case ANIMATION_WAIT:
    case ANIMATION_TOSTART:
        initAnimationVals();
    break;
    case ANIMATION_NONE:
        ShowWindow(SW_HIDE);
        KillTimer(1);
    break;
    }
    m_ticker.sync();
}

void PopupWindow::initAnimationVals()
{
    const Animation &a = m_animation;
    float speed = a.speed;
    switch(m_animation_state)
    {
    case ANIMATION_TOEND:
        dx = (a.end_pos.x - a.start_pos.x) * speed;
        dy = (a.end_pos.y - a.start_pos.y) * speed; 
        pos_x = static_cast<float>(a.start_pos.x);
        pos_y = static_cast<float>(a.start_pos.y);
    break;
    case ANIMATION_TOSTART:
        dx = (a.start_pos.x - a.end_pos.x) * speed;
        dy = (a.start_pos.y - a.end_pos.y) * speed;
        pos_x = static_cast<float>(a.end_pos.x);
        pos_y = static_cast<float>(a.end_pos.y);
    break;
    case ANIMATION_WAIT:
        dx = 0;
        dy = 0;
        wait_timer = 0;
    break;
    }
}

void PopupWindow::moveWindow()
{
    const Animation &a = m_animation;
    LONG x = static_cast<LONG>(pos_x);
    LONG y = static_cast<LONG>(pos_y);
    RECT pos = { x, y, x + a.window_size.cx, y + a.window_size.cy };
    MoveWindow(&pos);
}

void PopupWindow::calcDCSize()
{
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(*m_font);
    GetTextExtentPoint32(dc, m_text.c_str(), m_text.length(), &m_dc_size);
    dc.SelectFont(oldfont);
}
