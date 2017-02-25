#pragma  once
#include "roomObjects.h"

/*
struct LevelZoneSize
{
    LevelZoneSize() : minlevel(0), maxlevel(0), left(0), right(0), top(0), bottom(0) {}
    int minlevel, maxlevel;
    int left, right, top, bottom;
    int width() const { return right-left+1; }
    int height() const { return bottom-top+1; }
    int levels() const { return maxlevel-minlevel+1; }
};

class RoomsLevel
{
    friend class Zone;
public:
    RoomsLevel(Zone* parent_zone) : m_changed(false), m_invalidBox(false), m_parentZone(parent_zone) {
        assert(parent_zone);
        rooms.push_back(new row);
    }
    ~RoomsLevel() {
        std::for_each(rooms.begin(), rooms.end(), [](row *r){ delete r; });
    }
public:
    Zone* getZone() const;
    const LevelZoneSize& size() const;
    Room* getRoom(int x, int y);
    bool  addRoom(Room* r, int x, int y);
    Room* detachRoom(int x, int y);
    void  deleteRoom(int x, int y);
    //bool  isEmpty() const;
private:
    bool isChanged() const { return m_changed; }
    void resizeLevel(int width, int height);
    bool convertCoords(int& x, int& y);
    struct row {
        row() {}
        ~row() { std::for_each(rr.begin(), rr.end(), [](Room* r) { delete r; }); }
        std::vector<Room*> rr;
    };
    std::vector<row*> rooms;
    bool m_changed;
    bool m_invalidBox;
    Zone *m_parentZone;
};

class Zone
{
public:
    Zone(const tstring& zonename) : m_name(zonename) {}
    ~Zone() { std::for_each(m_levels.begin(), m_levels.end(), [](RoomsLevel* rl) { delete rl; }); }
    const LevelZoneSize& size() const { return m_size; }
    void setName(const tstring& name) { m_name = name; }
    const tstring& getName() const { return m_name; }
    RoomsLevel* getLevel(int level, bool create_if_notexist);
    //RoomsLevel* getDefaultLevel();
    bool findLevel(const RoomsLevel *level, int *index) const;
    void resizeLevels(int x, int y);
    bool isChanged() const;
private:
    tstring m_name;
    LevelZoneSize m_size;
    std::vector<RoomsLevel*> m_levels;
};
*/
