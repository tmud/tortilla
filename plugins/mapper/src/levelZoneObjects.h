#pragma  once
#include "roomObjects.h"

struct LevelZoneSize
{
    LevelZoneSize() : minlevel(0), maxlevel(0), left(0), right(0), top(0), bottom(0) {}
    int minlevel, maxlevel;
    int left, right, top, bottom;
};

class Zone;
class RoomsLevel
{
    friend class Zone;
public:
    RoomsLevel(Zone* parent_zone) : m_changed(false), m_parentZone(parent_zone) {
        assert(parent_zone);
        rooms.push_back(new row);
    }
    ~RoomsLevel() {
        std::for_each(rooms.begin(), rooms.end(), [](row *r){ delete r; });
    }
    Zone* getZone() const { return m_parentZone; }
    const LevelZoneSize& size() const { m_parentZone->size(); }

    Room* getRoom(int x, int y);
    bool  addRoom(Room* r, int x, int y);
    Room* detachRoom(int x, int y);
    void  deleteRoom(int x, int y);

    bool  isChanged() const { return m_changed; }
    //bool  isEmpty() const;

private:
    void resizeLevel(int x, int y);
    bool checkCoords(int x, int y);
    struct row {
        row() : left(-1), right(-1) {}
        ~row() { std::for_each(rr.begin(), rr.end(), [](Room* r) { delete r; }); }
        void recalc_leftright() {
            left = right = -1;
            for (int i = 0, e = rr.size(); i < e; ++i) {
                if (!rr[i]) continue;
                if (left == -1) left = i;
                right = i;
            }
        }
        std::vector<Room*> rr;
        int left, right;
    };
    std::vector<row*> rooms;
    bool m_changed;
    Zone *m_parentZone;
};

class Zone
{
public:
    Zone(const tstring& zonename) : m_name(zonename) {}
    ~Zone() { std::for_each(m_levels.begin(), m_levels.end(), [](RoomsLevel* rl) {delete rl; }); }
    const LevelZoneSize& size() const { return m_size; }
    const tstring& getName() const { return m_name; }

    RoomsLevel* getLevel(int level, bool create_if_notexist);
    RoomsLevel* getDefaultLevel();

    void resizeLevels(int x, int y);
    bool isChanged() const;
    void setName(const tstring& name);
private:
    tstring m_name;
    LevelZoneSize m_size;
    std::vector<RoomsLevel*> m_levels;
};
