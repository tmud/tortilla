#include "stdafx.h"
#include "dirObjects.h"

RoomDir MapperDirCommand::check(const tstring& cmd) const
{
    int size = cmd.size();
    if (size < main_size) return RD_UNKNOWN;
    if (size == main_size)
        return (cmd == main) ? dir : RD_UNKNOWN;
    tstring main_part(cmd.substr(0, main_size));
    if (main_part != main) return RD_UNKNOWN;
    if (rel.empty()) return RD_UNKNOWN;
    tstring rel_part(cmd.substr(main_size));
    int rel_part_size = rel_part.size();
    if (rel_part_size > rel_size) return RD_UNKNOWN;
    return rel.find(rel_part) == 0 ? dir : RD_UNKNOWN;
}
