#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsViewRender.h"
#include "pluginSupport.h"

PluginsViewRender::PluginsViewRender(lua_State *pL, int index, HWND wnd) : renderL(pL), m_render_func_index(index), m_wnd(wnd),
m_inside_render(false), m_bkg_color(0), m_text_color(RGB(128,128,128)), m_width(0), m_height(0),
current_pen(NULL), current_brush(NULL), current_font(NULL)
{
    assert(pL && index > 0);
}

PluginsViewRender::~PluginsViewRender()
{
    lua_State *L = renderL;
    lua_getglobal(L, "_pvrender");
    if (lua_istable(L, -1) && m_render_func_index > 0)
    {
        lua_pushinteger(L, m_render_func_index);
        lua_pushnil(L);
        lua_settable(L, -3);
    }
    lua_pop(L, 1);

    for (int i=0,e=images.size(); i<e; ++i)
        delete images[i];
}

bool PluginsViewRender::render()
{
    lua_State *L = renderL;

    RECT rc; m_wnd.GetClientRect(&rc);
    CPaintDC dc(m_wnd);
    CMemoryDC mdc(dc, rc);
    m_dc = mdc;

    m_width = rc.right; m_height = rc.bottom;
    m_dc.FillSolidRect(&rc, m_bkg_color);

    lua_getglobal(L, "_pvrender");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return true;
    }
    lua_pushinteger(L, m_render_func_index);
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return true;
    }
    lua_insert(L, -2);
    lua_pop(L, 1);
    luaT_pushobject(L, this, LUAT_RENDER);
    m_inside_render = true;
    bool result = true;
    if (lua_pcall(L, 1, 0, 0))
    {
        // error in call
        result = false;
        if (luaT_check(L, 1, LUA_TSTRING))
            pluginError(L"render", luaT_towstring(L, -1));
        else
            pluginError(L"render", L"неизвестная ошибка");
        lua_settop(L, 0);
    }
    m_inside_render = false;
    return result;
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

void PluginsViewRender::regImage(Image *img)
{
    images.push_back(img);
}

void PluginsViewRender::drawRect(const RECT& r)
{
    if (!m_inside_render)
        return;
    if (r.left >= r.right || r.top >= r.bottom)
        return;
    if (current_pen)
        m_dc.SelectPen(*current_pen);    
    m_dc.MoveTo(r.left, r.top);
    m_dc.LineTo(r.right-1, r.top);
    m_dc.LineTo(r.right-1, r.bottom-1);
    m_dc.LineTo(r.left, r.bottom-1);
    m_dc.LineTo(r.left, r.top);
}

void PluginsViewRender::drawSolidRect(const RECT& r)
{
    if (!m_inside_render)
        return;
    if (r.left > r.right || r.top > r.bottom)
        return;
    if (current_brush)
        m_dc.FillRect(&r, *current_brush);
}

void PluginsViewRender::drawImage(Image *img, int x, int y)
{
    if (!m_inside_render)
        return;
    img->render(m_dc, x, y);
}

void PluginsViewRender::drawImage(Image *img, int x, int y, int w, int h)
{
    if (!m_inside_render)
        return;
    image_render_ex p; p.w = w; p.h = h;
    img->render(m_dc, x, y, &p);
}

int PluginsViewRender::print(int x, int y, const tstring& text)
{
    if (!m_inside_render)
        return 0;
    m_dc.SetBkMode(TRANSPARENT);
    m_dc.SetTextColor(m_text_color);
    if (current_font)
        m_dc.SelectFont(*current_font);
    SIZE sz = { 0, 0 };
    GetTextExtentPoint32(m_dc, text.c_str(), text.length(), &sz);
    m_dc.TextOut(x, y, text.c_str(), text.length());
    return sz.cx;
}

void PluginsViewRender::update()
{
    m_wnd.Invalidate();
}

int PluginsViewRender::getFontHeight()
{
    CDC dc(m_wnd.GetDC());
    HFONT old = (current_font) ? dc.SelectFont(*current_font) : NULL;
    SIZE sz = { 0, 0 };
    GetTextExtentPoint32(dc, L"W", 1, &sz);
    if (current_font)
        dc.SelectFont(old);
    return sz.cy;
}

int PluginsViewRender::getTextWidth(const tstring& text)
{
    CDC dc(m_wnd.GetDC());
    HFONT old = (current_font) ? dc.SelectFont(*current_font) : NULL;
    SIZE sz = { 0, 0 };
    GetTextExtentPoint32(dc, text.c_str(), text.length(), &sz);
    if (current_font)
        dc.SelectFont(old);
    return sz.cx;
}
//-------------------------------------------------------------------------------------------------
int render_setBackground(lua_State *L)
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

    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        ParametersReader reader(L);
        COLORREF color = 0;
        reader.parsecolor(&color);
        r->setBackground(color);
        return 0;
    }
    return pluginInvArgs(L, L"render:setBackground");
}

int render_textColor(lua_State *L)
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
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TTABLE))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        ParametersReader reader(L);
        COLORREF color = 0;
        reader.parsecolor(&color);
        r->setTextColor(color);
        return 0;
    }
    return pluginInvArgs(L, L"render:textColor");
}

int render_width(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->width());
        return 1;
    }
    return pluginInvArgs(L, L"render:width");
}

int render_height(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->height());
        return 1;
    }
    return pluginInvArgs(L, L"render:height");
}

int render_createPen(lua_State *L)
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
    return pluginInvArgs(L, L"render:createPen");
}

int render_createBrush(lua_State *L)
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
    return pluginInvArgs(L, L"render:createBrush");
}

int render_createFont(lua_State *L)
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
    return pluginInvArgs(L, L"render:createFont");
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
    return pluginInvArgs(L, L"render:select");
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
    return pluginInvArgs(L, L"render:rect");
}

int render_solidRect(lua_State *L)
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
    return pluginInvArgs(L, L"render:solidRect");
}

int render_print(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUA_TNUMBER, LUA_TNUMBER, LUA_TSTRING))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        tstring text(TU2W(lua_tostring(L, 4)));
        int width = r->print(lua_tointeger(L, 2), lua_tointeger(L, 3), text);
        lua_pushinteger(L, width);
        return 1;
    }
    return pluginInvArgs(L, L"render:print");
}

int render_update(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        r->update();
        return 0;
    }
    return pluginInvArgs(L, L"render:update");
}

int render_fontHeight(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_RENDER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        lua_pushinteger(L, r->getFontHeight());
        return 1;
    }
    return pluginInvArgs(L, L"render:fontHeight");
}

int render_textWidth(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TSTRING))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        tstring text(TU2W(lua_tostring(L, 2)));
        lua_pushinteger(L, r->getTextWidth(text));
        return 1;
    }
    return pluginInvArgs(L, L"render:textWidth");
}

int render_createImage(lua_State *L)
{
    if (luaT_check(L, 2, LUAT_RENDER, LUA_TSTRING) || 
        luaT_check(L, 3, LUAT_RENDER, LUA_TSTRING, LUA_TNUMBER))
    {
        tstring path(luaT_towstring(L, 2));
        tstring_replace(&path, L"/", L"\\");
        Image *img = new Image;
        int param = (lua_gettop(L) == 3) ? lua_tointeger(L, 3) : -1;
        if (!img->load(path.c_str(), param))
        {
            delete img;
            pluginLoadFail(L, L"render::createImage", luaT_towstring(L, 2));
            lua_pushnil(L);
            return 1;
        }
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        r->regImage(img);
        luaT_pushobject(L, img, LUAT_IMAGE);
        return 1;
    }
    return pluginInvArgs(L, L"render::createImage");
}

int render_drawImage(lua_State *L)
{
    if (luaT_check(L, 4, LUAT_RENDER, LUAT_IMAGE, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        Image *img = (Image *)luaT_toobject(L, 2);
        r->drawImage(img,lua_tointeger(L, 3), lua_tointeger(L, 4));
        return 0;
    }
    else if (luaT_check(L, 6, LUAT_RENDER, LUAT_IMAGE, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER, LUA_TNUMBER))
    {
        PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
        Image *img = (Image *)luaT_toobject(L, 2);
        r->drawImage(img, lua_tointeger(L, 3), lua_tointeger(L, 4), lua_tointeger(L, 5), lua_tointeger(L, 6));
        return 0;
    }
    else if (luaT_check(L, 3, LUAT_RENDER, LUAT_IMAGE, LUA_TTABLE))
    {
        ParametersReader pr(L);
        RECT rc;
        if (pr.getrect(&rc))
        {
            PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
            Image *img = (Image *)luaT_toobject(L, 2);
            r->drawImage(img, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
            return 0;
        }
        int x =0; int y = 0;
        if (pr.getxy(&x, &y))
        {
            PluginsViewRender *r = (PluginsViewRender *)luaT_toobject(L, 1);
            Image *img = (Image *)luaT_toobject(L, 2);
            r->drawImage(img, x, y);
            return 0;
        }
    }
    return pluginInvArgs(L, L"render::drawImage");
}

void reg_mt_render(lua_State *L)
{
    luaL_newmetatable(L, "render");
    regFunction(L, "setBackground", render_setBackground);
    regFunction(L, "textColor", render_textColor);
    regFunction(L, "width", render_width);
    regFunction(L, "height", render_height);
    regFunction(L, "createPen", render_createPen);
    regFunction(L, "createBrush", render_createBrush);
    regFunction(L, "createFont", render_createFont);
    regFunction(L, "createImage", render_createImage);
    regFunction(L, "select", render_select);
    regFunction(L, "rect", render_rect);
    regFunction(L, "solidRect", render_solidRect);
    regFunction(L, "print", render_print);
    regFunction(L, "drawImage", render_drawImage);
    regFunction(L, "update", render_update);
    regFunction(L, "fontHeight", render_fontHeight);
    regFunction(L, "textWidth", render_textWidth);
    regIndexMt(L);
    lua_pop(L, 1);
}
