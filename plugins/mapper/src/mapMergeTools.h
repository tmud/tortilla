#pragma once
#include "roomsContainer.h"

class MapZoneMirror {
public:
    MapZoneMirror(const Rooms3dContainerEx& src);
    ~MapZoneMirror();
    bool tryMergeZone(const Rooms3dCube* z);
private:
    Rooms3dContainerMirror rooms;

};
