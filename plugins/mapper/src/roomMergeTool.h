#pragma once
#include "roomObjects.h"
#include "mapInstance.h"

/*class MapWaveArray 
{
public:
    MapWaveArray(const Rooms3dCubeSize& size) {
        sz = size;
        size_t total = size.width() * size.height() * size.levels();
        wave.resize(total, 0);        
    }
    bool set(const Rooms3dCubePos&p, int v) {
        size_t index = 0;
        if (getindex(p, &index))
        {
            wave[index] = v;
            return true;
        }
        assert(false);
        return false;
    }
    int get(const Rooms3dCubePos&p) const {
        size_t index = 0;
        if (getindex(p, &index)) {
            return wave[index];
        }
        assert(false);
        return -1;
    }
private:
    bool getindex(const Rooms3dCubePos&p, size_t *index) const {
        assert(index);
        if (!p.valid(sz) )
            return false;
        int x = p.x - sz.left;
        int y = p.y - sz.top;
        int z = p.z - sz.minlevel;
        *index = (z * sz.width()*sz.height()) + (y*sz.width()) + x;
        return true;
    }
    std::vector<int> wave;
    Rooms3dCubeSize sz;
};*/

class RoomMergeTool {
public:
    RoomMergeTool(Rooms3dCube *z);
    bool tryMakeNewZone(const Room* room, RoomDir dir);
    void getNewZoneRooms(std::vector<const Room*>* rooms);
private:
    void clear();
    bool runWaveAlgoritm(const Rooms3dCubePos& start, RoomDir dir);
    void deleteRoom(const Room* room);
    Rooms3dCube *zone;
    std::unordered_map<const Room*, int> nodes;
    typedef std::pair<const Room*, const Room*> branch;
    std::vector<branch> branches;
    typedef std::vector<branch>::const_iterator branches_const_iterator;
    typedef std::vector<branch>::iterator branches_iterator;
    typedef std::unordered_map<const Room*, int>::const_iterator const_iterator;
    typedef std::unordered_map<const Room*, int>::iterator iterator;
    int index(const Room* r) const;
    bool exist(const Room* r) const;
};
