#include "stdafx.h"
#include "../mudclient/src/common/tokenizer.h"
#include "common.h"

void tstring_tolower(tstring *str)
{
    std::locale loc("");
    const std::ctype<wchar_t>& ct = std::use_facet<std::ctype<wchar_t> >(loc);
    std::transform(str->begin(), str->end(), str->begin(), std::bind1st(std::mem_fun(&std::ctype<wchar_t>::tolower), &ct));
}

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

class Phrase
{
public:
    Phrase(const tchar* phrase_in_string) 
    {
        tstring p(phrase_in_string);
        tstring_tolower(&p);
        Tokenizer t(p.c_str(), L" ");
        t.trimempty();
        t.swap(words);
    }
    int len() const
    {
        return words.size(); 
    }
    const tstring& get(int index) const
    {
        return words[index];
    }
    bool similar(const Phrase& p) const 
    {
        int plen = len();
        if (plen != p.len())
            return false;
        if (plen == 0) return true;
        bool result = true;
        for (int i=0; i<plen;++i)
        {
              const tstring &p1 = words[i];
              const tstring &p2 = p.words[i];
              int len = min(p1.size(), p2.size());
              int delta = max(p1.size(), p2.size()) - len;
              if (delta > 2)
                  { result = false; break; }
              int compared = 0;
              for (int i=0;i<len;++i) {
                  if (p1[i] != p2[i]) break;
                  compared++;
              }
              int notcompared = len - compared;
              if (notcompared > compared)
                  { result = false; break; }
        }
        return result;
    }
    bool less(const Phrase& p) const
    {
        tstring ps, s;
        p.getFullPhrase(&ps);
        getFullPhrase(&s);
        return s < ps;
    }
    bool equal(const Phrase& p) const
    {
        int slen = len();
        if (slen != p.len()) return false;
        for (int i=0;i<slen;++i) {
          if (words[i] != p.words[i])
              return false;
        }
        return true;
    }

    void getFullPhrase(tstring *s) const
    {
        s->clear();
        for (int i=0,e=words.size();i<e;++i)
        {
            if (i!=0) s->append(L" ");
            s->append(words[i]);
        }
    }
private:
    std::vector<tstring> words;
};

class PhrasesList
{
public:
    bool addPhrase(Phrase *p)
    {
       int index = 0;
       int b = 0, e = m_phrases.size();
       while (b != e)
       {
            index = (e-b)/2;
            index += b;
            if (m_phrases[index]->equal(*p))
            {
                delete p;
                return true;
            }
            if (m_phrases[index]->less(*p))
            {
                b = index+1;
                index = b;
            }
            else
            {
                e = index;
            }
       }
       m_phrases.insert(m_phrases.begin()+index, p);
       return true;
    }
    bool findPhrase(const Phrase& p, tstring* result)
    {
        int begin = -1;
        tchar c = p.get(0).at(0);
        for (int i=0,e=m_phrases.size();i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c == c2) { begin = i; break; }
        }
        if (begin == -1) return false;
        int end = m_phrases.size();
        for (int i=begin,e=m_phrases.size();i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c != c2) { end = i; break; }
        }
        for (int i=begin;i<end;++i)
        {
            if (m_phrases[i]->similar(p))
            {
                m_phrases[i]->getFullPhrase(result);
                return true;
            }
        }
        return false;
    }
    const Phrase* getPhrase(int index) const
    {
        return (index >= 0 && index < getPhrasesCount()) ? m_phrases[index] : NULL;    
    }
    int getPhrasesCount() const 
    {
        return m_phrases.size();
    }
private:
    std::vector<Phrase*> m_phrases;
};

// словарь слов
class Dictonary
{
public:
    Dictonary() : m_changed(false) {}
    ~Dictonary() { clear(); }
    bool addPhrase(const tchar *phrase) 
    {
        Phrase *p = new Phrase(phrase);
        int len = p->len();
        if (len == 0)
            { delete p; return false; }
        bool result = true;
        iterator it = m_data.find(len);
        if (it == m_data.end())
        {
            PhrasesList *newlist = new PhrasesList();
            result = newlist->addPhrase(p);
            if (!result)
                { delete newlist; }
            else
                m_data[len] = newlist;
        }
        else
        {
            result = it->second->addPhrase(p);
        }
        if (result)
            m_changed = true;
        return result;
    }
    bool findPhrase(const tchar* t, tstring* result) const
    {
        Phrase p(t);
        int len = p.len();
        if (len == 0)
            return false;
        iterator it = m_data.find(len);
        if (it == m_data.end())
            return false;
        return it->second->findPhrase(p, result);
    }
    void clear() 
    {
        std::for_each(m_data.begin(), m_data.end(), [](std::pair<int, PhrasesList*> p){ delete p.second; });
        m_data.clear();
        m_changed = true;
    }
    bool check(const tchar* t, int index) const
    {
        Phrase p(t);
        int len = p.len();
        if (len == 0)
            return false;
        iterator it = m_data.find(len);
        if (it == m_data.end())
            return false;
        const Phrase *p2 = it->second->getPhrase(index);
        if (!p2)
            return false;
        return p.equal(*p2);
    }
    int maxPhrasesLen() const 
    {
        int result = 0;
        iterator it = m_data.begin(), it_end = m_data.end();
        for (;it != it_end;++it)
        {
            if (it->first > result)
                result = it->first;
        }
        return result;
    }
    PhrasesList* getPhrasesList(int index) const 
    {
        iterator it = m_data.find(index);
        return (it != m_data.end()) ? it->second : NULL;
    }
    bool getAndResetChanged()
    {
        bool state = m_changed;
        m_changed = false;
        return state;
    }
private:
    bool m_changed;
    std::map<int, PhrasesList*> m_data;
    typedef std::map<int, PhrasesList*>::const_iterator iterator;
};

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
        bool result = d->addPhrase(luaT_towstring(L, 2));
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
        if (d->findPhrase(luaT_towstring(L, 2), &result_string))
        {
            luaT_pushwstring(L, result_string.c_str());
            return 1;
        }
        return 0;
    }
    return declension_invalidargs(L, "find");
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
                TU2W t(lf.text[i].c_str());
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
      Phrase p1(luaT_towstring(L, 2));
      Phrase p2(luaT_towstring(L, 3));
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
      bool result = d->check(luaT_towstring(L, 2), lua_tointeger(L, 3)-1);
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
