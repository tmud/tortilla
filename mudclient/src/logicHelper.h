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
    ~LogicWrapper() { clear(); }
    void clear() { autodel<T> z(*this); }
    void init(PropertiesValues *values, const std::vector<tstring>& active_groups)
    {
        clear();
        int count = values->size();
        for (int i=0; i<count; ++i)
        {
            const property_value &v = values->get(i);
            if (std::find(active_groups.begin(), active_groups.end(), v.group) == active_groups.end())
                continue;
            T *s = new T(v);
            push_back(s);
        }
    }
};

template <class T>
class LogicWrapperParams : public std::vector<T*>
{
public:
    ~LogicWrapperParams() { clear(); }
    void clear() { autodel<T> z(*this); }
    void init(InputTemplateParameters &p, PropertiesValues *values, const std::vector<tstring>& active_groups)
    {
        clear();
        int count = values->size();
        for (int i=0; i<count; ++i)
        {
            const property_value &v = values->get(i);
            if (std::find(active_groups.begin(), active_groups.end(), v.group) == active_groups.end())
                continue;
            T *s = new T(v, p);
            push_back(s);
        }
    }
};

class LogicWrapperTimers : public std::vector<Timer*>
{
public:
    LogicWrapperTimers() {}
    ~LogicWrapperTimers() 
    {
        autodel<Timer> z(*this);
    }
    void init(PropertiesValues *values, const std::vector<tstring>& active_groups, const InputTemplateParameters& p)
    {
        std::vector<int> timers;
        for (int i=0,e=values->size(); i<e; ++i)
        {
            const property_value &v = values->get(i);
            if (!isInt(v.key))
                continue;
            int id = _wtoi(v.key.c_str());
            if (id < 1 || id > TIMERS_COUNT)
                continue;
            if (std::find(active_groups.begin(), active_groups.end(), v.group) == active_groups.end())
                continue;
            timers.push_back(i);
        }

        for (int i=0,e=timers.size(); i<e; ++i)
        {
            const property_value &v = values->get(timers[i]);
            int index = -1;
            for (int j=0,je=size(); j<je; ++j)
            {
                if (!v.key.compare(at(j)->id))
                    { index = j; break; }
            }
            Timer *t = (index == -1) ? new Timer() : at(index);
            if (index == -1)
                push_back(t);
            t->init(v, p);
        }

        std::vector<int> todelete;
        for (int j=0,je=size(); j<je; ++j)
        {
            bool exist = false;
            for (int i=0,e=timers.size(); i<e; ++i)
            {
                const property_value &v = values->get(timers[i]);
                if (!v.key.compare( at(j)->id))
                    { exist = true; break; }
            }
            if (!exist)
                todelete.push_back(j);
        }

        int last=todelete.size()-1;
        for (int i=last; i>=0; --i)
        {
            int id = todelete[i];
            Timer *t = at(id);
            erase(begin() + id);
            delete t;
        }
    }
};

struct parseData;
class LogicHelper
{
public:
    enum { UPDATE_ALL =0, UPDATE_ALIASES, UPDATE_ACTIONS, UPDATE_HOTKEYS, UPDATE_SUBS, UPDATE_ANTISUBS, 
           UPDATE_GAGS, UPDATE_HIGHLIGHTS, UPDATE_TIMERS, UPDATE_VARS, UPDATE_GROUPS, UPDATE_TABS };

    LogicHelper();
    void updateProps(int what = UPDATE_ALL);
    bool processAliases(const InputCommand* cmd, InputCommands* newcmds);
    bool processHotkeys(const tstring& key, InputCommands* newcmds);
    void processActions(parseData *parse_data, PluginsTriggersHandler* plugins_triggers, LogicPipelineElement *pe);
    void processSubs(parseData *parse_data);
    void processAntiSubs(parseData *parse_data);
    void processGags(parseData *parse_data);
    void processHighlights(parseData *parse_data);
    void processTimers(InputCommands* newcmds);
    void resetTimers();

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
        str->append(L"Триггеры (actions)"); str->append(stateStr(md.actions));
        str->append(L"Макросы (aliases)"); str->append(stateStr(md.aliases));
        str->append(L"Замены (subs)"); str->append(stateStr(md.subs));
        str->append(L"Aнтизамены (antisubs)"); str->append(stateStr(md.antisubs));
        str->append(L"Фильтры (gags)"); str->append(stateStr(md.gags));
        str->append(L"Подсветки (highlights)"); str->append(stateStr(md.highlights));
        str->append(L"Горячие клавиши (hotkeys)"); str->append(stateStr(md.hotkeys));
        str->append(L"Группы (groups)"); str->append(stateStr(md.groups));
        str->append(L"Переменные (vars)"); str->append(stateStr(md.variables));
        str->append(L"Таймеры (timers)"); str->append(stateStr(md.timers));
        str->append(L"Подстановки (tabs)"); str->append(stateStr(md.tabwords));
        removeLastRN(str);
    }

    bool setMode(const tstring& state, const tstring& mode_value)
    {
        int stateid = recognizeState(state);
        if (stateid == -1)
            return false;

        int newstate = recognizeValue(mode_value);
        if (newstate == -1)
        {
            if (stateid == LogicHelper::UPDATE_ALL)
                return false;
            int curstate = getState(stateid);
            newstate = curstate ? 0 : 1;
        }
        if (stateid == LogicHelper::UPDATE_ALL)
        {
            propData->messages.initDefault(newstate);
            return true;
        }
        setState(stateid, newstate);
        return true;
    }

    void getStateString(const tstring& state, tstring *str)
    {
        static const tchar* cmds[] = { L"Триггеры (actions)", L"Макросы (aliases)", L"Замены (subs)", L"Aнтизамены (antisubs)", L"Фильтры (gags)",
            L"Подсветки (highlights)", L"Горячие клавиши (hotkeys)", L"Группы (groups)", L"Переменные (vars)", L"Таймеры (timers)", L"Подстановки (tabs)" };
        static const int ids[] = { LogicHelper::UPDATE_ACTIONS, LogicHelper::UPDATE_ALIASES, LogicHelper::UPDATE_SUBS,
            LogicHelper::UPDATE_ANTISUBS, LogicHelper::UPDATE_GAGS, LogicHelper::UPDATE_HIGHLIGHTS, LogicHelper::UPDATE_HOTKEYS,
            LogicHelper::UPDATE_GROUPS, LogicHelper::UPDATE_VARS, LogicHelper::UPDATE_TIMERS, LogicHelper::UPDATE_TABS, 0 };        
        int stateid = recognizeState(state);
        if (stateid == LogicHelper::UPDATE_ALL)
        {
            str->assign(L"Все элементы (all)");
            str->append(stateStrEx(getState(LogicHelper::UPDATE_ACTIONS)));
            removeLastRN(str);
            return;
        }

        for (int i = 0; ids[i]; ++i)
        {
            if (stateid == ids[i])
            { 
                str->assign(cmds[i]);
                str->append(stateStrEx(getState(stateid)));
                removeLastRN(str);
                break;
            }
        }
    }

private:
    const tchar* stateStr(int state)
    {
        return (state ? L" - Вкл \r\n" : L"\r\n"); // L" - Выкл \r\n");
    }

    const tchar* stateStrEx(int state)
    {
        return (state ? L" - Вкл \r\n" : L" - Выкл \r\n");
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

    void removeLastRN(tstring *str)
    {
        int len = str->length();
        if (len > 0)
        {
            tstring tmp(str->substr(0, len - 2));
            str->swap(tmp);
        }
    }
};
