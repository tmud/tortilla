#pragma once
#include "mapInstance.h"
#include "roomsWave.h"
#include "mapCursor.h"

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
    void unselectRooms();
private:
    void deleteWaveTool();
    RoomWaveAlgoritm *waveTool;
};

class MapMoveRoomsToNewZoneTool
{
    MapInstance *map;
public:
    MapMoveRoomsToNewZoneTool(MapInstance *m) : map(m) { assert(m); }
	bool makeNewZone(std::vector<const Room*>& rooms, const tstring& zoneName);
};

class MapMoveRoomToolToAnotherZone
{
    MapInstance *map;
public:
    MapMoveRoomToolToAnotherZone(MapInstance *m) : map(m) { assert(m); }
    bool tryMoveRoom(const Room* room, RoomDir dir);
};

class MapMoveRoomByMouse 
{
    MapInstance *map;
public:
    MapMoveRoomByMouse(MapInstance *m) : map(m) { assert(m); }
    bool tryMoveRooms(std::vector<const Room*>& rooms, int x, int y);
};

class MapConcatZonesInOne
{
    MapInstance *map;
public:
    MapConcatZonesInOne(MapInstance *m) : map(m) { assert(m); }
    bool tryConcatZones(const Room* room, RoomDir dir);
};
