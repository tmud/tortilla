#include "stdafx.h"
#include <vector>
#include "bass/c/bass.h"
#pragma comment(lib, "bass/c/bass.lib")
#include "bobjects.h"
#include "common.h"

class BassLoader
{
    bool bass_loaded;
    std::vector<BassObject*> m_objects;
    std::vector<int> m_indexes;
    std::wstring m_error_msg;
public:
    BassLoader() : bass_loaded(false) {}
    ~BassLoader() {}

    bool loadBass()
    {
        if (HIWORD(BASS_GetVersion()) != BASSVERSION)
            return error(L"An incorrect version of bass.dll");
        if (!BASS_Init(-1, 44100, 0, NULL, NULL))
            return error_bass(L"Can't initialize sound system");
        bass_loaded = true;
        return true;
    }

    bool unloadBass()
    {
        if (!bass_loaded)
            return true;
        bass_loaded = false;
        for (int i = 0, e = m_objects.size(); i < e; ++i)
            delete m_objects[i];
        m_objects.clear();
        m_indexes.clear();
        if (!BASS_Free())
            return error_bass(L"Can't unload bass");
        return true;
    }

    int load(const wchar_t* file, bool as_sample)
    {
        if (!bass_loaded)
            { error(L"BASS not initialized"); return -1; }

        HANDLE hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            return error_file(L"Can't open file", file);
        DWORD hsize = 0;
        DWORD size = GetFileSize(hfile, &hsize);
        CloseHandle(hfile);
        if (hsize != 0)
            return error_file(L"File is huge", file);
        if (size == 0)
            return error_file(L"File is empty", file);

        int index = -1;

        // get extension
        bool correct_ext = false;
        std::wstring f(file);
        int pos = f.rfind(L'.');
        if (pos != -1)
        {
            std::wstring ext(f.substr(pos+1));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == L"mp3" || ext == L"wav" || ext == L"ogg" || ext == L"mp2" || ext == L"mp1" || ext == L"aiff")
            {
                correct_ext = true;
                if (as_sample)
                   index = _bass_loader.loadSample(file);
                 else
                   index = _bass_loader.loadStream(file);
            }
            else if (ext == L"mo3" || ext == L"xm" || ext == L"it" || ext == L"s3m" || ext == L"mtm" || ext == L"mod" || ext == L"umx")
            {
                if (!as_sample)
                    return error_file(L"Can't load as stream", file);
                 correct_ext = true;
                 index = _bass_loader.loadMusic(file);
            }
         }
         if (!correct_ext)
             return error_file(L"Unknown file type", file);
         if (index == -1)
             return error_file(L"Can't open file", file);
         return index;
    }

    bool unload(int id)
    {
        BassObject* object = get(id);
        if (!object) 
            return error_id(id);
        m_objects[id] = NULL;
        delete object;
        m_indexes.push_back(id);
        return true;
    }

    bool play(int id, int volume)
    {
        if (!bass_loaded)
            return error(L"BASS not initialized");
        int objects = m_objects.size();
        if (id >= 0 && id < objects)
        {
            BassObject *object = m_objects[id];
            if (!object->play(volume_ToFloat(volume)))
                return error(object->geterror().c_str());
            return true;
        }
        return error_id(id);
    }

    bool stop(int id)
    {
        BassObject *object = get(id);
        if (!object)
            return error_id(id);
        object->stop();
        return true;
    }

    void stopAll()
    {
        for (int i=0,e=m_objects.size();i<e;++i)
            m_objects[i]->stop();
    }

    int isSample(int id)
    {
        BassObject *object = get(id);
        if (!object)
            { error_id(id); return -1; }
        return object->issample() ? 1 : 0;
    }

    int isStream(int id)
    {
        BassObject *object = get(id);
        if (!object)
            { error_id(id); return -1; }
        return object->issample() ? 0 : 1;
    }

    int isPlaying(int id)
    {
        BassObject *object = get(id);
        if (!object)
            { error_id(id); return -1; }
        return object->isplaying() ? 1 : 0;
    }

    bool isHandle(int id) const
    {
        BassObject *object = get(id);
        return (object) ? true : false;
    }

    bool getPath(int id, std::wstring* path)
    {
        BassObject *object = get(id);
        if (!object)
            { error_id(id); return false; }
        path->assign(object->getname());
        return true;    
    }

    const wchar_t* getLastError() const
    {
        return m_error_msg.c_str();
    }

private:
    bool error_bass(const wchar_t* error_text)
    {
       std::wstring msg(error_text);
       msg.append(L" (error code: ");
       wchar_t buffer[16];
       msg.append(_itow(BASS_ErrorGetCode(), buffer, 10));
       msg.append(L")");
       return error(msg.c_str());
    }

    bool error(const wchar_t* error_text)
    {
        m_error_msg.assign(error_text);
        return false;
    }

    bool error_id(int id)
    {
        wchar_t buffer[16];
        _itow(id, buffer, 10);
        m_error_msg.assign(L"Incorrect sound id: ");
        m_error_msg.append(buffer);
        return false;
    }

    int error_file(const wchar_t* error_text, const wchar_t* file)
    {
        m_error_msg.assign(error_text);
        m_error_msg.append(L": ");
        m_error_msg.append(file);
        return -1;
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

    BassObject* get(int id) const
    {
        int count = m_objects.size();
        return (id >= 0 && id < count) ? m_objects[id] : NULL;
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

int lbass_play(lua_State *L)
{
    int n = lua_gettop(L);
    if ((n == 1 && lua_isnumber(L, 1)) || 
        (n == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2)))
    {
        int id = lua_tointeger(L, 1);
        int volume = (n == 2) ? 100 : lua_tointeger(L, 2);
        if (!_bass_loader.play(id, volume))
             return error(L, _bass_loader.getLastError());
        lua_pushboolean(L, 1);
        return 1;
    }
    return error_invargs(L, L"play");
}

int lbass_unload(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        if ( !_bass_loader.unload(lua_tointeger(L,1)) )
            return error(L, _bass_loader.getLastError());
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
        int id = _bass_loader.load(file.c_str(), as_sample);
        if (id == -1)
            return error(L, _bass_loader.getLastError());
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
        bool result = _bass_loader.isHandle(lua_tointeger(L, 1));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isHandle");
}

int lbass_isSample(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int result = _bass_loader.isSample(lua_tointeger(L, 1));
        if (result == -1)
            return error(L, _bass_loader.getLastError());
        lua_pushboolean(L, (result==1) ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isSample");
}

int lbass_isStream(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int result = _bass_loader.isStream(lua_tointeger(L, 1));
        if (result == -1)
            return error(L, _bass_loader.getLastError());
        lua_pushboolean(L, (result==1) ? 1 : 0);
        return 1;
    }
    return error_invargs(L, L"isStream");
}

int lbass_isPlaying(lua_State *L)
{
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        int result = _bass_loader.isPlaying(lua_tointeger(L, 1));
        if (result == -1)
            return error(L, _bass_loader.getLastError());
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
        if (!_bass_loader.getPath(lua_tointeger(L, 1), &path))
            return error(L, _bass_loader.getLastError());
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
       if (!_bass_loader.stop(lua_tointeger(L, 1)))
            return error(L, _bass_loader.getLastError());
       lua_pushboolean(L, 1);
       return 1;
   }
   return error_invargs(L, L"stop");
}

int lbass_stopAll(lua_State *L)
{
    if (lua_gettop(L) == 0)
    {
        _bass_loader.stopAll();
        return 0;
    }
    return error_invargs(L, L"stopAll");
}

int lbass_init(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error_invargs(L, L"init");
    if (!_bass_loader.loadBass())
        return error(L, _bass_loader.getLastError());
    lua_pushboolean(L, 1);
    return 1;
}

int lbass_free(lua_State* L)
{
    if (lua_gettop(L) == 0)
    {
        if (!_bass_loader.unloadBass())
            return error(L, _bass_loader.getLastError());
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

static const luaL_Reg lbass_methods[] =
{
    { "init", lbass_init },
    { "free", lbass_free },
    { "loadSample", lbass_loadSample },
    { "loadStream", lbass_loadStream },
    { "unload", lbass_unload },
    { "isHandle", lbass_isHandle},
    { "isSample", lbass_isSample},
    { "isStream", lbass_isStream},
    { "play", lbass_play },
    { "isPlaying", lbass_isPlaying },
    { "getPath", lbass_getPath },
    { "stop", lbass_stop },
    { "stopAll", lbass_stopAll },
    { "setVolume", lbass_setVolume },
    { "getVolume", lbass_getVolume },
    { NULL, NULL }
};

int luaopen_lbass(lua_State *L)
{
    luaL_newlib(L, lbass_methods);
    return 1;
}
