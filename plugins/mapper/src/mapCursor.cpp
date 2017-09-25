#include "stdafx.h"
#include "mapInstance.h"
#include "mapCursor.h"

Rooms3dCubeSize MapCursorImplementation::m_empty;
Rooms3dCubePos  MapCursorImplementation::m_empty_pos;

void MapCursorImplementation::init()
{
    map_zone = NULL;
    if (!room_ptr)
        return;
    int zoneid = room_ptr->pos.zid;
    map_zone = map_ptr->findZone(zoneid);
}

const Rooms3dCubeSize& MapCursorImplementation::size() const
{
    return (map_zone) ? map_zone->size() : m_empty;
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
    if (!map_zone)
        return NULL;
    return map_zone->getRoom(p);
}

const Rooms3dCube* MapCursorImplementation::zone() const
{
    return map_zone;
}

bool MapCursorImplementation::valid() const
{
     return map_zone ? true : false;
}

MapCursorInterface* MapCursorImplementation::dublicate()
{
    return new MapCursorImplementation( map_ptr, room_ptr, ccolor); 
}

bool MapCursorImplementation::move(RoomDir dir)
{
     return false;
}
//-----------------------------------------------------------
Rooms3dCubeSize MapZoneCursorImplementation::m_empty;

void MapZoneCursorImplementation::init(const Rooms3dCube *zone, int level)
{
    assert(zone);
    map_zone = zone;
    m_zone_pos.zid = zone->id();
    m_zone_pos.z = level;
}

const Rooms3dCubeSize& MapZoneCursorImplementation::size() const
{
    return (map_zone) ? map_zone->size() : m_empty;
}

const Rooms3dCubePos& MapZoneCursorImplementation::pos() const
{
    return m_zone_pos;
}

MapCursorColor MapZoneCursorImplementation::color() const
{
    return RCC_NONE;
}

const Room* MapZoneCursorImplementation::room(const Rooms3dCubePos& p) const
{
    if (!map_zone)
        return NULL;
    return map_zone->getRoom(p);
}

const Rooms3dCube* MapZoneCursorImplementation::zone() const
{
    return map_zone;
}

bool MapZoneCursorImplementation::valid() const
{
    return map_zone ? true : false;
}

 MapCursorInterface* MapZoneCursorImplementation::dublicate()
 {
    return new MapZoneCursorImplementation(map_ptr, map_zone, m_zone_pos.z); 
 }

 bool MapZoneCursorImplementation::move(RoomDir dir)
 {
     if (dir == RD_UP || dir == RD_DOWN) {
         Rooms3dCubePos pos(m_zone_pos);
         pos.move(dir);
         if (pos.valid( size() )) 
         {
             m_zone_pos = pos;
             return true;         
         }
     }
     return false;
}