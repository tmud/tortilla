#include "stdafx.h"
#include "mapTools.h"
#include "roomMergeTool.h"

MapToolsImpl::MapToolsImpl(std::vector<Rooms3dCube*>& szones, MapToolsApply* tool) : endTool(tool), changed(false), newZoneTool(NULL)
{    
    std::vector<Rooms3dCube*>::iterator it = szones.begin(), it_end = szones.end();
    for (; it!=it_end; ++it) {
        Rooms3dCube* zone = *it;
        int id = zone->id();
        assert(zones.find(id)==zones.end());
        zones[id] = zone;
    }   
}

MapToolsImpl::~MapToolsImpl()
{
    delete newZoneTool;
    if (!changed || !endTool) 
        return;
    std::vector<Rooms3dCube*> z;
    std::map<int, Rooms3dCube*>::iterator it = zones.begin(), it_end = zones.end();
    for (; it!=it_end; ++it)
        z.push_back(it->second);
    endTool->updateMap(z);
}

Room* MapToolsImpl::castRoom(const Room* room) const
{
    std::map<int, Rooms3dCube*>::const_iterator rt = zones.find(room->pos.zid);
    if (rt == zones.end()) {        
        return NULL;
    }
    Rooms3dCube* z = rt->second;
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
    std::map<int, Rooms3dCube*>::iterator it = zones.find(room->pos.zid);
    if (it == zones.end()) {
        assert(false);
        return false;    
    }
    delete newZoneTool;   
    newZoneTool = new RoomWaveAlgoritm();
    if (!newZoneTool->runWaveAlgoritm(it->second, room, dir)) {
        delete newZoneTool;
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
       rooms.push_back(const_cast<Room*>(r));
    }

    Rooms3dCube* new_zone = new Rooms3dCube(newZoneId(), zoneName);
    for (Room *r : rooms) 
    {
        Rooms3dCubePos p (r->pos);
        Rooms3dCube *z = getZone(r);
        if (z->detachRoom(p))
            new_zone->addRoom(p, r);
    }
    
    //zones.push_back(new_zone);
    




   /*  std::vector<Room*> rooms;
        for(const Room* pr : r) {
            rooms.push_back(const_cast<Room*>(pr));
        }
        if (m_map.migrateRoomsNewZone(dlg.getName(), rooms))
        {
            int zid = rooms[0]->pos.zid;
            Rooms3dCube *zone = m_map.getZone(rooms[0]);
            m_zones_control.addNewZone(zone);
        }*/

    /*
    
        if (rooms.empty()) {
        assert(false);
        return false;
    }
    //migrate rooms
    Rooms3dCube* new_zone = new Rooms3dCube(zones.size(), getNewZoneName(name));
    for (Room *r : rooms) {
        Rooms3dCubePos p (r->pos);
        Rooms3dCube *z = getZone(r);
        if (z->detachRoom(p))
            new_zone->addRoom(p, r);
    }
    zones.push_back(new_zone);
    return true;
    */
}

int MapToolsImpl::newZoneId() const 
{
    int rndid = 0;
    SYSTEMTIME st;
    while(true) {
        GetSystemTime(&st);
        rndid = (st.wMilliseconds+1) * (st.wSecond+1) * (st.wMinute+1);
        if (zones.find(rndid) == zones.end())
            break;
    }
    return rndid;
}
