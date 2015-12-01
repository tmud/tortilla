#include "stdafx.h"
#include "saveSoundDlg.h"
#include "soundPlayer.h"

SoundPlayer *player = NULL;

int get_name(lua_State *L)
{
    luaT_pushwstring(L, L"Звуковой плагин");
    return 1;
}

int get_description(lua_State *L)
{
    luaT_pushwstring(L, L"Плагин предназначен для воспроизведения звуковых файлов, а также их записи с микрофона.\r\n"
        L"Воспроизводятся wav,mp3,ogg,s3m,it,xm,mod. Запись с микрофона производится в wav.\r\n"
        L"#sound play file [volume] - воспроизведение музыкального файла или плейлиста (*.lst)\r\n"
        L"#sound playfx|fx file [volume] - воспроизведение  звукового эффекта\r\n"
        L"#sound volume [значение] - устанавливает или показывает текущую мастер-громкость\r\n"
        L"#sound stop - останавливает воспроизведение музыкального файла\r\n"
        L"#sound update - обновляет список файлов, доступных по короткому имени\r\n"
        L"Подробнее в справке #help sound");
    return 1;
}

int get_version(lua_State *L)
{
    luaT_pushwstring(L, L"1.0");
    return 1;
}

int init(lua_State *L)
{
    player = new SoundPlayer(L);
    if (!player->isPlayerLoaded())
    {
        base::log(L, L"Модуль SoundPlayer не загружен.");
        base::terminate(L);
        return 0;
    }
    base::addMenu(L, L"Плагины/Записать звук...", 1);
    base::addCommand(L, L"sound");
    return 0;
}

int release(lua_State *L)
{
    delete player;
    player = NULL;
    return 0;
}

int menucmd(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TNUMBER))
        return 0;
    int menuid = lua_tointeger(L, 1);
    lua_pop(L, 1);
    if (menuid == 1)
    {
        HWND parent = base::getParent(L);
        SaveSoundDlg dlg(player);
        dlg.DoModal(parent);
    }
    return 0;
}

int syscmd(lua_State *L)
{
    if (luaT_check(L, 1, LUA_TTABLE))
    {
        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
        std::wstring cmd(luaT_towstring(L, -1));
        lua_pop(L, 1);
        if (cmd == L"sound")
        {
            std::vector<std::wstring> params;
            int n = luaL_len(L, -1);
            std::wstring text;
            for (int i=2; i<=n; ++i)
            {
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                if (!lua_isstring(L, -1))
                {
                    luaT_pushwstring(L, L"Неверные параметры");
                    return 1;
                }
                else
                {
                    std::wstring p(luaT_towstring(L, -1));
                    params.push_back(p);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            std::wstring error;
            player->runCommand(params, &error);
            if (!error.empty())
                luaT_pushwstring(L, error.c_str() );
            else
                lua_pushnil(L);
            return 1;
        }
    }
    return 1;
}

static const luaL_Reg sound_methods[] =
{
    { "name", get_name },
    { "description", get_description },
    { "version", get_version },
    { "init", init },
    { "release", release },
    { "menucmd", menucmd },
    { "syscmd", syscmd },
    { NULL, NULL }
};

int WINAPI plugin_open(lua_State *L)
{
    luaL_newlib(L, sound_methods);
    lua_setglobal(L, "sound");
    return 0;
}


/*BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}*/
