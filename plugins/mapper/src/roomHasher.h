#pragma once

#include "roomsZone.h"

class Rooms3dCubeHash {
public:
    Rooms3dCubeHash(const Rooms3dCube* c);
    const tstring& getHash() const { return m_hash; }
private:
    tstring m_hash;
};
