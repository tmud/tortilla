#include "stdafx.h"
#include "levelZoneObjects.h"



/*int RoomsLevel::height() const
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
}*/

/*bool RoomsLevel::isEmpty() const
{
    const RoomsLevelParams& p = params();    
    for (int y = 0; y < p.height; ++y) {
    for (int x = 0; x < p.width; ++x) {
        if (rooms[y]->rr[x]) return false;
    }}
    return true;
}*/


const RoomsLevelParams& RoomsLevel::params()
{
    RoomsLevelParams &p = m_params;
    if (rooms.empty()) {
        p.width = p.height = 0;
    }
    else {
        p.width = rooms[0]->rr.size();
        p.height = rooms.size();
    }
    if (m_invalidBox)
    {
        m_invalidBox = false;
        p.left = p.right = p.top = p.bottom = -1;
        for (int i = 0, e = rooms.size(); i < e; ++i)
        {
            int left = rooms[i]->left;
            if (left == -1) continue;
            if (p.left == -1 || left < p.left) p.left = left;
            int right = rooms[i]->right;
            if (right > p.right) p.right = right;
            if (p.top == -1 && p.left != -1) p.top = i;
            if (p.left != -1) p.bottom = i;
        }
    }
    return p;
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
    m_zone->resizeLevels(x, y); // resize levels first
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    row *row_y = rooms[y];
    if (row_y->rr[x])
    {
        assert(false);
        return false;                 // room exist
    }
    r->x = x;
    r->y = y;
    r->level = this;
    row_y->rr[x] = r;
    row_y->recalc_leftright();
    m_invalidBox = true;
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
    {
        assert(false); return NULL;
    }
    m_changed = true;
    row *row_y = rooms[y];
    Room *room = row_y->rr[x];
    if (room)
    {
        room->level = NULL;
        row_y->rr[x] = NULL;
        row_y->recalc_leftright();
        m_invalidBox = true;
    }
    return room;
}

void RoomsLevel::resizeLevel(int x, int y)
{
    const RoomsLevelParams&p = params();
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
    }
}

bool RoomsLevel::checkCoords(int x, int y)
{
    const RoomsLevelParams&p = params();
    return (x >= 0 && x < p.width && y >= 0 && y < p.height) ? true : false;
}

RoomsLevel* Zone::getLevel(int level, bool create_if_notexist)
{
    ZoneParams& zp = m_params;
    if (level >= zp.minlevel && level <= zp.maxlevel)
    {
        int index = level - zp.minlevel;
        RoomsLevel *level = m_levels[index];
        if (level)
            return level;
    }
    if (!create_if_notexist)
        return NULL;

    RoomsLevel *new_level = new RoomsLevel(this, level);
    if (m_levels.empty())
    {
        zp.minlevel = level;
        m_levels.push_back(new_level);
        return new_level;
    }
    RoomsLevel *rl = m_levels[0];
    const RoomsLevelParams& p = rl->params();
    new_level->resizeLevel(p.width - 1, p.height - 1);

    if (level < m_params.minlevel)
    {
        int count = m_params.minlevel - level;
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
        const RoomsLevelParams&p = level->params();
        int w = p.right - p.left + 1;
        int h = p.bottom - p.top + 1;
        int new_area = w * h;
        if (new_area > area) { area = new_area; index = i; }
    }
    if (index == -1) index = 0;
    return m_levels[index];
}

const ZoneParams& Zone::params()
{
    if (m_levels.empty())
        return m_params;

    int lc = m_levels.size();
    m_params.maxlevel = m_params.minlevel + lc - 1;
    if (m_levels.empty()) {
        m_params.
    }

    int width = 0;
    if (m_levels.size() > 0)
        width = m_levels[0]->width();
    return width;

    int height = 0;
    if (m_levels.size() > 0)
        height = m_levels[0]->height();
    return height;
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

void Zone::setName(const tstring& name)
{
    m_params.name = name;
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
