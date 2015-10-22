#pragma once

class PluginsTriggerString
{
    MudViewString *s;
public:
    PluginsTriggerString(MudViewString *ps) : s(ps) { assert(s); }
    int blocks() const { return s->blocks.size(); }
    void getText(tstring* text) { s->getText(text); }
    int  getTextLen() const { return s->getTextLen(); }
    void getBlockText(int block, tstring* text) {
        if (block >= 0 && block < blocks()) {
            text->assign(s->blocks[block].string);
            return;
        }
        text->clear();
    }
    void setBlockText(int block, const tstring& text)
    {
        if (block >= 0 && block < blocks())
            s->blocks[block].string.assign(text);
    }
    

};

class PluginsTrigger
{
public:
    PluginsTrigger();
    ~PluginsTrigger();
    bool init(lua_State *pL);
    bool compare(const CompareData& cd, bool incompl_flag);
    void enable(bool enable);
private:
    lua_State *L;
    CompareObject m_compare;
    int m_tigger_func_index;
    bool m_enabled;
};
