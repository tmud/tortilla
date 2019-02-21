#include "stdafx.h"

int declension_new(lua_State *L);
int dict_new(lua_State *L);

static const luaL_Reg extra_methods[] =
{
    { "declension", declension_new },
    { "dictonary", dict_new },
    { NULL, NULL }
};

void luaopen_extra(lua_State *L)
{
    luaL_newlib(L, extra_methods);
    lua_setglobal(L, "extra");
}
