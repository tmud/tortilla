#include "stdafx.h"
#include "common.h"

void regFunction(lua_State *L, const char* name, lua_CFunction f)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, f);
    lua_settable(L, -3);
}
