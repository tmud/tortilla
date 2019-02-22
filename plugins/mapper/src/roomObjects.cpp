#include "stdafx.h"
#include "roomObjects.h"

RoomDir RoomDirHelper::cast(int index)
{
    if (index >= 0 && index < ROOM_DIRS_COUNT)
        return static_cast<RoomDir>(index);
    assert(false);
    return RD_UNKNOWN;
}

int RoomDirHelper::index(RoomDir dir)
{
    return static_cast<int>(dir);
}

const wchar_t* unknownDirName = L"unknown";
const wchar_t* RoomDirName[] = { L"north", L"south", L"west", L"east", L"up", L"down" };
RoomDir RoomDirHelper::revertDir(RoomDir dir)
{
    if (dir == RD_NORTH)
        return RD_SOUTH;
    if (dir == RD_SOUTH)
        return RD_NORTH;
    if (dir == RD_WEST)
        return RD_EAST;
    if (dir == RD_EAST)
        return RD_WEST;
    if (dir == RD_UP)
        return RD_DOWN;
    if (dir == RD_DOWN)
        return RD_UP;
    assert(false);
    return RD_UNKNOWN;
}
const wchar_t* RoomDirHelper::getDirName(RoomDir dir)
{
    int index = static_cast<int>(dir);
    if (index >= 0 && index <= 5)
        return RoomDirName[index];
    return unknownDirName;
}
RoomDir RoomDirHelper::getDirByName(const wchar_t* dirname)
{
    tstring name(dirname);
    for (int index = 0; index <= 5; ++index)
    {
        if (!name.compare(RoomDirName[index]))
            return static_cast<RoomDir>(index);
    }
    return RD_UNKNOWN;
}

RoomDir RoomDirHelper::getDirFromMsdp(const tstring& msdpdir)
{
    if (msdpdir == L"w" || msdpdir == L"W") {
        return RD_WEST;
    }
    if (msdpdir == L"e" || msdpdir == L"E") {
        return RD_EAST;
    }
    if (msdpdir == L"n" || msdpdir == L"N") {
        return RD_NORTH;
    }
    if (msdpdir == L"s" || msdpdir == L"S") {
        return RD_SOUTH;
    }
    if (msdpdir == L"u" || msdpdir == L"U") {
        return RD_UP;
    }
    if (msdpdir == L"d" || msdpdir == L"D") {
        return RD_UP;
    }
    assert(false);
    return RD_UNKNOWN;
}