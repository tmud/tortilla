#pragma once
#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_NORTH = 0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN, RD_UNKNOWN=-1 };
struct Room;
class RoomsLevel;

class RoomDirHelper
{
public:
    RoomDir revertDir(RoomDir dir);
    const wchar_t* getDirName(RoomDir dir);
    RoomDir getDirByName(const wchar_t* dirname);
};

class RoomCursor
{
public:
    RoomCursor(Room* current_room);
    bool  isValid(RoomDir dir);
    Room* getRoom(RoomDir dir);
    bool  addRoom(RoomDir dir, Room* room);
    bool  isExplored(RoomDir dir);
private:
    bool  move(RoomDir dir);
    Room* m_current_room;
    int x, y, level;
};
