#include "stdafx.h"
#include "phrase.h"

std::map<lua_State*, int> m_decl_types;
typedef std::map<lua_State*, int>::iterator iterator;
int gettype(lua_State *L)
{
    iterator it = m_decl_types.find(L);
    if (it == m_decl_types.end())
        return -1;
    return it->second;
}
void regtype(lua_State *L, int type)
{
    m_decl_types[L] = type;
}

int declension_invalidargs(lua_State *L, const char* function_name)
{
    luaT_push_args(L, function_name);
    return lua_error(L);
}

int declension_add(lua_State *L)
{
    if (luaT_check(L, 2, gettype(L), LUA_TSTRING))
    {
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        tstring p(luaT_towstring(L, 2));
        bool result = d->addPhrase(p);
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return declension_invalidargs(L, "add");
}

int declension_find(lua_State *L)
{
    if (luaT_check(L, 2, gettype(L), LUA_TSTRING))
    {
        tstring result_string;
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        tstring p(luaT_towstring(L, 2));
        if (d->findPhrase(p, &result_string))
        {
            luaT_pushwstring(L, result_string.c_str());
            return 1;
        }
        return 0;
    }
    return declension_invalidargs(L, "find");
}

int declension_remove(lua_State *L)
{
    if (luaT_check(L, 2, gettype(L), LUA_TSTRING))
    {
        /*todo Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        tstring p(luaT_towstring(L, 2));
        d->de

        if (d->findPhrase(p, &result_string))
        {
            luaT_pushwstring(L, result_string.c_str());
            return 1;
        }*/
        return 0;
    }
    return declension_invalidargs(L, "remove");

}

int declension_load(lua_State *L)
{
    if (luaT_check(L, 2, gettype(L), LUA_TSTRING))
    {
        bool result = false;
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        tstring filepath(luaT_towstring(L, 2));
        load_file lf(filepath);
        result = lf.result;
        if (result)
        {
            d->clear();
            for (int i=0,e=lf.text.size(); i<e; ++i)
            {
                tstring t(TU2W(lf.text[i].c_str()));
                d->addPhrase(t);
            }
        } else {
            if (lf.file_missed)
                result = true;
        }
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return declension_invalidargs(L, "load");
}

int declension_save(lua_State *L)
{
    if (luaT_check(L, 2, gettype(L), LUA_TSTRING))
    {
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        if (!d->getAndResetChanged())
        {
            lua_pushboolean(L, 1);
            return 1;
        }

        bool result = false;
        tstring filepath(luaT_towstring(L,2));
        HANDLE hFile = CreateFile(filepath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            result = true;
            tstring s; char br[3] = { 13, 10, 0 };
            for (int i=1,e=d->maxPhrasesLen();i<=e;++i)
            {
                PhrasesList *list = d->getPhrasesList(i);
                if (!list) continue;
                for (int j=0,je=list->getPhrasesCount();j<je;++j)
                {
                    const Phrase *p = list->getPhrase(j);
                    p->getFullPhrase(&s);
                    u8string object(TW2U(s.c_str()));
                    object.append(br);
                    DWORD written = 0;
                    DWORD towrite = object.length();
                    if ( !WriteFile(hFile, object.c_str(), towrite, &written, NULL ) || written != towrite)
                    {
                        result = false;
                        break;
                    }
                }
                if (!result) break;
            }
            CloseHandle(hFile);
        }
        lua_pushboolean(L, result ? 1 : 0);
        return 1;
    }
    return declension_invalidargs(L, "save");
}

int declension_clear(lua_State *L)
{
    if (luaT_check(L, 1, gettype(L)))
    {
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        d->clear();
        return 0;
    }
    return declension_invalidargs(L, "clear");
}

int declension_compare(lua_State *L)
{
   if (luaT_check(L, 3, gettype(L), LUA_TSTRING, LUA_TSTRING))
   {
      tstring t1(luaT_towstring(L, 2));
      tstring t2(luaT_towstring(L, 3));
      Phrase p1(t1);
      Phrase p2(t2);
      int result = p1.similar(p2) ? 1 : 0;
      lua_pushboolean(L, result);
      return 1;
   }
   return declension_invalidargs(L, "compare");
}

int declension_check(lua_State *L)
{
   if (luaT_check(L, 3, gettype(L), LUA_TSTRING, LUA_TNUMBER))
   {
      Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
      tstring p(luaT_towstring(L, 2));
      bool result = d->check(p, lua_tointeger(L, 3)-1);
      lua_pushboolean(L, result ? 1 : 0);
      return 1;
   }
   return declension_invalidargs(L, "check");
}

int declension_gc(lua_State *L)
{
    if (luaT_check(L, 1, gettype(L)))
    {
        Dictonary *d = (Dictonary *)luaT_toobject(L, 1);
        delete d;
    }
    return 0;
}

int declension_new(lua_State *L)
{
    if (lua_gettop(L) != 0)
    {
        luaT_push_args(L, "new");
        return lua_error(L);
    }

    if (gettype(L) == -1)
    {
        int type = luaT_regtype(L, "declension");
        if (!type)
            return 0;
        regtype(L, type);
        luaL_newmetatable(L, "declension");
        regFunction(L, "add", declension_add );
        regFunction(L, "find", declension_find );
        regFunction(L, "remove", declension_remove );
        regFunction(L, "load", declension_load );
        regFunction(L, "save", declension_save );
        regFunction(L, "clear", declension_clear );
        regFunction(L, "compare", declension_compare );
        regFunction(L, "check", declension_check );
        regFunction(L, "__gc", declension_gc);
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pushstring(L, "__metatable");
        lua_pushstring(L, "access denied");
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    Dictonary* nd = new Dictonary();
    luaT_pushobject(L, nd, gettype(L));
    return 1;
}
