#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggers.h"

extern Plugin* _cp;

PluginsTrigger::PluginsTrigger() : L(NULL), m_tigger_func_index(0), m_enabled(false)
{
}

PluginsTrigger::~PluginsTrigger()
{
    m_enabled = false;
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

    tstring key(U2W(lua_tostring(L, 1)));
    m_compare.init(key, true);

    lua_len(L, -1);
    int index = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_insert(L, -2);
    lua_pushinteger(L, index);
    lua_insert(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1);
    m_tigger_func_index = index;
    m_enabled = true;
    return true;
}

bool PluginsTrigger::compare(const CompareData& cd, bool incompl_flag)
{
    if (!m_enabled)
        return false;
    if (incompl_flag && m_compare.isFullstrReq())
        return false;
    if (!m_compare.compare(cd.fullstr))
        return false;
    lua_getglobal(L, "_triggers");
    lua_pushinteger(L, m_tigger_func_index);
    lua_gettable(L, -2);
    lua_insert(L, -2);
    lua_pop(L, 1);

    PluginsTriggerString vs(cd.string, m_compare);
    luaT_pushobject(L, &vs, LUAT_VIEWSTRING);
    if (lua_pcall(L, 1, 0, 0))
    {
        if (luaT_check(L, 1, LUA_TSTRING))
            pluginError("trigger", lua_tostring(L, -1));
        else
            pluginError("trigger", "неизвестная ошибка");
        lua_settop(L, 0);
    }
    return true;
}

void PluginsTrigger::enable(bool enable)
{
    m_enabled = enable;
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
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(true);
        return 0;
    }
    return pluginInvArgs(L, "trigger:enable");
}

int trigger_disable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(false);
        return 0;
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

int ts_getBlocksCount(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushinteger(L, s->blocks());
        return 1;
    }
    return pluginInvArgs(L, "viewstring:getBlocksCount");
}

int ts_getText(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        tstring text;
        s->getText(&text);
        lua_pushstring(L, W2U(text.c_str()) );
        return 1;
    }
    return pluginInvArgs(L, "viewstring:getText");
}

int ts_getParameter(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWSTRING, LUA_TNUMBER))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        tstring p;
        if (!s->getParam( lua_tointeger(L, 2), &p))
            lua_pushnil(L);
        else
            lua_pushstring(L, TW2U(p.c_str()));
        return 1;
    }
    return pluginInvArgs(L, "viewstring:getParameter");
}

int ts_getParamsCount(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushinteger(L, s->getParamsCount());
        return 1;
    }
    return pluginInvArgs(L, "viewstring:getParamsCount");
}

int ts_getComparedText(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        tstring p;
        if (!s->getCompared(&p))
            lua_pushnil(L);
        else
            lua_pushstring(L, TW2U(p.c_str()));
        return 1;
    }
    return pluginInvArgs(L, "viewstring:getParamsCount");
}

void reg_mt_trigger_string(lua_State *L)
{
    luaL_newmetatable(L, "viewstring");
    regFunction(L, "getBlocksCount", ts_getBlocksCount);
    regFunction(L, "getText", ts_getText);
    regFunction(L, "getParamsCount", ts_getParamsCount);
    regFunction(L, "getParameter", ts_getParameter);
    regFunction(L, "getComparedText", ts_getComparedText);
    regIndexMt(L);
    lua_pop(L, 1);
}
