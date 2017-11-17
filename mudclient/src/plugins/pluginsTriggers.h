#pragma once

class TriggerActionHook {
public:
    virtual ~TriggerActionHook() {}
    virtual void run() = 0;
};
typedef std::shared_ptr<TriggerActionHook> TriggerAction;

struct triggerParseDataString
{
    std::vector<tstring> params;
    tstring crc;
    void clear() {
        params.clear();
        crc.clear();
    }
};

class triggerKeyData {
public:
    virtual bool getKey(int index, tstring* key) = 0;
    virtual int getLen() const = 0;
};

class triggerParseData
{
    triggerKeyData *tr;
    parseData m_parseData;
    int m_current_compare_pos;        
    std::vector<triggerParseDataString*> m_strings;
    std::vector<int> m_indexes;
public:
    triggerParseData(triggerKeyData *t);
    ~triggerParseData();    
    int  getComparePos() const { return m_current_compare_pos; }
    void incComparePos() { m_current_compare_pos++; }
    void pushString(const CompareData& cd, const CompareObject &co, bool incompl_flag);
    void reset();
    void markDeleted(int string_index) 
    {
        if (!correctindex(string_index))
            return;
        m_indexes.erase(m_indexes.begin()+string_index);
    }
    void markDeletedAll()
    {
        m_indexes.clear();
    }
    int getParameters(int string_index) const
    {
        triggerParseDataString *s = get(string_index);
        return (s) ? s->params.size() : 0;
    }
    bool getParameter(int string_index, int parameter, tstring* p) const
    {
        triggerParseDataString *s = get(string_index);
        if (s) 
        {
            int count = s->params.size();
            if (parameter >= 0 && parameter < count)
                { p->assign(s->params[parameter]); return true; }
        }
        return false;
    }
    bool getCRC(int string_index, tstring* crc) const
    {
        triggerParseDataString *s = get(string_index);
        if (!s) return false;
        crc->assign(s->crc);
        return true;
    }
    triggerParseDataString* get(int string_index) const
    {
        if (correctindex(string_index))
        {
            int s = m_indexes[string_index];
            return m_strings[s];
        }
        return NULL;
    }
    bool getKey(int string_index, tstring* key) const
    {
        if (correctindex(string_index)) {

            int s = m_indexes[string_index];
            return tr->getKey(s, key);
        }
        return false;
    }
private:    
    void resetindex();
    bool correctindex(int string_index) const;
};

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
    void moveFrom(triggerParseVector &v) {
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
