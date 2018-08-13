#include "stdafx.h"
#include "embedded.h"

void luaopen_rnd(lua_State *L);
void luaopen_system(lua_State *L);
void luaopen_extra(lua_State *L);

void loadEmbeddedModules(lua_State *L)
{
    luaopen_system(L);
    luaopen_rnd(L);
    luaopen_extra(L);
}