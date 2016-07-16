#include "stdafx.h"
#include "accessors.h"
#include "mudViewParser.h"
#include "logicHelper.h"

LogicHelper::LogicHelper()
{
     m_if_regexp.setRegExp(L"^([^ =~!<>]*) *(=|==|!=|~=|<>|<|>|<=|>=) *([^ =~!<>]*)$", true);     
     m_math_regexp.setRegExp(L"^([^+-/*]*) *([+-/*]) *([^+-/*]*)$", true);
     m_params_regexp.setRegExp(L"['{\"]?(.*['}\"]?[^'}\"])", true);
}

bool LogicHelper::processAliases(const InputCommand cmd, InputCommands* newcmds)
{
    for (int i=0,e=m_aliases.size(); i<e; ++i)
    {
        if (m_aliases[i]->processing(cmd, newcmds))
           return true;
    }
    return false;
}

bool LogicHelper::processHotkeys(const tstring& key, InputCommands* newcmds)
{
    for (int i=0,e=m_hotkeys.size(); i<e; ++i)
    {
        if (m_hotkeys[i]->processing(key, newcmds))
            return true;
    }
    return false;
}

bool LogicHelper::processActions(parseData *parse_data, int index, LogicPipelineElement *pe)
{
    int j = index; int je = parse_data->strings.size()-1;
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped) return false;

        bool incomplstr = (j==je && !parse_data->last_finished);
        bool processed = false;
        for (int i=0, e=m_actions.size(); i<e; ++i)
        {
           CompareData cd(s);
           if (m_actions[i]->processing(cd, incomplstr, &pe->commands))
              processed = true;
           if (s->dropped) break;
        }

        return processed;
    }
}

void LogicHelper::processSubs(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped) continue;
        bool incomplstr = (j==je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_subs.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_subs[i]->processing(cd))
                cd.reinit();
        }
    }
}

void LogicHelper::processAntiSubs(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped) continue;
        bool incomplstr = (j == je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_antisubs.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_antisubs[i]->processing(cd))
                cd.reinit();
        }
    }
}

void LogicHelper::processGags(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped) continue;
        bool incomplstr = (j == je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_gags.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_gags[i]->processing(cd))
            {
                if (s->dropped) break;
                cd.fullinit();
            }
        }
    }
}

void LogicHelper::processHighlights(parseData *parse_data)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        for (int i=0,e=m_highlights.size(); i<e; ++i)
        {
            MudViewString *s = parse_data->strings[j];
            if (s->dropped) continue;
            CompareData cd(s);
            while (m_highlights[i]->processing(cd))
                cd.reinit();  // restart highlight
        }
    }
}

void LogicHelper::processTimers(InputCommands* newcmds)
{
    int dt = m_ticker.getDiff();
    m_ticker.sync();
    for (int i=0,e=m_timers.size(); i<e; ++i)
    {
        if (m_timers[i]->tick(dt))
            m_timers[i]->makeCommands(newcmds);
    }
}

void LogicHelper::resetTimers()
{
    for (int i=0,e=m_timers.size(); i<e; ++i)
        m_timers[i]->reset();
    m_ticker.sync();
}

int LogicHelper::getLeftTime(int timer)
{
    int count = m_timers.size();
    return (timer >= 0 && timer < count) ? m_timers[timer]->left() : 0;
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

     // check brackets in parameters - get params without brackets
     if (!p1.empty()) {
     m_params_regexp.find(p1);
     if (m_params_regexp.getSize() != 2) 
         return LogicHelper::IF_ERROR;
     m_params_regexp.getString(1, &p1);
     tstring_trim(&p1);
     }

     if (!p2.empty()) {
     m_params_regexp.find(p2);
     if (m_params_regexp.getSize() != 2) 
         return LogicHelper::IF_ERROR;
     m_params_regexp.getString(1, &p2);
     tstring_trim(&p1);
     }

     if (tortilla::getVars()->processVarsStrong(&p1, true) && tortilla::getVars()->processVarsStrong(&p2, true))
     {
         if (isInt(p1) && isInt(p2))
         {
             int n1 = _wtoi(p1.c_str());
             int n2 = _wtoi(p2.c_str());
             if (n1 == n2 && (cond == L"=" || cond == L"==" || cond == L"<=" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (n1 < n2 && (cond == L"<" || cond == L"<="))
                 return LogicHelper::IF_SUCCESS;
             if (n1 > n2 && (cond == L">" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (n1 != n2 && (cond == L"!=" || cond == L"~=" || cond == L"<>"))
                 return LogicHelper::IF_SUCCESS;
          }
          else
          {
             int result = wcscmp(p1.c_str(), p2.c_str());
             if (result == 0 && (cond == L"=" || cond == L"==" || cond == L"<=" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (result < 0 && (cond == L"<" || cond == L"<="))
                 return LogicHelper::IF_SUCCESS;
             if (result > 0 && (cond == L">" || cond == L">="))
                 return LogicHelper::IF_SUCCESS;
             if (result != 0 && (cond == L"!=" || cond == L"~=" || cond == L"<>"))
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

      // check brackets in parameters - get params without brackets
     if (!p1.empty()) {
     m_params_regexp.find(p1);
     if (m_params_regexp.getSize() != 2) 
         return LogicHelper::MATH_ERROR;
     m_params_regexp.getString(1, &p1);
     tstring_trim(&p1);
     }

     if (!p2.empty()) {
     m_params_regexp.find(p2);
     if (m_params_regexp.getSize() != 2) 
         return LogicHelper::MATH_ERROR;
     m_params_regexp.getString(1, &p2);
     tstring_trim(&p2);
     }

     if (tortilla::getVars()->processVarsStrong(&p1, true) && tortilla::getVars()->processVarsStrong(&p2, true))
     {
         if (isInt(p1) && isInt(p2))
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

void LogicHelper::updateProps(int what)
{
    PropertiesData *pdata = tortilla::getProperties();
    std::vector<tstring> active_groups;
    for (int i=0,e=pdata->groups.size(); i<e; i++)
    {
        const property_value& v = pdata->groups.get(i);
        if (v.value == L"1")
            active_groups.push_back(v.key);
    }

    InputTemplateParameters p;
    p.separator = pdata->cmd_separator;
    p.prefix = pdata->cmd_prefix;

    if (what == UPDATE_ALL || what == UPDATE_ALIASES)
        m_aliases.init(p, &pdata->aliases, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ACTIONS)
        m_actions.init(p, &pdata->actions, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_SUBS)
        m_subs.init(&pdata->subs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HOTKEYS)
        m_hotkeys.init(p, &pdata->hotkeys, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_ANTISUBS)
        m_antisubs.init(&pdata->antisubs, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_GAGS)
        m_gags.init(&pdata->gags, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_HIGHLIGHTS)
        m_highlights.init(&pdata->highlights, active_groups);
    if (what == UPDATE_ALL || what == UPDATE_TIMERS)
        m_timers.init(&pdata->timers, active_groups, p);
}
