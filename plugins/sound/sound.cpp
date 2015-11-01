#include "stdafx.h"
#include "saveSoundDlg.h"
#include "soundPlayer.h"

SoundPlayer *player = NULL;

int get_name(lua_State *L)
{
    lua_pushwstring(L, L"�������� ������");
    return 1;
}

int get_description(lua_State *L)
{
    lua_pushwstring(L, L"������ ������������ ��� ��������������� �������� ������, � ����� �� ������ � ���������.\r\n"
        L"��������������� wav,mp3,ogg,aiff,s3m,it,xm,mod,umx. ������ � ��������� ������������ � wav.");
    return 1;
}

int get_version(lua_State *L)
{
    lua_pushwstring(L, L"1.0");
    return 1;
}

int init(lua_State *L)
{
    player = new SoundPlayer(L);
    if (!player->isBassLoaded())
    {
        base::log(L, L"������ Bass �� ��������.");
        base::terminate(L);
        return 0;
    }
    base::addMenu(L, "�������/�������� ����...", 1);
    base::addCommand(L, "sound");
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
        SaveSoundDlg dlg;
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
        tstring cmd(lua_towstring(L, -1));
        lua_pop(L, 1);
        if (cmd == L"sound")
        {
            std::vector<tstring> params;
            int n = luaL_len(L, -1);
            u8string text;
            for (int i=2; i<=n; ++i)
            {
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                if (!lua_isstring(L, -1))
                {
                    lua_pushwstring(L, L"�������� ���������");
                    return 1;
                }
                else
                {
                    tstring p(lua_towstring(L, -1));
                    params.push_back(p);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
            tstring error;
            player->runCommand(params, &error);
            if (!error.empty())
                lua_pushwstring(L, error.c_str() );
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
