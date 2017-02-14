#pragma  once

#include "roomObjects.h"

class Zone;
struct RoomsLevelParams {
    int left, right, top, bottom;
    int level;
    int width, height;
};

class RoomsLevel
{
    friend class Zone;
public:
    RoomsLevel(Zone* parent_zone, int floor) : m_invalidBox(true), m_changed(false), m_zone(NULL)
    {
        m_zone = parent_zone;
        m_params.level = floor,
            rooms.push_back(new row);
    }
    ~RoomsLevel() { std::for_each(rooms.begin(), rooms.end(), [](row *r){ delete r; }); }
    const RoomsLevelParams& params();
    Room* getRoom(int x, int y);
    bool  addRoom(Room* r, int x, int y);
    Room* detachRoom(int x, int y);
    void  deleteRoom(int x, int y); 
    Zone* getZone() const { return m_zone; }   

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
    //Zone* zone;
    //int level;
    //RoomsLevelBox m_box;
    bool m_invalidBox;
    bool m_changed;

    RoomsLevelParams m_params;
    Zone *m_zone;
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
    ~Zone() { std::for_each(m_levels.begin(), m_levels.end(), [](RoomsLevel* rl) {delete rl; }); }
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
