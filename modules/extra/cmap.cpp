#include "stdafx.h"
#include "phrase.h"

const DWORD max_db_filesize = 8 * 1024 * 1024; // 8mb
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

class filereader
{
    HANDLE hfile;
    DWORD  fsize;
public:
   filereader() : hfile(INVALID_HANDLE_VALUE), fsize(0) {}
   ~filereader() { if (hfile != INVALID_HANDLE_VALUE) CloseHandle(hfile); }
   bool open(const tstring& path, DWORD csize)
   {
       if (hfile != INVALID_HANDLE_VALUE)
       {
           assert(fsize == csize);
           return true;       
       }
       hfile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
       if (hfile == INVALID_HANDLE_VALUE)
            return false;
       DWORD hsize = 0;
       DWORD lsize = GetFileSize(hfile, &hsize);
       if (hsize > 0)
       {
           CloseHandle(hfile);
           hfile = INVALID_HANDLE_VALUE;
           return false;
       }
       fsize = lsize;
       assert(fsize == csize);
       return true;
   }
   DWORD size() const { return fsize; }
   bool read(DWORD pos_begin, DWORD pos_end, MemoryBuffer* buffer)
   {
       if (hfile == INVALID_HANDLE_VALUE)
            return false;
       if (pos_begin > fsize || pos_end > fsize || pos_end < pos_begin)
           return false;
       DWORD toread = pos_end - pos_begin;
       if (SetFilePointer(hfile, pos_begin, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
           return false;
       buffer->alloc(toread);
       DWORD readed = 0;
       if (!ReadFile(hfile, buffer->getData(), toread, &readed, NULL) || readed != toread)
       {
           buffer->alloc(0);
           return false;       
       }
       return true;
   }
};

class filewriter
{
    HANDLE hfile;
public:
    filewriter() : hfile(INVALID_HANDLE_VALUE), start_name(0), start_data(0), written(0) {}
    ~filewriter() { if (hfile!=INVALID_HANDLE_VALUE) CloseHandle(hfile); }
    DWORD start_name;
    DWORD start_data;
    DWORD written;

    bool write(const tstring &path, const tstring& name, const tstring& data)
    {
        hfile = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);                             
        if (hfile == INVALID_HANDLE_VALUE)
            return false;
        DWORD hsize = 0;
        DWORD size = GetFileSize(hfile, &hsize);
        if (hsize > 0)
            return false;
        if (SetFilePointer(hfile, size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return false;        
        DWORD written1 = 0;
        tstring tmp(name); tmp.append(L"\n");
        if (!write_tofile(hfile, tmp, &written1))
            return error(size);
        DWORD written2 = 0;
        if (!write_tofile(hfile, data, &written2))
            return error(size);
        DWORD written3 = 0;
        if (!write_tofile(hfile, L"\n\n", &written3))
            return error(size);
        start_name = size;
        start_data = size + written1;
        written = written1 + written2 + written3;
        CloseHandle(hfile);
        hfile = INVALID_HANDLE_VALUE;
        return true;    
    }

private:
  bool error(DWORD pos) 
  {
      SetFilePointer(hfile,pos,NULL,FILE_BEGIN);
      SetEndOfFile(hfile);
      CloseHandle(hfile);
      hfile = INVALID_HANDLE_VALUE;
      return false;
  }
  bool write_tofile(HANDLE hfile, const tstring& t, DWORD *written)
  {
      *written = 0;
      u8string tmp(TW2U(t.c_str()));
      DWORD towrite = tmp.length();
      if (!WriteFile(hfile, tmp.c_str(), towrite, written, NULL) || *written!=towrite)
          return false;
      return true;
  }
};

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
        index() : file(-1), pos_in_file(0), data_in_file(0) {}
        int file;
        DWORD pos_in_file;
        DWORD data_in_file;
    };
    typedef std::vector<index> indexes;
    std::unordered_map<tstring, indexes> m_indexes;
    typedef std::unordered_map<tstring, indexes>::iterator iterator;
    int m_current_file;
    tstring m_base_dir;
    lua_State *L;
    void error(const tstring& error) {
        base::log(L, error.c_str());
    }

public:
    MapDictonary(const tstring& dir, lua_State *pl) : m_current_file(-1), m_base_dir(dir), L(pl) {
        load_db();
    }
    ~MapDictonary() {}
    void add(const tstring& name, const tstring& data)
    {
        index ix = add_tofile(name, data);
        if (ix.file == -1)
            return;
        add_index(name, ix);
    }
    bool find(const tstring& name, tstring* value)
    {
        tstring n(name);
        tstring_tolower(&n);
        Phrase p(n);
        tstring result;
        if (!m_phrases.findPhrase(p, &result))
            return false;
        iterator it = m_indexes.find(result);
        if (it == m_indexes.end())
            return false;
        
        MemoryBuffer buffer;
        std::vector<filereader> open_files(m_files.size());        
        indexes &ix = it->second;
        for (int i=0,e=ix.size()-1;i<=e;++i)
        {
            int fileid = ix[i].file;
            fileinfo& fi = m_files[fileid];
            filereader& fr = open_files[fileid];
            if (!fr.open(fi.path, fi.size))
            {
                //error
                continue;
            }
            bool result = false;
            if (i == e)
                result = fr.read(ix[i].data_in_file, fr.size(), &buffer);
            else
            {
                DWORD pos_end = ix[i+1].pos_in_file;                
                result = fr.read(ix[i].data_in_file, pos_end, &buffer);
            }
            int size = buffer.getSize();
            buffer.keepalloc(size+1);
            char *p = buffer.getData();
            p[size] = 0;
            value->append(TU2W(p));
        }        
        return true;
    }

private:
    void add_index(const tstring& name, index ix)
    {
        tstring n(name);
        tstring_tolower(&n);
        Phrase p(n);
        int count = p.len();
        if (count > 1)
        {
            for (int i = 0, e = p.len(); i < e; ++i)
            {
                m_phrases.addPhrase(new Phrase(p.get(i)));
                add_toindex(p.get(i), ix);
            }
        }
        m_phrases.addPhrase(new Phrase(n));
        add_toindex(n, ix);
    }
    void add_toindex(const tstring& t, index ix)
    {
        iterator it = m_indexes.find(t);
        if (it == m_indexes.end())
        {
            indexes empty;
            m_indexes[t] = empty;
            it = m_indexes.find(t);
        }
        it->second.push_back(ix);
    }

    index add_tofile(const tstring& name, const tstring& data)
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

        index ix;
        if (m_current_file == -1)
        {
            int idx = m_files.size();
            tchar buffer[16];
            tstring filename;
            while(true) {
                swprintf(buffer,L"%d.db", idx);
                filename.assign(m_base_dir);
                filename.append(buffer);
                if (GetFileAttributes(filename.c_str()) == INVALID_FILE_ATTRIBUTES)
                    break;
                idx++;
            }
            m_current_file = m_files.size();
            fileinfo f;
            f.path = filename;
            f.size = 0;
            m_files.push_back(f);
        }
        fileinfo &f = m_files[m_current_file];
        filewriter fw;
        if (!fw.write(f.path, name, data))
           return ix;
        f.size += fw.written;
        ix.file = m_current_file;
        ix.pos_in_file = fw.start_name;
        ix.data_in_file = fw.start_data;
        return ix;
    }

    void load_db()
    {
        tstring mask(m_base_dir);
        mask.append(L"*.db");
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(mask.c_str(), &fd);
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
                    tstring path(m_base_dir);
                    path.append(fd.cFileName);
                    f.path = path;
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
            // read files in to catalog
            load_file lf(m_files[i].path);
            if (!lf.result) {
                tstring e(L"Не получилось загрузить файл: ");
                e.append(m_files[i].path);
                error(e);
                continue;
            }

            index ix;
            ix.file = i;
            ix.pos_in_file = 0;
            ix.data_in_file = 0;

            DWORD start_pos = 0;
            u8string str, name;
            bool find_name_mode = true;
            while (lf.readNextString(&str, &start_pos))
            {
                if (str.empty())
                {
                    find_name_mode = true;
                    if (!name.empty())
                    {                        
                        if (ix.data_in_file != 0)
                        {
                            tstring tn(TU2W(name.c_str()));
                            add_index(tn, ix);
                        }
                        ix.pos_in_file = 0;
                        ix.data_in_file = 0;
                        name.clear();
                    }                    
                    continue;
                }
                if (find_name_mode) 
                {
                    name.assign(str);                  
                    find_name_mode = false;
                    ix.pos_in_file = start_pos;
                    ix.data_in_file = lf.getPosition();
                }
            }

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
    if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        tstring result;
        if (d->find(id, &result))
            luaT_pushwstring(L, result.c_str());
        else
            lua_pushnil(L);
        return 1;
    }
    return dict_invalidargs(L, "find");
}

/*int dict_remove(lua_State *L)
{
    if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        bool result = d->remove(id);
        lua_pushboolean(L, result ? 1:0);
        return 1;
    }
    return dict_invalidargs(L, "remove");
}*/

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
        //regFunction(L, "remove", dict_remove);
        regFunction(L, "__gc", dict_gc);
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);
        lua_settable(L, -3);
        lua_pushstring(L, "__metatable");
        lua_pushstring(L, "access denied");
        lua_settable(L, -3);
        lua_pop(L, 1);
    }
    tstring path;
    base::getPath(L, L"", &path);
    MapDictonary* nd = new MapDictonary(path, L);
    luaT_pushobject(L, nd, get_dict(L));
    return 1;
}
