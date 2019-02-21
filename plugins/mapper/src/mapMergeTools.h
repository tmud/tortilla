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

class RoomWaveAlgoritm2
{
public:
    bool runFromRoom(Room* room);
    bool runFromBranch(Room* room, int dir);
    void getRoomsInWave(std::vector<const Room*>* rooms);
private:
    bool run(int weight, int zoneid, std::vector<Room*>& cursors);
    std::vector<Room*> rooms;
    std::unordered_map<const Room*, int> nodes;
    typedef std::pair<const Room*, const Room*> branch;
    std::vector<branch> branches;
    typedef std::vector<branch>::const_iterator branches_const_iterator;
    typedef std::vector<branch>::iterator branches_iterator;
    typedef std::unordered_map<const Room*, int>::const_iterator const_iterator;
    typedef std::unordered_map<const Room*, int>::iterator iterator;
    bool exist(const Room* r) const;
    int index(const Room* r) const;
};
