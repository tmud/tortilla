#pragma once
#include <map>

class SoundPlayer
{
    lua_State *L;
    std::wstring *perror;
    //int m_music;
    //std::map<std::wstring, int> m_sounds;
    //typedef std::map<std::wstring, int>::iterator iterator;

public:
    SoundPlayer(lua_State* l) : L(l), perror(NULL) {}
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
            pushPlayer();
            if (!luaT_run(L, "music", "tsd", name.c_str(), volume))
               return incorrectMethod(L"music");
            return true;
        }
        return incorrectParameters(L"music");
    }

    /*bool runInt_Bool(const char* method, int param, bool* result)
    {
        pushBass();
        if (!luaT_run(L, method, "td", param))
            return incorrectMethod(TA2W(method));
        if (!lua_isboolean(L, -1))
            return incorrectResult(TA2W(method));
        *result = lua_toboolean(L, -1) ? true : false;
        lua_pop(L, 1);
        return true;
    }

    bool runString_Int(const char* method, const wchar_t* param, int* result)
    {
        pushBass();
        if (!luaT_run(L, method, "ts", param))
            return incorrectMethod(TA2W(method));
        if (!lua_isnumber(L, -1))
            return incorrectResult(TA2W(method));
        *result = lua_tointeger(L, -1) ? true : false;
        lua_pop(L, 1);
        return true;
    }*/

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
    void pushPlayer() {
        lua_getglobal(L, "soundplayer");
    }
};
