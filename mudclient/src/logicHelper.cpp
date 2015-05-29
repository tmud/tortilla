#include "stdafx.h"
#include "mudViewParser.h"
#include "logicHelper.h"

LogicHelper::LogicHelper(PropertiesData *propData) : m_propData(propData)
{
     m_if_regexp.setRegExp(L"^('.*'|\".*\"|{.*}|[^ =~!<>]+) *(=|!=|<|>|<=|>=) *('.*'|\".*\"|{.*}|[^ =~!<>]+)$", true);
     m_math_regexp.setRegExp(L"^('.*'|\".*\"|{.*}|[^ */+-]+) *([*/+-]) *('.*'|\".*\"|{.*}|[^ */+-]+)$", true);
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
        MudViewString *s = parse_data->strings[j];
        if (s->gamecmd || s->system) continue;
        CompareData cd(s);
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
        MudViewString *s = parse_data->strings[j];
        if (s->gamecmd || s->system) continue;
        CompareData cd(s);
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
        MudViewString *s = parse_data->strings[j];
        if (s->gamecmd || s->system) continue;
        CompareData cd(s);
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
        MudViewString *s = parse_data->strings[j];
        if (s->gamecmd || s->system) continue;
        CompareData cd(s);
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
    int dt = m_ticker.getDiff();
    m_ticker.sync();

    for (int i=0,e=m_timers.size(); i<e; ++i)
    {
        if (m_timers[i]->tick(dt))
        {
            tstring cmd(m_timers[i]->cmd);
            new_cmds->push_back(cmd);
        }
    }
}

void LogicHelper::resetTimers()
{
    for (int i=0,e=m_timers.size(); i<e; ++i)
        m_timers[i]->reset();
    m_ticker.sync();
}

bool LogicHelper::canSetVar(const tstring& var)
{
    return m_varproc.canset(var);
}

bool LogicHelper::getVar(const tstring& var, tstring *value)
{
    return m_varproc.getVar(m_propData->variables, var, value);
}

bool LogicHelper::setVar(const tstring& var, const tstring &value)
{
    return m_varproc.setVar(m_propData->variables, var, value);
}

bool LogicHelper::delVar(const tstring& var)
{
    return m_varproc.delVar(m_propData->variables, var);
}

LogicHelper::IfResult LogicHelper::compareIF(const tstring& param)
{
     m_if_regexp.find(param);
     if (m_if_regexp.getSize() != 4)
         return LogicHelper::IF_ERROR;

     tstring p1, p2, cond;
     m_if_regexp.getString(1, &p1);  //1st parameter
     m_if_regexp.getString(3, &p2);  //2nd parameter
     m_if_regexp.getString(2, &cond);//condition

     if (processVarsStrong(&p1) && processVarsStrong(&p2))
     {
         if (isOnlyDigits(p1) && isOnlyDigits(p2))
         {
             int n1 = _wtoi(p1.c_str());
             int n2 = _wtoi(p2.c_str());
             if (n1 == n2 && (cond == L"=" || cond == L"<=" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (n1 < n2 && (cond == L"<" || cond == L"<=" || cond == L"!="))
                 return LogicHelper::IF_SUCCESS;
             if (n1 > n2 && (cond == L">" || cond == L">=" || cond == L"!="))
                 return LogicHelper::IF_SUCCESS;
          }
          else
          {
             int result = wcscmp(p1.c_str(), p2.c_str());
             if (result == 0 && (cond == L"=" || cond == L"<=" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (result < 0 && (cond == L"<" || cond == L"<=" || cond == L"!="))
                 return LogicHelper::IF_SUCCESS;
             if (result > 0 && (cond == L">" || cond == L">=" || cond == L"!="))
                 return LogicHelper::IF_SUCCESS;
           }
     }
     return LogicHelper::IF_FAIL;
}

LogicHelper::MathResult LogicHelper::mathOp(const tstring& expr, tstring* result)
{
    m_math_regexp.find(expr);
    if (m_math_regexp.getSize() != 4)
         return LogicHelper::MATH_ERROR;

     tstring p1, p2, op;
     m_math_regexp.getString(1, &p1);  //1st parameter
     m_math_regexp.getString(3, &p2);  //2nd parameter
     m_math_regexp.getString(2, &op);  //operator

     if (processVarsStrong(&p1) && processVarsStrong(&p2))
     {
         if (isOnlyDigits(p1) && isOnlyDigits(p2))
         {
             int r = 0;
             int n1 = _wtoi(p1.c_str());
             int n2 = _wtoi(p2.c_str());
             if (op == L"*")
                 r = n1 * n2;
             else if (op == L"/")
                 r = (n2 != 0) ? n1 / n2 : 0;
             else if (op == L"+")
                 r = n1 + n2;
             else if (op == L"-")
                 r = n1 - n2;
             tchar buffer[16]; _itow(r, buffer, 10);
             result->assign(buffer);
             return LogicHelper::MATH_SUCCESS;
         }
         return LogicHelper::MATH_ERROR;
     }
     return LogicHelper::MATH_VARNOTEXIST;
}

bool LogicHelper::processVars(tstring *cmdline)
{
    return m_varproc.processVars(cmdline, m_propData->variables, false);
}

bool LogicHelper::processVarsStrong(tstring *cmdline)
{
    return m_varproc.processVars(cmdline, m_propData->variables, true);
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

    InputCommandParameters p;
    p.separator = m_propData->cmd_separator;
    p.prefix = m_propData->cmd_prefix;

    if (what == UPDATE_ALL || what == UPDATE_ALIASES)
        m_aliases.init(p, &m_propData->aliases, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ACTIONS)
        m_actions.init(p, &m_propData->actions, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_SUBS)
        m_subs.init(&m_propData->subs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HOTKEYS)
        m_hotkeys.init(p, &m_propData->hotkeys, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ANTISUBS)
        m_antisubs.init(&m_propData->antisubs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_GAGS)
        m_gags.init(&m_propData->gags, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HIGHLIGHTS)
        m_highlights.init(&m_propData->highlights, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_TIMERS)
        m_timers.init(&m_propData->timers, active_groups);
}
