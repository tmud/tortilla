#include "stdafx.h"
#include "luaSupport.h"
#pragma comment(lib,"lua.lib")

luaT_script::luaT_script(lua_State* pL) : L(pL)
{
    assert(L);
}

luaT_script::~luaT_script()
{
}

bool luaT_script::run(const tstring& script, tstring* error)
{
    TW2U w2u(script.c_str());
    luaL_loadstring(L, w2u);
    if (!lua_pcall(L, 0, 0, 0))
        return true;
    if (error)
        error->assign( lua_toerror(L) );
    lua_pop(L, 1);
    return false;
}

luaT_State::luaT_State()
{
	L = luaL_newstate();
	luaL_openlibs(L);
}
