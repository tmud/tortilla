#pragma once
#include "pluginsRenderObjects.h"

class PluginsViewRender
{
public:
    PluginsViewRender(lua_State *pL, int index, HWND wnd);
    bool render();
    void setBackground(COLORREF color);
    void setTextColor(COLORREF color);
    int  width();
    int  height();
    CPen* createPen(lua_State *L);
    CBrush* createBrush(lua_State *L);
    CFont* createFont(lua_State *L);
    void selectPen(CPen* p);
    void selectBrush(CBrush* b);
    void selectFont(CFont* f);

    void drawRect(const RECT& r);
    void drawSolidRect(const RECT& r);
    void print(int x, int y, const tstring& text);
    void update();
    int  getFontHeight();

private:
    lua_State *renderL;
    int m_render_func_index;
    CWindow m_wnd;
    CDCHandle m_dc;
    bool m_inside_render;

    COLORREF m_bkg_color;
    COLORREF m_text_color;
    int m_width;
    int m_height;

    PluginsRenderCollectionT<CPen, PenFactory> pens;
    PluginsRenderCollectionT<CBrush, BrushFactory> brushes;
    PluginsRenderCollectionT<CFont, FontFactory> fonts;
    //PluginsRenderCollectionT<CBitmap, BitmapFactory> bitmaps;
    CPen* current_pen;
    CBrush* current_brush;
    CFont* current_font;
};
