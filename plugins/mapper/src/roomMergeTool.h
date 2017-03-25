#pragma once
#include "roomObjects.h"

class RoomMergeTool
{
public:
    RoomMergeTool();
    void init(Rooms3dCube *map);
    bool makeNewZone(RoomDir dir);
private:
    Rooms3dCube *map;
};
