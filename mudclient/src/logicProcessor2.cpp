// In that file - Code for scripting
#include "stdafx.h"
#include "logicProcessor.h"
#include "helpManager.h"

//parse any variations {'" or space:
//#cmd par1 par2 par3
//#cmd {par1} {par2} {par3} -> legacy from jmc3
//#cmd 'par1' 'par2' 'par3'
//#cmd "par1" "par2" "par3"
class parser
{
public:
    parser(const tstring&cmdname, tstring *error_out) { cmd = new InputCommand(cmdname); perror = error_out; }
    ~parser() { delete cmd; }
    int size() const { return cmd->parameters_list.size(); }
    const tstring& at(int index) const { return cmd->parameters_list[index]; }
    const tchar* c_str(int index) const { return cmd->parameters_list[index].c_str(); }
    const tstring& params() const { return cmd->parameters; }
    bool isInteger(int index) const { const tstring& p = cmd->parameters_list[index];
        return (wcsspn(p.c_str(), L"0123456789-") == p.length()) ? true : false; }
    int toInteger(int index) const { const tstring& p = cmd->parameters_list[index];
        return _wtoi(p.c_str()); }
    void error(const tstring& errmsg) { perror->assign(errmsg); }
    void invalidargs() { error(L"Неправильный набор параметров."); }
private:
    InputCommand *cmd;
    tstring *perror;
};
//-------------------------------------------------------------------
#include "logicScripts.h"
WCHAR buffer[256];
LogicProcessor* g_lprocessor = NULL;
#define IMPL(fn) void fn(parser *p) { g_lprocessor->impl_##fn(p); } void LogicProcessor::impl_##fn(parser* p)
tchar MethodsHelper::buffer[1024]; //logicScripts.h
//------------------------------------------------------------------
void LogicProcessor::processSystemCommand(const tstring& cmd)
{
    if (propData->show_system_commands)
        simpleLog(cmd);

    tstring scmd(cmd);
    tstring_trim(&scmd);

    tstring main_cmd;
    int pos = scmd.find(L' ');
    if (pos == -1)
        main_cmd.append(scmd.substr(1));
    else
        main_cmd.append(scmd.substr(1, pos-1));

    tstring params(scmd.substr(1));
    tstring error;
    typedef std::map<tstring, syscmd_fun>::iterator iterator;
    iterator it = m_syscmds.find(main_cmd);
    iterator it_end = m_syscmds.end();
    if (it != it_end)
    {
        parser p(params, &error);
        it->second(&p);
    }
    else
    {
        // пробуем подобрать по сокращенному имени
        int len = main_cmd.size();
        if (len >= 3)
        {
            std::vector<tstring> cmds;
            for (it = m_syscmds.begin(); it != it_end; ++it)
            {
                if (!it->first.compare(0, len, main_cmd))
                    cmds.push_back(it->first);
            }

            int count = cmds.size();
            if (count == 1)
            {
                it = m_syscmds.find(cmds[0]);
                parser p(params, &error);
                it->second(&p);
            }
            else if (count == 0)
            {
                error.append(L"Неизвестная команда");
            }
            else
            {
                error.append(L"Уточните команду (варианты): ");
                for (int i = 0; i < count; ++i) { if (i != 0) error.append(L", "); error.append(cmds[i]); }
            }
        }
        else
        {
            error.append(L"Неизвестная команда");
        }
    }

    if (!error.empty())
    {
        tstring msg(L"Ошибка: ");
        msg.append(error);
        msg.append(L" [");
        msg.append(scmd);
        msg.append(L"]");
        tmcLog(msg);
    }
}

void LogicProcessor::updateProps(int update, int options)
{
    if (update)
    {
        m_helper.updateProps(options);
        if (options == LogicHelper::UPDATE_ALIASES)
            m_input.updateProps(propData);
        if (!m_updatelog.empty())
            tmcLog(m_updatelog);
    }
    m_updatelog.clear();
}
//------------------------------------------------------------------
IMPL(drop)
{
    if (p->size() != 0)
        p->invalidargs();
}

IMPL(action) 
{
    AddParams3 script(this);
    int update = script.process(p, &propData->actions, &propData->groups, 
            L"Триггеры(actions)", L"Триггеры", L"Новый триггер");
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(unaction)
{
    DeleteParams3 script(this);
    int update = script.process(p, &propData->actions, L"Удаление триггера");
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(alias)
{
    AddParams3 script(this);
    int update = script.process(p, &propData->aliases, &propData->groups,
        L"Макросы(aliases)", L"Макросы", L"Новый макрос");
    updateProps(update, LogicHelper::UPDATE_ALIASES);
 }

IMPL(unalias)
{
    DeleteParams3 script(this);
    int update = script.process(p, &propData->aliases, L"Удаление команды");
    updateProps(update, LogicHelper::UPDATE_ALIASES);
}

IMPL(sub)
{
    AddParams3 script(this);
    int update = script.process(p, &propData->subs, &propData->groups,
        L"Замены(subs)", L"Замены", L"Новая замена");
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(unsub)
{
    DeleteParams3 script(this);
    int update = script.process(p, &propData->subs, L"Удаление замены");
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(hotkey)
{
    AddParams3 script(this);
    HotkeyTestControl control;
    int update = script.process(p, &propData->hotkeys, &propData->groups,
        L"Горячие клавиши(hotkeys)", L"Горячие клавиши", L"Новая горячая клавиша",
        &control);
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(unhotkey)
{
    DeleteParams3 script(this);
    int update = script.process(p, &propData->hotkeys, L"Удаление горячей клавиши");
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(highlight)
{
    AddParams3 script(this);
    HighlightTestControl control;
    int update = script.process(p, &propData->highlights, &propData->groups,
        L"Подсветки(highlights)", L"Подсветка", L"Новая подсветка",
        &control);
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(unhighlight)
{
    DeleteParams3 script(this);
    int update = script.process(p, &propData->highlights, L"Удаление подсветки");
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(gag)
{
    AddParams2 script(this);
    int update = script.process(p, &propData->gags, &propData->groups, L"Фильтры (gags)", L"Фильтр");
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(ungag)
{
    DeleteParams2 script(this);
    int update = script.process(p, &propData->gags, L"Удаление фильтра");
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(antisub)
{
    AddParams2 script(this);
    int update = script.process(p, &propData->antisubs, &propData->groups, L"Антизамены (antisubs)", L"Антизамена");
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
}

IMPL(unantisub)
{
    DeleteParams2 script(this);
    int update = script.process(p, &propData->antisubs, L"Удаление антизамены");
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
}

IMPL(var)
{
    int n = p->size();
    if (n == 0 || n == 1)
    {
        bool found = false;
        if (n == 0)
            tmcLog(L"Переменные(vars):");
        else
        {   
            swprintf(buffer, L"Переменные с '%s':", p->c_str(0));
            tmcLog(buffer);
        }

        int size = propData->variables.size();
        for (int i=0; i<size; ++i)
        {
            const property_value& v = propData->variables.get(i);
            if (n == 1 && v.key.find(p->at(0)) == -1)
                continue;
            swprintf(buffer, L"%s = '%s'", v.key.c_str(), v.value.c_str());
            simpleLog(buffer);
            found = true;
        }
        if (!found)
            tmcLog(L"Список пуст");
        return;
    }

    if (n == 2)
    {
        int index = propData->variables.find(p->at(0));
        propData->variables.add(index, p->at(0), p->at(1), L"");       
        swprintf(buffer, L"%s = '%s'", p->c_str(0), p->c_str(1));
        tmcSysLog(buffer);
        return;
    }
    p->invalidargs();
}

IMPL(unvar)
{
    int n = p->size();
    if (n == 1)
    {
        int index = propData->variables.find(p->at(0));
        if (index == -1)
        {
            swprintf(buffer, L"Переменная '%s' не существует.", p->c_str(0));
            tmcSysLog(buffer);
            return;
        }
        propData->variables.del(index);
        swprintf(buffer, L"Переменная '%s' удалена.", p->c_str(0));
        tmcSysLog(buffer);
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
    int n = p->size();
    if (n > 2)
        return p->invalidargs();

    if (n == 0)
    {
        tmcLog(L"Группы:");
        for (int i=0,e=propData->groups.size(); i<e; ++i)
        {
            const property_value &v = propData->groups.get(i);
            tstring value = (v.value == L"1") ? L"Вкл" : L"";
            swprintf(buffer, L"%s %s", v.key.c_str(), value.c_str());
            simpleLog(buffer);
        }
        return;
    }
   
    int index = -1;
    tstring group( (n==1) ? p->at(0) : p->at(1) );
    for (int i=0,e=propData->groups.size(); i<e; ++i)
    {
        const property_value &v = propData->groups.get(i);
        if (v.key == group) { index = i; break; }
    }
    if (index == -1)
    {
        swprintf(buffer, L"Группа '%s' не существует.", group.c_str());
        tmcLog(buffer);
        return;
    }

    tstring op; 
    if (n == 2) op = p->at(0);
    if (n == 1 || (op == L"info" || op == L"инф" || op == L"list" || op == L"список"))
    {
        swprintf(buffer, L"Группа '%s':", group.c_str());
        tmcLog(buffer);
        GroupCollector gc(group);
        int aliases = gc.count(propData->aliases);
        int actions = gc.count(propData->actions);
        int highlights = gc.count(propData->highlights);
        int hotkeys = gc.count(propData->hotkeys);
        int subs = gc.count(propData->subs);
        int gags = gc.count(propData->gags);
        int antisubs = gc.count(propData->antisubs);
        swprintf(buffer, L"Макросы(aliases): %d\nТриггеры(actions): %d\nПодсветки(highlights): %d\nГорячие клавиши(hotkeys): %d\nЗамены(subs): %d\nФильтры(gags): %d\nАнтизамены(antisubs): %d",
            aliases, actions, highlights, hotkeys, subs, gags, antisubs);
        simpleLog(buffer);
        return;
    }
    
    if (op == L"вкл" || op == L"enable" || op == L"on")
    {
        property_value &v = propData->groups.getw(index);
        v.value = L"1";
        updateProps();
        swprintf(buffer, L"Группа '%s' включена.", v.key.c_str());
        tmcLog(buffer);
        return;
    }

    if (op == L"выкл" || op == L"disable" || op == L"off")
    {
        property_value &v = propData->groups.getw(index);
        v.value = L"0";
        updateProps();
        swprintf(buffer, L"Группа '%s' выключена.", v.key.c_str());
        tmcLog(buffer);
        return;
    }   

    swprintf(buffer, L"Неизвестная операция '%s'.", op.c_str());
    tmcLog(buffer);
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
        tmcLog(L"Подключение...");
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
        processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS);
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
        tstring param(p->at(0));
        if (param.at(0) == propData->cmd_prefix)
            param = param.substr(1);
        openHelp(m_pHost->getMainWindow(), param);
        return;
    }
    p->invalidargs();
}

IMPL(hide)
{
    if (p->size() != 0)
    {
        tstring msg(L"*****\r\n");
        processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS);
        WCHAR br[2] = { 10, 0 };
        tstring cmd(p->params());
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
        IfProcessor::Result result = m_ifproc.compare(p->at(0), propData->variables);
        if (result == IfProcessor::IF_SUCCESS)
            processCommand(p->at(1));
        else if (result == IfProcessor::IF_ERROR)
            p->error(L"Неизвестное условие");
        return;
    }
    p->invalidargs();
}

IMPL(disconnect)
{
    if (p->size() == 0)
    {
        if (!m_connected)
        {
            tmcLog(L"Нет подключения.");
            return;
        }
        m_pHost->disconnectFromNetwork();
        tmcLog(L"Соединение завершено.");
        m_connected = false;
        return;
    }
    p->invalidargs();
}

IMPL(mccp)
{
    if (p->size() == 0)
    {
        int compressed = 0; int decompressed = 0;
        m_pHost->getNetworkRatio(&compressed, &decompressed);
        if (compressed == decompressed)
        {
            tmcLog(L"Сжатие трафика не работает.");
            return;
        }

        float d = (float)decompressed;
        float c = (float)compressed;
        float ratio = 0; 
        if (d > 0)
            ratio = 100 - ((c / d) * 100);        
        swprintf(buffer, L"Трафик: %.2f Кб, Игровые данные: %.2f Кб, Сжатие: %.2f%%", c/1024, d/1024, ratio);
        tmcLog(buffer);
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
         m_logs.closeLog(id);
         m_wlogs[log] = -1;
         if (log == 0)
            swprintf(buffer, L"Лог закрыт: '%s'.", oldfile.c_str());
         else
            swprintf(buffer, L"Лог в окне %d закрыт: '%s'.", log, oldfile.c_str());
         tmcLog(buffer);
         if (file.empty())
             return;
    }
    else
    {
        if (file.empty())
        {
            if (log == 0)
                swprintf(buffer, L"Лог не был открыт.");
            else
                swprintf(buffer, L"Лог в окне %d не был открыт.", log);
            tmcLog(buffer);
            return; 
        }
    }

    tstring logfile(file);
    int pos = logfile.rfind(L'.');
    if (pos == -1)
        logfile.append(L".html");
    else
    {
        tstring ext(logfile.substr(pos+1));
        if (ext != L"htm" && ext != L"html")
            logfile.append(L".html");
    }
    
    id = m_logs.openLog(logfile, newlog);
    if (id == -1)
    {
        if (log == 0)
            swprintf(buffer, L"Ошибка! Лог открыть не удалось: '%s'.", logfile.c_str());
        else
            swprintf(buffer, L"Ошибка! Лог в окне %d открыть не удалось: '%s'.", log, logfile.c_str());
    }
    else
    {
        if (log == 0)
            swprintf(buffer, L"Лог открыт: '%s'.",  logfile.c_str());
        else
            swprintf(buffer, L"Лог в окне %d открыт: '%s'.", log, logfile.c_str());
        m_wlogs[log] = id;
    }
    tmcLog(buffer);    
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
//-------------------------------------------------------------------
void LogicProcessor::invalidwindow(parser *p, int view0, int view)
{
    swprintf(buffer, L"Недопустимый индекс окна: %d (корректные значения: %d-%d)", view, view0, OUTPUT_WINDOWS);
    tmcLog(buffer);
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
        swprintf(buffer, L"Неверный параметр: '%s'.", p->c_str(0));
        tmcLog(buffer);
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
        swprintf(buffer, L"Неверный параметр: '%s'.", p->c_str(0));
        tmcLog(buffer);
    }
    p->invalidargs();
}

void LogicProcessor::printex(int view, const std::vector<tstring>& params)
{
    parseData data;
    MudViewString *new_string = new MudViewString();
    MudViewStringBlock block;
    HighlightTestControl tc;

    for (int i=0,e=params.size(); i<e; ++i)
    {
        tstring p(params[i]);
        if (tc.checkText(&p))       // it color teg
        {
            PropertiesHighlight hl;
            hl.convertFromString(p);
            MudViewStringParams &p = block.params;
            p.ext_text_color = hl.textcolor;
            p.ext_bkg_color = hl.bkgcolor;
            p.blink_status = hl.border;
            p.italic_status = hl.italic;
            p.underline_status = hl.underlined;
            p.use_ext_colors = 1;
        }
        else
        {
            block.string = params[i];
            new_string->blocks.push_back(block);
        }
    }

    data.strings.push_back(new_string);

    int log = m_wlogs[view];
    if (log != -1)
        m_logs.writeLog(log, data);

    m_pHost->postprocessText(view, &data);
    m_pHost->addText(view, &data);
}

IMPL(wprint)
{
    int n = p->size();
    if (n > 0 && p->isInteger(0))
    {
        int window = p->toInteger(0);
        if (window < 0 || window > OUTPUT_WINDOWS)
            return invalidwindow(p, 0, window);
        std::vector<tstring> data;
        for (int i=1; i<n; ++i)
            data.push_back(p->at(i));
        return printex(window, data);
    }
    p->invalidargs();
}

IMPL(print)
{
    int n = p->size();
    if (n > 0)
    {
        std::vector<tstring> data;
        for (int i=0; i<n; ++i)
            data.push_back(p->at(i));
        return printex(0, data);
    }
    p->invalidargs();
}

IMPL(tab)
{
    int n = p->size();
    if (n == 0)
    {
        tmcLog(L"Автоподстановки(tabs):");
        int size = propData->tabwords.size();
        for (int i=0; i<size; ++i)
        {
            const tstring &v = propData->tabwords.get(i);
            simpleLog(v.c_str());
        }
        if (size == 0)
            tmcLog(L"Список пуст.");
        return;
    }

    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = propData->tabwords.find(tab);
        if (index == -1)
            propData->tabwords.add(index, tab);
        swprintf(buffer, L"Автоподстановка '%s' добавлена.", tab.c_str());        
        tmcLog(buffer);
        return;
    }
    p->invalidargs();
}

IMPL(untab)
{
    int n = p->size();
    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = propData->tabwords.find(tab);
        if (index == -1)
            swprintf(buffer, L"Автоподстановки '%s' не существует.", tab.c_str());
        else
        {
            propData->tabwords.del(index);
            swprintf(buffer, L"Автоподстановкa '%s' удалена.", tab.c_str());
        }
        tmcLog(buffer);
        return;
    }
    p->invalidargs();
}

IMPL(timer)
{
    int n = p->size();
    if (n == 0)
    {
        tmcLog(L"Таймеры:");
        if (!propData->timers_on)
            simpleLog(L"Таймеры выключены.");

        const PropertiesValues &t  = propData->timers;
        if (t.size() == 0)
        {
            tmcLog(L"Список пуст.");
            return;
        }            
        for (int i=0,e=t.size(); i<e; ++i)
        {            
            const property_value &v = t.get(i);
            PropertiesTimer pt; pt.convertFromString(v.value);
            swprintf(buffer, L"#%s %s сек: '%s' '%s'", v.key.c_str(), pt.timer.c_str(), 
                pt.cmd.c_str(), v.group.c_str());
            simpleLog(buffer);
        }
        return;
    }

    if (n == 1 && !p->isInteger(0))
    {
        tstring op(p->at(0));
        if (op == L"disable" || op == L"off" || op == L"выкл") {
            propData->timers_on = 0;
            tmcLog(L"Таймеры выключены.");
            return;
        }
        if (op == L"enable" || op == L"on" || op == L"вкл") {
            if (!m_connected)
            {
                tmcLog(L"Нет подключения.");
                return;
            }
            m_helper.resetTimers();
            propData->timers_on = 1;
            tmcLog(L"Таймеры включены.");
            return;
        }
    }

    if (n == 1 && p->isInteger(0))
    {
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        _itow(key, buffer, 10);
        tstring id(buffer);
        
        int index = propData->timers.find(id);
        if (index == -1)
        {
            swprintf(buffer, L"Таймер #%s не используется.", id.c_str());
            tmcLog(buffer);
            return;
        }
        const PropertiesValues &t  = propData->timers;
        const property_value &v = t.get(index);
        PropertiesTimer pt; pt.convertFromString(v.value);
        swprintf(buffer, L"#%s %s сек: '%s' '%s'", v.key.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), v.group.c_str());
        simpleLog(buffer);
        return;
    }

    if (n >= 2  && n <= 4 && p->isInteger(0) && p->isInteger(1))
    {
        int key = p->toInteger(0);
        if (key < 1 || key > TIMERS_COUNT)
            return p->invalidargs();
        int delay = p->toInteger(1);
        if (delay < 0 || delay > 999)
            return p->invalidargs();

        _itow(key, buffer, 10);
        tstring id(buffer);
        int index = propData->timers.find(id);
        if (index == -1 && n == 2)
        {
            swprintf(buffer, L"Ошибка. Таймер #%s не существует.", id.c_str());
            tmcLog(buffer);
            return;
        }

        PropertiesTimer pt;
        _itow(delay, buffer, 10);
        pt.timer.assign(buffer);
        if (n > 2)
           pt.cmd = p->at(2);

        tstring group;
        PropertiesValues* groups = &propData->groups;
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
            property_value& v = propData->timers.getw(index);
            if (n < 4)
                group = v.group;
            PropertiesTimer tmp;
            tmp.convertFromString(v.value);
            if (n == 2)
               pt.cmd = tmp.cmd;
        }
        
        tstring value;
        pt.convertToString(&value);
        propData->timers.add(index, id, value, group);
        swprintf(buffer, L"#%s %s сек: '%s' '%s'", id.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), group.c_str());
        simpleLog(buffer);
        return updateProps(0, LogicHelper::UPDATE_TIMERS);
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
        return;
    }
    p->invalidargs();
}
//-------------------------------------------------------------------
void LogicProcessor::regCommand(const char* name, syscmd_fun f)
{
    AnsiToWide a2w(name);
    tstring cmd(a2w);
    m_syscmds[cmd] = f;
    PropertiesList &p = propData->tabwords_commands;    
    p.add(-1, cmd);
}

bool LogicProcessor::init()
{
    g_lprocessor = this;

    if (!m_logs.init())
        return false;

    regCommand("help", help);
    regCommand("clear", clear);
    regCommand("connect", connect);
    regCommand("cr", cr);
    regCommand("zap", disconnect);
    regCommand("disconnect", disconnect);

    regCommand("drop", drop);
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
    regCommand("var", var);
    regCommand("unvar", unvar);

    regCommand("hide", hide);        
    regCommand("if", ifop);
    regCommand("group", group);

    regCommand("wshow", wshow);
    regCommand("whide", whide);
    regCommand("mccp", mccp);
    regCommand("woutput", wprint);
    regCommand("output", print);

    regCommand("tab", tab);
    regCommand("untab", untab);
    regCommand("timer", timer);

    regCommand("hidewindow", hidewindow);
    regCommand("showwindow", showwindow);
    regCommand("log", logs);
    regCommand("logn", logn);

    regCommand("wlog", wlog);
    regCommand("wlogn", wlogn);
    regCommand("wname", wname);
    return true;
}
 