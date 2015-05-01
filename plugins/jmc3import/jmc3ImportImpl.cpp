#include "stdafx.h"
#include "jmc3ImportImpl.h"
#include "../mudclient/src/common/selectFileDlg.h"
#include "paramsDlg.h"

Jmc3Import::Jmc3Import(lua_State *pL) : m_aliases(pL, "aliases"), m_actions(pL, "actions"), m_subs(pL, "subs"), m_antisubs(pL, "antisubs"),
m_highlights(pL, "highlights"), m_hotkeys(pL, "hotkeys"), m_gags(pL, "gags"), m_vars(pL, "vars"), m_groups(pL, "groups"), rewrite_mode(false)
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
    rewrite_mode = params.rewrite_mode;

    HCURSOR cursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));

    std::vector<u8string> &v = params.strings;

    std::vector<u8string> disabled_groups;

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
        {
            if (c == "group" && disable_group.find(p.c_str()) )
            {
                u8string group;
                disable_group.get(1, &group);
                disabled_groups.push_back(group);            
            }
            continue;
        }

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
    for (int i=0,e=disabled_groups.size(); i<e; ++i)
    {
        for (int j=1,je=m_groups.size(); j<=je; ++j)
        {
            m_groups.select(j);
            u8string group;
            m_groups.get(luaT_ActiveObjects::KEY, &group);
            if (group == disabled_groups[i])
            {
                m_groups.set(luaT_ActiveObjects::VALUE, "0");
                break;
            }
        }
    }

    SetCursor(cursor);

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
    return(rewrite_mode) ? m_aliases.replace(p[0].c_str(), p[1].c_str(), p[2].c_str()) :
     m_aliases.add(p[0].c_str(), p[1].c_str(), p[2].c_str());
}

bool Jmc3Import::processAction()
{
    std::vector<u8string> p;
    if (!parseParams(4, 4, &p))
        return false;
    convert(&p[1]);
    return (rewrite_mode) ? m_actions.replace(p[0].c_str(), p[1].c_str(), p[3].c_str()) :
        m_actions.add(p[0].c_str(), p[1].c_str(), p[3].c_str());
}

bool Jmc3Import::processSubs()
{
    std::vector<u8string> p;
    if (!parseParams(2, 2, &p))
        return false;
    return (rewrite_mode) ? m_subs.replace(p[0].c_str(), p[1].c_str(), "default") :
        m_subs.add(p[0].c_str(), p[1].c_str(), "default");
}

bool Jmc3Import::processAntisub()
{
    std::vector<u8string> p;
    if (!parseParams(1, 1, &p))
        return false;
    return (rewrite_mode) ? m_antisubs.replace(p[0].c_str(), NULL, "default") :
        m_antisubs.add(p[0].c_str(), NULL, "default");
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
    return (rewrite_mode) ? m_hotkeys.replace(p[0].c_str(), p[1].c_str(), group.c_str()) :
        m_hotkeys.add(p[0].c_str(), p[1].c_str(), group.c_str());
}

bool Jmc3Import::processGags()
{
    std::vector<u8string> p;
    if (!parseParams(1, 1, &p))
        return false;
    return (rewrite_mode) ? m_gags.replace(p[0].c_str(), NULL, "default") :
        m_gags.add(p[0].c_str(), NULL, "default");
}

bool Jmc3Import::processHighlight()
{
    std::vector<u8string> p;
    if (!parseParams(3, 3, &p))
        return false;
    return (rewrite_mode) ? m_highlights.replace(p[1].c_str(), p[0].c_str(), p[2].c_str()) :
        m_highlights.add(p[1].c_str(), p[0].c_str(), p[2].c_str());
}

bool Jmc3Import::processVariable()
{
    std::vector<u8string> p;
    if (!parseParams(2, 2, &p)) 
        return false;
    return (rewrite_mode) ?  m_vars.replace(p[0].c_str(), p[1].c_str(), NULL) :
        m_vars.add(p[0].c_str(), p[1].c_str(), NULL);
}

bool Jmc3Import::convert(u8string *str)
{
    std::vector<u8string> cmds;
    bool params_exists = param.findall(str->c_str());
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
            if (params_exists)
            {
                for (int j=1, je = param.size(); j < je; ++j)
                {
                    if (sep_pos >= param.first(j) && sep_pos < param.last(j))
                        { inside = true; break; }
                }
            }
            if (!inside)
                pos.push_back(sep_pos);
        }
        int startpos = 0;
        for (int i=0, e=pos.size(); i<e; ++i)
        {
            u8string cmd(str->substr(startpos, pos[i] - startpos));
            U8 t(cmd); t.trim();
            if (!cmd.empty())
                cmds.push_back(cmd);
            startpos = pos[i] + 1;
        }
        u8string cmd(str->substr(startpos));
        U8 t(cmd); t.trim();
        if (!cmd.empty())
            cmds.push_back(cmd);
    }

    str->clear();
    u8string default_cmdsymbol("#");
    for (int i=0, e=cmds.size(); i<e; ++i)
    {
        if (i != 0)
            str->append(separator);

        u8string s(cmds[i]);
        U8 t(s); t.trim();
        replaceLegacy(&s);
        u8string p(t.at(0));

        if (jmc_cmdsymbol.compare(p) && default_cmdsymbol.compare(p)) 
             str->append(s);        // it game command
        else
        {
            u8string new_cmd(cmdsymbol);
            if (!ifcmd.find(s.c_str())) 
            {
                if (!base.find(s.c_str()))
                    new_cmd.append(s.substr(p.length()));
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
              int pos = p.length(); // postition after prefix
              new_cmd.append(s.substr(pos, right_index-pos));
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
        while (pos != -1)
        {         
            u8string newstr(legacy->substr(0, pos));
            newstr.append(it->second);
            pos = pos + it->first.length();
            newstr.append(legacy->substr(pos));
            legacy->swap(newstr);
            pos = legacy->find(it->first);
        }
    }
}

void Jmc3Import::replaceCommand(u8string *cmd)
{
    if (cmd->length() < 3)
        return;
     iterator it = m_commands.begin(), it_end = m_commands.end();
     for (;it!=it_end;++it)
     {
         const u8string& key = it->first;
         if (!strncmp(key.c_str(), cmd->c_str(), cmd->length()))
         {
            cmd->assign(it->second);
            return;
         }
     }
}

void Jmc3Import::initLegacy()
{
    std::map<u8string, u8string>& l = m_legacy;
    l["%%"] = "%";
    std::map<u8string, u8string>& c = m_commands;    
    c["daa"] = "hide";
    c["restorewindow"] = "showwindow";
    c["showme"] = "output";
    c["substitute"] = "sub";
    c["antisubstitute"] = "antisub";
    c["unantisubstitute"] = "unantisub";
    c["tabadd"] = "tab";
    c["tabdel"] = "untab";
    c["variable"] = "var";    
}

void Jmc3Import::initPcre()
{
    base.init("^(\\W)(.*?) +(.*) *");
    param.init("\\{((?:(?>[^{}]+)|(?R))*)\\}");
    ifcmd.init("^.if .*");
    disable_group.init("disable (.*)");
}

void Jmc3Import::initCmdSymbols()
{
    luaT_Props p(L);
    p.cmdPrefix(&cmdsymbol);
    p.cmdSeparator(&separator);
}
