#pragma once
#include <map>

class SoundPlayer
{
    lua_State *L;
    std::wstring *perror;
    int m_music;
    std::map<std::wstring, int> m_sounds;
    typedef std::map<std::wstring, int>::iterator iterator;

public:
    SoundPlayer(lua_State* l) : L(l), perror(NULL), m_music(-1) {}
    bool isBassLoaded() 
    {
        pushBass();
        bool result = (lua_istable(L, -1)) ? true : false;
        lua_pop(L, 1);
        return result;
    }

    void runCommand(const std::vector<std::wstring>& params, std::wstring* error)
    {
        if (params.empty())
            { error->assign(L"Не заданы параметры"); return; }

        perror = error;
        const std::wstring &cmd = params[0];
        if (cmd == L"play")
            return play(params);
        else if (cmd == L"music")
            return music(params);
        else if (cmd == L"volume")
            return volume(params);
        error->assign(L"Неизвестная команда: ");
        error->append(cmd);
    }

private:
    void volume(const std::vector<std::wstring>& params)
    {
        int count = params.size() - 1;
        if (count != 0 && count != 1)
            return incorrectParameters(L"volume");

        if (count == 0)
        {
            pushBass();
            if (!luaT_run(L, "getVolume", "t"))
               return incorrectMethod(L"getVolume");
            if (!lua_isnumber(L, -1))
                return incorrectResult(L"getVolume");
            std::wstring v(L"Текущая громкость: ");
            v.append( luaT_towstring(L, -1) );
            print(v);
            return;
       }

       bool check = false;
       int volume = wstring_to_int(params[1].c_str(), &check);
       if (check && volume >= 0 && volume <= 100)
       {
           pushBass();
           if (!luaT_run(L, "setVolume", "td", volume))
               return incorrectMethod(L"setVolume");
           std::wstring v(L"Установлена громкость: ");
           v.append( int_to_wstring(volume) );
           print(v);
           return;
       }

       return incorrectParameters(L"volume");
    }

    void play(const std::vector<std::wstring>& params)
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
            const std::wstring &name = params[1];
            iterator it = m_sounds.find(name);



            return;
        }
        return incorrectParameters(L"play");
    }

    void music(const std::vector<std::wstring>& params)
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

            return;
        }
        return incorrectParameters(L"music");
    }

    void print(const std::wstring& message)
    {
        std::wstring text(L"[sound] ");
        text.append(message);
        base::print(L, text.c_str() );
    }

    void pushBass() {
        lua_getglobal(L, "bass");
    }

    void incorrectMethod(const wchar_t* method)
    {
        perror->assign(L"Неизвестный метод bass.");
        perror->append(method);
    }
    void incorrectResult(const wchar_t* method)
    {
        perror->assign(L"Некорректный результат из функции bass.");
        perror->append(method);
    }
    void incorrectParameters(const wchar_t* cmd)
    {
        perror->assign(L"Некорректные параметры для команды '");
        perror->append(cmd);
        perror->append(L"'");
    }
};
