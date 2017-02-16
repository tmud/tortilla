#pragma once 
#include "properties.h"
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

class MapperDirsVector
{
public:
    MapperDirsVector();
    void init(PropertiesMapper* props);
    void find(const tstring& cmd);
private:
   std::vector<MapperDirCommand> dirs;
};
