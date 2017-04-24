#pragma once

#include "highlightHelper.h"
#include "hotkeyTable.h"
#include "logicHelper.h"
void pluginsUpdateActiveObjects(int type);

class ActiveObjectsFilter
{
public:
    virtual bool canset(const tchar* var) = 0;
};

class ActiveObjects
{
public:
    ActiveObjects() {}
    virtual ~ActiveObjects() {}
    virtual const tchar* type() const = 0;
    virtual bool select(int index) = 0;
    virtual bool get(int param, tstring* value) = 0;
    virtual bool set(int param, const tchar* value) = 0;
    virtual int  size() const = 0;
    virtual bool add(const tchar* key, const tchar* value, const tchar* group) = 0;
    virtual bool replace(const tchar* key, const tchar* value, const tchar* group) = 0;
    virtual int  getindex() = 0;
    virtual bool setindex(int index) = 0;
    virtual void update() = 0;
    virtual bool del() = 0;
    virtual bool showmessage() const = 0;
    virtual void format(tstring *fmt) = 0;
    virtual void formatdel(tstring *fmt) = 0;
    virtual int find(const tchar* key) = 0;
    virtual int findnext(const tchar* key) = 0;
    virtual bool isGroupActive() = 0;
};

class ActiveObjectsEx : public ActiveObjects
{
public:
    enum { CAN_ALL = 0, CAN_VALUE = 1, CAN_GROUP = 2 };
    ActiveObjectsEx(PropertiesData* data, PropertiesValues* obj, const tchar* type, int updatetype, DWORD flags) : 
        pdata(data), actobj(obj), m_type(type), m_updatetype(updatetype), selected(-1), m_flags(flags)
    {
        check_doubles.setRegExp(L"(%[0-9]){1}");
    }
    const tchar* type() const
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
    bool get(int param, tstring* value)
    {
        if (m_flags&CAN_GROUP && param == luaT_ActiveObjects::VALUE)
            return false;
        if (m_flags&CAN_VALUE && param == luaT_ActiveObjects::GROUP)
            return false;
        if (selected >= 0 && selected < size())
        {
            const property_value &v = actobj->get(selected);
            if (param == luaT_ActiveObjects::KEY)
                { value->assign(v.key); return true; }
            if (param == luaT_ActiveObjects::VALUE)
                { value->assign(v.value); return true; }
            if (param == luaT_ActiveObjects::GROUP)
                { value->assign(v.group); return true; }
            return false;
        }
        return false;
    }
    bool set(int param, const tchar* value)
    {
        if (m_flags&CAN_GROUP && param == luaT_ActiveObjects::VALUE)
            return false;
        if (m_flags&CAN_VALUE && param == luaT_ActiveObjects::GROUP)
            return false;
        if (selected >= 0 && selected < size())
        {
            tstring val(value);
            if (!canset(param, val))
                return false;
            property_value &v = actobj->getw(selected);
            if (param == luaT_ActiveObjects::KEY)
            {
                if (find(val.c_str()) != -1)
                    return false;
                v.key = val;
                return true;
            }
            if (param == luaT_ActiveObjects::VALUE)
                { v.value = val; return true; }
            if (param == luaT_ActiveObjects::GROUP)
                { v.group = val; return true; }
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

    virtual bool canset(int param, tstring& value) 
    {
        return true; 
    }
    int find(const tchar* key)
    {
        return find(key, 0);
    }
    int findnext(const tchar* key)
    {
        if (selected == -1)
            return find(key, 0);
        return find(key, selected+1);
    }
    virtual bool isGroupActive()
    {
        if (selected == -1)
            return false;
        const property_value &v = actobj->get(selected);
        const tstring& group = v.group;
        int index = pdata->groups.find(group);
        if (index == -1) {
           assert(false);
           return false;
        }
        const property_value &g = pdata->groups.get(index);
        return (g.value == L"1");
    }
protected:
    int find(const tchar* key, int from)
    {
        for (int i = from, e = actobj->size(); i < e; ++i)
        {
            const property_value &v = actobj->get(i);
            if (!v.key.compare(key))
                return i;
        }
        return -1;
    }
    void add3(int index, const tchar* key, const tchar* value, const tchar* group)
    {
        if (wcslen(group) > 0)
            pdata->addGroup(group);
        actobj->add(index, key, value, group);
        if (index == -1)
            selected = size()-1;
        else
            selected = index;
    }

    void add3group(int index, const tchar* key, const tchar* value, const tchar* group)
    {
        if (wcslen(group) == 0) {
            const property_value& v = pdata->groups.get(0);
            group = v.key.c_str();
        }
        add3(index, key, value, group);
    }

    PropertiesData* pdata;
    PropertiesValues* actobj;
    tstring m_type;
    int m_updatetype;
    Pcre16 check_doubles;
    int selected;
    DWORD m_flags;
};

class AO_Aliases : public ActiveObjectsEx
{
public:
    AO_Aliases(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->aliases, L"aliases", LogicHelper::UPDATE_ALIASES, CAN_ALL)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3group(-1, key, value, group);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3group(index, key, value, group);
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.aliases ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->aliases.get(selected);
        std::wstringstream ss;
        ss << L"+alias {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->aliases.get(selected);
        std::wstringstream ss;
        ss << L"-alias {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
};

class AO_Subs : public ActiveObjectsEx
{
public:
    AO_Subs(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->subs, L"subs", LogicHelper::UPDATE_SUBS, CAN_ALL)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3group(-1, key, value, group);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3group(index, key, value, group);
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.subs ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->subs.get(selected);
        std::wstringstream ss;
        ss << L"+action {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->subs.get(selected);
        std::wstringstream ss;
        ss << L"-action {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
};

class AO_Antisubs : public ActiveObjectsEx
{
public:
    AO_Antisubs(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->antisubs, L"antisubs", LogicHelper::UPDATE_ANTISUBS, CAN_GROUP)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3group(-1, key, L"", group);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3group(index, key, L"", group);
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.antisubs ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->antisubs.get(selected);
        std::wstringstream ss;
        ss << L"+antisub {" << v.key << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->antisubs.get(selected);
        std::wstringstream ss;
        ss << L"-antisub {" << v.key << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
};

class AO_Actions : public ActiveObjectsEx
{
public:
    AO_Actions(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->actions, L"actions", LogicHelper::UPDATE_ACTIONS, CAN_ALL)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3group(-1, key, value, group);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3group(index, key, value, group);
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.actions ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->actions.get(selected);
        std::wstringstream ss;
        ss << L"+action {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->actions.get(selected);
        std::wstringstream ss;
        ss << L"-action {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
};

class AO_Highlihts : public ActiveObjectsEx
{
    HighlightHelper hh;
public:
    AO_Highlihts(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->highlights, L"highlights", LogicHelper::UPDATE_HIGHLIGHTS, CAN_ALL)
    {
    }
    bool canset(int param, tstring& value)
    {
        if (param == luaT_ActiveObjects::VALUE)
        {
            if (!hh.translateColor(&value))
                return false;
            return true;
        }
        return true;
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, false);
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, true);
    }
    bool showmessage() const
    {
        return pdata->messages.highlights ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->highlights.get(selected);
        std::wstringstream ss;
        ss << L"+highlight {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->highlights.get(selected);
        std::wstringstream ss;
        ss << L"-highlight {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
private:
    bool add(const tchar* key, const tchar* value, const tchar* group, bool replace_mode)
    {
        int index = find(key);
        if (index != -1 && !replace_mode)
            return false;
        tstring color(value);
        tstring_replace(&color, L",", L" "); 
        if (!hh.translateColor(&color))  // Highlight helper
            return false;
        add3group(index, key, color.c_str(), group);
        return true;
    }
};

class AO_Hotkeys : public ActiveObjectsEx
{
    HotkeyTable hk;
public:
    AO_Hotkeys(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->hotkeys, L"hotkeys", LogicHelper::UPDATE_HOTKEYS, CAN_ALL)
    {
    }
    bool canset(int param, tstring& value)
    {
        if (param == luaT_ActiveObjects::KEY)
        {
            tstring norm;
            return hk.isKey(value, &norm);
        }
        return true;
    }

    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, false);
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, true);
    }
    bool showmessage() const
    {
        return pdata->messages.hotkeys ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->hotkeys.get(selected);
        std::wstringstream ss;
        ss << L"+hotkey {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->hotkeys.get(selected);
        std::wstringstream ss;
        ss << L"-hotkey {" << v.key << L"} {" << v.value << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
private:
    bool add(const tchar* key, const tchar* value, const tchar* group, bool replace_mode)
    {
        tstring normkey;
        if (!hk.isKey(key, &normkey))
            return false;
        int index = find(normkey.c_str());
        if (index != -1 && !replace_mode)
            return false;
        add3group(index, normkey.c_str(), value, group);
        return true;
    }
};

class AO_Gags : public ActiveObjectsEx
{
public:
    AO_Gags(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->gags, L"gags", LogicHelper::UPDATE_GAGS, CAN_GROUP)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3group(-1, key, L"", group);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3group(index, key, L"", group);
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.gags ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->gags.get(selected);
        std::wstringstream ss;
        ss << L"+gag {" << v.key << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }    
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->gags.get(selected);
        std::wstringstream ss;
        ss << L"-gag {" << v.key << L"} [" << v.group << L"]";
        fmt->assign(ss.str());
    }
};

class AO_Vars : public ActiveObjectsEx
{
    ActiveObjectsFilter *m_pFilter;
public:
    AO_Vars(PropertiesData* obj, ActiveObjectsFilter* filter) : ActiveObjectsEx(obj, &obj->variables, L"vars", -1, CAN_VALUE), m_pFilter(filter)
    {
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        if (find(key) != -1)
            return false;
        add3(-1, key, value, L"");
        return true;
    }

    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        int index = find(key);
        add3(index, key, value, L"");
        return true;
    }

    bool canset(int param, tstring& value)
    {
        if (param == luaT_ActiveObjects::KEY && m_pFilter)
            return m_pFilter->canset(value.c_str());
        return true;
    }
    bool showmessage() const
    {
        return pdata->messages.variables ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->variables.get(selected);
        std::wstringstream ss;
        ss << L"$" << v.key << L"='" << v.value << L"'";
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->variables.get(selected);
        std::wstringstream ss;
        ss << L"-$" << v.key;
        fmt->assign(ss.str());
    }
};

class AO_Timers : public ActiveObjectsEx
{
public:
    AO_Timers(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->timers, L"timers", LogicHelper::UPDATE_TIMERS, CAN_ALL)
    {
    }
    bool canset(int param, tstring& value)
    {
        if (param == luaT_ActiveObjects::KEY)
        {
            return isindex(value.c_str());
        }
        if (param == luaT_ActiveObjects::VALUE)
        {
            const tchar *v = value.c_str();
            const tchar *p = wcschr(v, L';');
            if (!p) return false;
            tstring period(value, p - v);
            return isnumber(period);
        }
        return true;
    }

    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, false);
    }

    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, true);
    }
    bool showmessage() const
    {
        return pdata->messages.timers ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value &v = pdata->timers.get(selected);
        PropertiesTimer pt; pt.convertFromString(v.value);
        std::wstringstream ws;
        ws << L"+timer #" << v.key << L" " << pt.timer << L" сек: {" << pt.cmd << L"} [" << v.group << L"]";
        fmt->assign(ws.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value& v = pdata->timers.get(selected);
        std::wstringstream ss;
        ss << L"-timer #" << v.key;
        fmt->assign(ss.str());
    }
private:
    bool add(const tchar* key, const tchar* value, const tchar* group, bool replace_mode)
    {
        // key = 1..10, value = interval;action (ex. 1;drink)
        if (isindex(key))
        {
            const tchar *p = wcschr(value, L';');
            if (!p) return false;
            tstring period(value, p - value);
            if (!isnumber(period)) return false;
            tstring action(p + 1);
            if (action.empty()) return false;
            int index = find(key);
            if (index != -1 && !replace_mode)
                return false;
            add3group(index, key, value, group);
            return true;
        }
        return false;
    }

    bool isnumber(const tstring& str) const
    {
        return isOnlyDigits(str);
    }
    bool isindex(const tstring& str) const
    {
        if (!isnumber(str))
            return false;
        int index = 0;
        w2int(str, &index);
        return (index >= 1 && index <= 10) ? true : false;
    }
};

class AO_Groups : public ActiveObjectsEx
{
public:
    AO_Groups(PropertiesData* obj) : ActiveObjectsEx(obj, &obj->groups, L"groups", LogicHelper::UPDATE_ALL, CAN_VALUE)
    {
    }
    bool canset(int param, tstring& value)
    {
        if (param == luaT_ActiveObjects::VALUE)
        {
            if (value == L"0" || value == L"1")
                return true;
            return false;
        }
        return true;
    }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        return add(key, value, group, false);
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        return false;
    }
    bool showmessage() const
    {
        return pdata->messages.groups ? true : false;
    }
    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value& v = pdata->groups.get(selected);
        std::wstringstream ss;
        ss << L"group {" << v.key << L"} ";
        if (v.value == L"0")
            ss << L"off";
        else if (v.value == L"1")
            ss << L"on";
        else {
            return;
        }
        fmt->assign(ss.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const property_value& v = pdata->groups.get(selected);
        std::wstringstream ss;
        ss << L"-group {" << v.key << L"}";
        fmt->assign(ss.str());
    }
    bool isGroupActive()
    {
        if (selected == -1) return false;
        const property_value& v = pdata->groups.get(selected);
        return (v.value == L"1");
    }
private:
    bool add(const tchar* key, const tchar* value, const tchar* group, bool replace_mode)
    {
        int index = find(key);
        if (index != -1 && !replace_mode)
            return false;
        tstring v(value);
        if (v.empty())
            v = L"0";
        if (v == L"0" || v == L"1")
        {
            add3(index, key, v.c_str(), L"");
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
    const tchar* type() const { return L"tabs"; }
    bool select(int index)
    {
        if (index >= 1 && index <= size()) { selected = index-1; return true; }
        return false;
    }
    int find(const tchar* key)
    {
        return find(key, 0);
    }
    int findnext(const tchar* key)
    {
        if (selected == -1)
            return find(key, 0);
        return find(key, selected + 1);
    }
    bool isGroupActive() 
    {
        return true;    
    }
protected:
    int find(const tchar* key, int from)
    {
        for (int i = from, e = data->tabwords.size(); i < e; ++i)
        {
            const tstring &v =  data->tabwords.get(i);
            if (!v.compare(key))
                return i;
        }
        return -1;
    }
    bool get(int param, tstring* value)
    {
        if (param != luaT_ActiveObjects::KEY)
            return false;
        if (selected >= 0 && selected < size())
        {
            const tstring &v = data->tabwords.get(selected);
            value->assign(v);
            return true;
        }
        return false;
    }
    bool set(int param, const tchar* value)
    {
        if (param != luaT_ActiveObjects::KEY)
            return false;
        if (selected >= 0 && selected < size())
        {
            tstring &v = data->tabwords.getw(selected);
            v.assign(value);
            return true;
        }
        return false;
    }
    int getindex() { return selected+1; }
    bool setindex(int index) { return false; }
    int size() const { return data->tabwords.size(); }
    bool add(const tchar* key, const tchar* value, const tchar* group)
    {
        tstring new_tab(key);
        if (new_tab.empty())
            return false;
        PropertiesList &tabs = data->tabwords;
        if (tabs.find(new_tab) != -1)
            return false;
        selected = tabs.add(-1, new_tab);
        return true;
    }
    bool replace(const tchar* key, const tchar* value, const tchar* group)
    {
        tstring new_tab(key);
        if (new_tab.empty())
            return false;
        PropertiesList &tabs = data->tabwords;
        int index = tabs.find(new_tab);
        selected = tabs.add(index, new_tab);
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

    void format(tstring *fmt)
    {
        if (selected == -1) return;
        const tstring &v = data->tabwords.get(selected);
        std::wstringstream ws;
        ws << L"+tab " << v;
        fmt->assign(ws.str());
    }
    void formatdel(tstring *fmt)
    {
        if (selected == -1) return;
        const tstring& v = data->tabwords.get(selected);
        std::wstringstream ss;
        ss << L"-tab " << v;
        fmt->assign(ss.str());
    }

    bool showmessage() const
    {
        return data->messages.tabwords ? true : false;
    }
};
