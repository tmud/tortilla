#include "stdafx.h"
#include "mapTools.h"
#include "mapSmartTools.h"
#include "roomObjects.h"

bool MapTools::setRoomOnMap(Room* from,  Room* next, RoomDir dir)
{
    assert(from && next && dir != RD_UNKNOWN);
    Rooms3dCubePos pos = from->pos;
    if (!pos.move(dir))
        return false;
    Rooms3dCube* zone = NULL;
    Rooms3dCubeList zones;
    map->getZones(&zones);
    for (Rooms3dCube *z : zones) {
        if (z->id() == pos.zid)
            { zone = z; break; }
    }
    if (!zone) {
        assert(false);
        return false;
    }
    if (findRoom(next->hash())) {
        assert(false);
        return false;
    }
    Rooms3dCube::AR_STATUS s = zone->addRoom(pos, next);
    if (s == Rooms3dCube::AR_OK)
        return true;
    if (s == Rooms3dCube::AR_INVALIDROOM || s == Rooms3dCube::AR_FAIL) {
		assert(false);
        return false;
    }
     if (s != Rooms3dCube::AR_BUSY)
         return false;

     return createNewZone(next);
}

Room* MapTools::findRoom(const tstring& hash)
{
    Rooms3dCubeList zones;
    map->getZones(&zones);
    for (Rooms3dCube *z : zones) {
        Room *r = z->findRoom(hash);
        if (r)
            return r;
    }
    return NULL;
}

bool MapTools::createNewZone(Room *firstroom)
{
    Room *room = firstroom;
    if (!room || room->roomdata.vnum.empty() || findRoom(room->roomdata.vnum))
    {
        assert(false);
        return false;
    }
    Rooms3dCube *newzone = map->createNewZone();
    Rooms3dCubePos p;
    newzone->addRoom(p, room);
    return true;
}

bool MapTools::addNewRoom(Room* from, Room* newroom, RoomDir dir)
{
    if (!from || !newroom || dir == RD_UNKNOWN || newroom->roomdata.vnum.empty())
    {
        assert(false);
        return false;
    }
    MapSmartTools st;
    if (st.isMultiExit(from, dir))
    {
        return setRoomOnMap(from, newroom, dir);
    }

    Room* next = st.getRoom(from, dir);
    if (!next)
    {
        bool result = setRoomOnMap(from, newroom, dir);
        if (result)
            st.addLink(from, newroom, dir);
        return result;
    }

    // конфликтный мультивыход
    st.setMultiExit(from, dir);
    return setRoomOnMap(from, newroom, dir);
}


MapCursor MapTools::createCursor(Room *room, MapCursorColor color)
{
    return std::make_shared<MapCursorImplementation>(map, room, color);
}

MapCursor MapTools::createZoneCursor(Rooms3dCube* zone)
{
    return std::make_shared<MapZoneCursorImplementation>(map, zone, 0);
}

bool MapNewZoneTool::tryMakeNewZone(const Room* room, RoomDir dir)
{
    MapTools t(map);
    Room *r = t.findRoom(room->hash());
    if (!r || dir == RD_UNKNOWN) {
        assert(false);
        return false;
    }
    Rooms3dCube* z = map->findZone(room->pos.zid);
    if (!z) {
        assert(false);
        return false;
    }
    deleteWaveTool();
    waveTool = new RoomWaveAlgoritm();
    if (!waveTool->runWaveAlgoritm(z, room, dir)) {
        deleteWaveTool();
        return false;
    }
    return true;
}

bool MapNewZoneTool::applyMakeNewZone(const tstring& zoneName)
{
    if (!waveTool) {
        assert(false);
        return false;
    }
    std::vector<const Room*> constr;
    waveTool->getNewZoneRooms(&constr);
    deleteWaveTool();
    if (constr.empty())
        return false;
    std::vector<Room*> rooms;
    for(const Room* r : constr) {
       Rooms3dCube *z = map->findZone(r->pos.zid);
       if (!z) {
           assert(false);
           return false;
       }
       rooms.push_back(const_cast<Room*>(r));
    }

    Rooms3dCube* t = nullptr;
    Rooms3dCube* new_zone =  map->createNewZone(zoneName);
    for (Room *r : rooms) 
    {
        Rooms3dCube *z = map->findZone(r->pos.zid);
        Rooms3dCubePos p (r->pos);
        if (z->detachRoom(p))
        {
            t = z;
            z->optimizeSize();            
            Rooms3dCube::AR_STATUS status = new_zone->addRoom(p, r);
            assert(status ==Rooms3dCube::AR_OK );
        }
    }
    new_zone->optimizeSize();
	return true;
}

void MapNewZoneTool::deleteWaveTool()
{
    if (waveTool)
        delete waveTool;
    waveTool = NULL;
}

bool MapMoveRoomToolToAnotherZone::tryMoveRoom(const Room* room, RoomDir dir)
{
    RoomHelper c(room);
    if (!c.isZoneExit(dir)) {
        assert(false);
        return false;
    }

    MapTools t(map);
    Room *r = t.findRoom(room->hash());   
    Rooms3dCube* srczone = map->findZone(r->pos.zid);
    if (!srczone) {
        assert(false);
        return false;
    }
    MapSmartTools st;
    Room *dest_r = st.getRoom(r, dir);
    if (!dest_r || dest_r->pos.zid == r->pos.zid) {
        assert(false);
        return false;
    }
    Rooms3dCube* dstzone = map->findZone(dest_r->pos.zid);
    if (!dstzone) {
       assert(false);
       return false;
    }
    Rooms3dCubePos pos(dest_r->pos);
    RoomDirHelper dh;
    if (!pos.move(dh.revertDir(dir))) {
        assert(false);
        return false;
    }
    if (!dstzone->getRoom(pos)) {
        Room *detached = srczone->detachRoom(r->pos);
        assert(detached == r);
        Rooms3dCube::AR_STATUS status = dstzone->addRoom(pos, r);
        assert (status == Rooms3dCube::AR_OK);
        return (status == Rooms3dCube::AR_OK);    
    }
    return false;
}

bool MapMoveRoomByMouse::tryMoveRoom(const Room* room, int x, int y)
{
     const Rooms3dCubePos& from = room->pos;
     Rooms3dCube* zone = map->findZone(from.zid);
     if (!zone) {
         assert(false);
         return false;
     }
     /*if (!p.valid(zone->size()))  {        
         assert(false);
         return false;
     }*/

     Rooms3dCubePos p = from;
     const Rooms3dCubeSize& sz = zone->size();
     
     int dx = x - (p.x - sz.left);
     int dy = y - (p.y - sz.top);

     p.x += dx;
     p.y += dy;
         
     if (zone->getRoom(p))
     {
         // blocked place
         return false;
     }
       
     Room *r = zone->detachRoom(from);
     Rooms3dCube::AR_STATUS status = zone->addRoom(p, r);
     if (status != Rooms3dCube::AR_OK) 
     {
         zone->addRoom(from, r);
         assert(false);
         return false;
     }
     return true;
}