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
    bool play(const wchar_t* file, int volume, bool as_sample)
    {
        if (!bass_loaded)
            return error(L"bass not initialized");
        int index = -1;
        for (int i=0,e=m_objects.size();i<e;++i)
        {
            BassObject* obj = m_objects[i];
            if (obj && !obj->getname().find(file) && obj->issample() == as_sample && !obj->isplaying())
                { index = i; break; }
        }
        if (index == -1)
        {
            index = load(file);
            if (index == -1)
                return false;
        }
        BassObject *obj = m_objects[index];
        if (!obj->play(volume_ToFloat(volume)))
            return error(obj->geterror().c_str());
        return true;
    }

    void stop(const wchar_t* file)
    {
        for (int i=0,e=m_objects.size();i<e;++i)
        {
            BassObject* obj = m_objects[i];
            if (obj && !obj->getname().find(file))
                obj->stop();
        }
    }

    void stopAll()
    {
        for (int i=0,e=m_objects.size();i<e;++i)
            m_objects[i]->stop();
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
    int error_file(const wchar_t* error_text, const wchar_t* file)
    {
        m_error_msg.assign(error_text);
        m_error_msg.append(L": ");
        m_error_msg.append(file);
        return -1;    
    }

    int load(const wchar_t* file)
    {
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
                /* if (load_as_sample)
                     index = _bass_loader.loadSample(filename.c_str());
                 else
                     index = _bass_loader.loadStream(filename.c_str());*/
            }
            else if (ext == L"mo3" || ext == L"xm" || ext == L"it" || ext == L"s3m" || ext == L"mtm" || ext == L"mod" || ext == L"umx")
            {
                 correct_ext = true;
                 //index = _bass_loader.loadMusic(file);
            }
         }
         if (!correct_ext)
             return error_file(L"Unknown file type", file);
         if (index == -1)
             return error_file(L"Can't open file", file);
         return index;
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

/*int lbass_load_ex(lua_State *L, bool load_as_sample)
{
    if (lua_gettop(L) == 1 && lua_isstring(L, 1))
    {
        TU2W f(lua_tostring(L,1));
        std::wstring filename(f);
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
*/


/*int lbass_load(lua_State *L)
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
}*/

int lbass_play(lua_State *L, const utf8* method, bool as_sample)
{
    int n = lua_gettop(L);
    if ((n == 1 && lua_isstring(L, 1)) || 
        (n == 2 && lua_isstring(L, 1) && lua_isnumber(L, 2)))
    {
        int volume = (n == 2) ? lua_tointeger(L, 2) : 100;
        if (volume >= 0 && volume <= 100)
        {
            S2W file(lua_tostring(L, 1));
            if (!_bass_loader.play(file, volume, as_sample))
            {
                W2S errmsg(_bass_loader.getLastError());
                return error(L, errmsg);
            }
            return 0;
        }
    }
    return error_invargs(L, method);
}

int lbass_playSample(lua_State *L)
{
   return lbass_play(L, "playSample", true);
}

int lbass_playStream(lua_State *L)
{
    return lbass_play(L, "playStream", false);
}

int lbass_stop(lua_State *L)
{
   int n = lua_gettop(L);
   if (n == 1 && lua_isstring(L, 1))
   {
       S2W file(lua_tostring(L, 1));
       _bass_loader.stop(file);
       return 0;
   }
   return error_invargs(L, "stop");
}

int lbass_stopAll(lua_State *L)
{
    if (lua_gettop(L) == 0)
    {
        _bass_loader.stopAll();
        return 0;    
    }
    return error_invargs(L, "stopAll");
}

int lbass_init(lua_State* L)
{
    if (lua_gettop(L) != 0)
        return error_invargs(L, "init");
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
   return error_invargs(L, "free");
}

int lbass_getVolume(lua_State* L)
{
    if (lua_gettop(L) == 0) 
    {
        float v = BASS_GetVolume();
        lua_pushinteger(L, volume_toInt(v) );
        return 1;
    }
    return error_invargs(L, "getVolume");
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
    return error_invargs(L, "setVolume");
}

static const luaL_Reg lbass_methods[] =
{
    { "init", lbass_init },
    { "free", lbass_free },
    { "playSample", lbass_playSample },
    { "playStream", lbass_playStream },
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
