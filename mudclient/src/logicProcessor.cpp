// In that file - Code for processing game data
#include "stdafx.h"
#include "logicProcessor.h"

LogicProcessor::LogicProcessor(PropertiesData *data, LogicProcessorHost *host) :
propData(data), m_pHost(host), m_connecting(false), m_connected(false), m_helper(data),
m_prompt_mode(OFF), m_prompt_counter(0)
{
    for (int i=0; i<OUTPUT_WINDOWS+1; ++i)
        m_wlogs[i] = -1;
}

LogicProcessor::~LogicProcessor()
{
}

void LogicProcessor::processTick()
{
    if (!m_connected || !propData->timers_on)
        return;
    std::vector<tstring> timers_cmds;
    m_helper.processTimers(&timers_cmds);
    for (int i=0,e=timers_cmds.size(); i<e; ++i)
        processCommand(timers_cmds[i]);
}

void LogicProcessor::processNetworkData(const WCHAR* text, int text_len)
{
    processIncoming(text, text_len);
}

void LogicProcessor::processNetworkConnect()
{
    propData->timers_on = 0;
    m_helper.resetTimers();
    m_connected = true;
    m_connecting = false;
}

bool LogicProcessor::processHotkey(const tstring& hotkey)
{
    if (hotkey.empty())
        return false;

    tstring newcmd;
    if (m_helper.processHotkeys(hotkey, &newcmd))
    {
        processCommand(newcmd);
        return true;
    }
    return false;
}

void LogicProcessor::processCommand(const tstring& cmd)
{
    std::vector<tstring> loops;
    WCHAR cmd_prefix = propData->cmd_prefix;
    m_input.process(cmd, &m_helper, &loops);

    if (!loops.empty())
    {
        tstring msg;
        int size = loops.size();
        if (size == 1) {
            msg.append(L"Макрос '"); msg.append(loops[0]); msg.append(L"' зациклен. Выполнение невозможно.");
        }
        else {
            msg.append(L"Макросы '");
            for (int i = 0; i < size; ++i) { if (i != 0) msg.append(L","); msg.append(loops[i]); }
            msg.append(L"' зациклены. Их выполнение невозможно.");
        }
        tmcLog(msg);
    }

    for (int i=0,e=m_input.commands.size(); i<e; ++i)
    {
        tstring cmd = m_input.commands[i]->full_command;
        if (!cmd.empty() && cmd.at(0) == cmd_prefix)
            processSystemCommand(cmd); //it is system command for client (not game command)
        else
            processGameCommand(cmd); // it is game command
    }
}

void LogicProcessor::updateProps()
{
    m_helper.updateProps();
    m_input.updateProps(propData);
    m_logs.updateProps(propData);
    m_prompt_mode = OFF;
    if (propData->recognize_prompt)
    {
        // calc regexp from template
        tstring tmpl(propData->recognize_prompt_template);
        Pcre16 t1;
        t1.setRegExp(L"\\\\\\*");
        t1.findAllMatches(tmpl);
        std::vector<tstring> parts;
        int pos = 0;
        for (int i = 0, e = t1.getSize(); i < e;  ++i)
        {
            int last = t1.getFirst(i);
            parts.push_back(tmpl.substr(pos, last - pos));
            pos = t1.getLast(i);
        }
        parts.push_back(tmpl.substr(pos));
        for (int i = 0, e = parts.size(); i < e; ++i)
        {
            MaskSymbolsBySlash mask(parts[i], L"+/?|^$.[]()\\");
            parts[i] = mask;
            tstring_replace(&parts[i], L"*", L".*");
        }

        int last = parts.size() - 1;
        tmpl.clear();
        for (int i = 0; i < last; ++i)
        {
            tmpl.append(parts[i]);
            tmpl.append(L"\\*");
        }
        tmpl.append(parts[last]);

        bool result = m_prompt_pcre.setRegExp(tmpl, true);
        if (!result)
        {
            MessageBox(m_pHost->getMainWindow(), L"Ошибка в шаблоне для распознавания Prompt-строки!", L"Ошибка", MB_OK | MB_ICONERROR);
            propData->recognize_prompt = 0;
        }
    }
}

void LogicProcessor::processNetworkDisconnect()
{
    processNetworkError(L"Соединение завершено(обрыв).");
}

void LogicProcessor::processNetworkConnectError()
{
    processNetworkError(L"Не удалось подключиться.");
}

void LogicProcessor::processNetworkError()
{
    processNetworkError(L"Ошибка cети. Соединение завершено.");
}

void LogicProcessor::processNetworkMccpError()
{
    processNetworkError(L"Ошибка в протоколе сжатия. Соединение завершено.");
}

void LogicProcessor::tmcLog(const tstring& cmd)
{
    tstring log(L"[tortilla] ");
    log.append(cmd);
    simpleLog(log);
}

void LogicProcessor::simpleLog(const tstring& cmd)
{
    tstring log(cmd);
    log.append(L"\r\n");
    processIncoming(log.c_str(), log.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_PLUGINS|GAME_LOG);
}

void LogicProcessor::syscmdLog(const tstring& cmd)
{
    if (!propData->show_system_commands)
        return;
    tstring log(cmd);
    log.append(L"\r\n");
    processIncoming(log.c_str(), log.length(), SKIP_ACTIONS|SKIP_SUBS|GAME_LOG|GAME_CMD);
}

void LogicProcessor::pluginLog(const tstring& cmd)
{
    if (!propData->plugins_logs)
        return;
    int window = propData->plugins_logs_window;
    if (window >= 0 && window <= OUTPUT_WINDOWS)
    {
        tstring log(L"[plugin] ");
        log.append(cmd);
        log.append(L"\r\n");
        processIncoming(log.c_str(), log.length(), SKIP_ACTIONS|SKIP_SUBS|SKIP_PLUGINS|GAME_LOG, window);
    }
}

void LogicProcessor::updateActiveObjects(int type)
{
    m_helper.updateProps(type);
}

bool LogicProcessor::checkActiveObjectsLog(int type)
{
    MessageCmdHelper mh(propData);
    int state = mh.getState(type);
    return (!state) ? false : true;
}

bool LogicProcessor::addSystemCommand(const tstring& cmd)
{
    PropertiesList &p = propData->tabwords_commands;
    if (p.find(cmd) != -1)
        return false;
    m_plugins_cmds.push_back(cmd);
    propData->tabwords_commands.add(-1, cmd);
    return true;
}

bool LogicProcessor::deleteSystemCommand(const tstring& cmd)
{
    std::vector<tstring>::iterator it = std::find(m_plugins_cmds.begin(), m_plugins_cmds.end(), cmd);
    if (it == m_plugins_cmds.end())
        return false;
    m_plugins_cmds.erase(it);
    PropertiesList &p = propData->tabwords_commands;
    int index = p.find(cmd);
    p.del(index);
    return true;
}

void LogicProcessor::doGameCommand(const tstring& cmd)
{
    processCommand(cmd);
}

void LogicProcessor::updateLog(const tstring& msg)
{
    m_updatelog.append(msg);
}

bool LogicProcessor::sendToNetwork(const tstring& cmd)
{
    if (m_connected)
    {
        m_pHost->sendToNetwork(cmd);
        return true;
    }
    tmcLog(L"Нет подключения.");
    return false;
}

void LogicProcessor::processNetworkError(const tstring& error)
{
    m_prompt_mode = OFF;
    m_prompt_counter = 0;
    if (m_connected || m_connecting)
        tmcLog(error.c_str());
    m_connected = false;
    m_connecting = false;
}
