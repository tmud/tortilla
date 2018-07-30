#include "stdafx.h"
#include "embedded.h"

int luaopen_rnd(lua_State *L);
int luaopen_system(lua_State *L);

void loadEmbeddedModules(lua_State *L)
{
    luaopen_system(L);
    luaopen_rnd(L);
}