#pragma once
#include "roomObjects.h"
#include "mapInstance.h"

/*class RoomFreePlaceTool {
public:
    RoomFreePlaceTool(Rooms3dCube *z);
    bool tryFreePlace(const Room* room, RoomDir dir);
private:
     Rooms3dCube *zone;
};*/

class RoomWaveAlgoritm {
public:
    bool runWaveAlgoritm(const Rooms3dCube *zone, const Room* room, RoomDir dir);
    void getNewZoneRooms(std::vector<const Room*>* rooms);
private:
    bool run(const Rooms3dCube *zone, const Rooms3dCubePos& start, RoomDir dir);
    void deleteRoom(const Room* room);    
    std::unordered_map<const Room*, int> nodes;
    typedef std::pair<const Room*, const Room*> branch;
    std::vector<branch> branches;
    typedef std::vector<branch>::const_iterator branches_const_iterator;
    typedef std::vector<branch>::iterator branches_iterator;
    typedef std::unordered_map<const Room*, int>::const_iterator const_iterator;
    typedef std::unordered_map<const Room*, int>::iterator iterator;
    int index(const Room* r) const;
    bool exist(const Room* r) const;
};
