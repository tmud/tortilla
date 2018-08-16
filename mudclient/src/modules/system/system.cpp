#include "stdafx.h"
#include "tasks.h"
#include "alert.h"
#pragma comment(lib, "lua.lib")

int system_messagebox(lua_State *L)
{
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
    {
        HWND parent = base::getParent(L);
        result = MessageBox(parent, text.c_str(), caption.c_str(), buttons);
    }
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

void wstring_replace(std::wstring *str, const std::wstring& what, const std::wstring& forr)
{
    size_t pos = 0;
    while((pos = str->find(what, pos)) != std::wstring::npos)
    {
        str->replace(pos, what.length(), forr);
        pos += forr.length();
    }
}

int system_alert(lua_State *L)
{
    std::wstring text;
    for (int i=1,e=lua_gettop(L);i<=e;++i)
    {
        if (i != 1)
            text.append(L" ");
        std::wstring val;
        if (lua_istable(L, i))
        {
            lua_pushnil(L);                      // first key
            while (lua_next(L, i) != 0)          // table, value index = -1
            {
                formatByType(L, -1, &val);
                text.append(val);
                text.append(L" ");
                lua_pop(L, 1);
            }
            continue;
        }
        formatByType(L, i, &val);
        text.append(val);
    }
	wstring_replace(&text, L"\r", L"");
	wstring_replace(&text, L"\n", L"");
    wstring_replace(&text, L"\\n", L"\n");

    AlertDlg *dlg = new AlertDlg();
    RECT rc;
    rc.left = 0; rc.top = 0;
    rc.bottom = 200; rc.right = 350;
    dlg->Create(NULL, &rc, L"Alert", WS_POPUP|WS_CAPTION|WS_SYSMENU, WS_EX_TOPMOST);
    dlg->CenterWindow( base::getParent(L) );
    dlg->ShowWindow(SW_SHOW);
	dlg->setText(text);
    return 0;
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
        HANDLE file = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

int system_saveTextFile(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TTABLE))
    {
        std::wstring filename( luaT_towstring(L, 1) );
        HANDLE file = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file != INVALID_HANDLE_VALUE)
        {
            bool error = false;
            AutoCloseHandle auto_close(file);
            {
                unsigned char bom[3] = {  0xEF, 0xBB, 0xBF };
                DWORD written = 0;
                if (!WriteFile(file, bom, 3, &written, NULL))
                    error = true;
            }

            std::string text;
            int index = 1;
            while (!error)
            {
                lua_pushinteger(L, index++);
                lua_gettable(L, -2);                
                if (lua_isstring(L, -1))
                    text.assign(lua_tostring(L, -1));
                else if lua_isnil(L, -1)  {
                    lua_pop(L, 1);
                    break;
                }
                else {
                    text.clear();
                }
                lua_pop(L, 1);
                text.append("\r\n");
                DWORD written = 0;
                if (!WriteFile(file, text.c_str(), text.length(), &written, NULL))
                    error = true;
            }
            if (!error) {
            lua_pushboolean(L, 1);
            return 1;
            }
        }
    }
    lua_pushboolean(L, 0);
    return 1;
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

std::auto_ptr<BeepTasks> g_background_beep_tasks(new BeepTasks());
int system_beep(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TNUMBER, LUA_TNUMBER))
    {
        g_background_beep_tasks->runTask( lua_tounsigned(L, 1), lua_tounsigned(L, 2) );
        return 0;
    }
    return luaT_error(L, L"Incorrect parameters system.beep");
}

int system_getTime(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        lua_pushinteger(L, st.wSecond);
        lua_pushinteger(L, st.wMinute);
        lua_pushinteger(L, st.wHour);
        return 3;
    }
    return luaT_error(L, L"Incorrect parameters system.getTime");
}

int system_getDate(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        lua_pushinteger(L, st.wDay);
        lua_pushinteger(L, st.wMonth);
        lua_pushinteger(L, st.wYear);
        return 3;
    }
    return luaT_error(L, L"Incorrect parameters system.getDate");
}

int system_getTicks(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        DWORD ticks = GetTickCount();
        lua_pushunsigned(L, ticks);
        return 1;
    }
    return luaT_error(L, L"Incorrect parameters system.getTicks");
}

int system_appendStringToFile(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TSTRING))
    {
        std::wstring filename( luaT_towstring(L, 1) );
        HANDLE file = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        AutoCloseHandle auto_close(file);
        if (SetFilePointer(file, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        std::string text( lua_tostring(L, 2));
        DWORD len = text.length();
        DWORD written = 0;
        if (!WriteFile(file, text.c_str(), len, &written, NULL))
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        lua_pushboolean(L, 1);
        return 1;
    }
    return luaT_error(L, L"Incorrect parameters system.appendStringToFile");
}

int system_deleteFile(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TSTRING))
    {
        std::wstring filename( luaT_towstring(L, 1) );
        DeleteFile(filename.c_str());
        return 0;
    }
    return luaT_error(L, L"Incorrect parameters system.deleteFile");
}

static const luaL_Reg system_methods[] =
{
    { "dbgstack", system_debugstack},
    { "dbgtable", system_dbgtable },
    { "dbglog", system_dbglog },
    { "msgbox", system_messagebox },
    { "alert", system_alert },
    { "sleep", system_sleep },
    { "loadTextFile", system_loadTextFile },
    { "convertFromWin", system_convertFromWin },
    { "convertToWin", system_convertToWin },
    { "beep", system_beep },
    { "getTime", system_getTime },
    { "getDate", system_getDate },
    { "getTicks", system_getTicks },
    { "saveTextFile", system_saveTextFile },
    { "appendStringToFile", system_appendStringToFile },
    { "deleteFile", system_deleteFile },
    { NULL, NULL }
};

void luaopen_system(lua_State *L)
{
    luaL_newlib(L, system_methods);
    lua_setglobal(L, "system");
}
