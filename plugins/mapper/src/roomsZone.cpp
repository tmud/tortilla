#include "stdafx.h"
#include "roomsZone.h"

Rooms3dCube::AR_STATUS Rooms3dCube::addRoom(const Rooms3dCubePos& p, Room* r)
{
    if (r->pos.zid != -1)
        return AR_INVALIDROOM;
    if (rooms.get(p))
        return AR_BUSY;
    rooms.add(p, r, z_id);
    m_hashmap[r->roomdata.hash()] = r;
    return AR_OK;
}

const Room* Rooms3dCube::getRoom(const Rooms3dCubePos& p) const
{
    return rooms.get(p);
}

void Rooms3dCube::deleteRoom(const Rooms3dCubePos& p)
{
    Room *r = detachRoom(p);
    if (r) { clearExits(r); delete r; }
    rooms.collapse();
}

Room* Rooms3dCube::detachRoom(const Rooms3dCubePos& p)
{
    Room *r = rooms.detach(p);
    if (r)
    {
        hashmap_iterator it = m_hashmap.find(r->roomdata.hash());
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
    rooms.collapse();
}

bool Rooms3dCube::setAsLevel0(int level)
{
    return rooms.setAsLevel0(level);
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
