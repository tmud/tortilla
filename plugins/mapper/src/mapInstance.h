#pragma once
#include "dirObjects.h"
#include "levelZoneObjects.h"

class MapInstance
{
public:
    MapInstance();
    ~MapInstance();
    Room* findRoom(const tstring& vnum);
    bool addNewZoneAndRoom(Room* newroom);
    bool addNewRoom(Room* from, Room* newroom, RoomDir dir);
    bool addLink(Room* from, Room* to, RoomDir dir);


    //void saveMaps(lua_State *L);
    //void loadMaps(lua_State *L);

private:
    bool  isMultiExit(Room* from, RoomDir dir);
    bool  setMultiExit(Room* from, RoomDir dir);
    Room* getRoom(Room* from, RoomDir dir);
    bool  setRoom(Room* from, RoomDir dir, Room* next);
private:
    void  setRoomOnMap(Room* from, RoomDir dir, Room* next);

private:
    std::map<tstring, Zone*> m_zones;
    typedef std::map<tstring, Zone*>::iterator zone_iterator;
    int m_nextzone_id;
    std::map<tstring, Room*> m_rooms;
    typedef std::map<tstring, Room*>::iterator room_iterator;
};
