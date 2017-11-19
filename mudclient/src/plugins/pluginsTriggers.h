#pragma once
#include "pluginsTriggerData.h"
#include "pluginsTriggersHandler.h"

class triggerParseVector {
public:
    ~triggerParseVector() { std::for_each(tpd.begin(), tpd.end(), [](triggerParseData* t) { delete t; }); }
    void push_back(triggerParseData* t) { tpd.push_back(t); }
    triggerParseData* pop_back() { 
        if (tpd.empty()) return NULL;
        int last = tpd.size() - 1;
        triggerParseData* t = tpd[last];
        tpd.pop_back();
        return t;
    }
    bool empty() const { return tpd.empty(); }
    int size() const { return tpd.size(); }
    triggerParseData* operator[](int index) {
        int count = tpd.size();
        return (index >=0 && index < count) ? tpd[index] : NULL;
    }
    void clear() { tpd.clear(); }
    void replaceFrom(triggerParseVector &v) {
        tpd.clear();
        tpd.swap(v.tpd);
    }
private:
    std::vector<triggerParseData*> tpd;
};

class PluginsTrigger : public triggerKeyData
{
public:
    PluginsTrigger();
    ~PluginsTrigger();
    bool init(lua_State *pl, Plugin *pp);
    void enable(bool enable);
    bool isEnabled() const;
    int  getLen() const;
    bool getKey(int index, tstring* key);  
    TriggerAction compare(const CompareData& cd, bool incompl_flag);
    void run(triggerParseVector* action);
private:
    enum CompareResult { CR_FAIL = 0, CR_NEXT, CR_OK };
    CompareResult compareParseData(triggerParseData* tpd, const CompareData& cd, bool incompl_flag);
    void freeTriggerData(triggerParseData* tpd);
    triggerParseData* getFreeTriggerData();
    lua_State *L;
    Plugin* p;
    bool m_enabled;
    lua_ref m_trigger_func_ref;
    std::vector<CompareObject> m_compare_objects;
    triggerParseVector m_triggers_in_comparing;
    triggerParseVector m_empty_data;
};
