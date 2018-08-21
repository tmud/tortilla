#pragma once
#include "roomObjects.h"

class Rooms3dContainer
{
    friend class Rooms3dContainerEx;
public:
    Rooms3dContainer(bool delrooms) : deleterooms(delrooms) 
    {
        row *r = new row(deleterooms);
        level *l = new level;
        l->rooms.push_back(r);
        zone.push_back(l);
    }
    const Rooms3dCubeSize getSize() const { return cube_size; }
    void  set(const Rooms3dCubePos& p, Room *room);
    Room* get(const Rooms3dCubePos& p) const;
    void  collapse();
#ifdef _DEBUG
    bool testInvariant();
#endif
private:
    Room** getp(const Rooms3dCubePos& p) const;
    void extends_height(const Rooms3dCubePos& p);
    void extends_width(const Rooms3dCubePos& p);
    void extends_levels(const Rooms3dCubePos& p);
    bool checkCoords(const Rooms3dCubePos& p) const;
    struct row {
      row(bool _deleterooms, int count=1) : deleterooms(_deleterooms) { rr.resize(count, NULL); }
      ~row() {
          if (!deleterooms) return;
          std::for_each(rr.begin(), rr.end(), [](Room* r) { delete r; });
      }
      std::vector<Room*> rr;
      bool deleterooms;
    };
    struct level {
      ~level() { std::for_each(rooms.begin(), rooms.end(), [](row* r) { delete r; }); }
      std::vector<row*> rooms;
    };
    bool emptyLevel(level *l);
    std::vector<level*> zone;
    Rooms3dCubeSize cube_size;
    bool deleterooms;
};

class Rooms3dContainerEx 
{
public:
    Rooms3dContainerEx() : rooms(true) {}
    const Rooms3dCubeSize getSize() const { return rooms.getSize(); }
    void add(const Rooms3dCubePos& p, Room *room, int zoneid) 
    {
        rooms.set(p, room);
        room->pos = p;
        room->pos.zid = zoneid;
    }
    const Room* get(const Rooms3dCubePos& p) const 
    {
        return rooms.get(p); 
    }
    Room* detach(const Rooms3dCubePos& p) const {
        Room *room = rooms.get(p);
        room->pos.clear();
        return room;
    }
    void collapse();
    bool setAsLevel0(int level);
private:
    typedef Rooms3dContainer::level level;
    typedef Rooms3dContainer::row row;
    void correctPositions();
    void normalizePositions();
    int  countRooms(level *l);
    Rooms3dContainer rooms;
};
