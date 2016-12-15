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
    tortilla::getVars()->processVars(cmd, false);
}

bool InputCommandsVarsFilter::checkFilter(InputCommand cmd)
{
    if (!cmd->system)
        return false;
    const tstring& c = cmd->command;
    if (!c.compare(0, 3, L"action", 3))
        return true;
    if (!c.compare(0, 3, L"alias", 3))
        return true;
    if (!c.compare(0, 3, L"sub", 3))
        return true;
    if (!c.compare(0, 3, L"highlight", 3))
        return true;
    if (!c.compare(0, 3, L"hotkey", 3))
        return true;
    if (!c.compare(0, 3, L"unsub", 3))
        return true;
    if (!c.compare(0, 4, L"unhighlight", 4))
        return true;
    if (!c.compare(0, 4, L"unhotkey", 4))
        return true;
    if (!c.compare(0, 3, L"gag", 3))
        return true;
    if (!c.compare(0, 3, L"ungag", 3))
        return true;
    if (!c.compare(0, 3, L"antisub", 3))
        return true;
    if (!c.compare(0, 4, L"unaction", 4))
        return true;
    if (!c.compare(0, 4, L"unalias", 4))
        return true;
    if (!c.compare(0, 4, L"unantisub", 4))
        return true;
    if (!c.compare(0, 3, L"timer", 3))
        return true;
    if (!c.compare(0, 3, L"untimer", 3))
        return true;
    if (!c.compare(0, 3, L"uptimer", 3))
        return true;
    return false;
}

bool InputCommandVarsProcessor::makeCommand(InputCommand cmd)
{
    VarProcessor *vp = tortilla::getVars();
    vp->processVars(&cmd->parameters, false);
    for (int i=0,e=cmd->parameters_list.size(); i<e; ++i)
       vp->processVars(&cmd->parameters_list[i], false);
    return vp->processVars(&cmd->command, false);
}

void InputTranslateParameters::doit(const InputParameters *params, tstring *cmd)
{
    assert(params && cmd);
    std::vector<tstring> params_list;
    params->getParameters(&params_list);
    int params_count = params_list.size();

    tstring result;
    int pos = 0;
    ParamsHelper values(*cmd, false);
    int values_count = values.getSize();
    for (int i = 0; i < values_count; ++i)
    {
        result.append(cmd->substr(pos, values.getFirst(i) - pos));
        int id = values.getId(i);
        if (id < params_count && id >= 0)
        {
            tstring param(params_list[id]);
            values.cutParameter(i, &param);
            result.append(param);
        }
        pos = values.getLast(i);
    }
    result.append(cmd->substr(pos));
    cmd->swap(result);
    if (values_count == 0)
    {
        params->doNoValues(cmd);
    }
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

int InputTemplateCommands::size() const
{
    return base::size();
}

void InputTemplateCommands::makeTemplates()
{
    for (int i=0,e=size(); i<e; ++i)
    {
        if (at(i).system)   // маркируем только системные
        { 
            bool markered = markbrackets(&at(i).templ);
            at(i).markered = markered;
        }
    }
}

void InputTemplateCommands::makeCommands(InputCommands *cmds, const InputParameters* params)
{
    tchar prefix[] = { _params.prefix, 0 };
    for (int i=0,e=size(); i<e; ++i)
    {
        const InputSubcmd &subcmd = at(i);

        InputCommand cmd =  std::make_shared<InputCommandData>();
        cmd->system = subcmd.system;
        if (cmd->system)
            cmd->srccmd.append(prefix);

        // make src parameters
        const tstring& s = subcmd.srccmd;
        size_t pos = s.find(L" ");
        if (pos == -1)
            cmd->srccmd.append(s);
        else if (pos == 0) 
        {
             size_t from = wcsspn(s.c_str(), L" ");
             pos = s.find(L" ", from);
             if (pos == -1) { 
                 cmd->srccmd.append(s); 
             }
             else {
                 cmd->srccmd.append(s.substr(0, from));
                 cmd->srcparameters.append(s.substr(from)); 
             }
        }
        else {
            cmd->srccmd.append(s.substr(0, pos));
            cmd->srcparameters.append(s.substr(pos));
        }

        tstring t(subcmd.templ);    //template of cmd
        if (params)                 //translate parameters
        {
            InputTranslateParameters tp;
            tp.doit(params, &t);
        }

        pos = t.find(L" ");
        if (pos == -1) {
            cmd->command = t;
        }
        else if (pos == 0)
        {
            size_t from = wcsspn(t.c_str(), L" ");
            if (cmd->system)
                cmd->parameters.append(t.substr(from));
            else {
            pos = t.find(L" ", from);
            from = 0; //не обрезаем ведущие пробелы для игровых команд
            if (pos == -1)
                 cmd->command = t.substr(from);
            else {
                 cmd->command.append(t.substr(from, pos-from));
                 cmd->parameters.append(t.substr(pos));
            }}
        }
        else
        {
            cmd->command.append(t.substr(0, pos));
            cmd->parameters.append(t.substr(pos));
        }

        if (cmd->system)
        {
            if (!subcmd.markered)
                markbrackets(&cmd->parameters);
            fillsyscmd(cmd);
        }
        else
            fillgamecmd(cmd);
        cmds->push_back(cmd);
    }
}

void InputTemplateCommands::fillsyscmd(InputCommand cmd)
{
    unmarkbrackets(&cmd->parameters, &cmd->parameters_list);
}

void InputTemplateCommands::fillgamecmd(InputCommand cmd)
{
    const tstring& params = cmd->parameters;
    const tchar *p = params.c_str();
    const tchar *e = p + params.length();
    while (p != e)
    {
       const tchar *s = p;
       while (s != e && *s == L' ') s++;
       if (s != e) {
           while (s != e && *s != L' ') s++;
       }
       cmd->parameters_list.push_back(tstring(p, s-p));
       p = s;
    }
    if (!cmd->parameters_list.empty())
    {
        tstring &p = cmd->parameters_list[0];
        if (p.at(0) == L' ') {
            p = p.substr(1);
        }
    }
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

bool InputTemplateCommands::markbrackets(tstring *cmd) const
{
    bool marker_used = false;
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
        // check space or bracket before open bracket
        if (!bracket_begin)
        {
            if (p != b0)
            {
                const tchar* p1 = p-1;
                if (!iscloseorspace(p1) )
                    { p++; continue; }
            }
        }
        // check space after close bracket
        else
        {
            if (*p != '{' && p+1 != e)
            {
                const tchar* p1 = p+1;
                if (!isbracketorspace(p1) && *p1 != _params.separator)
                    { p++; continue; }
            }
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
               marker_used = true;
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
    return marker_used;
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

bool InputTemplateCommands::isopenorspace(const tchar *p) const
{
    return (wcschr(L"{ \"'", *p)) ? true : false;
}

bool InputTemplateCommands::iscloseorspace(const tchar *p) const
{
    return (wcschr(L"} \"'", *p)) ? true : false;
}

bool InputTemplateCommands::isbracketorspace(const tchar *p) const
{
    if (*p == L' ') return true;
    return isbracket(p);
}
