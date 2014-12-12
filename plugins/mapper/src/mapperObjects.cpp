#include "stdafx.h"
#include "mapperObjects.h"

const utf8* RoomDirName[] = { "north", "south", "west", "east", "up", "down" };
RoomCursor::RoomCursor() { reset(); }
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
}

int RoomsLevel::height() const
{
    return rooms.size();
}

int RoomsLevel::width() const
{
    if (rooms.empty())
        return 0;
    return rooms[0]->rr.size();
}

const RoomsLevelBox& RoomsLevel::box()
{
    if (m_invalidBoundingBox)
    {
        m_invalidBoundingBox = false;
        calcBoundingBox();
    }
    return m_box;
}

bool RoomsLevel::isEmpty() const
{
    int w = width();
    int h = height();
    for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
        if (rooms[y]->rr[x])
            return false;
    }}
    return true;
}

Room* RoomsLevel::getRoom(int x, int y)
{
    if (!checkCoords(x, y))
        return NULL;
    row *row_y = rooms[y];
    return row_y->rr[x];
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

bool RoomsLevel::checkCoords(int x, int y) const
{
    return (x >= 0 && x < width() && y >= 0 && y < height()) ? true : false;            
}

RoomsLevel* Zone::getLevel(int level, bool create_if_notexist)
{
    int last = m_levels.size() + start_index - 1;
    if (level >= start_index && level <= last)
    {
        int index = level - start_index;
        RoomsLevel *level = m_levels[index];
        if (level)
            return level;
    }
    if (!create_if_notexist)
        return NULL;   

    RoomsLevel *new_level = new RoomsLevel(this, level);    
    if (m_levels.empty())
    {
        start_index = level;
        m_levels.push_back(new_level);
        return new_level;
    }
    RoomsLevel *rl = m_levels[0];
    new_level->resizeLevel(rl->width()-1, rl->height()-1);

    if (level < start_index)
    {
        int count = start_index - level;
        m_levels.insert(m_levels.begin(), count, NULL);
        start_index = level;        
    }
    else if (level > last)
    {
        int count = level - last;
        m_levels.insert(m_levels.end(), count, NULL);
    }

    int index = level - start_index;
    m_levels[index] = new_level;
    return new_level;
}

RoomsLevel* Zone::getDefaultLevel()
{   
    // find maximized level of the zone
    int area = 0; int index = -1;
    for (int i = 0, e = m_levels.size(); i < e; ++i)
    {
        RoomsLevel *level = m_levels[i];
        const RoomsLevelBox& box = level->box();
        int w = box.right - box.left + 1;
        int h = box.bottom - box.top + 1;
        int new_area = w * h;
        if (new_area > area) { area = new_area; index = i; }
    }
    if (index == -1) index = 0;
    return m_levels[index];
}

void Zone::getParams(ZoneParams* params) const
{
    params->name = m_name;
    params->original_name = m_original_name;
    if (m_levels.empty())
        { params->empty = true; return; }
    int lc = m_levels.size();
    params->minl = start_index;
    params->maxl = start_index + lc - 1;
    params->empty = false;    
}

void Zone::resizeLevels(int x, int y)
{
    for (int i=0,e=m_levels.size(); i<e; ++i)       
        m_levels[i]->resizeLevel(x, y);
}

bool Zone::isChanged() const
{
    if (m_name != m_original_name) return true;
    for (int i = 0, e = m_levels.size(); i < e; ++i)
    {
        if (m_levels[i]->isChanged())
            return true;
    }
    return false;
}

int Zone::width() const
{
    int width = 0;
    if (m_levels.size() > 0)
        width = m_levels[0]->width();
    return width;
}

int Zone::height() const
{
    int height = 0;
    if (m_levels.size() > 0)
        height = m_levels[0]->height();
    return height;
}
