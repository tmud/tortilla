#pragma once
#include <map>

class SoundPlayer
{
    lua_State *L;
    std::wstring *perror;
    std::map<std::wstring, std::wstring> m_files_list;
    typedef std::map<std::wstring, std::wstring>::iterator iterator;

public:
    SoundPlayer(lua_State* l) : L(l), perror(NULL) 
    {
        scanFiles();
    }

    bool isPlayerLoaded() 
    {
        pushPlayer();
        bool result = (lua_istable(L, -1)) ? true : false;
        lua_pop(L, 1);
        return result;
    }

    bool runCommand(const std::vector<std::wstring>& params, std::wstring* error)
    {
        if (params.empty())
            { error->assign(L"Не заданы параметры"); return false; }

        perror = error;
        const std::wstring &cmd = params[0];
        if (cmd == L"play")
            return play(params);
        else if (cmd == L"music")
            return music(params);
        else if (cmd == L"volume")
            return volume(params);
        else if (cmd == L"update")
            return update(params);
        error->assign(L"Неизвестная команда: ");
        error->append(cmd);
        return false;
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
               return incorrectMethod(L"getVolume");
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
               return incorrectMethod(L"setVolume");
           std::wstring v(L"Новая громкость: ");
           v.append( int_to_wstring(volume) );
           print(v);
           return true;
       }
       return incorrectParameters(L"volume");
    }

    bool play(const std::vector<std::wstring>& params)
    {
        /*int count = params.size() - 1;
        if (count == 1 || count == 2)
        {
            int volume = 100;
            if (count == 2) {
                bool check = false;
                volume = wstring_to_int(params[2].c_str(), &check);
                if (!check)
                    return incorrectParameters(L"play");
            }
            const std::wstring &name = params[1];
            iterator it = m_sounds.find(name);
            return;
        }*/
        return incorrectParameters(L"play");
    }

    bool music(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count == 1 || count == 2)
        {
            int volume = 100;
            if (count == 2) {
                bool check = false;
                volume = wstring_to_int(params[2].c_str(), &check);
                if (!check)
                    return incorrectParameters(L"music");
            }
            const std::wstring &name = params[1];
            iterator it = m_files_list.find(name);
            if (it == m_files_list.end())
               return incorrectFile(name.c_str());

            pushPlayer();
            if (!luaT_run(L, "music", "tsd", it->second.c_str(), volume))
               return incorrectMethod(L"music");
            return true;
        }
        return incorrectParameters(L"music");
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

    bool isMusicFile(const std::wstring& ext)
    {
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

    bool incorrectMethod(const wchar_t* method)
    {
        perror->assign(L"Неизвестный метод soundplayer.");
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

    bool incorrectFile(const wchar_t* file)
    {
        perror->assign(L"Не найден звуковой файл '");
        perror->append(file);
        perror->append(L"'");
        return false;
    }

    void pushPlayer() {
        lua_getglobal(L, "soundplayer");
    }
};
