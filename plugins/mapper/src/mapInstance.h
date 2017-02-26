#pragma once
#include "roomObjects.h"
#include "dirObjects.h"
#include "mapCursor.h"

class MapInstance
{
    friend class MapCursorImplementation;
public:
    MapInstance();
    ~MapInstance();
    MapCursor createCursor(Room *room, MapCursorColor color);
    MapCursor createZoneCursor(const Rooms3dCube *zone);
    Room* findRoom(const tstring& hash);
    bool addNewZoneAndRoom(Room* newroom);
    bool addNewRoom(Room* from, Room* newroom, RoomDir dir);
    bool addLink(Room* from, Room* to, RoomDir dir);    

    //void saveMaps(lua_State *L);
    //void loadMaps(lua_State *L);

private:
    Rooms3dCube* findZone(const tstring& name);
    bool  isMultiExit(Room* from, RoomDir dir);
    bool  setMultiExit(Room* from, RoomDir dir);
    Room* getRoom(Room* from, RoomDir dir);
    bool  setRoomOnMap(Room* from, Room* next, RoomDir dir);
    void  addRoomToHashTable(Room* r);
    void  removeRoomFromHashTable(Room *r);
private:
    int m_nextzone_id;
    std::vector<Rooms3dCube*> zones;
    typedef std::vector<Rooms3dCube*>::iterator zone_iterator;
    std::map<tstring, Room*> rooms_hash_table;
    typedef std::map<tstring, Room*>::iterator room_iterator;    
};

class MapCursorImplementation : public MapCursorInterface
{
public:
    MapCursorImplementation(MapInstance* m, Room *r, MapCursorColor c) : map_ptr(m), room_ptr(r), ccolor(c), not_empty(false)
    {
        assert(m);
        init();
    }
    ~MapCursorImplementation() {}
protected:
    const Rooms3dCubeSize& size() const;
    const Rooms3dCubePos& pos() const;
    const Room* room(const Rooms3dCubePos& p) const;
    MapCursorColor color() const;
    const Rooms3dCube* zone() const;
    bool valid() const;
private:
    void init();
    MapInstance *map_ptr;
    Room *room_ptr;
    MapCursorColor ccolor;
    Rooms3dCube* map_zone;
    bool not_empty;
    static Rooms3dCubeSize m_empty;
    static Rooms3dCubePos m_empty_pos;
};

class MapZoneCursorImplementation : public MapCursorInterface
{
public:
    MapZoneCursorImplementation(MapInstance* m, const Rooms3dCube *zone) : map_ptr(m), map_zone(zone)
    {
        assert(m && zone);
    }
    ~MapZoneCursorImplementation() {}
protected:
    const Rooms3dCubeSize& size() const;
    const Rooms3dCubePos& pos() const;
    const Room* room(const Rooms3dCubePos& p) const;
    MapCursorColor color() const;
    const Rooms3dCube* zone() const;
    bool valid() const;
private:
    MapInstance *map_ptr;
    const Rooms3dCube* map_zone;
    static Rooms3dCubeSize m_empty;
    static Rooms3dCubePos m_empty_pos;
};
