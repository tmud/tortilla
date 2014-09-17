#pragma once

#include "propertiesPages/propertiesData.h"
#include "logicElements.h"

template <class T>
class LogicWrapper : public std::vector<T*>
{
public:
    LogicWrapper() {}
    ~LogicWrapper() { clear(); }
    void clear() 
    {
        autodel<T> z(*this);
    }
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

class LogicWrapperTimers : public std::vector<Timer*>
{
public:
    LogicWrapperTimers() {}
    ~LogicWrapperTimers() 
    {
        autodel<Timer> z(*this);
    }
    void init(PropertiesValues *values, const std::vector<tstring>& active_groups)
    {
        std::vector<int> timers;
        for (int i=0,e=values->size(); i<e; ++i)
        {
            const property_value &v = values->get(i);
            if (!isOnlyDigits(v.key))
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
            t->init(v);
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
           UPDATE_GAGS, UPDATE_HIGHLIGHTS, UPDATE_TIMERS };

    LogicHelper(PropertiesData *propData);
    void updateProps(int what = UPDATE_ALL);
    bool processAliases(const tstring& key, tstring* newcmd);
    bool processHotkeys(const tstring& key, tstring* newcmd);
    void processActions(parseData *parse_data, std::vector<tstring>* new_cmds);
    void processSubs(parseData *parse_data);
    void processAntiSubs(parseData *parse_data);
    void processGags(parseData *parse_data);
    void processHighlights(parseData *parse_data);
    void processTimers(std::vector<tstring>* new_cmds);
    void resetTimers();
    void processVars(tstring *cmdline);

private:
    // current workable elements
    LogicWrapper<Alias> m_aliases;
    LogicWrapper<Hotkey> m_hotkeys;
    LogicWrapper<Action> m_actions;
    LogicWrapper<Sub> m_subs;
    LogicWrapper<AntiSub> m_antisubs;
    LogicWrapper<Gag> m_gags;
    LogicWrapper<Highlight> m_highlights;
    LogicWrapperTimers m_timers;
    DWORD m_ticker;
    Pcre16 m_vars_regexp;
    PropertiesData *m_propData;
};
