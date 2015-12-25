#include "stdafx.h"
#include <sapi51.h>
#include <sphelper.h>

ISpVoice* pVoice = NULL; 
IEnumSpObjectTokens* pTokens = NULL;
ISpObjectToken* pToken = NULL;

int error_voice(lua_State* L)
{
    lua_pushstring(L, "Voice engine doesn't initialized!");
    return lua_error(L);
}

int error_invargs(lua_State *L, const wchar_t* func)
{
    std::wstring msg(L"Incorrect parameters in function: 'voice.");
    msg.append(func);
    msg.append(L"'");
    lua_dumpparams p(L, msg.c_str());
    lua_pushwstring(L, p);
    return lua_error(L);
}

int return_result(lua_State*L, const wchar_t* func, HRESULT hr)
{
    if (FAILED(hr))
    {
        std::wstring msg(L"Incorrect result in function: 'voice.");
        msg.append(func);
        msg.append(L"'");
        lua_pushwstring(L, msg.c_str());
        return lua_error(L);
    }
    return 0;
}

void voice_destroy()
{
    if (pToken)
        pToken->Release();
    if (pTokens)
        pTokens->Release();
    if (pVoice)
        pVoice->Release();
}

int voice_play(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        lua_towstring text(L, -1);
        HRESULT hr = pVoice->Speak(text, SPF_ASYNC, NULL);
        return return_result(L, L"play", hr);
    }
    return error_invargs(L, L"play");
}

int voice_stop(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 0)
    {
        HRESULT hr = pVoice->Speak(L"", SPF_ASYNC|SPF_PURGEBEFORESPEAK, NULL);
        return return_result(L, L"stop", hr);
    }
    return error_invargs(L, L"stop");
}

int voice_setVolume(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int volume = lua_tointeger(L, 1);
        if (volume >= 0 && volume <= 100)
        {
            USHORT v = (USHORT)volume;
            HRESULT hr = pVoice->SetVolume(v);
            return return_result(L, L"setVolume", hr);
        }
    }
    return error_invargs(L, L"setVolume");
}

int voice_getVolume(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 0)
    {
        USHORT v = 0;
        HRESULT hr = pVoice->GetVolume(&v);
        if (SUCCEEDED(hr))
        {
            lua_pushinteger(L, v);
            return 1;
        }
        return return_result(L, L"getVolume", hr);
    }
    return error_invargs(L, L"getVolume");
}

int voice_setRate(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int rate = lua_tointeger(L, 1);
        if (rate >= -10 && rate <= 10)
        {
            long r = (long)rate;
            HRESULT hr = pVoice->SetRate(r);
            return return_result(L, L"setRate", hr);
        }
    }
    return error_invargs(L, L"setRate");
}

int voice_getRate(lua_State *L)
{
    if (!pVoice)
        return error_voice(L);
    if (lua_gettop(L) == 0)
    {
        long r = 0;
        HRESULT hr = pVoice->GetRate(&r);
        if (SUCCEEDED(hr))
        {
            lua_pushinteger(L, r);
            return 1;
        }
        return return_result(L, L"getRate", hr);
    }
    return error_invargs(L, L"getRate");
}

int voice_getVoices(lua_State *L)
{
    if (!pVoice || !pTokens)
        return error_voice(L);
    if (lua_gettop(L) == 0) 
    {
        ULONG count = 0;
        HRESULT hr = pTokens->GetCount(&count);
        if (FAILED(hr))
        {
            lua_pushnil(L);
            return 1;    
        }
        lua_newtable(L);
        WCHAR *buffer = NULL;
        ISpObjectToken *token = NULL;
        for (ULONG i=0; i<count; ++i)
        {
            pTokens->Item(i, &token);
            LANGID lang = 0;
            hr = SpGetLanguageFromToken(token, &lang);
            if (SUCCEEDED(hr))
                hr = SpGetDescription(token, &buffer, lang);
            token->Release();
            token=NULL;
            if (SUCCEEDED(hr)) {
            lua_pushinteger(L, i+1);
            lua_pushwstring(L, buffer);
            lua_settable(L, -3);        
            CoTaskMemFree(buffer);
            buffer = NULL;
            } 
        }
        return 1;
    }
    return error_invargs(L, L"getVoices");
}

int voice_selectVoice(lua_State *L)
{
    if (!pVoice || !pTokens)
        return error_voice(L);
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int index = lua_tointeger(L, 1) - 1;
        ULONG count = 0;
        HRESULT hr = pTokens->GetCount(&count);
        if (FAILED(hr))
        {
            lua_pushboolean(L, 0);
            return 1;
        }
        if (index >= 0 && index < (int)count)
        {

                    
        }
    }
    return error_invargs(L, L"selectVoice");
}

int voice_init(lua_State *L)
{
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if (SUCCEEDED(hr))
    {
        // Enumerate the available voices.
        hr = ::SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pTokens);
    }
    if (FAILED(hr))
        voice_destroy();
    lua_pushboolean(L, (SUCCEEDED(hr)) ? 1 : 0);
    return 1;
}

int voice_release(lua_State *L)
{
    voice_destroy();
    return 0;
}

static const luaL_Reg voice_methods[] =
{
    { "init", voice_init },
    { "release", voice_release },
    { "play", voice_play },
    { "stop", voice_stop },
    { "getVolume", voice_getVolume },
    { "setVolume", voice_setVolume },
    { "getRate", voice_getRate },
    { "setRate", voice_setRate },
    { "getVoices", voice_getVoices },
    { "selectVoice", voice_selectVoice },
    { NULL, NULL }
};

int luaopen_voice(lua_State *L)
{    
    luaL_newlib(L, voice_methods);
    return 1;
}
