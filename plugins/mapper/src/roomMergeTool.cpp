#include "stdafx.h"
#include "roomMergeTool.h"

RoomMergeTool::RoomMergeTool(Rooms3dCube *z) : zone(z)
{
    assert(z);
}

bool RoomMergeTool::makeNewZone(const Room* room, RoomDir dir)
{
    if (!room || dir == RoomDir::RD_UNKNOWN || room->pos.zid != zone->id() ) {
        assert(false);
        return false;
    }
    if (!runWaveAlgoritm(room->pos, dir))
        return false;

    using pairtype=std::pair<const Room*, int>;
    pairtype t = *std::max_element(wave.begin(), wave.end(), [] (const pairtype & p1, const pairtype & p2) { return p1.second < p2.second;});
    int max_weight = t.second;

    std::vector<const Room*> rooms;
    for (const_iterator it = wave.begin(), it_end=wave.end(); it!=it_end; ++it) {
        if (it->second == max_weight)
            rooms.push_back(it->first);
    }


    return true;
}

bool RoomMergeTool::runWaveAlgoritm(const Rooms3dCubePos& start, RoomDir dir)
{
    Rooms3dCubePos s(start);
    const Room* r = zone->getRoom(s);
    if (!r)
        return false;
    wave[r] = 1;
    if (!s.move(dir))
    {
        wave.clear();
        return false;
    }
    r = zone->getRoom(s);
    if (!r)
    {
        wave.clear();
        return false;
    }
    wave[r] = 2;

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
                    wave[next] = weight;
                    next_cursors.push_back(next);
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
    const_iterator it = wave.find(r);
    return (it == wave.end()) ? -1 : it->second;
}

bool RoomMergeTool::exist(const Room* r) const
{
    return (index(r) == -1) ? false : true;
}
