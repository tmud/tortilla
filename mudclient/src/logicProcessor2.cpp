﻿// In that file - Code for scripting
#include "stdafx.h"
#include "accessors.h"
#include "logicProcessor.h"
#include "inputProcessor.h"
#include "helpManager.h"
#include "passwordDlg.h"

//parse any variations {'" or space:
//#cmd par1 par2 par3
//#cmd {par1} {par2} {par3} -> legacy from jmc3
//#cmd 'par1' 'par2' 'par3'
//#cmd "par1" "par2" "par3"
class parser
{
public:
    parser(InputCommand pcmd, tstring *error_out) : cmd(pcmd) { perror = error_out; }
    int size() const { return cmd->parameters_list.size(); }
    const tstring& at(int index) const { return cmd->parameters_list[index]; }
    int spacesBefore(int index) const { int count = cmd->parameters_spacesbefore.size();
        return (index >= 0 && index < count) ? cmd->parameters_spacesbefore[index] : 0; }
    const tchar* c_str(int index) const { return cmd->parameters_list[index].c_str(); }
    const tstring& params() const { return cmd->parameters; }
    bool isInteger(int index) const { const tstring& p = cmd->parameters_list[index]; return isInt(p); }
    int toInteger(int index) const { const tstring& p = cmd->parameters_list[index]; return _wtoi(p.c_str()); }
    bool isNumber(int index) const { const tstring& p = cmd->parameters_list[index]; return isItNumber(p); }
    double toNumber(int index) const { const tstring& p = cmd->parameters_list[index]; double v = 0; w2double(p, &v); return v; }
    void invalidargs() { error(L"Некорректный набор параметров."); }
    void invalidoperation() { error(L"Невозможно вычислить."); }
    void invalidvars() { error(L"Используются неизвестные переменные."); }
    void blockedbyprops() { error(L"Команда не выполнена (открыто окно настроек)."); }
    bool isError() const { return perror->empty() ? false : true; }
private:
    void error(const tstring& errmsg) { perror->assign(errmsg); }
    InputCommand cmd;
    tstring *perror;
};
//-------------------------------------------------------------------
#include "logicScripts.h"
ParamsBuffer pb;
LogicProcessor* g_lprocessor = NULL;
#define IMPL(fn) void fn(parser *p) { g_lprocessor->impl_##fn(p); } void LogicProcessor::impl_##fn(parser* p)
//------------------------------------------------------------------
void LogicProcessor::recognizeSystemCommand(tstring* cmd, tstring* error)
{
   tstring& main_cmd(*cmd);
   typedef std::map<tstring, syscmd_fun>::iterator iterator;
   typedef std::vector<tstring>::iterator piterator;
   iterator it = m_syscmds.find(main_cmd);
   iterator it_end = m_syscmds.end();
   if (it == it_end)
   {
       // команда в системных не найдена - ищем в плагинах
       piterator p_end = m_plugins_cmds.end();
       piterator p = std::find(m_plugins_cmds.begin(), p_end, main_cmd);
       if (p != p_end)
           main_cmd.assign(*p);
       else
       {
           //пробуем подобрать по сокращенному имени
           int len = main_cmd.size();
           if (len < 3)
               error->append(L"Короткое имя для команды");
           else
           {
               std::vector<tstring> cmds;
               for (it = m_syscmds.begin(); it != it_end; ++it)
               {
                   if (!it->first.compare(0, len, main_cmd))
                       cmds.push_back(it->first);
               }
               for (p = m_plugins_cmds.begin(); p != p_end; ++p)
               {
                   if (!p->compare(0, len, main_cmd))
                       cmds.push_back(*p);
               }
               int count = cmds.size();
               if (count == 1)
                   main_cmd.assign(cmds[0]);
               else if (count != 0)    // count = 0 в словаре команды нет - все равно пробуем пройти через syscmd
               {
                   error->append(L"Уточните команду (варианты): ");
                   for (int i = 0; i < count; ++i) { if (i != 0) error->append(L", "); error->append(cmds[i]); }
               }
           }
       }
   }
}

void LogicProcessor::processSystemCommand(InputCommand cmd)
{
    tstring error, main_cmd(cmd->command);
    recognizeSystemCommand(&main_cmd, &error);

    PropertiesData *pdata = tortilla::getProperties();
    tchar prefix[2] = { pdata->cmd_prefix, 0 };
    bool hide_cmd = (!main_cmd.compare(L"hide")) ? true : false;

    bool unknown_cmd = false;
    bool fullcmd_loged = false;
    if (error.empty())
    {
        cmd->command.assign(main_cmd);

        tstring fullcmd(prefix);
        fullcmd.append(main_cmd);
        if (!hide_cmd)
            fullcmd.append(cmd->parameters);

        syscmdLog(fullcmd);
        fullcmd_loged = true;

        if (!hide_cmd)
            m_pHost->preprocessCommand(cmd);

        if (cmd->dropped)
        {
            tstring tmp(L"-");
            tmp.append(fullcmd);
            syscmdLog(tmp);
            return;
        }

        if (cmd->changed && cmd->command.empty())
            return;

        if (cmd->changed)
        {
            tstring tmp(L">");
            tmp.assign(prefix);
            tmp.append(cmd->command);
            tmp.append(L" ");
            tmp.append(cmd->parameters);
            syscmdLog(tmp);
            main_cmd.assign(cmd->command);
            recognizeSystemCommand(&main_cmd, &error);
            cmd->command.assign(main_cmd);
        }

        if (error.empty()) 
        {
            typedef std::map<tstring, syscmd_fun>::iterator iterator;
            iterator it_end = m_syscmds.end();
            iterator it = m_syscmds.find(main_cmd);
            if (it != it_end)
            {
                parser p(cmd, &error);
                it->second(&p);
            }
            else
            {
                typedef std::vector<tstring>::iterator piterator;
                piterator p_end = m_plugins_cmds.end();
                piterator p = std::find(m_plugins_cmds.begin(), p_end, main_cmd);
                if (p == p_end)
                {
                    unknown_cmd = true;
                    error.append(L"Неизвестная команда");
                }
            }
        }
    }

    if (!error.empty())
    {
        if (!fullcmd_loged)
        {
            tstring fullcmd;
            fullcmd.append(cmd->srccmd);
            if (!hide_cmd)
                fullcmd.append(cmd->srcparameters);
            syscmdLog(fullcmd);
        }

        if (unknown_cmd && pdata->unknown_cmd && !cmd->user) {}
        else {
            tstring msg(L"Ошибка: ");
            msg.append(error);
            msg.append(L" [");
            bool usesrc = (cmd->alias.empty() && !cmd->changed ) ? true : false;
            if (cmd->changed) msg.append(prefix);
            if (!usesrc && cmd->system) msg.append(prefix);
            msg.append(usesrc ? cmd->srccmd : cmd->command);
            if (!hide_cmd)
                msg.append(usesrc ? cmd->srcparameters : cmd->parameters);
            msg.append(L"]");
            if (!cmd->alias.empty())
            {
                msg.append(L", макрос: ");
                msg.append(cmd->alias);
            }
            tmcLog(msg);
        }
    }
}

void LogicProcessor::processGameCommand(InputCommand cmd)
{
    tstring br(L"\r\n");
    tstring tmp(cmd->command);
    tmp.append(cmd->parameters);
    tmp.append(br);

    int flags = SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS|GAME_CMD;
    if (tortilla::getProperties()->newline_commands && !cmd->srccmd.empty())
    {
        flags |= NEW_LINE;
    }

    processIncoming(tmp.c_str(), tmp.length(), flags, 0);

    m_pHost->preprocessCommand(cmd);
    if (cmd->dropped)
    {
        tstring tmp(L"-");
        tmp.append(cmd->srccmd);
        tmp.append(cmd->srcparameters);
        syscmdLog(tmp);  // шлем через метод (отключается как вывод системых команд)
        return;
    }
    if (cmd->changed && cmd->command.empty())
        return;
    if (cmd->changed)
    {
        tmp.assign(L">");
        tmp.append(cmd->command);
        tmp.append(L" ");
        tmp.append(cmd->parameters);
        tmp.append(br);
        processIncoming(tmp.c_str(), tmp.length(), flags, 0);
        tmp.erase(tmp.begin());
    }
    sendToNetwork(tmp);
}

void LogicProcessor::updateProps(int update, int options)
{
    if (update)
    {
        m_helper.updateProps(options);
        if (!m_updatelog.empty())
            tmcLog(m_updatelog);
    }
    m_updatelog.clear();
}
//------------------------------------------------------------------
bool LogicProcessor::setComponent(const tstring& name, bool mode)
{
    ModeCmdHelper ch(tortilla::getProperties());
    bool result = ch.setMode(name, mode);
    if (result) {
       tstring str;
       ch.getStateString(name, &str);
       tmcLog(str);
       tortilla::getPluginsManager()->processPluginsMethod("compsupdated", 0);
    }
    return result;
}
//------------------------------------------------------------------
IMPL(drop)
{
    if (p->size() != 0)
        p->invalidargs();
}
IMPL(stop)
{
    if (p->size() != 0)
        p->invalidargs();
}
//------------------------------------------------------------------
class ElementsHelper : public MethodsHelper
{
    LogicProcessorMethods *m_pMethods;
    int  m_type;
    bool m_skip;
    bool canDoIt() {
        if (!m_skip && m_pMethods->checkActiveObjectsLog(m_type)) return true;
        return m_skip;
    }
public:
    ElementsHelper(LogicProcessorMethods *methods, int type) : m_pMethods(methods), m_type(type), m_skip(false) {}
    void tmcLog(const tstring& msg) { if (canDoIt()) m_pMethods->tmcLog(msg); }
    void simpleLog(const tstring& msg) { if (canDoIt()) m_pMethods->simpleLog(msg); }
    void updateLog(const tstring& msg) { if (canDoIt()) m_pMethods->updateLog(msg); }
    void skipCheckMode() { m_skip = true; }
    operator MethodsHelper*() { return this; }
};

IMPL(action) 
{
    if (tortilla::isPropertiesOpen()) 
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ACTIONS);
    int update = script.process(p, &pdata->actions, &pdata->groups, 
            L"Триггеры(actions)", L"Триггеры", L"action", &ph);
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(unaction)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ACTIONS);
    int update = script.process(p, &pdata->actions, L"Удаление триггера", L"action", &ph);
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(alias)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ALIASES);
    AliasTestControl control;
    int update = script.process(p, &pdata->aliases, &pdata->groups,
        L"Макросы(aliases)", L"Макросы", L"alias", &ph, &control);
    updateProps(update, LogicHelper::UPDATE_ALIASES);
}

IMPL(unalias)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ALIASES);
    int update = script.process(p, &pdata->aliases, L"Удаление макроса", L"alias", &ph);
    updateProps(update, LogicHelper::UPDATE_ALIASES);
}

IMPL(sub)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_SUBS);
    int update = script.process(p, &pdata->subs, &pdata->groups,
        L"Замены(subs)", L"Замены", L"sub", &ph);
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(unsub)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_SUBS);
    int update = script.process(p, &pdata->subs, L"Удаление замены", L"sub", &ph);
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(hotkey)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HOTKEYS);
    HotkeyTestControl control;
    int update = script.process(p, &pdata->hotkeys, &pdata->groups,
        L"Горячие клавиши(hotkeys)", L"Горячие клавиши", L"hotkey", &ph,
        &control);
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(unhotkey)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HOTKEYS);
    int update = script.process(p, &pdata->hotkeys, L"Удаление горячей клавиши", L"hotkey", &ph);
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(highlight)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HIGHLIGHTS);
    HighlightTestControl control;
    int update = script.process(p, &pdata->highlights, &pdata->groups,
        L"Подсветки(highlights)", L"Подсветка", L"highlight", &ph,
        &control);
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(unhighlight)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HIGHLIGHTS);
    int update = script.process(p, &pdata->highlights, L"Удаление подсветки", L"highlight", &ph);
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(gag)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_GAGS);
    int update = script.process(p, &pdata->gags, &pdata->groups, L"Фильтры (gags)", L"gag", &ph);
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(ungag)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_GAGS);
    int update = script.process(p, &pdata->gags, L"Удаление фильтра", L"gag", &ph);
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(antisub)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    AddParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_ANTISUBS);
    int update = script.process(p, &pdata->antisubs, &pdata->groups, L"Антизамены (antisubs)", L"antisub", &ph);
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
}

IMPL(unantisub)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    DeleteParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_ANTISUBS);
    int update = script.process(p, &pdata->antisubs, L"Удаление антизамены", L"antisub", &ph);
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
}

IMPL(math)
{
    int n = p->size();
    if (p->size() == 2)
    {
        VarProcessor *vp = tortilla::getVars();
        ElementsHelper ph(this, LogicHelper::UPDATE_VARS);
        if (!vp->canSetVar(p->at(0)))
        {
            swprintf(pb.buffer, pb.buffer_len, L"Переменную $%s изменить невозможно", p->c_str(0));
            ph.tmcLog(pb.buffer);
            return;
        }

        tstring result;
        LogicHelper::MathResult r = m_helper.mathOp(p->at(1), &result);
        if (r == LogicHelper::MATH_ERROR)
            { p->invalidoperation(); return; }
        else if (r == LogicHelper::MATH_VARNOTEXIST)
            { p->invalidvars(); return; }

        if (vp->setVar(p->at(0), result))
            swprintf(pb.buffer, pb.buffer_len, L"$%s='%s'", p->c_str(0), result.c_str());
        else
            swprintf(pb.buffer, pb.buffer_len, L"Недопустимое имя переменной: $%s", p->c_str(0));
        ph.tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}

IMPL(strop)
{
    int n = p->size();
    if (n >= 3)
    {
        VarProcessor *vp = tortilla::getVars();
        ElementsHelper ph(this, LogicHelper::UPDATE_VARS);
        if (!vp->canSetVar(p->at(0)))
        {
            swprintf(pb.buffer, pb.buffer_len, L"Переменную $%s изменить невозможно", p->c_str(0));
            ph.tmcLog(pb.buffer);
            return;
        }
        tstring result;
        tstring op(p->at(2));
        if (op == L"concat") 
        {
            if (p->size() == 4) {
                result.assign(p->at(1));
                result.append(p->at(3));
            } else {
                p->invalidargs();
            }
        }
        else if (op == L"trim") 
        {
            if (p->size() == 3) {
                result.assign(p->at(1));
                tstring_trim(&result);
            } else {
                p->invalidargs();
            }
        }
        else if (op == L"replace")
        {
            if (p->size() == 5)
            {
                result.assign(p->at(1));
                tstring_replace(&result, p->at(3), p->at(4));
            } else {
                p->invalidargs();
            }
        } else {
            p->invalidoperation();
        }
        if (!p->isError())
        {
            if (vp->setVar(p->at(0), result))
                swprintf(pb.buffer, pb.buffer_len, L"$%s='%s'", p->c_str(0), result.c_str());
            else
                swprintf(pb.buffer, pb.buffer_len, L"Недопустимое имя переменной: $%s", p->c_str(0));
            ph.tmcLog(pb.buffer);
        }
        return;
    }
    p->invalidargs();
}

IMPL(var)
{
    ElementsHelper ph(this, LogicHelper::UPDATE_VARS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 0 || n == 1)
    {
        helper->skipCheckMode();
        bool found = false;
        if (n == 0)
            helper->tmcLog(L"Переменные(vars):");
        else
        {
            swprintf(pb.buffer, pb.buffer_len, L"Переменные с '%s':", p->c_str(0));
            helper->tmcLog(pb.buffer);
        }

        PropertiesData *pdata = tortilla::getProperties();
        int size = pdata->variables.size();
        for (int i=0; i<size; ++i)
        {
            const property_value& v = pdata->variables.get(i);
            if (n == 1 && v.key.find(p->at(0)) == -1)
                continue;
            swprintf(pb.buffer, pb.buffer_len, L"$%s='%s'", v.key.c_str(), v.value.c_str());
            helper->simpleLog(pb.buffer);
            found = true;
        }
        if (!found)
            helper->tmcLog(L"Список пуст");
        return;
    }

    if (n == 2)
    {
        VarProcessor *vp = tortilla::getVars();
        if (!vp->canSetVar(p->at(0)))
            swprintf(pb.buffer, pb.buffer_len, L"Переменную $%s изменить невозможно", p->c_str(0));
        else
        {
            if (vp->setVar(p->at(0), p->at(1)))
                swprintf(pb.buffer, pb.buffer_len, L"$%s='%s'", p->c_str(0), p->c_str(1));
            else
                swprintf(pb.buffer, pb.buffer_len, L"Недопустимое имя переменной: $%s", p->c_str(0));
        }
        helper->tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}

IMPL(unvar)
{
    ElementsHelper ph(this, LogicHelper::UPDATE_VARS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 1)
    {
        VarProcessor *vp = tortilla::getVars();
        if (!vp->canSetVar(p->at(0)))
        {
            swprintf(pb.buffer, pb.buffer_len, L"Переменную $%s удалить невозможно.", p->c_str(0));
            helper->tmcLog(pb.buffer);
            return;
        }
        if (!vp->delVar(p->at(0)))
            swprintf(pb.buffer, pb.buffer_len, L"Переменная $%s не существует.", p->c_str(0));
		else
        	swprintf(pb.buffer, pb.buffer_len, L"Переменная $%s удалена.", p->c_str(0));
        helper->tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}
//----------------------------------------------------------------------------------
class GroupCollector
{   tstring m_group;
public:
    GroupCollector(const tstring& group) : m_group(group) {}
    int count(const PropertiesValues& v)
    {
        int count = 0;
        for (int i=0,e=v.size(); i<e; ++i)
        {
            if (v.get(i).group == m_group)
                count++;
        }
        return count;
    }
};

IMPL(group)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    int n = p->size();
    if (n > 2)
        return p->invalidargs();

    PropertiesData* pdata = tortilla::getProperties();
    ElementsHelper ph(this, LogicHelper::UPDATE_GROUPS);
    MethodsHelper* helper = ph;
    if (n == 0)
    {
        helper->skipCheckMode();
        helper->tmcLog(L"Группы:");
        for (int i=0,e=pdata->groups.size(); i<e; ++i)
        {
            const property_value &v = pdata->groups.get(i);
            tstring value = (v.value == L"1") ? L"Вкл" : L"";
            swprintf(pb.buffer, pb.buffer_len, L"%s %s", v.key.c_str(), value.c_str());
            helper->simpleLog(pb.buffer);
        }
        return;
    }

    int index = -1;
    tstring group( (n==1) ? p->at(0) : p->at(1) );
    for (int i=0,e=pdata->groups.size(); i<e; ++i)
    {
        const property_value &v = pdata->groups.get(i);
        if (v.key == group) { index = i; break; }
    }
    if (index == -1)
    {
        swprintf(pb.buffer, pb.buffer_len, L"Группа {%s} не существует.", group.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }

    tstring op; 
    if (n == 2) op = p->at(0);
    if (n == 1 || (op == L"info" || op == L"инф" || op == L"list" || op == L"список"))
    {
        helper->skipCheckMode();
        swprintf(pb.buffer, pb.buffer_len, L"Группа {%s}:", group.c_str());
        helper->tmcLog(pb.buffer);
        GroupCollector gc(group);
        int aliases = gc.count(pdata->aliases);
        int actions = gc.count(pdata->actions);
        int highlights = gc.count(pdata->highlights);
        int hotkeys = gc.count(pdata->hotkeys);
        int subs = gc.count(pdata->subs);
        int gags = gc.count(pdata->gags);
        int antisubs = gc.count(pdata->antisubs);
        swprintf(pb.buffer, pb.buffer_len, L"Макросы(aliases): %d\nТриггеры(actions): %d\nПодсветки(highlights): %d\nГорячие клавиши(hotkeys): %d\nЗамены(subs): %d\nФильтры(gags): %d\nАнтизамены(antisubs): %d",
            aliases, actions, highlights, hotkeys, subs, gags, antisubs);
        helper->simpleLog(pb.buffer);
        return;
    }

    if (op == L"вкл" || op == L"enable" || op == L"on" || op == L"1")
    {
        property_value &v = pdata->groups.getw(index);
        v.value = L"1";
        updateProps(1, LogicHelper::UPDATE_GROUPS);
        swprintf(pb.buffer, pb.buffer_len, L"Группа {%s} включена.", v.key.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }

    if (op == L"выкл" || op == L"disable" || op == L"off" || op == L"0")
    {
        property_value &v = pdata->groups.getw(index);
        v.value = L"0";
        updateProps(1, LogicHelper::UPDATE_GROUPS);
        swprintf(pb.buffer, pb.buffer_len, L"Группа {%s} выключена.", v.key.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }

    swprintf(pb.buffer, pb.buffer_len, L"Неизвестная операция %s.", op.c_str());
    helper->tmcLog(pb.buffer);
    return;
}

IMPL(clear)
{
    int n = p->size();
    if (n == 0)
    {
        m_pHost->clearText(0);
        return;
    }
    if (n == 1 && p->isInteger(0))
    {
        int window = p->toInteger(0);
        if (window < 0 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 0, window);
        m_pHost->clearText(window);
        return;
    }
    p->invalidargs();
}

IMPL(connect)
{
    int n = p->size();
    if (n == 2 && p->isInteger(1))
    {
        if (m_connected)
        {
            tmcLog(L"Уже есть подключение! Нужно сначала отключиться от текущего сервера.");
            return;
        }
        if (m_connecting)
        {
            tmcLog(L"Подключение уже устанавливается...");
            return;
        }
        swprintf(pb.buffer, pb.buffer_len, L"Подключение '%s:%d'...", p->at(0).c_str(), p->toInteger(1));
        tmcLog(pb.buffer);
        m_connecting = true;
        m_pHost->connectToNetwork( p->at(0), p->toInteger(1) );
        return;
    }
    p->invalidargs();
}

IMPL(cr)
{
    int n = p->size();
    if (n == 0)
    {
        tstring msg(L"\r\n");
        processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS, 0);
        WCHAR br[2] = { 10, 0 };
        tstring cmd(br);
        sendToNetwork(cmd);
        return;
    }
    p->invalidargs();
}

IMPL(help)
{
    int n = p->size();
    if (n == 0)
    {
        openHelp(m_pHost->getMainWindow(), L"");
        return;
    }
    if (n == 1)
    {
        PropertiesData *pdata = tortilla::getProperties();
        tstring param(p->at(0));
        if (param.at(0) == pdata->cmd_prefix)
            param = param.substr(1);
        openHelp(m_pHost->getMainWindow(), param);
        return;
    }
    p->invalidargs();
}

IMPL(password)
{
    int n = p->size();
    if (n == 0 || n == 1)
    {
        tstring pass;
        PasswordDlg dlg(n == 0 ? L"" : p->at(0));
        if (dlg.DoModal() == IDOK)
            pass = dlg.getPassword();
        if (!pass.empty())
        {
            tstring msg(L"*****\r\n");
            processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS|NEW_LINE, 0);
            WCHAR br[2] = { 10, 0 };
            pass.append(br);
            sendToNetwork(pass);
        }
        return;
    }
    p->invalidargs();
}

IMPL(hide)
{
    if (p->size() != 0)
    {
        tstring msg(L"*****\r\n");
        processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS|NEW_LINE, 0);
        WCHAR br[2] = { 10, 0 };
        tstring cmd;
        for (int i=0,e=p->size(); i<e; ++i)
        {
            if (i != 0)
                cmd.append(L" ");
            cmd.append(p->at(i));
        }
        cmd.append(br);
        sendToNetwork(cmd);
        return;
    }
    p->invalidargs();
}

IMPL(ifop)
{
    if (p->size() == 2)
    {
        LogicHelper::IfResult result = m_helper.compareIF(p->at(0));
        if (result == LogicHelper::IF_SUCCESS)
        {
            tstring cmd(p->at(1));
            processCommand(cmd);
        }
        else if (result == LogicHelper::IF_ERROR)
            p->invalidoperation();
        return;
    }
    if (p->size() == 3) // if-else
    {
        LogicHelper::IfResult result = m_helper.compareIF(p->at(0));
        if (result == LogicHelper::IF_SUCCESS)
        {
            tstring cmd(p->at(1));
            processCommand(cmd);
        }
        else if (result == LogicHelper::IF_FAIL)
        {
            tstring cmd(p->at(2));
            processCommand(cmd);
        }
        else if (result == LogicHelper::IF_ERROR)
            p->invalidoperation();
        return;
    }
    p->invalidargs();
}

IMPL(disconnect)
{
    if (p->size() == 0)
    {
        if (!m_connected && !m_connecting)
        {
            tmcLog(L"Нет подключения.");
            return;
        }
        processNetworkError(L"Соединение завершено.");
        m_pHost->disconnectFromNetwork();
        return;
    }
    p->invalidargs();
}

IMPL(mccp)
{
    if (p->size() == 0)
    {
        MccpStatus status;
        m_pHost->getMccpStatus(&status);
        if (!status.status)
        {
            tmcLog(L"Сжатие трафика не работает.");
            return;
        }

        float d = (float)status.game_data_len;
        float c = (float)status.network_data_len;
        float ratio = 0; 
        if (d > 0)
            ratio = 100 - ((c / d) * 100);
        swprintf(pb.buffer, pb.buffer_len, L"Трафик: %.2f Кб, Игровые данные: %.2f Кб, Сжатие: %.2f%%", c/1024, d/1024, ratio);
        tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}
//------------------------------------------------------------------- 
void LogicProcessor::wlogf_main(int log, const tstring& file, bool newlog)
{
    int id = m_wlogs[log];
    if (id != -1)
    {
         tstring oldfile(m_logs.getLogFile(id));
         if (log == 0)
            swprintf(pb.buffer, pb.buffer_len, L"Лог закрыт: '%s'.", oldfile.c_str());
         else
             swprintf(pb.buffer, pb.buffer_len, L"Лог в окне %d закрыт: '%s'.", log, oldfile.c_str());
         tmcLog(pb.buffer);
         m_wlogs[log] = -1;
         m_logs.closeLog(id);
         if (file.empty())
             return;
    }
    else
    {
        if (file.empty())
        {
            if (log == 0)
                swprintf(pb.buffer, pb.buffer_len, L"Лог открыт не был.");
            else
                swprintf(pb.buffer, pb.buffer_len, L"Лог в окне %d открыт не был.", log);
            tmcLog(pb.buffer);
            return; 
        }
    }

    tstring logfile(file);
    id = m_logs.openLog(logfile, newlog, 0);
    if (id == -1)
    {
        if (log == 0)
            swprintf(pb.buffer, pb.buffer_len, L"Ошибка! Лог открыть не удалось: '%s'.", logfile.c_str());
        else
            swprintf(pb.buffer, pb.buffer_len, L"Ошибка! Лог в окне %d открыть не удалось: '%s'.", log, logfile.c_str());
    }
    else
    {
        if (log == 0)
            swprintf(pb.buffer, pb.buffer_len, L"Лог открыт: '%s'.", logfile.c_str());
        else
            swprintf(pb.buffer, pb.buffer_len, L"Лог в окне %d открыт: '%s'.", log, logfile.c_str());
        m_wlogs[log] = id;
    }
    tmcLog(pb.buffer);
}

void LogicProcessor::logf(parser *p, bool newlog)
{
    int n = p->size();
    if (n == 0)  { wlogf_main(0, L"", newlog); return; }
    if (n == 1)  { wlogf_main(0, p->at(0), newlog); return; }
    p->invalidargs();
}

IMPL(logs)
{
    logf(p, false);
}

IMPL(logn)
{
    logf(p, true);
}

void LogicProcessor::wlogf(parser *p, bool newlog)
{
    int n = p->size();
    if (n == 1 || n == 2)
    {
        int log = p->toInteger(0);
        if (log < 0 || log > OUTPUT_WINDOWS)
            return invalidwindow(p, 0, log);
        wlogf_main(log, (n==1) ? L"" : p->at(1), newlog);
        return;
    }
    p->invalidargs();
}

IMPL(wlog)
{
    wlogf(p, false);
}

IMPL(wlogn)
{
    wlogf(p, true);
}

void LogicProcessor::clogf_main(const tstring& file, bool newlog)
{
    if (m_clog != -1)
    {
        tstring oldfile(m_logs.getLogFile(m_clog));
        swprintf(pb.buffer, pb.buffer_len, L"Лог закрыт: '%s'.", oldfile.c_str());
        tmcLog(pb.buffer);
        m_logs.closeLog(m_clog);
        m_clog = -1;
        if (file.empty())
            return;
    }
    else
    {
        if (file.empty())
        {
            swprintf(pb.buffer, pb.buffer_len, L"Лог открыт не был.");
            tmcLog(pb.buffer);
            return;
        }
    }

    tstring logfile(file);
    int id = m_logs.openLog(logfile, newlog, 1);
    if (id == -1)
    {
        swprintf(pb.buffer, pb.buffer_len, L"Ошибка! Лог открыть не удалось: '%s'.", logfile.c_str());
    }
    else
    {
        swprintf(pb.buffer, pb.buffer_len, L"Лог открыт: '%s'.", logfile.c_str());
        m_clog = id;
    }
    tmcLog(pb.buffer);
}

void LogicProcessor::clogf(parser *p, bool newlog)
{
    int n = p->size();
    if (n == 0)  { clogf_main(L"", newlog); return; }
    if (n == 1)  { clogf_main(p->at(0), newlog); return; }
    p->invalidargs();
}

IMPL(clog)
{
    clogf(p, false);
}

IMPL(clogn)
{
    clogf(p, true);
}

IMPL(wname)
{
    int n = p->size();
    if (n == 2)
    {
        int window = p->toInteger(0);
        if (window < 1 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 1, window);
        m_pHost->setWindowName(window, p->at(1));
        return;
    }
    p->invalidargs();
}

IMPL(plugin)
{
    int n = p->size();
    if (n == 2)
    {
        if (tortilla::getPluginsManager()->setPluginState(p->at(0), p->at(1)))
            return;
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
void LogicProcessor::invalidwindow(parser *p, int view0, int view)
{
    swprintf(pb.buffer, pb.buffer_len, L"Недопустимый номер окна: %d (корректные значения: %d-%d)", view, view0, OUTPUT_WINDOWS);
    tmcLog(pb.buffer);
    p->invalidargs();
}

IMPL(wshow)
{
    if (p->size() == 1)
    {
        if (p->isInteger(0))
        {
            int window = p->toInteger(0);
            if (window < 1 || window > OUTPUT_WINDOWS)
                return invalidwindow(p, 1, window);
            m_pHost->showWindow(window, true);
            return;
        }
        swprintf(pb.buffer, pb.buffer_len, L"Некорректный параметр: '%s'.", p->c_str(0));
        tmcLog(pb.buffer);
    }
    p->invalidargs();
}

IMPL(whide)
{
    if (p->size() == 1)
    {
        if (p->isInteger(0))
        {
            int window = p->toInteger(0);
            if (window < 1 || window > OUTPUT_WINDOWS)
                return invalidwindow(p, 1, window);
            m_pHost->showWindow(window, false);
            return;
        }
        swprintf(pb.buffer, pb.buffer_len, L"Некорректный параметр: '%s'.", p->c_str(0));
        tmcLog(pb.buffer);
    }
    p->invalidargs();
}

void LogicProcessor::printex(int view, const std::vector<tstring>& params, bool enable_actions_subs)
{
    parseData data;
    MudViewString *new_string = new MudViewString();
    MudViewStringBlock block;
    HighlightTestControl tc;

    bool last_color_teg = false;
    for (int i=0,e=params.size(); i<e; ++i)
    {
        tstring p(params[i]);
        if (!last_color_teg && tc.checkText(&p))    // it is color teg
        {
            last_color_teg = true;
            PropertiesHighlight hl;
            hl.convertFromString(p);
            MudViewStringParams &p = block.params;
            p.ext_text_color = hl.textcolor;
            p.ext_bkg_color = hl.bkgcolor;
            p.blink_status = hl.border;
            p.italic_status = hl.italic;
            p.underline_status = hl.underlined;
            p.use_ext_colors = 1;
            continue;
        }
        if (last_color_teg || new_string->blocks.empty())
        {
            last_color_teg = false;
            block.string.assign(p);
            new_string->blocks.push_back(block);
        }
        else
        {
            int last = new_string->blocks.size() - 1;
            tstring &s = new_string->blocks[last].string;
            s.append(L" ");
            s.append(p);
        }
    }

    new_string->system = true;
    data.strings.push_back(new_string);
    int flags = GAME_LOG|WORK_OFFLINE;
    if (!enable_actions_subs)
        flags |= (SKIP_ACTIONS|SKIP_SUBS);
    printIncoming(data, flags, view);
}

void LogicProcessor::printex(int view, const parser* p, int from, bool enable_actions_subs)
{
      std::vector<tstring> params;
      int n = p->size();
      params.resize(n);
      for (int i=from; i<n; ++i)
      {
          int spaces = p->spacesBefore(i);
          bool notfirstparam = (from != 0 && from == i && spaces > 0);
          if (spaces > 1 || notfirstparam) {
              tstring tmp(spaces-1, L' ');
              tmp.append(p->at(i));
              params[i].assign(tmp);
          } else {
              params[i].assign(p->at(i));
          }
      }
      int last = p->spacesBefore(n);
      if (last > 0) {
          tstring tmp(last-1, L' ');
          params.push_back(tmp);
      }
      if (from != 0)
          params.erase(params.begin(), params.begin() + from);
      printex(view, params, enable_actions_subs);
}

IMPL(wprint)
{
    int n = p->size();
    if (n > 0 && p->isInteger(0))
    {
        int window = p->toInteger(0);
        if (window < 0 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 0, window);
        return printex(window, p, 1, false);
    }
    p->invalidargs();
}

IMPL(print)
{
    int n = p->size();
    if (n >= 0)
    {
        return printex(0, p, 0, true);
    }
    p->invalidargs();
}

IMPL(tab)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    ElementsHelper ph(this, LogicHelper::UPDATE_TABS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 0)
    {
        helper->skipCheckMode();
        helper->tmcLog(L"Автоподстановки(tabs):");
        int size = pdata->tabwords.size();
        for (int i=0; i<size; ++i)
        {
            const tstring &v = pdata->tabwords.get(i);
            helper->simpleLog(v.c_str());
        }
        if (size == 0)
            helper->tmcLog(L"Список пуст.");
        return;
    }

    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = pdata->tabwords.find(tab);
        if (index == -1)
            pdata->tabwords.add(index, tab);
        swprintf(pb.buffer, pb.buffer_len, L"+tab {%s}", tab.c_str());        
        helper->tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}

IMPL(untab)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    ElementsHelper ph(this, LogicHelper::UPDATE_TABS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = pdata->tabwords.find(tab);
        if (index == -1) {
            helper->tmcLog(L"Варианты не найдены.");
            return;
        }
        else
        {
            pdata->tabwords.del(index);
            swprintf(pb.buffer, pb.buffer_len, L"-tab {%s}", tab.c_str());
        }
        helper->tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}

IMPL(timer)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    PropertiesData *pdata = tortilla::getProperties();
    ElementsHelper ph(this, LogicHelper::UPDATE_TIMERS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 0)
    {
        helper->skipCheckMode();
        const PropertiesValues &t  = pdata->timers;
        if (t.size() == 0)
        {
            helper->tmcLog(L"Не создано ни одного таймера.");
            return;
        }
        if (!pdata->timers_on)
            helper->tmcLog(L"Таймеры (выключены):");
        else
            helper->tmcLog(L"Таймеры:");

        std::map<tstring, int> sorted_map;
        for (int i=0,e=t.size(); i<e; ++i)
        {
            const property_value &v = t.get(i);
            sorted_map[v.key] = i;
        }

        std::map<tstring, int>::iterator it = sorted_map.begin(), it_end = sorted_map.end();
        for (; it!=it_end; ++it)
        {
            int i = it->second;
            const property_value &v = t.get(i);
            PropertiesTimer pt; pt.convertFromString(v.value);
            if (!pdata->timers_on)
            {
                swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: {%s} [%s]", v.key.c_str(), pt.timer.c_str(), 
                  pt.cmd.c_str(), v.group.c_str());
            }
            else
            {
                int left = m_helper.getLeftTime(v.key);
                double dleft = static_cast<double>(left); dleft /= 1000.0f;
                swprintf(pb.buffer, pb.buffer_len, L"#%s %.1f/%s сек: {%s} [%s]", v.key.c_str(), dleft, pt.timer.c_str(),
                    pt.cmd.c_str(), v.group.c_str());
            }
            helper->simpleLog(pb.buffer);
        }
        return;
    }

    if (n == 1 && !p->isInteger(0))
    {
        helper->skipCheckMode();
        tstring op(p->at(0));
        if (op == L"disable" || op == L"off" || op == L"выкл") {
            pdata->timers_on = 0;
            helper->tmcLog(L"Таймеры выключены.");
            return;
        }
        if (op == L"enable" || op == L"on" || op == L"вкл") {
            if (!m_connected)
            {
                helper->tmcLog(L"Нет подключения.");
                return;
            }
            m_helper.resetTimers();
            pdata->timers_on = 1;
            helper->tmcLog(L"Таймеры включены.");
            return;
        }
    }

    if (n == 1 && p->isInteger(0))
    {
        helper->skipCheckMode();
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        tchar tmp[16];
        _itow(key, tmp, 10);
        tstring id(tmp);

        int index = pdata->timers.find(id);
        if (index == -1)
        {
            swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s не используется.", id.c_str());
            helper->tmcLog(pb.buffer);
            return;
        }
        const PropertiesValues &t  = pdata->timers;
        const property_value &v = t.get(index);
        PropertiesTimer pt; pt.convertFromString(v.value);

        if (!pdata->timers_on)
        {
            swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: {%s} [%s]", v.key.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), v.group.c_str());
        }
        else
        {
            int left = m_helper.getLeftTime(v.key);
            double dleft = static_cast<double>(left); dleft /= 1000.0f;
            swprintf(pb.buffer, pb.buffer_len, L"#%s %.1f/%s сек: {%s} [%s]", v.key.c_str(), dleft, pt.timer.c_str(),
                pt.cmd.c_str(), v.group.c_str());
        }
        helper->simpleLog(pb.buffer);
        return;
    }

    if (n >= 2  && n <= 4 && p->isInteger(0) && p->isNumber(1))
    {
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        double delay = p->toNumber(1);
        if (delay < 0 || delay > 9999.9f)
            return p->invalidargs();

         tchar tmp[16];
        _itow(key, tmp, 10);
        tstring id(tmp);
        int index = pdata->timers.find(id);
        if (index == -1 && n == 2)
        {
            swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s не cоздан.", id.c_str());
            helper->skipCheckMode();
            helper->tmcLog(pb.buffer);
            return;
        }

        PropertiesTimer pt;
        pt.setTimer(delay);
        if (n > 2)
           pt.cmd = p->at(2);

        tstring group;
        PropertiesValues* groups = &pdata->groups;
        if (n > 3)
        {
           group.assign(p->at(3));
           if (!groups->exist(group))
               groups->add(-1, group, L"1", L"");
        }
        else
        {
            group.assign(groups->get(0).key);
        }

        if (index != -1)
        {
            property_value& v = pdata->timers.getw(index);
            if (n < 4)
                group = v.group;
            PropertiesTimer tmp;
            tmp.convertFromString(v.value);
            if (n == 2)
               pt.cmd = tmp.cmd;
        }

        tstring value;
        pt.convertToString(&value);
        pdata->timers.add(index, id, value, group);
        swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: {%s} [%s]", id.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), group.c_str());
        helper->simpleLog(pb.buffer);

        int timer_id = LogicHelper::UPDATE_TIMER1+key-1;
        return updateProps(1, timer_id);
    }
    p->invalidargs();
}

IMPL(untimer)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    int n = p->size();
    if (n == 1 && p->isInteger(0))
    {
        ElementsHelper ph(this, LogicHelper::UPDATE_TIMERS);
        MethodsHelper* helper = ph;
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        tchar tmp[16];
        _itow(key, tmp, 10);
        tstring id(tmp);

        PropertiesData *pdata = tortilla::getProperties();

        int index = pdata->timers.find(id);
        if (index == -1)
        {
            swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s не используется.", id.c_str());
            helper->tmcLog(pb.buffer);
            return;
        }
        pdata->timers.del(index);
        swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s удален.", id.c_str());
        helper->tmcLog(pb.buffer);

        int timer_id = LogicHelper::UPDATE_TIMER1+key-1;
        return updateProps(1, timer_id);
    }
    p->invalidargs();
}

IMPL(uptimer)
{
    if (tortilla::isPropertiesOpen())
        return p->blockedbyprops();
    int n = p->size();
    if (n == 1 && p->isInteger(0))
    {
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        tchar tmp[16];
        _itow(key, tmp, 10);
        tstring id(tmp);

        if (m_helper.upTimer(id))
        {
            ElementsHelper ph(this, LogicHelper::UPDATE_TIMERS);
            MethodsHelper* helper = ph;
            swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s перезапущен.", id.c_str());
            helper->tmcLog(pb.buffer);
            return;
        }
    }
    p->invalidargs();
}

IMPL(hidewindow)
{
    if (p->size() == 0)
    {
        CWindow w(m_pHost->getMainWindow());
        w.ShowWindow(SW_MINIMIZE);
        return;
    }
    p->invalidargs();
}

IMPL(showwindow)
{
    if (p->size() == 0)
    {
        CWindow w(m_pHost->getMainWindow());
        w.ShowWindow(SW_RESTORE);
        SetForegroundWindow(w);
        return;
    }
    p->invalidargs();
}

IMPL(message)
{
    PropertiesData *pdata = tortilla::getProperties();
    int n = p->size();
    if (n == 0)
    {
        MessageCmdHelper mh(pdata);
        tstring str;
        mh.getStrings(&str);
        tmcLog(L"Эхо-уведомления:");
        simpleLog(str);
        return;
    }
    if (n == 1)
    {
        tstring str;
        MessageCmdHelper mh(pdata);
        if (!mh.getStateString(p->at(0), &str))
        {
            p->invalidargs();
        }
        else
        {
            tmcLog(str);
        }
        return;
    }
    if (n == 2)
    {
        MessageCmdHelper mh(pdata);
        if (!mh.setMode(p->at(0), p->at(1)))
        {
            p->invalidargs();
        }
        else
        {
            tstring str;
            mh.getStateString(p->at(0), &str);
            tmcLog(str);
        }
        return;
    }
    p->invalidargs();
}

IMPL(debug_tr)
{
    PropertiesData *pdata = tortilla::getProperties();
    int n = p->size();
    if (n == 0)
    {
        DebugCmdHelper mh(pdata);
        tstring str;
        mh.getStrings(&str);
        tmcLog(L"Debug-уведомления:");
        simpleLog(str);
        return;
    }

    if (n == 1)
    {
        DebugCmdHelper mh(pdata);
        tstring str;
        mh.getStateString(p->at(0), &str);
        tmcLog(str);
        return;       
    }
        
    if (n == 2)
    {
        DebugCmdHelper mh(pdata);
        if (!mh.setMode(p->at(0), p->at(1)))
        {
            p->invalidargs();
        }
        else
        {
            tstring str;
            mh.getStateString(p->at(0), &str);
            tmcLog(str);
        }
        return;
    }
    p->invalidargs();
}

IMPL(wlock)
{
    int n = p->size();
    if (n == 1 && p->isInteger(0))
    {
        int window = p->toInteger(0);
        if (window < 1 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 1, window);
        m_pHost->lockWindow(window, true);
        swprintf(pb.buffer, pb.buffer_len, L"В окне #%d автоскролл отключен.", window);
        tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}

IMPL(wunlock)
{
    int n = p->size();
    if (n == 1 && p->isInteger(0))
    {
        int window = p->toInteger(0);
        if (window < 1 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 1, window);
        m_pHost->lockWindow(window, false);
        swprintf(pb.buffer, pb.buffer_len, L"В окне #%d автоскролл включен.", window);
        tmcLog(pb.buffer);
        return;
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
IMPL(component)
{
    int n = p->size();
    if (n == 0)
    {
        PropertiesData *pdata = tortilla::getProperties();
        ModeCmdHelper mh(pdata);
        tstring str;
        mh.getStrings(&str);
        tmcLog(L"Компоненты:");
        simpleLog(str);
        return;
    }

    if (n == 1 || n == 2)
    {
        PropertiesData *pdata = tortilla::getProperties();
        ModeCmdHelper mh(pdata);
        if (!mh.setMode(p->at(0), n == 2 ? p->at(1) : L""))
        {
            p->invalidargs();
        }
        else
        {
            tstring str;
            mh.getStateString(p->at(0), &str);
            tmcLog(str);
            tortilla::getPluginsManager()->processPluginsMethod("compsupdated", 0);
        }
        return;
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
IMPL(wait)
{
     if (p->size() == 2 && isOnlySymbols(p->at(0), L"0123456789."))
     {
         double delay = 0;
         w2double(p->at(0), &delay);
         if (delay > 0)
         {
             delay = delay * 1000;
             int delay_ms = static_cast<int>(delay);
             m_waitcmds.add(delay_ms, p->at(1));
             return;
         }
     }
     p->invalidargs();
}
//-------------------------------------------------------------------
IMPL(load)
{
    int n = p->size();
    if (n == 1 || n == 2)
    {
        tstring profile(p->at(0));
        tstring group;
        if (n==2) group.assign(p->at(1));
        tstring error;
        m_pHost->loadProfile(profile, group, &error);
        if (!error.empty())
            tmcLog(error);
        return;
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
IMPL(savelog)
{
    int n = p->size();
    if (n == 1 || n == 2)
    {
        tstring window( (n==1) ? L"0" : p->at(0));
        tstring filename( (n==1) ? p->at(0) : p->at(1));
        int v = 0;
        if (w2int(window, &v) && v >= 0 && v < OUTPUT_WINDOWS)
        {
            if (!m_pHost->saveViewData(v, filename))
            {
                tstring error(L"Ошибка при сохранении лог-файла: ");
                error.append(filename);
                tmcLog(error);
            }
            else
            {
                tstring msg(L"Лог ");
                msg.append(filename);
                msg.append(L" сохранен.");
                tmcLog(msg);
            }
            return;
        }
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
IMPL(none)
{
}
//-------------------------------------------------------------------
void LogicProcessor::regCommand(const char* name, syscmd_fun f, bool skip_autoset)
{    
    AnsiToWide a2w(name);
    tstring cmd(a2w);
    m_syscmds[cmd] = f;
    if (!skip_autoset) {
        PropertiesData *pdata = tortilla::getProperties();
        PropertiesList &p = pdata->tabwords_commands;
        p.add(-1, cmd);
    }
}

bool LogicProcessor::init()
{
    g_lprocessor = this;

    m_univ_prompt_pcre.setRegExp(L"(?:[0-9]+[HMVXCжэбом] +){2,}.*[СЮЗВПОv^()]*>", true);

    if (!m_logs.init())
        return false;

    regCommand("help", help);
    regCommand("clear", clear);
    regCommand("connect", connect);
    regCommand("cr", cr);
    regCommand("zap", disconnect);
    regCommand("disconnect", disconnect);

    regCommand("drop", drop, true);
    regCommand("stop", stop, true);

    regCommand("action", action);
    regCommand("unaction", unaction);
    regCommand("alias", alias);
    regCommand("unalias", unalias);
    regCommand("sub", sub);
    regCommand("unsub", unsub);
    regCommand("hotkey", hotkey);
    regCommand("unhotkey", unhotkey);
    regCommand("highlight", highlight);
    regCommand("unhighlight", unhighlight);
    regCommand("gag", gag);
    regCommand("ungag", ungag);
    regCommand("antisub", antisub);
    regCommand("unantisub", unantisub);
    regCommand("math", math);
    regCommand("var", var);
    regCommand("unvar", unvar);
    regCommand("wait", wait);

    regCommand("password", password);
    regCommand("hide", hide);
    regCommand("if", ifop);
    regCommand("group", group);

    regCommand("wshow", wshow);
    regCommand("whide", whide);
    regCommand("mccp", mccp);
    regCommand("woutput", wprint);
    regCommand("output", print);
    regCommand("message", message);
    regCommand("echo", message);
    regCommand("component", component);

    regCommand("tab", tab);
    regCommand("untab", untab);
    regCommand("timer", timer);
    regCommand("untimer", untimer);
    regCommand("uptimer", uptimer);

    regCommand("hidewindow", hidewindow);
    regCommand("showwindow", showwindow);
    regCommand("log", logs);
    regCommand("logn", logn);

    regCommand("wlog", wlog);
    regCommand("wlogn", wlogn);
    regCommand("clog", clog);
    regCommand("clogn", clogn);

    regCommand("wname", wname);

    regCommand("plugin", plugin);
    regCommand("load", load);

    regCommand("savelog", savelog);

    regCommand("none", none, true);

    regCommand("wlock", wlock);
    regCommand("wunlock", wunlock);

    regCommand("debug", debug_tr);
    regCommand("strop", strop);

    return true;
}
