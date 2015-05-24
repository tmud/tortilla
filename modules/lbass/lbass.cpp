#include "stdafx.h"
#include <vector>
#include "bass/c/bass.h"
#pragma comment(lib, "bass/c/bass.lib")
#include "bobjects.h"

class BassLoader
{
    bool bass_loaded;
    std::vector<BassObject*> m_objects;
    std::vector<int> m_indexes;
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
    int loadStream(const wchar_t* file)
    {
        BassStream *stream = new (std::nothrow) BassStream;
        if (!stream || !stream->load(file))
            { delete stream; return -1; }
        return push(stream);
    }
    int loadSample(const wchar_t* file)
    {
        BassSample *sample = new (std::nothrow) BassSample;
        if (!sample || !sample->load(file))
            { delete sample; return -1; }
        return push(sample);
    }
    int loadMusic(const wchar_t* file)
    {
        BassMusic *music = new (std::nothrow) BassMusic;
        if (!music || !music->load(file))
            { delete music; return -1; }
        return push(music);
    }
    bool unload(int id)
    {
        BassObject* obj = get(id);
        if (!obj) return false;
        m_objects[id] = NULL;
        delete obj;
        m_indexes.push_back(id);
        return true;
    }
    int find(const wchar_t* file)
    {
        for (int i=0,e=m_objects.size();i<e;++i)
        {
            BassObject* obj = m_objects[i];
            if (obj && !obj->getname().find(file))
                return i;
        }
        return -1;
    }
    int getCount() const
    {
        return m_objects.size();
    }
    BassObject* get(int id) const
    {
        return (id >= 0 && id < getCount()) ? m_objects[id] : NULL;
    }
private:
    int push(BassObject* obj)
    {
        if (!m_indexes.empty()) {
            int last = m_indexes.size() - 1;
            int index = m_indexes[last];
            m_indexes.pop_back();
            m_objects[index] = obj;
            return index;
        }
        int index = m_objects.size();
        m_objects.push_back(obj);
        return index;
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

int error_invargs(lua_State* L, const utf8* func)
{
     std::string msg("Invalid arguments in function: 'bass.");
     msg.append(func);
     msg.append("'");
     return error(L, msg.c_str(), false);
}

int lbass_load_ex(lua_State *L, bool load_as_sample)
{
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        std::wstring filename(luaM_towstring(L, 1));
        int index = _bass_loader.find(filename.c_str());
        if (index != -1)
        {
            lua_pushinteger(L, index);
            return 1;
        }

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

        // get extension
        bool correct_ext = false;
        int pos = filename.rfind(L'.');
        if (pos != -1)
        {
            std::wstring ext(filename.substr(pos+1));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == L"mp3" || ext == L"wav" || ext == L"ogg" || ext == L"mp2" || ext == L"mp1" || ext == L"aiff")
            {
                correct_ext = true;
                if (load_as_sample)
                    index = _bass_loader.loadSample(filename.c_str());
                else
                    index = _bass_loader.loadStream(filename.c_str());
            }
            else if (ext == L"mo3" || ext == L"xm" || ext == L"it" || ext == L"s3m" || ext == L"mtm" || ext == L"mod" || ext == L"umx")
            {
                if (load_as_sample)
                    return error_file(L, "Can't load as sample: ", lua_tostring(L, 1), false);
                correct_ext = true;
                index = _bass_loader.loadMusic(filename.c_str());
            }
        }

        if (!correct_ext)
            return error_file(L, "Unknown file type: ", lua_tostring(L, 1), false);
        if (index == -1)
            return error_file(L, "Can't open file: ", lua_tostring(L, 1), true);
        lua_pushinteger(L, index);
        return 1;
    }
    return error_invargs(L, load_as_sample ? "loadSample" : "load");
}

int lbass_load(lua_State *L)
{
    return lbass_load_ex(L, false);
}

int lbass_loadsample(lua_State *L)
{
    return lbass_load_ex(L, true);
}

int getIndex(lua_State *L)
{
    return (lua_gettop(L) == 1 && lua_isnumber(L, 1)) ? lua_tointeger(L, 1) : -1;
}

BassObject* getObject(lua_State *L)
{
    int index = getIndex(L);
    return _bass_loader.get(index);
}

int lbass_unload(lua_State *L)
{
    int index = getIndex(L);
    if (index < 0)
        return error_invargs(L, "unload");
    bool result = _bass_loader.unload(index);
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int lbass_play(lua_State *L)
{
    BassObject *obj = getObject(L);
    if (!obj)
        return error_invargs(L, "play");
    bool result = obj->play();
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

int lbass_init(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error_invargs(L, "init");
    if (HIWORD(BASS_GetVersion()) != BASSVERSION)
        return error(L, "An incorrect version of bass.dll was loaded", false);
    if (!_bass_loader.loadBass())
        return error(L, "Can't initialize sound system.", true);
    lua_pushboolean(L, 1);
    return 1;
}

int lbass_free(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error_invargs(L, "free");
   _bass_loader.unloadBass();
   return 0;
}

int lbass_getVolume(lua_State* L)
{
    float v = BASS_GetVolume();
    if (v < 0)
         return error(L, "Can't get volume", true);
    int volume = static_cast<int>(v*100);
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    lua_pushinteger(L, volume);
    return 1;
}

int lbass_setVolume(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
       int v = lua_tointeger(L, 1);
       if (v >= 0 && v <= 100)
       {
            float volume = static_cast<float>(v);
            volume = volume / 100.0f;
            BASS_SetVolume(volume);
            return 0;
       }
    }
    return error_invargs(L, "setVolume");
}

static const luaL_Reg lbass_methods[] =
{
    { "load", lbass_load },
    { "loadSample", lbass_loadsample },
    { "unload", lbass_unload },
    { "play", lbass_play },
    { "init", lbass_init },
    { "free", lbass_free },
    { "setVolume", lbass_setVolume },
    { "getVolume", lbass_getVolume },
    { NULL, NULL }
};

int luaopen_lbass(lua_State *L)
{
    luaL_newlib(L, lbass_methods);
    return 1;
}
