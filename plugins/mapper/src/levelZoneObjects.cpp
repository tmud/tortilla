#include "stdafx.h"
#include "levelZoneObjects.h"

Zone* RoomsLevel::getZone() const 
{
    return m_parentZone;
}

const LevelZoneSize& RoomsLevel::size() const 
{
    return m_parentZone->size(); 
}

/*bool RoomsLevel::isEmpty() const
{
    const RoomsLevelParams& p = params();    
    for (int y = 0; y < p.height; ++y) {
    for (int x = 0; x < p.width; ++x) {
        if (rooms[y]->rr[x]) return false;
    }}
    return true;
}*/

Room* RoomsLevel::getRoom(int x, int y)
{
    if (!convertCoords(x, y))
        return NULL;
    row *row_y = rooms[y];
    return row_y->rr[x];
}

bool RoomsLevel::addRoom(Room* r, int x, int y)
{
    if (!convertCoords(x, y)) {
        assert(false);
        return false;
    }  
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
    m_invalidBox = true;
    m_changed = true;
    return true;
}

void RoomsLevel::deleteRoom(int x, int y)
{
    delete detachRoom(x, y);
}

Room* RoomsLevel::detachRoom(int x, int y)
{
    if (!convertCoords(x, y)) {
        assert(false);
        return NULL;
    }
    m_changed = true;
    row *row_y = rooms[y];
    Room *room = row_y->rr[x];
    if (room)
    {
        room->level = NULL;
        row_y->rr[x] = NULL;
        m_invalidBox = true;
    }
    return room;
}

bool RoomsLevel::convertCoords(int& x, int& y)
{
    const LevelZoneSize&p = size();
    if (x >= p.left && x <= p.right && y >= p.top && y <= p.bottom) 
    {
        x = x - p.left;
        y = y - p.top;
        return true;
    }
    return false;
}

void RoomsLevel::resizeLevel(int width, int height)
{



    /*const RoomsLevelParams&p = params();
    int mx = 0; int my = 0; // delta for coords
    if (x < 0)
    {
        mx = -x;
        for (int i = 0, e = rooms.size(); i < e; ++i)
        {
            std::vector<Room*> &v = rooms[i]->rr;
            for (int c = -x; c; --c)
                v.insert(v.begin(), NULL);
        }
    }
    else if (x >= p.width)
    {
        for (int i = 0, e = rooms.size(); i < e; ++i)
        {
            std::vector<Room*> &v = rooms[i]->rr;
            v.resize(x + 1, NULL);
        }
    }
    if (y < 0)
    {
        my = -y;
        for (int c = -y; c; --c)
        {
            row *r = new row;
            r->rr.resize(p.width, NULL);
            rooms.insert(rooms.begin(), r);
        }
    }
    else if (y >= p.height)
    {
        int c = y - p.height + 1;
        for (; c; --c)
        {
            row *r = new row;
            r->rr.resize(p.width, NULL);
            rooms.push_back(r);
        }
    }
    if (mx == 0 && my == 0)
        return;

    for (int i = 0, e = rooms.size(); i < e; ++i)
        rooms[i]->recalc_leftright();

    for (int i = 0, e = p.width; i < e; ++i) {
        for (int j = 0, je = p.height; j < je; ++j) {
            Room *room = getRoom(i, j);
            if (room) { room->x += mx; room->y += my; }
        }
    }*/
}

RoomsLevel* Zone::getLevel(int level, bool create_if_notexist)
{
    if (level >= m_size.minlevel && level <= m_size.maxlevel)
    {
        int index = level - m_size.minlevel;
        RoomsLevel *level = m_levels[index];
        if (level)
            return level;
    }
    if (!create_if_notexist)
        return NULL;

    RoomsLevel *new_level = new RoomsLevel(this);
    if (m_levels.empty())
    {
        m_size.minlevel = level;
        m_size.maxlevel = level;
        m_levels.push_back(new_level);
        return new_level;
    }

    new_level->resizeLevel(m_size.width(), m_size.height());
    if (level < m_size.minlevel)
    {
        int count = m_size.minlevel - level;
        m_levels.insert(m_levels.begin(), count, NULL);
        m_size.minlevel = level;
    }
    else if (level > m_size.maxlevel)
    {
        int count = level - m_size.maxlevel;
        m_levels.insert(m_levels.end(), count, NULL);
        m_size.maxlevel = level;
    }
    int index = level - m_size.minlevel;
    assert(m_levels[index] == NULL);
    m_levels[index] = new_level;
    return new_level;
}

/*RoomsLevel* Zone::getDefaultLevel()
{
    // find maximized level of the zone
    int area = 0; int index = -1;
    for (int i = 0, e = m_levels.size(); i < e; ++i)
    {
        int w = m_size.width();
        int h = m_size.height();
        int new_area = w * h;
        if (new_area > area) { area = new_area; index = i; }
    }
    if (index == -1) index = 0;
    return m_levels[index];
}*/

bool Zone::findLevel(const RoomsLevel *level, int *index) const
{
    for (int i=0,e=m_levels.size();i<e;++i) {
        if (m_levels[i] == level)
        {
            *index = ( m_size.minlevel + i);
            return true;
        }
    }
    return false;
}

void Zone::resizeLevels(int x, int y)
{

    for (int i = 0, e = m_levels.size(); i < e; ++i)
        m_levels[i]->resizeLevel(x, y);
}

bool Zone::isChanged() const
{
    //if (m_name != m_original_name) return true;
    for (int i = 0, e = m_levels.size(); i < e; ++i)
    {
        if (m_levels[i]->isChanged())
            return true;
    }
    return false;
}
