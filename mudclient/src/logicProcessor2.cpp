// In that file - Code for scripting
#include "stdafx.h"
#include "logicProcessor.h"
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
    parser(const tstring&cmdname, tstring *error_out) : cmd(cmdname) { perror = error_out; }
    int size() const { return cmd.parameters_list.size(); }
    const tstring& at(int index) const { return cmd.parameters_list[index]; }
    const tchar* c_str(int index) const { return cmd.parameters_list[index].c_str(); }
    const tstring& params() const { return cmd.parameters; }
    bool isInteger(int index) const { const tstring& p = cmd.parameters_list[index];
        return (wcsspn(p.c_str(), L"0123456789-") == p.length()) ? true : false; }
    int toInteger(int index) const { const tstring& p = cmd.parameters_list[index];
        return _wtoi(p.c_str()); }    
    void invalidargs() { error(L"Некорректный набор параметров."); }
    void invalidoperation() { error(L"Некорректная операция."); }
private:
    void error(const tstring& errmsg) { perror->assign(errmsg); }
    InputCommand cmd;
    tstring *perror;
};
//-------------------------------------------------------------------
#include "logicScripts.h"
ParamsBuffer pb;
tchar buffer[1024];
const int buffer_len = 1024;
LogicProcessor* g_lprocessor = NULL;
#define IMPL(fn) void fn(parser *p) { g_lprocessor->impl_##fn(p); } void LogicProcessor::impl_##fn(parser* p)
//------------------------------------------------------------------
void LogicProcessor::processSystemCommand(tstring& cmd)
{
    tstring scmd(cmd);
    tstring_trim(&scmd);

    tstring main_cmd;
    int pos = scmd.find(L' ');
    if (pos == -1)
        main_cmd.assign(scmd.substr(1));
    else
        main_cmd.assign(scmd.substr(1, pos - 1));

    typedef std::map<tstring, syscmd_fun>::iterator iterator;
    typedef std::vector<tstring>::iterator piterator;

    tchar prefix[2] = { propData->cmd_prefix, 0 };
    tstring fullcmd(prefix);
    tstring error;
    iterator it = m_syscmds.find(main_cmd);
    iterator it_end = m_syscmds.end();
    if (it != it_end)
        fullcmd.append(it->first);
    else
    {
        // команда в системных не найдена - ищем в плагинах
        piterator p_end = m_plugins_cmds.end();
        piterator p = std::find(m_plugins_cmds.begin(), p_end, main_cmd);
        if (p != p_end)
            fullcmd.append(*p);
        else
        {
            //пробуем подобрать по сокращенному имени
            int len = main_cmd.size();
            if (len < 3)
                error.append(L"Неизвестная команда");
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
                    fullcmd.append(cmds[0]);
                else if (count == 0)
                    fullcmd.append(main_cmd);       // в словаре команды нет - все равно пробуем пройти через syscmd
                else {
                    error.append(L"Уточните команду (варианты): ");
                    for (int i = 0; i < count; ++i) { if (i != 0) error.append(L", "); error.append(cmds[i]); }
                }
            }
        }
    }

    if (error.empty())
    {
        if (pos != -1)
            fullcmd.append(scmd.substr(pos));     // add params
        m_pHost->preprocessGameCmd(&fullcmd);
        tstring_trim(&fullcmd);
        if (fullcmd.empty())
        {
            syscmdLog(cmd);
            tmcLog(L"Команда заблокирована");
            return;
        }

        syscmdLog(fullcmd);

        pos = fullcmd.find(L' ');
        if (pos == -1)
            main_cmd.assign(fullcmd.substr(1));
        else
            main_cmd.assign(fullcmd.substr(1, pos - 1));
        it = m_syscmds.find(main_cmd);
        if (it != it_end)
        {
            parser p(fullcmd.substr(1), &error);
            it->second(&p);
        }
        else
        {
            piterator p_end = m_plugins_cmds.end();
            piterator p = std::find(m_plugins_cmds.begin(), p_end, main_cmd);
            if (p == p_end)
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

void LogicProcessor::processGameCommand(tstring& cmd)
{
    bool src_cmd_empty = cmd.empty();
    m_pHost->preprocessGameCmd(&cmd);
    if (cmd.empty() && !src_cmd_empty)
        return;
    WCHAR br[2] = { 10, 0 };
    cmd.append(br);
    processIncoming(cmd.c_str(), cmd.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_HIGHLIGHTS|GAME_CMD);
    sendToNetwork(cmd);
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
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ACTIONS);
    int update = script.process(p, &propData->actions, &propData->groups, 
            L"Триггеры(actions)", L"Триггеры", L"Новый триггер", &ph);
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(unaction)
{
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ACTIONS);
    int update = script.process(p, &propData->actions, L"Удаление триггера", &ph);
    updateProps(update, LogicHelper::UPDATE_ACTIONS);
}

IMPL(alias)
{
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ALIASES);
    int update = script.process(p, &propData->aliases, &propData->groups,
        L"Макросы(aliases)", L"Макросы", L"Новый макрос", &ph);
    updateProps(update, LogicHelper::UPDATE_ALIASES);
}

IMPL(unalias)
{
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_ALIASES);
    int update = script.process(p, &propData->aliases, L"Удаление команды", &ph);
    updateProps(update, LogicHelper::UPDATE_ALIASES);
}

IMPL(sub)
{
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_SUBS);
    int update = script.process(p, &propData->subs, &propData->groups,
        L"Замены(subs)", L"Замены", L"Новая замена", &ph);
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(unsub)
{
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_SUBS);
    int update = script.process(p, &propData->subs, L"Удаление замены", &ph);
    updateProps(update, LogicHelper::UPDATE_SUBS);
}

IMPL(hotkey)
{
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HOTKEYS);
    HotkeyTestControl control;
    int update = script.process(p, &propData->hotkeys, &propData->groups,
        L"Горячие клавиши(hotkeys)", L"Горячие клавиши", L"Новая горячая клавиша", &ph,
        &control);
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(unhotkey)
{
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HOTKEYS);
    int update = script.process(p, &propData->hotkeys, L"Удаление горячей клавиши", &ph);
    updateProps(update, LogicHelper::UPDATE_HOTKEYS);
}

IMPL(highlight)
{
    AddParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HIGHLIGHTS);
    HighlightTestControl control;
    int update = script.process(p, &propData->highlights, &propData->groups,
        L"Подсветки(highlights)", L"Подсветка", L"Новая подсветка", &ph,
        &control);
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(unhighlight)
{
    DeleteParams3 script; ElementsHelper ph(this, LogicHelper::UPDATE_HIGHLIGHTS);
    int update = script.process(p, &propData->highlights, L"Удаление подсветки", &ph);
    updateProps(update, LogicHelper::UPDATE_HIGHLIGHTS);
}

IMPL(gag)
{
    AddParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_GAGS);
    int update = script.process(p, &propData->gags, &propData->groups, L"Фильтры (gags)", L"Фильтр", &ph);
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(ungag)
{
    DeleteParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_GAGS);
    int update = script.process(p, &propData->gags, L"Удаление фильтра", &ph);
    updateProps(update, LogicHelper::UPDATE_GAGS);
}

IMPL(antisub)
{
    AddParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_ANTISUBS);
    int update = script.process(p, &propData->antisubs, &propData->groups, L"Антизамены (antisubs)", L"Антизамена", &ph);
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
}

IMPL(unantisub)
{
    DeleteParams2 script; ElementsHelper ph(this, LogicHelper::UPDATE_ANTISUBS);
    int update = script.process(p, &propData->antisubs, L"Удаление антизамены", &ph);
    updateProps(update, LogicHelper::UPDATE_ANTISUBS);
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

        int size = propData->variables.size();
        for (int i=0; i<size; ++i)
        {
            const property_value& v = propData->variables.get(i);
            if (n == 1 && v.key.find(p->at(0)) == -1)
                continue;
            swprintf(pb.buffer, pb.buffer_len, L"%s = '%s'", v.key.c_str(), v.value.c_str());
            helper->simpleLog(pb.buffer);
            found = true;
        }
        if (!found)
            helper->tmcLog(L"Список пуст");
        return;
    }

    if (n == 2)
    {
        int index = propData->variables.find(p->at(0));
        propData->variables.add(index, p->at(0), p->at(1), L"");       
        swprintf(pb.buffer, pb.buffer_len, L"%s = '%s'", p->c_str(0), p->c_str(1));
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
        int index = propData->variables.find(p->at(0));
        if (index == -1)
        {
            swprintf(pb.buffer, pb.buffer_len, L"Переменная '%s' не существует.", p->c_str(0));
            helper->tmcLog(pb.buffer);
            return;
        }
        propData->variables.del(index);
        swprintf(pb.buffer, pb.buffer_len, L"Переменная '%s' удалена.", p->c_str(0));
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
    int n = p->size();
    if (n > 2)
        return p->invalidargs();

    ElementsHelper ph(this, LogicHelper::UPDATE_VARS);
    MethodsHelper* helper = ph;
    if (n == 0)
    {
        helper->skipCheckMode();
        helper->tmcLog(L"Группы:");
        for (int i=0,e=propData->groups.size(); i<e; ++i)
        {
            const property_value &v = propData->groups.get(i);
            tstring value = (v.value == L"1") ? L"Вкл" : L"";
            swprintf(pb.buffer, pb.buffer_len, L"%s %s", v.key.c_str(), value.c_str());
            helper->simpleLog(pb.buffer);
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
        swprintf(pb.buffer, pb.buffer_len, L"Группа '%s' не существует.", group.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }

    tstring op; 
    if (n == 2) op = p->at(0);
    if (n == 1 || (op == L"info" || op == L"инф" || op == L"list" || op == L"список"))
    {
        helper->skipCheckMode();
        swprintf(pb.buffer, pb.buffer_len, L"Группа '%s':", group.c_str());
        helper->tmcLog(pb.buffer);
        GroupCollector gc(group);
        int aliases = gc.count(propData->aliases);
        int actions = gc.count(propData->actions);
        int highlights = gc.count(propData->highlights);
        int hotkeys = gc.count(propData->hotkeys);
        int subs = gc.count(propData->subs);
        int gags = gc.count(propData->gags);
        int antisubs = gc.count(propData->antisubs);
        swprintf(pb.buffer, pb.buffer_len, L"Макросы(aliases): %d\nТриггеры(actions): %d\nПодсветки(highlights): %d\nГорячие клавиши(hotkeys): %d\nЗамены(subs): %d\nФильтры(gags): %d\nАнтизамены(antisubs): %d",
            aliases, actions, highlights, hotkeys, subs, gags, antisubs);
        helper->simpleLog(pb.buffer);
        return;
    }
    
    if (op == L"вкл" || op == L"enable" || op == L"on" || op == L"1")
    {
        property_value &v = propData->groups.getw(index);
        v.value = L"1";
        updateProps();
        swprintf(pb.buffer, pb.buffer_len, L"Группа '%s' включена.", v.key.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }

    if (op == L"выкл" || op == L"disable" || op == L"off" || op == L"0")
    {
        property_value &v = propData->groups.getw(index);
        v.value = L"0";
        updateProps();
        swprintf(pb.buffer, pb.buffer_len, L"Группа '%s' выключена.", v.key.c_str());
        helper->tmcLog(pb.buffer);
        return;
    }   

    swprintf(pb.buffer, pb.buffer_len, L"Неизвестная операция '%s'.", op.c_str());
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
            processIncoming(msg.c_str(), msg.length(), SKIP_ACTIONS | SKIP_SUBS | SKIP_HIGHLIGHTS);
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
        m_pHost->disconnectFromNetwork();
        processNetworkError(L"Соединение завершено.");
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
        tchar buffer[64];
        swprintf(buffer, buffer_len, L"Трафик: %.2f Кб, Игровые данные: %.2f Кб, Сжатие: %.2f%%", c/1024, d/1024, ratio);
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
            swprintf(buffer, buffer_len, L"Лог закрыт: '%s'.", oldfile.c_str());
         else
             swprintf(buffer, buffer_len, L"Лог в окне %d закрыт: '%s'.", log, oldfile.c_str());
         tmcLog(buffer);
         if (file.empty())
             return;
    }
    else
    {
        if (file.empty())
        {
            if (log == 0)
                swprintf(buffer, buffer_len, L"Лог не был открыт.");
            else
                swprintf(buffer, buffer_len, L"Лог в окне %d не был открыт.", log);
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
            swprintf(buffer, buffer_len, L"Ошибка! Лог открыть не удалось: '%s'.", logfile.c_str());
        else
            swprintf(buffer, buffer_len, L"Ошибка! Лог в окне %d открыть не удалось: '%s'.", log, logfile.c_str());
    }
    else
    {
        if (log == 0)
            swprintf(buffer, buffer_len, L"Лог открыт: '%s'.", logfile.c_str());
        else
            swprintf(buffer, buffer_len, L"Лог в окне %d открыт: '%s'.", log, logfile.c_str());
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
    swprintf(buffer, buffer_len, L"Недопустимый индекс окна: %d (корректные значения: %d-%d)", view, view0, OUTPUT_WINDOWS);
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
        swprintf(buffer, buffer_len, L"Некорректный параметр: '%s'.", p->c_str(0));
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
        swprintf(buffer, buffer_len, L"Некорректный параметр: '%s'.", p->c_str(0));
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
    ElementsHelper ph(this, LogicHelper::UPDATE_TABS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 0)
    {
        helper->skipCheckMode();
        helper->tmcLog(L"Автоподстановки(tabs):");
        int size = propData->tabwords.size();
        for (int i=0; i<size; ++i)
        {
            const tstring &v = propData->tabwords.get(i);
            helper->simpleLog(v.c_str());
        }
        if (size == 0)
            helper->tmcLog(L"Список пуст.");
        return;
    }

    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = propData->tabwords.find(tab);
        if (index == -1)
            propData->tabwords.add(index, tab);
        swprintf(pb.buffer, pb.buffer_len, L"Автоподстановка '%s' добавлена.", tab.c_str());        
        helper->tmcLog(buffer);
        return;
    }
    p->invalidargs();
}

IMPL(untab)
{
    ElementsHelper ph(this, LogicHelper::UPDATE_TABS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 1)
    {
        const tstring &tab = p->at(0);
        int index = propData->tabwords.find(tab);
        if (index == -1)
            swprintf(buffer, buffer_len, L"Автоподстановки '%s' не существует.", tab.c_str());
        else
        {
            propData->tabwords.del(index);
            swprintf(buffer, buffer_len, L"Автоподстановкa '%s' удалена.", tab.c_str());
        }
        helper->tmcLog(buffer);
        return;
    }
    p->invalidargs();
}

IMPL(timer)
{
    ElementsHelper ph(this, LogicHelper::UPDATE_TIMERS);
    MethodsHelper* helper = ph;
    int n = p->size();
    if (n == 0)
    {
        helper->skipCheckMode();
        helper->tmcLog(L"Таймеры:");
        if (!propData->timers_on)
            helper->simpleLog(L"Таймеры выключены.");

        const PropertiesValues &t  = propData->timers;
        if (t.size() == 0)
        {
            helper->tmcLog(L"Список пуст.");
            return;
        }
        for (int i=0,e=t.size(); i<e; ++i)
        {
            const property_value &v = t.get(i);
            PropertiesTimer pt; pt.convertFromString(v.value);
            swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: '%s' '%s'", v.key.c_str(), pt.timer.c_str(), 
                pt.cmd.c_str(), v.group.c_str());
            helper->simpleLog(buffer);
        }
        return;
    }

    if (n == 1 && !p->isInteger(0))
    {
        tstring op(p->at(0));
        if (op == L"disable" || op == L"off" || op == L"выкл") {
            propData->timers_on = 0;
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
            propData->timers_on = 1;
            helper->tmcLog(L"Таймеры включены.");
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
            swprintf(pb.buffer, pb.buffer_len, L"Таймер #%s не используется.", id.c_str());
            helper->tmcLog(buffer);
            return;
        }
        const PropertiesValues &t  = propData->timers;
        const property_value &v = t.get(index);
        PropertiesTimer pt; pt.convertFromString(v.value);
        swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: '%s' '%s'", v.key.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), v.group.c_str());
        helper->simpleLog(buffer);
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
            swprintf(pb.buffer, pb.buffer_len, L"Ошибка. Таймер #%s не существует.", id.c_str());
            helper->tmcLog(buffer);
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
        swprintf(pb.buffer, pb.buffer_len, L"#%s %s сек: '%s' '%s'", id.c_str(), pt.timer.c_str(), 
              pt.cmd.c_str(), group.c_str());
        helper->simpleLog(buffer);
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

IMPL(message)
{
    int n = p->size();
    if (n == 0)
    {
        MessageCmdHelper mh(propData);
        tstring str;
        mh.getStrings(&str);
        if (!str.empty()) {
            tmcLog(L"Уведомления:");
            simpleLog(str);
        }
        else
            tmcLog(L"Все уведомления отключены");
        return;
    }

    if (n == 1 || n == 2)
    {
        MessageCmdHelper mh(propData);
        if (!mh.setMode(p->at(0), n == 2 ? p->at(1) : L""))
        {
            if (n == 1)
                swprintf(buffer, buffer_len, L"Некорректный параметр: '%s'.", p->at(0).c_str());
            else
                swprintf(buffer, buffer_len, L"Некорректный набор параметров: '%s' '%s'.", p->at(0).c_str(), p->at(1).c_str());
            tmcLog(buffer);
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

    m_univ_prompt_pcre.setRegExp(L"(?:[0-9]+[HMVXC] +)+(?:Вых)?:[СЮЗВПОv^]*>", true);

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
