#include "stdafx.h"
#include "inputProcessor.h"

InputCommand::InputCommand(const tstring& cmd) : empty(true)
{
    // save command
    full_command.assign(cmd);
    tstring& fcmd = full_command;

    // trim command
    tstring_trimleft(&fcmd);
    if (fcmd.empty())
        return;

    empty = false;

    // divide cmd for cmd+parameters
    int pos = fcmd.find(L' ');
    if (pos == -1)
    {
        command.assign(fcmd);
        return;
    }
    command.assign(fcmd.substr(0,pos));
    parameters.assign(fcmd.substr(pos+1));
    fcmd.resize(pos+1);

    BracketsMarker bm;
    bm.unmark(&parameters, &parameters_list);
    fcmd.append(parameters);
}

void InputCommand::replace_command(const tstring& cmd)
{
    if (cmd == command)
        return;
    command.assign(cmd);
    full_command.assign(cmd);
    full_command.append(L" ");
    full_command.append(parameters);
}

void InputCommandsList::parse(const tstring& cmds)
{
    /*tstring tmp(cmds);
    tstring_trimleft(&tmp);
    // marker for brackets
    BracketsMarker bm;
    tchar marker = bm.getMarker();

    // truncate to separate commands
    const WCHAR *p = sep_cmd.c_str();
    const WCHAR *e = p + sep_cmd.length();
    bool inside_brackets = false;

    const WCHAR *b = p;
    while (p != e)
    {
        if (*p == m_separator && !inside_brackets)
        {
            InputCommand *icmd = new InputCommand( tstring(b, p-b) );
            result->push_back(icmd);
            p++; b = p;
            continue;
        }
        if (marker == *p)
            inside_brackets = !inside_brackets;
        p++;
    }
    if (b != e || sep_cmd.empty())
    {
        InputCommand *icmd = new InputCommand(b);
        result->push_back(icmd);
    }*/
}

InputCommandTemplate::InputCommandTemplate(const tstring& cmd, tchar separator, tchar syscmd, tchar marker)
{
    const tchar *p = cmd.c_str();
    const tchar *e = p + cmd.length();
    /*if (p == e) { 
       m_subcmds.push_back( subcmd(tstring(), 0) );
       return;
    }*/

    while (p != e)
    {
        const tchar *b = p;
        while (*p == L' ' && p != e) p++;
        if (p == e) 
        {   // финальная строка из пробелов
            m_subcmds.push_back( subcmd (tstring(b), 0) );
            return;
        }
        if (*p == L'{')
        {
            // игровая в фигурных скобках (можно исп.;)
            tstring cmd(b, p-b);
            const tchar* bracket_begin = p++;
            int stack = 0;
            while (p != e)
            {
                if (stack == 0 && *p == L'}')
                {
                    cmd.append(bracket_begin+1, p-bracket_begin-2);                    
                    break;
                }
                else
                {
                     if (*p == '}')
                         stack--;
                     else
                         stack++;
                }
                p++;
            }
            if (p == e) {
                 cmd.append(bracket_begin);
                 m_subcmds.push_back( subcmd (cmd, 0) );
                 return;
            }
            p++;
            b = p;
            while (p != e && *p != separator)  p++;
            cmd.append(b, p-b);
            m_subcmds.push_back( subcmd (cmd, 0) );
            if (*p == separator) p++;
            continue;
        }
        if (*p == syscmd)
        {
            // системная команда - парсинг сепаратора с учетои скобок
            p++;
            b = p;
            
            std::vector<tchar> stack;
            while (p != e)
            {
                if (*p == separator && stack.empty())
                {
                    tstring cmd(b, p-b);
                    m_subcmds.push_back( subcmd (cmd, 1) );
                    break;
                }
                if (isbracket(p))
                {
                    if (!stack.empty())
                    {
                        int last = stack.size()-1;
                        if (((*p == L'\'' || *p == L'"') && stack[last] == *p) ||
                            (*p == L'}' && stack[last] == L'{'))
                        {
                            stack.pop_back();
                        }
                        else
                        {
                            stack.push_back(*p);
                        }
                    }
                    else if (*p != L'}')
                        { stack.push_back(*p); }
                }
                p++;
            }
            if (p == e)
            {
               tstring cmd(b);
               m_subcmds.push_back( subcmd (cmd, 1) );
            }
            p++;
            continue;
        }

        // игровая, но начинается не со скобок
        while (p != e && *p != separator)  p++;
        tstring cmd(b, p-b);
        m_subcmds.push_back( subcmd (cmd, 0) );
        if (*p == separator) p++;
    }
}

bool InputCommandTemplate::isbracket(const tchar *p)
{
    return (wcschr(L"{}\"'", *p)) ? true : false;
}

void InputCommandTemplate::translate(InputCommandsList *cmd)
{
}

InputProcessor::InputProcessor(tchar separator, tchar prefix) : m_separator(separator), m_prefix(prefix)
{
}

InputProcessor::~InputProcessor()
{
}

void InputProcessor::process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds)
{
    // clear data
    //clear();

    // process separators
    processSeparators(cmd, &commands);

    // process aliases
    int queue_size = commands.size();
    if (queue_size == 0) 
        return;

    // to protect from loops in aliases
    std::vector<tstring> loops_hunter;
    for (int i=0; i<queue_size;)
    {
        loops_hunter.push_back(commands[i]->command);

        bool alias_found = false;
        tstring cmd(commands[i]->command);
        tstring alias;
        if (!cmd.empty() && cmd.at(0) != m_prefix // skip empty and system commands
            && helper->processAliases(cmd, &alias))
        {
            if (alias != cmd)
                alias_found = true;
        }

        if (alias_found)
        {
            tstring result;
            processParameters(alias, commands[i], &result);
            InputCommandsList new_cmd_list;
            processSeparators(result, &new_cmd_list);

            bool loop = false;
            for (int j=0,je=new_cmd_list.size(); j<je; ++j)
            {
                if (std::find(loops_hunter.begin(), loops_hunter.end(), new_cmd_list[j]->command) !=
                    loops_hunter.end())
                    {
                       loop = true;
                       loop_cmds->push_back(new_cmd_list[j]->command);
                       break;
                    }
            }

            if (loop)
            {
                //loop in aliases - skip current command
                commands.erase(i);
                queue_size = commands.size();
                loops_hunter.clear();
                continue;
            }

            commands.erase(i);
            commands.insert(i, new_cmd_list);
            queue_size = commands.size();
        }
        else
        {
            i++;
            loops_hunter.clear();
        }
    }
}

void InputProcessor::processSeparators(const tstring& sep_cmd, InputCommandsList* result)
{
    // marker for brackets
    BracketsMarker bm;
    tchar marker = bm.getMarker();

    // truncate to separate commands
    const WCHAR *p = sep_cmd.c_str();
    const WCHAR *e = p + sep_cmd.length();
    bool inside_brackets = false;

    const WCHAR *b = p;
    while (p != e)
    {
        if (*p == m_separator && !inside_brackets)
        {
            InputCommand *icmd = new InputCommand( tstring(b, p-b) );
            result->push_back(icmd);
            p++; b = p;
            continue;
        }
        if (marker == *p)
            inside_brackets = !inside_brackets;
        p++;
    }
    if (b != e || sep_cmd.empty())
    {
        InputCommand *icmd = new InputCommand(b);
        result->push_back(icmd);
    }
}

void InputProcessor::processParameters(const tstring& cmd, InputCommand* params, tstring* result)
{
    // find parameters
    ParamsHelper ph(cmd);
    if (!ph.getSize())
    {
        result->assign(cmd);
        int cmd_size = params->command.length();
        result->append(params->full_command.substr(cmd_size));
        return;
    }

    result->clear();
    int params_count = params->parameters_list.size();

    int ptr = 0;
    int size = ph.getSize();
    for (int i=0; i<size; ++i)
    {
        int begin = ph.getFirst(i);
        int end = ph.getLast(i);
        result->append(cmd.substr(ptr, begin-ptr));

        int parameter_index = ph.getId(i);
        if (parameter_index <= params_count)
        {
            const tstring& p = (parameter_index == 0) ? params->parameters : 
                params->parameters_list[parameter_index-1];
            result->append(p);
        }
        else
        {
            if (begin > 0 && cmd.at(begin - 1) == L' ')
            {
                int size = cmd.size();
                if (end < size && cmd.at(end) == L' ')
                    end = end + 1;
            }
        }
        ptr = end;
    }
    result->append(cmd.substr(ptr));
}
