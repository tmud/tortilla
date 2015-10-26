#pragma once

template <class T>
class PluginsRenderObjectT
{
public:
    PluginsRenderObjectT(T* obj, const u8string &k) : object(obj), key(k) {}
    void destroy() { delete object; }
    T* object;
    u8string key;
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
    int find(const u8string& key)
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
    void get(const utf8* field, u8string* value)
    {
        lua_pushstring(L, field);
        lua_gettable(L, -2);
        if (lua_isstring(L, -1))
            value->assign(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    void get(const utf8* field, int min, int max, int* value)
    {
        lua_pushstring(L, field);
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
    bool get(const utf8* field, LONG *value)
    {
        bool result = false;
        lua_pushstring(L, field);
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
    bool get(const utf8* field, int *value)
    {
        bool result = false;
        lua_pushstring(L, field);
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
            get("r", 0, 255, &r);
            get("g", 0, 255, &g);
            get("b", 0, 255, &b);
            *color = RGB(r, g, b);
            result = true;
        }      
        lua_pop(L, 1);
        return result;

    }
    bool getrect(RECT *rc)
    {
        bool left = false, right = false, top = false, bottom = false;

        if (get("left", &rc->left))
            left = true;
        if (get("top", &rc->top))
            top = true;
        if (get("right", &rc->right))
            right = true;
        if (get("bottom", &rc->bottom))
            bottom = true;

        if (!left && (get("x", &rc->left) || get(1, &rc->left)) )
            left = true;
        if (!top && (get("y", &rc->top) || get(2, &rc->top)) )
            top = true;
        if (!right && get(3, &rc->right))
            right = true;
        if (!bottom && get(4, &rc->bottom))
            bottom = true;

        if (get("width", &rc->right))
        {
            if (right)
                return false;
            rc->right += rc->left;
            right = true;
        }

        if (get("height", &rc->bottom))
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
        if (get("x", x) && get("y", y))
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
        utf8 buf[16];
        sprintf(buf, "%d", value);
        crc.append(buf);
    }
    void calc(const u8string& key)
    {
        delimeter();
        crc.append(key);
    }
    void calc(COLORREF color)
    {
        delimeter();
        utf8 buf[16];
        sprintf(buf, "%d,%d,%d", GetRValue(color),GetGValue(color),GetBValue(color));
        crc.append(buf);
    }

    u8string crc;
private:
    void delimeter()
    {
        if (!crc.empty())
            crc.append(",");
    }
};

struct PenFactory
{
    u8string key;
    u8string style;
    int width;
    COLORREF color;

    PenFactory(lua_State *L) : width(1), color(0)
    {
        ParametersReader r(L);
        r.get("style", &style);
        r.get("width", 1, 10, &width);
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
        if (style == "solid" || style == "")
            s = PS_SOLID;
        else if (style == "dash")
            s = PS_DASH;
        else if (style == "dot")
            s = PS_DOT;
        CPen *p = new CPen;
        p->CreatePen(s, width, color);
        return p;
    }
};

struct BrushFactory
{
    u8string key;
    u8string style;
    COLORREF color;

    BrushFactory(lua_State *L) : color(0)
    {
        ParametersReader r(L);
        r.get("style", &style);
        r.getcolor(&color);
        KeyFactoryCalc k;
        k.calc(style);
        k.calc(color);
        key = k.crc;
    }

    CBrush* create(HWND)
    {
        CBrush *b = new CBrush;
        if (style == "solid" || style == "")
            b->CreateSolidBrush(color);
        else if (style == "vertical")
            b->CreateHatchBrush(HS_VERTICAL, color);
        else if (style == "horizontal")
            b->CreateHatchBrush(HS_HORIZONTAL, color);
        else if (style == "cross")
            b->CreateHatchBrush(HS_CROSS, color);
        else if (style == "diagonal")
            b->CreateHatchBrush(HS_BDIAGONAL, color);
        else if (style == "diagcross")
            b->CreateHatchBrush(HS_DIAGCROSS, color);
        else
            b->CreateSolidBrush(color);
        return b;
    }
};

struct FontFactory
{
    u8string key;
    u8string font_name;
    int font_height;
    int font_bold;
    int font_italic;

    FontFactory(lua_State *L) : font_height(9), font_bold(0), font_italic(0)
    {
        ParametersReader r(L);
        r.get("font", &font_name);
        r.get("height", 8, 20, &font_height);
        int bold = 0;
        r.get("bold", 1, 5, &bold);
        font_bold = bold * 100 + FW_NORMAL;
        r.get("italic", 0, 1, &font_italic);

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
        wcscpy(lf.lfFaceName, TU2W(font_name.c_str()));
        f->CreateFontIndirect(&lf);
        return f;
    }
};
