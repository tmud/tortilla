#include "stdafx.h"
#include "../api.h"
#include <vector>

struct luaT_userdata
{
    luaT_userdata() : type(0), data(0) {}
    int type;
    void *data;
};

const char* metatables[] = { "window", "viewdata", "activeobjects" };
void getmetatable(lua_State *L, int type)
{
    type = type - LUAT_WINDOW;  // first type
    luaL_getmetatable(L, metatables[type]);
}

bool luaT_run(lua_State *L, const utf8* func, const utf8* op, ...)
{
    int n = lua_gettop(L);
    int on_stack = 0;
    bool object_method = false;
    bool success = true;
    int oplen = strlen(op);
    va_list args;
    va_start(args, op);
    for (int i = 0; i < oplen; ++i)
    {
        switch (op[i])
        {
        case 'd':
        {
            int val = va_arg(args, int);
            lua_pushinteger(L, val);
            break;
        }
        case 'f':
        {
            double val = va_arg(args, double);
            lua_pushnumber(L, val);
            break;
        }
        case 'u':
        {
            unsigned int val = va_arg(args, unsigned int);
            lua_pushunsigned(L, val);
            break;
        }
        case 's':
        {
            const char* str = va_arg(args, const char*);
            lua_pushstring(L, str);
            break;
        }
        case 't':
        {
            on_stack++;
            break;
        }
        case 'o':
        {
            if (i == 0) { object_method = true; on_stack++; }
            else { success = false; }
            break;
        }
        default:
        { success = false; break; }
        }
    }
    va_end(args);

    int required_func_pos = n - on_stack + 1;
    if (success && object_method)
    {
        if (!lua_isuserdata(L, required_func_pos) && !lua_istable(L, required_func_pos))
            success = false;
    }
    if (success)
    {
        if (!object_method)
        {
            lua_getglobal(L, func);
            lua_insert(L, required_func_pos);
        }
        else
        {
            if (lua_istable(L, required_func_pos))
            {
                lua_pushstring(L, func);
                lua_gettable(L, required_func_pos);
                lua_insert(L, required_func_pos);
                lua_pop(L, 1);
            }
            else
            {
                lua_getmetatable(L, required_func_pos);
                if (!lua_istable(L, -1))
                    success = false;
                else
                {
                    lua_pushstring(L, func);
                    lua_gettable(L, -2);
                    lua_insert(L, required_func_pos);
                    lua_pop(L, 1);
                }
            }
        }
    }

    if (success && !lua_isfunction(L, required_func_pos))
        success = false;

    if (!success)
    {
        lua_settop(L, n); // restore stack
        std::string error("luaT_run:");
        error.append(func);
        luaT_error(L, error.c_str());
        return false;
    }

    if (lua_pcall(L, oplen, LUA_MULTRET, 0))
    {
        return false;
    }
    return true;
}

bool luaT_check(lua_State *L, int n, ...)
{
    if (lua_gettop(L) != n)
        return false;

    bool result = true;
    va_list args;
    va_start(args, n);
    for (int i = 1; i <= n; ++i)
    {
        int type = va_arg(args, int);
        int stype = lua_type(L, i);
        if (type == LUA_TUSERDATA && stype == LUA_TUSERDATA)
            continue;
        if (type == LUA_TNUMBER && lua_isnumber(L, i))
            continue;
        if (stype == LUA_TUSERDATA)
        {
            luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, i);
            stype = o->type;
        }
        if (stype != type)
        {
            result = false; break;
        }
    }
    va_end(args);
    return result;
}

int luaT_error(lua_State *L, const utf8* error_message)
{
    lua_pushstring(L, error_message);
    lua_error(L);
    return 0;
}

void luaT_log(lua_State *L, const utf8* log_message)
{
    if (log_message)
        luaT_run(L, "log", "s", log_message);
}

void* luaT_toobject(lua_State* L, int index)
{
    if (!lua_isuserdata(L, index))
        return NULL;
    luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, index);
    return o->data;
}

void luaT_pushobject(lua_State* L, void *object, int type)
{
    luaT_userdata *o = (luaT_userdata*)lua_newuserdata(L, sizeof(luaT_userdata));
    o->data = object;
    o->type = type;
    getmetatable(L, type);
    lua_setmetatable(L, -2);
}

bool luaT_isobject(lua_State* L, int type, int index)
{
    if (!lua_isuserdata(L, index))
        return false;
    luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, index);
    return (o->type == type) ? true : false;
}
//-----------------------------------------------------------------------
typedef std::vector<std::string> tokens;
void gettokens(const utf8* path, tokens *parts)
{
    if (!path) return;
    const utf8 *p = path;
    const utf8 *e = p + strlen(p);
    while (p < e)
    {
        size_t len = strcspn(p, "/");
        parts->push_back(std::string(p, len));
        p = p + len + 1;
    }
}

bool incorrectname(const utf8* name)
{
    size_t len = strcspn(name, "/");
    return len == strlen(name) ? false : true;
}
//-----------------------------------------------------------------------
/*luaT_save::luaT_save(lua_State *pL) : L(pL), toparray(false)
{
    lua_newtable(L);
    nL = lua_gettop(L);
}

luaT_save::~luaT_save()
{
}

void luaT_save::end()
{
    lua_settop(L, nL);
}

bool luaT_save::select(const utf8* table)
{
    if (toparray)
    {
        lua_pop(L, 1); toparray = false;
    }
    if (!lua_istable(L, -1))
        return false;
    if (!prepare(table, false))
        return false;
    return true;
}

bool luaT_save::selectarray(const utf8* table)
{
    if (toparray)
    {
        lua_pop(L, 1); toparray = false;
    }
    if (!lua_istable(L, -1))
        return false;
    if (!prepare(table, false))
        return false;
    int index = luaL_len(L, -1) + 1;
    lua_newtable(L);
    lua_pushinteger(L, index);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    toparray = true;
    return true;
}

bool luaT_save::set(const utf8* name, int value)
{
    if (!lua_istable(L, -1)) return false;
    int n = lua_gettop(L);
    lua_pushinteger(L, value);
    if (toparray)
    {
        if (!incorrectname(name))
        {
            lua_settop(L, n);  return false;
        }
        lua_pushstring(L, name);
        lua_insert(L, -2);
        lua_settable(L, -3);
        return true;
    }
    if (!prepare(name, true))
    {
        lua_settop(L, n);  return false;
    }
    lua_settop(L, n);
    return true;
}

bool luaT_save::set(const utf8* name, const std::wstring& value)
{
    if (!lua_istable(L, -1)) return false;
    int n = lua_gettop(L);

    // push value
    int buffer_required = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, NULL, 0, NULL, NULL);
    char* buffer = new char[buffer_required + 1];
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, buffer, buffer_required, NULL, NULL);
    buffer[buffer_required] = 0;
    lua_pushstring(L, buffer);
    delete[]buffer;

    if (toparray)
    {
        if (!incorrectname(name))
        {
            lua_settop(L, n);  return false;
        }
        lua_pushstring(L, name);
        lua_insert(L, -2);
        lua_settable(L, -3);
        return true;
    }

    if (!prepare(name, true))
    {
        lua_settop(L, n);  return false;
    }
    lua_settop(L, n);
    return true;
}

bool luaT_save::prepare(const utf8* name, bool last_is_value)
{
    tokens t; gettokens(name, &t);
    if (t.empty())
        return false;
    if (t[0].empty())       // first '/' symbol -> root
    {
        lua_settop(L, nL);
        t.erase(t.begin());
    }
    int level = t.size() - (last_is_value ? 1 : 0);
    int val = 0;
    if (last_is_value) { lua_insert(L, -2); val = lua_gettop(L) - 1; } //swap data and table
    for (int i = 0; i < level; ++i)
    {
        lua_pushstring(L, t[i].c_str());
        lua_gettable(L, -2);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushstring(L, t[i].c_str());
            lua_pushvalue(L, -2);
            lua_settable(L, -4);
        }
        else if (!lua_istable(L, -1)) { return false; }
    }
    if (last_is_value)
    {
        lua_pushstring(L, t[level].c_str());
        lua_pushvalue(L, val);
        lua_settable(L, -3);
        lua_pop(L, level);
        lua_insert(L, -2);
    }
    return true;
}
//-----------------------------------------------------------------------
luaT_load::luaT_load(lua_State *pL) : L(pL)
{
    nL = lua_gettop(L);
}

luaT_load::~luaT_load()
{
}

void luaT_load::end()
{
    lua_settop(L, nL);
}

bool luaT_load::select(const utf8* table)
{
    if (!lua_istable(L, -1))
        return false;
    if (!prepare(table, false))
    {
        lua_settop(L, nL);  return false;
    }
    return true;
}

bool luaT_load::get(const utf8* name, int *value)
{
    if (!lua_istable(L, -1))
        return false;
    int n = lua_gettop(L);
    if (!prepare(name, true) || !lua_isnumber(L, -1))
    {
        lua_settop(L, n);  return false;
    }
    *value = lua_tointeger(L, -1);
    lua_settop(L, n);
    return true;
}

bool luaT_load::get(const utf8* name, std::wstring* value)
{
    if (!lua_istable(L, -1))
        return false;
    int n = lua_gettop(L);
    if (!prepare(name, true) || !lua_isstring(L, -1))
    {
        lua_settop(L, n);  return false;
    }

    const char* str = lua_tostring(L, -1);
    int symbols_count = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    int buffer_required = (symbols_count + 1) * sizeof(wchar_t);
    wchar_t *buffer = new wchar_t[buffer_required];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer, buffer_required);
    buffer[symbols_count] = 0;
    value->assign(buffer);
    delete[]buffer;

    lua_settop(L, n);
    return true;
}

bool luaT_load::prepare(const utf8* name, bool last_is_value)
{
    tokens t; gettokens(name, &t);
    if (t.empty())
        return false;
    if (t[0].empty())       // root
    {
        lua_settop(L, nL);
        t.erase(t.begin());
    }

    int level = t.size() - (last_is_value ? 1 : 0);
    for (int i = 0; i < level; ++i)
    {
        lua_pushstring(L, t[i].c_str());
        lua_gettable(L, -2);
        if (!lua_istable(L, -1)) { return false; }
        if (i != 0) { lua_insert(L, -2); lua_pop(L, 1); }
    }
    if (last_is_value)
    {
        lua_pushstring(L, t[level].c_str());
        lua_gettable(L, -2);
    }
    return true;
}*/

void luaT_formatByType(lua_State* L, int index, wchar_t *dbuf)
{
    int i = index;
    int type = lua_type(L, i);
    switch (type)
    {
    case LUA_TNIL:
        wcscpy(dbuf, L"nil");
        break;
    case LUA_TNUMBER:
        swprintf(dbuf, L"number: %d", lua_tointeger(L, i));
        break;
    case LUA_TBOOLEAN:
        swprintf(dbuf, L"boolean: %s", (lua_toboolean(L, i) == 0) ? "false" : "true");
        break;
    case LUA_TSTRING:
    {
        wchar_t buffer[96];
        MultiByteToWideChar(CP_UTF8, 0, lua_tostring(L, i), -1, buffer, 95);        
        swprintf(dbuf, L"string: %s", buffer);
        break;
    }
    case LUA_TUSERDATA:
        swprintf(dbuf, L"userdata: 0x%p", lua_topointer(L, i));
        break;
    case LUA_TLIGHTUSERDATA:
        swprintf(dbuf, L"lightuserdata: 0x%p", lua_topointer(L, i));
        break;
    case LUA_TFUNCTION:
        swprintf(dbuf, L"function: 0x%p", lua_topointer(L, i));
        break;
    case LUA_TTHREAD:
        wcscpy(dbuf, L"thread");
        break;
    case LUA_TTABLE:
        swprintf(dbuf, L"table: 0x%p", lua_topointer(L, i));
        break;
    default:
        wcscpy(dbuf, L"unknown");
        break;
    }
}

void luaT_showLuaStack(lua_State* L, const utf8* label)
{
    wchar_t dbuf[128];
    if (label)
        swprintf(dbuf, L"\r\nLabel: %s\r\n", convert_utf8_to_wide(label));        
    else
        wcscpy(dbuf, L"\r\nLabel: ?\r\n");
    OutputDebugString(dbuf);
    int n = lua_gettop(L);
    int j = -1;
    for (int i = n; i >= 1; --i)
    {
        luaT_formatByType(L, i, dbuf);
        std::wstring t(dbuf);
        swprintf(dbuf, L"[%d][%d] %s\r\n", i, j--, t.c_str());
        OutputDebugString(dbuf);
    }
}

void luaT_showTableOnTop(lua_State* L, const utf8* label)
{
    wchar_t dbuf[128];
    if (label)
        swprintf(dbuf, L"\r\nLabel: %s\r\n", convert_utf8_to_wide(label));
    else
        wcscpy(dbuf, L"\r\nLabel: ?\r\n");
    OutputDebugString(dbuf);

    if (!lua_istable(L, -1))
    {        
        OutputDebugString(L"Not TABLE on the top of stack!\r\n");
        return;
    }
    swprintf(dbuf, L"Table 0x%p data:\r\n", lua_topointer(L, -1));
    OutputDebugString(dbuf);

    lua_pushnil(L);                     // first key
    while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
    {
        int type = lua_type(L, -2);
        switch (type)
        {
        case LUA_TNUMBER:
            swprintf(dbuf, L"number index [%d]=", lua_tointeger(L, -2));
            break;
        case LUA_TSTRING:
            swprintf(dbuf, L"string index [%s]=", convert_utf8_to_wide(lua_tostring(L, -2)));
            break;
        default:
            wcscpy(dbuf, L"unknown type index []=");
            break;
        }
        std::wstring t(dbuf);
        luaT_formatByType(L, -1, dbuf);
        t.append(dbuf);
        t.append(L"\r\n");
        OutputDebugString(t.c_str());
        lua_pop(L, 1);              // remove 'value', keeps 'key' for next iteration 
    }
}
