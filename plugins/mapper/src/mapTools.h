#pragma once
#include "roomObjects.h"

class RoomWaveAlgoritm;
class MapToolsImpl
{
public:
    MapToolsImpl(std::vector<Rooms3dCube*>& szones);
    ~MapToolsImpl();
    bool tryMakeNewZone(const Room* room, RoomDir dir);
	Rooms3dCube* applyMakeNewZone(const tstring& zoneName);
private:
    Room *castRoom(const Room* room) const;
    int newZoneId() const;
    Rooms3dCube* findZone(int zid) const;
    std::vector<Rooms3dCube*> zones;
    bool changed;
    RoomWaveAlgoritm *newZoneTool;
};

typedef std::shared_ptr<MapToolsImpl> MapTools;