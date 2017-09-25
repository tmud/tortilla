#include "stdafx.h"
#include "mapTools.h"
#include "mapSmartTools.h"

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

Rooms3dCube* MapNewZoneTool::applyMakeNewZone(const tstring& zoneName)
{
    if (!waveTool)
        return NULL;
    std::vector<const Room*> constr;
    waveTool->getNewZoneRooms(&constr);
    deleteWaveTool();
    if (constr.empty())
        return NULL;
    std::vector<Room*> rooms;
    for(const Room* r : constr) {
       Rooms3dCube *z = map->findZone(r->pos.zid);
       if (!z) {
           assert(false);
           return NULL;
       }
       rooms.push_back(const_cast<Room*>(r));
    }
    Rooms3dCube* new_zone =  map->createNewZone(zoneName);
    for (Room *r : rooms) 
    {
        Rooms3dCube *z = map->findZone(r->pos.zid);
        Rooms3dCubePos p (r->pos);
        if (z->detachRoom(p))
            new_zone->addRoom(p, r);
    }
	return new_zone;
}

void MapNewZoneTool::deleteWaveTool()
{
    if (waveTool)
        delete waveTool;
    waveTool = NULL;
}



/*RoomFreePlaceTool::RoomFreePlaceTool(Rooms3dCube *z) : zone(z) 
{
    assert(z);
}
bool RoomFreePlaceTool::tryFreePlace(const Room* room, RoomDir dir)
{
    if (!room || dir == RoomDir::RD_UNKNOWN || room->pos.zid != zone->id() ) {
        assert(false);
        return false;
    }

    //check free place
    Rooms3dCubePos p = room->pos;
    if (!p.move(dir)) {
        assert(false);
        return false;
    }
    const Room *cnext = zone->getRoom(p);
    if (!cnext)
        return true;
    
     //todo! try move the room
    
    return false;
}*/