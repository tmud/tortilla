#include "stdafx.h"
#include "roomObjects.h"

Rooms3dCube::AR_STATUS Rooms3dCube::addRoom(const Rooms3dCubePos& p, Room* r)
{
    if (r->pos.zid != -1)
        return AR_INVALIDROOM;
    if (!extends(p))
        return AR_FAIL;
    Room** ptr = getp(p);
    if (*ptr)
        return AR_BUSY;
    r->pos = p;
    r->pos.zid = z_id;
    *ptr = r;
    m_hashmap[r->hash()] = r;
    return AR_OK;
}

const Room* Rooms3dCube::getRoom(const Rooms3dCubePos& p) const
{
    return get(p);
}

void Rooms3dCube::deleteRoom(const Rooms3dCubePos& p)
{
    Room *r = detachRoom(p);
    if (r) { clearExits(r); delete r; }
}

Room* Rooms3dCube::detachRoom(const Rooms3dCubePos& p)
{
    Room *r = NULL;
    Room **ptr = getp(p);
    if (ptr && *ptr)
    {
        r = *ptr;
        *ptr = NULL;
        r->pos.clear();
        collapse();
        hashmap_iterator it = m_hashmap.find(r->hash());
        if (it != m_hashmap.end()) {
            m_hashmap.erase(it);
        } else {       
            assert(false);
        }
    }
    return r;
}

Room* Rooms3dCube::findRoom(const tstring& hash) const
{    
    hashmap_const_iterator it = m_hashmap.find(hash);
    return (it == m_hashmap.end()) ? NULL : it->second;
}

void Rooms3dCube::optimizeSize()
{
    collapse();
}

bool Rooms3dCube::testInvariant()
{
    const Rooms3dCubeSize&  s = size();
    int levels_count = zone.size();
    if (levels_count != s.levels())
        return false;
    for (level *l : zone) {
        std::vector<row*>&r = l->rooms;
        int rows = r.size();
        if (rows != s.height())
            return false;        
        for (row *rv : r) {
            int columns = rv->rr.size();
            if (columns != s.width())
                return false;
        }
    }
    return true;
}

Room* Rooms3dCube::get(const Rooms3dCubePos& p) const
{
    Room** ptr = getp(p);
    if (!ptr) return NULL;
    return *ptr;
}

Room** Rooms3dCube::getp(const Rooms3dCubePos& p) const
{
    if (!checkCoords(p))
        return NULL;
    int z = p.z - cube_size.minlevel;
    assert(z>=0 && z<(int)zone.size());
    level* l = zone[z];
    int x = p.x - cube_size.left;
    int y = p.y - cube_size.top;
    assert(y>=0 && y<(int)l->rooms.size());
    row* r = l->rooms[y];
    assert(x>=0 && x<(int)r->rr.size());
    return &r->rr[x];
}

bool Rooms3dCube::extends(const Rooms3dCubePos& p)
{
    if (checkCoords(p))
        return true;
    //1. extends rows on all levels (y)
    extends_height(p);
    // 2. extends rows by new rooms (x)
    extends_width(p);
    // 3. extends levels (z)
    extends_levels(p);
    return true;
}

void Rooms3dCube::extends_height(const Rooms3dCubePos& p)
{
    //extends rows on all levels (y)
    int count = 0;
    if (p.y < cube_size.top)
        count = cube_size.top - p.y;
    if (p.y > cube_size.bottom)
        count = cube_size.bottom - p.y;
    if (count == 0) return;

    for (int i=0; i<cube_size.levels(); ++i) 
    {
       level *l = zone[i];
       std::vector<row*>& v = l->rooms;
       if (count > 0) {
            for(int c = count; c>0; --c)
                v.insert(v.begin(), new row(cube_size.width()) );
            cube_size.top = p.y;
       } else {
            for(int c = -count; c>0; --c)
               v.push_back( new row(cube_size.width()) );
       }
    }
    if (count > 0)
        cube_size.top = p.y;
    else
        cube_size.bottom = p.y;
}

void Rooms3dCube::extends_width(const Rooms3dCubePos& p)
{
    // extends rows by new rooms (x)
    int count = 0;
    if (p.x < cube_size.left)
        count = cube_size.left - p.x;
    if (p.x > cube_size.right)
        count = cube_size.right - p.x;
    if (count == 0) return;
    
    for (int i=0; i<cube_size.levels(); ++i)
    {
        level *l = zone[i];
        std::vector<row*>& v = l->rooms;
        for (int j=0, je=v.size(); j<je; ++j)
        {
            std::vector<Room*>& rr = v[j]->rr;
            if (count > 0)
              rr.insert(rr.begin(), count, NULL);              
            else
              rr.insert(rr.end(), -count, NULL);
        }
    }
    if (count > 0)
        cube_size.left = p.x;
    else
        cube_size.right = p.x;
}

void Rooms3dCube::extends_levels(const Rooms3dCubePos& p)
{
    // extends levels (z)
    int count = 0;
    if (p.z < cube_size.minlevel)
        count = cube_size.minlevel - p.z;
    if (p.z > cube_size.maxlevel)
        count = cube_size.maxlevel - p.z;
    if (count == 0) return;
    bool insert_begin = (count > 0) ? true : false;
    if (count < 0) count = -count;
    for (;count > 0; --count)
    {
        level *l = new level;
        l->rooms.resize(cube_size.height(), NULL);
        for (int i=0;i<cube_size.height(); ++i)
            l->rooms[i] = new row(cube_size.width());
        if (insert_begin)
            zone.insert(zone.begin(), l);
        else
            zone.push_back(l);
    }
    if (insert_begin)
        cube_size.minlevel = p.z;
    else
        cube_size.maxlevel = p.z;
}

bool Rooms3dCube::emptyLevel(level *l)
{
    for (row *r : l->rooms) {
        for (Room* room : r->rr ) {
            if (room) return false;
        }
    }
    return true;
}

void Rooms3dCube::collapse()
{
    int levels = zone.size();
    // collapse from top level
    while (levels > 1) {
         level *l = zone[0];
         if (!emptyLevel(l))
             break;
         zone.erase(zone.begin());
         delete l;
         levels = zone.size();
         cube_size.maxlevel -= 1;
    }
    // collapse from bottom level
    while (levels > 1) {
         int bottom = levels-1;
         level *l = zone[bottom];
         if (!emptyLevel(l))
             break;
         zone.erase(zone.begin()+bottom);
         delete l;
         levels = zone.size();
         cube_size.minlevel += 1;
    }
    // collapse from left    
    while(true) {
        bool trim_left = true;
        for (level *l : zone) {
        if (!trim_left) break;
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            if (rr.size() < 2 || rr[0]) { trim_left = false; break; }
        }}
        if (!trim_left)
            break;
        for (level *l : zone) {
        for (row *r : l->rooms) {
            r->rr.erase(r->rr.begin());
        }}
        cube_size.left += 1;
    }
    // collapse from right
    while (true) {
        bool trim_right = true;
        for (level *l : zone) {
        if (!trim_right) break;
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            int last = rr.size()-1;
            if (last == 0 || rr[last]) { trim_right = false; break; }
        }}
        if (!trim_right)
            break;
        for (level *l : zone) {
        for (row *r : l->rooms) {
            std::vector<Room*> &rr = r->rr;
            int last = rr.size()-1;
            rr.erase(rr.begin()+last);
        }}
        cube_size.right -= 1;
    }

    // collapse from top
    while (true) {        
        bool trim_top = true;
        for (level *l : zone) {
            if (l->rooms.size()==1 || !trim_top) break;
            row *r = l->rooms[0];
            for (Room *room : r->rr) {
                if (room) { trim_top = false; break; }
            }
        }
        if (!trim_top)
            break;
        for (level *l : zone) {
             row *r = l->rooms[0];
             l->rooms.erase(l->rooms.begin());
             delete r;
        }
        cube_size.top += 1;
    }

    // collapse from bottom
    while (true) {        
        bool trim_bottom = true;
        for (level *l : zone) {
            if (l->rooms.size()==1 || !trim_bottom) break;
            int last = l->rooms.size()-1;
            row *r = l->rooms[last];
            for (Room *room : r->rr) {
                if (room) { trim_bottom = false; break; }
            }
        }
        if (!trim_bottom)
            break;
        for (level *l : zone) {
             int last = l->rooms.size()-1;
             row *r = l->rooms[last];
             l->rooms.pop_back();
             delete r;
        }
        cube_size.bottom -= 1;
    }
}

bool Rooms3dCube::checkCoords(const Rooms3dCubePos& p) const
{
    if (p.z >= cube_size.minlevel && p.z <= cube_size.maxlevel)
    {
        if (p.x >= cube_size.left && p.x <= cube_size.right && p.y >= cube_size.top && p.y <= cube_size.bottom) 
        {
            return true;
        }
    }
    return false;
}

void Rooms3dCube::clearExits(Room *r)
{
    RoomDirHelper dh;
    for (int i=0,e=ROOM_DIRS_COUNT; i<e; ++i)
    {
        RoomDir dir = dh.cast(i);
        if (dir == RD_UNKNOWN) continue;
        RoomDir rdir = dh.revertDir(dir);
        Room *next = r->dirs[i].next_room;
        if (next && next->dirs[rdir].next_room == r) { next->dirs[rdir].next_room = NULL; }
    }
}

RoomDir RoomDirHelper::cast(int index)
{
    if (index >= 0 && index < ROOM_DIRS_COUNT)
        return static_cast<RoomDir>(index);
    assert(false);
    return RD_UNKNOWN;
}

int RoomDirHelper::index(RoomDir dir)
{
    return static_cast<int>(dir);
}

const wchar_t* unknownDirName = L"unknown";
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
    return unknownDirName;
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