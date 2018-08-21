#pragma once
#include "roomsContainer.h"

class MapZoneMergeTool
{
public:
    bool tryMergeZone(const Rooms3dCube* z1, const Rooms3dCube* z2);
private:    
    void makeRoomsCopies(const Rooms3dCube* z);
    bool linkRoomsExits(const Rooms3dCube* z1, const Rooms3dCube* z2);
    void deleteCopies();
    void deleteCopies(int zid);
    Rooms3dContainerEx rooms;
    std::map<const Room*, Room *> i2c;      // map for source room to copy (in new zone)
    typedef std::map<const Room*, Room *>::iterator iterator;
};
