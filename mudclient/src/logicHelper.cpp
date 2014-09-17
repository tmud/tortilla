#include "stdafx.h"
#include "mudViewParser.h"
#include "logicHelper.h"

LogicHelper::LogicHelper(PropertiesData *propData) : m_propData(propData)
{
    m_ticker = GetTickCount();
}

bool LogicHelper::processAliases(const tstring& key, tstring* newcmd)
{
    for (int i=0,e=m_aliases.size(); i<e; ++i)
    {
        if (m_aliases[i]->processing(key, newcmd))
        {
            processVars(newcmd);
            return true;
        }
    }
    return false;
}

bool LogicHelper::processHotkeys(const tstring& key, tstring* newcmd)
{
    for (int i=0,e=m_hotkeys.size(); i<e; ++i)
    {
        if (m_hotkeys[i]->processing(key, newcmd))
        {
            processVars(newcmd);
            return true;
        }
    }
    return false;
}

void LogicHelper::processActions(parseData *parse_data, std::vector<tstring>* new_cmds)
{
    for (int j=0,je=parse_data->strings.size(); j<je; ++j)
    {
        CompareData cd(parse_data->strings[j]);
        for (int i=0,e=m_actions.size(); i<e; ++i)
        {
            tstring newcmd;
            if (m_actions[i]->processing(cd, &newcmd))
            {
               processVars(&newcmd);
               new_cmds->push_back(newcmd);
               break;
            }
        }
    }
}

void LogicHelper::processSubs(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size(); j<je; ++j)
    {
        CompareData cd(parse_data->strings[j]);
        for (int i=0,e=m_subs.size(); i<e; ++i)
        {
            while (m_subs[i]->processing(cd))
                cd.reinit();
        }
        MudViewString *str = parse_data->strings[j];
        for (int i = 0, e = str->blocks.size(); i < e; ++i)
            processVars(&str->blocks[i].string);
    }
}

void LogicHelper::processAntiSubs(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size(); j<je; ++j)
    {
        CompareData cd(parse_data->strings[j]);
        for (int i=0,e=m_antisubs.size(); i<e; ++i)
        {
            while (m_antisubs[i]->processing(cd))
                cd.reinit();
        }
    }
}

void LogicHelper::processGags(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size(); j<je; ++j)
    {
        CompareData cd(parse_data->strings[j]);
        for (int i=0,e=m_gags.size(); i<e; ++i)
        {
            while (m_gags[i]->processing(cd))
                cd.reinit();
        }
    }
}

void LogicHelper::processHighlights(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size(); j<je; ++j)
    {
        CompareData cd(parse_data->strings[j]);
        for (int i=0,e=m_highlights.size(); i<e; ++i)
        {
            while (m_highlights[i]->processing(cd))
                cd.reinit();  // restart highlight
        }
    }
}

void LogicHelper::processTimers(std::vector<tstring>* new_cmds)
{
    DWORD diff = -1;
    DWORD tick = GetTickCount();
    if (tick >= m_ticker)
    {
        diff = tick - m_ticker;
    }
    else
    {   // overflow 49.7 days (MSDN GetTickCount)
        diff = diff - m_ticker;
        diff = diff + tick + 1;
    }
    m_ticker = tick;
    int dt = diff;

    for (int i=0,e=m_timers.size(); i<e; ++i)
    {
        if (m_timers[i]->tick(dt))
        {
            tstring cmd(m_timers[i]->cmd);
            processVars(&cmd);
            new_cmds->push_back(cmd);
        }
    }
}

void LogicHelper::resetTimers()
{
    for (int i=0,e=m_timers.size(); i<e; ++i)
        m_timers[i]->reset();
    m_ticker = GetTickCount();
}

void LogicHelper::processVars(tstring *cmdline)
{
    tstring &cmd = *cmdline;
    m_vars_regexp.findAllMatches(cmd);
    int vars = m_vars_regexp.getSize();
    if (!vars) return;

    tstring new_cmd_line(cmd.substr(0, m_vars_regexp.getFirst(0)));
    for (int i = 0,e=vars-1; i <= e; ++i)
    {
        tstring tmp;
        m_vars_regexp.getString(i, &tmp);
        int index = m_propData->variables.find( tmp.substr(1) );
        if (index != -1)
        {
            const tstring &var = m_propData->variables.get(index).value;
            new_cmd_line.append(var);
        }

        if (i < e)
        {
            int end = m_vars_regexp.getLast(i);
            int next = m_vars_regexp.getFirst(i+1);
            new_cmd_line.append(cmd.substr(end, next - end));
        }
        else
        {
            int end = m_vars_regexp.getLast(i);
            new_cmd_line.append(cmd.substr(end));
        }
    }
    cmdline->assign(new_cmd_line);
}

void LogicHelper::updateProps(int what)
{
    std::vector<tstring> active_groups;
    for (int i=0,e=m_propData->groups.size(); i<e; i++)
    {
        const property_value& v = m_propData->groups.get(i);
        if (v.value == L"1")
            active_groups.push_back(v.key);
    }

    if (what == UPDATE_ALL || what == UPDATE_ALIASES)
        m_aliases.init(&m_propData->aliases, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ACTIONS)
        m_actions.init(&m_propData->actions, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_SUBS)
        m_subs.init(&m_propData->subs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HOTKEYS)
        m_hotkeys.init(&m_propData->hotkeys, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ANTISUBS)
        m_antisubs.init(&m_propData->antisubs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_GAGS)
        m_gags.init(&m_propData->gags, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HIGHLIGHTS)
        m_highlights.init(&m_propData->highlights, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_TIMERS)
        m_timers.init(&m_propData->timers, active_groups);

    tchar separator[2] = { m_propData->cmd_separator, 0 };
    tstring regexp(L"\\$[^ ");
    regexp.append(separator);
    regexp.append(L"]+");
    m_vars_regexp.setRegExp(regexp, true);
}
