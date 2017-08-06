#include "stdafx.h"
#include "mapTools.h"

MapToolsImpl::MapToolsImpl(const Room* start, std::vector<Rooms3dCube*>& szones)
{    
    std::vector<Rooms3dCube*>::iterator it = szones.begin(), it_end = szones.end();
    for (; it!=it_end; ++it) {
        Rooms3dCube* zone = *it;
        int id = zone->id();
        assert(zones.find(id)==zones.end());
        zones[id] = zone;
    }
    int roomZoneid = start->pos.zid;





}