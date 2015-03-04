#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsViewRender.h"

extern CFont* _stdfont;

PluginsViewRender::PluginsViewRender(lua_State *pL, int index, HWND wnd) : renderL(pL), m_render_func_index(index), m_wnd(wnd),
m_inside_render(false), m_bkg_color(0), m_text_color(RGB(128,128,128)), m_width(0), m_height(0),
current_pen(NULL), current_brush(NULL), current_font(NULL)
{
    assert(pL && index > 0);
}

void PluginsViewRender::render()
{
    lua_State *L = renderL;

    CPaintDC dc(m_wnd);
    m_dc = dc;

    RECT rc; m_wnd.GetClientRect(&rc);
    m_width = rc.right; m_height = rc.bottom;
    m_dc.FillSolidRect(&rc, m_bkg_color);

    lua_getglobal(L, "pvrender");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return;
    }
    lua_pushinteger(L, m_render_func_index);
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return;
    }
    lua_insert(L, -2);
    lua_pop(L, 1);
    luaT_pushobject(L, this, LUAT_RENDER);
    m_inside_render = true;
    lua_pcall(L, 1, 0, -2);
    m_inside_render = false;
}

void PluginsViewRender::setBackground(COLORREF color)
{
    m_bkg_color = color;
    m_wnd.Invalidate();
}

void PluginsViewRender::setTextColor(COLORREF color)
{
    m_text_color = color;
}

int PluginsViewRender::width()
{
    if (m_inside_render)
        return m_width;
    RECT rc; m_wnd.GetClientRect(&rc);
    return rc.right;
}

int PluginsViewRender::height()
{
    if (m_inside_render)
        return m_height;
    RECT rc; m_wnd.GetClientRect(&rc);
    return rc.bottom;
}

CPen* PluginsViewRender::createPen(lua_State *L)
{
    assert(L == renderL);
    return pens.create(L);
}

CBrush* PluginsViewRender::createBrush(lua_State *L)
{
    assert(L == renderL);
    return brushes.create(L);
}

CFont* PluginsViewRender::createFont(lua_State *L)
{
    assert(L == renderL);
    fonts.setParentWnd(m_wnd);
    return fonts.create(L);
}

CFont* PluginsViewRender::createDefaultFont()
{
    return _stdfont;
}

void PluginsViewRender::selectPen(CPen* p)
{
    current_pen = p;
}

void PluginsViewRender::selectBrush(CBrush* b)
{
    current_brush = b;
}

void PluginsViewRender::selectFont(CFont* f)
{
    current_font = f;
}

void PluginsViewRender::drawRect(const RECT& r)
{
    if (!m_inside_render)
        return;
    if (current_pen)
        m_dc.SelectPen(*current_pen);
    m_dc.MoveTo(r.left, r.top);
    m_dc.LineTo(r.right, r.top);
    m_dc.LineTo(r.right, r.bottom);
    m_dc.LineTo(r.left, r.bottom);
    m_dc.LineTo(r.left, r.top);    
}

void PluginsViewRender::drawSolidRect(const RECT& r)
{
    if (!m_inside_render)
        return;
    if (current_brush)
        m_dc.FillRect(&r, *current_brush);
    drawRect(r);
}

void PluginsViewRender::print(int x, int y, const tstring& text)
{
    if (!m_inside_render)
        return;
    m_dc.SetBkMode(TRANSPARENT);
    m_dc.SetTextColor(m_text_color);
    if (current_font)
        m_dc.SelectFont(*current_font);
    m_dc.TextOut(x, y, text.c_str(), text.length());
}

void PluginsViewRender::update()
{
    m_wnd.Invalidate();
}
//-------------------------------------------------------------------------------------------------
int render_setbackground(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        r->setBackground(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        r->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, "render.setbackground");
}

int render_textcolor(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = RGB(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
        r->setTextColor(color);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        COLORREF color = lua_tounsigned(L, 2);
        r->setTextColor(color);
        return 0;
    }
    return pluginInvArgs(L, "render.textcolor");
}

int render_width(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->width());
        return 1;
    }
    return pluginInvArgs(L, "render.width");
}

int render_height(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->height());
        return 1;
    }
    return pluginInvArgs(L, "render.height");
}

int render_createpen(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CPen *p = r->createPen(L);
        if (p)
            luaT_pushobject(L, p, LUAT_PEN);
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "render.createpen");
}

int render_createbrush(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CBrush *b = r->createBrush(L);
        if (b)
            luaT_pushobject(L, b, LUAT_BRUSH);
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "render.createbrush");
}

int render_createfont(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CFont *f = r->createFont(L);
        if (f)
            luaT_pushobject(L, f, LUAT_FONT);
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "render.createfont");
}

int render_defaultfont(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CFont *f = r->createDefaultFont();
        if (f)
            luaT_pushobject(L, f, LUAT_FONT);
        else
            lua_pushnil(L);
        return 1;
    }
    return pluginInvArgs(L, "render.defaultfont");
}

int render_select(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUAT_PEN))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CPen* p = (CPen *)luaT_toobject(L, 2);
        r->selectPen(p);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUAT_BRUSH))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CBrush* b = (CBrush *)luaT_toobject(L, 2);
        r->selectBrush(b);
        return 0;
    }
    if (luaT_check(L, 2, LUAT_RENDER, LUAT_FONT))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        CFont* f = (CFont *)luaT_toobject(L, 2);
        r->selectFont(f);
        return 0;
    }
    return pluginInvArgs(L, "render.select");
}

int render_rect(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        RECT rc;
        ParametersReader pr(L);
        pr.getrect(&rc);
        r->drawRect(rc);
        return 0;
    }
    return pluginInvArgs(L, "render.rect");
}

int render_solidrect(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        RECT rc;
        ParametersReader pr(L);
        pr.getrect(&rc);
        r->drawSolidRect(rc);
        return 0;
    }
    return pluginInvArgs(L, "render.solidrect");
}

int render_print(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TSTRING))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        tstring text(TU2W(lua_tostring(L, 4)));
        r->print(lua_tointeger(L, 2), lua_tointeger(L, 3), text);        
        return 0;
    }
    return pluginInvArgs(L, "render.print");
}

int render_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        r->update();
        return 0;
    }
    return pluginInvArgs(L, "render.update");
}

void reg_mt_render(lua_State *L)
{
    luaL_newmetatable(L, "render");
    regFunction(L, "setbackground", render_setbackground);
    regFunction(L, "textcolor", render_textcolor);
    regFunction(L, "width", render_width);
    regFunction(L, "height", render_height);
    regFunction(L, "createpen", render_createpen);
    regFunction(L, "createbrush", render_createbrush);
    regFunction(L, "createfont", render_createfont);
    regFunction(L, "defaultfont", render_defaultfont);
    regFunction(L, "select", render_select);
    regFunction(L, "rect", render_rect);
    regFunction(L, "solidrect", render_solidrect);
    regFunction(L, "print", render_print);
    regFunction(L, "update", render_update);
    regIndexMt(L);
    lua_pop(L, 1);
}
