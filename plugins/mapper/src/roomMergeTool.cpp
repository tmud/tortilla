#include "stdafx.h"
#include "roomMergeTool.h"

RoomMergeTool::RoomMergeTool(Rooms3dCube *z) : zone(z)
{
    assert(z);
#ifdef _DEBUG
    z->resetColors();
#endif
}

bool RoomMergeTool::tryMakeNewZone(const Room* room, RoomDir dir)
{
    clear();
    if (!room || dir == RoomDir::RD_UNKNOWN || room->pos.zid != zone->id() ) {
        assert(false);
        return false;
    }
    if (!runWaveAlgoritm(room->pos, dir))
        return false;

    branches_const_iterator bt = branches.cbegin(), bt_end = branches.cend();
    for (; bt != bt_end; ++bt) {
        if (bt->second == room) 
        {
            // cycle in rooms graph
            return false;
        }
    }

    deleteRoom(room);

#ifdef _DEBUG
    const_iterator it = nodes.cbegin(), it_end = nodes.end();
    for (; it != it_end; ++it) {
        const Room* r = it->first;
        r->use_color = 1;
        r->color = RGB(200,200,0);
    }
#endif
    return true;
}

void RoomMergeTool::makeNewZone(const tstring& name)
{

}

void RoomMergeTool::deleteRoom(const Room* room)
{
    std::vector<branch> new_branches;
    branches_const_iterator bt = branches.cbegin(), bt_end = branches.cend();
    for (; bt != bt_end; ++bt) {
        if (bt->first == room || bt->second == room) 
            continue;
        new_branches.push_back(*bt);
    }
    branches.swap(new_branches);
    iterator it = nodes.find(room);
    if (it != nodes.end())
            nodes.erase(it);
}

void RoomMergeTool::clear()
{
    nodes.clear();
    branches.clear();
}

bool RoomMergeTool::runWaveAlgoritm(const Rooms3dCubePos& start, RoomDir dir)
{
    Rooms3dCubePos s(start);
    const Room* r0 = zone->getRoom(s);
    if (!r0)
        return false;
    nodes[r0] = 1;
    if (!s.move(dir))
    {
        nodes.clear();
        return false;
    }
    const Room* r = zone->getRoom(s);
    if (!r)
    {
        nodes.clear();
        return false;
    }
    nodes[r] = 2;
    branch b; b.first = r0; b.second = r;
    branches.push_back(b);

    int weight = 3;
    std::vector<const Room*> cursors, next_cursors;
    cursors.push_back(r);
    while (!cursors.empty())
    {
        for (int i=0,e=cursors.size();i<e;++i)
        {
            const Room* r = cursors[i];
            for (int rd = beginRoomDir; rd<=endRoomDir; rd++)
            {
                const Room* next = r->dirs[rd].next_room;
                if (next && next->pos.zid == r->pos.zid) 
                {
                    if (exist(next))
                        continue;
                    nodes[next] = weight;
                    next_cursors.push_back(next);
                    b.first = r; b.second = next;
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


int RoomMergeTool::index(const Room* r) const
{
    const_iterator it = nodes.find(r);
    return (it == nodes.end()) ? -1 : it->second;
}

bool RoomMergeTool::exist(const Room* r) const
{
    return (index(r) == -1) ? false : true;
}
