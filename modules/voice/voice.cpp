#include "stdafx.h"
#include "api/api.h"

#include <sapi53.h>
ISpVoice * pVoice = NULL;

int voice_play(lua_State *L)
{
    if (!pVoice)
    {
        lua_pushstring(L, "Voice engine doesn't initialized!");
        return lua_error(L);
    }

    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        lua_towstring text(L, -1);
        pVoice->Speak(text, 0, NULL);    
    }
}

int voice_setVolume(lua_State *L)
{
    return 0;
}

int voice_getVolume(lua_State *L)
{
    return 0;
}

int voice_setRate(lua_State *L)
{
    return 0;
}

int voice_getRate(lua_State *L)
{
    return 0;
}

static const luaL_Reg voice_methods[] =
{
    { "play", voice_play },
    { "getVolume", voice_getVolume },
    { "setVolume", voice_setVolume },
    { "getRate", voice_getRate },
    { "setRate", voice_setRate },
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
    {
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
        if (FAILED(hr)) {
            pVoice = NULL;
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        if (pVoice) {
            pVoice->Release();
            pVoice = NULL;
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
