#include "stdafx.h"
#include "roomsZone.h"
#include "mapMergeTools.h"

MapZoneMirror::MapZoneMirror()
{
}

MapZoneMirror::~MapZoneMirror() 
{
}

bool MapZoneMirror::tryMergeZone(const Rooms3dCube* z1, const Rooms3dCube* z2)
{
    makeRoomsCopies(z1);
    makeRoomsCopies(z2);
    if (!linkRoomsExits(z1, z2)) {
        deleteCopies();
        return false;
    }
    // fill rooms containers from zone1
    int id1 = z1->id();
    iterator it = i2c.begin(), it_end = i2c.end();
    for (; it != it_end; ++it) {
        Room *d = it->second;
        if (d->pos.zid == id1)
        {
            rooms.add(d->pos, d, id1);
        }
    }
    rooms.collapse();
    const Rooms3dCubeSize &sz = z1->size();
    const Rooms3dCubeSize &sz2 = rooms.getSize();
    bool compare = (sz2.left == sz.left && sz.right == sz2.right && sz.top == sz2.top && sz.bottom == sz2.bottom
        && sz.maxlevel == sz2.maxlevel && sz.minlevel == sz2.minlevel);
    if (!compare)
    {
        assert(false);
        int id2 = z2->id();
        deleteCopies(id2); // other rooms deleted by rooms container
        return false;
    }

    // try merge zone2 over zone1 




    return false;
}

void MapZoneMirror::makeRoomsCopies(const Rooms3dCube* z)
{
    // make zone's rooms deep copies (without exits yet)
    const Rooms3dCubeSize &sz = z->size();
    for (int l = sz.minlevel; l <= sz.maxlevel; ++l)
    {
        Rooms3dCubePos p; p.z = l;
        for (int y = sz.top; y <= sz.bottom; ++y)
        {
            p.y = y;
            for (int x = sz.left; x <= sz.right; ++x)
            {
                p.x = x;
                const Room *r = z->getRoom(p);
                if (!r) continue;
                Room *room_copy = new Room();
                room_copy->roomdata = r->roomdata;
                room_copy->pos = r->pos;
                room_copy->use_color = r->use_color;
                room_copy->color = r->color;
                room_copy->icon = r->icon;
                i2c[r] = room_copy;
            }
        }
    }
}

bool MapZoneMirror::linkRoomsExits(const Rooms3dCube* z1, const Rooms3dCube* z2)
{
    int id1 = z1->id();
    int id2 = z2->id();
    iterator it = i2c.begin(), it_end = i2c.end();
    for (; it != it_end; ++it) {

        const Room* s = it->first;
        Room *d = it->second;
        for (int rd = beginRoomDir; rd <= endRoomDir; ++rd)
        {
            const RoomExit &e = s->dirs[rd];
            d->dirs[rd] = e;
            const Room* next = e.next_room;
            if (!next)
                continue;
            int next_zone_id = next->pos.zid;
            if (next_zone_id == id1 || next_zone_id == id2)
            {
                d->dirs[rd].next_room = nullptr;
                iterator nt = i2c.find(next);
                if (nt == i2c.end()) {
                   assert(false);
                   return false;
                }
                d->dirs[rd].next_room = nt->second;
            }
        }
    }
    return true;
}

void MapZoneMirror::deleteCopies(int zid)
{
    iterator it = i2c.begin(), it_end = i2c.end();
    for (; it != it_end; ++it) {
        Room *d = it->second;
        if (d->pos.zid == zid) {
            delete d;
            i2c.erase(it);
        }
    }    
}

void MapZoneMirror::deleteCopies()
{
    deleteCopies(-1); // all zones
    i2c.clear();
}
