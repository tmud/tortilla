#include "stdafx.h"
#include "phrase.h"

std::map<lua_State*, int> m_dict_types;
typedef std::map<lua_State*, int>::iterator iterator;
int get_dict(lua_State *L)
{
    iterator it = m_dict_types.find(L);
    if (it == m_dict_types.end())
        return -1;
    return it->second;
}
void regtype_dict(lua_State *L, int type)
{
    m_dict_types[L] = type;
}
int dict_invalidargs(lua_State *L, const char* function_name)
{
    luaT_push_args(L, function_name);
    return lua_error(L);
}

class MapDictonary 
{
    typedef std::vector<tstring> collection;
public:
    MapDictonary() {}
    ~MapDictonary() {
        std::for_each(m_dictonary.begin(), m_dictonary.end(),
            [](std::pair<const tstring, collection*> &o) { delete o.second; });
    }
    void add(const tstring& name, const tstring& data)
    {
        iterator it = m_dictonary.find(name),it_end = m_dictonary.end();
        if (it == it_end)
        {
            collection *c = new collection;
            c->push_back(data);
            m_dictonary[name] = c;
        }
        else
        {
            collection *c = it->second;
            c->push_back(data);        
        }
        Phrase *p = new Phrase(name.c_str());
        m_phrases.addPhrase(p);
    }
private:
    PhrasesList m_phrases;
    std::unordered_map<tstring, collection*> m_dictonary;
    typedef std::unordered_map<tstring, collection*>::iterator iterator;
};

int dict_add(lua_State *L)
{
    if (luaT_check(L, 3, get_dict(L), LUA_TSTRING, LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        tstring info(luaT_towstring(L, 3));        
        d->add(id, info);
        lua_pushboolean(L, 1);
        return 1;
    }
    return dict_invalidargs(L, "add");
}

int dict_find(lua_State *L)
{
    return 0;
}

int dict_load(lua_State *L)
{
    return 0;
}

int dict_save(lua_State *L)
{
    return 0;
}

int dict_gc(lua_State *L)
{
    if (luaT_check(L, 1, get_dict(L)))
    {
        MapDictonary *d = (MapDictonary *)luaT_toobject(L, 1);
        delete d;
    }
    return 0;
}

int dict_new(lua_State *L)
{
    if (lua_gettop(L) != 0)
    {
        luaT_push_args(L, "new");
        return lua_error(L);
    }
    if (get_dict(L) == -1)
    {
        int type = luaT_regtype(L, "dictonary");
        if (!type)
            return 0;
        regtype_dict(L, type);
        luaL_newmetatable(L, "dictonary");
        regFunction(L, "add", dict_add);
        regFunction(L, "find", dict_find);
        regFunction(L, "load", dict_load);
        regFunction(L, "save", dict_save);
        regFunction(L, "__gc", dict_gc);
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pushstring(L, "__metatable");
        lua_pushstring(L, "access denied");
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    MapDictonary* nd = new MapDictonary();
    luaT_pushobject(L, nd, get_dict(L));
    return 1;
}
