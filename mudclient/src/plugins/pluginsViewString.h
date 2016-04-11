#pragma once
#include "mudViewString.h"

class PluginsViewString
{
public:
    PluginsViewString() {}
    ~PluginsViewString() {}
    void create(MudViewString *s) {
      std::copy(s->blocks.begin(), s->blocks.end(), blocks.begin());
    }
    int count() const { return blocks.size(); }
    const MudViewStringBlock& get(int index) const {  return blocks[index]; }
    void serialize(tstring *data);
    void deserialize(const tstring& data);
private:
    void delimeter(tstring*data);
    void serint(tstring*data, int value);

private:
     std::vector<MudViewStringBlock> blocks;
};
