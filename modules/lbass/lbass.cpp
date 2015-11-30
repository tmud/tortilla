#include "stdafx.h"
#pragma comment(lib, "bass/c/bass.lib")
#include "common.h"
#include "player.h"

class BassCaller : public BassObjectEvents
{
    lua_State *L;
    lua_ref m_function_ref;
    int m_id;
public:
    BassCaller(lua_State *l, int id) : L(l), m_id(id) { m_function_ref.createRef(L); }
    ~BassCaller() { m_function_ref.unref(L); }
    void playingEnd()
    {
        m_function_ref.pushValue(L);
        m_function_ref.unref(L);
        lua_pushinteger(L, m_id);
        lua_pcall(L, 1, 0, 0);
    }
};

BassPlayer _bass_player;

int error(lua_State* L, const wchar_t* errmsg)
{
    std::wstring msg(L"BASS error: ");
    msg.append(errmsg);
    lua_pushwstring(L, msg.c_str());
    return lua_error(L);
}

int error_invargs(lua_State* L, const wchar_t* func)
{
    std::wstring msg(L"Incorrect parameters in function: 'bass.");
    msg.append(func);
    msg.append(L"'");
    lua_dumpparams p(L, msg.c_str());
    lua_pushwstring(L, p);
    return lua_error(L);
}

int lbass_play(lua_State *L)
{
    int n = lua_gettop(L);
    if (n >= 1 && n <= 3 && lua_isnumber(L, 1))
    {
        bool params_ok = true;
        int volume = 100;
        if (n >= 2)
        {
            if (lua_isnumber(L, 2))
                volume = lua_tointeger(L, 2);
            else if (!lua_isnil(L, 2))
                params_ok = false;
        }
        if (params_ok && n == 3)
        {
            if (!lua_isfunction(L, 3))
                params_ok = false;
        }
        if (params_ok)
        {
            int id = lua_tointeger(L, 1);
            BassCaller *func = NULL;
            if (n == 3)
                func = new BassCaller(L, id);
            if (!_bass_player.play(id, volume, func))
                 return error(L, _bass_player.getLastError());
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    return error_invargs(L, L"play");
}

int lbass_unload(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        if ( !_bass_player.unload(lua_tointeger(L,1)) )
            return error(L, _bass_player.getLastError());
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, L"unload");
}

int lbass_load(lua_State *L, const wchar_t* method, bool as_sample)
{
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        std::wstring file(lua_towstring(L, 1));
        int id = _bass_player.load(file.c_str(), as_sample);
        if (id == -1)
            return error(L, _bass_player.getLastError());
        lua_pushinteger(L, id);
        return 1;
    }
    return error_invargs(L, method);
}

int lbass_loadSample(lua_State *L)
{
   return lbass_load(L, L"loadSample", true);
}

int lbass_loadStream(lua_State *L)
{
    return lbass_load(L, L"loadStream", false);
}

int lbass_isHandle(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        bool result = _bass_player.isHandle(lua_tointeger(L, 1));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isHandle");
}

int lbass_isSample(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int result = _bass_player.isSample(lua_tointeger(L, 1));
        if (result == -1)
            return error(L, _bass_player.getLastError());
        lua_pushboolean(L, (result==1) ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isSample");
}

int lbass_isPlaying(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int result = _bass_player.isPlaying(lua_tointeger(L, 1));
        if (result == -1)
            return error(L, _bass_player.getLastError());
        lua_pushboolean(L, (result==1) ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isPlaying");
}

int lbass_getPath(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        std::wstring path; 
        if (!_bass_player.getPath(lua_tointeger(L, 1), &path))
            return error(L, _bass_player.getLastError());
        lua_pushwstring(L, path.c_str());
        return 1;
    }
    return error_invargs(L, L"getPath");
}

int lbass_stop(lua_State *L)
{
   int n = lua_gettop(L);
   if (n == 1 && lua_isnumber(L, 1))
   {
       if (!_bass_player.stop(lua_tointeger(L, 1)))
            return error(L, _bass_player.getLastError());
       lua_pushboolean(L, 1);
       return 1;
   }
   return error_invargs(L, L"stop");
}

int lbass_stopAll(lua_State *L)
{
    if (lua_gettop(L) == 0)
    {
        _bass_player.stopAll();
        return 0;
    }
    return error_invargs(L, L"stopAll");
}

int lbass_init(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error_invargs(L, L"init");
    if (!_bass_player.loadBass())
        return error(L, _bass_player.getLastError());
    lua_pushboolean(L, 1);
    return 1;
}

int lbass_free(lua_State* L)
{
    if (lua_gettop(L) == 0)
    {
        if (!_bass_player.unloadBass())
            return error(L, _bass_player.getLastError());
        return 0;
    }
   return error_invargs(L, L"free");
}

int lbass_getVolume(lua_State* L)
{
    if (lua_gettop(L) == 0)
    {
        float v = BASS_GetVolume();
        lua_pushinteger(L, volume_toInt(v) );
        return 1;
    }
    return error_invargs(L, L"getVolume");
}

int lbass_setVolume(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
       int v = lua_tointeger(L, 1);
       if (v >= 0 && v <= 100)
       {
            BASS_SetVolume( volume_ToFloat(v) );
            return 0;
       }
    }
    return error_invargs(L, L"setVolume");
}

int lbass_setRecord(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 2 && lua_isstring(L, 1) && lua_isnumber(L, 2))
    {
        std::wstring p(lua_towstring(L, 1));
        int value = lua_tointeger(L, 2);
        if (_bass_player.setRecord(p.c_str(), value))
            return 0;
    }
    return error_invargs(L, L"setRecord");
}

int lbass_canRecord(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 0) {
        lua_pushboolean(L, _bass_player.canRecord() ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"canRecord");
}

int lbass_startRecord(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 1 && lua_isstring(L, 1))
    {
        if (!_bass_player.startRecord(lua_towstring(L, 1)))
            return error(L, _bass_player.getLastError());
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, L"startRecord");
}

int lbass_stopRecord(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 0) {
      _bass_player.stopRecord();
      return 0;
    }
    return error_invargs(L, L"stopRecord");
}

int lbass_isRecording(lua_State *L)
{
    int n = lua_gettop(L);
    if (n == 0) {
        bool r = _bass_player.isRecording();
        lua_pushboolean(L, r ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isRecording");
}

static const luaL_Reg lbass_methods[] =
{
    { "init", lbass_init },
    { "free", lbass_free },
    { "loadSample", lbass_loadSample },
    { "loadStream", lbass_loadStream },
    { "unload", lbass_unload },
    { "isHandle", lbass_isHandle},
    { "isSample", lbass_isSample},
    { "play", lbass_play },
    { "isPlaying", lbass_isPlaying },
    { "getPath", lbass_getPath },
    { "stop", lbass_stop },
    { "stopAll", lbass_stopAll },
    { "setVolume", lbass_setVolume },
    { "getVolume", lbass_getVolume },
    { "setRecord", lbass_setRecord },
    { "canRecord", lbass_canRecord },
    { "startRecord", lbass_startRecord },
    { "stopRecord", lbass_stopRecord },
    { "isRecording", lbass_isRecording },
    { NULL, NULL }
};

int luaopen_lbass(lua_State *L)
{
    luaL_newlib(L, lbass_methods);
    return 1;
}
