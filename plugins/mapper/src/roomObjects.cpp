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
        collapse(p);
    }
    return r;
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

void Rooms3dCube::collapse(const Rooms3dCubePos& p)
{
    if (p.x == cube_size.left) {
    }
    if (p.x == cube_size.right) {       
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