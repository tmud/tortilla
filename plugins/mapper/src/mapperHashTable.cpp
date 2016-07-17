#include "stdafx.h"
#include "mapperHashTable.h"

/*MapperHashTable::MapperHashTable()
{
}

MapperHashTable::~MapperHashTable()
{
    std::for_each(rooms.begin(), rooms.end(),
        [](std::pair<const uint, rooms_list*>&it){ delete it.second; });
}

void MapperHashTable::addRoom(Room* room)
{
    if (!room) { assert(false); return; }
    const RoomData &d = room->roomdata;
    iterator it = rooms.find(d.hash);
    if (it == rooms.end())
    {
        rooms_list* rlist = new rooms_list;
        rlist->push_back(room);
        rooms[d.hash] = rlist;
    }
    else
    {
        rooms_list* rlist = it->second;
        rlist->push_back(room);
    }
}

void MapperHashTable::deleteRoom(Room* room)
{
    if (!room) { assert(false); return; }
    const RoomData &d = room->roomdata;
    iterator it = rooms.find(d.hash);
    if (it != rooms.end())
    {
        rooms_list* rlist = it->second;
        rooms_list_iterator rt = std::find(rlist->begin(), rlist->end(), room);
        if (rt != rlist->end())
           rlist->erase(rt);
    }
}

void MapperHashTable::findRooms(const RoomData& room, std::vector<Room*> *vr)
{
    assert(vr && vr->empty());
    iterator it = rooms.find(room.hash);
    if (it == rooms.end())
        return;
    if (!room.dhash)        // get all rooms
    {
        rooms_list* rlist = it->second;
        for (int i=0,e=rlist->size();i<e;++i){
            vr->push_back(rlist->at(i));
        }
        return;
    }
    // get only rooms with same dhash
    rooms_list* rlist = it->second;
    for (int i = 0, e = rlist->size(); i < e; ++i)
    {
        Room* r = rlist->at(i);
        if (r->roomdata.dhash == room.dhash)
            vr->push_back(r);
    }
}
*/
