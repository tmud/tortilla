#include "stdafx.h"
#include "roomMergeTool.h"

RoomMergeTool::RoomMergeTool()
{
}

void RoomMergeTool::init(Rooms3dCube *pmap)
{
    assert(map);
    map = pmap;
    if (!map)
        return;
    Rooms3dCubeSize size = map->size();
    


}

bool RoomMergeTool::makeNewZone(RoomDir dir)
{
    if (!map || dir == RoomDir::RD_UNKNOWN)
    {
        assert(false);
        return false;
    }
    


    return false;
}
