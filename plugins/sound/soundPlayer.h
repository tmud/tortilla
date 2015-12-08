#pragma once
#include <map>
#include "api/base.h"

int endplaying(lua_State *L);

class SoundPlayerCallback
{
public:
    virtual void endPlaying() = 0;
};

struct SoundPlayerRecordParams
{
    SoundPlayerRecordParams() : sensitivity(75), destfolder(0) {}
    int sensitivity;
    int destfolder;
};

class SoundPlayer : public SoundPlayerCallback
{
    lua_State *L;
    std::wstring *perror;
    std::map<std::wstring, std::wstring> m_files_list;
    typedef std::map<std::wstring, std::wstring>::iterator iterator;
    int m_playing_music;
    SoundPlayerCallback *m_pcb;
    int m_replay_sound_id;
    SoundPlayerRecordParams m_recording_params;

public:
    SoundPlayer(lua_State* l) : L(l), perror(NULL), m_playing_music(-2), m_pcb(NULL), m_replay_sound_id(-1)
    {
        if (base::loadTable(L, L"config.xml"))
        {
            lua_pushstring(L, "sensitivity");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
            {
                int value = lua_tointeger(L, -1);
                value = max(min(value, 100), 0);
                m_recording_params.sensitivity = value;
            }
            lua_pop(L, 1);
            lua_pushstring(L, "destination");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
            {
                int value = lua_tointeger(L, -1);
                if (value != 1) value = 0;
                m_recording_params.destfolder = value;
            }
            lua_pop(L, 1);
            lua_pushstring(L, "volume");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
            {
                int value = lua_tointeger(L, -1);
                value = max(min(value, 100), 0);
                pushPlayer();
                luaT_run(L, "setVolume", "td", value);
            }
            lua_pop(L, 2);
        }
        scanFiles();
    }

    ~SoundPlayer()
    {
       stopMusic();
       lua_newtable(L);
       lua_pushstring(L, "sensitivity");
       lua_pushinteger(L, m_recording_params.sensitivity);
       lua_settable(L, -3);
       lua_pushstring(L, "destination");
       lua_pushinteger(L, m_recording_params.destfolder);
       lua_settable(L, -3);
       pushPlayer();
       if (luaT_run(L, "getVolume", "t") && lua_isnumber(L, -1))
       {
           lua_pushstring(L, "volume");
           lua_insert(L, -2);
           lua_settable(L, -3);
       }
       base::saveTable(L, L"config.xml");
    }

    bool isPlayerLoaded() 
    {
        pushPlayer();
        bool result = (lua_istable(L, -1)) ? true : false;
        lua_pop(L, 1);
        return result;
    }
    bool runPlayCommand(const std::vector<std::wstring>& params, std::wstring* error)
    {
        perror = error;
        int count = params.size();
        if (count == 0)
        {
            stopMusic();
            return true;
        }
        if (count == 1 || count == 2)
        {
            int volume = 100;
            if (count == 2) {
                bool check = false;
                volume = wstring_to_int(params[1].c_str(), &check);
                if (!check)
                {
                    error->assign(L"Неверный набор параметров");
                    return false;
                }
            }
            std::wstring name = params[0];
            iterator it = m_files_list.find(name);
            if (it != m_files_list.end())
                name = it->second;

            int pos = name.rfind(L'.');
            if (pos != -1)
            {
                std::wstring ext(name.substr(pos + 1));
                if (ext == L"lst")
                {
                    bool result = playlist(name, volume);
                    if (result)
                        m_playing_music = -1;   // playlist id
                    return result;
                }
            }

            pushPlayer();
            if (!luaT_run(L, "play", "tsd", name.c_str(), volume))
                return incorrectCall(L"play");
             m_playing_music = lua_tointeger(L, -1);
            return true;
        }
        error->assign(L"Неверный набор параметров");
        return false;
    }

    bool runCommand(const std::vector<std::wstring>& params, std::wstring* error)
    {
        if (params.empty())
            { error->assign(L"Не заданы параметры"); return false; }
        perror = error;
        const std::wstring &cmd = params[0];
        if (cmd == L"playfx" || cmd == L"fx")
            return playfx(params);
        else if (cmd == L"play")
            return play(params);
        else if (cmd == L"volume")
            return volume(params);
        else if (cmd == L"update")
            return update(params);
        else if (cmd == L"stop")
            return stop(params);
        error->assign(L"Неизвестная команда: ");
        error->append(cmd);
        return false;
    }

    bool startRecord(const wchar_t* filename, std::wstring* error)
    {
        pushPlayer();
        if (!luaT_run(L, "startRecord", "tsd", filename, m_recording_params.sensitivity))
            return setError(error);
        return true;
    }

    bool stopRecord(std::wstring* error)
    {
        pushPlayer();
        if (!luaT_run(L, "stopRecord", "t"))
            return setError(error);
        return true;
    }

    SoundPlayerRecordParams& recordingParams()
    {
        return m_recording_params;
    }

    bool saveFile(const wchar_t* temp_file, const wchar_t* filename)
    {
        std::wstring destpath;
        if (m_recording_params.destfolder)
          base::getResource(L, filename, &destpath);
        else
          base::getPath(L, filename, &destpath);
        bool result = CopyFile(temp_file, destpath.c_str(), TRUE) ? true : false;
        if (result)
            scanFiles();
        return result;
    }

    bool playFile(const wchar_t* file, std::wstring* error, SoundPlayerCallback *cb)
    {
        if (m_replay_sound_id != -1)
        {
            if (!luaT_run(L, "stop", "td", m_replay_sound_id))
                return setError(error);
            m_replay_sound_id = -1;
        }

        m_pcb = cb;
        pushPlayer();
        if (!luaT_run(L, "play", "tsdF", file, 100, (void*)endplaying ))
            return setError(error);
        m_replay_sound_id = lua_tointeger(L, -1);
        return true;
    }

    bool stopPlayFile(std::wstring* error)
    {
        if (m_replay_sound_id == -1)
            return true;
        pushPlayer();
        if (!luaT_run(L, "stop", "td", m_replay_sound_id))
            return setError(error);
        m_replay_sound_id = -1;
        return true;
    }

    void endPlaying()
    {
        if (m_pcb)
            m_pcb->endPlaying();
        m_pcb = NULL;
    }

private:
    bool volume(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count != 0 && count != 1)
            return incorrectParameters(L"volume");
        if (count == 0)
        {
            pushPlayer();
            if (!luaT_run(L, "getVolume", "t"))
               return incorrectCall(L"getVolume");
            if (!lua_isnumber(L, -1))
                return incorrectResult(L"getVolume");
            std::wstring v(L"Текущая громкость: ");
            v.append( luaT_towstring(L, -1) );
            print(v);
            return true;
       }
       bool check = false;
       int volume = wstring_to_int(params[1].c_str(), &check);
       if (check && volume >= 0 && volume <= 100)
       {
           pushPlayer();
           if (!luaT_run(L, "setVolume", "td", volume))
               return incorrectCall(L"setVolume");
           std::wstring v(L"Новая громкость: ");
           v.append( int_to_wstring(volume) );
           print(v);
           return true;
       }
       return incorrectParameters(L"volume");
    }

    bool playfx(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count == 1 || count == 2)
        {
            int volume = 100;
            if (count == 2) {
                bool check = false;
                volume = wstring_to_int(params[2].c_str(), &check);
                if (!check)
                    return incorrectParameters(L"playfx");
            }
            std::wstring name = params[1];
            iterator it = m_files_list.find(name);
            if (it != m_files_list.end())
                name = it->second;
            pushPlayer();
            if (!luaT_run(L, "playfx", "tsd", name.c_str(), volume))
                return incorrectCall(L"playfx");
            return true;
        }
        return incorrectParameters(L"playfx");
    }

    bool play(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count == 1 || count == 2)
        {
            int volume = 100;
            if (count == 2) {
                bool check = false;
                volume = wstring_to_int(params[2].c_str(), &check);
                if (!check)
                    return incorrectParameters(L"play");
            }
            std::wstring name = params[1];
            iterator it = m_files_list.find(name);
            if (it != m_files_list.end())
                name = it->second;

            int pos = name.rfind(L'.');
            if (pos != -1)
            {
                std::wstring ext(name.substr(pos+1));
                if (ext == L"lst")
                {
                    bool result = playlist(name, volume);
                    if (result)
                        m_playing_music = -1;   // playlist id
                    return result;
                }
            }

            pushPlayer();
            if (!luaT_run(L, "play", "tsd", name.c_str(), volume))
               return incorrectCall(L"play");
            m_playing_music = lua_tointeger(L, -1);
            return true;
        }
        return incorrectParameters(L"play");
    }

    bool stop(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count == 0)
        {
            stopMusic();
            return true;
        }
        return incorrectParameters(L"stop");
    }

    void stopMusic()
    {
        if (m_playing_music != -2) {
            pushPlayer();
            luaT_run(L, "stop", "td", m_playing_music);
        }
        m_playing_music = -2;
    }

    bool update(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count == 0)
        {
            scanFiles();
            return true;
        }
        return incorrectParameters(L"update");
    }

    void replace(std::wstring *str)
    {
        size_t pos = 0;
        while ((pos = str->find(L"\\", pos)) != std::string::npos)
        {
            str->replace(pos, 1, L"\\\\");
            pos += 2;
        }
    }

    bool playlist(const std::wstring& playlist, int volume)
    {
        std::wstring list(playlist);
        replace(&list);
        lua_getglobal(L, "system");
        if (!luaT_run(L, "loadTextFile", "ts", list.c_str()))
            return false;
        if (!lua_istable(L, -1))
            return incorrectPlaylist(playlist.c_str());;

        lua_len(L, -1);
        int len = lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (len == 0)
        {
            base::log(L, L"[sound] Список пуст");
            return false;
        }
        for (int i=1;i<=len;++i)
        {
            lua_pushinteger(L, i);
            lua_gettable(L, -2);
            if (!lua_isstring(L, -1))
            {
                lua_settop(L, 0);
                return incorrectPlaylist(playlist.c_str());
            }
            std::wstring name(luaT_towstring(L, -1));
            lua_pop(L, 1);
            iterator it = m_files_list.find(name);
            if (it != m_files_list.end())
            {
                name = it->second;
                lua_pushinteger(L, i);
                luaT_pushwstring(L, name.c_str());
                lua_settable(L, -3);
            }
        }

        pushPlayer();
        lua_insert(L, -2);
        if (!luaT_run(L, "playlist", "ttd", volume))
            return incorrectCall(L"playlist");
        return true;
    }

    bool isMusicFile(const std::wstring& ext)
    {
       if (ext == L"lst") return true;  // playlist file
       return (ext == L"wav" || ext == L"mp3" || ext == L"ogg" || ext == L"s3m" || ext == L"it" || ext == L"xm" || ext == L"mod" ) ? true : false;
    }

    void scanCurrentDir(const wchar_t* current_dir)
    {
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    const wchar_t *file = fd.cFileName;
                    const wchar_t *e = wcsrchr(file, L'.');
                    if (e)
                    { 
                        std::wstring ext(e+1);
                        if (isMusicFile(ext))
                        {
                            std::wstring id(file);
                            std::wstring path(current_dir);
                            path.append(file);
                            m_files_list[id] = path;
                        }
                    }
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
    }

    void scanFiles()
    {
        std::wstring path;
        base::getResource(L, L"", &path);

        DWORD buffer_required = ::GetCurrentDirectory(0, NULL);
        wchar_t * buffer = new wchar_t[buffer_required];
        GetCurrentDirectory(buffer_required, buffer);
        if (SetCurrentDirectory(path.c_str()))
        {
            scanCurrentDir(path.c_str());
        }
        SetCurrentDirectory(buffer);
        base::getPath(L, L"", &path);
        if (SetCurrentDirectory(path.c_str()))
        {
            scanCurrentDir(path.c_str());
        }
        SetCurrentDirectory(buffer);
        delete []buffer;
    }

    void print(const std::wstring& message)
    {
        std::wstring text(L"[sound] ");
        text.append(message);
        base::print(L, text.c_str() );
    }

    bool incorrectCall(const wchar_t* method)
    {
        perror->assign(L"Ошибка при вызове метода soundplayer.");
        perror->append(method);
        return false;
    }
    bool incorrectResult(const wchar_t* method)
    {
        perror->assign(L"Некорректный результат из функции soundplayer.");
        perror->append(method);
        return false;
    }
    bool incorrectParameters(const wchar_t* cmd)
    {
        perror->assign(L"Некорректные параметры для команды '");
        perror->append(cmd);
        perror->append(L"'");
        return false;
    }

    bool incorrectPlaylist(const wchar_t* file)
    {
        perror->assign(L"Некорректный файл плейлиста '");
        perror->append(file);
        perror->append(L"'");
        return false;
    }

    bool setError(std::wstring* error)
    {
        if (lua_isstring(L, -1))
            error->assign(luaT_towstring(L, -1));
        return false;
    }

    void pushPlayer() {
        lua_getglobal(L, "soundplayer");
    }
};

extern SoundPlayer* player;
int endplaying(lua_State *L)
{
    if (player)
        player->endPlaying();
    return 0;
}
