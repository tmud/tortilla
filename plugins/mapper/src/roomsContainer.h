#pragma once
#include "roomObjects.h"

class Rooms3dContainer
{
public:
    Rooms3dContainer(bool _deleterooms) : deleterooms(_deleterooms) {
        row *r = new row(deleterooms);
        level *l = new level;
        l->rooms.push_back(r);
        zone.push_back(l);
    }
    const Rooms3dCubeSize getSize() const { 
        return cube_size; }    
    void  add(const Rooms3dCubePos& p, Room *room, int zoneid);
    const Room* get(const Rooms3dCubePos& p) const;
    Room* detach(const Rooms3dCubePos& p) const;
    void collapse();
    bool setAsLevel0(int level);
#ifdef _DEBUG
    bool testInvariant();
#endif
private:
    Room** getp(const Rooms3dCubePos& p) const;
    void correctPositions();
    void normalizePositions();
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
    int  countRooms(level *l);
    void release() { std::for_each(zone.begin(), zone.end(), [](level* l) { delete l; }); }
    std::vector<level*> zone;
    Rooms3dCubeSize cube_size;
    bool deleterooms;
};
