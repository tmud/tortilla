#pragma once

class MapZoneMirror {
public:
    MapZoneMirror(const Rooms3dCube* src);
    ~MapZoneMirror();
    bool tryMergeZone(const Rooms3dCube* z);
private:
    struct row {
      row(int count=1) { rr.resize(count, NULL); }
      std::vector<Room*> rr;
    };
    struct level {
      ~level() { std::for_each(rooms.begin(), rooms.end(), [](row* r) { delete r; }); }
      std::vector<row*> rooms;
    };
    void release() { std::for_each(zone.begin(), zone.end(), [](level* l) { delete l; }); }
    std::vector<level*> zone;
};
