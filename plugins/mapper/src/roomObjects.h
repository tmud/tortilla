#pragma once
#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH = 0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
const int beginRoomDir = RD_NORTH;
const int endRoomDir = RD_DOWN;
struct Room;

struct RoomData
{
    tstring vnum;
    tstring zonename;
    tstring name;
    //tstring descr;
    //tstring exits;
    std::map<tstring, tstring> exits; // dir -> vnum
};

struct RoomExit
{
    RoomExit() : next_room(NULL), exist(false), door(false), multiexit(false) {}
    Room *next_room;
    bool exist;
    bool door;
    bool multiexit;
};

struct Rooms3dCubeSize
{
    Rooms3dCubeSize() : minlevel(0), maxlevel(0), left(0), right(0), top(0), bottom(0) {}
    int minlevel, maxlevel;
    int left, right, top, bottom;
    int width() const { return right-left+1; }
    int height() const { return bottom-top+1; }
    int levels() const { return maxlevel-minlevel+1; }
};

struct Rooms3dCubePos 
{
    Rooms3dCubePos() { clear(); }
    void clear() { x = y = z = 0; zid = -1; }
    bool move(RoomDir dir) 
    {
        switch (dir)
        {
        case RD_NORTH: y -= 1; break;
        case RD_SOUTH: y += 1; break;
        case RD_WEST:  x -= 1; break;
        case RD_EAST:  x += 1; break;
        case RD_UP:    z += 1; break;
        case RD_DOWN:  z -= 1; break;
        default:
            assert(false);
            return false;
        }
        return true;
    }
    bool valid(const Rooms3dCubeSize& sz) const {
        return (z >= sz.minlevel && z <= sz.maxlevel && 
        x >= sz.left && x <= sz.right && y >= sz.top && y <= sz.bottom) ? true : false;
    }
    int x, y, z, zid;
};

struct Room
{
    Room() : icon(0), use_color(0), color(0), selected(false) 
    {
    }
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    Rooms3dCubePos pos;             // relative position in the level x,y,level,zoneid. all >= 0
    mutable int icon;               // icon if exist
    mutable int use_color;          // flag for use background color
    mutable COLORREF color;         // background color
    mutable bool selected;          // to render selection flag
    const tstring hash() const { return roomdata.vnum; }
};

class RoomDirHelper
{
public:
    RoomDir cast(int index);
    int     index(RoomDir dir);
    RoomDir revertDir(RoomDir dir);
    const wchar_t* getDirName(RoomDir dir);
    RoomDir getDirByName(const wchar_t* dirname);
};

