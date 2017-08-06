#pragma once
#include "roomObjects.h"

class RoomWaveAlgoritm;
class MapToolsApply {
public:
    virtual void updateMap(std::vector<Rooms3dCube*>& zones) = 0;
};

class MapToolsImpl
{
public:
    MapToolsImpl(std::vector<Rooms3dCube*>& szones, MapToolsApply* tool);
    ~MapToolsImpl();
    bool tryMakeNewZone(const Room* room, RoomDir dir);
    void applyMakeNewZone(const tstring& zoneName);
private:
    Room *castRoom(const Room* room) const;
    int newZoneId() const;
    std::map<int, Rooms3dCube*> zones;
    MapToolsApply* endTool;
    bool changed;
    RoomWaveAlgoritm *newZoneTool;
};

typedef std::shared_ptr<MapToolsImpl> MapTools;