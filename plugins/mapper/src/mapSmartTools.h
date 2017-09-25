#pragma once
#include "roomObjects.h"

class MapSmartTools 
{
public: 
    bool  isMultiExit(Room* from, RoomDir dir);
    bool  setMultiExit(Room* from, RoomDir dir);    
    bool  addLink(Room* from, Room* to, RoomDir dir);
    Room* getRoom(Room* from, RoomDir dir);
};
