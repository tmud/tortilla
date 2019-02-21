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
