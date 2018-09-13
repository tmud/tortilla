#include "stdafx.h"
#include "mapSmartTools.h"

bool MapSmartTools::addLink(Room* from, Room* to, RoomDir dir)
{
    if (!from || !to || dir == RD_UNKNOWN) {
        assert(false); return false;
    }
    Room* current_next = from->dirs[dir].next_room;
    if (current_next && current_next != to) {
        assert(false); return false;
    }
    from->dirs[dir].next_room = to;
    return true;
}

bool MapSmartTools::isMultiExit(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
		return false;
    }
    return from->dirs[dir].multiexit;
}

bool MapSmartTools::setMultiExit(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
		return false;
    }
    from->dirs[dir].multiexit = true;
    from->dirs[dir].next_room = NULL;
    return true;
}

Room* MapSmartTools::getRoom(Room* from, RoomDir dir)
{
    if (!from || dir == RD_UNKNOWN) {
        assert(false);
        return NULL;
    }
    return from->dirs[dir].next_room;
}

