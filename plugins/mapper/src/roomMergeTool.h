#pragma once
#include "roomObjects.h"
#include "mapInstance.h"

class MapWaveArrayPos
{
public:
    MapWaveArrayPos(Rooms3dCubePos& p) : x(p.x), y(p.y), z(p.z) {}
    bool valid(const Rooms3dCubeSize& sz) const {
        return (z >= sz.minlevel && z <= sz.maxlevel && 
        x >= sz.left && x <= sz.right && y >= sz.top && y <= sz.bottom) ? true : false;
    }
    int x,y,z;
};

class MapWaveArray 
{
public:
    MapWaveArray(const Rooms3dCubeSize& size) {
        size_t total = size.width() * size.height() * size.levels();
        wave.resize(total, 0);
        sz = size;
    }
    void set(const MapWaveArrayPos&p, int v) {
        size_t index = 0;
        if (getindex(p, &index)) 
            wave[index] = v;
    }
    int get(const MapWaveArrayPos&p) const {
        size_t index = 0;
        return (getindex(p, &index)) ? wave[index] : -1;
    }
private:
    bool getindex(const MapWaveArrayPos&p, size_t *index) const {
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
};

class MapWave {
public:
    MapWave(Rooms3dCube *z);
private:
    Rooms3dCube *zone;
    std::shared_ptr<MapWaveArray> wave;
};

class MapWaveAlgorithms {
public:
    MapWaveAlgorithms()
    bool wave(MapWaveArray &wa, MapWaveArrayPos& start, RoomDir dir);
};

class RoomMergeTool
{
public:
    RoomMergeTool(MapInstance *map, const Room* tracked);
    bool makeNewZone(RoomDir dir);
private:
    Rooms3dCube *zone;
    Room *start_room;
};
