#include "stdafx.h"
#include "roomsZone.h"
#include "mapMergeTools.h"

MapZoneMirror::MapZoneMirror(const Rooms3dCube* src) 
{
}    

MapZoneMirror::~MapZoneMirror() 
{
    release();
}

bool MapZoneMirror::tryMergeZone(const Rooms3dCube* z)
{
    return false;
}