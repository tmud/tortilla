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

bool Jmc3Import::import(HWND parent_for_dlgs, std::vector<std::wstring>* errors)
{
    m_parent = parent_for_dlgs;
    ParamsDialog params;
    if (params.DoModal(m_parent) == IDCANCEL)
        return false;
    jmc_cmdsymbol = params.cmdsymbol;
    jmc_separator = params.separator;
    rewrite_mode = params.rewrite_mode;

    HCURSOR cursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));

    std::vector<std::wstring> &v = params.strings;

    std::vector<std::wstring> disabled_groups;

    // get jmc cmd prefix
    for (int i=0,e=v.size(); i<e; ++i)
    {
        base.find(v[i].c_str());
        if (!base.size())
            continue;
        std::wstring cmdsymbol;
        base.get(1, &cmdsymbol);
        if (cmdsymbol != params.cmdsymbol)
            continue;

        std::wstring c, p;
        base.get(2, &c);          // command
        base.get(3, &p);          // params

        param.findall(p.c_str());
        if (!param.size())        //simple options
        {
            if (c == L"group" && disable_group.find(p.c_str()) )
            {
                std::wstring group;
                disable_group.get(1, &group);
                disabled_groups.push_back(group);            
            }
            continue;
        }

        bool result = true;
        if (c == L"action")
            result = processAction();
        else if (c == L"alias")
            result = processAlias();
        else if (c == L"substitute")
            result = processSubs();
        else if (c == L"antisubstitute")
            result = processAntisub();
        else if (c == L"highlight")
            result = processHighlight();
        else if (c == L"hot")
            result = processHotkey();
        else if (c == L"gag")
            result = processGags();
        else if (c == L"variable")
            result = processVariable();
        if (!result && errors)
            errors->push_back(v[i]);
    }    
    for (int i=0,e=disabled_groups.size(); i<e; ++i)
    {
        for (int j=1,je=m_groups.size(); j<=je; ++j)
        {
            m_groups.select(j);
            std::wstring group;
            m_groups.get(luaT_ActiveObjects::KEY, &group);
            if (group == disabled_groups[i])
            {
                m_groups.set(luaT_ActiveObjects::VALUE, L"0");
                break;
            }
        }
    }

    SetCursor(cursor);

    // update all elements, through updating groups
    m_groups.update();
    return true;
}

bool Jmc3Import::parseParams(int min, int max, std::vector<std::wstring> *params)
{
    int n = param.size()-1;
    if (n < min || n > max) return false;
    for (int i=1; i<=n; i++)
    {
        std::wstring t;
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
    std::vector<std::wstring> p;
    if (!parseParams(3, 3, &p))
        return false;
    convert(&p[1]);
    return(rewrite_mode) ? m_aliases.replace(p[0].c_str(), p[1].c_str(), p[2].c_str()) :
     m_aliases.add(p[0].c_str(), p[1].c_str(), p[2].c_str());
}

bool Jmc3Import::processAction()
{
    std::vector<std::wstring> p;
    if (!parseParams(4, 4, &p))
        return false;
    convert(&p[1]);
    replaceDoubles(&p[0]);
    return (rewrite_mode) ? m_actions.replace(p[0].c_str(), p[1].c_str(), p[3].c_str()) :
        m_actions.add(p[0].c_str(), p[1].c_str(), p[3].c_str());
}

bool Jmc3Import::processSubs()
{
    std::vector<std::wstring> p;
    if (!parseParams(2, 2, &p))
        return false;
    replaceDoubles(&p[0]);
    return (rewrite_mode) ? m_subs.replace(p[0].c_str(), p[1].c_str(), L"default") :
        m_subs.add(p[0].c_str(), p[1].c_str(), L"default");
}

bool Jmc3Import::processAntisub()
{
    std::vector<std::wstring> p;
    if (!parseParams(1, 1, &p))
        return false;
    replaceParams(&p[0]);
    return (rewrite_mode) ? m_antisubs.replace(p[0].c_str(), NULL, L"default") :
        m_antisubs.add(p[0].c_str(), NULL, L"default");
}

bool Jmc3Import::processHotkey()
{
    std::vector<std::wstring> p;
    if (!parseParams(2, 3, &p)) 
        return false;
    std::wstring group(L"default");
    int ps = p.size();
    if (ps == 3)
        group = p[2];
    convert(&p[1]);
    return (rewrite_mode) ? m_hotkeys.replace(p[0].c_str(), p[1].c_str(), group.c_str()) :
        m_hotkeys.add(p[0].c_str(), p[1].c_str(), group.c_str());
}

bool Jmc3Import::processGags()
{
    std::vector<std::wstring> p;
    if (!parseParams(1, 1, &p))
        return false;
    replaceParams(&p[0]);
    return (rewrite_mode) ? m_gags.replace(p[0].c_str(), NULL, L"default") :
        m_gags.add(p[0].c_str(), NULL, L"default");
}

bool Jmc3Import::processHighlight()
{
    std::vector<std::wstring> p;
    if (!parseParams(3, 3, &p))
        return false;
    replaceParams(&p[1]);
    return (rewrite_mode) ? m_highlights.replace(p[1].c_str(), p[0].c_str(), p[2].c_str()) :
        m_highlights.add(p[1].c_str(), p[0].c_str(), p[2].c_str());
}

bool Jmc3Import::processVariable()
{
    std::vector<std::wstring> p;
    if (!parseParams(2, 2, &p)) 
        return false;
    return (rewrite_mode) ?  m_vars.replace(p[0].c_str(), p[1].c_str(), NULL) :
        m_vars.add(p[0].c_str(), p[1].c_str(), NULL);
}

bool Jmc3Import::convert(std::wstring *str)
{
    std::vector<std::wstring> cmds;
    bool params_exists = param.findall(str->c_str());
    Pcre find_separators;
    std::wstring regexp(L"\\");
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
            std::wstring cmd(str->substr(startpos, pos[i] - startpos));
            wstring_helper t(cmd); t.trim();
            if (!cmd.empty())
                cmds.push_back(cmd);
            startpos = pos[i] + 1;
        }
        std::wstring cmd(str->substr(startpos));
        wstring_helper t(cmd); t.trim();
        if (!cmd.empty())
            cmds.push_back(cmd);
    }

    str->clear();
    std::wstring default_cmdsymbol(L"#");

    // support import #wait command
    std::wstring waitcmd(jmc_cmdsymbol);
    waitcmd.append(L"wait");
    int last = cmds.size()-1;
    for (int i=last; i>=0; --i)
    {
        std::wstring &cmd = cmds[i];
        if (cmd.find(waitcmd) != -1 && i != last)
        {
            cmd.append(L" {");
            cmd.append(cmds[i+1]);
            cmd.append(L"}");
            cmds.erase(cmds.begin()+(i+1));
            last = cmds.size()-1;
        }
    }

    for (int i=0, e=cmds.size(); i<e; ++i)
    {
        if (i != 0)
            str->append(separator);

        std::wstring s(cmds[i]);
        wstring_helper t(s); t.trim();
        replaceLegacy(&s);
        wchar_t tmp[2] = { s.at(0), 0 };
        std::wstring p(tmp);

        if (jmc_cmdsymbol.compare(p) && default_cmdsymbol.compare(p)) 
             str->append(s);        // it game command
        else
        {
            std::wstring new_cmd(cmdsymbol);
            if (!ifcmd.find(s.c_str())) 
            {
                if (!base.find(s.c_str()))
                    new_cmd.append(s.substr(p.length()));
                else {
                std::wstring curr_cmd;
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
              std::wstring if_cmds;
              param.get(2, &if_cmds);
              int right_index = param.first(2);
              int len = if_cmds.length() - 2;
              if_cmds.assign(if_cmds.substr(1, len));
              if (!convert(&if_cmds))
                  return false;
              int pos = p.length(); // postition after prefix
              new_cmd.append(s.substr(pos, right_index-pos));
              new_cmd.append(L"{");
              new_cmd.append(if_cmds);
              new_cmd.append(L"}");
            }
            str->append(new_cmd);
        }
    }
    return true;
}

void Jmc3Import::replaceDoubles(std::wstring* str)
{
    params.findall(str->c_str());
    if (params.size() == 0)
        return;
    std::vector<int> ids;
    int maxid = -1;
    for (int i = 1, e = params.size(); i<e; ++i)
    {
        int pos = params.first(i) + 1;
        wchar_t symbol = str->at(pos);
        int id = symbol - L'0';
        ids.push_back(id);
        if (id > maxid)
            maxid = id;
    }
    if (maxid == -1) return;

    std::vector<int> indexes(maxid + 1, 0);
    for (int i = ids.size() - 1; i >= 0; --i)
    {
        int index = ids[i];
        if (index == -1) continue;
        if (indexes[index] != 0)
            ids[i] = -1;
        indexes[index]++;
    }

    std::wstring result;
    int pos = 0;
    for (int i = 1, e = params.size(); i < e; ++i)
    {
        int id = ids[i-1];
        result.append(str->substr(pos, params.first(i) - pos));
        if (id == -1)
            result.append(L"%%");
        else
        {
            std::wstring tmp;
            params.get(i, &tmp);
            result.append(tmp);
        }
        pos = params.last(i);
    }
    result.append(str->substr(pos));
    str->swap(result);
}

void Jmc3Import::replaceParams(std::wstring* str)
{
    params.findall(str->c_str());
    if (params.size() == 0)
        return;
    std::wstring result;
    int pos = 0;
    for (int i=1, e=params.size(); i<e; ++i)
    {
        result.append(str->substr(pos, params.first(i) - pos));
        result.append(L"%%");
        pos = params.last(i);
    }
    result.append(str->substr(pos));
    str->swap(result);
}

void Jmc3Import::replaceLegacy(std::wstring *legacy)
{
    iterator it = m_legacy.begin(), it_end = m_legacy.end();
    for (; it!=it_end; ++it)
    {
        int pos = legacy->find(it->first);
        while (pos != -1)
        {         
            std::wstring newstr(legacy->substr(0, pos));
            newstr.append(it->second);
            pos = pos + it->first.length();
            newstr.append(legacy->substr(pos));
            legacy->swap(newstr);
            pos = legacy->find(it->first);
        }
    }
}

void Jmc3Import::replaceCommand(std::wstring *cmd)
{
    if (cmd->length() < 3)
        return;
     iterator it = m_commands.begin(), it_end = m_commands.end();
     for (;it!=it_end;++it)
     {
         const std::wstring& key = it->first;
         if (!wcsncmp(key.c_str(), cmd->c_str(), cmd->length()))
         {
            cmd->assign(it->second);
            return;
         }
     }
}

void Jmc3Import::initLegacy()
{
    std::map<std::wstring, std::wstring>& l = m_legacy;
    l[L"%%"] = L"%";
    std::map<std::wstring, std::wstring>& c = m_commands;
    c[L"daa"] = L"hide";
    c[L"restorewindow"] = L"showwindow";
    c[L"showme"] = L"woutput 1";
    c[L"substitute"] = L"sub";
    c[L"antisubstitute"] = L"antisub";
    c[L"unantisubstitute"] = L"unantisub";
    c[L"tabadd"] = L"tab";
    c[L"tabdel"] = L"untab";
    c[L"variable"] = L"var";
    c[L"output"] = L"woutput 1";
}

void Jmc3Import::initPcre()
{
    base.init(L"^(\\W)(.*?) +(.*) *");
    param.init(L"\\{((?:(?>[^{}]+)|(?R))*)\\}");
    ifcmd.init(L"^.if .*");
    disable_group.init(L"disable (.*)");
    params.init(L"(%[0-9]){1}");
}

void Jmc3Import::initCmdSymbols()
{
    luaT_Props p(L);
    p.cmdPrefix(&cmdsymbol);
    p.cmdSeparator(&separator);
}
