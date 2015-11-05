#pragma once

template <class T>
class PluginsRenderObjectT
{
public:
    PluginsRenderObjectT(T* obj, const tstring &k) : object(obj), key(k) {}
    void destroy() { delete object; }
    T* object;
    tstring key;
};

template <class T, class F>
class PluginsRenderCollectionT
{
    HWND m_parent;
public:
    PluginsRenderCollectionT() : m_parent(NULL) {}
    ~PluginsRenderCollectionT()
    {
        for (int i=0,e=objects.size();i<e;++i)
            objects[i].destroy();
    }

    T* create(lua_State *L)
    {
        if (!lua_istable(L, -1))
            return NULL;        
        F factory(L);
        int index = find(factory.key);
        if (index != -1)
            return objects[index].object;
        T* obj = factory.create(m_parent);
        if (obj)
            objects.push_back(PluginsRenderObjectT<T>(obj, factory.key));
        return obj;
    }

    void setParentWnd(HWND wnd)
    {
        m_parent = wnd;
    }
private:
    int find(const tstring& key)
    {
        for (int i = 0, e = objects.size(); i < e; ++i) {
            if (objects[i].key == key) return i;
        }
        return -1;
    }
    std::vector<PluginsRenderObjectT<T>> objects;
};

class ParametersReader
{
    lua_State *L;
public:
    ParametersReader(lua_State* pL) : L(pL) {}
    void get(const tchar* field, tstring* value)
    {
        luaT_pushwstring(L, field);
        lua_gettable(L, -2);
        if (lua_isstring(L, -1))
            value->assign(luaT_towstring(L, -1));
        lua_pop(L, 1);
    }
    void get(const tchar* field, int min, int max, int* value)
    {
        luaT_pushwstring(L, field);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1))
        {
            int v = lua_tointeger(L, -1);
            if (v < min) v = min;
            if (v > max) v = max;
            *value = v;
        }
        lua_pop(L, 1);
    }
    bool get(const tchar* field, LONG *value)
    {
        bool result = false;
        luaT_pushwstring(L, field);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1))
        {
            *value = lua_tointeger(L, -1);
            result = true;
        }
        lua_pop(L, 1);
        return result;
    }
    bool get(int index, LONG *value)
    {
        bool result = false;
        lua_pushinteger(L, index);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1))
        {
            *value = lua_tointeger(L, -1);
            result = true;
        }
        lua_pop(L, 1);
        return result;
    }
    bool get(const tchar* field, int *value)
    {
        bool result = false;
        luaT_pushwstring(L, field);
        lua_gettable(L, -2);
        if (lua_isnumber(L, -1))
        {
            *value = lua_tointeger(L, -1);
            result = true;
        }
        lua_pop(L, 1);
        return result;
    }
    bool getcolor(COLORREF *color)
    {
        lua_pushstring(L, "color");
        lua_gettable(L, -2);
        return parsecolor(color);
    }

    bool parsecolor(COLORREF *color)
    {
        bool result = false;
        if (lua_isnumber(L, -1))
        {
            *color = lua_tounsigned(L, -1);
            result = true;
        }
        else if (lua_istable(L, -1))
        {
            int r = 0, g = 0, b = 0;
            get(L"r", 0, 255, &r);
            get(L"g", 0, 255, &g);
            get(L"b", 0, 255, &b);
            *color = RGB(r, g, b);
            result = true;
        }      
        lua_pop(L, 1);
        return result;

    }
    bool getrect(RECT *rc)
    {
        bool left = false, right = false, top = false, bottom = false;

        if (get(L"left", &rc->left))
            left = true;
        if (get(L"top", &rc->top))
            top = true;
        if (get(L"right", &rc->right))
            right = true;
        if (get(L"bottom", &rc->bottom))
            bottom = true;

        if (!left && (get(L"x", &rc->left) || get(1, &rc->left)) )
            left = true;
        if (!top && (get(L"y", &rc->top) || get(2, &rc->top)) )
            top = true;
        if (!right && get(3, &rc->right))
            right = true;
        if (!bottom && get(4, &rc->bottom))
            bottom = true;

        if (get(L"width", &rc->right))
        {
            if (right)
                return false;
            rc->right += rc->left;
            right = true;
        }

        if (get(L"height", &rc->bottom))
        {
            if (bottom)
                return false;
            rc->bottom += rc->top;
            bottom = true;
        }
        return (left && top && right && bottom) ? true : false;     
    }

    bool getxy(int *x, int *y)
    {
        if (get(L"x", x) && get(L"y", y))
            return true;
        return false;
    }
};

class KeyFactoryCalc
{
public:
    void calc(int value)
    {
        delimeter();
        tchar buf[16];
        swprintf(buf, L"%d", value);
        crc.append(buf);
    }
    void calc(const tstring& key)
    {
        delimeter();
        crc.append(key);
    }
    void calc(COLORREF color)
    {
        delimeter();
        tchar buf[16];
        swprintf(buf, L"%d,%d,%d", GetRValue(color),GetGValue(color),GetBValue(color));
        crc.append(buf);
    }

    tstring crc;
private:
    void delimeter()
    {
        if (!crc.empty())
            crc.append(L",");
    }
};

struct PenFactory
{
    tstring key;
    tstring style;
    int width;
    COLORREF color;

    PenFactory(lua_State *L) : width(1), color(0)
    {
        ParametersReader r(L);
        r.get(L"style", &style);
        r.get(L"width", 1, 10, &width);
        r.getcolor(&color);
        KeyFactoryCalc k;
        k.calc(style);
        k.calc(width);
        k.calc(color);
        key = k.crc;
    }

    CPen* create(HWND)
    {
        int s = PS_NULL;
        if (style == L"solid" || style == L"")
            s = PS_SOLID;
        else if (style == L"dash")
            s = PS_DASH;
        else if (style == L"dot")
            s = PS_DOT;
        CPen *p = new CPen;
        p->CreatePen(s, width, color);
        return p;
    }
};

struct BrushFactory
{
    tstring key;
    tstring style;
    COLORREF color;

    BrushFactory(lua_State *L) : color(0)
    {
        ParametersReader r(L);
        r.get(L"style", &style);
        r.getcolor(&color);
        KeyFactoryCalc k;
        k.calc(style);
        k.calc(color);
        key = k.crc;
    }

    CBrush* create(HWND)
    {
        CBrush *b = new CBrush;
        if (style == L"solid" || style == L"")
            b->CreateSolidBrush(color);
        else if (style == L"vertical")
            b->CreateHatchBrush(HS_VERTICAL, color);
        else if (style == L"horizontal")
            b->CreateHatchBrush(HS_HORIZONTAL, color);
        else if (style == L"cross")
            b->CreateHatchBrush(HS_CROSS, color);
        else if (style == L"diagonal")
            b->CreateHatchBrush(HS_BDIAGONAL, color);
        else if (style == L"diagcross")
            b->CreateHatchBrush(HS_DIAGCROSS, color);
        else
            b->CreateSolidBrush(color);
        return b;
    }
};

struct FontFactory
{
    tstring key;
    tstring font_name;
    int font_height;
    int font_bold;
    int font_italic;

    FontFactory(lua_State *L) : font_height(9), font_bold(0), font_italic(0)
    {
        ParametersReader r(L);
        r.get(L"font", &font_name);
        r.get(L"height", 8, 20, &font_height);
        int bold = 0;
        r.get(L"bold", 1, 5, &bold);
        font_bold = bold * 100 + FW_NORMAL;
        r.get(L"italic", 0, 1, &font_italic);

        KeyFactoryCalc k;
        k.calc(font_name);
        k.calc(font_height);
        k.calc(font_bold);
        k.calc(font_italic);
        key = k.crc;
    }

    CFont* create(HWND hwnd)
    {
        CFont *f = new CFont();
        LOGFONT lf;
        lf.lfHeight = -MulDiv(font_height, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72);
        lf.lfWidth = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = font_bold;
        lf.lfItalic = font_italic ? 1 : 0;
        lf.lfUnderline = 0;
        lf.lfStrikeOut = 0;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH;
        wcscpy(lf.lfFaceName, font_name.c_str());
        f->CreateFontIndirect(&lf);
        return f;
    }
};
