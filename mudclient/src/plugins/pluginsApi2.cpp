#include "stdafx.h"
#include "api/api.h"
#include "pluginsApi.h"
#include "pluginsView.h"
#include "pluginSupport.h"
#include "pluginsParseData.h"
#include "pluginsActiveObjects.h"

#include "../MainFrm.h"
extern CMainFrame _wndMain;
extern Palette256* _palette;
extern PropertiesData* _pdata;
extern LogicProcessorMethods* _lp;

void regFunction(lua_State *L, const char* name, lua_CFunction f)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, f);
    lua_settable(L, -3);
}

void regIndexMt(lua_State *L)
{
    assert(lua_istable(L, -1));
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "access denied");
    lua_settable(L, -3);
}
//--------------------------------------------------------------------
int window_attach(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TNUMBER))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        HWND child = (HWND)lua_tointeger(L, 2);
        v->attachChild(child);
        return 0;
    }
    return pluginInvArgs(L, "window:attach");
}

int window_hwnd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        HWND wnd = *v;
        lua_pushunsigned(L, (unsigned int)wnd);
        return 1;
    }
    return pluginInvArgs(L, "window:hwnd");
}

int window_floathwnd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        HWND wnd = _wndMain.m_gameview.getFloatingWnd(v);
        lua_pushunsigned(L, (unsigned int)wnd);
        return 1;
    }
    return pluginInvArgs(L, "window:floathwnd");
}

bool window_side(const wchar_t* side, bool checkfloat, std::vector<int> *sv)
{
    Tokenizer tk(side, L",; ");
    if (tk.empty())
        return false;
    for (int i = 0, e = tk.size(); i < e; ++i)
    {
        int dock_side = _wndMain.m_gameview.convertSideFromString(tk[i]);
        if (!checkfloat && dock_side == DOCK_FLOAT)
            dock_side = -1;
        if (dock_side == -1)
        {
            sv->clear();
            return false;
        }
        sv->push_back(dock_side);
    }
    return true;
}

int window_dock(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        std::vector<int> sv;
        if (window_side(luaT_towstring(L, 2), false, &sv))
        {
            for (int i = 0, e = sv.size(); i < e; ++i)
                _wndMain.m_gameview.dockDockPane(v, sv[i]);
            return 0;
        }
    }
    return pluginInvArgs(L, "window:dock");
}

int window_undock(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.undockDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window:undock");
}

int window_block(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        std::vector<int> sv;
        if (window_side(luaT_towstring(L, 2), true, &sv))
        {
            for (int i = 0, e = sv.size(); i < e; ++i)
                _wndMain.m_gameview.blockDockPane(v, sv[i]);
            return 0;
        }
    }
    return pluginInvArgs(L, "window:block");
}

int window_unblock(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        std::vector<int> sv;
        if (window_side(luaT_towstring(L, 2), true, &sv))
        {
            for (int i = 0, e = sv.size(); i < e; ++i)
                _wndMain.m_gameview.unblockDockPane(v, sv[i]);
            return 0;
        }
    }
    return pluginInvArgs(L, "window:unblock");
}

int window_show(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.showDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window:show");
}

int window_hide(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.hideDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window:hide");
}

int window_isVisible(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int state = v->IsWindowVisible() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "window:isVisible");
}

int window_setRender(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TFUNCTION))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        if (v->isChildAttached())
            lua_pushnil(L);
        else
        {
            PluginsViewRender* r = v->setRender(L);
            luaT_pushobject(L, r, LUAT_RENDER);
        }
        return 1;
    }
    return pluginInvArgs(L, "window:setRender");
}
//--------------------------------------------------------------------
void reg_mt_window(lua_State *L)
{
    luaL_newmetatable(L, "window");
    regFunction(L, "attach", window_attach);
    regFunction(L, "hwnd", window_hwnd);
    regFunction(L, "floathwnd", window_floathwnd);
    regFunction(L, "dock", window_dock);
    regFunction(L, "undock", window_undock);
    regFunction(L, "block", window_block);
    regFunction(L, "unblock", window_unblock);
    regFunction(L, "show", window_show);
    regFunction(L, "hide", window_hide);
    regFunction(L, "isVisible", window_isVisible);
    regFunction(L, "setRender", window_setRender);
    regIndexMt(L);
    lua_pop(L, 1);
}
//--------------------------------------------------------------------
// viewdata
int vd_size(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushinteger(L, pdata->size());
        return 1;
    }
    return pluginInvArgs(L, "viewdata:size");
}

int vd_select(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        int result = pdata->select(lua_tointeger(L, 2)) ? 1 : 0;
        lua_pushboolean(L, result);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:select");
}

int vd_getIndex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushinteger(L, pdata->getindex());
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getIndex");
}

int vd_isGameCmd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->gamecmd) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:isGameCmd");
}

int vd_isPrompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->prompt > 0) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:isPrompt");
}

int vd_getPrompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        u8string prompt;
        pdata->getPrompt(&prompt);
        lua_pushstring(L, prompt.c_str());
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getPrompt");
}

int vd_isFirst(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushboolean(L, (pdata->pdata->update_prev_string) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:isFirst");
}

int vd_isLast (lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushboolean(L, (pdata->pdata->last_finished) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:isLast");
}

int vd_getText(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        if (str)
        {
            u8string text;
            str->getText(&text);
            lua_pushstring(L, text.c_str());
        }
        else
        {
            lua_pushnil(L);
        }
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getText");
}

int vd_getTextLen(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        int len = (str) ? str->getTextLen() : 0;
        lua_pushinteger(L, len);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getTextLen");
}

int vd_getHash(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        if (str)
        {
            u8string data;
            utf8 buffer[32];
            MudViewString *ms = pdata->getselected();
            for (int i = 0, e = ms->blocks.size(); i < e; ++i)
            {
                data.append(str->blocks[i]);
                MudViewStringParams &p = ms->blocks[i].params;
                sprintf(buffer, "$%d%d%d%d%d", p.underline_status, p.blink_status, p.italic_status, p.reverse_video, p.use_ext_colors);
                data.append(buffer);
                if (!p.use_ext_colors)
                {
                    int txt = p.text_color; if (txt <= 7 && p.intensive_status) txt += 8;
                    sprintf(buffer, "%d%d", txt, p.bkg_color);
                }
                else
                {
                    sprintf(buffer, "%x%x", p.ext_text_color, p.ext_bkg_color);
                }
                data.append(buffer);
            }
            lua_pushstring(L, data.c_str());
        }
        else
        {
            lua_pushnil(L);
        }
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getHash");
}

int vd_blocks(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        lua_pushinteger(L, str ? str->blocks.size() : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:blocks");
}

tbyte _check(unsigned int val, unsigned int min, unsigned int max)
{
    if (val < min) val = min;
    else if (val > max) val = max;
    return (tbyte)val;
}

int vd_gettype(const utf8* type);
int vd_get(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TNUMBER)||
        luaT_check(L, 3, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TSTRING))
    {
        int type = 0;
        if (lua_isnumber(L, 3))
            type = lua_tointeger(L, 3);
        else
        {
            type = vd_gettype(lua_tostring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, "viewdata:get");
        }

        bool ok = false;
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* str = pdata->getselected();        
        if (str)
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
    return pluginInvArgs(L, "viewdata:get");
}

int vd_copyBlock(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        bool ok = pdata->copy_block(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));        
        lua_pushboolean(L, (ok) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:copyBlock");
}

int vd_set(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER) || 
        luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TSTRING, LUA_TNUMBER))
    {
        int type = 0;
        if (lua_isnumber(L, 3))
            type = lua_tointeger(L, 3);
        else
        {
            type = vd_gettype(lua_tostring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, "viewdata:set");
        }

        bool ok = false;
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* str = pdata->getselected();
        if (str)
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
                        p.ext_bkg_color = _palette->getColor(p.bkg_color);
                    p.use_ext_colors = 1;
                    p.ext_text_color = v;
                    break;
                case luaT_ViewData::EXTBKGCOLOR:
                    if (!p.use_ext_colors)
                        p.ext_text_color = _palette->getColor(p.text_color);
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
    return pluginInvArgs(L, "viewdata:set");
}

int vd_getBlockText(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        u8string str;
        if (pdata->getselected_block(lua_tointeger(L, 2), &str))
            lua_pushstring(L, str.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:getBlockText");
}

int vd_setBlockText(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TSTRING))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString *str = pdata->getselected_pvs();
        bool ok = false;
        if (str)
        {
            int size = str->blocks.size();
            int index = lua_tointeger(L, 2);
            if (index >= 1 && index <= size)
            {
                str->blocks[index-1].assign(lua_tostring(L, 3));
                ok = true;
            }
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:setBlockText");
}

int vd_deleteBlock(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString *str = pdata->getselected_pvs();
        bool ok = false;
        if (str)
        {
            int size = str->blocks.size();
            int index = lua_tointeger(L, 2);
            if (index >= 1 && index <= size)
            {
                index = index - 1;
                str->blocks.erase(str->blocks.begin() + index);
                MudViewString *vs = pdata->getselected();
                vs->blocks.erase(vs->blocks.begin() + index);
                ok = true;
            }
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:deleteBlock");
}

int vd_deleteAllBlocks(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString *str = pdata->getselected_pvs();
        bool ok = false;
        if (str)
        {
            str->blocks.clear();
            MudViewString *vs = pdata->getselected();
            vs->blocks.clear();
            if (pdata->selected == 0)
                pdata->pdata->update_prev_string = false;
            ok = true;
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:deleteAllBlocks");
}

int vd_createString(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        bool ok = false;
        if (pdata->getselected_pvs())
        {
            pdata->insert_new_string();
            ok = true;
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:createString");
}

int vd_deleteString(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        bool ok = false;
        if (pdata->getselected())
        {
            pdata->delete_selected();
            ok = true;
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:deleteString");
}

int vd_find(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUAT_PCRE) || 
        luaT_check(L, 3, LUAT_VIEWDATA, LUAT_PCRE, LUA_TNUMBER))
    {
        bool result = false;
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        Pcre *p = (Pcre *)luaT_toobject(L, 2);
        int from = (lua_gettop(L) == 3) ? lua_tointeger(L, 3) : 1;
        int to = pdata->plugins_strings.size();
        if (!pdata->pdata->last_finished) to -= 1;
        for (int i = from; i <= to; ++i)
        {
            u8string text;
            pdata->plugins_strings[i-1]->getText(&text);
            result = p->find(text.c_str());
            if (result)
            {
                pdata->select(i);
                break;
            }
        }
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata:find");
}
//--------------------------------------------------------------------
std::map<u8string, int> vdtypes;
void init_vdtypes()
{
    vdtypes["textcolor"] = luaT_ViewData::TEXTCOLOR;
    vdtypes["bkgcolor"] = luaT_ViewData::BKGCOLOR;
    vdtypes["underline"] = luaT_ViewData::UNDERLINE;
    vdtypes["italic"] = luaT_ViewData::ITALIC;
    vdtypes["blink"] = luaT_ViewData::BLINK;
    vdtypes["reverse"] = luaT_ViewData::REVERSE;
    vdtypes["exttextcolor"] = luaT_ViewData::EXTTEXTCOLOR;
    vdtypes["extbkgcolor"] = luaT_ViewData::EXTBKGCOLOR;

}
int vd_gettype(const utf8* type)
{
    std::map<u8string, int>::iterator it = vdtypes.find(type);
    return (it == vdtypes.end()) ? -1 : it->second;
}

void reg_mt_viewdata(lua_State *L)
{
    init_vdtypes();
    luaL_newmetatable(L, "viewdata");
    regFunction(L, "size", vd_size);
    regFunction(L, "select", vd_select);
    regFunction(L, "getIndex", vd_getIndex);
    regFunction(L, "isFirst", vd_isFirst);
    regFunction(L, "isLast", vd_isLast);
    regFunction(L, "isGameCmd", vd_isGameCmd);
    regFunction(L, "isPrompt", vd_isPrompt);
    regFunction(L, "getPrompt", vd_getPrompt);
    regFunction(L, "getText", vd_getText);
    regFunction(L, "getTextLen", vd_getTextLen);
    regFunction(L, "getHash", vd_getHash);
    regFunction(L, "blocks", vd_blocks);
    regFunction(L, "get", vd_get);
    regFunction(L, "set", vd_set);
    regFunction(L, "setBlockText", vd_setBlockText);
    regFunction(L, "getBlockText", vd_getBlockText);
    regFunction(L, "deleteBlock", vd_deleteBlock);
    regFunction(L, "deleteAllBlocks", vd_deleteAllBlocks);
    regFunction(L, "copyBlock", vd_copyBlock);
    regFunction(L, "deleteString", vd_deleteString);
    regFunction(L, "createString", vd_createString);
    regFunction(L, "find", vd_find);
    regIndexMt(L);
    lua_pop(L, 1);
}
//--------------------------------------------------------------------
int ao_inv_args(lua_State *L, const utf8* fname)
{
    if (!luaT_isobject(L, LUAT_ACTIVEOBJS, 1))
        return pluginInvArgs(L, fname);
    ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
    u8string error(fname);
    error.append(":");
    error.append(ao->type());
    return pluginInvArgs(L, error.c_str());
}

int ao_select(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TNUMBER))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->select(lua_tointeger(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, "activeobjects:select");
}

int ao_size(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->size());
        return 1;
    }
    return ao_inv_args(L, "activeobjects:size");
}

int ao_checktype(const utf8* type)
{
    if (!strcmp(type, "key"))
        return luaT_ActiveObjects::KEY;
    if (!strcmp(type, "value")) 
        return luaT_ActiveObjects::VALUE;
    if (!strcmp(type, "group"))
        return luaT_ActiveObjects::GROUP;
    return -1;
}

int ao_get(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TNUMBER))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        u8string value;
        if (ao->get(lua_tointeger(L, 2), &value))
            lua_pushstring(L, value.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    else if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TSTRING))
    {
        int type = ao_checktype(lua_tostring(L, 2));
        if (type != -1)
        {
            ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
            u8string value;
            if (ao->get(type, &value))
                lua_pushstring(L, value.c_str());
            else
                lua_pushnil(L);
            return 1;
        }
    }
    return ao_inv_args(L, "activeobjects:get");
}

int ao_set(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TNUMBER, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->set(lua_tointeger(L, 2), lua_tostring(L, 3));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    else if (luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING))
    {
        int type = ao_checktype(lua_tostring(L, 2));
        if (type != -1)
        {
            ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
            bool result = ao->set(type, lua_tostring(L, 3));
            lua_pushboolean(L, result ? 1 : 0);
            return 1;
        }
    }
    return ao_inv_args(L, "activeobjects:set");
}

int ao_add(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(lua_tostring(L, 2), lua_tostring(L, 3), lua_tostring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TNIL, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(lua_tostring(L, 2), "", lua_tostring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TNIL) ||
        luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(lua_tostring(L, 2), lua_tostring(L, 3), "");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(lua_tostring(L, 2), "", "");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, "activeobjects:add");
}

int ao_delete(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->del();
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, "activeobjects:delete");
}

int ao_getIndex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->getindex());
        return 1;
    }
    return ao_inv_args(L, "activeobjects:getIndex");
}

int ao_setIndex(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TNUMBER))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->setindex(lua_tointeger(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, "activeobjects:setIndex");
}

int ao_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        ao->update();
        return 0;
    }
    return ao_inv_args(L, "activeobjects:update");
}

int ao_gc(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        delete ao;
    }
    return 0;
}

void reg_mt_activeobject(lua_State *L)
{
    luaL_newmetatable(L, "activeobjects");
    regFunction(L, "select", ao_select);
    regFunction(L, "set", ao_set);
    regFunction(L, "get", ao_get);
    regFunction(L, "size", ao_size);
    regFunction(L, "add", ao_add);
    regFunction(L, "delete", ao_delete);
    regFunction(L, "getindex", ao_getIndex);
    regFunction(L, "setindex", ao_setIndex);
    regFunction(L, "update", ao_update);
    regFunction(L, "__gc", ao_gc);
    regIndexMt(L);
    lua_pop(L, 1);
}

void reg_activeobject(lua_State *L, const utf8* type, void *object)
{
    luaT_pushobject(L, object, LUAT_ACTIVEOBJS);
    lua_setglobal(L, type);
}

class VarsFilter : public ActiveObjectsFilter
{
public:
    bool canset(const u8string& var) 
    {
        TU2W v(var.c_str());
        return _lp->canSetVar(tstring(v));
    }
} _vars_filter;

void reg_activeobjects(lua_State *L)
{
    reg_mt_activeobject(L);
    PropertiesData *p = _pdata;
    reg_activeobject(L, "aliases", new AO_Aliases(p));
    reg_activeobject(L, "actions", new AO_Actions(p));
    reg_activeobject(L, "subs", new AO_Subs(p));
    reg_activeobject(L, "antisubs", new AO_Antisubs(p));
    reg_activeobject(L, "highlights", new AO_Highlihts(p));
    reg_activeobject(L, "hotkeys", new AO_Hotkeys(p));
    reg_activeobject(L, "gags", new AO_Gags(p));
    reg_activeobject(L, "groups", new AO_Groups(p));
    reg_activeobject(L, "tabs", new AO_Tabs(p));
    reg_activeobject(L, "timers", new AO_Timers(p));
    reg_activeobject(L, "vars", new AO_Vars(p, &_vars_filter));
}
