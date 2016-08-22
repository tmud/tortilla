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
    void setBlocksCount(int count)
    {
        blocks.resize(count);
    }

    int insertBlock(int abspos)
    {
        int len = getTextLen();
        if (abspos == len+1) {
           blocks.push_back(MudViewStringBlock());
           return blocks.size();
        }
        if (abspos > 0 && abspos <= len)
        {
            int block = 0; int pos = 0;
            while (abspos > 0)
            {
                abspos--;
                for (int i = 0, e = blocks.size(); i < e; ++i)
                {
                    int size = blocks[i].string.size();
                    if (size > abspos)
                    {
                        block = i + 1;
                        pos = abspos + 1;
                        break;
                    }
                    abspos -= size;
                }
                if (block) break;
            }
            if (block > 0)
            {
                if (pos == 1) {
                    blocks.insert(blocks.begin()+block-1, MudViewStringBlock() );
                    return block;
                }               
                tstring text =  blocks[block-1].string;
                tstring p1(text.substr(0, pos));
                blocks[block-1].string = p1;
                blocks.insert(blocks.begin()+block, MudViewStringBlock());
                blocks.insert(blocks.begin()+block, MudViewStringBlock());

                tstring p2(text.substr(pos));
                blocks[block+1].string = p2;
                blocks[block+1].params = blocks[block-1].params;
                return block + 1;
            }
        }
        return 0;
    }
    void serialize(tstring *data);
    void deserialize(const tstring& data);
private:
     std::vector<MudViewStringBlock> blocks;
};
