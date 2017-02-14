#include "stdafx.h"
#include "roomObjects.h"
#include "levelZoneObjects.h"

RoomHelper::RoomHelper(Room *room) : r(room), x(0), y(0), z(0)
{
    assert(room && room->level && zone());
}

Zone* RoomHelper::zone()
{
    return level()->getZone();
}

RoomsLevel* RoomHelper::level()
{
    return r->level;
}

Room* RoomHelper::getRoomDir(RoomDir dir)
{
    if (dir == RD_UNKNOWN) {
        assert(false);  return NULL;
    }
    return r->dirs[dir].next_room;
}

/*bool RoomHelper::isExplored(RoomDir dir)
{
    Room *next = getRoomDir(dir);
    return (next && zone(r) == zone(next)) ? true : false;
}*/

bool RoomHelper::addLink(RoomDir dir, Room *room)
{
    Room *next = r->dirs[dir].next_room;
    if (!next)
    {
        r->dirs[dir].next_room = room;
        return true;
    }
    return (next == room);
}

bool RoomHelper::addRoom(RoomDir dir, Room* room)
{
    /*if (!move(dir))
        return false;
    if (!m_current_room || !m_current_room->level) {
        assert(false);
        return false;
    }
    Zone *zone = m_current_room->level->getZone();
    RoomsLevel *rl = zone->getLevel(level, true);
    bool result = rl->addRoom(room, x, y);
    if (result)
    {
        result = addLink(dir, room);
        if (!result)
        {
            rl->detachRoom(x, y);
            assert(false);
        }
    }
    return result;*/
    return false;
}

bool RoomHelper::move(RoomDir dir)
{
    x = r->x;
    y = r->y;
    RoomsLevel *level = r->level;
    z = level->params().level;
    switch (dir)
    {
        case RD_NORTH: y -= 1; break;
        case RD_SOUTH: y += 1; break;
        case RD_WEST:  x -= 1; break;
        case RD_EAST:  x += 1; break;
        case RD_UP: z += 1; break;
        case RD_DOWN: z -= 1; break;
        default:
            assert(false);
            return false;
    }
    return true;
}

Zone* RoomHelper::zone(Room *room)
{
    assert(room);
    if (room) {
      assert(room->level);
      return room->level->getZone();   
    }
    return NULL;
}

const wchar_t* RoomDirName[] = { L"north", L"south", L"west", L"east", L"up", L"down" };
RoomDir RoomDirHelper::revertDir(RoomDir dir)
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
    return RD_UNKNOWN;
}
const wchar_t* RoomDirHelper::getDirName(RoomDir dir)
{
    int index = static_cast<int>(dir);
    if (index >= 0 && index <= 5)
        return RoomDirName[index];
    return NULL;
}
RoomDir RoomDirHelper::getDirByName(const wchar_t* dirname)
{
    tstring name(dirname);
    for (int index = 0; index <= 5; ++index)
    {
        if (!name.compare(RoomDirName[index]))
            return static_cast<RoomDir>(index);
    }
    return RD_UNKNOWN;
}