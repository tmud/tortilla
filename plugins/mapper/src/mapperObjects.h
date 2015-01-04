#pragma once

#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH=0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
extern const utf8* RoomDirName[];
struct Room;
class RoomsLevel;
class Zone;

struct RoomData
{
    tstring name;
    tstring zonename;
    tstring key;
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

struct RoomsLevelBox
{
    int left;
    int right;
    int top;
    int bottom;
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
    int   getWidth() const { return rooms.empty() ? 0 : rooms[0]->rr.size(); }
    int   getHeight() const { return rooms.size(); }
    void  getBox(RoomsLevelBox *box) const
    {
        //todo
        box->left = 0; box->top = 0;
        box->right = getWidth(); box->bottom = getHeight();
    }
    bool check(int x, int y) const { return (y >= 0 && y < getHeight() && x >= 0 && x < getWidth()) ? true : false; }
private:
    friend class Zone;
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

class Zone
{
public:
    Zone(const tstring& zonename) : m_changed(true), m_name(zonename)
    {
        m_levels[0] = new RoomsLevel(1, 1, 0, this);
    }
    Zone(const tstring& zonename, int width, int height, int minlevel, int maxlevel) : m_changed(false), m_name(zonename)
    {
        assert(minlevel <= maxlevel);
        assert(width > 0 && height > 0);
        for (int i = minlevel; i <= maxlevel; ++i)
            m_levels[i] = new RoomsLevel(width, height, i, this);
    }
    ~Zone() 
    {
        lvls_iterator it = m_levels.begin(), it_end = m_levels.end();
        for (; it != it_end; ++it)
            delete it->second;
    }
    bool  isChanged() const { return m_changed; }
    void  setChanged(bool changed) { m_changed = changed; }
    bool  isEmpty() const 
    {
        clvls_iterator it = m_levels.begin(), it_end = m_levels.end();
        for (; it != it_end; ++it) {
            RoomsLevel *level = it->second;
            for (int y = 0, ye = level->getHeight(); y < ye; ++y){
            for (int x = 0, xe = level->getWidth(); x < xe; ++x){
                if (level->get(x, y)) return false;
            }}
        }
        return true;
    }
    void  setName(const tstring& newname) {
        if (m_name == newname) return;
        m_name = newname;
        setChanged(true);
    }
    const tstring& getName() const { return m_name; }
    int   getWidth() const { return getDefaultLevel()->getWidth(); }
    int   getHeight() const { return getDefaultLevel()->getHeight(); }
    RoomsLevel* getLevel(int level) const {
        clvls_iterator it = m_levels.find(level);
        return (it != m_levels.end()) ? it->second : NULL;
    }    
    RoomsLevel* getDefaultLevel() const { return getLevel(0); }
    int getMinLevel() const { return m_levels.cbegin()->first; }
    int getMaxLevel() const { return m_levels.crbegin()->first; }
    void extend(RoomDir d, int count);

    //bool  addRoom(int x, int y, int level, Room* room);
    //Room* getRoom(int x, int y, int level;
    //RoomsLevel* getLevelAlways(int level);
    //void resizeLevels(int x, int y);

private:
    //RoomsLevel* getl(int level, bool create_if_notexist);    

private:
    std::map<int, RoomsLevel*> m_levels;
    typedef std::map<int, RoomsLevel*>::iterator lvls_iterator;
    typedef std::map<int, RoomsLevel*>::const_iterator clvls_iterator;
    bool m_changed;
    tstring m_name;
};

class RoomCursor
{
//public:
    Room* room;
    RoomDir dir;
public:
    RoomCursor(Room *r, RoomDir d);
    RoomCursor(Room *r, int d);
    Room* next() const;
    void  setNext(Room* r);
    bool  isSameByRevert() const;
    bool  delDirByRevert() const;
    bool  isNeighbor(Room* r) const;
    static RoomDir revertDir(RoomDir dir);

private:
    //RoomsLevel* getOffsetLevel(int offset);
    //void updateCoords();
    
};
