#include "stdafx.h"
#include "mapperObjects.h"
#include "mapperProcessor.h"

const utf8* RoomDirName[] = { "north", "south", "west", "east", "up", "down" };
extern MapperProcessor* m_mapper_processor;

/*RoomCursor::RoomCursor(Room *r) : room(r), x(-1), y(-1), level(0) { assert(room); if (room) updateCoords(); }
Room* RoomCursor::move(int dir)
{
    if (!room)
        return NULL;
    int x = room->x;
    int y = room->y;    
    switch (dir)
    {
        case RD_NORTH: 
            room = room->level->get(x, y - 1);
        break;
        case RD_SOUTH:
            room = room->level->get(x, y + 1);
        break;
        case RD_WEST:
            room = room->level->get(x - 1, y);
        break;
        case RD_EAST: 
            room = room->level->get(x + 1, y);
        break;
        case RD_UP:
        {
            RoomsLevel* next = getOffsetLevel(1);
            room = (next) ? next->get(x, y) : NULL;
        }
        break;
        case RD_DOWN:
        {
            RoomsLevel* next = getOffsetLevel(-1);
            room = (next) ? next->get(x, y) : NULL;
        }
        break;
        default:
        {
            assert(false);
            room = NULL;
        }            
    }
    updateCoords();
    return room;
}

RoomsLevel* RoomCursor::getOffsetLevel(int offset)
{
    int z = room->level->getLevel() + offset;
    Zone *zone = room->level->getZone();
    return zone->getLevel(z);
}

void RoomCursor::updateCoords()
{
    if (!room)
    {
        x = y = -1;
        level = 0;
        return;
    }
    x = room->x;
    y = room->y;
    level = room->level->getLevel();
}*/

//RoomsLevel::RoomsLevel(int width, int height, int level, Zone *parent_zone) : m_pZone(parent_zone), m_level(level)

/*RoomsLevel::RoomsLevel(RoomsArea *parent_area, int level, int w, int h) : m_pArea(parent_area), m_level(level)
{
    setsize(w, h);
}

RoomsLevel::~RoomsLevel()
{
    struct{ void operator() (row* r) { delete r; } } del;
    std::for_each(rooms.begin(), rooms.end(), del);
}

bool RoomsLevel::set(int x, int y, Room* room)
{
    if (!check(x, y))
    {
        assert(false);
        return false;
    }
    if (room)
    {
        if (rooms[y]->rr[x])
            return false;       // место занято
        room->x = x;
        room->y = y;
        room->level = this;
    }    
    rooms[y]->rr[x] = room;
    m_pArea->setChanged();
    return true;
}

Room* RoomsLevel::get(int x, int y) const
{
    return (check(x, y)) ? rooms[y]->rr[x] : NULL;
}

void RoomsLevel::extend(RoomDir d, int count)
{
    if (d == RD_UP || d == RD_DOWN || count <= 0)
    {
        assert(false);
        return;
    }
    if (rooms.empty())
    {
        switch (d)
        {
        case RD_WEST:
        case RD_EAST:
            setsize(count, 1);
            break;
        case RD_NORTH:
        case RD_SOUTH:
            setsize(1, count);
            break;
        default:
            assert(false);
        }
        return;
    }
    switch (d)
    {
    case RD_WEST:
        for (int x = 0, e = getWidth(); x < e; ++x) {
            for (int y = 0, ye = getHeight(); y < ye; ++y) {
                Room *room = rooms[y]->rr[x];
                if (room) { room->x += count; }
            }
        }
        for (int y = 0, e = rooms.size(); y < e; ++y)
        {
            std::deque<Room*> &rr = rooms[y]->rr;
            rr.insert(rr.begin(), count, NULL);
        }
        break;
    case RD_EAST:
        for (int y = 0, e = rooms.size(); y < e; ++y)
        {
            std::deque<Room*> &rr = rooms[y]->rr;
            rr.insert(rr.end(), count, NULL);
        }
        break;
    case RD_NORTH:
        for (int x = 0, e = getWidth(); x < e; ++x) {
            for (int y = 0, ye = getHeight(); y < ye; ++y) {
                Room *room = rooms[y]->rr[x];
                if (room) { room->y += count; }
            }
        }
        for (int i = 0; i < count; ++i)
        {
            row *r = new row;
            r->rr.resize(getWidth(), NULL);
            rooms.push_front(r);
        }
        break;
    case RD_SOUTH:
        for (int i = 0; i < count; ++i)
        {
            row *r = new row;
            r->rr.resize(getWidth(), NULL);
            rooms.push_back(r);
        }
        break;
    }
}

void RoomsLevel::setsize(int dx, int dy)
{
    if (dx <= 0 || dy <= 0) return;
    for (int i = 0; i < dy; ++i)
    {
        row *r = new row;
        r->rr.resize(dx, NULL);
        rooms.push_front(r);
    }
}


/*const RoomsLevelBox& RoomsLevel::box()
{
    if (m_invalidBoundingBox)
    {
        m_invalidBoundingBox = false;
        calcBoundingBox();
    }
    return m_box;
}*/

/*
Room* RoomsLevel::getRoom(int x, int y) const
{
    return rooms.get(x, y);
}

bool RoomsLevel::addRoom(Room* r, int x, int y)
{
    m_changed = true;
    
    
    zone->resizeLevels(x, y);   // resize levels first
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    row *row_y = rooms[y];
    if (row_y->rr[x])
    {
        assert(false);
        return false;           // room exist
    }
    r->x = x;
    r->y = y;
    r->level = this;
    row_y->rr[x] = r;
    row_y->recalc_leftright();    
    m_invalidBoundingBox = true;
    return true;
}

void RoomsLevel::deleteRoom(int x, int y)
{
    Room *room = detachRoom(x, y);
    delete room;
}

Room* RoomsLevel::detachRoom(int x, int y)
{
    if (!checkCoords(x, y))
        { assert(false); return NULL; }
    m_changed = true;
    row *row_y = rooms[y];
    Room *room = row_y->rr[x];
    if (room)
    {
        room->level = NULL;
        row_y->rr[x] = NULL;
        row_y->recalc_leftright();
        m_invalidBoundingBox = true;
    }
    return room;
}

void RoomsLevel::calcBoundingBox()
{
    RoomsLevelBox &b = m_box;
    b.left = b.right = b.top = b.bottom = -1;
    for (int i = 0, e = rooms.size(); i < e; ++i)
    {
        int left = rooms[i]->left;        
        if (left == -1) continue;
        if (b.left == -1 || left < b.left) b.left = left;
        int right = rooms[i]->right;
        if (right > b.right) b.right = right;
        if (b.top == -1 && b.left != -1) b.top = i;
        if (b.left != -1) b.bottom = i;
    }
}

void RoomsLevel::resizeLevel(int x, int y)
{
    int mx = 0; int my = 0; // delta for coords
    if (x < 0)
    {
        mx = -x;
        for (int i=0,e=rooms.size(); i<e; ++i)
        {
            std::vector<Room*> &v = rooms[i]->rr;
            for (int c=-x; c; --c)
               v.insert(v.begin(), NULL);
        }
    }
    else if (x >= width())
    {
        for (int i=0,e=rooms.size(); i<e; ++i)
        {
            std::vector<Room*> &v = rooms[i]->rr;
            v.resize(x+1, NULL);
        }
    }
    if (y < 0)
    {
        my = -y;
        for (int c=-y; c; --c)
        {
            row *r = new row;
            r->rr.resize(width(), NULL);
            rooms.insert(rooms.begin(), r);
        }
    }
    else if (y >= height())
    {
        int c=y-height()+1;
        for (; c; --c)
        {
            row *r = new row;
            r->rr.resize(width(), NULL);
            rooms.push_back(r);
        }
    }
    if (mx == 0 && my == 0)
        return;
    
    for (int i = 0, e = rooms.size(); i < e; ++i)
        rooms[i]->recalc_leftright();
    
    for (int i=0,e=width(); i<e; ++i) {
    for (int j=0,je=height(); j<je; ++j) {
    Room *room = getRoom(i, j);
    if (room) { room->x += mx; room->y += my; }
    }}
}
*/

/*
void RoomsArea::extend(RoomDir d, int count)
{
    switch (d)
    {
    case RD_NORTH:
    case RD_SOUTH:
    case RD_WEST:
    case RD_EAST:
    {
        area_iterator it = m_area.begin(), it_end = m_area.end();
        for (; it != it_end; ++it)
           it->second->extend(d, count);
        m_changed = true;
    }
    break;
    case RD_UP:
    {
        int w = getWidth(); int h = getHeight();
        int level = m_area.rbegin()->first + 1;
        for (; count > 0; --count)
        {
            RoomsLevel *new_level = new RoomsLevel(this, level, w, h);
            m_area[level] = new_level;
            level++;
        }
        m_changed = true;
    }
    break;
    case RD_DOWN:
    {
        int w = getWidth(); int h = getHeight();
        int level = m_area.begin()->first - 1;
        for (; count > 0; --count)
        {
            RoomsLevel *new_level = new RoomsLevel(this, level, w, h);
            m_area[level] = new_level;
            level--;
        }
        m_changed = true;
    }
    break;
    default:
        assert(false);
        break;
    } // switch
}




*/

/*void Zone::resizeLevels(int x, int y)
{
    for (int i=0,e=m_levels.size(); i<e; ++i)       
        m_levels[i]->resizeLevel(x, y);
}*/

/*bool Zone::addRoom(int x, int y, int level, Room* room)
{
    RoomsLevel *rlevel = getl(level, true);


    bool result = rlevel->set(x, y, room);
    assert(result);
    return result;
}*/

/*Room* Zone::getRoom(int x, int y, int level)
{
    RoomsLevel *rlevel = getl(level, false);
    return (rlevel) ? rlevel->get(x, y) : NULL;
}*/


/*RoomsLevel* Zone::getLevelAlways(int level)
{
    return getl(level, true);
}*/

/*RoomsLevel* Zone::getl(int level, bool create_if_notexist)
{
    lvls_iterator it = m_levels.find(level);
    if (it == m_levels.end())
    {
        if (!create_if_notexist)
            return NULL;
        int w = (m_width > 0) ? m_width : 1;
        int h = (m_height > 0) ? m_height : 1;
        RoomsLevel *new_level = new RoomsLevel(w, h, level, this);
        m_levels[level] = new_level;
        return new_level;
    }
    return it->second;
}*/

bool Table::getLimits(const TablePos& p, TableLimits* limits)
{
    int zones = m_zones.size();
    if (p.zone >= 0 && p.zone < zones)
    {
        limits->areas = m_zones[p.zone]->areas.size();
        if (p.area >= 0 && p.area < limits->areas)
        {
            area *parea = m_zones[p.zone]->areas[p.area];
            limits->minlevel = parea->levels.begin()->first;
            limits->maxlevel = parea->levels.rbegin()->first;
            if (p.level >= limits->minlevel && p.level <= limits->maxlevel)
            {
                level *plevel = parea->levels[p.level];
                limits->height = plevel->lines.size();
                if (limits->height > 0)
                {
                    limits->width = plevel->lines[0]->elements.size();
                    return true;
                }
            }
        }
    }
    return false;
}

Room* Table::get(const TablePos& p)
{
    TableLimits limits;
    if (!getLimits(p, &limits))
        return NULL;
    if (p.x >= 0 && p.x < limits.width && p.y >= 0 && p.y < limits.height)
    {
        area *parea = m_zones[p.zone]->areas[p.area];
        level *plevel = parea->levels[p.level];
        return plevel->lines[p.y]->elements[p.x]->room;
    }
    return NULL;
}

Room* Table::addNewRoom(Room *current_room, const RoomData& rd, RoomDir dir)
{
    std::vector<Room*> vr;
    m_hash_table.findRooms(rd, &vr);
    int count = vr.size();

    Room *target = NULL;
    for (int i = 0; i < count; ++i)
    {
        TableCell *cell = vr[i]->cell;
        const RoomData& rdata = cell->room->roomdata;
        if (rdata.roomid == rd.roomid && rdata.zonename == rd.zonename)
           {  target = vr[i]; break; }
    }

    if (!current_room || dir == RD_UNKNOWN)
    {
        if (target)
            return target;
        TablePos pos;
        createNewPlace(rd.zonename, &pos);
        Room* new_room = createRoom(rd);
        set(pos, new_room);
        return new_room;
    }

    if (target)
    {
        current_room->dirs[dir].setNext(target);
        return target;
    }

    if (current_room->roomdata.zonename != rd.zonename)
    {
        TablePos pos;
        createNewPlace(rd.zonename, &pos);
        Room* new_room = createRoom(rd);
        set(pos, new_room);
        current_room->dirs[dir].setNext(new_room);
        return new_room;
    }

    TablePos pos;
    pos.init(current_room);

    TableLimits limits;
    getLimits(pos, &limits);

    int x = 1;


        //todo

    /*/


    TablePos pos;
    pos.init(current_room);
    createPlace(pos, dir);
    Room* new_room = createRoom(rd);
    set(pos, new_room);
    current_room->dirs[dir].setNext(new_room);
    return new_room;*/
    return NULL;
}

void Table::createNewPlace(const tstring& zone_name, TablePos *pos)
{
    pos->zone = getZone(zone_name);
    zone *pzone = m_zones[pos->zone];
    area *parea = new area();
    pos->area = pzone->areas.size();
    pzone->areas.push_back(parea);
    level *plevel = new level();
    pos->level = 0;
    parea->levels[0] = plevel;
    line *pline = new line();
    plevel->lines.push_back(pline);
    pos->y = 0;
    TableCell* cell = new TableCell();
    pline->elements.push_back(cell);
    pos->x = 0;   
    int index = getIndex(*pos);
    cell->index = m_indexes[index];
}

void Table::createPlace(TablePos& p, RoomDir dir)
{
    
    


}

Room* Table::createRoom(const RoomData& room)
{
    Room *new_room = new Room();
    PropertiesMapper *p = m_mapper_processor->m_propsData;

    // parse new_room->roomdata.exits to room->dirs
    const tstring& e = room.exits;
    if (e.find(p->north_exit) != -1)
        new_room->dirs[RD_NORTH].exist = true;
    if (e.find(p->south_exit) != -1)
        new_room->dirs[RD_SOUTH].exist = true;
    if (e.find(p->west_exit) != -1)
        new_room->dirs[RD_WEST].exist = true;
    if (e.find(p->east_exit) != -1)
        new_room->dirs[RD_EAST].exist = true;
    if (e.find(p->up_exit) != -1)
        new_room->dirs[RD_UP].exist = true;
    if (e.find(p->down_exit) != -1)
        new_room->dirs[RD_DOWN].exist = true;

    new_room->roomdata = room;
    m_hash_table.addRoom(new_room);
    return new_room;
}

void Table::set(const TablePos& p, Room *r)
{
    area *parea = m_zones[p.zone]->areas[p.area];
    level *plevel = parea->levels[p.level];
    TableCell *cell = plevel->lines[p.y]->elements[p.x];
    cell->x = p.x;
    cell->y = p.y;
    cell->room = r;
    r->cell = cell;
}

int Table::getZone(const tstring& name)
{
    for (int i = 0, e = m_zones.size(); i < e; ++i)
        if (m_zones[i]->name == name) { return i; }
    zone *new_zone = new zone();
    new_zone->name = name;
    int index = m_zones.size();
    m_zones.push_back(new_zone);
    return index;
}

int Table::getIndex(const TablePos& p)
{
    for (int i = 0, e = m_indexes.size(); i < e; ++i)
    {
        TableIndex *index = m_indexes[i];
        if (index->zone == p.zone && index->area == p.area && index->level == p.level)
            return i;
    }
    TableIndex *index = new TableIndex();
    index->zone = p.zone;
    index->area = p.area;
    index->level = p.level;
    int ti = m_indexes.size();
    m_indexes.push_back(index);
    return ti;
}
