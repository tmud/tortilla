#include "stdafx.h"

int declension_new(lua_State *L);
int dict_new(lua_State *L);

static const luaL_Reg extra_methods[] =
{
    { "declension", declension_new },
    { "dictonary", dict_new },
    { NULL, NULL }
};

int luaopen_extra(lua_State *L)
{
    luaL_newlib(L, extra_methods);
    return 1;
}


/*BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
*/