#pragma once

class PluginsTriggerString
{
    tstring md5;
    std::vector<tstring> m_params;
public:
    PluginsTriggerString(const CompareObject& co, const CompareData& cd) {
        cd.string->getMd5(&md5);
        co.getParameters(&m_params);
    }
    //MudViewString* string() const { return s; }

    int getParamsCount() const {
        int count = m_params.size();
        return (count > 0) ? count-1 : 0;
    }
    bool getParam(int index, tstring* p) {
        int count = m_params.size();
        if (index > 0 && index < count)
        {
            p->assign(m_params[index]);
            return true;
        }
        return false;
    }
    bool getCompared(tstring *p) {
        if (m_params.empty()) 
            return false;
        p->assign(m_params[0]);
        return true;
    }
};

class PluginsTrigger
{
public:
    PluginsTrigger();
    ~PluginsTrigger();
    bool init(lua_State *pl); //, Plugin *pp);
    void reset();
    void enable(bool enable);
    bool isEnabled() const;
    int  getLen() const;
    bool compare(const CompareData& cd, bool incompl_flag);
    void run();
private:
    lua_State *L;
    std::vector<CompareObject> m_compare_objects;
    std::vector<PluginsTriggerString*> m_strings;
    lua_ref m_trigger_func_ref;
    int m_current_compare_pos;
    bool m_enabled;
    bool m_triggered;
};
