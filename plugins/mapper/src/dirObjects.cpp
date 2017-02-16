#include "stdafx.h"
#include "dirObjects.h"

class InitDirVector {
    Pcre r1, r2;
public:
    InitDirVector() { r1.init(L","); r2.init(L"\\|"); }
    bool make(RoomDir dir, const tstring& key, std::vector<MapperDirCommand>& dc)
    {
        bool result = true;
        r1.findall(key.c_str());
        int b = 0;
        for (int i = 1, ie = r1.size(); i < ie; ++i) {
            int e = r1.first(i);
            if (!set(dir, key.substr(b, e - b), dc))
                result = false;
            b = e + 1;
        }
        if (!set(dir, key.substr(b), dc))
            result = false;
        return result;
    }
private:
    bool set(RoomDir dir, const tstring& dkey, std::vector<MapperDirCommand>& m) {
        if (dkey.empty()) return true;
        if (!r2.find(dkey.c_str())) {
            MapperDirCommand k(dir, dkey, L"");
            m.push_back(k);
            return true;
        }
        if (r2.size() != 1)
            return false;
        int p = r2.first(0);
        tstring main(dkey.substr(0, p));
        tstring rel(dkey.substr(p + 1));
        MapperDirCommand k(dir, main, rel);
        m.push_back(k);
        return true;
    }
};

MapperDirsVector::MapperDirsVector()
{
}

void MapperDirsVector::find(const tstring& cmd)
{
    RoomDir dir = RD_UNKNOWN;
    for (int i = 0, e = dirs.size(); i < e; ++i)
    {
        dir = dirs[i].check(cmd);
        if (dir != RD_UNKNOWN) { m_path.push_back(dir); break; }
    }
}

void MapperDirsVector::init(PropertiesMapper* props)
{
    dirs.clear();
    InitDirVector h;
    h.make(RD_NORTH, props->north_cmd, dirs);
    h.make(RD_SOUTH, props->south_cmd, dirs);
    h.make(RD_WEST, props->west_cmd, dirs);
    h.make(RD_EAST, props->east_cmd, dirs);
    h.make(RD_UP, props->up_cmd, dirs);
    h.make(RD_DOWN, props->down_cmd, dirs);
}


/*#include "mapperObjects.h"
#include "mapperObjects2.h"


RoomCursor::RoomCursor(Room* current_room) : 
m_current_room(current_room), x(0), y(0), level(0)
{
    assert(m_current_room && m_current_room->level && m_current_room->level->getZone());
}

Room* RoomCursor::getRoom(RoomDir dir)
{
    if (!move(dir))
        return NULL;
    if (!m_current_room || !m_current_room->level) {
        assert(false);
        return NULL; 
    }
    Zone *zone = m_current_room->level->getZone();
    if (!zone) { 
        assert(false);
        return NULL;
    }
    RoomsLevel *rl = zone->getLevel(level, false);
    if (!rl)
        return NULL;
    return rl->getRoom(x, y);
}

bool RoomCursor::addRoom(RoomDir dir, Room* room)
{
    if (!move(dir))
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
    return result;
}

bool RoomCursor::addLink(RoomDir dir, Room *room)
{
    Room *next = m_current_room->dirs[dir].next_room;
    if (!next)
    {
        m_current_room->dirs[dir].next_room = room;
        return true;
    }
    return (next == room);
}

bool RoomCursor::move(RoomDir dir)
{
    x = m_current_room->x;
    y = m_current_room->y;
    level = m_current_room->level->getLevel();
    switch (dir)
    {
    case RD_NORTH: y -= 1; break;
    case RD_SOUTH: y += 1; break;
    case RD_WEST:  x -= 1; break;
    case RD_EAST:  x += 1; break;
    case RD_UP: level += 1; break;
    case RD_DOWN: level -= 1; break;
    default:
        assert(false);
        return false;
    }
    return true;
}

bool RoomCursor::isExplored(RoomDir dir)
{
    Room *r = m_current_room->dirs[dir].next_room;
    if (r && m_current_room->level->getZone() == r->level->getZone())      
       return true;
    return false;
}

Zone* RoomCursorNewZone::createNewZone(const tstring& name, Room* room)
{
    Zone *new_zone = new Zone(name);
    RoomsLevel *level = new_zone->getLevel(0, true);
    level->addRoom(room, 0, 0);
    return new_zone;
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
    return rel.find(rel_part) == 0 ? dir : RD_UNKNOWN;
}
*/