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
public:
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
        T* obj = factory.create();
        if (obj)
            objects.push_back(PluginsRenderObjectT<T>(obj, factory.key));
        return obj;
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
    void getcolor(COLORREF *color)
    {
        int r = 0, g = 0, b = 0;
        get("r", 0, 255, &r);
        get("g", 0, 255, &g);
        get("b", 0, 255, &b);
        *color = RGB(r, g, b);
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

    CPen* create()
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
