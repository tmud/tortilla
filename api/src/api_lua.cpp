﻿#include "stdafx.h"
#include "../api.h"
#include <vector>

struct luaT_userdata
{
    luaT_userdata() : type(0), data(0) {}
    int type;
    void *data;
};

const char* metatables[] = { "window", "viewdata", "activeobjects", "panel", "render", "pen", "brush", "font", "pcre", "image", "trigger", "viewstring" };
std::vector<std::string> types;

void getmetatable(lua_State *L, int type)
{
    if (type >= LUAT_WINDOW && type <= LUAT_LAST)
    {
        type = type - LUAT_WINDOW;
        luaL_getmetatable(L, metatables[type]);
    }
    else
    {
        int index = type-(LUAT_LAST+1);
        int count = types.size();
        if (index >= 0 && index < count) {
            luaL_getmetatable(L, types[index].c_str());
        }
        else {
            lua_pushnil(L);
        }
    }
}

const wchar_t* lua_types_str[] = {L"nil", L"bool", L"lightud", L"number", L"string", L"table", L"function", L"userdata", L"thread"  };
int luaT_push_args(lua_State *L, const char* func)
{
    int n = lua_gettop(L);
    lua_Debug ar;  lua_getstack(L, 1, &ar); lua_getinfo(L, "nSl", &ar);
    std::wstring log( TA2W(ar.short_src) );
    log.append(L" ");
    log.append( TA2W(func) );
    wchar_t buffer[16];
    if (ar.currentline > 0)
    {
        log.append(L":");
        _itow(ar.currentline, buffer, 10);
        log.append(buffer);
    }
    log.append(L": (");
    _itow(n, buffer, 10);
    log.append(buffer);
    log.append(L"):");
    for (int i = 1; i <= n; ++i)
    {
        int t = lua_type(L, i);
        if (t >= 0 && t < LUA_NUMTAGS)
        {
            bool known_user_data = false;
            if (t == LUA_TUSERDATA)
            {
                luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, i);
                if (o && o->type >= LUAT_WINDOW && o->type <= LUAT_LAST)
                {
                    int ti = o->type - LUAT_WINDOW;
                    TA2W mt(metatables[ti] );
                    log.append(mt);
                    known_user_data = true;
                }
            }
            if (t == LUA_TNUMBER || t == LUA_TBOOLEAN || t == LUA_TSTRING)
            {
                std::wstring str;
                lua_format lf;
                lf.format(L, i, &str);
                log.append(str);
            } else 
            {
                if (!known_user_data)
                  log.append(lua_types_str[t]);
            }
        }
        else
        {
            log.append(L"unknown");
        }
        if (i != n) log.append(L",");
    }
    luaT_pushwstring(L, log.c_str());
    return 1;
}

int luaT_regtype(lua_State *L, const char* type_name)
{
    int count = LUAT_LAST - LUAT_WINDOW + 1;
    for (int i=0;i<count;++i) 
    {
        if (!strcmp(type_name, metatables[i]))
            return 0;    
    }
    for (int i=0,e=types.size();i<e;++i)
    {
        if (!types[i].compare(type_name))
            return 0;
    }
    luaL_getmetatable(L, type_name);
    bool result =(lua_istable(L, -1));
    lua_pop(L, 1);
    if (result)     // table exist
        return 0;
    types.push_back(type_name);
    int id = types.size() + LUAT_LAST;
    return id;
}

const char* luaT_typename(lua_State* L, int index)
{
    if (!lua_isuserdata(L, index))
        return lua_typename(L, lua_type(L, index));
    luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, index);
    if (o && o->type >= LUAT_WINDOW && o->type <= LUAT_LAST)
    {
        int ti = o->type - LUAT_WINDOW;
        return metatables[ti];
    }
    if (o)
    {
        int type = o->type;
        int index = type-(LUAT_LAST+1);
        int count = types.size();
        if (index >= 0 && index < count) {
            return types[index].c_str();
        }
    }
    return "unknown";
}

bool luaT_run(lua_State *L, const char* func, const char* op, ...)
{
    int n = lua_gettop(L);
    if (n < 0)
    {
        n = 0;
        lua_settop(L, 0);
    }
    int on_stack = 0;
    bool object_method = false;
    bool simple_method = false;
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
        case 'b':
        {
            bool val = va_arg(args, bool);
            lua_pushboolean(L, val ? 1 : 0);
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
            const wchar_t* str = va_arg(args, const wchar_t*);
            luaT_pushwstring(L, str);
            break;
        }
        case 'F':
        {
            void *f = va_arg(args, void*);
            lua_pushcfunction(L, (lua_CFunction)f);
            break;
        }
        case 'r':
        {
            on_stack++;
            break;
        }
        case 't':
        {
            if (i == 0) { object_method = true; simple_method = true; }
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

    std::wstring error_msg;
    int required_func_pos = n - on_stack + 1;
    if (success && object_method)
    {
        if (!lua_isuserdata(L, required_func_pos) && !lua_istable(L, required_func_pos))
        {
            error_msg.append(L"Попытка вызвать функцию у объекта: ");
            TA2W type(luaT_typename(L, required_func_pos));
            error_msg.append(type);
            success = false;
        }
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
                if (simple_method)
                {
                    oplen--;
                    if (oplen > 0)
                        lua_remove(L, required_func_pos+1);
                    else
                        lua_pop(L, 1);
                }
            }
            else
            {
                lua_getmetatable(L, required_func_pos);
                if (!lua_istable(L, -1))
                {
                    error_msg.append(L"Функция у объекта не существует");
                    success = false;
                }
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
    {
        error_msg.append(L"Функция у объекта не существует");
        success = false;
    }

    if (!success)
    {
        lua_settop(L, n); // restore stack
        std::wstring error(L"Ошибка вызова luaT_run '");
        error.append(TU2W(func));
        error.append(L"': ");
        error.append(error_msg);
        base::log(L, error.c_str());
        return false;
    }

    if (required_func_pos > 1) {
        int to_pop = required_func_pos - 1;
        while (to_pop > 0) {
            lua_remove(L, 1);
            to_pop--;
        }
    }

    if (lua_pcall(L, oplen, LUA_MULTRET, 0))
    {
        if (lua_isstring(L, -1))
            base::log(L, lua_toerror(L));
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

int luaT_error(lua_State *L, const wchar_t* error_message)
{
    if (error_message)
    {
        luaT_pushwstring(L, error_message);
        lua_error(L);
    }
    return 0;
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
    if (!object) { lua_pushnil(L);  return; }
    luaT_userdata *o = (luaT_userdata*)lua_newuserdata(L, sizeof(luaT_userdata));
    o->data = object;
    o->type = type;
    getmetatable(L, type);
    if (lua_istable(L, -1))
        lua_setmetatable(L, -2);
    else
        lua_pop(L, 1);      // error - userdata without metatable!
}

bool luaT_isobject(lua_State* L, int type, int index)
{
    if (!lua_isuserdata(L, index))
        return false;
    luaT_userdata *o = (luaT_userdata*)lua_touserdata(L, index);
    return (o->type == type) ? true : false;
}

bool luaT_dostring(lua_State *L, const wchar_t* script_text)
{
    if (!luaL_dostring(L, TW2U(script_text)))
        return true;
    return false;
}

void formatByType(lua_State* L, int index, std::wstring *buf)
{
    lua_format lf;
    lf.format(L, index, buf);
}

void luaT_showLuaStack(lua_State* L, const wchar_t* label)
{
    std::wstring msg(L"\r\nLabel: ");
    msg.append(label ? label : L"?");
    msg.append(L"\r\n");
    
    int n = lua_gettop(L);
    int j = -1;
    for (int i = n; i >= 1; --i)
    {
        wchar_t dbuf[32];
        swprintf(dbuf, L"[%d][%d]", i, j--);
        msg.append(dbuf);
        std::wstring par;
        formatByType(L, i, &par);
        msg.append(par);
        msg.append(L"\r\n");
    }
    OutputDebugString(msg.c_str());
}

void luaT_showTableOnTop(lua_State* L, const wchar_t* label)
{
    std::wstring msg(L"\r\nLabel: ");
    msg.append(label ? label : L"?");
    msg.append(L"\r\n");

    if (!lua_istable(L, -1))
    {
        msg.append(L"Not TABLE on the top of stack!\r\n");
    }
    else
    {
        wchar_t dbuf[32];
        swprintf(dbuf, L"Table 0x%p\r\n", lua_topointer(L, -1));
        msg.append(dbuf);

        lua_pushnil(L);                     // first key
        while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
        {
            int type = lua_type(L, -2);
            switch (type)
            {
            case LUA_TNUMBER:
                swprintf(dbuf, L"number index [%d]=", lua_tointeger(L, -2));
                msg.append(dbuf);
                break;
            case LUA_TSTRING:
                msg.append(L"string index [");
                msg.append(luaT_towstring(L, -2));
                msg.append(L"]=");
                break;
            default:
                msg.append(L"unknown type index []=");
                break;
            }
            std::wstring par;
            formatByType(L, -1, &par);
            msg.append(par);
            msg.append(L"\r\n");
            lua_pop(L, 1);              // remove 'value', keeps 'key' for next iteration 
        }
    }
    OutputDebugString(msg.c_str());
}
