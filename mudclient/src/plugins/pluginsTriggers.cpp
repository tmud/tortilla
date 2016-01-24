#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggers.h"
#include "accessors.h"
extern Plugin* _cp;

PluginsTrigger::PluginsTrigger() : L(NULL), m_current_compare_pos(0), m_enabled(false), m_triggered(false)
{
}

PluginsTrigger::~PluginsTrigger()
{
    reset();
    m_trigger_func_ref.unref(L);
}

bool PluginsTrigger::init(lua_State *pl) //, Plugin *pp)
{
    assert(pl); // && pp);
    L = pl; 
    //p = pp;
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION) ||
        luaT_check(L, 2, LUA_TTABLE, LUA_TFUNCTION))
    {
        m_trigger_func_ref.createRef(L);  // save function on the top of stack
        if (lua_isstring(L, 1))
        {
            m_compare_objects.resize(1);
            tstring key(luaT_towstring(L, 1));
            m_compare_objects[0].init(key, true);
        }
        else
        {
            std::vector<tstring> keys;
            lua_pushnil(L);                     // first key
            while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
            {
                if (!lua_isstring(L, -1))
                    return false;
                tstring key(luaT_towstring(L, -1));
                keys.push_back(key);
                lua_pop(L, 1);
            }
            m_compare_objects.resize(keys.size());
            for (int i=0,e=keys.size();i<e;++i) {
                m_compare_objects[i].init(keys[i], true);
            }
        }
        m_enabled = true;
        return true;
    }
    return false;
}

void PluginsTrigger::reset()
{
    m_current_compare_pos = 0;
    std::for_each(m_strings.begin(), m_strings.end(), [](PluginsTriggerString* pts) {delete pts;} );
    m_strings.clear();
    m_triggered = false;
}

void PluginsTrigger::enable(bool enable)
{
    if (enable != m_enabled)
        reset();
    m_enabled = enable;    
}

bool PluginsTrigger::isEnabled() const
{
    return m_enabled;
}

int PluginsTrigger::getLen() const
{
    return m_compare_objects.size();
}

bool PluginsTrigger::compare(const CompareData& cd, bool incompl_flag)
{
    if (!m_enabled)
        return false;
    if (m_triggered)
        reset();
    bool result = false;
    CompareObject &co = m_compare_objects[m_current_compare_pos];
    if (incompl_flag && co.isFullstrReq()) {
        // not compared / full string req.
    }
    else {
        result = co.compare(cd.fullstr);
    }
    if (result)
    {
        PluginsTriggerString *pts = new PluginsTriggerString(co, cd);
        m_strings.push_back(pts);
        int last = m_compare_objects.size() - 1;
        if (m_current_compare_pos == last)
        {
            m_triggered = true;
            return true;
        }
        m_current_compare_pos++;
        return false;
    }

    if (m_current_compare_pos > 0)
    {
        reset();
        return compare(cd, incompl_flag);
    }
    return false;
}

void PluginsTrigger::run()
{
    /*m_trigger_func_ref.pushValue(L);

    PluginsTriggerString vs(cd.string, m_compare);
    luaT_pushobject(L, &vs, LUAT_VIEWSTRING);
    if (lua_pcall(L, 1, 0, 0))
    {
        if (luaT_check(L, 1, LUA_TSTRING))
            pluginError(L"trigger", luaT_towstring(L, -1));
        else
            pluginError(L"trigger", L"неизвестная ошибка");
        lua_settop(L, 0);
    }*/
    reset();
}

int trigger_create(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION))
    {
        PluginsTrigger *t = new PluginsTrigger();
        if (!t->init(L))
            { delete t; }
        else
        {
            _cp->triggers.push_back(t);
            luaT_pushobject(L, t, LUAT_TRIGGER);
            return 1;
        }
    }
    return pluginInvArgs(L, L"createTrigger");
}

int trigger_enable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(true);
        return 0;
    }
    return pluginInvArgs(L, L"trigger:enable");
}

int trigger_disable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(false);
        return 0;
    }
    return pluginInvArgs(L, L"trigger:disable");
}

//void reg_mt_trigger_string(lua_State *L);
void reg_mt_trigger(lua_State *L)
{
    lua_register(L, "createTrigger", trigger_create);
    luaL_newmetatable(L, "trigger");
    regFunction(L, "enable", trigger_enable);
    regFunction(L, "disable", trigger_disable);
    regIndexMt(L);
    lua_pop(L, 1);
    //reg_mt_trigger_string(L);
}

/*int ts_getBlocksCount(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushinteger(L, s->string()->blocks.size());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getBlocksCount");
}

int ts_getText(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        tstring text;
        s->string()->getText(&text);
        luaT_pushwstring(L, text.c_str() );
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getText");
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
            luaT_pushwstring(L, p.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getParameter");
}

int ts_getParamsCount(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushinteger(L, s->getParamsCount());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getParamsCount");
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
            luaT_pushwstring(L, p.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getParamsCount");
}

int ts_isPrompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushboolean(L, (s->string()->prompt > 0) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:isPrompt");
}

int ts_getPrompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        if (s->string()->prompt <= 0)
            lua_pushnil(L);
        else {
            tstring text;
            s->string()->getPrompt(&text);
            luaT_pushwstring(L, text.c_str());
        }
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getPrompt");
}

int ts_isGameCmd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushboolean(L, s->string()->gamecmd ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:isGameCmd");
}

int ts_isSystem(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        lua_pushboolean(L, s->string()->system ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:isSystem");
}

int ts_drop(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        s->string()->dropped = true;
        return 0;
    }
    return pluginInvArgs(L, L"viewstring:drop");
}

int ts_deleteBlock(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING, LUA_TNUMBER))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        MudViewString *vs = s->string();
        bool ok = false;
        int size = vs->blocks.size();
        int index = lua_tointeger(L, 2);
        if (index >= 1 && index <= size)
        {
            index = index - 1;
            vs->blocks.erase(vs->blocks.begin() + index);
            ok = true;        
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 0;
    }
    return pluginInvArgs(L, L"viewstring:deleteBlock");
}

int ts_deleteAllBlocks(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsTriggerString *s = (PluginsTriggerString*)luaT_toobject(L, 1);
        s->string()->blocks.clear();
        return 0;
    }
    return pluginInvArgs(L, L"viewstring:drop");
}

int vd_gettype(const tchar* type);
tbyte _check(unsigned int val, unsigned int min, unsigned int max);
int ts_get(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_VIEWSTRING, LUA_TNUMBER, LUA_TNUMBER)||
        luaT_check(L, 3, LUAT_VIEWSTRING, LUA_TNUMBER, LUA_TSTRING))
    {
        int type = 0;
        if (lua_isnumber(L, 3))
            type = lua_tointeger(L, 3);
        else
        {
            type = vd_gettype(luaT_towstring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, L"viewstring:get");
        }

        bool ok = false;
        PluginsTriggerString *s = (PluginsTriggerString *)luaT_toobject(L, 1);
        MudViewString* str = s->string();
        {
            int block = lua_tointeger(L, 2);
            int size = str->blocks.size();
            if (block >= 1 && block <= size)
            {
                MudViewStringParams &p = str->blocks[block-1].params;
                ok = true;
                switch (type)
                {
                    case luaT_ViewData::TEXTCOLOR:
                        if (p.use_ext_colors)
                            ok = false;
                        else
                        {
                            tbyte color = p.text_color;
                            if (color <= 7 && p.intensive_status) color += 8;
                            lua_pushunsigned(L, color);
                        }
                        break;
                    case luaT_ViewData::BKGCOLOR:
                        if (p.use_ext_colors)
                            ok = false;
                        else
                            lua_pushunsigned(L, p.bkg_color);
                        break;
                    case luaT_ViewData::UNDERLINE:
                        lua_pushunsigned(L, p.underline_status);
                        break;
                    case luaT_ViewData::ITALIC:
                        lua_pushunsigned(L, p.italic_status);
                        break;
                    case luaT_ViewData::BLINK:
                        lua_pushunsigned(L, p.blink_status);
                        break;
                    case luaT_ViewData::REVERSE:
                        lua_pushunsigned(L, p.reverse_video);
                        break;
                    case luaT_ViewData::EXTTEXTCOLOR:
                        if (p.use_ext_colors)
                            lua_pushunsigned(L, p.ext_text_color);
                        else
                            ok = false;
                        break;
                    case luaT_ViewData::EXTBKGCOLOR:
                        if (p.use_ext_colors)
                            lua_pushunsigned(L, p.ext_bkg_color);
                        else
                            ok = false;
                        break;
                    default:
                        ok = false;
                        break;
                }
            }
        }
        if (!ok)
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:get");
}

int ts_set(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER) || 
        luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TSTRING, LUA_TNUMBER))
    {
        int type = 0;
        if (lua_isnumber(L, 3))
            type = lua_tointeger(L, 3);
        else
        {
            type = vd_gettype(luaT_towstring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, L"viewstring:set");
        }

        bool ok = false;
        PluginsTriggerString *s = (PluginsTriggerString *)luaT_toobject(L, 1);
        MudViewString* str = s->string();
        {
            int block = lua_tointeger(L, 2);
            int size = str->blocks.size();
            if (block >= 1 && block <= size)
            {
                unsigned int v = lua_tounsigned(L, 4);
                MudViewStringParams &p = str->blocks[block-1].params;
                ok = true;
                switch (type)
                {
                case luaT_ViewData::TEXTCOLOR:
                    if (p.use_ext_colors)
                        p.bkg_color = 0;
                    p.use_ext_colors = 0;
                    p.intensive_status = 0;
                    p.text_color = _check(v, 0, 255);
                    break;
                case luaT_ViewData::BKGCOLOR:
                    if (p.use_ext_colors)
                        p.text_color = 7;
                    p.use_ext_colors = 0;
                    p.intensive_status = 0;
                    p.bkg_color = _check(v, 0, 255);
                    break;                
                case luaT_ViewData::UNDERLINE:
                    p.underline_status = _check(v, 0, 1);
                    break;
                case luaT_ViewData::ITALIC:
                    p.italic_status = _check(v, 0, 1);
                    break;
                case luaT_ViewData::BLINK:
                    p.blink_status = _check(v, 0, 1);
                    break;
                case luaT_ViewData::REVERSE:
                    p.reverse_video = _check(v, 0, 1);
                    break;
                case luaT_ViewData::EXTTEXTCOLOR:
                    if (!p.use_ext_colors)
                        p.ext_bkg_color = tortilla::getPalette()->getColor(p.bkg_color);
                    p.use_ext_colors = 1;
                    p.ext_text_color = v;
                    break;
                case luaT_ViewData::EXTBKGCOLOR:
                    if (!p.use_ext_colors)
                        p.ext_text_color = tortilla::getPalette()->getColor(p.text_color);
                    p.use_ext_colors = 1;
                    p.ext_bkg_color = v;
                    break;
                default:
                    ok = false;
                    break;
                }
            }
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:set");
}

void reg_mt_trigger_string(lua_State *L)
{
    luaL_newmetatable(L, "viewstring");
    regFunction(L, "getBlocksCount", ts_getBlocksCount);
    regFunction(L, "getText", ts_getText);
    regFunction(L, "getParamsCount", ts_getParamsCount);
    regFunction(L, "getParameter", ts_getParameter);
    regFunction(L, "getComparedText", ts_getComparedText);
    regFunction(L, "isPrompt", ts_isPrompt);
    regFunction(L, "getPrompt", ts_getPrompt);
    regFunction(L, "isGameCmd", ts_isGameCmd);
    regFunction(L, "isSystem", ts_isSystem);
    regFunction(L, "drop", ts_drop);
    regFunction(L, "deleteBlock", ts_deleteBlock);
    regFunction(L, "deleteAllBlocks", ts_deleteAllBlocks);
    regFunction(L, "get", ts_get);
    regFunction(L, "set", ts_set);
    regIndexMt(L);
    lua_pop(L, 1);
}*/
