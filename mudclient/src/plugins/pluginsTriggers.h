#pragma once

struct triggerParseDataString
{
    std::vector<tstring> params;
    tstring crc;
    void clear() {
        params.clear();
        crc.clear();
    }
};

class triggerParseData
{
public:
    triggerParseData() {}
    ~triggerParseData() { destroy(); }
    void init(const std::vector<CompareObject>& cobjs)
    {
        destroy();
        int count = cobjs.size();
        m_strings.resize(count, NULL);
        for (int i=0;i<count;++i) { m_strings[i] = new triggerParseDataString;  }
        m_keys.resize(count);
        for (int i=0;i<count;++i) { cobjs[i].getKey(&m_keys[i]); }
        resetindex();
    }
    void reset()
    {
        std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { tpd->clear(); });
        resetindex();
    }
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
            key->assign(m_keys[string_index]);
            return true;
        }
        return false;
    }
private:
    void destroy() {
        std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { delete tpd;} );
        m_strings.clear();
    }
    void resetindex() {
        int count = m_strings.size();
        m_indexes.resize(count);
        for (int i=0;i<count;++i) m_indexes[i] = i;
    }
    bool correctindex(int string_index) const {
        int count = m_indexes.size();
        return (string_index >= 0 && string_index < count) ? true : false;
    }
    std::vector<triggerParseDataString*> m_strings;
    std::vector<int> m_indexes;
    std::vector<tstring> m_keys;
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
    bool getKey(int index, tstring* key);
    bool compare(const CompareData& cd, bool incompl_flag);
    void run();
private:
    lua_State *L;
    Plugin* p;
    std::vector<CompareObject> m_compare_objects;
    parseData m_parseData;
    triggerParseData m_triggerParseData;
    lua_ref m_trigger_func_ref;
    int m_current_compare_pos;
    bool m_enabled;
    bool m_triggered;
};
