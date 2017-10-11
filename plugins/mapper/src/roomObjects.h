#pragma once
#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH = 0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
const int beginRoomDir = RD_NORTH;
const int endRoomDir = RD_DOWN;
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
    Room() : icon(0), use_color(0), color(0), debugcolor(0) {}
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    Rooms3dCubePos pos;             // relative position in the level x,y,level,zoneid. all >= 0
    mutable int icon;               // icon if exist
    mutable int use_color;          // flag for use background color
    mutable COLORREF color;         // background color
#ifdef _DEBUG
    mutable COLORREF debugcolor;    // only for debug mode
#endif
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

class Rooms3dCube
{
    friend class Rooms3dCubeCursor;
    friend class MapToolsImpl;
public:
    Rooms3dCube(int zid, const tstring& name) : z_id(zid), m_name(name) {
        assert(zid >= 0);
        row *r = new row;
        level *l = new level;
        l->rooms.push_back(r);
        zone.push_back(l);
    }
    ~Rooms3dCube() {  release();  }
    const Rooms3dCubeSize& size() const {  return cube_size; }
    const tstring& name() const { return m_name; }
    int id() const { return z_id; }
    enum AR_STATUS { AR_OK = 0, AR_INVALIDROOM, AR_BUSY, AR_FAIL };
    AR_STATUS addRoom(const Rooms3dCubePos& p, Room* r);
    const Room* getRoom(const Rooms3dCubePos& p) const;
    void  deleteRoom(const Rooms3dCubePos& p);
    Room* detachRoom(const Rooms3dCubePos& p);
    Room* findRoom(const tstring& hash) const;
    void  optimizeSize();
    bool  setByZeroLevel(int level);
#ifdef _DEBUG
    bool testInvariant();
#endif
private:
    void clearExits(Room *r);
    Room*  get(const Rooms3dCubePos& p) const;
    Room** getp(const Rooms3dCubePos& p) const;
    bool extends(const Rooms3dCubePos& p);
    void extends_height(const Rooms3dCubePos& p);
    void extends_width(const Rooms3dCubePos& p);
    void extends_levels(const Rooms3dCubePos& p);
    bool checkCoords(const Rooms3dCubePos& p) const;    
    void collapse();
    void correctPositions();
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
    bool emptyLevel(level *l);
    void release() { std::for_each(zone.begin(), zone.end(), [](level* l) { delete l; }); }
    std::vector<level*> zone;
    Rooms3dCubeSize cube_size;
    int z_id;
    tstring m_name;
    std::map<tstring, Room*> m_hashmap;
    typedef std::map<tstring, Room*>::iterator hashmap_iterator;
    typedef std::map<tstring, Room*>::const_iterator hashmap_const_iterator;
};

typedef std::vector<Rooms3dCube*> Rooms3dCubeList;

class RoomHelper
{
    const Room* room;
public:
    RoomHelper(const Room *r) : room(r) {}
    bool isExplored(RoomDir dir) {
      Room *next = room->dirs[dir].next_room;
      if (next && room->pos.zid == next->pos.zid)
         return true;
      return false;
    }
    bool isZoneExit(RoomDir dir) {
      Room *next = room->dirs[dir].next_room;
      if (next && room->pos.zid != next->pos.zid)
          return true;
      return false;
    }    
};
