#pragma once
#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH = 0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
struct Room;
class RoomsLevel;
class Zone;

struct RoomData
{
    tstring vnum;
    tstring zonename;   // if known ?
    tstring name;
    tstring descr;
    tstring exits;
};

struct RoomExit
{
    RoomExit() : next_room(NULL), exist(false), door(false), multiexit(false) {}
    Room *next_room;
    bool exist;
    bool door;
    bool multiexit;
};

struct Room
{
    Room() : level(NULL), x(0), y(0), icon(0), use_color(0), color(0) {}
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    RoomsLevel *level;              // parent level (owner of room)
    int x, y;                       // position in the level
    int icon;                       // icon if exist
    int use_color;                  // flag for use background color
    COLORREF color;                 // background color
};

class RoomHelper
{
    Room* r;
    int x, y, z;
public:
    RoomHelper(Room *room);
    Zone* zone();
    RoomsLevel* level();
    Room* getRoomDir(RoomDir dir);
    //bool  isExplored(RoomDir dir);
    bool  addRoom(RoomDir dir, Room* room);
    bool  addLink(RoomDir dir, Room *room);
private:
    bool move(RoomDir dir);
    Zone* zone(Room *room);
};

class RoomDirHelper
{
public:
    RoomDir revertDir(RoomDir dir);
    const wchar_t* getDirName(RoomDir dir);
    RoomDir getDirByName(const wchar_t* dirname);
};

/*class RoomCursor
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
*/

/*struct RoomsLevelBox
{
    RoomsLevelBox() : left(0), right(0), top(0), bottom(0) {}
    int left;
    int right;
    int top;
    int bottom;
};*/

