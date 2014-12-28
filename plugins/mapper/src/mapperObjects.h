#pragma once

#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_NORTH=0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
extern const utf8* RoomDirName[];
struct Room;
class RoomsLevel;
class Zone;

struct RoomData
{
    tstring name;
    tstring descr;
    tstring exits;
    uint    hash;
    uint    dhash;

    bool equal(const RoomData& rd) const
    {
        assert(hash && rd.hash);
        return (hash && dhash && hash == rd.hash && dhash == rd.dhash) ? true : false;
    }

    bool similar(const RoomData& rd) const
    {
        assert(hash && rd.hash);
        return (hash && hash == rd.hash) ? true : false;
    }

    bool compare(const RoomData& rd) const
    {
        if (name != rd.name)
            return false;
        if (exits != rd.exits)
            return false;
        if (descr != rd.descr)
            return false;        
        return true;
    }
    
    void calcHash()
    {
        CRC32 crc;
        crc.process(name.c_str(), name.length() * sizeof(WCHAR));        
        crc.process(exits.c_str(), exits.length() * sizeof(WCHAR));
        hash = crc.getCRC32();
        dhash = 0;
        if (!descr.empty()) 
        {
            CRC32 dcrc;
            dcrc.process(descr.c_str(), descr.length() * sizeof(WCHAR));
            dhash = dcrc.getCRC32();
        }
    }

    RoomData() : hash(0), dhash(0) {}

#ifdef _DEBUG
    void printDebugData()
    {
        WCHAR buf[64];
        swprintf(buf, L"-----\r\nhash: 0x%x, dhash: 0x%x\r\n", hash, dhash);
        OutputDebugString(buf);
        OutputDebugString(name.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(descr.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(exits.c_str());
        OutputDebugString(L"\r\n");
    }
#endif
};

struct RoomExit
{
    RoomExit() : next_room(NULL), exist(false), door(false) {}
    Room *next_room;
    bool exist;
    bool door;
};

struct Room
{
    Room() : level(NULL), x(0), y(0), icon(0), use_color(0), color(0), special(0) {}
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    RoomsLevel *level;              // callback ptr to parent level (owner of room)
    int x, y;                       // position in level
    int icon;                       // icon if exist
    int use_color;                  // flag for use background color
    COLORREF color;                 // background color
    int special;                    //todo special internal value for algoritms (don't saved)
};

class RoomsLevel
{
public:
    RoomsLevel(int width, int height, int level, Zone *parent_zone);
    ~RoomsLevel();
    Zone* getZone() const { return m_pZone; }
    int   getLevel() const { return m_level; }
    bool  set(int x, int y, Room* room);
    Room* get(int x, int y) const;
    int   getWidth() const;
    int   getHeight() const;

private:  
    friend class Zone;
    bool check(int x, int y) const;
    void extend(RoomDir d, int count);
    void setsize(int dx, int dy);
    struct row 
    {   ~row()
        { struct{ void operator() (Room* r) { delete r; }} del;
          std::for_each(rr.begin(), rr.end(), del);
        }
        std::deque<Room*> rr;
    };
    std::deque<row*> rooms;
    Zone* m_pZone;
    int m_level;
};

/*struct RoomCursor
{
    Room* room;
    int x, y, level;
public:
    RoomCursor(Room *r);
    Room* move(int dir);
private:
    RoomsLevel* getOffsetLevel(int offset);
    void updateCoords();
};*/

class Zone
{
public:
    Zone(const tstring& zonename) : 
        m_changed(false), m_name(zonename), width(0), height(0), minlevel(-1), maxlevel(-1)  { }
    ~Zone()
    {
      lvls_iterator it = m_levels.begin(), it_end = m_levels.end();
      for (; it != it_end; ++it)
        delete it->second;
    }
    void  setName(const tstring& newname) { m_name = newname; m_changed = true; }
    const tstring& getName() const { return m_name; }
    int   getWidth() const { return width; }
    int   getHeight() const { return height; }
    bool  isChanged() const { return m_changed; }
    bool  isEmpty() const { return (minlevel == -1 && maxlevel == -1) ? true : false; }

    bool  addRoom(int x, int y, int level, Room* room);

    //Room* getRoom(int x, int y, int level);
    RoomsLevel* getLevel(int level);

    //RoomsLevel* getLevelAlways(int level);
    RoomsLevel* getDefaultLevel();
   
    //void resizeLevels(int x, int y);

private:
    RoomsLevel* getl(int level, bool create_if_notexist);
    void extend(RoomDir d, int count);

private:
    std::map<int, RoomsLevel*> m_levels;
    typedef std::map<int, RoomsLevel*>::iterator lvls_iterator;
    bool m_changed;
    tstring m_name;
    int width;
    int height;
    int minlevel;
    int maxlevel;
};
