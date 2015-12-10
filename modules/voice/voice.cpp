#include "stdafx.h"
#include "api/api.h"

int voice_play(lua_State *L)
{
    luaT_showLuaStack(L, L"x");
    return 0;
}

static const luaL_Reg voice_methods[] =
{
    { "play", voice_play },
    { NULL, NULL }
};

int luaopen_voice(lua_State *L)
{
    luaL_newlib(L, voice_methods);
    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID)
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

