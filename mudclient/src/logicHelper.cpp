﻿#include "stdafx.h"
#include "accessors.h"
#include "mudViewParser.h"
#include "logicHelper.h"

LogicHelper::LogicHelper()
{
     m_if_regexp.setRegExp(L"^([^ =~!<>]*) *(=|==|!=|~=|<>|<|>|<=|>=) *([^ =~!<>]*)$", true);     
     m_math_regexp.setRegExp(L"^([^+/*%]*) *([+-/*%]) *([^+/*%]*)$", true);
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

bool LogicHelper::processActions(parseData *parse_data, int index, InputCommands *newcmds, LogicTriggered* triggered)
{
    int j = index; int je = parse_data->strings.size()-1;
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped) return false;
        bool incomplstr = (!s->prompt && j==je && !parse_data->last_finished);
        bool processed = false;
        for (int i=0, e=m_actions.size(); i<e; ++i)
        {
           CompareData cd(s);
           if (m_actions[i]->processing(cd, incomplstr, newcmds))
           {
               if (triggered) {
                   triggered->push_back(m_actions[i]->getKey());
               }
               processed = true;
           }
           if (s->dropped) break;
        }
        return processed;
    }
}

void LogicHelper::processSubs(parseData *parse_data, LogicTriggered* triggered)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped || s->subs_processed) continue;
        bool incomplstr = (!s->prompt && j==je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_subs.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_subs[i]->processing(cd))
            {
                if (triggered) {
                   triggered->push_back(m_subs[i]->getKey());
                }
                cd.reinit();
                s->subs_processed = true;
            }
        }
    }
}

void LogicHelper::processAntiSubs(parseData *parse_data, LogicTriggered* triggered)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped || s->subs_processed) continue;
        bool incomplstr = (!s->prompt && j == je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_antisubs.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_antisubs[i]->processing(cd))
            {
                if (triggered) {
                   triggered->push_back(m_antisubs[i]->getKey());
                }
                cd.reinit();
            }
        }
    }
}

void LogicHelper::processGags(parseData *parse_data, LogicTriggered* triggered)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        MudViewString *s = parse_data->strings[j];
        if (s->dropped || s->subs_processed) continue;
        bool incomplstr = (!s->prompt && j == je && !parse_data->last_finished);
        if (incomplstr) continue;
        for (int i=0,e=m_gags.size(); i<e; ++i)
        {
            CompareData cd(s);
            while (m_gags[i]->processing(cd))
            {
                if (triggered) {
                   triggered->push_back(m_gags[i]->getKey());
                }
                if (s->dropped) break;
                cd.fullinit();
            }
        }
    }
}

void LogicHelper::processHighlights(parseData *parse_data, LogicTriggered* triggered)
{
    for (int j=0,je=parse_data->strings.size()-1; j<=je; ++j)
    {
        for (int i=0,e=m_highlights.size(); i<e; ++i)
        {
            MudViewString *s = parse_data->strings[j];
            if (s->dropped) continue;
            CompareData cd(s);
            while (m_highlights[i]->processing(cd))
            {
                if (triggered) {
                   triggered->push_back(m_highlights[i]->getKey());
                }
                cd.reinit();  // restart highlight
            }
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

int LogicHelper::getLeftTime(const tstring& timer_id)
{
    int count = m_timers.size();
    for (int i=0;i<count;++i)
    {
        Timer *t = m_timers[i];
        if (t->id == timer_id)
            return t->left();
    }
    return 0;
}

bool LogicHelper::upTimer(const tstring& timer_id)
{
    int count = m_timers.size();
    for (int i=0;i<count;++i)
    {
        Timer *t = m_timers[i];
        if (t->id == timer_id)
        {
           t->reset();
           return true;
        }
    }
    return false;
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
    {
        tstring p(expr);
        if (tortilla::getVars()->processVarsStrong(&p, true))
        {
            result->assign(p);
            return LogicHelper::MATH_SUCCESS;
        }
        return LogicHelper::MATH_ERROR;
    }

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

     if ( op == L"%" && p1.empty() && isInt( p2 ) )
     {
         result->clear();
         int len = p2.length();
         const tchar* p = p2.c_str();
         if (*p == L'-') { p++; len--; }

         int first = len % 3;
         result->append(p, first);
         p += first;
         
         for (int count = len/3; count > 0; count--)
         {
             if ( !result->empty() )
                 result->append(L" ");
             result->append( p, 3 );
             p += 3;
         }
         
         p = p2.c_str();
         if ( *p == L'-' ) 
             result->insert(0, L"-");

         return LogicHelper::MATH_SUCCESS;
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
    std::set<tstring> active_groups;
    for (int i=0,e=pdata->groups.size(); i<e; i++)
    {
        const property_value& v = pdata->groups.get(i);
        if (v.value == L"1")
            active_groups.insert(v.key);
    }

    InputTemplateParameters p;
    p.separator = pdata->cmd_separator;
    p.prefix = pdata->cmd_prefix;

    const int last = UPDATE_TIMER1+TIMERS_COUNT;
    if (what >= UPDATE_TIMER1 && what < last)
    {
        int id = what-UPDATE_TIMER1+1;
        m_timers.update(id, &pdata->timers, active_groups, p);
        return;
    }

    bool update_groups = (what == UPDATE_GROUPS);
    if (update_groups)
        what = UPDATE_ALL;

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
    {
        if (!update_groups)
            m_timers.init(&pdata->timers, active_groups, p);
        else
            m_timers.updateall(&pdata->timers, active_groups, p);
    }
}
