#pragma once

extern "C" {
#include "../include/lua.h"
#include "../include/lualib.h"
#include "../include/lauxlib.h"
}
class luaT_State
{
public:
    luaT_State() { L = luaL_newstate(); }
    ~luaT_State() { if (L) lua_close(L); L = NULL; }
    operator lua_State*() { return L; }
private:
    lua_State *L;
};

// class to load script from file and run it
class luaT_script
{
public:
    luaT_script(lua_State* pL);
    ~luaT_script();
    bool run(const tstring& script, tstring* error);

private:
    lua_State* L;   
};

// class to support table
class luaT_fun_table
{
   std::string global_name;
   bool in_next;
public:
    luaT_fun_table(const char* table_name) : global_name(table_name), in_next(false) {}
    int pushFunction(lua_State *L)
    {
        if (!lua_isfunction(L, -1))
            { assert(false); return 0; }
        lua_getglobal(L, global_name.c_str());
        if (!lua_istable(L, -1))
        {
            if (!lua_isnil(L, -1))
            {
                lua_pop(L, 1);                
                return 0;
            }
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushvalue(L, -1);
            lua_setglobal(L, global_name.c_str());
        }
        lua_len(L, -1);
        int index = lua_tointeger(L, -1) + 1;
        lua_pop(L, 1);
        lua_insert(L, -2);
        lua_pushinteger(L, index);
        lua_insert(L, -2);
        lua_settable(L, -3);
        lua_pop(L, 1);
        return index;
    }

    void popFunction(lua_State *L, int index)
    {
        lua_getglobal(L, global_name.c_str());
        if (lua_istable(L, -1) && index > 0)
        {
            lua_pushinteger(L,index);
            lua_pushnil(L);
            lua_settable(L, -3);
        }
        lua_pop(L, 1);
    }

    bool next(lua_State *L)
    {
        if (!in_next)
        {
            lua_getglobal(L, global_name.c_str());
            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                return false;
            }
            lua_pushnil(L);                     // first key
            in_next = true;
        }
        if (!lua_istable(L, -2))
            { assert(false); in_next = false; return false; }

        while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
        {
            if (lua_isfunction(L, -1))
                return true;
            else
               { lua_pop(L, 1); }
        }
        in_next = false; 
        return false;
    }

    bool getFunction(lua_State *L, int index)
    {
        lua_getglobal(L, global_name.c_str());
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        lua_pushinteger(L, index);
        lua_gettable(L, -2);
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 2);
            return false;
        }
        lua_insert(L, -2);
        lua_pop(L, 1);
        return true;
    }
};
