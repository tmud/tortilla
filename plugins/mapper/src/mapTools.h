#pragma once
#include "roomObjects.h"

class MapToolsImpl
{
public:
    MapToolsImpl(const Room* start, std::vector<Rooms3dCube*>& szones);

private:
    Room *srcroom;
    std::map<int, Rooms3dCube*> zones;

};

typedef std::shared_ptr<MapToolsImpl> MapTools;