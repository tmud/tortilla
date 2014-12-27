#include "stdafx.h"
#include "mapper.h"
#include "mapperObjects.h"

Mapper::Mapper(PropertiesMapper *props) : m_propsData(props), m_lastDir(-1), 
m_pCurrentRoom(NULL), m_pCurrentLevel(NULL)
{
}

Mapper::~Mapper()
{
    auto_delete<Zone>(m_zones);
}

void Mapper::processNetworkData(const wchar_t* text, int text_len)
{
    RoomData room;
    if (!m_processor.processNetworkData(text, text_len, &room))
    {
        if (m_prompt.processNetworkData(text, text_len))
            popDir();
        return;
    }
    popDir();
    processData(room);
}

void Mapper::processCmd(const wchar_t* text, int text_len)
{
    tstring cmd(text, text_len);
    if (cmd.empty())
        return;
    int dir = -1;
    if (cmd == m_propsData->north_cmd)
        dir = RD_NORTH;
    else if (cmd == m_propsData->south_cmd)
        dir = RD_SOUTH;
    else if (cmd == m_propsData->west_cmd)
        dir = RD_WEST;
    else if (cmd == m_propsData->east_cmd)
        dir = RD_EAST;
    else if (cmd == m_propsData->up_cmd)
        dir = RD_UP;
    else if (cmd == m_propsData->down_cmd)
        dir = RD_DOWN;
    if (dir != -1)
        m_path.push_back(dir);
}

void Mapper::popDir()
{
    if (m_path.empty())
        m_lastDir = -1;
    else 
    {
        m_lastDir = *m_path.begin();
        m_path.pop_front();
    }
}

void Mapper::updateProps()
{
    m_processor.updateProps(m_propsData);
    m_prompt.updateProps(m_propsData);
}

void Mapper::redrawPosition()
{
    if (m_pCurrentRoom)
        m_view.setCurrentRoom(m_pCurrentRoom);
    else/* if (m_pCurrentLevel)*/
        m_view.setCurrentLevel(m_pCurrentLevel);

    Zone *newzone = (m_pCurrentLevel) ? m_pCurrentLevel->getZone() : NULL;    
    m_zones_control.zoneChanged(newzone);
}

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
    m_zones_control.setNotifications(m_hWnd, WM_USER);
    updateProps();    

    // select zone/level to view  //todo save last zone at last session
    for (int i = 0, e = m_zones.size(); i < e; ++i)
    {
        Zone *zone = m_zones[i];
        if (zone->isEmpty())
            continue;
        int from = zone->params().minlevel;
        int to = zone->params().maxlevel;       
        for (int j = from; j <= to; ++j)
        {
            m_pCurrentLevel = zone->getLevel(j);
            if (m_pCurrentLevel && m_pCurrentLevel->getWidth() > 0 && m_pCurrentLevel->getHeight() > 0)
                break;
        }
    }
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
    Zone *zone = m_zones_control.getCurrentZone();
    if (zone)
        m_pCurrentLevel = zone->getDefaultLevel();    
    redrawPosition();
}
