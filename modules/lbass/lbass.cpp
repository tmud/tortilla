#include "stdafx.h"
#include <vector>
#include "bass/c/bass.h"
#pragma comment(lib, "bass/c/bass.lib")
#include "bobjects.h"

class BassLoader
{
    bool bass_loaded;
    std::vector<BassObject*> m_objects;
public:
    BassLoader() : bass_loaded(false) {}
    ~BassLoader() {}
    bool loadBass()
    {
        if (!BASS_Init(-1, 44100, 0, NULL, NULL))
            return false;
        bass_loaded = true;
        return true;
    }
    void unloadBass()
    {
        if (!bass_loaded)
            return;
        bass_loaded = false;
        for (int i = 0, e = m_objects.size(); i < e; ++i)
            delete m_objects[i];
        BASS_Free();
    }
    BassObject* loadStream(const wchar_t* file)
    {
        BassStream *stream = new BassStream;
        if (!stream->load(file))
            { delete stream; stream = NULL; }
        if (stream)
            m_objects.push_back(stream);
        return stream;
    }
    BassObject* loadSample(const wchar_t* file)
    {
        BassSample *sample = new BassSample;
        if (!sample->load(file))
            { delete sample; sample = NULL; }
        if (sample)
            m_objects.push_back(sample);
        return sample;
    }
    BassObject* find(const wchar_t* file)
    {
        for (int i=0,e=m_objects.size();i<e;++i)
        {
            if (!m_objects[i]->getname().find(file))
                return m_objects[i];
        }
        return NULL;
    }
} _bass_loader;

int error(lua_State* L, const utf8* errmsg, bool with_errcode)
{
    u8string str("BASS error: ");
    str.append(errmsg);
    if (with_errcode)
    {
        str.append(" (error code: ");
        char buffer[16];
        str.append(itoa(BASS_ErrorGetCode(), buffer, 10));
        str.append(")");
    }
    lua_pushnil(L);
    lua_pushstring(L, str.c_str());
    return 2;
}

int error_file(lua_State* L, const utf8* errormsg, const utf8* file, bool with_errcode)
{
    std::string msg(errormsg);
    msg.append(file);
    return error(L, msg.c_str(), with_errcode);
}

int lbass_play(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        std::wstring filename(luaM_towstring(L, 1));
        BassObject *obj = _bass_loader.find(filename.c_str());
        if (!obj)
        {
            HANDLE file = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file == INVALID_HANDLE_VALUE)
                return error_file(L, "Can't open file: ", lua_tostring(L, 1), false);
            DWORD hsize = 0;
            DWORD size = GetFileSize(file, &hsize);
            if (hsize != 0)
                return error_file(L, "File is huge: ", lua_tostring(L, 1), false);
            if (size == 0)
                return error_file(L, "File is empty: ", lua_tostring(L, 1), false);
            CloseHandle(file);

            obj = _bass_loader.loadSample(filename.c_str());
            if (!obj)
                return error_file(L, "Can't open file: ", lua_tostring(L, 1), true);
        }
        if (!obj->play())
            return error_file(L, "Can't play file: ", lua_tostring(L, 1), true);
        lua_pushboolean(L, 1);
        return 1;
    }    
    return error(L, "Incorrect parameters", false);
}

int lbass_init(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error(L, "Incorrect parameters", false);
    if (HIWORD(BASS_GetVersion()) != BASSVERSION)
        return error(L, "An incorrect version of dll was loaded", false);
    if (!_bass_loader.loadBass())
        return error(L, "Can't initialize sound system.", true);
    lua_pushboolean(L, 1);
    return 1;
}

int lbass_free(lua_State* L)
{
   _bass_loader.unloadBass();
   return 0;
}

static const luaL_Reg lbass_methods[] =
{
    { "play", lbass_play },
    { "init", lbass_init },
    { "free", lbass_free },
    { NULL, NULL }
};

int luaopen_lbass(lua_State *L)
{
    luaL_newlib(L, lbass_methods);
    return 1;
}
