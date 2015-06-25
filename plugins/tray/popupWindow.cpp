#include "stdafx.h"
#include "popupWindow.h"
void sendLog(const utf8* msg); //debug

void PopupWindow::onCreate()
{
    long l = GetWindowLong(GWL_EXSTYLE);
    l |= WS_EX_LAYERED;
    SetWindowLong(GWL_EXSTYLE, l);
    setAlpha(0);
}

void PopupWindow::onTick()
{
    DWORD dt = m_ticker.getDiff();
    m_ticker.sync();
    if (m_animation_state == ANIMATION_MOVE)
    {
        POINT &p = m_animation.pos;
        const POINT& target = m_move_animation.pos;

        float x = static_cast<float>(p.x);
        float y = static_cast<float>(p.y);

        float dx = m_move_dx * m_move_animation.speed * dt;
        float dy = m_move_dy * m_move_animation.speed * dt;

        float ax = abs(dx); float ay = abs(dy);
        if (ax > 6 || ay > 6) // || (ax > 0 && ax < 1) || (ay > 0 && ay < 1))
        {
            p.x = target.x;
            p.y = target.y;
        }
        else
        {
            p.x = static_cast<LONG>(x+dx);
            p.y = static_cast<LONG>(y+dy);
        }

        bool stop = false;
        if ((m_move_dx < 0 && p.x <= target.x) || (m_move_dx > 0 && p.x >= target.x))
            { p.x = target.x; stop = true; }
        if ((m_move_dy < 0 && p.y <= target.y) || (m_move_dy > 0 && p.y >= target.y))
            { p.y = target.y; stop = true; }
        if (!stop && p.x == target.x && p.y == target.y)
            { stop = true; }
        SIZE sz = getSize();
        RECT pos = { p.x, p.y, p.x+sz.cx, p.y+sz.cy };
        MoveWindow(&pos);
        if (stop)
        {
            setState(ANIMATION_WAIT);
            sendNotify(MOVEANIMATION_FINISHED);
        }
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
        if (alpha > 255)
        {
            sendLog("error: a > 255");
        }
        setAlpha(alpha);
        if (alpha == 255)
        {
            setState(ANIMATION_WAIT);
            sendNotify(STARTANIMATION_FINISHED);
        }
    }
    if (m_animation_state == ANIMATION_TOSTART)
    {
        alpha = max(alpha-da, 0);
        if (alpha < 0)
        {
            sendLog("error: a < 0");
        }
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

void PopupWindow::startMoveAnimation(const MoveAnimation& a)
{
    const POINT &p = m_animation.pos;
    m_move_dx = static_cast<float>(a.pos.x - p.x);
    m_move_dy = static_cast<float>(a.pos.y - p.y);
    m_move_animation = a;
    setState(ANIMATION_MOVE);
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
    }
    break;
    case ANIMATION_NONE:
        setAlpha(0);
        ShowWindow(SW_HIDE);
        wait_timer = 0;
        alpha = 0;
        sendNotify(ANIMATION_FINISHED);
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
    if (!SetLayeredWindowAttributes(m_hWnd, 0, va, LWA_ALPHA))
    {
        DWORD error = GetLastError(); //debug
        char buffer[128];
        sprintf(buffer, "SLWA error %d,%f", error, a);
        sendLog(buffer);
    }
}

void PopupWindow::onClickButton()
{
    setState(ANIMATION_TOSTART);
}

void PopupWindow::sendNotify(int state)
{
     if (m_animation.notify_wnd)
            ::PostMessage(m_animation.notify_wnd, m_animation.notify_msg, m_animation.notify_param, state);
}