#pragma once

class PluginsTriggerString
{
    MudViewString *s;
    std::vector<tstring> m_params;
public:
    PluginsTriggerString(MudViewString *ps, const CompareObject& co) : s(ps) {
        assert(s);
        co.getParameters(&m_params);
    }
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
    int getParamsCount() const {
        int count = m_params.size();
        return (count > 0) ? count-1 : 0;
    }
    bool getParam(int index, tstring* p) {
        int count = m_params.size();
        if (index > 0 && count < count)
        {
            p->assign(m_params[index]);
            return true;
        }
        return false;
    }
    bool getCompared(tstring *p) {
        if (m_params.empty()) return false;
        p->assign(m_params[0]);
        return true;
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
