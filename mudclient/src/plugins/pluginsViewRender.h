#pragma once

template <class T>
class PluginsRenderObjectT
{
public:
    ~PluginsRenderObjectT<T>() { delete object; }
    u8string key;
    T* object;    
};

template <class T>
class PluginsRenderCollectionT
{
public:
    int find(const u8string& key)
    {
        for (int i = 0, e = objects.size(); i < e; ++i) {
            if (objects[i].key == key) return i;
        }
        return -1;
    }
    std::vector<PluginsRenderObjectT<T>> objects;
};

class PluginsViewRender
{
public:
    PluginsViewRender(lua_State *pL, int index, HWND wnd);
    void render();
    void setBackground(COLORREF color);
    int  width();
    int  height();

private:
    lua_State *L;
    int m_render_func_index;
    CWindow m_wnd;
    CDCHandle m_dc;
    bool m_inside_render;

    COLORREF m_bkg_color;
    int m_width;
    int m_height;

    PluginsRenderCollectionT<CPen> pens;
    PluginsRenderCollectionT<CBrush> brushes;
    PluginsRenderCollectionT<CFont> fonts;
    PluginsRenderCollectionT<CBitmap> bitmaps;

private:
    void push_robject(lua_State* L, void *object)
    {
        /*luaT_userdata *o = (luaT_userdata*)lua_newuserdata(L, sizeof(luaT_userdata));
        o->data = object;
        o->type = type;*/
    }
};

