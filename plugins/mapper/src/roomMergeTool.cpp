#include "stdafx.h"
#include "roomMergeTool.h"

void MapWaveAlgorithm::wave(MapWaveArray &wa, MapWaveArrayPos& start)
{

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
    MapWaveAlgorithm alg;
    alg.wave(wa, p);
        
    return false;
}
