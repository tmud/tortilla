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
    Room* getRoom(RoomDir dir);
    bool  addRoom(RoomDir dir, Room* room);
    bool  addLink(RoomDir dir, Room *room);
    bool  isExplored(RoomDir dir);
private:
    bool  move(RoomDir dir);
    Room* m_current_room;
    int x, y, level;
};

class Zone;
class RoomCursorNewZone
{
public:
    Zone* createNewZone(const tstring& name, Room* room);
};

class MapperDirCommand
{
    RoomDir dir; tstring main, rel;
    int main_size, rel_size;
public:
    MapperDirCommand(RoomDir d, const tstring& main_part, const tstring& rel_part) : dir(d), main(main_part), rel(rel_part)
    {
        main_size = main.size();
        rel_size = rel.size();
    }
    RoomDir check(const tstring& cmd) const;
};
typedef std::vector<MapperDirCommand> DirsVector;
