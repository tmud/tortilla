#include "stdafx.h"
#include "accessors.h"
#include "api/api.h"
#include "pluginsApi.h"
#include "pluginsView.h"
#include "pluginSupport.h"
#include "pluginsParseData.h"
#include "pluginsActiveObjects.h"
#include "pluginsViewString.h"

#include "../MainFrm.h"
extern CMainFrame _wndMain;
extern LogicProcessorMethods* lp();

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
    return pluginInvArgs(L, L"window:attach");
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
    return pluginInvArgs(L, L"window:hwnd");
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
    return pluginInvArgs(L, L"window:floathwnd");
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
    return pluginInvArgs(L, L"window:dock");
}

int window_undock(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.undockDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, L"window:undock");
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
    return pluginInvArgs(L, L"window:block");
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
    return pluginInvArgs(L, L"window:unblock");
}

int window_show(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.showDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, L"window:show");
}

int window_hide(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.hideDockPane(v);
        return 0;
    }
    return pluginInvArgs(L, L"window:hide");
}

int window_isVisible(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        bool state = _wndMain.m_gameview.isDockPaneVisible(v);        
        lua_pushboolean(L, state ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"window:isVisible");
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
    return pluginInvArgs(L, L"window:setRender");
}

int window_setFixedSize(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_WINDOW, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
        _wndMain.m_gameview.setFixedSize(*v, lua_tointeger(L, 2), lua_tointeger(L, 3));
        return 0;
    }
    return pluginInvArgs(L, L"window:setFixedSize");
}

int window_getSize(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_WINDOW))
    {
         PluginsView *v = (PluginsView *)luaT_toobject(L, 1);
         SIZE sz = _wndMain.m_gameview.getDockPaneSize(v);
         lua_pushinteger(L, sz.cy);
         lua_pushinteger(L, sz.cx);
         return 2;
    }
    return pluginInvArgs(L, L"window:getSize");
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
    regFunction(L, "setFixedSize", window_setFixedSize);
    regFunction(L, "getSize", window_getSize);
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
    return pluginInvArgs(L, L"viewdata:size");
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
    return pluginInvArgs(L, L"viewdata:select");
}

int vd_getIndex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushinteger(L, pdata->getindex());
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:getIndex");
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
    return pluginInvArgs(L, L"viewdata:isGameCmd");
}

int vd_isSystem(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->system) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isSystem");
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
    return pluginInvArgs(L, L"viewdata:isPrompt");
}

int vd_isDropped(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString* s = pdata->getselected();
        int state = (s && s->dropped) ? 1 : 0;
        lua_pushboolean(L, state);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isDropped");
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
    return pluginInvArgs(L, L"viewdata:getPrompt");
}

int vd_isFirst(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushboolean(L, (pdata->pdata->update_prev_string) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isFirst");
}

int vd_isLast (lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        lua_pushboolean(L, (pdata->pdata->last_finished) ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isLast");
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
    return pluginInvArgs(L, L"viewdata:getText");
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
    return pluginInvArgs(L, L"viewdata:getTextLen");
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
    return pluginInvArgs(L, L"viewdata:getHash");
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
    return pluginInvArgs(L, L"viewdata:blocks");
}

int vd_gettype(const tchar* type);
bool vsp_pushparam(lua_State *L, const MudViewStringParams &p, int type)
{
    bool ok = true;
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
   return ok;
}

tbyte _check(unsigned int val, unsigned int min, unsigned int max)
{
    if (val < min) val = min;
    else if (val > max) val = max;
    return (tbyte)val;
}
bool vsp_setparam(MudViewStringParams &p, int type,  unsigned int v)
{
    bool ok = true;
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
    return ok;
}

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
            type = vd_gettype(luaT_towstring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, L"viewdata:get");
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
                ok = vsp_pushparam(L, p, type);
            }
        }
        if (!ok)
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:get");
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
    return pluginInvArgs(L, L"viewdata:copyBlock");
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
            type = vd_gettype(luaT_towstring(L, 3));
            if (type == -1)
                return pluginInvArgs(L, L"viewdata:set");
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
                ok = vsp_setparam(p, type, v);
            }
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:set");
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
    return pluginInvArgs(L, L"viewdata:getBlockText");
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
    return pluginInvArgs(L, L"viewdata:setBlockText");
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
    return pluginInvArgs(L, L"viewdata:deleteBlock");
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
    return pluginInvArgs(L, L"viewdata:deleteAllBlocks");
}

int vd_createStringEx(lua_State *L, const tchar* f, int delta)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA) || luaT_check(L, 3, LUAT_VIEWDATA, LUA_TBOOLEAN, LUA_TBOOLEAN))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        bool ok = false;
        bool system = false; bool gamecmd = false;
        if (lua_gettop(L) == 3) {
            system = lua_toboolean(L, 2) != 0 ? true : false;
            gamecmd = lua_toboolean(L, 3) != 0 ? true : false;
        }
        if (pdata->getselected_pvs())
        {
            pdata->insert_new_string(gamecmd, system, delta);
            ok = true;
        }
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, f);
}

int vd_createString(lua_State *L)
{
    return vd_createStringEx(L, L"viewdata:createString", 1);
}

int vd_insertString(lua_State *L)
{
    return vd_createStringEx(L, L"viewdata:insertString", 0);
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
    return pluginInvArgs(L, L"viewdata:deleteString");
}

int vd_deleteAllStrings(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
         PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
         pdata->deleteall();
         return 0;
    }
    return pluginInvArgs(L, L"viewdata:deleteAllStrings");
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
        int to = pdata->size();
        if (!pdata->pdata->last_finished) to -= 1;
        for (int i = from; i <= to; ++i)
        {
            tstring text;
            pdata->pdata->strings[i-1]->getText(&text);
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
    return pluginInvArgs(L, L"viewdata:find");
}

int vd_getBlockPos(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        int abspos = lua_tointeger(L, 2);
        PluginViewString *str = pdata->getselected_pvs();
        int block = 0; int pos = 0;
        if (str && abspos > 0) 
        {
            abspos-=1;
            for (int i=0,e=str->blocks.size();i<e;++i)
            {
                int size = u8string_len(str->blocks[i]);
                if (size > abspos)
                {
                    block = i+1;
                    pos = abspos+1;
                    break;
                }
                abspos -= size;
            }
        }
        if (block > 0)
        {
            lua_pushinteger(L, block);
            lua_pushinteger(L, pos);
        }
        else
        {
            lua_pushnil(L);
            lua_pushnil(L);
        }
        return 2;
    }
    return pluginInvArgs(L, L"viewdata:getBlockPos");
}
//--------------------------------------------------------------------
std::map<tstring, int> vdtypes;
void init_vdtypes()
{
    vdtypes[L"textcolor"] = luaT_ViewData::TEXTCOLOR;
    vdtypes[L"bkgcolor"] = luaT_ViewData::BKGCOLOR;
    vdtypes[L"underline"] = luaT_ViewData::UNDERLINE;
    vdtypes[L"italic"] = luaT_ViewData::ITALIC;
    vdtypes[L"blink"] = luaT_ViewData::BLINK;
    vdtypes[L"reverse"] = luaT_ViewData::REVERSE;
    vdtypes[L"exttextcolor"] = luaT_ViewData::EXTTEXTCOLOR;
    vdtypes[L"extbkgcolor"] = luaT_ViewData::EXTBKGCOLOR;

}
int vd_gettype(const tchar* type)
{
    std::map<tstring, int>::iterator it = vdtypes.find(type);
    return (it == vdtypes.end()) ? -1 : it->second;
}

int vd_setNext(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TBOOLEAN))
    {
         PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
         MudViewString *str = pdata->getselected();
         if (str)
         {
            str->next = lua_toboolean(L, 2) ? true : false;
            lua_pushboolean(L, 1);
            return 1;
         }
         lua_pushboolean(L, 0);
         return 1;
    }
    return pluginInvArgs(L, L"viewdata:setNext");
}

int vd_setPrev(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TBOOLEAN))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString *str = pdata->getselected();
        if (str)
        {
            str->prev = lua_toboolean(L, 2) ? true : false;
            lua_pushboolean(L, 1);
            return 1;
        }
        lua_pushboolean(L, 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:setPrev");
}

int vd_isNext(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString *str = pdata->getselected();
        if (str)
        {
            lua_pushboolean(L, str->next ? 1 : 0);
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isNext");
}

int vd_isPrev(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString *str = pdata->getselected();
        if (str) 
        {
            lua_pushboolean(L, str->prev ? 1 : 0);
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isPrev");
}

int vd_parameters(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);        
        lua_pushinteger(L, pdata->get_params());
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:parameters");
}

int vd_getParameter(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        tstring param;
        if (pdata->get_parameter(index, &param))
            luaT_pushwstring(L, param.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:getParameter");
}

int vd_isChanged(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        PluginsParseData::StringChanged sc = pdata->is_changed();
        if (sc == PluginsParseData::ISC_UNKNOWN)
            lua_pushnil(L);
        else
            lua_pushboolean(L, sc==PluginsParseData::ISC_NOTCHANGED ? 0 : 1);
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:isChanged");
}

int vd_getKey(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        tstring k;
        if (!pdata->get_key(&k))
            lua_pushnil(L);
        else
            luaT_pushwstring(L, k.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"viewdata:getKey");
}

int vd_createRef(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString *s = pdata->getselected();
        if (s) 
        {
            PluginsViewString *p = new PluginsViewString();
            p->create(s);
            luaT_pushobject(L, p, LUAT_VIEWSTRING);
            return 1;
        } else {
            lua_pushnil(L);
            return 1;
        }
    }
    return pluginInvArgs(L, L"viewdata:createRef");
}

int vd_print(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWDATA, LUA_TNUMBER))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        MudViewString *s = pdata->getselected();
        int view = lua_tointeger(L, 2);
        if (s && view >= 0 && view < OUTPUT_WINDOWS)
        {
            lp()->pluginsOutput(view, s->blocks);
            return 0;
        }
    }
    return pluginInvArgs(L, L"viewdata:print");
}

int vd_toWatch(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWDATA))
    {
        PluginsParseData *pdata = (PluginsParseData *)luaT_toobject(L, 1);
        int parameters = pdata->get_params();
        int size = pdata->size();
        int selected = pdata->getindex();

        lua_newtable(L);
        for (int i=1; i<=size; ++i)
        {
            pdata->select(i);
            PluginViewString *s = pdata->getselected_pvs();
            u8string text;
            s->getText(&text);

            if (parameters > 0)
            {
                lua_newtable(L);
                lua_pushstring(L, "string");
                lua_pushstring(L, text.c_str());
                lua_settable(L, -3);
                tstring k;
                pdata->get_key(&k);
                lua_pushstring(L, "key");
                luaT_pushwstring(L, k.c_str());
                lua_settable(L, -3);
                for (int pi=0; pi<parameters; ++pi)
                {
                    tstring p;
                    pdata->get_parameter(pi, &p);
                    lua_pushinteger(L, pi);
                    luaT_pushwstring(L, p.c_str());
                    lua_settable(L, -3);
                }
            }
            else
            {
                lua_pushstring(L, text.c_str());
            }
            lua_pushinteger(L, i);
            lua_insert(L, -2);
            lua_settable(L, -3);
        }
        pdata->select(selected);
        return 1;
    }
    return 0;
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
    regFunction(L, "isSystem", vd_isSystem);
    regFunction(L, "isPrompt", vd_isPrompt);
    regFunction(L, "isDropped", vd_isDropped);
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
    regFunction(L, "deleteAllStrings", vd_deleteAllStrings);
    regFunction(L, "createString", vd_createString);
    regFunction(L, "insertString", vd_insertString);
    regFunction(L, "find", vd_find);
    regFunction(L, "getBlockPos", vd_getBlockPos);
    regFunction(L, "setNext", vd_setNext);
    regFunction(L, "setPrev", vd_setPrev);
    regFunction(L, "isNext", vd_isNext);
    regFunction(L, "isPrev", vd_isPrev);
    regFunction(L, "parameters", vd_parameters);
    regFunction(L, "getParameter", vd_getParameter);
    regFunction(L, "isChanged", vd_isChanged);
    regFunction(L, "getKey", vd_getKey);
    regFunction(L, "createRef", vd_createRef);
    regFunction(L, "print", vd_print);
    regFunction(L, "__towatch", vd_toWatch);
    regIndexMt(L);
    lua_pop(L, 1);
}
//--------------------------------------------------------------------
int vs_getText(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        tstring text;
        s->getText(&text);
        luaT_pushwstring(L, text.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getText");
}

int vs_getTextLen(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        lua_pushinteger(L, s->getTextLen());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getTextLen");
}

int vs_blocks(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        lua_pushinteger(L, s->count());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:blocks");
}

int vs_setBlockText(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_VIEWSTRING, LUA_TNUMBER, LUA_TSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        tstring text(luaT_towstring(L, 3));
        int result = (s->setBlockText(index-1, text)) ? 1 : 0;
        lua_pushboolean(L, result);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:setBlockText");
}

int vs_getBlockText(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWSTRING, LUA_TNUMBER))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        tstring text;
        if (s->getBlockText(index-1, &text))
            luaT_pushwstring(L, text.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getBlockText");
}

int vs_deleteBlock(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING, LUA_TNUMBER))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        int result = s->deleteBlock(index-1) ? 1 : 0;
        lua_pushboolean(L, result);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:deleteBlock");
}

int vs_set(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_VIEWSTRING, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER) || 
        luaT_check(L, 4, LUAT_VIEWSTRING, LUA_TNUMBER, LUA_TSTRING, LUA_TNUMBER))
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
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        if (index >= 1 && index <= s->count())
        {
            unsigned int v = lua_tounsigned(L, 4);
            MudViewStringBlock &b = s->get(index-1);
            ok = vsp_setparam(b.params, type, v);
        }
        if (!ok)
            lua_pushnil(L);
        lua_pushboolean(L, ok ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:set");
}

int vs_get(lua_State *L)
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
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        int index = lua_tointeger(L, 2);
        if (index >= 1 && index <= s->count())
        {
            const MudViewStringBlock &b = s->get(index-1);
            ok = vsp_pushparam(L, b.params, type);
        }
        if (!ok)
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:get");
}

int vs_print(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWSTRING, LUA_TNUMBER))
    {
        int window = lua_tointeger(L, 2);
        if (window >= 0 && window <= OUTPUT_WINDOWS)
        {
            PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
            lp()->pluginsOutput(window, s->getBlocks());
            return 0;
        }
    }
    return pluginInvArgs(L, L"viewstring:print");
}

int vs_getData(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        tstring text;
        s->serialize(&text);
        luaT_pushwstring(L, text.c_str());
        return 1;
    }
    return pluginInvArgs(L, L"viewstring:getData");
}

int vs_setData(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_VIEWSTRING, LUA_TSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        tstring text(luaT_towstring(L, 2));
        s->deserialize(text);
        return 0;
    }
    return pluginInvArgs(L, L"viewstring:setData");
}

int vs_toWatch(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        char buffer[32];
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        lua_newtable(L);
        for (int i=0,e=s->count();i<e;++i)
        {
            const MudViewStringBlock &b = s->get(i);
            const MudViewStringParams& p = b.params;
            lua_newtable(L);
            lua_pushstring(L, "string");
            luaT_pushwstring(L, b.string.c_str());
            lua_settable(L, -3);
            lua_pushstring(L, "color");
            if (p.use_ext_colors) {
                COLORREF c = p.ext_text_color;
                sprintf(buffer, "%02x%02x%02x", GetRValue(c), GetGValue(c), GetBValue(c));
            }
            else {
                sprintf(buffer, "%d", p.text_color);
            }
            lua_pushstring(L, buffer);
            lua_settable(L, -3);
            lua_pushstring(L, "background");
            if (p.use_ext_colors) {
                COLORREF c = p.ext_bkg_color;
                sprintf(buffer, "%02x%02x%02x", GetRValue(c), GetGValue(c), GetBValue(c));
            }
            else {
                sprintf(buffer, "%d", p.bkg_color);
            }
            lua_pushstring(L, buffer);
            lua_settable(L, -3);

            lua_pushstring(L, "attr");
            std::string a;
            if (p.italic_status) a.append("i");
            if (p.underline_status) a.append("u");
            if (!p.use_ext_colors && p.intensive_status) a.append("^");
            if (p.blink_status) a.append("f");
            if (p.reverse_video) a.append("r");
            lua_pushstring(L, a.c_str());
            lua_settable(L, -3);

            lua_pushinteger(L, i+1);
            lua_insert(L, -2);
            lua_settable(L, -3);
        }
        return 1;
    }
    return 0;
}

int vs_gc(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_VIEWSTRING))
    {
        PluginsViewString *s = (PluginsViewString *)luaT_toobject(L, 1);
        delete s;
    }
    return 0;
}

int vs_create(lua_State *L)
{
    if (luaT_check(L, 0))
    {
        MudViewString *vs = new MudViewString();
        luaT_pushobject(L, vs, LUAT_VIEWSTRING);
        return 1;
    }
    return pluginInvArgs(L, L"createViewString");
}

void reg_mt_viewstring(lua_State *L)
{
    lua_register(L, "createViewString", vs_create);
    luaL_newmetatable(L, "viewstring");
    regFunction(L, "getText", vs_getText);
    regFunction(L, "getTextLen", vs_getTextLen);
    regFunction(L, "blocks", vs_blocks);
    regFunction(L, "setBlockText", vs_setBlockText);
    regFunction(L, "getBlockText", vs_getBlockText);
    regFunction(L, "deleteBlock", vs_deleteBlock);
    regFunction(L, "set", vs_set);
    regFunction(L, "get", vs_get);
    regFunction(L, "print", vs_print);
    regFunction(L, "getData", vs_getData);
    regFunction(L, "setData", vs_setData);
    regFunction(L, "__towatch", vs_toWatch);
    regFunction(L, "__gc", vs_gc);
    regIndexMt(L);
    lua_pop(L, 1);
}
//--------------------------------------------------------------------
int ao_inv_args(lua_State *L, const tchar* fname)
{
    if (!luaT_isobject(L, LUAT_ACTIVEOBJS, 1))
        return pluginInvArgs(L, fname);
    ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
    tstring error(fname);
    error.append(L":");
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
    return ao_inv_args(L, L"activeobjects:select");
}

int ao_size(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->size());
        return 1;
    }
    return ao_inv_args(L, L"activeobjects:size");
}

int ao_checktype(const tchar* type)
{
    if (!wcscmp(type, L"key"))
        return luaT_ActiveObjects::KEY;
    if (!wcscmp(type, L"value")) 
        return luaT_ActiveObjects::VALUE;
    if (!wcscmp(type, L"group"))
        return luaT_ActiveObjects::GROUP;
    return -1;
}

int ao_get(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TNUMBER))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        tstring value;
        if (ao->get(lua_tointeger(L, 2), &value))
            luaT_pushwstring(L, value.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    else if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TSTRING))
    {
        int type = ao_checktype(luaT_towstring(L, 2));
        if (type != -1)
        {
            ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
            tstring value;
            if (ao->get(type, &value))
                luaT_pushwstring(L, value.c_str());
            else
                lua_pushnil(L);
            return 1;
        }
    }
    return ao_inv_args(L, L"activeobjects:get");
}

int ao_set(lua_State *L)
{
    if (luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TNUMBER, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->set(lua_tointeger(L, 2), luaT_towstring(L, 3));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    else if (luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING))
    {
        int type = ao_checktype(luaT_towstring(L, 2));
        if (type != -1)
        {
            ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
            bool result = ao->set(type, luaT_towstring(L, 3));
            lua_pushboolean(L, result ? 1 : 0);
            return 1;
        }
    }
    return ao_inv_args(L, L"activeobjects:set");
}

int ao_add(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(luaT_towstring(L, 2), luaT_towstring(L, 3), luaT_towstring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TNIL, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(luaT_towstring(L, 2), L"", luaT_towstring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TNIL) ||
        luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(luaT_towstring(L, 2), luaT_towstring(L, 3), L"");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->add(luaT_towstring(L, 2), L"", L"");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, L"activeobjects:add");
}

int ao_replace(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->replace(luaT_towstring(L, 2), luaT_towstring(L, 3), luaT_towstring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TNIL, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->replace(luaT_towstring(L, 2), L"", luaT_towstring(L, 4));
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 4, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING, LUA_TNIL) ||
        luaT_check(L, 3, LUAT_ACTIVEOBJS, LUA_TSTRING, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->replace(luaT_towstring(L, 2), luaT_towstring(L, 3), L"");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    if (luaT_check(L, 2, LUAT_ACTIVEOBJS, LUA_TSTRING))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        bool result = ao->replace(luaT_towstring(L, 2), L"", L"");
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return ao_inv_args(L, L"activeobjects:replace");
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
    return ao_inv_args(L, L"activeobjects:delete");
}

int ao_getIndex(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        lua_pushinteger(L, ao->getindex());
        return 1;
    }
    return ao_inv_args(L, L"activeobjects:getIndex");
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
    return ao_inv_args(L, L"activeobjects:setIndex");
}

int ao_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_ACTIVEOBJS))
    {
        ActiveObjects *ao = (ActiveObjects *)luaT_toobject(L, 1);
        ao->update();
        return 0;
    }
    return ao_inv_args(L, L"activeobjects:update");
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
    regFunction(L, "replace", ao_replace);
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
    bool canset(const tchar* var) 
    {
        VarProcessor *vp = tortilla::getVars();
        return vp->canSetVar(tstring(var));
    }
} _vars_filter;

void reg_activeobjects(lua_State *L)
{
    reg_mt_activeobject(L);
    PropertiesData *p = tortilla::getProperties();
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
