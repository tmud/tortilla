#include "stdafx.h"
#include "inputProcessor.h"

class InputCommandHelper
{
public:
    Pcre16 pcre;
    InputCommandHelper() {
         pcre.setRegExp(L"\\{.*?\\}|\\\".*?\\\"|\\'.*?\\'|[^ ]+", true);
    }    
} m_ich;

InputCommand::InputCommand(const tstring& cmd)
{
    // save command
    full_command.assign(cmd);
    tstring& fcmd = full_command;

    // trim command
    tstring_trimleft(&fcmd);
    if (fcmd.empty())
        return;
        
    // divide cmd for cmd+parameters
    int pos = fcmd.find(L' ');
    if (pos == -1)
    {
        command.assign(fcmd);
        return;
    }
    command.assign(fcmd.substr(0,pos));
    parameters.append(fcmd.substr(pos+1));
    tstring_trimleft(&parameters);

    // get parameters
    const WCHAR* p = fcmd.c_str() + pos + 1;
    m_ich.pcre.findAllMatches(p);
    for (int i=0,e=m_ich.pcre.getSize(); i<e; ++i)
    {
        int f = m_ich.pcre.getFirst(i);
        int l = m_ich.pcre.getLast(i);
        tstring tmp(p+f, l-f);
        WCHAR t = tmp.at(0);
        if (t == L'{' || t == L'\'' || t == L'\"')        
            tmp = tmp.substr(1, tmp.length()-2);
        parameters_list.push_back(tmp);
    }
}

InputProcessor::InputProcessor() : pData(NULL)
{
}

InputProcessor::~InputProcessor()
{
    clear();
}

void InputProcessor::updateProps(PropertiesData *pdata) 
{
    pData = pdata;
}

void InputProcessor::process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds)
{   
    // clear data
    clear();

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
        WCHAR prefix = pData->cmd_prefix;
        tstring cmd(commands[i]->command);
        tstring alias;
        if (!cmd.empty() && cmd.at(0) != prefix // skip empty and system commands
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
                delete commands[i];
                commands.erase(commands.begin() + i);
                queue_size = commands.size();
                loops_hunter.clear();                
                continue;
            }
            
            delete commands[i];
            commands.erase(commands.begin() + i);
            commands.insert(commands.begin() + i, new_cmd_list.begin(), new_cmd_list.end());
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
    // truncate to separate commands
    WCHAR separator = pData->cmd_separator;
    size_t pos0 = 0;
    size_t pos = sep_cmd.find(separator);
    while (pos != tstring::npos)
    {
        InputCommand *icmd = new InputCommand( sep_cmd.substr(pos0, pos-pos0) );
        result->push_back(icmd);        
        pos0 = pos+1;
        pos = sep_cmd.find(separator, pos0);
    }
    InputCommand *icmd = new InputCommand( sep_cmd.substr(pos0) );
    result->push_back(icmd);
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

void InputProcessor::clear()
{
    autodel<InputCommand> z(commands);
}
