#pragma once

class Jmc3Import
{
    HWND m_parent;
    Pcre base, param, ifcmd, disable_group, params, regul, varcmd;
    std::map<std::wstring, std::wstring> m_legacy;
    std::map<std::wstring, std::wstring> m_commands;
    typedef std::map<std::wstring, std::wstring>::iterator iterator;
    luaT_ActiveObjects m_aliases, m_actions, m_subs, m_antisubs, m_highlights, m_hotkeys, m_gags, m_vars, m_groups;
    lua_State *L;
    std::wstring cmdsymbol, separator;
    std::wstring jmc_cmdsymbol, jmc_separator;
    bool rewrite_mode;

public:
    Jmc3Import(lua_State *pL);
    ~Jmc3Import();
    bool import(HWND parent_for_dlgs, std::vector<std::wstring>* errors);

private:
    void initPcre();
    void initLegacy();
    void initCmdSymbols();
    bool parseParams(int min, int max, std::vector<std::wstring> *params);
    bool processAlias();
    bool processAction();
    bool processSubs();
    bool processAntisub();
    bool processHighlight();
    bool processHotkey();
    bool processGags();
    bool processVariable();
    bool convert(std::wstring *str);
    void replaceDoubles(std::wstring* str);
    bool replaceRegular(std::wstring* str);
    void replaceParams(std::wstring* str);
    void replaceLegacy(std::wstring *legacy);
    void replaceCommand(std::wstring *cmds);
private: // fix separate commands
    void initSeparateCmdsPcre();
    void fixSeparateCmd(std::wstring* cmd);
    void fixHotkeysBrackets(std::wstring* cmd);
    void fixStatusCmd(std::wstring* cmd);

private:
    Pcre hotkey_pcre;
    Pcre status_pcre;
};
