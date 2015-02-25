#pragma once
#include "pluginsRenderObjects.h"

class PluginsViewRender
{
public:
    PluginsViewRender(lua_State *pL, int index, HWND wnd);
    void render();
    void setBackground(COLORREF color);
    int  width();
    int  height();
    CPen* createPen(lua_State *L);

private:
    lua_State *renderL;
    int m_render_func_index;
    CWindow m_wnd;
    CDCHandle m_dc;
    bool m_inside_render;

    COLORREF m_bkg_color;
    int m_width;
    int m_height;

    PluginsRenderCollectionT<CPen, PenFactory> pens;
    /*PluginsRenderCollectionT<CBrush> brushes;
    PluginsRenderCollectionT<CFont> fonts;
    PluginsRenderCollectionT<CBitmap> bitmaps;*/

private:
    void push_robject(lua_State* L, void *object)
    {
        /*luaT_userdata *o = (luaT_userdata*)lua_newuserdata(L, sizeof(luaT_userdata));
        o->data = object;
        o->type = type;*/
    }
};

