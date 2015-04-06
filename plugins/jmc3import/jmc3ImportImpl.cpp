#include "stdafx.h"
#include "jmc3ImportImpl.h"
#include "../mudclient/src/common/selectFileDlg.h"
#include "paramsDlg.h"

void trim(u8string *str)
{
    int pos = strspn(str->c_str(), " ");
    if (pos != 0)
        str->assign(str->substr(pos));
    if (str->empty())
        return;
    int last = str->size() - 1;
    pos = last;
    while (str->at(pos) == ' ')
        pos--;
    if (pos != last)
        str->assign(str->substr(0, pos + 1));
}

Jmc3Import::Jmc3Import(lua_State *pL) : m_aliases(pL, "aliases"), m_actions(pL, "actions"), m_subs(pL, "subs"), m_antisubs(pL, "antisubs"),
m_highlights(pL, "highlights"), m_hotkeys(pL, "hotkeys"), m_gags(pL, "gags"), m_vars(pL, "vars"), m_groups(pL, "groups")
{
    L = pL;
    initPcre();
    initCmdSymbols();
    initLegacy();
}
Jmc3Import::~Jmc3Import() {}

bool Jmc3Import::import(HWND parent_for_dlgs, std::vector<u8string>* errors)
{
    m_parent = parent_for_dlgs;
    ParamsDialog params;
    if (params.DoModal(m_parent) == IDCANCEL)
        return false;
    jmc_cmdsymbol = params.cmdsymbol;
    jmc_separator = params.separator;

    std::vector<u8string> &v = params.strings;

    // get jmc cmd prefix
    for (int i=0,e=v.size(); i<e; ++i)
    {
        base.find(v[i].c_str());
        if (!base.size())
            continue;
        u8string cmdsymbol;
        base.get(1, &cmdsymbol);
        if (cmdsymbol != params.cmdsymbol)
            continue;

        u8string c, p;
        base.get(2, &c);          // command
        base.get(3, &p);          // params

        param.findall(p.c_str());
        if (!param.size())        //simple options
            continue;

        bool result = true;
        if (c == "action")
            result = processAction();
        else if (c == "alias")
            result = processAlias();
        else if (c == "substitute")
            result = processSubs();
        else if (c == "antisubstitute")
            result = processAntisub();
        else if (c == "highlight")
            result = processHighlight();
        else if (c == "hot")
            result = processHotkey();
        else if (c == "gag")
            result = processGags();
        else if (c == "variable")
            result = processVariable();
        if (!result && errors)
            errors->push_back(v[i]);
    }

    // update all elements, through updating groups
    m_groups.update();
    return true;
}

bool Jmc3Import::parseParams(int min, int max, std::vector<u8string> *params)
{
    int n = param.size()-1;
    if (n < min || n > max) return false;
    for (int i=1; i<=n; i++)
    {
        u8string t;
        param.get(i, &t);
        int l = t.size()-2;
        if (l <= 0)
            return false;
        t = t.substr(1, l);
        params->push_back(t);
    }
    return true;
}

bool Jmc3Import::processAlias()
{
    std::vector<u8string> p;
    if (!parseParams(3, 3, &p))
        return false;
    convert(&p[1]);
    return m_aliases.add(p[0].c_str(), p[1].c_str(), p[2].c_str());
}

bool Jmc3Import::processAction()
{
    std::vector<u8string> p;
    if (!parseParams(4, 4, &p))
        return false;
    convert(&p[1]);
    return m_actions.add(p[0].c_str(), p[1].c_str(), p[3].c_str());
}

bool Jmc3Import::processSubs()
{
    std::vector<u8string> p;
    if (!parseParams(2, 2, &p))
        return false;
    return m_subs.add(p[0].c_str(), p[1].c_str(), "default");
}

bool Jmc3Import::processAntisub()
{
    std::vector<u8string> p;
    if (!parseParams(1, 1, &p))
        return false;
    return m_antisubs.add(p[0].c_str(), NULL, "default");
}

bool Jmc3Import::processHotkey()
{
    std::vector<u8string> p;
    if (!parseParams(2, 3, &p)) 
        return false;
    u8string group("default");
    int ps = p.size();
    if (ps == 3)
        group = p[2];
    convert(&p[1]);
    return m_hotkeys.add(p[0].c_str(), p[1].c_str(), group.c_str());
}

bool Jmc3Import::processGags()
{
    std::vector<u8string> p;
    if (!parseParams(1, 1, &p))
        return false;
    return m_gags.add(p[0].c_str(), NULL, "default");
}

bool Jmc3Import::processHighlight()
{
    std::vector<u8string> p;
    if (!parseParams(3, 3, &p))
        return false;
    return m_highlights.add(p[1].c_str(), p[0].c_str(), p[2].c_str());
}

bool Jmc3Import::processVariable()
{
    std::vector<u8string> p;
    if (!parseParams(2, 2, &p)) 
        return false;
    return m_vars.add(p[0].c_str(), p[1].c_str(), NULL);
}

bool Jmc3Import::convert(u8string *str)
{
    std::vector<u8string> cmds;
    if (!param.findall(str->c_str()))
        cmds.push_back(*str);
    else
    {
        Pcre find_separators;
        u8string regexp("\\");
        regexp.append(jmc_separator);
        find_separators.init(regexp.c_str());
        if (!find_separators.findall(str->c_str()))
            cmds.push_back(*str);
        else
        {
            std::vector<int> pos;
            for (int i=1, e=find_separators.size(); i < e; ++i)
            {
                int sep_pos = find_separators.first(i);
                bool inside = false;
                for (int j=1, je = param.size(); j < je; ++j)
                {
                    if (sep_pos >= param.first(j) && sep_pos < param.last(j))
                        { inside = true; break; }
                }
                if (!inside)
                    pos.push_back(sep_pos);
            }
            int startpos = 0;
            for (int i=0, e=pos.size(); i<e; ++i)
            {
                cmds.push_back(str->substr(startpos, pos[i] - startpos));
                startpos = pos[i] + 1;
            }
            cmds.push_back(str->substr(startpos));
        }
    }

    str->clear();
    u8string default_cmdsymbol("#");
    for (int i=0, e=cmds.size(); i<e; ++i)
    {
        if (i != 0)
            str->append(separator);

        u8string &s = cmds[i];
        trim(&s);
        replaceLegacy(&s);
        utf8 p[2] = { s.at(0), 0 };
        if (jmc_cmdsymbol.compare(p) && default_cmdsymbol.compare(p)) // not command
             str->append(s);
        else
        {
            u8string new_cmd(cmdsymbol);
            if (!ifcmd.find(s.c_str())) 
            {
                if (!base.find(s.c_str()))
                    new_cmd.append(s.substr(1));
                else {
                u8string curr_cmd;
                base.get(2, &curr_cmd);
                replaceCommand(&curr_cmd);
                new_cmd.append(curr_cmd);
                new_cmd.append(s.substr(base.last(2)));
                }
            }
            else
            { // if command -> recursive process
              if (!param.findall(s.c_str()) || param.size() != 3)
                  return false;
              u8string if_cmds;
              param.get(2, &if_cmds);
              int right_index = param.first(2);
              int len = if_cmds.length() - 2;
              if_cmds.assign(if_cmds.substr(1, len));
              if (!convert(&if_cmds))
                  return false;
              new_cmd.append(s.substr(1, right_index-1));
              new_cmd.append("{");
              new_cmd.append(if_cmds);
              new_cmd.append("}");
            }
            str->append(new_cmd);
        }
    }
    return true;
}

void Jmc3Import::replaceLegacy(u8string *legacy)
{
    iterator it = m_legacy.begin(), it_end = m_legacy.end();
    for (; it!=it_end; ++it)
    {
        int pos = legacy->find(it->first);
        if (pos == -1)
            continue;
        u8string newstr(legacy->substr(0, pos));
        newstr.append(it->second);
        pos = pos + it->first.length();
        newstr.append(legacy->substr(pos));
        legacy->swap(newstr);
    }
}

void Jmc3Import::replaceCommand(u8string *cmd)
{
     iterator it = m_commands.find(*cmd);
     if (it != m_commands.end())
         cmd->assign(it->second);
}

void Jmc3Import::initLegacy()
{
    std::map<u8string, u8string>& l = m_legacy;
    l["%%"] = "%";
    l = m_commands;
    l["daa"] = "hide";
    l["restorewindow"] = "showwindow";
    l["showme"] = "output";
    l["substitute"] = "sub";
    l["antisubstitute"] = "antisub";
    l["unantisubstitute"] = "unantisub";
    l["tabadd"] = "tab";
    l["tabdel"] = "untab";
    l["variable"] = "var";    
}

void Jmc3Import::initPcre()
{
    base.init("^(\\W)(.*?) +(.*) *");
    param.init("\\{((?:(?>[^{}]+)|(?R))*)\\}");
    ifcmd.init("^.if .*");
}

void Jmc3Import::initCmdSymbols()
{
    luaT_Props p(L);
    p.cmdPrefix(&cmdsymbol);
    p.cmdSeparator(&separator);
}
