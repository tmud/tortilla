#pragma once
#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH = 0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
struct Room;

struct RoomData
{
    tstring vnum;
    //tstring zonename;   // if known ?
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
    Room() : x(0), y(0), z(0), zid(-1), icon(0), use_color(0), color(0) {}
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    int x, y;                       // relative position in the level (x,y >= 0)
    int z, zid;                     // relative position level(z-coord >=0) and zone id(zid-coord >= 0)
    int icon;                       // icon if exist
    int use_color;                  // flag for use background color
    COLORREF color;                 // background color
};

class RoomDirHelper
{
public:
    RoomDir cast(int index);
    RoomDir revertDir(RoomDir dir);
    const wchar_t* getDirName(RoomDir dir);
    RoomDir getDirByName(const wchar_t* dirname);
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
    Rooms3dCubePos(): x(0),y(0),z(0),zid(-1) {}
    int x, y, z, zid;
};

class Rooms3dCube
{
public:
    Rooms3dCube(int zid) : z_id(zid) {
        assert(zid >= 0);
        row *r = new row;
        level *l = new level;
        l->rooms.push_back(r);
        zone.push_back(l);
    }
    ~Rooms3dCube() {  release();  }
    const Rooms3dCubeSize& size() const {  return cube_size; }

    enum AR_STATUS { AR_OK = 0, AR_INVALIDROOM, AR_BUSY, AR_FAIL };
    AR_STATUS addRoom(const Rooms3dCubePos& p, Room* r);
    const Room* getRoom(const Rooms3dCubePos& p) const;
    void  deleteRoom(const Rooms3dCubePos& p);
    Room* detachRoom(const Rooms3dCubePos& p);
private:
    void clearExits(Room *r);
    Room*  get(const Rooms3dCubePos& p) const;
    Room** getp(const Rooms3dCubePos& p) const;
    bool extends(const Rooms3dCubePos& p);
    void extends_height(const Rooms3dCubePos& p);
    void extends_width(const Rooms3dCubePos& p);
    void extends_levels(const Rooms3dCubePos& p);
    bool checkCoods(const Rooms3dCubePos& p) const;
    void collapse(const Rooms3dCubePos& p);
private:
    struct row {
      row(int count=1) { rr.resize(count, NULL); }
      ~row() { std::for_each(rr.begin(), rr.end(), [](Room* r) { delete r; }); }
      std::vector<Room*> rr;
    };
    struct level {
      ~level() { std::for_each(rooms.begin(), rooms.end(), [](row* r) { delete r; }); }
      std::vector<row*> rooms;
    };
    void release() { std::for_each(zone.begin(), zone.end(), [](level* l) { delete l; }); }
    std::vector<level*> zone;
    Rooms3dCubeSize cube_size;
    int z_id;
};


/*class RoomHelper
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
};*/

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

