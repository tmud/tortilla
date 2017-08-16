#include "stdafx.h"
#include "mapTools.h"
#include "roomMergeTool.h"

MapToolsImpl::MapToolsImpl(std::vector<Rooms3dCube*>& szones) : zones(szones), changed(false), newZoneTool(NULL)
{   
    zones.assign(szones.begin(), szones.end());
}

MapToolsImpl::~MapToolsImpl()
{
    delete newZoneTool;
    /*if (!changed || !endTool) 
        return;
    std::vector<Rooms3dCube*> z;
    std::map<int, Rooms3dCube*>::iterator it = zones.begin(), it_end = zones.end();
    for (; it!=it_end; ++it)
        z.push_back(it->second);
    endTool->updateMap(z);*/
}

Room* MapToolsImpl::castRoom(const Room* room) const
{
    /*std::map<int, Rooms3dCube*>::const_iterator rt = zones.find(room->pos.zid);
    if (rt == zones.end()) {        
        return NULL;
    }*/
    Rooms3dCube* z = findZone(room->pos.zid);
    Room* srcroom = z->get(room->pos);
    assert(srcroom);
    if (srcroom) {
        if (srcroom->roomdata.vnum != room->roomdata.vnum)
            return NULL;    
    }
    return srcroom;
}

bool MapToolsImpl::tryMakeNewZone(const Room* room, RoomDir dir)
{
    Room *r = castRoom(room);
    if (!r || dir == RD_UNKNOWN) {
        assert(false);
        return false;
    }
    Rooms3dCube* z = findZone(room->pos.zid);
    if (!z) {
        assert(false);
        return false;    
    }
    delete newZoneTool;
    newZoneTool = new RoomWaveAlgoritm();
    if (!newZoneTool->runWaveAlgoritm(z, room, dir)) {
        delete newZoneTool;
        newZoneTool = NULL;
        return false;
    }
    return true;
}

void MapToolsImpl::applyMakeNewZone(const tstring& zoneName)
{
    if (!newZoneTool)
        return;
    std::vector<const Room*> constr;
    newZoneTool->getNewZoneRooms(&constr);
    delete newZoneTool;
    newZoneTool = NULL;
    if (constr.empty())
        return;
    std::vector<Room*> rooms;
    for(const Room* r : constr) {
       Rooms3dCube *z = findZone(r->pos.zid);
       if (!z) {
           assert(false);
           return;
       }
       rooms.push_back(const_cast<Room*>(r));
    }
    Rooms3dCube* new_zone = new Rooms3dCube(newZoneId(), zoneName);
    for (Room *r : rooms) 
    {
        Rooms3dCube *z = findZone(r->pos.zid);
        Rooms3dCubePos p (r->pos);
        if (z->detachRoom(p))
            new_zone->addRoom(p, r);
    }
    zones.push_back(new_zone);
}

int MapToolsImpl::newZoneId() const 
{
    int rndid = 0;
    SYSTEMTIME st;
    while(true) {
        GetSystemTime(&st);
        rndid = (st.wMilliseconds+1) * (st.wSecond+1) * (st.wMinute+1);
        if (!findZone(rndid))
            break;
    }
    return rndid;
}

Rooms3dCube* MapToolsImpl::findZone(int zid) const
{
    std::vector<Rooms3dCube*>::const_iterator it = std::find_if(zones.begin(), zones.end(), [&](Rooms3dCube* z){ return (z->id() == zid); } );
    return (it != zones.end()) ? *it : NULL;
}
