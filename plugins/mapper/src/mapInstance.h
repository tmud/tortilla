#pragma once
#include "roomObjects.h"
#include "dirObjects.h"
#include "mapCursor.h"
#include "mapTools.h"

class MapInstance
{
    friend class MapCursorImplementation;
    friend class MapZoneCursorImplementation;
public:
    MapInstance();
    ~MapInstance();
    MapCursor createCursor(Room *room, MapCursorColor color);
    MapCursor createZoneCursor(int zoneid);
    MapTools getTools();
    Room* findRoom(const tstring& hash);
    bool addNewZone(Room* firstroom);
    bool addNewRoom(Room* from, Room* newroom, RoomDir dir);
    bool addLink(Room* from, Room* to, RoomDir dir);
    void saveMaps(const tstring& dir);
    void loadMaps(const tstring& dir);
    void clearMaps();
	void getZonesIds(std::vector<int>* ids);
private:
    Rooms3dCube* findZone(int zid);
    Rooms3dCube* findZone(const tstring& name);
    bool  isMultiExit(Room* from, RoomDir dir);
    bool  setMultiExit(Room* from, RoomDir dir);
    Room* getRoom(Room* from, RoomDir dir);
    bool  setRoomOnMap(Room* from, Room* next, RoomDir dir);
    void  addRoomToHashTable(Room* r);
    void  removeRoomFromHashTable(Room *r);
    tstring  getNewZoneName(const tstring& templ);
    void clear();
private:
    int m_nextzone_id;
	struct zonedata {
		zonedata(Rooms3dCube* z) : zone(z) { assert(zone); }
		Rooms3dCube* zone;
		tstring hash;
	};
	std::vector<zonedata> zones;
	typedef std::vector<zonedata>::iterator zone_iterator;
    std::map<tstring, Room*> rooms_hash_table;
    typedef std::map<tstring, Room*>::iterator room_iterator;
    struct exitdata {
      Room *room;
      RoomDir dir;
      tstring vnum;
    };
};

class MapCursorImplementation : public MapCursorInterface
{
public:
    MapCursorImplementation(MapInstance* m, Room *r, MapCursorColor c) : map_ptr(m), room_ptr(r), ccolor(c)
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
    MapCursorInterface* dublicate();
    bool move(RoomDir dir);
private:
    void init();
    MapInstance *map_ptr;
    Room *room_ptr;
    MapCursorColor ccolor;
    Rooms3dCube* map_zone;
    static Rooms3dCubeSize m_empty;
    static Rooms3dCubePos m_empty_pos;
};

class MapZoneCursorImplementation : public MapCursorInterface
{
public:
    MapZoneCursorImplementation(MapInstance* m, int zoneid, int level) : map_ptr(m), map_zone(NULL)
    {
        assert(m);
        init(zoneid, level);
    }
    ~MapZoneCursorImplementation() {}
protected:
    const Rooms3dCubeSize& size() const;
    const Rooms3dCubePos& pos() const;
    const Room* room(const Rooms3dCubePos& p) const;
    MapCursorColor color() const;
    const Rooms3dCube* zone() const;
    bool valid() const;
    MapCursorInterface* dublicate();
    bool move(RoomDir dir);
private:
    void init(int zoneid, int level);
    MapInstance *map_ptr;
    const Rooms3dCube* map_zone;
    static Rooms3dCubeSize m_empty;
    Rooms3dCubePos m_zone_pos;
};

class MapNullCursorImplementation : public MapCursorInterface 
{
public:
    MapNullCursorImplementation() {}
    const Rooms3dCubeSize& size() const { return m_null_size; }
    const Rooms3dCubePos& pos() const { return m_null_pos; }
    const Room* room(const Rooms3dCubePos& p) const { return NULL; }
    MapCursorColor color() const { return RCC_NONE; }
    const Rooms3dCube* zone() const { return NULL; }
    bool valid() const { return false; }
    MapCursorInterface* dublicate() { return new MapNullCursorImplementation(); }
    bool move(RoomDir dir) { return false; }
private:
    Rooms3dCubePos m_null_pos;
    Rooms3dCubeSize m_null_size;
};