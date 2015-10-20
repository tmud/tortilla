#include "stdafx.h"
#include "src/tinymt32.h"

tinymt32_t random_handler;

int rnd_diap(int a, int b)
{
    if (a > b) { int t = a; a = b; b = t; }
    float r = tinymt32_generate_floatOC(&random_handler);
    float t = static_cast<float>(b - a + 1);
    r = t * r;
    int result = static_cast<int>(r);
    result = result + a;
    if (result > b) result = b;
    return result;
}

int rnd_seed(lua_State *L)
{
    int result = 0;
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        unsigned int seed = lua_tounsigned(L, 1);
        tinymt32_init(&random_handler, seed);
        result = 1;
    }
    lua_pushboolean(L, result);
    return 1;
}

int rnd_rand(lua_State *L)
{
    bool params_ok = false;
    int a = 0; int b = 0;
    if (lua_gettop(L) == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        a = lua_tointeger(L, 1);
        b = lua_tointeger(L, 2);
        params_ok = true;
    }
    if (!params_ok)
        lua_pushnil(L);
    else
        lua_pushinteger(L, rnd_diap(a, b));
    return 1;
}

int rnd_float(lua_State *L)
{
    float r = tinymt32_generate_float(&random_handler);
    lua_pushnumber(L, r);
    return 1;
}

int rnd_uint(lua_State *L)
{
    unsigned int r = tinymt32_generate_uint32(&random_handler);
    lua_pushunsigned(L, r);
    return 1;
}

const char* symbols = "0123456789 abcdefghijklmnopqrstuvwxyz _-ABCDEFGHIJKLMNOPQRSTUVWXYZ _-: ";
int rnd_string(lua_State *L)
{
    bool params_ok = false;
    int a = 0; int b = 0;
    if (lua_gettop(L) == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2))
    {
        a = lua_tointeger(L, 1);
        b = lua_tointeger(L, 2);
        if (a >= 0 && b >= 0)
            params_ok = true;       
    }
    if (lua_gettop(L) == 1 && lua_isnumber(L, 1))
    {
        b = lua_tointeger(L, 1);
        if (b >= 0)
            params_ok = true;
    }
    if (!params_ok)
        lua_pushnil(L);
    else
    {
        if (a > b) { int t = a; a = b; b = t; }
        if (a != b)
            a = rnd_diap(a, b);
        if (a == 0)
            lua_pushstring(L, "");
        else
        {   int maxs = strlen(symbols) - 1;
            char *t = new char[a+1];
            for (int i=0;i<a;++i)
            {
                int index = rnd_diap(0, maxs);
                t[i] = symbols[index];
            }
            t[a] = 0;
            lua_pushstring(L, t);
            delete []t;
        }
    }
    return 1;
}

static const luaL_Reg rnd_methods[] =
{
    { "seed", rnd_seed },
    { "rand", rnd_rand },
    { "uint", rnd_uint },
    { "float", rnd_float },
    { "string", rnd_string },
    { NULL, NULL }
};

int luaopen_rnd(lua_State *L)
{
    luaL_newlib(L, rnd_methods);
    return 1;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason_for_call, LPVOID)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        DWORD ticks = GetTickCount();
        WORD t1 = LOWORD(ticks);
        WORD t2 = HIWORD(ticks);
        uint32_t seeds[8];
        seeds[0] = MAKELONG(st.wDay^t1, st.wDayOfWeek);
        seeds[1] = MAKELONG(st.wMilliseconds, st.wHour^st.wMonth);
        seeds[2] = MAKELONG(st.wYear^t2, st.wDay);
        seeds[3] = MAKELONG(st.wMilliseconds^st.wYear, st.wSecond);
        seeds[4] = MAKELONG(st.wMonth, st.wMilliseconds^t2);
        seeds[5] = MAKELONG(st.wDayOfWeek, t1);
        seeds[6] = MAKELONG(st.wMinute^t1, st.wHour^st.wMilliseconds);
        seeds[7] = MAKELONG(st.wSecond^st.wDayOfWeek, st.wYear^st.wSecond);
        tinymt32_init_by_array(&random_handler, seeds, 8);
    }
    return TRUE;
}