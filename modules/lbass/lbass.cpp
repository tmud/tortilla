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
    int find(const wchar_t* file)
    {
        for (int i=0,e=m_objects.size();i<e;++i)
        {
            if (!m_objects[i]->getname().find(file))
                return i;
        }
        return -1;
    }
    int getCount() const { return m_objects.size(); }
    BassObject* get(int index) const { return m_objects[index]; }
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

int lbass_load(lua_State *L)
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
                if (size >= 128*1024)
                    index = _bass_loader.loadStream(filename.c_str());
                else
                    index = _bass_loader.loadSample(filename.c_str());            
            }
            else if (ext == L"mo3" || ext == L"xm" || ext == L"it" || ext == L"s3m" || ext == L"mtm" || ext == L"mod" || ext == L"umx")
            {
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
    return error(L, "Incorrect parameters", false);
}

BassObject* getObject(lua_State *L)
{
    if (lua_gettop(L) != 1 || !lua_isnumber(L, 1))
        return NULL;
    int index = lua_tointeger(L, 1);
    if (index >= 0 && index <_bass_loader.getCount())
        return _bass_loader.get(index);
    return NULL;
}

int lbass_play(lua_State *L)
{
    bool result = false;
    BassObject *obj = getObject(L);
    if (obj)
        result = obj->play();
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
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
    { "load", lbass_load },
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
