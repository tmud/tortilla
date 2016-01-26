#pragma once

struct triggerParseData
{
    std::vector<tstring> params;
    tstring crc;
    void clear() {
        params.clear();
        crc.clear();
    }
};

class PluginsTrigger
{
public:
    PluginsTrigger();
    ~PluginsTrigger();
    bool init(lua_State *pl, Plugin *pp);
    void reset();
    void enable(bool enable);
    bool isEnabled() const;
    int  getLen() const;
    bool compare(const CompareData& cd, bool incompl_flag);
    void run();
private:
    lua_State *L;
    Plugin* p;
    std::vector<CompareObject> m_compare_objects;
    parseData m_parseData;
    std::vector<triggerParseData*> m_triggerParseData;
    lua_ref m_trigger_func_ref;
    int m_current_compare_pos;
    bool m_enabled;
    bool m_triggered;
};
