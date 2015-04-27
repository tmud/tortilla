#pragma once

class Jmc3Import
{
    HWND m_parent;
    Pcre base, param, ifcmd;
    std::map<u8string, u8string> m_legacy;
    std::map<u8string, u8string> m_commands;
    typedef std::map<u8string, u8string>::iterator iterator;
    luaT_ActiveObjects m_aliases, m_actions, m_subs, m_antisubs, m_highlights, m_hotkeys, m_gags, m_vars, m_groups;
    lua_State *L;
    u8string cmdsymbol, separator;
    u8string jmc_cmdsymbol, jmc_separator;
    bool rewrite_mode;

public:
    Jmc3Import(lua_State *pL);
    ~Jmc3Import();
    bool import(HWND parent_for_dlgs, std::vector<u8string>* errors);

private:
    void initPcre();
    void initLegacy();
    void initCmdSymbols();
    bool parseParams(int min, int max, std::vector<u8string> *params);
    bool processAlias();
    bool processAction();
    bool processSubs();
    bool processAntisub();
    bool processHighlight();
    bool processHotkey();
    bool processGags();
    bool processVariable();
    bool convert(u8string *str);
    void replaceLegacy(u8string *legacy);
    void replaceCommand(u8string *cmds);
};
