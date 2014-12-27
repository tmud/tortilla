#include "stdafx.h"
#include "mapper.h"
#include "helpers.h"
//-------------------------------------------------------------------------------------
void Mapper::processData(const RoomData& room)
{
    std::vector<Room*> rooms;
    findRooms(room, &rooms);

    int count = rooms.size();
    if (count == 0)
    {
        // if cant find or create new, so it is mean -> lost position
        m_pCurrentRoom = NULL;
        redrawPosition();
        return;
    }

    if (count == 1)
    {
        if (m_pCurrentRoom)
        {
            RoomExit &e = m_pCurrentRoom->dirs[m_lastDir];
            if (!e.next_room)
                e.next_room = rooms[0];
        }
        m_pCurrentRoom = rooms[0];
        m_pCurrentLevel = m_pCurrentRoom->level;
        redrawPosition();
        return;
    }

    /*Room* new_room = (cached) ? findRoomCached(room) : findRoom(room);
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
}

void Mapper::findRooms(const RoomData& room, std::vector<Room*> *vr)
{
    assert(vr->empty());
    if (m_lastDir == -1 || !m_pCurrentRoom)
    {
        m_table.findRooms(room, vr);
        if (vr->empty())
        {
            Room *new_room = addNewRoom(room);
            if (new_room)
                vr->push_back(new_room);
        }
        return;
    }

    Room* next = m_pCurrentRoom->dirs[m_lastDir].next_room;
    if (next)
    {
        if (room.dhash && room.equal(next->roomdata))
        {
            vr->push_back(next); return;
        }
        if (!room.dhash && room.similar(next->roomdata))
        {
            vr->push_back(next); return;
        }

        //todo new zone ? rooms conflict, multiexit ?
        Room *new_room = addNewRoom(room);
        if (new_room)
            vr->push_back(new_room);
        return;
    }

    int backDir = revertDir(m_lastDir);

    // check rooms like current and their back directions
    m_table.findRooms(m_pCurrentRoom->roomdata, vr);
    int size = vr->size();
    std::vector<Room*> like_current;
    for (int i = 0; i < size; ++i)
    {
        Room *candidate = vr->at(i);
        if (candidate == m_pCurrentRoom)
            continue;
        Room* next = candidate->dirs[m_lastDir].next_room;
        if (next && next->roomdata.equal(room))
        {
            Room* back = next->dirs[backDir].next_room;
            if (back && back->roomdata.equal(m_pCurrentRoom->roomdata))
                like_current.push_back(candidate);
        }
    }

    if (like_current.empty())
    {
        vr->clear();

        // check neighborhood room by coordinates
        RoomCursor cursor(m_pCurrentRoom);
        Room *next = cursor.move(m_lastDir);
        if (next && !next->dirs[backDir].next_room &&
            next->roomdata.equal(room))
        {
            m_pCurrentRoom->dirs[m_lastDir].next_room = next;
            next->dirs[backDir].next_room = m_pCurrentRoom;
            vr->push_back(next);
            return;
        }

        // create new room
        Room *new_room = addNewRoom(room);
        if (new_room) {
            vr->push_back(new_room);
            m_pCurrentRoom->dirs[m_lastDir].next_room = new_room;
        }
        return;
    }
    vr->swap(like_current);
}

Room* Mapper::addNewRoom(const RoomData& room)
{
    /*if (!m_pCurrentRoom || m_lastDir == -1)
    {
        if (!m_lastpos.current_room || m_lastDir == -1)  // last known room
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
            if (!m_lastpos.setOffsetRoom(new_room))
            {
                m_table.deleteRoom(new_room);
                delete new_room;
                return NULL;
            }
            return new_room;
        }
    }

    /*
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
    return new_room;*/
    return NULL;
}

Zone* Mapper::addNewZone()
{
    /*ZoneParams zp;
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
    return new_zone;*/
    return NULL;
}
//-------------------------------------------------------------------------------------
void Mapper::newZone(Room *room, RoomDir dir)
{
  /*  RoomHelper rh(room);
    if (rh.isCycled())
    {
        MessageBox(L"Невозможно создать новую зону из-за цикла!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    std::vector<Room*> rooms;
    rh.getSubZone(dir, &rooms);
    if (rooms.empty())
    {
        MessageBox(L"Нет комнат для создания новой зоны!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    int min_x = -1, max_x = 0, min_y = -1, max_y = 0, min_z = -1, max_z = 0;
    for (int i = 0, e = rooms.size(); i < e; ++i)
    {
        Room *r = rooms[i];
        if (min_x == -1 || min_x > r->x) min_x = r->x;
        if (max_x < r->x) max_x = r->x;
        if (min_y == -1 || min_y > r->y) min_y = r->y;
        if (max_y < r->y) max_y = r->y;
        int z = r->level->getLevel();
        if (min_z == -1 || min_z > z) min_z = z;
        if (max_z < z) max_z = z;
    }
    
    Zone *new_zone = addNewZone();
    int levels = max_z - min_z + 1;
    for (int i = 0; i < levels; ++i)
       new_zone->getLevel(i, true);       
    new_zone->resizeLevels(max_x - min_x, max_y - min_y);
    
    for (int i = 0, e = rooms.size(); i < e; ++i)
    {
        Room *r = rooms[i];
        int x = r->x - min_x;
        int y = r->y - min_y;
        int z = r->level->getLevel() - min_z;            
        r->level->detachRoom(r->x, r->y);
        RoomsLevel *level = new_zone->getLevel(z, false);            
        level->addRoom(r, x, y);
    }

    m_zones_control.addNewZone(new_zone);
    m_view.Invalidate();*/
}

Room* Mapper::createRoom(const RoomData& room)
{
    Room *new_room = new Room();
    new_room->roomdata = room;

    // parse new_room->roomdata.exits to room->dirs
    const tstring& e = new_room->roomdata.exits;
    if (e.find(m_propsData->north_exit) != -1)
        new_room->dirs[RD_NORTH].exist = true;
    if (e.find(m_propsData->south_exit) != -1)
        new_room->dirs[RD_SOUTH].exist = true;
    if (e.find(m_propsData->west_exit) != -1)
        new_room->dirs[RD_WEST].exist = true;
    if (e.find(m_propsData->east_exit) != -1)
        new_room->dirs[RD_EAST].exist = true;
    if (e.find(m_propsData->up_exit) != -1)
        new_room->dirs[RD_UP].exist = true;
    if (e.find(m_propsData->down_exit) != -1)
        new_room->dirs[RD_DOWN].exist = true;

    m_table.addRoom(new_room);
    return new_room;
}

void Mapper::deleteRoom(Room* room)
{
    m_table.deleteRoom(room);
    for (int i = 0, e = ROOM_DIRS_COUNT; i < e; ++i)
    {
        Room *next = room->dirs[i].next_room;
        if (next)
            next->dirs[revertDir(i)].next_room = NULL;
    }
    room->level->set(room->x, room->y, NULL);
    delete room;
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
    assert(false);
    return -1;
}
