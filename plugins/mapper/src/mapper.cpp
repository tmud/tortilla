#include "stdafx.h"
#include "mapper.h"
#include "mapperObjects.h"

void Mapper::onCreate()
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    m_toolbar.Create(m_hWnd, rcDefault, style);

    RECT rc;
    m_toolbar.GetClientRect(&rc);
    m_toolbar_height = rc.bottom;

    GetClientRect(&rc);
    rc.top = m_toolbar_height;
    m_vSplitter.Create(m_hWnd, rc, NULL, 0, WS_EX_CLIENTEDGE);
    m_vSplitter.m_cxySplitBar = 3;      
    m_vSplitter.SetSplitterRect();
    m_vSplitter.SetDefaultSplitterPos();

    RECT pane_left, pane_right;
    m_vSplitter.GetSplitterPaneRect(0, &pane_left); pane_left.right -= 3;
    m_vSplitter.GetSplitterPaneRect(1, &pane_right);

    m_zones_control.Create(m_vSplitter, pane_left, style);
    m_view.Create(m_vSplitter, pane_right, NULL, style | WS_VSCROLL | WS_HSCROLL);
    m_vSplitter.SetSplitterPanes(m_zones_control, m_view);
//    m_zones_control.setNotifications(m_hWnd, WM_USER);
}

void Mapper::onSize()
{
    RECT rc;
    GetClientRect(&rc);
    int height = rc.bottom; rc.bottom = m_toolbar_height;
    m_toolbar.MoveWindow(&rc);
    rc.top = rc.bottom; rc.bottom = height;
    m_vSplitter.MoveWindow(&rc, FALSE);
    m_vSplitter.SetSplitterRect();
}

void Mapper::onZoneChanged()
{
    //todo
    /*Zone *zone = m_zones_control.getCurrentZone();
    if (zone)
        setCurrentLevel(zone->getDefaultLevel());*/
}
