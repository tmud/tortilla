#pragma once

#define ROOM_DIRS_COUNT 6
enum RoomDir { RD_UNKNOWN = -1, RD_NORTH=0, RD_SOUTH, RD_WEST, RD_EAST, RD_UP, RD_DOWN };
extern const utf8* RoomDirName[];

struct RoomData
{
    RoomData() : hash(0), dark(false) {}

    tstring name;
    tstring zonename;
    tstring roomid;
    tstring descr;
    tstring exits;
    uint    hash;
    bool    dark;

    /*bool samehash(const RoomData& rd) const
    {
        assert(hash && rd.hash);
        return(hash && hash == rd.hash) ? true : false;
    }

    bool equal(const RoomData& rd) const
    {
        return (roomid == rd.roomid && zonename == rd.zonename) ? true : false;
    }

    bool checkparams(const RoomData& rd) const
    {
        if (name != rd.name)
            return false;
        if (exits != rd.exits)
            return false;
        if (descr != rd.descr)
            return false;        
        return true;
    }*/
    
    void calcHash()
    {
        CRC32 crc;
        crc.process(zonename.c_str(), zonename.length() * sizeof(WCHAR));        
        crc.process(roomid.c_str(), roomid.length() * sizeof(WCHAR));
        hash = crc.getCRC32();
    }

    void printDebugData()
    {
#ifdef _DEBUG
        WCHAR buf[64];
        swprintf(buf, L"hash: %x,", hash);
        OutputDebugString(buf);
        OutputDebugString(name.c_str());
        if (dark)
            OutputDebugString(L",dark");
        OutputDebugString(L"\r\n");
        OutputDebugString(zonename.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(roomid.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(descr.c_str());
        OutputDebugString(L"\r\n");
        OutputDebugString(exits.c_str());
        OutputDebugString(L"\r\n---------------------------\r\n");
#endif
    }
};

struct Room;
struct RoomExit
{
    RoomExit() : exist(false), door(false) {}
    bool isNext() const { return !next_rooms.empty(); }
    void setNext(Room* r)
    {
        std::vector<Room*>& nr = next_rooms;
        if (std::find(nr.begin(), nr.end(), r) != nr.end())
            return;
        nr.push_back(r);
    }
    void delNext(Room* r)
    {
        std::vector<Room*>& nr = next_rooms;
        iterator it = std::find(nr.begin(), nr.end(), r);
        if (it != nr.end())
            nr.erase(it);
    }
    std::vector<Room*> next_rooms;
    typedef std::vector<Room*>::iterator iterator;
    bool exist;
    bool door;
};

struct TableCell;
struct Room
{
    Room() : cell(NULL), icon(0), use_color(0), color(0), special(0) {}
    TableCell *cell;                // position in world
    RoomData roomdata;              // room key data
    RoomExit dirs[ROOM_DIRS_COUNT]; // room exits
    int icon;                       // icon if exist
    int use_color;                  // flag for use background color
    COLORREF color;                 // background color
    int special;                    // special internal temp value for algorithms
};

struct TableIndex
{
    int level, area, zone;
};

struct TableCell
{
    TableCell() : x(0), y(0), index(NULL), room(NULL) {}
    ~TableCell() { delete room; }
    int x, y;
    TableIndex *index;
    Room  *room;
};

struct TablePos
{
    TablePos() : x(-1), y(-1), level(0), area(0), zone(0) {}   
    int x, y, level, area, zone;
    bool valid() const { return (x >= 0 && y >= 0) ? true : false; }
    void init(Room *r) { 
        assert(r);
        TableCell *c = r->cell;
        x = c->x; y = c->y;
        level = c->index->level;
        area = c->index->area;
        zone = c->index->zone;
    }
};

struct TableLimits
{
    TableLimits() : width(0), height(0), minlevel(0), maxlevel(0), areas(0) {}
    int width, height, minlevel, maxlevel, areas;
};

#include "mapperHashTable.h"
class Table
{
public:
    bool  getLimits(const TablePos& p, TableLimits* limits);
    Room* get(const TablePos& p);
    Room* addNewRoom(Room *current_room, const RoomData& rd, RoomDir dir);
    
public:
    Table() {}
    ~Table() {
        std::for_each(m_zones.begin(), m_zones.end(), [](zone* obj){ delete obj; });
        std::for_each(m_indexes.begin(), m_indexes.end(), [](TableIndex* obj){ delete obj; });
    }
private:
    void set(const TablePos& p, Room *r);
    void createNewPlace(const tstring& zone_name, TablePos *pos);
    Room* createRoom(const RoomData& room);
    int getZone(const tstring& name);
    int getIndex(const TablePos& p);
    void createPlace(TablePos& p, RoomDir dir);

private:
    struct line {
        ~line() { std::for_each(elements.begin(), elements.end(), [](TableCell* obj){ delete obj; }); }
        std::deque<TableCell*> elements;
    };
    struct level {
        ~level() { std::for_each(lines.begin(), lines.end(), [](line* obj){ delete obj; }); }
        std::deque<line*> lines;
    };
    struct area {
        ~area() { std::map<int, level*>::iterator it = levels.begin(), it_end= levels.end();
        for (; it != it_end; ++it) { delete it->second; }
        }
        std::map<int, level*> levels;
    };
    struct zone {
        ~zone() { std::for_each(areas.begin(), areas.end(), [](area* obj){ delete obj; }); }
        std::deque<area*> areas;
        tstring name;
    };
    std::vector<TableIndex*> m_indexes;
    std::vector<zone*> m_zones;
    //std::map<uint, Room*> m_hash_table;

    MapperHashTable m_hash_table;
};
