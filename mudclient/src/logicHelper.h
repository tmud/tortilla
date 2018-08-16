#pragma once

#include "propertiesPages/propertiesData.h"
#include "logicElements.h"
#include "varProcessor.h"
#include "plugins/pluginsTriggersHandler.h"
#include "logicPipeline.h"

template <class T>
class LogicWrapper : public std::vector<T*>
{
public:
    ~LogicWrapper() { std::for_each(begin(), end(), [](T* t) { delete t; }); }
    void init(PropertiesValues *values, const std::set<tstring>& active_groups)
    {
        int count = values->size();
        int current = size();
        if (current < count)
            resize(count, NULL);
        int pos = 0;
        for (int i=0; i<count; ++i)
        {
            const property_value &v = values->get(i);
            if (active_groups.find(v.group) == active_groups.end())
                continue;
            T *s = operator[](pos);
            if (!s) { s = new T(); operator[](pos) = s; }
            s->init(v);
            pos++;
        }
        std::for_each(begin()+pos, end(), [](T* t) { delete t; });
        resize(pos);
    }
};

template <class T>
class LogicWrapperParams : public std::vector<T*>
{
public:
    ~LogicWrapperParams() { std::for_each(begin(), end(), [](T* t) { delete t; });  }
    void init(InputTemplateParameters &p, PropertiesValues *values, const std::set<tstring>& active_groups)
    {
        int count = values->size();
        int current = size();
        if (current < count)
            resize(count, NULL);
        int pos = 0;
        for (int i=0; i<count; ++i)
        {
            const property_value &v = values->get(i);
            if (active_groups.find(v.group) == active_groups.end())
                continue;
            T *s = operator[](pos);
            if (!s) { s = new T(); operator[](pos) = s; }
            s->init(v, p);
            pos++;
        }
        std::for_each(begin()+pos, end(), [](T* t) { delete t; });
        resize(pos);
    }
};

class LogicWrapperTimers : public std::vector<Timer*>
{
    int find(const tstring& key) const {
      for (int i=0,e=size(); i<e; ++i) {
        Timer *t = operator[](i);
        if (t->id == key) { return i; }
      }
      return -1;
    }
    void del(const tstring& key) {
       int index = find(key);
       if (index == -1) return;
       Timer *t = operator[](index);
       delete t;
       erase(begin()+index);
    }
public:
    LogicWrapperTimers() {}
    ~LogicWrapperTimers() 
    {
        autodel<Timer> z(*this);
    }
    void update(int id, PropertiesValues *values, const std::set<tstring>& active_groups, const InputTemplateParameters& p )
    {
        tstring key;
        int2w(id, &key);
        int i = values->find(key);
        if (i == -1)
            del(key);
        else
        {
            const property_value &v = values->get(i);
            if (active_groups.find(v.group) == active_groups.end())
                del(key);
            else
            {
                int index = find(key);
                Timer *t = (index == -1) ? new Timer() : operator[](index);
                t->init(v, p);
                if (index == -1)
                    push_back(t);
            }
        }
    }

    void updateall(PropertiesValues *values, const std::set<tstring>& active_groups, const InputTemplateParameters& p)
    {
        std::vector<Timer*> newtimers;
        for (int i=0,e=values->size(); i<e; ++i)
        {
            const property_value &v = values->get(i);
            if (active_groups.find(v.group) == active_groups.end())
                continue;
            int index = find(v.key);
            if (index == -1)
            {
                Timer *t = new Timer();
                t->init(v, p);
                newtimers.push_back(t);
            } 
            else
            {
                Timer *t = operator[](index);
                erase(begin()+index);
                newtimers.push_back(t);
            }
        }
        swap(newtimers);
        std::for_each(newtimers.begin(), newtimers.end(), [](Timer*t) {delete t;});
    }

    void init(PropertiesValues *values, const std::set<tstring>& active_groups, const InputTemplateParameters& p)
    {
        std::map<int, int> timers;
        for (int i=0,e=values->size(); i<e; ++i)
        {
            const property_value &v = values->get(i);
            int id = 0;
            if (!w2int(v.key, &id) || id < 1 || id > TIMERS_COUNT)
                continue;
            if (active_groups.find(v.group) == active_groups.end())
                continue;
            timers[id] = i;
        }

        std::vector<Timer*> newtimers;
        std::map<int, int>::iterator it = timers.begin(); 
        for (int k=0,ke=timers.size(); k<ke; ++k, ++it)
        {
            int i = it->second;
            const property_value &v = values->get(i);
            Timer *t = new Timer();
            t->init(v, p);
            newtimers.push_back(t);
        }
        swap(newtimers);
        std::for_each(newtimers.begin(), newtimers.end(), [](Timer*t) {delete t;});
    }
};

typedef std::vector<tstring> LogicTriggered;

struct parseData;
class LogicHelper
{
public:
    enum { UPDATE_ALL =0, UPDATE_ALIASES, UPDATE_ACTIONS, UPDATE_HOTKEYS, UPDATE_SUBS, UPDATE_ANTISUBS, 
           UPDATE_GAGS, UPDATE_HIGHLIGHTS, UPDATE_TIMERS, UPDATE_VARS, UPDATE_GROUPS, UPDATE_TABS,
           UPDATE_TIMER1 /* UPDATE_TIMER1 + TIMERS_COUNT */
    };

    enum { MODE_ALIASES =1, MODE_ACTIONS, MODE_HOTKEYS, MODE_SUBS, MODE_ANTISUBS, MODE_GAGS, MODE_HIGHLIGHTS, MODE_PLUGINS    
    };

    LogicHelper();
    void updateProps(int what = UPDATE_ALL);
    bool processAliases(const InputCommand cmd, InputCommands* newcmds);
    bool processHotkeys(const tstring& key, InputCommands* newcmds);
    bool processActions(parseData *parse_data, int index, InputCommands *newcmds, LogicTriggered* triggered);
    void processSubs(parseData *parse_data, LogicTriggered* triggered);
    void processAntiSubs(parseData *parse_data, LogicTriggered* triggered);
    void processGags(parseData *parse_data, LogicTriggered* triggered);
    void processHighlights(parseData *parse_data, LogicTriggered* triggered);
    void processTimers(InputCommands* newcmds);
    void resetTimers();
    int  getLeftTime(const tstring& timer_id);
    bool upTimer(const tstring& timer_id);

    enum IfResult { IF_SUCCESS = 0, IF_FAIL, IF_ERROR };
    IfResult compareIF(const tstring& param);
    enum MathResult { MATH_SUCCESS = 0, MATH_VARNOTEXIST, MATH_ERROR };
    MathResult mathOp(const tstring& expr, tstring* result);
private:
    // current workable elements
    LogicWrapperParams<Alias> m_aliases;
    LogicWrapperParams<Hotkey> m_hotkeys;
    LogicWrapperParams<Action> m_actions;
    LogicWrapper<Sub> m_subs;
    LogicWrapper<AntiSub> m_antisubs;
    LogicWrapper<Gag> m_gags;
    LogicWrapper<Highlight> m_highlights;
    LogicWrapperTimers m_timers;
    Pcre16 m_if_regexp;
    Pcre16 m_math_regexp;
    Pcre16 m_params_regexp;
    Ticker m_ticker;
};

class MessageCmdHelper
{
    PropertiesData *propData;
public:
    MessageCmdHelper(PropertiesData *pd) : propData(pd) {}
    int getState(int state)
    {
        int flag = -1;
        switch (state) 
        {
            case LogicHelper::UPDATE_ACTIONS: flag = propData->messages.actions; break;
            case LogicHelper::UPDATE_ALIASES: flag = propData->messages.aliases; break;
            case LogicHelper::UPDATE_ANTISUBS: flag = propData->messages.antisubs; break;
            case LogicHelper::UPDATE_HIGHLIGHTS: flag = propData->messages.highlights; break;
            case LogicHelper::UPDATE_HOTKEYS: flag = propData->messages.hotkeys; break;
            case LogicHelper::UPDATE_GAGS: flag = propData->messages.gags; break;
            case LogicHelper::UPDATE_GROUPS: flag = propData->messages.groups; break;
            case LogicHelper::UPDATE_SUBS: flag = propData->messages.subs; break;
            case LogicHelper::UPDATE_TABS: flag = propData->messages.tabwords; break;
            case LogicHelper::UPDATE_TIMERS: flag = propData->messages.timers; break;
            case LogicHelper::UPDATE_VARS: flag = propData->messages.variables; break;
        }
        return flag;
    }

    bool setState(int state, int value)
    {
        bool ok = true;
        switch (state)
        {
            case LogicHelper::UPDATE_ACTIONS: propData->messages.actions = value; break;
            case LogicHelper::UPDATE_ALIASES: propData->messages.aliases = value; break;
            case LogicHelper::UPDATE_ANTISUBS: propData->messages.antisubs = value; break;
            case LogicHelper::UPDATE_HIGHLIGHTS: propData->messages.highlights = value; break;
            case LogicHelper::UPDATE_HOTKEYS: propData->messages.hotkeys = value; break;
            case LogicHelper::UPDATE_GAGS: propData->messages.gags = value; break;
            case LogicHelper::UPDATE_GROUPS: propData->messages.groups = value; break;
            case LogicHelper::UPDATE_SUBS: propData->messages.subs = value; break;
            case LogicHelper::UPDATE_TABS: propData->messages.tabwords = value; break;
            case LogicHelper::UPDATE_TIMERS: propData->messages.timers = value; break;
            case LogicHelper::UPDATE_VARS: propData->messages.variables = value; break;
            default: ok = false; break;
        }
        return ok;
    }

    void getStrings(tstring *str)
    {
        PropertiesData::message_data &md = propData->messages;
        str->append(stateStr(md.actions)); str->append(L" Триггеры (actions)\r\n");
        str->append(stateStr(md.aliases)); str->append(L" Макросы (aliases)\r\n");
        str->append(stateStr(md.subs)); str->append(L" Замены (subs)\r\n");
        str->append(stateStr(md.antisubs)); str->append(L" Aнтизамены (antisubs)\r\n");
        str->append(stateStr(md.gags)); str->append(L" Фильтры (gags)\r\n");
        str->append(stateStr(md.highlights)); str->append(L" Подсветки (highlights)\r\n");
        str->append(stateStr(md.hotkeys)); str->append(L" Горячие клавиши (hotkeys)\r\n");
        str->append(stateStr(md.groups)); str->append(L" Группы (groups)\r\n");
        str->append(stateStr(md.variables)); str->append(L" Переменные (vars)\r\n");
        str->append(stateStr(md.timers)); str->append(L" Таймеры (timers)\r\n");
        str->append(stateStr(md.tabwords)); str->append(L" Подстановки (tabs)");
    }

    bool setMode(const tstring& state, const tstring& mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;

        int newstate = recognizeValue(mode_value);
        if (newstate == -1)
            return false;
        if (stateid == LogicHelper::UPDATE_ALL)
        {
            propData->messages.initDefault(newstate);
            return true;
        }
        setState(stateid, newstate);
        return true;
    }

    bool getStateString(const tstring& state, tstring *str)
    {
        static const tchar* cmds[] = { L"триггеров (actions)", L"макросов (aliases)", L"замен (subs)", L"антизамен (antisubs)", L"фильтров (gags)",
            L"подсветок (highlights)", L"горячих клавиш (hotkeys)", L"групп (groups)", L"переменных (vars)", L"таймеров (timers)", L"подстановок (tabs)" };
        static const int ids[] = { LogicHelper::UPDATE_ACTIONS, LogicHelper::UPDATE_ALIASES, LogicHelper::UPDATE_SUBS,
            LogicHelper::UPDATE_ANTISUBS, LogicHelper::UPDATE_GAGS, LogicHelper::UPDATE_HIGHLIGHTS, LogicHelper::UPDATE_HOTKEYS,
            LogicHelper::UPDATE_GROUPS, LogicHelper::UPDATE_VARS, LogicHelper::UPDATE_TIMERS, LogicHelper::UPDATE_TABS, 0 };        
        int stateid = recognizeState(state);
        if (stateid == LogicHelper::UPDATE_ALL)
        {
            str->assign(L"Все эхо-уведомления (all) ");
            str->append(stateStr(getState(LogicHelper::UPDATE_ACTIONS)));
            return false;
        }

        for (int i = 0; ids[i]; ++i)
        {
            if (stateid == ids[i])
            { 
                str->assign(L"Эхо-уведомления ");
                str->append(cmds[i]);
                str->append(L" ");
                str->append(stateStr(getState(stateid)));
                break;
            }
        }
        return true;
    }

private:
    const tchar* stateStr(int state)
    {
        return (state ? L"[+]" : L"[-]");
    }

    int recognizeValue(const tstring& value)
    {
        const tstring& n = value;
        if (n.empty())
            return -1;
        if (n == L"вкл" || n == L"enable" || n == L"on" || n == L"1")
            return 1;
        if (n == L"выкл" || n == L"disable" || n == L"off" || n == L"0")
            return 0;
        return -1;
    }

    int recognizeState(const tstring& state)
    {
        static const tchar* cmds[] = { L"actions", L"aliases", L"subs", L"antisubs", L"gags",
            L"highlights", L"hotkeys", L"groups", L"vars", L"timers", L"tabs", L"all", NULL };
        static const int ids[] = { LogicHelper::UPDATE_ACTIONS, LogicHelper::UPDATE_ALIASES, LogicHelper::UPDATE_SUBS,
            LogicHelper::UPDATE_ANTISUBS, LogicHelper::UPDATE_GAGS, LogicHelper::UPDATE_HIGHLIGHTS, LogicHelper::UPDATE_HOTKEYS,
            LogicHelper::UPDATE_GROUPS, LogicHelper::UPDATE_VARS, LogicHelper::UPDATE_TIMERS, LogicHelper::UPDATE_TABS,
            LogicHelper::UPDATE_ALL };

        int stateid = -1;
        int len = state.length();
        for (int i = 0; cmds[i]; ++i)
        {
            if (!wcsncmp(state.c_str(), cmds[i], len))
            {
                stateid = ids[i]; break;
            }
        }
        return stateid;
    }
};

class ModeCmdHelper
{
   PropertiesData *propData;
public:
    ModeCmdHelper(PropertiesData *pd) : propData(pd) {}
    int getState(int state)
    {
        int flag = -1;
        switch (state) 
        {
            case LogicHelper::MODE_ACTIONS: flag = propData->mode.actions; break;
            case LogicHelper::MODE_ALIASES: flag = propData->mode.aliases; break;
            case LogicHelper::MODE_ANTISUBS: flag = propData->mode.antisubs; break;
            case LogicHelper::MODE_HIGHLIGHTS: flag = propData->mode.highlights; break;
            case LogicHelper::MODE_HOTKEYS: flag = propData->mode.hotkeys; break;
            case LogicHelper::MODE_GAGS: flag = propData->mode.gags; break;
            case LogicHelper::MODE_SUBS: flag = propData->mode.subs; break;
            case LogicHelper::MODE_PLUGINS: flag = propData->mode.plugins; break;
        }
        return flag;
    }
    bool setState(int state, int value)
    {
        bool ok = true;
        switch (state)
        {
            case LogicHelper::MODE_ACTIONS: propData->mode.actions = value; break;
            case LogicHelper::MODE_ALIASES: propData->mode.aliases = value; break;
            case LogicHelper::MODE_ANTISUBS: propData->mode.antisubs = value; break;
            case LogicHelper::MODE_HIGHLIGHTS: propData->mode.highlights = value; break;
            case LogicHelper::MODE_HOTKEYS: propData->mode.hotkeys = value; break;
            case LogicHelper::MODE_GAGS: propData->mode.gags = value; break;
            case LogicHelper::MODE_SUBS: propData->mode.subs = value; break;
            case LogicHelper::MODE_PLUGINS: propData->mode.plugins = value; break;
            default: ok = false; break;
        }
        return ok;
    }
    void getStrings(tstring *str)
    {
        PropertiesData::working_mode &md = propData->mode;
        str->append(stateStr(md.actions)); str->append(L" Триггеры (actions)\r\n");
        str->append(stateStr(md.aliases)); str->append(L" Макросы (aliases)\r\n");
        str->append(stateStr(md.subs)); str->append(L" Замены (subs)\r\n");
        str->append(stateStr(md.antisubs)); str->append(L" Aнтизамены (antisubs)\r\n");
        str->append(stateStr(md.gags)); str->append(L" Фильтры (gags)\r\n");
        str->append(stateStr(md.highlights)); str->append(L" Подсветки (highlights)\r\n");
        str->append(stateStr(md.hotkeys)); str->append(L" Горячие клавиши (hotkeys)\r\n");
        str->append(stateStr(md.plugins)); str->append(L" Плагины (plugins)");
    }
    bool setMode(const tstring& state, const tstring& mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;

        int newstate = recognizeValue(mode_value);
		if (newstate == -2)
			return false;
        if (newstate == -1)
            return true;
        if (newstate == 2)
        {
            int curstate = getState(stateid);
            newstate = (curstate == 0) ? 1 : 0;
        }
        setState(stateid, newstate);
        return true;
    }
    bool setMode(const tstring& state, bool mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;
        setState(stateid, mode_value ? 1 : 0);
        return true;
    }
    int getState(const tstring& state) {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return -1;
        return getState(stateid);
    }
    void getStateString(const tstring& state, tstring *str)
    {
        static const tchar* cmds[] = { L"триггеры (actions)", L"макросы (aliases)", L"замены (subs)", L"антизамены (antisubs)", L"фильтры (gags)",
            L"подсветки (highlights)", L"горячие клавиши (hotkeys)", L"плагины (plugins)" };
        static const int ids[] = { LogicHelper::MODE_ACTIONS, LogicHelper::MODE_ALIASES, LogicHelper::MODE_SUBS,
            LogicHelper::MODE_ANTISUBS, LogicHelper::MODE_GAGS, LogicHelper::MODE_HIGHLIGHTS, LogicHelper::MODE_HOTKEYS,
            LogicHelper::MODE_PLUGINS, 0 };
        int stateid = recognizeState(state);
        for (int i = 0; ids[i]; ++i)
        {
            if (stateid == ids[i])
            { 
                str->assign(L"Компонент ");
                str->append(cmds[i]);
                str->append(L" ");
                str->append(stateStr(getState(stateid)));
                break;
            }
        }
    }
private:
    const tchar* stateStr(int state)
    {
        return (state ? L"[+]" : L"[-]");
    }

    int recognizeValue(const tstring& value)
    {
        const tstring& n = value;
        if (n.empty())
            return -1;
        if (n == L"вкл" || n == L"enable" || n == L"on" || n == L"1")
            return 1;
        if (n == L"выкл" || n == L"disable" || n == L"off" || n == L"0")
            return 0;
        if (n == L"перекл" || n == L"toggle" || n == L"switch")
            return 2;
        return -2;
    }

    int recognizeState(const tstring& state)
    {
        static const tchar* cmds[] = { L"actions", L"aliases", L"subs", L"antisubs", L"gags",
            L"highlights", L"hotkeys", L"plugins", NULL };
        static const int ids[] = { LogicHelper::MODE_ACTIONS, LogicHelper::MODE_ALIASES, LogicHelper::MODE_SUBS,
            LogicHelper::MODE_ANTISUBS, LogicHelper::MODE_GAGS, LogicHelper::MODE_HIGHLIGHTS, LogicHelper::MODE_HOTKEYS,
            LogicHelper::MODE_PLUGINS };
        int stateid = -1;
        int len = state.length();
        for (int i = 0; cmds[i]; ++i)
        {
            if (!wcsncmp(state.c_str(), cmds[i], len))
            {
                stateid = ids[i]; break;
            }
        }
        return stateid;
    }
};

class DebugCmdHelper
{
   PropertiesData *propData;
public:
    DebugCmdHelper(PropertiesData *pd) : propData(pd) {}
    int getState(int state)
    {
        int flag = -1;
        switch (state) 
        {
            case LogicHelper::MODE_ACTIONS: flag = propData->debug.actions; break;
            case LogicHelper::MODE_ANTISUBS: flag = propData->debug.antisubs; break;
            case LogicHelper::MODE_HIGHLIGHTS: flag = propData->debug.highlights; break;
            case LogicHelper::MODE_GAGS: flag = propData->debug.gags; break;
            case LogicHelper::MODE_SUBS: flag = propData->debug.subs; break;
        }
        return flag;
    }
    bool setState(int state, int value)
    {
        bool ok = true;
        switch (state)
        {
            case LogicHelper::MODE_ACTIONS: propData->debug.actions = value; break;
            case LogicHelper::MODE_ANTISUBS: propData->debug.antisubs = value; break;
            case LogicHelper::MODE_HIGHLIGHTS: propData->debug.highlights = value; break;
            case LogicHelper::MODE_GAGS: propData->debug.gags = value; break;
            case LogicHelper::MODE_SUBS: propData->debug.subs = value; break;
            default: ok = false; break;
        }
        return ok;
    }

    void getStrings(tstring *str)
    {
        PropertiesData::debug_data &md = propData->debug;
        str->append(stateStr(md.actions)); str->append(L" Триггеры (actions)\r\n");
        str->append(stateStr(md.subs)); str->append(L" Замены (subs)\r\n");
        str->append(stateStr(md.antisubs)); str->append(L" Aнтизамены (antisubs)\r\n");
        str->append(stateStr(md.gags)); str->append(L" Фильтры (gags)\r\n");
        str->append(stateStr(md.highlights)); str->append(L" Подсветки (highlights)");
    }

    bool setMode(const tstring& state, const tstring& mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;

        int newstate = recognizeValue(mode_value);
		if (newstate == -2)
			return false;
        if (newstate == -1)
            return true;
        if (newstate == 2)
        {
            int curstate = getState(stateid);
            newstate = (curstate == 0) ? 1 : 0;
        }
        setState(stateid, newstate);
        return true;
    }

    bool setMode(const tstring& state, bool mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;
        setState(stateid, mode_value ? 1 : 0);
        return true;
    }

    int getState(const tstring& state) {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return -1;
        return getState(stateid);
    }

    void getStateString(const tstring& state, tstring *str)
    {
        static const tchar* cmds[] = { L"триггеров (actions)", L"замен (subs)", L"антизамен (antisubs)", L"фильтров (gags)",
            L"подсветок (highlights)" };
        static const int ids[] = { LogicHelper::MODE_ACTIONS, LogicHelper::MODE_SUBS,
            LogicHelper::MODE_ANTISUBS, LogicHelper::MODE_GAGS, LogicHelper::MODE_HIGHLIGHTS, 0 };
        int stateid = recognizeState(state);
        for (int i = 0; ids[i]; ++i)
        {
            if (stateid == ids[i])
            { 
                str->assign(L"Отладка ");
                str->append(cmds[i]);
                str->append(L" ");
                str->append(stateStr(getState(stateid)));
                break;
            }
        }
    }
private:
    const tchar* stateStr(int state)
    {
        return (state ? L"[+]" : L"[-]");
    }

    int recognizeValue(const tstring& value)
    {
        const tstring& n = value;
        if (n.empty())
            return -1;
        if (n == L"вкл" || n == L"enable" || n == L"on" || n == L"1")
            return 1;
        if (n == L"выкл" || n == L"disable" || n == L"off" || n == L"0")
            return 0;
        if (n == L"перекл" || n == L"toggle" || n == L"switch")
            return 2;
        return -2;
    }

    int recognizeState(const tstring& state)
    {
        static const tchar* cmds[] = { L"actions", L"subs", L"antisubs", L"gags", L"highlights", NULL };
        static const int ids[] = { LogicHelper::MODE_ACTIONS, LogicHelper::MODE_SUBS,
            LogicHelper::MODE_ANTISUBS, LogicHelper::MODE_GAGS, LogicHelper::MODE_HIGHLIGHTS };

        int stateid = -1;
        int len = state.length();
        for (int i = 0; cmds[i]; ++i)
        {
            if (!wcsncmp(state.c_str(), cmds[i], len))
            {
                stateid = ids[i]; break;
            }
        }
        return stateid;
    }
};