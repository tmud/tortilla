#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggers.h"

extern Plugin* _cp;

PluginsTrigger::PluginsTrigger() : L(NULL), m_tigger_func_index(0)
{
}

PluginsTrigger::~PluginsTrigger()
{
    lua_getglobal(L, "_triggers");
    if (lua_istable(L, -1) && m_tigger_func_index > 0)
    {
        lua_pushinteger(L, m_tigger_func_index);
        lua_pushnil(L);
        lua_settable(L, -3);
    }
    lua_pop(L, 1);
}

bool PluginsTrigger::init(lua_State *pL)
{
    L = pL;
    assert(luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION));
    m_trigger.assign(lua_tostring(L, 1));
    return reg_trigger();
}

bool PluginsTrigger::reg_trigger()
{
    if (!lua_isfunction(L, -1))
        return false;
    lua_getglobal(L, "_triggers");
    if (!lua_istable(L, -1))
    {
        if (!lua_isnil(L, -1)) {
            lua_pop(L, 1);
            return false;
        }
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "_triggers");
    }
    lua_len(L, -1);
    int index = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_insert(L, -2);
    lua_pushinteger(L, index);
    lua_insert(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);
    m_tigger_func_index = index;
    return true;
}

int trigger_create(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION))
    {
        PluginsTrigger *t = new PluginsTrigger();
        if (!t->init(L))
            { delete t;  t = NULL; }
        else
            { _cp->triggers.push_back(t); }
        luaT_pushobject(L, t, LUAT_TRIGGER);
        return 1;
    }
    return pluginInvArgs(L, "createTrigger");
}

int trigger_enable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
    }
    return pluginInvArgs(L, "trigger:enable");
}

int trigger_disable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
    }
    return pluginInvArgs(L, "trigger:disable");
}

void reg_mt_trigger_string(lua_State *L);
void reg_mt_trigger(lua_State *L)
{
    lua_register(L, "createTrigger", trigger_create);
    luaL_newmetatable(L, "trigger");
    regFunction(L, "enable", trigger_enable);
    regFunction(L, "disable", trigger_disable);
    regIndexMt(L);
    lua_pop(L, 1);
    reg_mt_trigger_string(L);
}

int ts_blocks(lua_State *L)
{
    return 0;
}

void reg_mt_trigger_string(lua_State *L)
{
    luaL_newmetatable(L, "viewstring");
    regFunction(L, "blocks", ts_blocks);
    regIndexMt(L);
    lua_pop(L, 1);
}
