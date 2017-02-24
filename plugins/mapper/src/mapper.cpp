#include "stdafx.h"
#include "mapper.h"
#include "roomObjects.h"
#include "levelZoneObjects.h"
#include "debugHelpers.h"

Mapper::Mapper(PropertiesMapper *props) : m_propsData(props), 
m_lastDir(RD_UNKNOWN), m_pCurrentRoom(NULL)
{
}

Mapper::~Mapper()
{    
}

void Mapper::processNetworkData(const tchar* text, int text_len)
{
    bool in_dark = false;
    RoomData room;
    if (!m_processor.processNetworkData(text, text_len, &room))
    {
        if (m_prompt.processNetworkData(text, text_len))
        {
            popDir();
            if (m_dark.processNetworkData(text, text_len) && m_pCurrentRoom)
            {
                // move in dark to direction
                Room *next = m_pCurrentRoom->dirs[m_lastDir].next_room;
                //if (next)
                {
                    m_pCurrentRoom = next;
                    redrawPosition(RCC_LOST);
                }
            }            
        }
        return;
    }
    else
    {
        if (m_dark.processNetworkData(text, text_len))
            in_dark = true;
    }
    popDir();

    DEBUGOUT(L"------");
    DEBUGOUT(room.name);
    DEBUGOUT(room.vnum);
    DEBUGOUT(room.exits);

    Room *new_room = m_map.findRoom(room.vnum);
    if (!new_room && m_lastDir != RD_UNKNOWN)
    {
        new_room = new Room();
        new_room->roomdata = room;
        setExits(new_room);
        if (!m_pCurrentRoom)
        {
            if (!m_map.addNewZoneAndRoom(new_room))
            {
                delete new_room;
                new_room = NULL;        
            }
        }
        else
        {
            if (!m_map.addNewRoom(m_pCurrentRoom, new_room, m_lastDir))
            {
                delete new_room;
                new_room = NULL;    
            }        
        }
    }
    else
    {        
         m_map.addLink(m_pCurrentRoom, new_room, m_lastDir);
    }
    m_pCurrentRoom = new_room;
    /*if (m_pCurrentRoom && m_pCurrentRoom->level)
        m_zones_control.selectZone(m_pCurrentRoom->level->getZone(), true); */    
    redrawPosition(RCC_NORMAL);
}

/*Zone* Mapper::getZone(const RoomData& room)
{
    if (!room.zonename.empty())
    {
        const tstring& zone_name = room.zonename;
        zone_iterator it = m_zones.find(zone_name);
        if (it != m_zones.end())
            return it->second;
        Zone *new_zone = new Zone(zone_name);
        m_zones[zone_name] = new_zone;
        return new_zone;
    }

    tchar buffer[32];
    while (true)
    {
        swprintf(buffer, L"Новая зона %d", m_nextzone_id++);
        zone_iterator zt = m_zones.find(buffer);
        if (zt == m_zones.end())
            break;
   }
   tstring zone_name(buffer);
   Zone* new_zone = new Zone(zone_name);
   m_zones[zone_name] = new_zone;
   return new_zone;
}*/

void Mapper::processCmd(const tstring& cmd)
{    
    RoomDir dir = RD_UNKNOWN;
    for (int i = 0, e = m_dirs.size(); i < e; ++i)
    {
        dir = m_dirs[i].check(cmd);
        if (dir != RD_UNKNOWN) { m_path.push_back(dir); break; }
    }

#ifdef _DEBUG
    tstring d;
    switch(dir) {
    case RD_NORTH: d.append(L"с"); break;
    case RD_SOUTH: d.append(L"ю"); break;
    case RD_WEST:  d.append(L"з"); break;
    case RD_EAST:  d.append(L"в"); break;
    case RD_UP:    d.append(L"вв"); break;
    case RD_DOWN:  d.append(L"вн"); break;
    }
    if (!d.empty())
    {
        tstring t(L"dir:");
        t.append(d);
        DEBUGOUT(t);
    }
#endif
}

void Mapper::popDir()
{
    if (m_path.empty())
        m_lastDir = RD_UNKNOWN;
    else {
        m_lastDir = *m_path.begin();
        m_path.pop_front();
    }
}

void Mapper::checkExit(Room *room, RoomDir dir, const tstring& exit)
{
    const tstring& e = room->roomdata.exits; 
    if (e.find(exit) != -1) 
    {
        room->dirs[dir].exist = true;
        tstring door(L"(");
        door.append(exit);
        door.append(L")");
        if (e.find(door) != -1)
            room->dirs[dir].door = true;
    }
}

void Mapper::setExits(Room *room)
{
    // parse room->roomdata.exits to room->dirs
    checkExit(room,  RD_NORTH, m_propsData->north_exit);
    checkExit(room,  RD_SOUTH, m_propsData->south_exit);
    checkExit(room,  RD_WEST, m_propsData->west_exit);
    checkExit(room,  RD_EAST, m_propsData->east_exit);
    checkExit(room,  RD_UP, m_propsData->up_exit);
    checkExit(room,  RD_DOWN, m_propsData->down_exit);
}

/*int Mapper::revertDir(int dir)
{
    if (dir == RD_NORTH)
       return RD_SOUTH;
    if (dir == RD_SOUTH)
        return RD_NORTH;
    if (dir == RD_WEST)
        return RD_EAST;
    if (dir == RD_EAST)
        return RD_WEST;
    if (dir == RD_UP)
        return RD_DOWN;
    if (dir == RD_DOWN)
        return RD_UP;
    assert (false);
    return -1;
}*/

/*Room* Mapper::getNextRoom(Room *room, int dir)
{
    RoomCursor rc;
    rc.current_room = room;
    rc.move(dir);
    return rc.getOffsetRoom();
}*/

/*class InitDirVector {
    Pcre r1, r2; DirsVector& m;
 public:
    InitDirVector(DirsVector& p) : m(p) { r1.init(L","); r2.init(L"\\|"); m.clear(); }
    bool make(RoomDir dir, const tstring& key) {
       bool result = true;
       r1.findall(key.c_str());
       int b = 0;
       for (int i=1,ie=r1.size();i<ie;++i) {
           int e = r1.first(i);
           if (!set(dir, key.substr(b, e-b)))
               result = false;
           b=e+1;
       }
       if (!set(dir, key.substr(b)))
           result = false;
       return result;
    }
private:
    bool set(RoomDir dir, const tstring& dkey) {
        if (dkey.empty()) return true;
        if (!r2.find(dkey.c_str())) {
          MapperDirCommand k(dir, dkey, L"");
          m.push_back(k);
          return true;
        }
        if (r2.size()!=1)
            return false;
        int p = r2.first(0);
        tstring main(dkey.substr(0, p));
        tstring rel(dkey.substr(p+1));
        MapperDirCommand k(dir, main, rel);
        m.push_back(k);
        return true;
    }
};*/

void Mapper::updateProps()
{
    m_processor.updateProps(m_propsData);
    m_prompt.updateProps(m_propsData);
    m_dark.updateProps(m_propsData);
    InitDirVector h;
    h.make(RD_NORTH, m_propsData->north_cmd, m_dirs);
    h.make(RD_SOUTH, m_propsData->south_cmd, m_dirs);
    h.make(RD_WEST, m_propsData->west_cmd, m_dirs);
    h.make(RD_EAST, m_propsData->east_cmd, m_dirs);
    h.make(RD_UP, m_propsData->up_cmd, m_dirs);
    h.make(RD_DOWN, m_propsData->down_cmd, m_dirs);
}

void Mapper::redrawPosition(ViewCursorColor cursor)
{
    ViewMapPosition vp;
    vp.cursor = cursor;
    vp.room = m_pCurrentRoom;
    m_view.roomChanged(vp);
    if (vp.room)
    {
        Zone* z = vp.room->level->getZone();
        //ZoneParams zp;
        //z->getParams(&zp);
        //m_zones_control.selectZone(zp.name, true);
    }
}

void Mapper::onCreate()
{
    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    RECT rc;
    GetClientRect(&rc);
    m_vSplitter.Create(m_hWnd, rc);
    m_vSplitter.m_cxySplitBar = 3;

    RECT pane_left, pane_right;
    m_vSplitter.GetSplitterPaneRect(0, &pane_left); pane_left.right -= 3;
    m_vSplitter.GetSplitterPaneRect(1, &pane_right);

    m_zones_control.Create(m_vSplitter, pane_left, style);
    m_container.Create(m_vSplitter, pane_right, L"", style);

    m_toolbar.Create(m_container, rcDefault, style);
    m_view.Create(m_container, rcDefault, NULL, style | WS_VSCROLL | WS_HSCROLL, WS_EX_STATICEDGE);

    m_container.attach(40, m_toolbar, m_view);
    m_vSplitter.SetSplitterPanes(m_zones_control, m_container);

    m_zones_control.setNotifications(m_hWnd, WM_USER);
    m_vSplitter.SetSplitterRect();
    m_vSplitter.SetDefaultSplitterPos();
    updateProps();
}

void Mapper::onSize()
{
    RECT rc;
    GetClientRect(&rc);
    m_vSplitter.MoveWindow(&rc, FALSE);
    m_vSplitter.SetSplitterRect();
}

/*void Mapper::onZoneChanged()
{
    //m_viewpos.reset();
    Zone *zone = m_zones_control.getCurrentZone();
    if (zone)
        
        m_viewpos.level = zone->getDefaultLevel();
    redrawPosition();
    int c=1;
}*/
