#include "stdafx.h"
#include "tasks.h"
#pragma comment(lib, "lua.lib")

int system_messagebox(lua_State *L)
{
    HWND parent = base::getParent(L);
    if (!::IsWindow(parent))
        parent = NULL;

    bool params_ok = false;

    std::wstring text;
    std::wstring caption(L"Tortilla Mud Client");
    UINT buttons = MB_OK;
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        text.assign(luaT_towstring(L, 1));
        params_ok = true;
    }
    else if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(luaT_towstring(L, 1));
        text.assign(luaT_towstring(L, 2));
        params_ok = true;
    }
    else if (luaT_check(L, 3, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        caption.assign(luaT_towstring(L, 1));
        text.assign(luaT_towstring(L, 2));

        std::wstring b(luaT_towstring(L, 3));
        if (b == L"ok,cancel") buttons = MB_OKCANCEL;
        else if (b == L"cancel,ok") buttons = MB_OKCANCEL|MB_DEFBUTTON2;
        else if (b == L"yes,no") buttons = MB_YESNO;
        else if (b == L"no,yes") buttons = MB_YESNO|MB_DEFBUTTON2;

        if (wcsstr(b.c_str(), L"error")) buttons |= MB_ICONERROR;
        else if (wcsstr(b.c_str(), L"stop")) buttons |= MB_ICONERROR;
        else if (wcsstr(b.c_str(), L"info")) buttons |= MB_ICONINFORMATION;
        else if (wcsstr(b.c_str(), L"information")) buttons |= MB_ICONINFORMATION;
        else if (wcsstr(b.c_str(), L"warning")) buttons |= MB_ICONWARNING;
        else if (wcsstr(b.c_str(), L"question")) buttons |= MB_ICONQUESTION;

        params_ok = true;
    }
    UINT result = 0;
    if (params_ok)
        result = MessageBox(parent, text.c_str(), caption.c_str(), buttons);

    lua_pushinteger(L, result);
    return 1;
}

int system_debugstack(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::wstring label(luaT_towstring(L, -1));
        luaT_showLuaStack(L, label.c_str());
        return 0;
    }
    luaT_showLuaStack(L, NULL);
    return 0;
}

int system_dbgtable(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TTABLE, LUA_TSTRING))
    {
        std::wstring label(luaT_towstring(L, -1));
        luaT_showTableOnTop(L, label.c_str());
        return 0;
    }
    if (luaT_check(L, 1, LUA_TTABLE))
        luaT_showTableOnTop(L, NULL);
    return 0;
}

void formatByType(lua_State* L, int index, std::wstring *buf)
{
    int i = index;
    int type = lua_type(L, i);
    wchar_t dbuf[32];
    buf->clear();
    switch (type)
    {
    case LUA_TNIL:
        buf->append(L"nil");
        break;
    case LUA_TNUMBER:
        swprintf(dbuf, L"%d", lua_tointeger(L, i));
        buf->append(dbuf);
        break;
    case LUA_TBOOLEAN:
        swprintf(dbuf, L"%s", (lua_toboolean(L, i) == 0) ? L"false" : L"true");
        buf->append(dbuf);
        break;
    case LUA_TSTRING:
        buf->append(luaT_towstring(L, i));
        break;
    default:
        buf->append(L"[?]");
        break;
    }
}

int system_dbglog(lua_State *L)
{
    std::wstring msg;
    for (int i=1,e=lua_gettop(L); i<=e; ++i)
    {
        formatByType(L, i, &msg);
    }
    OutputDebugString(msg.c_str());
    return 0;
}

int system_sleep(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TNUMBER))
    {
        int wait = lua_tointeger(L, 1);
        if (wait > 0)
            ::Sleep(wait);
    }
    return 0;
}

class AutoCloseHandle {
    HANDLE hfile;
public:
    AutoCloseHandle(HANDLE file) : hfile(file) {}
    ~AutoCloseHandle() { CloseHandle(hfile); }
};

int system_loadTextFile(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::wstring filename( luaT_towstring(L, 1) );
        HANDLE file = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (file == INVALID_HANDLE_VALUE)
            return 0;
        AutoCloseHandle auto_close(file);
        DWORD high = 0;
        DWORD size = GetFileSize(file, &high);
        if (high != 0) return 0;

        lua_newtable(L);
        if (size == 0) return 1;

        typedef unsigned char uchar;

        int string_index = 1;
        std::string current_string;
        const DWORD buffer_size = 128;
        uchar* buffer = new uchar[buffer_size];

        DWORD inbuffer = 0;
        bool last_0d = false;
        bool bom_check = false;
        while (size > 0 || inbuffer > 0)
        {
            DWORD readed = inbuffer;
            if (size > 0) {
            DWORD toread = buffer_size - inbuffer;
            if (toread > size) toread = size;
            if (!ReadFile(file, buffer+inbuffer, toread, &readed, NULL))
                return 0;
            size -= readed;
            readed = inbuffer + readed;
            }

            uchar *p = buffer;
            uchar *e = p + readed;

            if (!bom_check && readed >= 3)
            {
                bom_check = true;
                if (p[0] == 0xef && p[1] == 0xbb && p[2] == 0xbf)
                    p += 3;
            }

            uchar *b = p;
            while (p != e)
            {
                uchar c = *p; p++;
                if (c == 0xd || c == 0xa)
                {
                    if (c == 0xa && last_0d)
                        { b = p; last_0d = false;  continue; }

                    current_string.append((char*)b, p-b-1);
                    lua_pushinteger(L, string_index++);
                    lua_pushstring(L, current_string.c_str());
                    lua_rawset(L, -3);
                    current_string.clear();

                    if (c == 0xd && last_0d)
                        { last_0d = false; break; }
                    if (c == 0xd)
                          last_0d = true;
                    break;
                }
                last_0d = false;

                if (c < ' ')
                {
                    uchar *x = p-1;
                    *x = ' ';
                    //current_string.append((char*)b, p-b-1);
                    //b = p;
                }
            }
            if (p == e)
                current_string.append((char*)b, e-b);
            DWORD processed = p - buffer;
            DWORD notprocessed = readed - processed;
            memcpy(buffer, p, notprocessed);
            inbuffer = notprocessed;
        }
        delete []buffer;
        if (!current_string.empty())
        {
            lua_pushinteger(L, string_index++);
            lua_pushstring(L, current_string.c_str());
            lua_rawset(L, -3);
        }
        return 1;
    }
    return 0;
}

int system_convertFromWin(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        TA2W t1(lua_tostring(L, 1));
        luaT_pushwstring(L, t1);
        return 1;
    }
    return 0;
}

int system_convertToWin(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::wstring t1(luaT_towstring(L, 1));
        lua_pushstring(L, TW2A(t1.c_str()));
        return 1;
    }
    return 0;
}

BeepTasks* g_background_beep_tasks = NULL;
int system_beep(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        g_background_beep_tasks->runTask( lua_tounsigned(L, 1), lua_tounsigned(L, 2) );
        return 0;
    }
    lua_pushstring(L, "Incorrect parameters system.beep");
    return lua_error(L);
}

static const luaL_Reg system_methods[] =
{
    { "dbgstack", system_debugstack},
    { "dbgtable", system_dbgtable },
    { "dbglog", system_dbglog },
    { "msgbox", system_messagebox },
    { "sleep", system_sleep },
    { "loadTextFile", system_loadTextFile },
    { "convertFromWin", system_convertFromWin },
    { "convertToWin", system_convertToWin },
    { "beep", system_beep },
    { NULL, NULL }
};

int luaopen_system(lua_State *L)
{
    luaL_newlib(L, system_methods);
    return 1;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_background_beep_tasks = new BeepTasks();
        break;
    case DLL_PROCESS_DETACH:
        delete g_background_beep_tasks;
        break;
    }
    return TRUE;
}
