#include "stdafx.h"
#include "roomsZone.h"
#include "mapMergeTools.h"

bool MapZoneMergeTool::tryMergeZone(const Rooms3dCube* z1, const Rooms3dCube* z2)
{
    makeRoomsCopies(z1);
    makeRoomsCopies(z2);
    if (!linkRoomsExits(z1, z2)) {
        deleteCopies();
        return false;
    }
    // fill rooms containers from zone1
    int id1 = z1->id();
    int id2 = z2->id();
    iterator it = i2c.begin(), it_end = i2c.end();
    for (; it != it_end; ++it) 
    {
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
        deleteCopies(id2); // other rooms deleted by rooms container
        return false;
    }
    // try merge zone2 over zone1 
    // get border room in zone1 connected to zone2
    Room* z1z2 = nullptr; int dir = -1;
    it = i2c.begin(), it_end = i2c.end();
    for (; it != it_end; ++it)
    {
        Room *r = it->second;
        if (r->pos.zid != id1) continue;
        for (int rd = beginRoomDir; rd <= endRoomDir; ++rd)
        {
            Room *next = r->dirs[rd].next_room;
            if (!next) continue;
            if (next->pos.zid == id2)
            {
                z1z2 = r;
                dir = rd;
                break;
            }
        }
        if (z1z2) break;
    }
    if (!z1z2)
    {
        assert(false);
        return false;
    }




    return false;
}

void MapZoneMergeTool::makeRoomsCopies(const Rooms3dCube* z)
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

bool MapZoneMergeTool::linkRoomsExits(const Rooms3dCube* z1, const Rooms3dCube* z2)
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

void MapZoneMergeTool::deleteCopies(int zid)
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

void MapZoneMergeTool::deleteCopies()
{
    deleteCopies(-1); // all zones
    i2c.clear();
}


bool RoomWaveAlgoritm2::runFromRoom(Room* room)
{
    nodes.clear();
    branches.clear();
    nodes[room] = 1;
    std::vector<Room*> cursors;
    cursors.push_back(room);
    return run(2, room->pos.zid, cursors);
}

bool RoomWaveAlgoritm2::runFromBranch(Room* room, int dir)
{
    nodes.clear();
    branches.clear();
    Room* next = room->dirs[dir].next_room;
    std::vector<Room*> cursors;
    cursors.push_back(next);
    nodes[room] = 1;
    nodes[next] = 2;
    branch b; b.first = room; b.second = next;
    branches.push_back(b);
    return run(3, room->pos.zid, cursors);
}

bool RoomWaveAlgoritm2::run(int weight, int zoneid, std::vector<Room*>& cursors)
{
    std::vector<Room*> next_cursors;    
    while (!cursors.empty())
    {
        for (int i = 0, e = cursors.size(); i < e; ++i)
        {
            const Room* r = cursors[i];
            for (int rd = beginRoomDir; rd <= endRoomDir; rd++)
            {
                Room* next = r->dirs[rd].next_room;
                if (next && next->pos.zid == zoneid)
                {
                    if (exist(next))
                        continue;
                    nodes[next] = weight;
                    next_cursors.push_back(next);
                    branch b; b.first = r; b.second = next;
                    branches.push_back(b);
                }
            }
        }
        cursors.clear();
        cursors.swap(next_cursors);
        weight++;
    }
    return true;
}

void RoomWaveAlgoritm2::getRoomsInWave(std::vector<const Room*>* rooms)
{
    const_iterator it = nodes.cbegin(), it_end = nodes.end();
    for (; it != it_end; ++it)
        rooms->push_back(it->first);
}

bool RoomWaveAlgoritm2::exist(const Room* r) const
{
    return (index(r) == -1) ? false : true;
}

int RoomWaveAlgoritm2::index(const Room* r) const
{
    const_iterator it = nodes.find(r);
    return (it == nodes.end()) ? -1 : it->second;
}
