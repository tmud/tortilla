#include "stdafx.h"
#include "mapInstance.h"

Rooms3dCubeSize MapCursorImplementation::m_empty;
Rooms3dCubePos  MapCursorImplementation::m_empty_pos;

void MapCursorImplementation::init()
{
    zone = NULL;
    if (!room_ptr)
        return;
    int zid = room_ptr->pos.zid;
    int count = map_ptr->zones.size();
    if (zid >= 0 && zid < count)
        zone = map_ptr->zones[zid];
    if (zone)
        not_empty = true;
}

const Rooms3dCubeSize& MapCursorImplementation::size() const
{
    return (zone) ? zone->size() : m_empty;
}

const Rooms3dCubePos& MapCursorImplementation::pos() const
{
    return (room_ptr) ? room_ptr->pos : m_empty_pos;
}

MapCursorColor MapCursorImplementation::color() const
{
    return ccolor;
}

const Room* MapCursorImplementation::room(const Rooms3dCubePos& p) const
{
    if (!zone)
        return NULL;
    return zone->getRoom(p);
}

bool MapCursorImplementation::valid() const
{
    return not_empty;
}
