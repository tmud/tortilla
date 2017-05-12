#include "stdafx.h"
#include "roomMergeTool.h"

bool MapWaveAlgorithms::wave(MapWaveArray &wa, MapWaveArrayPos& start, RoomDir dir)
{
    wa.set(start, 1);
    std::vector<MapWaveArrayPos> cursors, next;
    cursors.push_back(start);
    for (int i=0,e=cursors.size(); i<e; ++i) {
        MapWaveArrayPos c = cursors[i];
        for (int rd = beginRoomDir; rd<=endRoomDir; rd++ ) {
            RoomDir d = static_cast<RoomDir>(rd);
            if (i == 0 && d == dir) continue;


        }
    }
}

MapWave::MapWave(Rooms3dCube *z) : zone(z)
{
    assert(z);
}

RoomMergeTool::RoomMergeTool(MapInstance *map, const Room* tracked) : zone(NULL), start_room(NULL)
{
    assert(map && tracked);
    const Rooms3dCubePos &pos = tracked->pos;
    zone = map->findZone(pos.zid);
    if (!zone) {
        assert(false);
        return;
    }
    start_room = zone->get(pos);
    if (!start_room) {
        zone = NULL;
        assert(false);
        return;
    }
    assert( start_room->roomdata.vnum == tracked->roomdata.vnum &&  start_room->roomdata.descr == tracked->roomdata.descr);
}

bool RoomMergeTool::makeNewZone(RoomDir dir)
{
    if (!start_room)
        return false;
    if (dir == RoomDir::RD_UNKNOWN)
    {
        assert(false);
        return false;
    }

    MapWaveArray wa(zone->size());
    MapWaveArrayPos p(start_room->pos);
    MapWaveAlgorithms alg;
    bool result = alg.wave(wa, p, dir);
    if (!result) {
    
    }        
    return false;
}
