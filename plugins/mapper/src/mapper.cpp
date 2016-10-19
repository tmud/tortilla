#include "stdafx.h"
#include "mapper.h"
#include "mapperObjects.h"
#include "debugHelpers.h"

Mapper::Mapper(PropertiesMapper *props) : m_propsData(props), 
m_lastDir(RD_UNKNOWN), m_pCurrentRoom(NULL), m_nextzone_id(1)
{
}

Mapper::~Mapper()
{
    std::for_each(m_zones.begin(),m_zones.end(),[](std::pair<const tstring, Zone*> &p){delete p.second;});
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

    Room *new_room = findRoom(room);
    if (!new_room)
    {
        new_room = new Room();
        new_room->roomdata = room;
        checkExits(new_room);
        if (!m_pCurrentRoom)
        {
            createNewZone(new_room);
        }
        else 
        {
            RoomCursor c(m_pCurrentRoom);
            if (!c.isValid(m_lastDir))
            {
                delete new_room;
                new_room = NULL;
                assert(false);
            }
            else
            {
                if (c.getRoom(m_lastDir))
                {
                    // конфликт -> новая зона
                    createNewZone(new_room);
                    c.addLink(m_lastDir, new_room);
                }
                else if (!c.addRoom(m_lastDir, new_room))
                {
                    delete new_room;
                    new_room = NULL;
                    assert(false);
                }
            }
        }
        if (new_room)
            m_rooms[room.vnum] = new_room;
    }
    else
    {
        RoomCursor c(m_pCurrentRoom);
        c.addLink(m_lastDir, new_room);
    
    }
    m_pCurrentRoom = new_room;

    /*bool cached = m_cache.isExistRoom(m_pCurrentRoom);

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
    m_pCurrentRoom = new_room;*/
    redrawPosition(RCC_NORMAL);
}

Zone* Mapper::getZone(const RoomData& room)
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
}

void Mapper::createNewZone(Room *room)
{
    assert(!room->level);
    tchar buffer[32];
    while (true)
    {
        swprintf(buffer, L"Новая зона %d", m_nextzone_id++);
        zone_iterator zt = m_zones.find(buffer);
        if (zt == m_zones.end())
            break;
    }
    tstring zone_name(buffer);
    RoomCursorNewZone nz;
    m_zones[zone_name] = nz.createNewZone(zone_name, room);

}

RoomDir MapperDirCommand::check(const tstring& cmd) const
{
    int size = cmd.size();
    if (size < main_size) return RD_UNKNOWN;
    if (size == main_size)
        return (cmd == main) ? dir : RD_UNKNOWN;
    tstring main_part(cmd.substr(0, main_size));
    if (main_part != main) return RD_UNKNOWN;
    if (rel.empty()) return RD_UNKNOWN;
    tstring rel_part(cmd.substr(main_size));
    int rel_part_size = rel_part.size();
    if (rel_part_size > rel_size) return RD_UNKNOWN;
    return rel.find(rel_part)==0 ? dir : RD_UNKNOWN;
}

void Mapper::processCmd(const tstring& cmd)
{
    if (cmd.empty())
        return;
    RoomDir dir = RD_UNKNOWN;
    for (int i=0,e=m_dirs.size();i<e;++i)
    {
        dir = m_dirs[i].check(cmd);
        if (dir != -1) { m_path.push_back(dir); break; }
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

Room* Mapper::findRoom(const RoomData& room)
{
    room_iterator rt = m_rooms.find(room.vnum);
    return (rt != m_rooms.end()) ? rt->second : NULL;
}


/*

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
        //todo! m_cache.deleteRoom(m_pCurrentRoom);
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

    //todo! m_cache.deleteRoom(m_pCurrentRoom);
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
    } //todo!

    // Add new room in cache - will it checking later
    Room* new_room = addNewRoom(room);
    //todo! m_cache.addRoom(new_room);
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
                //todo! m_table.deleteRoom(new_room);
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
    //todo! m_table.addRoom(new_room);
    return new_room;
}

void Mapper::deleteRoom(Room* room)
{
    //todo! m_table.deleteRoom(room);
    for (int i=0,e=ROOM_DIRS_COUNT; i<e; ++i)
    {
        Room *next = room->dirs[i].next_room;
        if (next)
            next->dirs[revertDir(i)].next_room = NULL;
    }
    room->level->deleteRoom(room->x, room->y);
    delete room;
}*/

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

void Mapper::checkExits(Room *room)
{
    // parse room->roomdata.exits to room->dirs
    checkExit(room,  RD_NORTH, m_propsData->north_exit);
    checkExit(room,  RD_SOUTH, m_propsData->south_exit);
    checkExit(room,  RD_WEST, m_propsData->west_exit);
    checkExit(room,  RD_EAST, m_propsData->east_exit);
    checkExit(room,  RD_UP, m_propsData->up_exit);
    checkExit(room,  RD_DOWN, m_propsData->down_exit);
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

/*Room* Mapper::getNextRoom(Room *room, int dir)
{
    RoomCursor rc;
    rc.current_room = room;
    rc.move(dir);
    return rc.getOffsetRoom();
}*/

class InitDirVector {
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
};

void Mapper::updateProps()
{
    m_processor.updateProps(m_propsData);
    m_prompt.updateProps(m_propsData);
    m_dark.updateProps(m_propsData);
    InitDirVector h(m_dirs);
    h.make(RD_NORTH, m_propsData->north_cmd);
    h.make(RD_SOUTH, m_propsData->south_cmd);
    h.make(RD_WEST, m_propsData->west_cmd);
    h.make(RD_EAST, m_propsData->east_cmd);
    h.make(RD_UP, m_propsData->up_cmd);
    h.make(RD_DOWN, m_propsData->down_cmd);
}

void Mapper::redrawPosition(ViewCursorColor cursor)
{
    ViewMapPosition vp;
    vp.cursor = cursor;
    vp.room = m_pCurrentRoom;
    m_view.roomChanged(vp);
    //m_zones_control.roomChanged(m_viewpos);
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
//    m_zones_control.setNotifications(m_hWnd, WM_USER);

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
    /*m_viewpos.reset();
    Zone *zone = m_zones_control.getCurrentZone();
    if (zone)
        m_viewpos.level = zone->getDefaultLevel();
    redrawPosition();*/
}
