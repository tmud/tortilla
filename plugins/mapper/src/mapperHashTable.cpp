#include "stdafx.h"
#include "mapperHashTable.h"

MapperHashTable::MapperHashTable()
{
}

MapperHashTable::~MapperHashTable()
{
    iterator it = rooms.begin(), it_end = rooms.end();
    for(; it!=it_end; ++it)
        it->second.destroy();
}

void MapperHashTable::addRoom(Room* room)
{
    if (!room) { assert(false); return; }
    RoomData &d = room->roomdata;
    iterator it = rooms.find(d.hash);
    if (it == rooms.end())
    {
        hash_element h;
        h.room = room;
        rooms[d.hash] = h;
    }
    else
    {
        hash_element &tmp = it->second;
        tmp.add(room);
    }
}

void MapperHashTable::deleteRoom(Room* room)
{
    if (!room) { assert(false); return; }
    RoomData &d = room->roomdata;
    iterator it = rooms.find(d.hash);
    if (it != rooms.end())
    {
        hash_element &tmp = it->second;
        if (tmp.del(room))
            rooms.erase(it);
    }
}

void MapperHashTable::findRooms(const RoomData& room, std::vector<Room*> *vr)
{
    assert(vr && vr->empty());
    iterator it = rooms.find(room.hash);
    if (it == rooms.end())
        return;
    // get all rooms
    hash_element &tmp = it->second;
    vr->push_back(tmp.room);
    hash_element *p = tmp.next;
    for (; p; p=p->next)     
       vr->push_back(p->room);
}
