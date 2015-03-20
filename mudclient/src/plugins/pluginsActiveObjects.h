#pragma once

#include "highlightHelper.h"
#include "hotkeyTable.h"
#include "logicHelper.h"
void pluginsUpdateActiveObjects(int type);

class ActiveObjects
{
public:
    ActiveObjects() {}    
    virtual ~ActiveObjects() {}
    virtual const utf8* type() const = 0;
    virtual bool select(int index) = 0;      
    virtual bool get(int param, u8string* value) = 0;
    virtual bool set(int param, const utf8* value) = 0;
    virtual int  size() const = 0;
    virtual bool add(const utf8* key, const utf8* value, const utf8* group) = 0;
    virtual int  getindex() = 0;
    virtual bool setindex(int index) = 0;
    virtual void update() = 0;
    virtual bool del() = 0;

protected:
    bool _get(const tstring& src, u8string *dst)
    {
        w2u.convert(src.c_str(), src.length());
        dst->assign(w2u);
        return true;
    }
    bool _set(tstring& dst, const utf8 *value)
    {        
        u2w.convert(value);
        dst.assign(u2w);
        return true;
    }
    WideToUtf8 w2u;
    Utf8ToWide u2w;
};

class ActiveObjectsEx : public ActiveObjects
{
public:
    enum { CAN_ALL = 0, CAN_VALUE = 1, CAN_GROUP = 2 };
    ActiveObjectsEx(PropertiesData* data, PropertiesValues* obj, const utf8* type, int updatetype, DWORD flags) : 
        pdata(data), actobj(obj), m_type(type), m_updatetype(updatetype), selected(-1), m_flags(flags)
    {
        check_doubles.init("(%[0-9]){1}");
    }
    const utf8* type() const
    {
        return m_type.c_str();
    }
    int size() const 
    {
        return (actobj) ? actobj->size() : 0; 
    }    
    bool select(int index)
    {
        if (index >= 1 && index <= size()) { selected = index-1; return true; }
        return false;
    }
    bool get(int param, u8string* value)
    {
        if (m_flags&CAN_GROUP && param == luaT_ActiveObjects::VALUE)
            return false;
        if (m_flags&CAN_VALUE && param == luaT_ActiveObjects::GROUP)
            return false;
        if (selected >= 0 && selected < size())
        {
            const property_value &v = actobj->get(selected);            
            if (param == luaT_ActiveObjects::KEY)
                return _get(v.key, value);
            if (param == luaT_ActiveObjects::VALUE)
                return _get(v.value, value);
            if (param == luaT_ActiveObjects::GROUP)
                return _get(v.group, value);
            return false;
        }
        return false;
    }
    bool set(int param, const utf8* value)
    {
        if (m_flags&CAN_GROUP && param == luaT_ActiveObjects::VALUE)
            return false;
        if (m_flags&CAN_VALUE && param == luaT_ActiveObjects::GROUP)
            return false;
        if (selected >= 0 && selected < size())
        {
            u8string val(value);
            if (!canset(param, val))
                return false;
            property_value &v = actobj->getw(selected);           
            if (param == luaT_ActiveObjects::KEY)
            {
                if (find(val.c_str()) != -1)
                    return false;
                return _set(v.key, val.c_str());
            }
            if (param == luaT_ActiveObjects::VALUE)
                return _set(v.value, val.c_str());
            if (param == luaT_ActiveObjects::GROUP)
                return _set(v.group, val.c_str());
            return false;
        }
        return false;
    }
    int getindex() 
    {
        return selected+1;
    }
    bool setindex(int index) 
    {
        if (selected == -1)
            return false;
        if (index >= 1 && index <= size())
        {
            actobj->move(selected, index-1);
            selected = index-1;
            return true;
        }
        return false; 
    }
    void update()
    {
        if (m_updatetype >= 0)
            pluginsUpdateActiveObjects(m_updatetype);
    }

    bool del()
    {
        if (selected == -1)
            return false;
        actobj->del(selected);
        selected = -1;
        return true;
    }

    virtual bool canset(int param, u8string& value) 
    {
        return true; 
    }

protected:
    bool checkDoubles(const utf8* key)
    {
        int max_index = -1;
        std::vector<int> indexes;
        check_doubles.findall(key);
        for (int i = 1, e = check_doubles.size(); i<e; ++i)
        {
            int pos = check_doubles.first(i) + 1;
            char symbol = key[pos];
            int id = symbol - L'0';
            indexes.push_back(id);
            if (id > max_index)
                max_index = id;
        }
        if (indexes.empty())
            return false;
        std::vector<int> doubles(max_index + 1, 0);
        for (int i = 0, e = indexes.size(); i<e; ++i)
        {
            int index = indexes[i];
            if (doubles[index] != 0)
                return true;
            doubles[index]++;
        }
        return false;
    }

    int find(const utf8* key)
    {
        U2W k(key);
        for (int i = 0, e = actobj->size(); i < e; ++i)
        {
            const property_value &v = actobj->get(i);
            if (!v.key.compare(k))
                return i;
        }
        return -1;
    }
    void add3(int index, const utf8* key, const utf8* value, const utf8* group)
    {
        U2W _key(key), _value(value), _group(group);
        if (strlen(group) > 0)
            pdata->addGroup(_group);
        actobj->add(index, _key, _value, _group);
    }

    PropertiesData* pdata;
    PropertiesValues* actobj;
    u8string m_type;
    int m_updatetype;
    Pcre check_doubles;
    int selected;
    DWORD m_flags;
};

class AO_Aliases : public ActiveObjectsEx
{
public:
    AO_Aliases(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->aliases, "aliases", LogicHelper::UPDATE_ALIASES, CAN_ALL)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        add3(-1, key, value, group);
        return true;
    }
};

class AO_Subs : public ActiveObjectsEx
{
public:
    AO_Subs(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->subs, "subs", LogicHelper::UPDATE_SUBS, CAN_ALL)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        add3(-1, key, value, group);
        return true;
    }
};

class AO_Antisubs : public ActiveObjectsEx
{
public:
    AO_Antisubs(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->antisubs, "antisubs", LogicHelper::UPDATE_ANTISUBS, CAN_GROUP)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        add3(-1, key, "", group);
        return true;
    }
};

class AO_Actions : public ActiveObjectsEx
{
public:
    AO_Actions(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->actions, "actions", LogicHelper::UPDATE_ACTIONS, CAN_ALL)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        add3(-1, key, value, group);
        return true;
    }
};

class AO_Highlihts : public ActiveObjectsEx
{
    HighlightHelper hh;
public:
    AO_Highlihts(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->highlights, "highlights", LogicHelper::UPDATE_HIGHLIGHTS, CAN_ALL)
    {
    }
    bool canset(const utf8* name, u8string& value)
    {
        if (!strcmp(name,"value"))
        {
            U2W c(value);
            tstring color(c);
            if (!hh.checkText(&color))
                return false;
            W2U r(color.c_str());
            value.assign((const utf8*)r);
            return true;
        }
        return true;
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        U2W c(value);
        tstring color(c);
        tstring_replace(&color, L",", L" "); 
        if (!hh.checkText(&color))  // Highlight helper
            return false;
        U2W _key(key), _group(group);
        pdata->addGroup(_group);
        actobj->add(-1, _key, color, _group);
        return true;      
    }
};

class AO_Hotkeys : public ActiveObjectsEx
{
    HotkeyTable hk;
public:
    AO_Hotkeys(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->hotkeys, "hotkeys", LogicHelper::UPDATE_HOTKEYS, CAN_ALL)
    {
    }
    bool canset(const utf8* name, u8string& value)
    {
        if (!strcmp(name, "key"))
        {
            U2W k(value.c_str());
            tstring key(k);
            tstring norm;
            return hk.isKey(key, &norm);
        }
        return true;
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        U2W k(key);
        tstring _key(k);
        tstring normkey;
        if (!hk.isKey(_key, &normkey))
            return false;
        W2U nk(normkey.c_str());
        if (find(nk) != -1)
            return false;
        U2W _value(value), _group(group);
        pdata->addGroup(_group);
        actobj->add(-1, normkey, _value, _group);
        return true;
    }
};

class AO_Gags : public ActiveObjectsEx
{
public:
    AO_Gags(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->gags, "gags", LogicHelper::UPDATE_GAGS, CAN_GROUP)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (checkDoubles(key))
            return false;
        if (find(key) != -1)
            return false;
        add3(-1, key, "", group);
        return true;
    }
};

class AO_Vars : public ActiveObjectsEx
{
public:
    AO_Vars(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->variables, "vars", -1, CAN_VALUE)
    {
    }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (find(key) != -1)
            return false;
        add3(-1, key, value, "");
        return true;
    }
};

class AO_Timers : public ActiveObjectsEx
{
public:
    AO_Timers(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->timers, "timers", LogicHelper::UPDATE_TIMERS, CAN_ALL)
    {
    }
    bool canset(const utf8* name, u8string& value)
    {
        if (!strcmp(name, "key"))
        {
            return isindex(value.c_str());
        }
        if (!strcmp(name, "value"))
        {
            const utf8 *v = value.c_str();
            const utf8 *p = strchr(v, ';');
            if (!p) return false;
            u8string period(value, p - v);
            return isnumber(period.c_str());
        }
        return true;
    }

    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        // key = 1..10, value = interval;action (ex. 1;drink)
        if (isindex(key))
        {
            const utf8 *p = strchr(value, ';');
            if (!p) return false;
            u8string period(value, p - value);
            if (!isnumber(period.c_str())) return false;            
            u8string action(p + 1);
            if (action.empty()) return false;
            if (find(key) != -1)
                return false;
            add3(-1, key, value, group);
            return true;
        }
        return false;
    }

private:
    bool isnumber(const utf8* str) const
    {
        return (strspn(str, "0123456789") != strlen(str)) ? false : true;
    }
    bool isindex(const utf8* str) const
    {
        if (!isnumber(str))
            return false;
        int index = atoi(str);
        return (index >= 1 && index <= 10) ? true : false;
    }
};
   
class AO_Groups : public ActiveObjectsEx
{
public:
    AO_Groups(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->groups, "groups", LogicHelper::UPDATE_ALL, CAN_VALUE)
    {
    }
    bool canset(const utf8* name, u8string& value)
    {
        if (!strcmp(name, "value"))
        {
            if (value == "0" || value == "1")
                return true;
            return false;
        }        
        return true;
    }    
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        if (find(key) != -1)
            return false;
        u8string v(value);
        if (v.empty())
            v = "0";
        if (v == "0" || v == "1")            
        {
            add3(-1, key, v.c_str(), "");
            return true;
        }
        return false;
    }
};

class AO_Tabs : public ActiveObjects
{
    PropertiesData *data;
    int selected;
public:
    AO_Tabs(PropertiesData* obj) : data(obj), selected(-1)
    {
    }
    const utf8* type() const { return "tabs"; }
    bool select(int index)
    {
        if (index >= 1 && index <= size()) { selected = index-1; return true; }
        return false;
    }
    bool get(int param, u8string* value)
    {
        if (param != luaT_ActiveObjects::KEY)
            return false;
        if (selected >= 0 && selected < size())
        {
            const tstring &v = data->tabwords.get(selected);
            w2u.convert(v.c_str(), v.length());
            value->assign(w2u);
            return true;
        }
        return false;

    }
    bool set(int param, const utf8* value)
    {
        if (param != luaT_ActiveObjects::KEY)
            return false;
        if (selected >= 0 && selected < size())
        {
            tstring &v = data->tabwords.getw(selected);
            u2w.convert(value);
            v.assign(u2w);
            return true;
        }
        return false;
    }
    int getindex() { return selected+1; }
    bool setindex(int index) { return false; }
    int size() const { return data->tabwords.size(); }
    bool add(const utf8* key, const utf8* value, const utf8* group)
    {
        u2w.convert(key);
        tstring new_tab(u2w);
        if (new_tab.empty())
            return false;
        PropertiesList &tabs = data->tabwords;
        if (tabs.find(new_tab) != -1)
            return false;
        tabs.add(-1, new_tab);
        return true;
    }
    bool del()
    {
        if (selected == -1)
            return false;
        data->tabwords.del(selected);
        selected = -1;
        return true;
    }

    void update()
    {
    }
};