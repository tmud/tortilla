#pragma once
#include "pluginsRenderObjects.h"

class PluginsViewRender
{
public:
    PluginsViewRender(lua_State *pL, HWND wnd);
    ~PluginsViewRender();
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
    void regImage(Image *img);

    void point(int x, int y);
    void lineto(int x, int y);

    void ellipse(const RECT& r);
    void solidEllipse(const RECT& r);
    void solidEllipse(const RECT& r, COLORREF color);

    void circle(int x, int y, int r);
    void solidCircle(int x, int y, int r);
    void solidCircle(int x, int y, int r, COLORREF color);

    void drawRect(const RECT& r);
    void drawSolidRect(const RECT& r, COLORREF* solid_color);
    void drawImage(Image *img, int x, int y);
    void drawImage(Image *img, int x, int y, int w, int h);

    int  print(int x, int y, const tstring& text);
    int  print(const RECT& r, const tstring& text);
    void update();
    int  getFontHeight();
    int  getTextWidth(const tstring& text);

private:
    void calcRectForCircle(int x, int y, int r, RECT *rc);

    lua_State *renderL;
    lua_ref m_render_func_ref;
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
    std::vector<Image*> images;

    CPen* current_pen;
    CBrush* current_brush;
    CFont* current_font;
};
