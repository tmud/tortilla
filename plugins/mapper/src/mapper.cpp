#include "stdafx.h"
#include "mapper.h"
#include "mapperObjects.h"
#include "debugHelpers.h"

Mapper::Mapper(PropertiesMapper *props) : m_propsData(props), m_lastDir(-1), m_pCurrentRoom(NULL)
{
}

Mapper::~Mapper()
{
    autodel<Zone> z(m_zones);
}

void Mapper::processNetworkData(const tchar* text, int text_len)
{
    RoomData room;
    if (!m_processor.processNetworkData(text, text_len, &room))
    {
        if (m_prompt.processNetworkData(text, text_len))
            popDir();
        DEBUGOUT(L"------");
        DEBUGOUT(L"Не распознано");
        return;
    }
    if (room.descr.empty())
        room.descr.append(L"empty");
    else
    {
        tstring &dark = m_propsData->dark_room;
        if (!dark.empty() && dark == room.descr)
            room.descr.clear();
    }
    room.calcHash();
    popDir();

    DEBUGOUT(L"------");
    DEBUGOUT(room.name);
    DEBUGOUT(room.vnum);
    DEBUGOUT(room.exits);
    //DEBUGOUT(room.descr);
    
    bool cached = m_cache.isExistRoom(m_pCurrentRoom);

    // can be return NULL (if cant find or create new) so it is mean -> lost position
    Room* new_room = (cached) ? findRoomCached(room) : findRoom(room);
    if (!new_room)
    {
        if (m_lastDir == -1)
        {
            m_rpos.reset();
            m_pCurrentRoom = NULL;
        }
        else
        {
            if (m_pCurrentRoom)
            {
                m_rpos.reset();
                m_rpos.current_room = m_pCurrentRoom;
            }
            if (m_rpos.current_room)
                m_rpos.move(m_lastDir);
        }
    }
    else
    {
        if (m_pCurrentRoom) 
        {
            RoomExit &e = m_pCurrentRoom->dirs[m_lastDir];
            if (!e.next_room)
                e.next_room = new_room;
        }

        m_rpos.reset();
        m_rpos.current_room = m_pCurrentRoom;
        m_rpos.new_room = new_room;
    }

    Room* croom = m_rpos.new_room ? m_rpos.new_room : m_rpos.current_room;    
    m_viewpos.room = croom;
    m_viewpos.level = croom->level;
    redrawPosition();
    m_pCurrentRoom = new_room;
}

void Mapper::processCmd(const tstring& cmd)
{
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
    else {
        m_lastDir = m_path[0];
        m_path.erase(m_path.begin());
    }
}

Zone* Mapper::addNewZone()
{
    ZoneParams zp;
    WCHAR buffer[32];
    for (int i = 1;; ++i)
    {
        swprintf(buffer, L"Новая зона %d", i);
        bool found = false;
        for (int j = 0, e = m_zones.size(); j < e; ++j)
        {
            m_zones[j]->getParams(&zp);
            if (!zp.name.compare(buffer)) { found = true; break; }
        }
        if (!found)
            break;
    }

    Zone *new_zone = new Zone(buffer);
    m_zones.push_back(new_zone);
    return new_zone;
}

Room* Mapper::findRoomCached(const RoomData& room)
{
    if (m_lastDir == -1)
        return findRoom(room);

    Room* next = m_pCurrentRoom->dirs[m_lastDir].next_room;
    if (next)
    {  if (room.dhash && room.equal(next->roomdata))
          return next;
       if (!room.dhash && room.similar(next->roomdata))
          return next;
       return addNewRoom(room);
    }

    // get dir from room entered
    int from = -1;
    for (int i=0,e=ROOM_DIRS_COUNT; i<e; ++i)
    {
        if (m_pCurrentRoom->dirs[i].next_room)
            { from = i; break; }
    }
    if (from == -1) { assert(false); return NULL; }
    int revertFrom = revertDir(from);
    int revert = revertDir(m_lastDir);

    // get all rooms as cached
    int index = -1;
    std::vector<Room*> vr;
    m_table.findRooms(m_pCurrentRoom->roomdata, &vr);
    int size = vr.size();    
    for (int i=0; i<size; ++i)
    {
        if (vr[i] == m_pCurrentRoom)
            continue;
        if (vr[i]->dirs[from].next_room)
            continue;
        Room* next = vr[i]->dirs[m_lastDir].next_room;
        if (next && next->roomdata.equal(room))
        {
            Room* back = next->dirs[revert].next_room;
            if (back && back->roomdata.equal(m_pCurrentRoom->roomdata))
                { index = i; break; }
        }
    }

    if (index != -1)
    {
        m_cache.deleteRoom(m_pCurrentRoom);
        Room* new_current = vr[index];
        Room *f = m_pCurrentRoom->dirs[from].next_room;
        deleteRoom(m_pCurrentRoom);
        f->dirs[revertFrom].next_room = new_current;
        new_current->dirs[from].next_room = f;
        return new_current->dirs[m_lastDir].next_room;
    }

    // check neighborhood room by coordinates
    {
        RoomCursor pos;
        pos.current_room = m_pCurrentRoom;
        pos.move(m_lastDir);
        Room *next = pos.getOffsetRoom();
        if (next && !next->dirs[revert].next_room && 
            next->roomdata.equal(room))
        {
            m_pCurrentRoom->dirs[m_lastDir].next_room = next;
            next->dirs[revert].next_room = m_pCurrentRoom;
            return next;
        }
    }

    m_cache.deleteRoom(m_pCurrentRoom);
    return addNewRoom(room);
}

Room* Mapper::findRoom(const RoomData& room)
{
    // find current/new position
    if (m_pCurrentRoom)
    {
        // check neighborhood room
        if (m_lastDir != -1)
        {
            Room* next = m_pCurrentRoom->dirs[m_lastDir].next_room;
            if (next)
            {  if (room.dhash && room.equal(next->roomdata))
                   return next;
               if (!room.dhash && room.similar(next->roomdata))
                   return next;
            }
        }
        else
        {
            if (room.dhash && room.equal(m_pCurrentRoom->roomdata))
                return m_pCurrentRoom;
            if (!room.dhash && room.similar(m_pCurrentRoom->roomdata))
                return m_pCurrentRoom;
        }
    }

    if (!room.dhash)
        return NULL;            // unknown position

    // get all rooms
    std::vector<Room*> vr;
    m_table.findRooms(room, &vr);
    int size = vr.size();

    if (size == 0)
        return addNewRoom(room);

    if (m_lastDir == -1 || !m_pCurrentRoom)
    {
        if (size == 1)
            return vr[0];
        return NULL;            // unknown position
    }

    // try to find room (check back exit)
    int revert = revertDir(m_lastDir);
    for (int i=0; i<size; ++i)
    {
       if (vr[i]->dirs[revert].next_room == m_pCurrentRoom)
       {
           return vr[i];
       }
    }
    if (!m_pCurrentRoom->dirs[m_lastDir].next_room)
    {
       Room *next = getNextRoom(m_pCurrentRoom, m_lastDir);
       if (next && next->roomdata.equal(room) && !next->dirs[revert].next_room)
       {
           m_pCurrentRoom->dirs[m_lastDir].next_room = next;
           next->dirs[revert].next_room = m_pCurrentRoom;
           return next;
       }
    }

    if (size == 1)
    {
        Room *next = vr[0];
        if (!next->dirs[revert].next_room &&
            !m_pCurrentRoom->dirs[m_lastDir].next_room)
        {
           m_pCurrentRoom->dirs[m_lastDir].next_room = next;
           next->dirs[revert].next_room = m_pCurrentRoom;
           return next;
        }
    } //todo

    // Add new room in cache - will it checking later
    Room* new_room = addNewRoom(room);
    m_cache.addRoom(new_room);
    return new_room;
}

Room* Mapper::addNewRoom(const RoomData& room)
{
    if (!m_pCurrentRoom || m_lastDir == -1)
    {
        if (!m_rpos.current_room || m_lastDir == -1)  // last known room
        {
            // new zone, level and room
            Zone *new_zone = addNewZone();
            m_zones_control.addNewZone(new_zone);
            RoomsLevel *level = new_zone->getLevel(0, true);
            Room *new_room = createNewRoom(room);
            level->addRoom(new_room, 0, 0);
            return new_room;
        }
        else
        {
            Room *new_room = createNewRoom(room);
            if (!m_rpos.setOffsetRoom(new_room))
            {
                m_table.deleteRoom(new_room);
                delete new_room;
                return NULL;
            }
            return new_room;
        }
    }

    // add another room in last direction
    Room *new_room = createNewRoom(room);
    RoomExit &e = m_pCurrentRoom->dirs[m_lastDir];
    if (e.next_room)
    {
        // exit with different rooms ? -> make new zone
        Zone *new_zone = addNewZone();
        m_zones_control.addNewZone(new_zone);
        RoomsLevel *level = new_zone->getLevel(0, true);
        level->addRoom(new_room, 0, 0);
        return new_room;
    }

    RoomCursor pos; pos.current_room = m_pCurrentRoom;
    pos.move(m_lastDir);
    pos.setOffsetRoom(new_room);
    e.next_room = new_room;
    new_room->dirs[revertDir(m_lastDir)].next_room = m_pCurrentRoom;
    return new_room;
}

Room* Mapper::createNewRoom(const RoomData& room)
{
    Room *new_room = new Room();
    new_room->roomdata = room;    
    checkExits(new_room);
    m_table.addRoom(new_room);
    return new_room;
}

void Mapper::deleteRoom(Room* room)
{
    m_table.deleteRoom(room);
    for (int i=0,e=ROOM_DIRS_COUNT; i<e; ++i)
    {
        Room *next = room->dirs[i].next_room;
        if (next)
            next->dirs[revertDir(i)].next_room = NULL;
    }
    room->level->deleteRoom(room->x, room->y);
    delete room;
}

void Mapper::checkExits(Room *room)
{
    // parse room->roomdata.exits to room->dirs
    tstring e = room->roomdata.exits;
    if (e.find(m_propsData->north_exit) != -1)
        room->dirs[RD_NORTH].exist = true;
    if (e.find(m_propsData->south_exit) != -1)
        room->dirs[RD_SOUTH].exist = true;
    if (e.find(m_propsData->west_exit) != -1)
        room->dirs[RD_WEST].exist = true;
    if (e.find(m_propsData->east_exit) != -1)
        room->dirs[RD_EAST].exist = true;
    if (e.find(m_propsData->up_exit) != -1)
        room->dirs[RD_UP].exist = true;
    if (e.find(m_propsData->down_exit) != -1)
        room->dirs[RD_DOWN].exist = true;
}

int Mapper::revertDir(int dir)
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
}

Room* Mapper::getNextRoom(Room *room, int dir)
{
    RoomCursor rc;
    rc.current_room = room;
    rc.move(dir);
    return rc.getOffsetRoom();
}

void Mapper::updateProps()
{
    m_processor.updateProps(m_propsData);
    m_prompt.updateProps(m_propsData);
}

void Mapper::redrawPosition()
{
    m_zones_control.roomChanged(m_viewpos);
    m_view.roomChanged(m_viewpos);
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
    m_vSplitter.Create(m_hWnd, rc);
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
    m_viewpos.reset();
    Zone *zone = m_zones_control.getCurrentZone();
    if (zone)
        m_viewpos.level = zone->getDefaultLevel();
    redrawPosition();
}
