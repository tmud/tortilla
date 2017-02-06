#pragma once
#include "mapperObjects2.h"

struct RoomData
{
    tstring vnum;
    tstring zonename;   // if known
    tstring name;
    tstring descr;
    tstring exits;
};

struct Room;
struct RoomExit
{
    RoomExit() : next_room(NULL), exist(false), door(false) {}
    Room *next_room;
    bool exist;
    bool door;
};

class RoomsLevel;
struct Room
{
    Room() : level(NULL), x(0), y(0), icon(0), use_color(0), color(0) {} //, special(0) {}
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    RoomsLevel *level;              // parent level (owner of room)
    int x, y;                       // position in the level
    int icon;                       // icon if exist
    int use_color;                  // flag for use background color
    COLORREF color;                 // background color
    //int special;                    // special internal value for algoritms (not for save)
};

/*struct RoomsLevelBox
{
    RoomsLevelBox() : left(0), right(0), top(0), bottom(0) {}
    int left;
    int right;
    int top;
    int bottom;
};*/

class Zone;
struct RoomsLevelParams {
    int left,right,top,bottom;
    int level;
    int width, height;
    Zone *zone;
};

class RoomsLevel
{
    friend class Zone;
public:
    RoomsLevel(Zone* parent_zone, int floor) : m_invalidBoundingBox(true), m_changed(false)
    {
        m_params.zone = parent_zone;
        m_params.level = floor,
        rooms.push_back(new row);        
    }
    ~RoomsLevel() { std::for_each(rooms.begin(), rooms.end(), [](row *r){ delete r;}); }
    const RoomsLevelParams& params();
    bool  addRoom(Room* r, int x, int y);
    Room* detachRoom(int x, int y);
    void  deleteRoom(int x, int y);
    Room* getRoom(int x, int y);
    
    //Zone* getZone() const { return zone; }    
    //int   getLevel() const { return level; }
    //int   width() const;
    //int   height() const;
    //const RoomsLevelBox& box();

    bool  isChanged() const { return m_changed; }
    //bool  isEmpty() const;
private:
    void resizeLevel(int x, int y);
    bool checkCoords(int x, int y);
    struct row {
    row() : left(-1), right(-1) {}
    ~row() { std::for_each(rr.begin(), rr.end(), [](Room* r) { delete r;}); }    
    void recalc_leftright() {
      left = right = -1;
      for (int i = 0, e = rr.size(); i < e; ++i) {
        if (!rr[i]) continue;
        if (left == -1) left = i;
        right = i;
      }}
      std::vector<Room*> rr;
      int left, right;
    };
    std::vector<row*> rooms;
    //Zone* zone;
    //int level;
    //RoomsLevelBox m_box;
    bool m_invalidBox;
    bool m_changed;

    RoomsLevelParams m_params;
};

struct ZoneParams
{
    ZoneParams() : minlevel(0), maxlevel(0), width(0), height(0) {}
    tstring name;
    int minlevel, maxlevel;
    int width, height;
};

class Zone
{
public:
    Zone(const tstring& zonename) {
        m_params.name = zonename;
        m_params.minlevel = 0;
        m_params.maxlevel = 0;
    }
    ~Zone() { std::for_each(m_levels.begin(), m_levels.end(), [](RoomsLevel* rl) {delete rl;}); }
    const ZoneParams& params();

    RoomsLevel* getLevel(int level, bool create_if_notexist);
    RoomsLevel* getDefaultLevel();
    //int width() const;
    //int height() const;
    
    void resizeLevels(int x, int y);
    bool isChanged() const;
    void setName(const tstring& name);
private:
    std::vector<RoomsLevel*> m_levels;
    ZoneParams m_params;
};
