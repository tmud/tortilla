#pragma once
#include "mapInstance.h"
#include "roomsWave.h"
//#include "roomObjects.h"
#include "mapCursor.h"

/*class RoomWaveAlgoritm;
class MapToolsImpl
{
public:
    MapToolsImpl(std::vector<Rooms3dCube*> &zones);
    ~MapToolsImpl();
    bool tryMakeNewZone(const Room* room, RoomDir dir);
	Rooms3dCube* applyMakeNewZone(const tstring& zoneName);
private:
    Room *castRoom(const Room* room) const;
    int newZoneId() const;
    Rooms3dCube* findZone(int zid) const;    
    
    void deleteWaveTool();
};

typedef std::shared_ptr<MapToolsImpl> MapTools;
*/

class MapTools
{
    MapInstance *map;
public:
    MapTools(MapInstance *m) : map(m) { assert(m); }
    Room* findRoom(const tstring& hash);
    bool  createNewZone(Room *firstroom);
    bool  addNewRoom(Room* from, Room* newroom, RoomDir dir);
    
    MapCursor createCursor(Room *room, MapCursorColor color);
    MapCursor createZoneCursor(Rooms3dCube* zone);

private:
    bool setRoomOnMap(Room* from, Room* next, RoomDir dir);
};

class MapNewZoneTool 
{
    MapInstance *map;
public:
    MapNewZoneTool(MapInstance *m) : map(m), waveTool(NULL) { assert(m); }    
    ~MapNewZoneTool() { deleteWaveTool(); }
    bool tryMakeNewZone(const Room* room, RoomDir dir);
	bool applyMakeNewZone(const tstring& zoneName);
private:
    void deleteWaveTool();
    RoomWaveAlgoritm *waveTool;
};

/*class RoomFreePlaceTool {
public:
    RoomFreePlaceTool(Rooms3dCube *z);
    bool tryFreePlace(const Room* room, RoomDir dir);
private:
     Rooms3dCube *zone;
};*/