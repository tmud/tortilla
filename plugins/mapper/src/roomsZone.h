#pragma once
#include "roomsContainer.h"

class Rooms3dCube
{
    friend class Rooms3dCubeCursor;
    friend class MapToolsImpl;
public:
    Rooms3dCube(int zid, const tstring& name) : rooms(true), z_id(zid), m_name(name) {
        assert(zid >= 0);
    }
    ~Rooms3dCube() {}
    const Rooms3dCubeSize& size() const {  return rooms.getSize(); }
    const tstring& name() const { return m_name; }
    int id() const { return z_id; }
    enum AR_STATUS { AR_OK = 0, AR_INVALIDROOM, AR_BUSY };
    AR_STATUS addRoom(const Rooms3dCubePos& p, Room* r);
    const Room* getRoom(const Rooms3dCubePos& p) const;
    void  deleteRoom(const Rooms3dCubePos& p);
    Room* detachRoom(const Rooms3dCubePos& p);
    Room* findRoom(const tstring& hash) const;
    void  optimizeSize();
    bool  setAsLevel0(int level);
private:
    void clearExits(Room *r);
private:
    Rooms3dContainer rooms;
    int z_id;
    tstring m_name;
    std::map<tstring, Room*> m_hashmap;
    typedef std::map<tstring, Room*>::iterator hashmap_iterator;
    typedef std::map<tstring, Room*>::const_iterator hashmap_const_iterator;
};

typedef std::vector<Rooms3dCube*> Rooms3dCubeList;

class RoomHelper
{
    const Room* room;
public:
    RoomHelper(const Room *r) : room(r) {}
    bool isExplored(RoomDir dir) {
      Room *next = room->dirs[dir].next_room;
      if (next && room->pos.zid == next->pos.zid)
         return true;
      return false;
    }
    bool isZoneExit(RoomDir dir) {
      Room *next = room->dirs[dir].next_room;
      if (next && room->pos.zid != next->pos.zid)
          return true;
      return false;
    }
};
