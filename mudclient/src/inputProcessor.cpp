#include "stdafx.h"
#include "inputProcessor.h"
#include "logicProcessor.h"
extern LogicProcessorMethods* _lp;
bool InputVarsAccessor::get(const tstring&name, tstring* value)
{
    return _lp->getVar(name.c_str(), value);
}

void InputVarsAccessor::tanslateVars(tstring *cmd)
{
    _lp->processVars(cmd);
}

InputPlainCommands::InputPlainCommands() {}
InputPlainCommands::InputPlainCommands(const tstring& cmd)
{
    const tchar *separators = L"\r\n";
    if (!isExistSymbols(cmd, separators))
    {
        push_back(cmd);
        return;
    }

   const tchar *p = cmd.c_str();
   const tchar *e = p + cmd.length();
   while (p < e)
   {
      size_t len = wcscspn(p, separators);
      if (len > 0)
        push_back(tstring(p, len));
      p = p + len + 1;
   }
}

void InputTemplateCommands::init(const InputPlainCommands& cmds, const InputTemplateParameters& params)
{
    _params = params;
    clear();        // for multiply use
    for (int i=0,e=cmds.size();i<e;++i)
        parsecmd(cmds[i]);
}

void InputTemplateCommands::extract(InputPlainCommands* cmds)
{
    tchar prefix[] = { _params.prefix, 0 };
    cmds->clear();
    for (int i=0,e=size(); i<e; ++i)
    {
        tstring cmd;
        if (at(i).second)
            cmd.append(prefix);
        cmd.append(at(i).first);
        cmds->push_back(cmd);
    }
}

void InputTemplateCommands::makeTemplates()
{
    for (int i=0,e=size(); i<e; ++i)
    {
        if (at(i).second)   // маркируем только системные
            markbrackets(&at(i).first);
    }
}

void InputTemplateCommands::tranlateVars()
{
    InputVarsAccessor va;
    for (int i=0,e=size();i<e;++i)
        va.tanslateVars(&at(i).first);
}

void InputTemplateCommands::makeCommands(InputCommands *cmds)
{
    for (int i=0,e=size(); i<e; ++i)
    {
        const InputSubcmd &subcmd = at(i);
        const tstring& t = subcmd.first;   //template of cmd
        InputCommand *cmd = new InputCommand();
        cmd->system = (subcmd.second) ? true : false;
        size_t pos = t.find(L" ");
        if (pos == -1) {
            cmd->srccmd = t;
            cmd->command = t;
        }
        else
        {
            if (pos == 0)
            {
                pos = wcsspn(t.c_str(), L" ");
                size_t from = pos;
                pos = t.find(L" ", pos);
                if (pos == -1) {
                     cmd->srccmd = t;
                     cmd->command = t.substr(from);
                     return;
                }
                cmd->srccmd.assign(t.substr(0, pos));
                cmd->command.assign(t.substr(from, pos-from));
            }
            else
            {
                cmd->command.assign(t.substr(0, pos));
                cmd->srccmd = cmd->command;
            }            
            cmd->parameters.assign(t.substr(pos));
            if (cmd->system)
                fillsyscmd(cmd);
            else
                fillgamecmd(cmd);
        }
        cmds->push_back(cmd);
    }
}

void InputTemplateCommands::fillsyscmd(InputCommand *cmd)
{
    unmarkbrackets(&cmd->parameters, &cmd->parameters_list);
}

void InputTemplateCommands::fillgamecmd(InputCommand *cmd)
{
    const tstring& params = cmd->parameters;
    const tchar *p = params.c_str();
    const tchar *e = p + params.length();
    while (p != e)
    {
       const tchar *s = wcschr(p, L' ');
       if (!s) break;
       if (p != s)
            cmd->parameters_list.push_back(tstring(p, s-p));
       p = s + 1;
    }
    if (p != e)
        cmd->parameters_list.push_back(tstring(p, e-p));
}

void InputTemplateCommands::parsecmd(const tstring& cmd)
{
    tchar separator = _params.separator;
    tchar syscmd = _params.prefix;

    const tchar *p = cmd.c_str();
    const tchar *e = p + cmd.length();
    if (p == e) {
       push_back( InputSubcmd(tstring(), 0) );
       return;
    }

    while (p != e)
    {
        const tchar *b = p;
        while (*p == L' ' && p != e) p++;
        if (p == e) 
        {   // финальная строка из пробелов
            push_back( InputSubcmd (tstring(b), 0) );
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
                    cmd.append(bracket_begin+1, p-bracket_begin-1);                    
                    break;
                }
                else
                {
                     if (*p == L'}')
                         stack--;
                     else if (*p == L'{')
                         stack++;
                }
                p++;
            }
            if (p == e) {
                 cmd.append(bracket_begin);
                 push_back( InputSubcmd (cmd, 0) );
                 return;
            }
            p++;
            b = p;
            while (p != e && *p != separator)  p++;
            cmd.append(b, p-b);
            push_back( InputSubcmd (cmd, 0) );
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
                    push_back( InputSubcmd (cmd, 1) );
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
               push_back( InputSubcmd (cmd, 1) );
            }
            else 
                p++;
            continue;
        }

        // игровая, но начинается не со скобок
        while (p != e && *p != separator)  p++;
        tstring cmd(b, p-b);
        push_back( InputSubcmd (cmd, 0) );
        if (*p == separator) p++;
    }
}

void InputTemplateCommands::markbrackets(tstring *cmd) const
{
    const tchar marker[2] = { MARKER , 0 };
    const tchar *b0 = cmd->c_str();
    const tchar *p = b0;
    const tchar *e = p + cmd->length();

    const tchar* bracket_begin = NULL;
    std::vector<tchar> stack;
    tstring newp;

    const tchar* b = p;
    while (p != e)
    {
        if (!isbracket(p))
            { p++; continue; }
        // check space before open bracket
        if (!bracket_begin)
        {
            if (p != b0 && p[-1] != ' ')
                { p++; continue; }
        }
        // check space after close bracket
        else
        {
            if (p+1 != e && p[1] != ' ')
                { p++; continue; }
        }

        if (stack.empty())
        {
            if (*p == L'}')
                { p++; continue; }
            stack.push_back(*p);
            bracket_begin = p;
        }
        else
        {
            if (((*p == L'\'' || *p == L'"') && *bracket_begin == *p) ||
                (*p == L'}' && *bracket_begin == L'{' && stack.size() == 1))
            {
               stack.clear();
               // mark pair brackets
               newp.append(b, bracket_begin-b);
               newp.append(marker);
               newp.append(bracket_begin, p-bracket_begin);
               newp.append(marker);
               newp.append(p, 1);
               b = p + 1;
               p = b;
               bracket_begin = NULL;
               continue;
            }

            if (*p == L'{')
                stack.push_back(*p);
            else if (*p == L'}')
                stack.pop_back();
        }
        p++;
    }
    if (b != e)
        newp.append(b);
    cmd->swap(newp);
}

void InputTemplateCommands::unmarkbrackets(tstring* parameters, std::vector<tstring>* parameters_list) const
{
   assert(parameters && parameters_list);
   if (parameters->empty())
       return;

   std::vector<tstring> &tp = *parameters_list;

   // get parameters, delete markers from parameters
   const WCHAR *p = parameters->c_str();
   const WCHAR *e = p + parameters->length();

   const tchar* bracket_begin = NULL;
   bool combo_bracket = false;
   tstring newp;

   const WCHAR *b = p;
   while (p != e)
   {
       if (*p == MARKER /*&& (p+1)!=e*/ && isbracket(&p[1]))
       {
           if (!bracket_begin)
               bracket_begin = p;
           else if (*bracket_begin != MARKER)
           {
               newp.append(b, p-b);
               // get parameter without left spaces
               tstring cp(bracket_begin, p-bracket_begin);
               tp.push_back(cp);

               bracket_begin = p;
               b = p;
               p++;
               combo_bracket = true;
               continue;
           }
           else
           {
               newp.append(b, bracket_begin-b);
               bracket_begin++;
               newp.append(bracket_begin, p-bracket_begin);

               // get parameter without brackets
               tstring cp(bracket_begin+1, p-bracket_begin-1);
               if (combo_bracket)
               {
                   int last = tp.size()-1;
                   tp[last].append(cp);
                   combo_bracket = false;
               }
               else {
                   tp.push_back(cp); 
               }

               p++;
               newp.append(p, 1);
               p++;
               b = p;
               bracket_begin = NULL;
               continue;
           }
       }
       else if (*p != L' ' && !bracket_begin)
       {
           bracket_begin = p;
       }
       else if (*p == L' ' && bracket_begin && *bracket_begin != MARKER)
       {
           newp.append(b, p-b);

           // get parameter without left spaces
           tstring cp(bracket_begin, p-bracket_begin);
           tp.push_back(cp);
           bracket_begin = NULL;
           b = p;
           continue;
       }
       p++;
   }
   if (b != e)
   {
       if (bracket_begin && *bracket_begin == MARKER)
       {
           b++;
           newp.append(b);
           b++;
           tp.push_back(b);
       }
       else
       {
           newp.append(b);
           tstring tmp(b);
           tstring_trimleft(&tmp);
           if (!tmp.empty())
              tp.push_back(tmp);
       }
   }
   parameters->swap(newp);
}

bool InputTemplateCommands::isbracket(const tchar *p) const
{
    return (wcschr(L"{}\"'", *p)) ? true : false;
}


/*InputCommand::InputCommand(const tstring& cmd) : empty(true)
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

    /*BracketsMarker bm;
    bm.unmark(&parameters, &parameters_list);
    fcmd.append(parameters);
}*/

/*void InputCommands::parse(const tstring& cmds)
{
    tstring tmp(cmds);
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
    }
}*/

/*
InputCommandTemplate::InputCommandTemplate()
{
}

bool InputCommandTemplate::init(const tstring& key, const tstring& value, const InputCommandParameters& params)
{
    if (!m_key.init(key))
        return false;
}

bool InputCommandTemplate::compare(const tstring& str)
{
    return m_key.compare(str);
}

void InputCommandTemplate::translate(InputCommands *cmd) const
{
    //tstring result;
    for (int i=0,e=m_subcmds.size(); i<e; ++i)
    {
        // parameters and vars
        tstring value(m_subcmds[i].first);
        m_key.translateParameters(&value);
//        m_key.translateVars(&value);
        
        if (m_subcmds[i].second)
        {
            // brackets unmark operation (only system cmds)

        }
        else
        {
            // separate cmd and parameters by spaces

        
        
        }

        //InputCommand *cmd = new InputCommand;
        //cmd->    
    }
}*/



/*
InputProcessor::InputProcessor(tchar separator, tchar prefix) : m_separator(separator), m_prefix(prefix)
{
}

InputProcessor::~InputProcessor()
{
}

void InputProcessor::process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds)
{
    InputCommandTemplate t(cmd, m_separator, m_prefix);
    t.translate(&commands);


    // clear data
    //clear();
    // process separators
    //processSeparators(cmd, &commands);

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
            InputCommands new_cmd_list;
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

void InputProcessor::processSeparators(const tstring& sep_cmd, InputCommands* result)
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
*/