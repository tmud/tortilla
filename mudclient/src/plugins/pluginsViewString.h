#pragma once
#include "mudViewString.h"

class PluginsViewString
{
public:
    PluginsViewString() {}
    ~PluginsViewString() {}
    void create(MudViewString *s)
    {
      blocks.resize(s->blocks.size());
      std::copy(s->blocks.begin(), s->blocks.end(), blocks.begin());
    }
    void getText(tstring* t) const
    {
        t->clear();
        for (int i=0,e=blocks.size(); i<e; ++i) 
            t->append(blocks[i].string);
    }
    int getTextLen() const
    {
        int len = 0;
        for (int i=0,e=blocks.size(); i<e; ++i) 
            len += blocks[i].string.size();
        return len;
    }
    int count() const { return blocks.size(); }
    MudViewStringBlock& get(int index) {  return blocks[index]; }
    const MudViewStringBlock& ref(int index) const {  return blocks[index]; }
    const MudViewStringBlocks& getBlocks() const { return blocks; }

    bool setBlockText(int index, const tstring& text)
    {
        if (index >= 0 && index < count())
        {
            blocks[index].string = text;
            return true;
        }
        return false;
    }
    bool getBlockText(int index, tstring *text) const
    {
        if (index >= 0 && index < count())
        {
            text->assign(blocks[index].string);
            return true;
        }
        return false;
    }
    bool deleteBlock(int index)
    {
        if (index >= 0 && index < count())
        {
            blocks.erase(blocks.begin()+index);
            return true;
        }
        return false;
    }
    void serialize(tstring *data);
    void deserialize(const tstring& data);
private:
     std::vector<MudViewStringBlock> blocks;
};
