#include "stdafx.h"
#include "roomsZone.h"
#include "mapMergeTools.h"

MapZoneMirror::MapZoneMirror(const Rooms3dContainerEx& src) : rooms(src)
{
}    

MapZoneMirror::~MapZoneMirror() 
{
}

bool MapZoneMirror::tryMergeZone()
{

    return false;
}