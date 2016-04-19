#include "stdafx.h"
#include "phrase.h"

const DWORD max_db_filesize = 4 * 1024 * 1024; // 4mb
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
    PhrasesList m_phrases;
    struct fileinfo
    {
        tstring path;
        DWORD size;
    };
    std::vector<fileinfo> m_files;
    struct index 
    {
        int file;
        DWORD pos_in_file;
    }; 
    typedef std::vector<index> indexes;
    std::unordered_map<tstring, indexes> m_indexes;
    int m_current_file;

public:
    MapDictonary() : m_current_file(-1) {}
    ~MapDictonary() {
        //std::for_each(m_dictonary.begin(), m_dictonary.end(),           [](std::pair<const tstring, collection*> &o) { delete o.second; });
    }
    void add(const tstring& name, const tstring& data)
    {
        tstring n(name);
        tstring_tolower(&n);
        Phrase p(n);
        int count = p.len();
        
        /*



        iterator it = m_dictonary.find(n),it_end = m_dictonary.end();
        if (it == it_end)
        {
            collection *c = new collection;
            c->push_back(data);
            m_dictonary[n] = c;
        }
        else
        {
            collection *c = it->second;
            c->push_back(data);
        }
        Phrase p(n);
        if (p.len() > 1)
        {
            for (int i=0,e=p.len();i<e;++i)
                m_phrases.addPhrase( new Phrase(p.get(i)) );
        }
        m_phrases.addPhrase( new Phrase(n) );     */
    }
   /* const collection* find(const tstring& name)
    {
        tstring n(name);
        tstring_tolower(&n);
        Phrase p(n);
        tstring result;
        if (!m_phrases.findPhrase(p, &result))
            return NULL;
        iterator it = m_dictonary.find(result);
        if (it == m_dictonary.end())
            return NULL;
        return it->second;
    }
    bool remove(const tstring& name)
    {
        iterator it = m_dictonary.find(name);
        if (it != m_dictonary.end())
        {
            m_dictonary.erase(it);
            return true;
        }
        return false;
    }*/
private:
    index add_tofile(const tstring& data)
    {
        if (m_current_file != -1) {
           fileinfo &f = m_files[m_current_file];
           if (f.size > max_db_filesize) {
               m_current_file = -1;
           }
        }
        if (m_current_file == -1) {
            for (int i=0,e=m_files.size();i<e;++i) 
            {
                if (m_files[i].size < max_db_filesize)
                    { m_current_file = i; break; }
            }
        }
        if (m_current_file == -1) {
            int idx = m_files.size();
            tchar buffer[16];
            while(true) {
                swprintf(buffer,L"%d.db", idx); 
                if (GetFileAttributes(buffer) == INVALID_FILE_ATTRIBUTES)
                    break;
                idx++;
            } 
            index i;
            i.file = -1;
            i.pos_in_file = 0;
            fileinfo f;
            f.path = buffer;
            HANDLE hfile = CreateFile(buffer, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hfile == INVALID_HANDLE_VALUE)
                return i;
            DWORD pos = SetFilePointer(hfile, 0, NULL, FILE_CURRENT);
            DWORD written = 0; DWORD towrite = data.length()*sizeof(tchar);
            if (!WriteFile(hfile, data.c_str(), towrite, &written, NULL) || written!=towrite)
            {
                SetFilePointer(hfile,pos,NULL,FILE_CURRENT);
                CloseHandle(hfile);
                return i;
            }
            CloseHandle(hfile);
            f.size = towrite;
            m_current_file = m_files.size();
            m_files.push_back(f);
            i.file = m_current_file;
            return i;
        }

        index i;
        i.file = -1;
        i.pos_in_file = 0;
        fileinfo &f = m_files[m_current_file];
        HANDLE hfile = CreateFile(f.path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
                return i;
        DWORD size = GetFileSize(hfile, NULL);
        SetFilePointer(hfile, 0, NULL, FILE_END);
        DWORD written = 0; DWORD towrite = data.length()*sizeof(tchar);
        if (!WriteFile(hfile, data.c_str(), towrite, &written, NULL) || written!=towrite)
        {
            SetFilePointer(hfile,size,NULL,FILE_CURRENT);
            CloseHandle(hfile);
            return i;
        }
        i.file = m_current_file;
        i.pos_in_file = size;
        CloseHandle(hfile);
        return i;        
    }

    void load_db(const tstring& path)
    {
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.db", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    if (fd.nFileSizeHigh > 0)
                        continue;
                    DWORD max_size = max_db_filesize + 4096;
                    if (fd.nFileSizeLow >= max_size)
                        continue;
                    fileinfo f;
                    f.path = fd.cFileName;
                    f.size = fd.nFileSizeLow;
                    m_files.push_back(f);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
        std::sort(m_files.begin(),m_files.end(),[](const fileinfo&a, const fileinfo&b) {
            return a.path < b.path;
        });
        for (int i=0,e=m_files.size();i<e;++i)
        {
            //todo read files in to catalog
        }
    }    
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
    /*if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        const collection *c = d->find(id);
        if (!c)
            lua_pushnil(L);
        else
        {
            lua_newtable(L);
            for (int i=0,e=c->size();i<e;++i)
            {
                lua_pushinteger(L, i+1);
                luaT_pushwstring(L, c->at(i).c_str());
                lua_settable(L, -3);
            }
        }
        return 1;
    }*/
    return dict_invalidargs(L, "find");
}

int dict_remove(lua_State *L)
{
  /*  if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        bool result = d->remove(id);
        lua_pushboolean(L, result ? 1:0);
        return 1;
    }*/
    return dict_invalidargs(L, "remove");
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
        regFunction(L, "remove", dict_remove);
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
