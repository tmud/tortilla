#pragma once 
#include "roomObjects.h"

class MapperDirCommand
{
    RoomDir dir;
    tstring main, rel;
    int main_size, rel_size;
public:
    MapperDirCommand(RoomDir d, const tstring& main_part, const tstring& rel_part) : dir(d), main(main_part), rel(rel_part)
    {
        main_size = main.size();
        rel_size = rel.size();
    }
    RoomDir check(const tstring& cmd) const;
};

typedef std::vector<MapperDirCommand> MapperDirsVector;
class InitDirVector {
    Pcre r1, r2;
public:
    InitDirVector() { r1.init(L","); r2.init(L"\\|"); }
    bool make(RoomDir dir, const tstring& key, MapperDirsVector& dv)
    {
        bool result = true;
        r1.findall(key.c_str());
        int b = 0;
        for (int i = 1, ie = r1.size(); i < ie; ++i) {
            int e = r1.first(i);
            if (!set(dir, key.substr(b, e - b), dv))
                result = false;
            b = e + 1;
        }
        if (!set(dir, key.substr(b), dv))
            result = false;
        return result;
    }
private:
    bool set(RoomDir dir, const tstring& dkey, MapperDirsVector& dv) {
        if (dkey.empty()) return true;
        if (!r2.find(dkey.c_str())) {
            MapperDirCommand k(dir, dkey, L"");
            dv.push_back(k);
            return true;
        }
        if (r2.size() != 1)
            return false;
        int p = r2.first(0);
        tstring main(dkey.substr(0, p));
        tstring rel(dkey.substr(p + 1));
        MapperDirCommand k(dir, main, rel);
        dv.push_back(k);
        return true;
    }
};
