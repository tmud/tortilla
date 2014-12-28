#include "stdafx.h"
#include "mapperObjects.h"
const utf8* RoomDirName[] = { "north", "south", "west", "east", "up", "down" };

RoomCursor::RoomCursor(Room *r) : room(r), x(-1), y(-1), level(0) { assert(room); if (room) updateCoords(); }
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
}

/*void RoomsLevel::extend(ExtendDir d, int count)
{
    assert(count > 0);
    if (rooms.empty())
    {
        for (int i = 0; i < count; ++i) {
        row *r = new row;
        rooms.push_front(r);
        }
    }
    switch (d)
    {
    case EXTEND_LEFT:
        for (int y=0, e=rooms.size(); y<e; ++y)
        {
            std::deque<Room*> &d = rooms[y]->rr;
            for (int i = 0; i < count; ++i)
                d.push_front(NULL);
        }
        for (int x = 0, e = m_width; x < e; ++x) {
        for (int y = 0, ye = m_height; y < ye; ++y) {
        Room *room = rooms[y]->rr[x];
        if (room) { room->x += count; }
        }}
        m_width++;
    break;
    case EXTEND_RIGHT:
        for (int y = 0, e = rooms.size(); y < e; ++y)
        {
            std::deque<Room*> &d = rooms[y]->rr;
            for (int i = 0; i < count; ++i)
                d.push_back(NULL);
        }
        m_width++;
    break;
    case EXTEND_TOP:
        for (int i = 0; i < count; ++i)
        {
            row *r = new row;
            r->rr.resize(m_width, NULL);
            rooms.push_front(r);
        }
        for (int x = 0, e = m_width; x < e; ++x) {
        for (int y = count, ye = m_height; y < ye; ++y) {
           Room *room = rooms[y]->rr[x];
           if (room) { room->y += count; }
        }}
        m_height++;
    break;
    case EXTEND_BOTTOM:
        for (int i = 0; i < count; ++i)
        {
            row *r = new row;
            r->rr.resize(m_width, NULL);
            rooms.push_back(r);
        }
        m_height++;
    break;
    }
}*/

bool RoomsLevel::set(int x, int y, Room* room)
{
    if (!check(x, y))
    {
        assert(false);
        return false;
    }
    if (room)
    {
        if (!rooms[y]->rr[x])
        {
            assert(false);
            return false;
        }
        room->x = x;
        room->y = y;
        room->level = this;
    }    
    rooms[y]->rr[x] = room;
    return true;
}

Room* RoomsLevel::get(int x, int y) const
{
    if (!check(x, y))
        return NULL;
    return rooms[y]->rr[x];
}

int RoomsLevel::getWidth() const { return m_pZone->getWidth(); }
int RoomsLevel::getHeight() const { return m_pZone->getHeight(); }
bool RoomsLevel::check(int x, int y) const {
    return (x >= 0 && x < getWidth() && y >= 0 && y < getHeight()) ? true : false;
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

RoomsLevel* Zone::getDefaultLevel()
{
    return m_levels[0];
}

/*void Zone::resizeLevels(int x, int y)
{
    for (int i=0,e=m_levels.size(); i<e; ++i)       
        m_levels[i]->resizeLevel(x, y);
}*/

bool Zone::addRoom(int x, int y, int level, Room* room)
{
    RoomsLevel *rlevel = getl(level, true);


    bool result = rlevel->set(x, y, room);
    assert(result);
    return result;
}

Room* Zone::getRoom(int x, int y, int level)
{
    RoomsLevel *rlevel = getl(level, false);
    return (rlevel) ? rlevel->get(x, y) : NULL;
}

RoomsLevel* Zone::getLevel(int level)
{
    return getl(level, false);
}

RoomsLevel* Zone::getLevelAlways(int level)
{
    return getl(level, true);
}

RoomsLevel* Zone::getl(int level, bool create_if_notexist)
{
    lvls_iterator it = m_levels.find(level);    
    if (it == m_levels.end())        
    {
        if (!create_if_notexist)
            return NULL;
        RoomsLevel *new_level = new RoomsLevel(level, this);
        m_levels[level] = new_level;
        return new_level;
    }
    return it->second;
}




/*RoomCursor::RoomCursor() { reset(); }
void RoomCursor::reset() { current_room = NULL; new_room = NULL; x=y=z=0; }

RoomsLevel* RoomCursor::getOffsetLevel() const
{
if (!current_room) return NULL;
int pz = current_room->level->getLevel() + z;
Zone *zone = current_room->level->getZone();
return zone->getLevel(pz, true);
}

Room* RoomCursor::getOffsetRoom() const
{
RoomsLevel *rlevel = getOffsetLevel();
if (!rlevel)
return NULL;
int rx = current_room->x + x;
int ry = current_room->y + y;
return rlevel->getRoom(rx, ry);
}

bool RoomCursor::setOffsetRoom(Room* room) const
{
RoomsLevel *rlevel = getOffsetLevel();
if (!rlevel)
return false;
int rx = current_room->x + x;
int ry = current_room->y + y;
if (rlevel->getRoom(rx, ry))
return false;
rlevel->addRoom(room, rx, ry);
return true;
}

void RoomCursor::move(int dir)
{
if (dir == RD_NORTH)
y -= 1;
else if (dir == RD_SOUTH)
y += 1;
else if (dir == RD_WEST)
x -= 1;
else if (dir == RD_EAST)
x += 1;
else if (dir == RD_UP)
z += 1;
else if (dir == RD_DOWN)
z -= 1;
else { assert(false); }
}*/
