#include "stdafx.h"
#include "accessors.h"
#include "inputProcessor.h"
#include "logicProcessor.h"

bool InputVarsAccessor::get(const tstring&name, tstring* value)
{
    return tortilla::getVars()->getVar(name.c_str(), value);
}

void InputVarsAccessor::translateVars(tstring *cmd)
{
    tortilla::getVars()->processVars(cmd);
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
        if (at(i).system)
            cmd.append(prefix);
        cmd.append(at(i).srccmd);
        cmds->push_back(cmd);
    }
}

void InputTemplateCommands::makeTemplates()
{
    for (int i=0,e=size(); i<e; ++i)
    {
        if (at(i).system)   // маркируем только системные
            markbrackets(&at(i).templ);
    }
}

void InputTemplateCommands::makeCommands(InputCommands *cmds, const CompareObject* params)
{    
    for (int i=0,e=size(); i<e; ++i)
    {
        const InputSubcmd &subcmd = at(i);

        InputCommand *cmd = new InputCommand();
        cmd->system = subcmd.system;

        // make src parameters
        const tstring& s = subcmd.srccmd;
        size_t pos = s.find(L" ");
        if (pos == -1)
            cmd->srccmd.assign(s);
        else if (pos == 0) 
        {
             size_t from = wcsspn(s.c_str(), L" ");
             pos = s.find(L" ", from);
             if (pos == -1) { 
                 cmd->srccmd.assign(s); 
             }
             else {
                 cmd->srccmd.assign(s.substr(0, from));
                 cmd->srcparameters.assign(s.substr(from)); 
             }
        }
        else {
            cmd->srccmd.assign(s.substr(0, pos));
            cmd->srcparameters.assign(s.substr(pos));
        }                

        tstring t(subcmd.templ);    //template of cmd
        if (params)                 //translate parameters
            params->translateParameters(&t);
        InputVarsAccessor va;
        va.translateVars(&t);       //translate vars in template
        
        pos = t.find(L" ");
        if (pos == -1) {
            cmd->command = t;
        }
        else if (pos == 0)
        {
            size_t from = wcsspn(t.c_str(), L" ");
            pos = t.find(L" ", from);
            if (pos == -1)
                 cmd->command = t.substr(from);
            else {
                 cmd->command.assign(t.substr(from, pos-from));
                 cmd->parameters.assign(t.substr(pos));
            }
        }
        else
        {
            cmd->command.assign(t.substr(0, pos));
            cmd->parameters.assign(t.substr(pos));
        }

        if (cmd->system)
            fillsyscmd(cmd);
        else
            fillgamecmd(cmd);
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
       push_back( InputSubcmd(L"",  0) );
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
   const tchar *p = parameters->c_str();
   const tchar *e = p + parameters->length();

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

/*void InputTemplateCommands::trimcmd(tstring* cmd)
{
   size_t pos = cmd->find(L" ");
   if (pos == -1)
       return;
   if (pos != 0)
   {
       tstring tmp(cmd->substr(0, pos));
       cmd->swap(tmp);
       return;
   }
   size_t from = wcsspn(cmd->c_str(), L" ");
   pos = cmd->find(L" ", from);
   if (pos == -1)
   {
       tstring tmp(cmd->substr(from));
       cmd->swap(tmp);
       return;
   }
   tstring tmp(cmd->substr(from, pos-from));
   cmd->swap(tmp);
}*/
