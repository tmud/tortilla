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
    return pluginInvArgs(L, "window.attach");
}

int window_hwnd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        HWND wnd = *v;
        lua_pushinteger(L, (int)wnd);
        return 1;
    }
    return pluginInvArgs(L, "window.hwnd");
}

int window_side(const wchar_t* side, bool checkfloat)
{
    int dock_side = _wndMain.m_gameview.convertSideFromString(side);
    if (!checkfloat && dock_side == DOCK_FLOAT)
        dock_side = -1;
    return dock_side;
}

int window_dock(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int dock_side = window_side(luaT_towstring(L, 2), false);
        if (dock_side >= 0) 
        {
            _wndMain.m_gameview.dockDockPane(v, dock_side);
            return 0;
        }
        pluginError(L, "window.dock", "Некорректный параметр side");
        return 0;
    }

    return pluginInvArgs(L, "window.dock");
}

int window_undock(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.undockDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window.undock");
}

int window_block(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int dock_side = window_side(luaT_towstring(L, 2), true);
        if (dock_side >= 0)
        {
            _wndMain.m_gameview.blockDockPane(v, dock_side);
            return 0;
        }
        pluginError(L, "window.block", "Некорректный параметр side");
        return 0;
    }
    return pluginInvArgs(L, "window.block");
}

int window_unblock(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TSTRING))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int dock_side = window_side(luaT_towstring(L, 2), true);
        if (dock_side >= 0)
        {
            _wndMain.m_gameview.unblockDockPane(v, dock_side);
            return 0;
        }
        pluginError(L, "window.unblock", "Некорректный параметр side");
        return 0;
    }
    return pluginInvArgs(L, "window.unblock");
}

int window_show(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.showDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window.show");
}

int window_hide(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.hideDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, "window.hide");
}

int window_isvisible(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int state = v->IsWindowVisible() ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "window.isvisible");
}

int window_setrender(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TFUNCTION))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        int id = reg_pview_render(L);
        v->setRender(L, id);
        return 0;
    }
    return pluginInvArgs(L, "window.setrender");
}

int window_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        v->update();
        return 0;
    }
    return pluginInvArgs(L, "window.update");
}

int window_setbackground(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_WINDOW, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsView *r = (PluginsView *)luaT_toobject(L, 1);
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        r->setBackground(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_WINDOW, LUA_TNUMBER))
    {
        PluginsView *r = (PluginsView *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        r->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, "window.setbackground");
}
//--------------------------------------------------------------------
void reg_mt_window(lua_State *L)
{
    luaL_newmetatable(L, "window");
    regFunction(L, "attach", window_attach);
    regFunction(L, "hwnd", window_hwnd);
    regFunction(L, "dock", window_dock);
    regFunction(L, "undock", window_undock);
    regFunction(L, "block", window_block);
    regFunction(L, "unblock", window_unblock);
    regFunction(L, "show", window_show);
    regFunction(L, "hide", window_hide);
    regFunction(L, "isvisible", window_isvisible);
    regFunction(L, "setrender", window_setrender);
    regFunction(L, "update", window_update);
    regFunction(L, "setbackground", window_setbackground);
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
    return pluginInvArgs(L, "viewdata.size");
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
    return pluginInvArgs(L, "viewdata.select");
}

int vd_getindex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushinteger(L, pdata->getindex());
        return 1;
    }
    return pluginInvArgs(L, "viewdata.getindex");
}

int vd_isgamecmd(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->gamecmd) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "viewdata.isgamecmd");
}

int vd_isprompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->prompt > 0) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, "viewdata.isprompt");
}

int vd_getprompt(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        u8string prompt;
        pdata->getPrompt(&prompt);        
        lua_pushstring(L, prompt.c_str());
        return 1;
    }
    return pluginInvArgs(L, "viewdata.getprompt");
}

int vd_isfirst(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushboolean(L, (pdata->pdata->update_prev_string) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata.isfirst");
}

int vd_gettext(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.gettext");
}

int vd_gettextlen(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        int len = (str) ? str->getTextLen() : 0;
        lua_pushinteger(L, len);
        return 1;
    }
    return pluginInvArgs(L, "viewdata.gettextlen");
}

int vd_gethash(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginViewString* str = pdata->getselected_pvs();
        if (str)
        {
            u8string data;
            utf8 buffer[16];
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
    return pluginInvArgs(L, "viewdata.gethash");
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
    return pluginInvArgs(L, "viewdata.blocks");
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
                return pluginInvArgs(L, "viewdata.get");
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
    return pluginInvArgs(L, "viewdata.get");
}

int vd_copyblock(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_VIEWDATA, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        bool ok = pdata->copy_block(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));        
        lua_pushboolean(L, (ok) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, "viewdata.copyblock");
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
                return pluginInvArgs(L, "viewdata.set");
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
    return pluginInvArgs(L, "viewdata.set");
}

int vd_getblocktext(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.getblocktext");
}

int vd_setblocktext(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.setblocktext");
}

int vd_deleteblock(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.deleteblock");
}

int vd_deleteallblocks(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.deleteblock");
}

int vd_createstring(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.createstring");
}

int vd_deletestring(lua_State *L)
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
    return pluginInvArgs(L, "viewdata.deletestring");
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
    regFunction(L, "getindex", vd_getindex);
    regFunction(L, "isfirst", vd_isfirst);
    regFunction(L, "isgamecmd", vd_isgamecmd);
    regFunction(L, "isprompt", vd_isprompt);
    regFunction(L, "getprompt", vd_getprompt);
    regFunction(L, "gettext", vd_gettext);
    regFunction(L, "gettextlen", vd_gettextlen);
    regFunction(L, "gethash", vd_gethash);
    regFunction(L, "blocks", vd_blocks);
    regFunction(L, "get", vd_get);
    regFunction(L, "set", vd_set);
    regFunction(L, "setblocktext", vd_setblocktext);
    regFunction(L, "getblocktext", vd_getblocktext);
    regFunction(L, "deleteblock", vd_deleteblock);
    regFunction(L, "deleteallblocks", vd_deleteallblocks);
    regFunction(L, "copyblock", vd_copyblock);
    regFunction(L, "deletestring", vd_deletestring);
    regFunction(L, "createstring", vd_createstring);
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
    return ao_inv_args(L, "activeobjects.select");
}

int ao_size(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->size());
        return 1;
    }
    return ao_inv_args(L, "activeobjects.size");
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
    return ao_inv_args(L, "activeobjects.get");
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
    return ao_inv_args(L, "activeobjects.set");
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
    return ao_inv_args(L, "activeobjects.add");
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
    return ao_inv_args(L, "activeobjects.delete");
}

int ao_getindex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->getindex());
        return 1;
    }
    return ao_inv_args(L, "activeobjects.getindex");
}

int ao_setindex(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TNUMBER))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->setindex(lua_tointeger(L, 2));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, "activeobjects.setindex");
}

int ao_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        ao->update();
        return 0;
    }
    return ao_inv_args(L, "activeobjects.setindex");
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
    regFunction(L, "getindex", ao_getindex);
    regFunction(L, "setindex", ao_setindex);
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
    reg_activeobject(L, "vars", new AO_Vars(p));    
}

