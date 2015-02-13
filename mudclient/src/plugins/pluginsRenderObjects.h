#pragma once

template <class T>
class PluginsRenderObjectT
{
public:
    PluginsRenderObjectT(T* obj, const u8string &k) : object(obj), key(k) {}
    ~PluginsRenderObjectT<T>() { delete object; }    
    T* object;
    u8string key;
};

template <class T, class R>
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

    T* create(lua_State *L)
    {
        if (!lua_istable(L, -1))
            return NULL;        
        R reader(L);
        reader.key

        T* obj = reader.create(L);
        if (obj)
            objects.push_back(PluginsRenderObjectT<T>(obj, reader.key));
        return obj;
    }    
    std::vector<PluginsRenderObjectT<T>> objects;
};

namespace reader {
class Reader
{
    lua_State *L;    
public:
    Reader(lua_State* pL) : L(pL) {}
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
    }
};

struct Pen 
{
    u8string key;
    u8string style;
    int width;
    COLORREF color;

    Pen(lua_State *L) : width(1), color(0)
    {
        Reader r(L);
        r.get("style", &style);
        r.get("width", 1, 10, &width);
        r.getcolor(&color);
    }

    CPen* create()
    {
        retrun NULL;
    }    
};

} // reader
