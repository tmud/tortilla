#include "stdafx.h"
#include "jmc3ImportImpl.h"
#include "../mudclient/src/common/selectFileDlg.h"
#include "paramsDlg.h"

Jmc3Import::Jmc3Import(lua_State *pL) : m_aliases(pL, "aliases"), m_actions(pL, "actions"), m_subs(pL, "subs"), m_antisubs(pL, "antisubs"),
m_highlights(pL, "highlights"), m_hotkeys(pL, "hotkeys"), m_gags(pL, "gags"), m_vars(pL, "vars"), m_groups(pL, "groups")
{
    L = pL;
    initPcre();
    initCmdSymbols();
    initJmcCommands();
    initLegacy();
}
Jmc3Import::~Jmc3Import() {}

bool Jmc3Import::import(HWND parent_for_dlgs, std::vector<u8string>* errors)
{
    m_parent = parent_for_dlgs;

    ParamsDialog params;
    if (params.DoModal(m_parent) == IDCANCEL)
        return false;

/*    // get jmc cmd separator
    for (int i = 0, e = import.size(); i < e; ++i)
    {

    }

    // get jmc cmd prefix
    for (int i=0, e=import.size(); i<e; ++i)
    {
        base.find(import[i].c_str());
        if (!base.size())
            continue;
        u8string c, p;
        base.get(1, &jmc_prefix); // command prefix
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
            errors->push_back(import[i]);
    }
    // update all elements, through updating groups
    m_groups.update();*/
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

void Jmc3Import::convert(u8string *str)
{
    TU2W tmp(str->c_str());
    OutputDebugString(tmp);
    OutputDebugString(L"\r\n");
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

void Jmc3Import::initLegacy()
{
    std::map<u8string, u8string>& l = m_legacy;
    l["daa"] = "hide";
    l["restorewindow"] = "showwindow";
    l["showme"] = "output";
    l["substitute"] = "sub";
    l["antisubstitute"] = "antisub";
    l["unantisubstitute"] = "unantisub";
    l["tabadd"] = "tab";
    l["tabdel"] = "untab";
    l["variable"] = "var";
    l["%%"] = "%";
}

void Jmc3Import::initPcre()
{
    base.init("^(\\W)(.*?) +(.*) *");
    param.init("\\{((?:(?>[^{}]+)|(?R))*)\\}");

    pcre_jmcSeparator.init("(\\W)");

}

void Jmc3Import::initCmdSymbols()
{
    lua_getglobal(L, "props");
    luaT_run(L, "cmdPrefix", "t");
    prefix.assign(lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_getglobal(L, "props");
    luaT_run(L, "cmdSeparator", "t");
    separator.assign(lua_tostring(L, -1));
    lua_pop(L, 1);
}

void Jmc3Import::initJmcCommands()
{
    std::vector<u8string> &v = m_jmc_commands;
    v.push_back("hide");
}

bool Jmc3Import::isJmcCommand(const u8string& str)
{
    const std::vector<u8string>::iterator it = std::find(m_jmc_commands.begin(), m_jmc_commands.end(), str);
    return (it == m_jmc_commands.end()) ? false : true;
}
