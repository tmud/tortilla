#include "stdafx.h"
#include "jmc3ImportImpl.h"
#include "../mudclient/src/common/selectFileDlg.h"

class AutoCloseHandle {
    HANDLE hfile;
public:
AutoCloseHandle(HANDLE file) : hfile(file) {}
~AutoCloseHandle() { CloseHandle(hfile); }
};

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
    SelectFileDlg dlg(m_parent, L"JMC3 config set(*.set)|*.set||");
    if (dlg.DoModal())
    {
        std::wstring file(dlg.GetFile());
        HANDLE hfile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            return errorBox("Невозможно прочитать данный файл!");

        AutoCloseHandle _ach(hfile);

        DWORD high = 0;
        DWORD size = GetFileSize(hfile, &high);
        if (high != 0 || size > (128 * 1024))
            return errorBox("Файл слишком большого размера!");
        if (size == 0)
            return errorBox("Файл пустой!");

        std::vector <std::string> config;
        DataQueue dq;
        int buffer_size = 1024;
        MemoryBuffer buffer(buffer_size);
        while (size > 0)
        {
            DWORD toread = buffer_size;
            if (toread > size) toread = size;
            DWORD readed = 0;
            if (!ReadFile(hfile, buffer.getData(), toread, &readed, NULL) || readed != toread)
                return errorBox("Ошибка чтения файла!");
            dq.write(buffer.getData(), toread);
            parseQueue(dq, config);
            size -= toread;
        }
        char x = 0xa; dq.write(&x, 1);
        parseQueue(dq, config);

        for (int i = 0, e = config.size(); i < e; ++i)
        {
            TA2W wide(config[i].c_str());
            TW2U str(wide);
            parseString(u8string(str), errors);
        }

        // update all elements, through updating groups
        m_groups.update();
        return true;
    }
    return false;
}

void Jmc3Import::parseQueue(DataQueue &dq, std::vector<std::string>& out)
{
    const char* b = (const char*)dq.getData();
    const char* e = b + dq.getSize();
    const char* p = b;
    while (p != e)
    {
        for (;p != e; ++p)
        {
            if (*p == 0xd || *p == 0xa)
                break;
        }

        if (p != e)
        {
           std::string label(b, p-b);
           if (!label.empty())
                out.push_back(label);
           p++;
           b = p;
        }
    }
    const char* b0 = (const char*)dq.getData();
    dq.truncate(b-b0);
}

void Jmc3Import::parseString(const u8string& str, std::vector<u8string>* errors)
{  
    base.find(str.c_str());
    if (!base.size())
        return;

    u8string c, p;
    base.get(1, &jmc_prefix); // jmc cmd prefix
    base.get(2, &c);          // command
    base.get(3, &p);          // params

    param.findall(p.c_str());
    if (!param.size())
    {
        //simple options
        return;
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
        errors->push_back(str);
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
    base.init("^(.)(.*?) +(.*) *");
    param.init("\\{((?:(?>[^{}]+)|(?R))*)\\}");
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

bool Jmc3Import::errorBox(const utf8* msg)
{
    MessageBox(m_parent, TU2W(msg), L"Jmc3 Import", MB_OK | MB_ICONSTOP);
    return false;
}
