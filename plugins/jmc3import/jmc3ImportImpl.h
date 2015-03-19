#pragma once

class Jmc3Import
{
    HWND m_parent;
    Pcre base, param;
    std::map<u8string, u8string> m_legacy;
    typedef std::map<u8string, u8string>::iterator iterator;
    luaT_ActiveObjects m_aliases, m_actions, m_subs, m_antisubs, m_highlights, m_hotkeys, m_gags, m_vars, m_groups;
    lua_State *L;

    std::vector<u8string> m_jmc_commands;

    u8string cmdsymbol, separator;
    u8string jmc_cmdsymbol, jmc_separator;

public:
    Jmc3Import(lua_State *pL);
    ~Jmc3Import();
    bool import(HWND parent_for_dlgs, std::vector<u8string>* errors);

private:
    void initPcre();
    void initLegacy();
    void initCmdSymbols();
    void initJmcCommands();
    bool isJmcCommand(const u8string& str);
    bool parseParams(int min, int max, std::vector<u8string> *params);
    bool processAlias();
    bool processAction();
    bool processSubs();
    bool processAntisub();
    bool processHighlight();
    bool processHotkey();
    bool processGags();
    bool processVariable();
    void convert(u8string *str);
    void replaceLegacy(u8string *legacy);
};
