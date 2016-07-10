#include "stdafx.h"
#include "phrase.h"
#include <memory>

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
   bool open(const tstring& path)
   {
       if (hfile != INVALID_HANDLE_VALUE)
       {
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
       return true;
   }
   DWORD size() const { return fsize; }
   bool read(DWORD pos_begin, DWORD len, MemoryBuffer* buffer)
   {
       if (hfile == INVALID_HANDLE_VALUE)
            return false;
       if (pos_begin > fsize || pos_begin+len > fsize)
           return false;
       DWORD toread = len;
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

    bool write(const tstring &path, const tstring& name, const tstring& data, const std::vector<tstring>& tegs)
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
        tstring tmp(name);
        tmp.append(L";");
        for (int i=0,e=tegs.size();i<e;++i) 
        {
            tmp.append(tegs[i]);
            tmp.append(L";");
        }
        tmp.append(L"\n");
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

class new_filewriter
{
    HANDLE hfile;
protected:
    tstring m_path;
    DWORD start_data;
    DWORD written;
public:
    new_filewriter() : hfile(INVALID_HANDLE_VALUE), start_data(0), written(0) {}
    ~new_filewriter() { close(); }
    bool open(const tstring &path)
    {
        hfile = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            return false;
        m_path = true;
        return true;
    }
    void close()
    {
        if (hfile!=INVALID_HANDLE_VALUE)
            CloseHandle(hfile);
        hfile = INVALID_HANDLE_VALUE;
        start_data = 0;
        written = 0;
    }
    void remove()
    {
        error();
    }
protected:
    bool error() 
    {
       close();
       DeleteFile(m_path.c_str());
       return false;
    }
    bool write_tofile(const tstring& t, DWORD *written)
    {
        *written = 0;
        u8string tmp(TW2U(t.c_str()));
        DWORD towrite = tmp.length();
        if (!WriteFile(hfile, tmp.c_str(), towrite, written, NULL) || *written!=towrite)
            return false;
        return true;
    }
    bool write_tofile(const MemoryBuffer& t, DWORD *written)
    {
        *written = 0;
        DWORD towrite = t.getSize();
        if (!WriteFile(hfile, t.getData(), towrite, written, NULL) || *written!=towrite)
            return false;
        return true;
    }
};

class database_file_writer : public new_filewriter
{
public:
    struct WriteResult
    {
        DWORD start;
        DWORD title_len;
        DWORD data_len;
    };
    bool write(const tstring& name, const MemoryBuffer& data, const std::vector<tstring>& tegs, WriteResult* r)
    {
        tstring tmp(name);
        tmp.append(L";");
        for (int i=0,e=tegs.size();i<e;++i) 
        {
            tmp.append(tegs[i]);
            tmp.append(L";");
        }
        tmp.append(L"\n");

        start_data += written;
        DWORD written1 = 0;
        if (!write_tofile(tmp, &written1))
            return error();
        DWORD written2 = 0;
        if (!write_tofile(data, &written2))
            return error();
        written = written1 + written2;
        if (r) 
        {
          r->start = start_data;
          r->title_len = written1;
          r->data_len = written2;
        }
        return true;
    }
};

struct MapDictonaryData
{
    std::string data;
    std::vector<tstring> auto_tegs;
    std::vector<tstring> manual_tegs;
};
typedef std::shared_ptr<MapDictonaryData> data_ptr;
typedef std::map<tstring,data_ptr> MapDictonaryMap;

class MapDictonary
{
    struct fileinfo
    {
        tstring path;
        DWORD size;
    };
    std::vector<fileinfo> m_files;
    struct index
    {
        index() : file(-1), pos_in_file(0), name_tegs_len(0), data_len(0) {}
        tstring name;
        int file;
        DWORD pos_in_file;
        DWORD name_tegs_len;
        DWORD data_len;
        std::vector<tstring> manual_tegs;
    };
    typedef std::shared_ptr<index> index_ptr;

    const int name_teg = 0;
    const int manual_teg = 2;
    const int auto_teg = 1;
    struct position
    {
        index_ptr idx;
        int word_idx;
        int word_type;
    };
    typedef std::vector<position> positions_vector;
    typedef std::shared_ptr<positions_vector> positions_ptr;    
    struct worddata
    {
        tstring word;
        positions_ptr positions;
    };
    std::vector<worddata> m_words_table;
    typedef std::vector<worddata>::iterator words_table_iterator;

    bool m_manual_tegs_changed;

    int m_current_file;
    tstring m_base_dir;
    lua_State *L;
    MemoryBuffer buffer;

    void fileerror(const tstring& file) 
    {
        tstring e(L"������ ������ �����: ");
        e.append(file);
        base::log(L, e.c_str());
    }
    void filesave_error(const tstring& file)
    {
        tstring e(L"������ ������ �����: ");
        e.append(file);
        base::log(L, e.c_str());
    }
public:
    MapDictonary(const tstring& dir, lua_State *pl) : m_manual_tegs_changed(false), m_current_file(-1), m_base_dir(dir), L(pl)
    {
        buffer.alloc(4096);
        load_db();
        load_manual_tegs();
    }
    ~MapDictonary() 
    {
        save_manual_tegs();
    }
    enum { MD_OK = 0, MD_EXIST, MD_ERROR };

    bool update(const lua_ref& r, tstring *error)
    {
        /*typedef std::set<index_ptr> indexes_set;
        std::map<int, indexes_set> cat;
        indexes_set empty;
        for (int i=0,e=m_files.size(); i<e; ++i) {
            cat[i] = empty;
        }
        iterator it = m_indexes.begin(), it_end = m_indexes.end();
        for (; it!=it_end; ++it)
        {
            indexes &x = it->second;
            for (int i=0,e=x.size();i<e;++i)
            {
                if (x[i]->manual) continue;
                int file = x[i]->file;
                indexes_set &c = cat[file];
                c.insert(x[i]);
            }
        }

        MemoryBuffer mb;
        std::vector<tstring> tegs;
        struct new_index { DWORD pos; DWORD title_len; DWORD data_len; std::vector<tstring> tegs; };
        std::unordered_map<index_ptr, new_index*> new_indexes;
        std::vector<DWORD> new_files_size(m_files.size(), 0);

        bool result = true;
        std::map<int, indexes_set>::iterator ft = cat.begin(), ft_end = cat.end();
        for (; ft!=ft_end; ++ft)
        {
            fileinfo& fi = m_files[ ft->first ];
            const indexes_set &s = ft->second;
            filereader fr;
            if (!fr.open(fi.path))
            {
                error->assign(L"������ ��� �������� �����: ");
                error->append(fi.path);
                result = false;
                break;
            }

            if (fi.size != fr.size())
            {
                error->assign(L"���� ������� ������ ����������: ");
                error->append(fi.path);
                error->append(L". ������������� ������.");
                result = false;
                break;            
            }

            tstring newfile_path(fi.path);
            newfile_path.append(L".new");
            database_file_writer fw;
            if (!fw.open(newfile_path))
            {
                error->assign(L"������ ��� �������� ����� �� ������: ");
                error->append(newfile_path);
                result = false;
                break;
            }

            indexes_set::iterator si = s.begin(), si_end = s.end();
            for (;si != si_end; ++si)
            {
                index_ptr i = (*si);
                DWORD pos = i->pos_in_file+i->name_tegs_len;
                DWORD len = i->data_len;
                if (!fr.read(pos, len, &mb))
                {
                    error->assign(L"������ ��� ������ �����: ");
                    error->append(fi.path);
                    result = false;
                    break;
                }

                u8string data(mb.getData(), mb.getSize());
                r.pushValue(L);
                lua_pushstring(L, data.c_str());
                if (lua_pcall(L, 1, 1, 0))
                {
                    error->assign(lua_toerror(L));
                    result = false;
                    break;
                }

                tegs.clear();
                if (lua_istable(L, -1))
                {
                    lua_pushnil(L);                     // first key
                    while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
                    {
                        if (lua_isstring(L, -1))
                        {
                            tstring teg(luaT_towstring(L, -1));
                            tegs.push_back(teg);
                        }
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);
                database_file_writer::WriteResult r;
                if (!fw.write(i->name, mb, tegs, &r))
                {
                    error->assign(L"������ ��� ������ �����: ");
                    error->append(newfile_path);
                    result = false;
                    break;
                }
                new_index *ni = new new_index;
                ni->pos = r.start; ni->title_len=r.title_len; ni->data_len = r.data_len;
                tegs.push_back(i->name);
                ni->tegs.swap(tegs);
                new_indexes[i] = ni;
                new_files_size[ft->first] = r.start + (r.data_len+r.title_len);
            }
            if (!result) 
            {
                fw.remove();
                break;
            }
        }

        if (result)
        {
            // �������������� ������ ������
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                tstring oldname(name);
                oldname.append(L".old");
                if (!MoveFile(name.c_str(), oldname.c_str()))
                {
                    error->assign(L"������ ��� �������������� ������. ������������ �� �������.");
                    result = false;
                    break;
                }
            }
        }

        if (result)
        {
            // �������������� ����� ������
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                tstring newname(name);
                newname.append(L".new");
                if (!MoveFile(newname.c_str(), name.c_str()))
                {
                    error->assign(L"������ ��� �������������� ������. ������������ �� �������.");
                    result = false;
                    break;
                }
            }
        }

        if (result)
        {
             // �������� ������ ������
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                name.append(L".old");
                DeleteFile(name.c_str());
                m_files[i].size = new_files_size[i];
            }

            // ������������� ������� � ����
            m_indexes.clear();
            m_phrases.clear();
            m_objects.clear();
            std::map<tstring, manual_tegs_ptr> new_manual_tegs;
            std::unordered_map<index_ptr, new_index*>::iterator it = new_indexes.begin(), it_end = new_indexes.end();
            for (; it!=it_end; ++it)
            {
                index_ptr i = it->first;
                new_index* ni = it->second;
                i->pos_in_file = ni->pos;
                i->name_tegs_len = ni->title_len;
                i->data_len = ni->data_len;
                std::vector<tstring>& new_tegs = ni->tegs;
                for (int k=0,e=new_tegs.size();k<e;++k)
                    add_index(new_tegs[k], i);
                m_objects[i->name] = i;

                manual_tegs_iterator mt = m_manual_tegs.find(i->name);
                if (mt != m_manual_tegs.end())
                {
                    manual_tegs_ptr newmt = std::make_shared<manual_tegs>();
                    manual_tegs_ptr p = mt->second;
                    for (int j=0,e=p->size(); j<e; ++j) 
                    {
                       const tstring& manual_teg = p->at(j);
                       if (std::find(new_tegs.begin(), new_tegs.end(), manual_teg) == new_tegs.end())
                           newmt->push_back(manual_teg);
                    }
                    if (!newmt->empty())
                    {
                        index_ptr mi = std::make_shared<index>(*i);
                        mi->manual = true;
                        for (int j=0,e=newmt->size(); j<e; ++j)
                        {
                            const tstring& manual_teg = newmt->at(j);
                            add_index(manual_teg, mi);
                        }
                        new_manual_tegs[i->name] = newmt;
                    }
                }
                delete ni;
            }
            new_indexes.clear();
            m_manual_tegs.swap(new_manual_tegs);
            m_manual_tegs_changed = true;
            save_manual_tegs();
        }
        return result;*/
        return true;
    }

    void wipe()
    {
        m_current_file = -1;
        for (int i=0,e=m_files.size(); i<e; ++i) {
          DeleteFile(m_files[i].path.c_str());
        }
        m_files.clear();
        m_words_table.clear();
    }

    enum TegResult { ERR = 0, ABSENT, EXIST, ADDED, REMOVED };
    TegResult teg(const tstring& name, const tstring& teg)
    {
        if (teg.find(L";") != tstring::npos)
            return ERR;

        index_ptr ix = find_name(name);
        if (!ix)
            return ABSENT;
        tstring t(teg);
        tstring_tolower(&t);
        std::vector<tstring>& tegs = ix->manual_tegs;
        std::vector<tstring>::iterator it = std::find(tegs.begin(), tegs.end(), t);
        if (it != tegs.end())
        {
            tegs.erase(it);
            Phrase p(teg);
            for (int i=0,e=p.len();i<e;++i)
            {
                int pos = 0;
                if (find_pos(p.get(i), &pos))
                {
                    positions_vector& pv = *m_words_table[i].positions;
                    for (int i=0,e=pv.size();i<e;++i) {
                      if (pv[i].idx == ix) { pv.erase(pv.begin()+i); break; }
                    }
                }
            }
            m_manual_tegs_changed = true;
            return REMOVED;
        }

        Phrase p(teg);
        if (p.len()==0)
            return ERR;




        tstring t(teg);
        tstring_tolower(&t);
        int pos = 0;
        if (find_pos(t, &pos))
        {
            tstring n(name);
            tstring_tolower(&n);
            positions_vector &v = *m_words_table[pos].positions;
            for (int i=0,e=v.size();i<e;++i) {
                if (v[i].word_idx == manual_teg && v[i].idx->name == n)
                {
                    v.erase(v.begin()+i);
                    m_manual_tegs_changed = true;
                    return REMOVED;
                }
            }
        }

        index_ptr ix = find_name(name);
        if (!ix)    
            return ABSENT;
        add_index(teg, ix, true, true);
        m_manual_tegs_changed = true;
        return ADDED;
    }

    int add(const tstring& name, const tstring& data, const std::vector<tstring>& tegs)
    {
        if (name.empty() || data.empty())
            return MD_ERROR;
        if (find_name(name))
            return MD_EXIST;
        index_ptr ix = add_tofile(name, data, tegs);
        if (ix->file == -1)
            return MD_ERROR;
        add_index(name, ix, false, false);
        for (int i=0,e=tegs.size();i<e;++i)
            add_index(tegs[i], ix, true, false);
        return MD_OK;
    }

    bool find(const tstring& name, MapDictonaryMap* values)
    {
        Phrase p(name);
        if (p.len()==0)
            return false;
        positions_vector tmp;
        for (int i=0,e=p.len();i<e;++i)
        {
            find_similar(p.get(i), i, tmp);
            if (tmp.empty())
                return false;
        }

        // ������� ���������
        std::set<index_ptr> result;
        std::for_each(tmp.begin(), tmp.end(), [&result](position& p){ result.insert(p.idx); });

        // ������ ������
        std::vector<filereader> open_files(m_files.size());
        std::set<index_ptr>::iterator rt = result.begin(), rt_end = result.end();
        for (; rt != rt_end; ++rt)
        {
            index_ptr ix = *rt;

            int fileid = ix->file;
            fileinfo& fi = m_files[fileid];
            filereader& fr = open_files[fileid];
            if (!fr.open(fi.path) || fi.size != fr.size())
            {
                fileerror(fi.path);
                continue;
            }

            bool result = fr.read(ix->pos_in_file, ix->name_tegs_len, &buffer);
            if (!result)
            {
                fileerror(fi.path);
                continue;
            }

            data_ptr d = std::make_shared<MapDictonaryData>();
            {
                int size = buffer.getSize();
                buffer.keepalloc(size+1);
                char *b = buffer.getData();
                b[size] = 0;
                Tokenizer tk(TU2W(b), L";\r\n");
                tk.trimempty();
                for (int i=1,e=tk.size();i<e;++i)
                    d->auto_tegs.push_back(tk[i]);
            }
            result = fr.read(ix->pos_in_file+ix->name_tegs_len, ix->data_len, &buffer);
            if (!result)
            {
                fileerror(fi.path);
                continue;
            }
            // load data
            {
                int size = buffer.getSize();
                buffer.keepalloc(size+1);
                char *b = buffer.getData();
                b[size] = 0;
                d->data.assign(b);
            }
            // �������� manual tegs
            const std::vector<tstring> &t = ix->manual_tegs;
            d->manual_tegs.assign(t.begin(), t.end());
            values->operator[](ix->name) = d;
        }
        return true;
    }

private:
    void find_similar(const tstring& start_symbols, int word_idx, positions_vector& words_indexes)
    {
        if (start_symbols.empty())
            return;
        int begin = -1; int end = m_words_table.size();
        tchar c = start_symbols.at(0);
        for (int i=0;i<end;++i)
        {
            const tstring& word = m_words_table[i].word;
            if (word.compare(0, start_symbols.size(), start_symbols)==0)
              { begin = i; break; }
        }
        if (begin == -1) return;
        for (int i=begin,e=end;i<e;++i)
        {
            const tstring& word = m_words_table[i].word;
            if (word.compare(0, start_symbols.size(), start_symbols)!=0)
              { end = i; break; }
        }

        if (word_idx == 0)
        {
            for (int i=begin;i<end;++i)
            {
                const positions_vector& pv = *m_words_table[i].positions;
                std::for_each(pv.begin(), pv.end(), [&](const position& p) {
                    if (p.word_type == name_teg || p.word_idx == 0)
                        words_indexes.push_back(p);
                });
                words_indexes.insert(words_indexes.end(), pv.begin(), pv.end());
            }
        }
        else
        {
            positions_vector words_tmp;
            for (int j=0,je=words_indexes.size();j<je;++j)
            {
                bool found = false;
                for (int k=begin;k<end;++k)
                {
                    const positions_vector& pv = *m_words_table[k].positions;
                    for (int i=0,e=pv.size();i<e;++i)
                    {
                        if (words_indexes[j].idx != pv[i].idx)
                            continue;
                        if (words_indexes[j].word_type == name_teg)
                        {
                             if (words_indexes[j].word_idx < pv[i].word_idx)
                                found = true;
                        }
                        else
                        {
                            if ((pv[i].word_idx - words_indexes[j].word_idx) == 1)
                                found = true;                        
                        }

                        if (found)
                        {
                            words_tmp.push_back(pv[i]);
                            break; 
                        }
                    }
                    if (found) break;
                }
            }
            words_indexes.swap(words_tmp);
        }
    }

    bool find_pos(const tstring& word, int* pos)
    {
        int index = 0;
        int b = 0, e = m_words_table.size();
        while (b != e)
        {
           index = (e-b)/2;
           index += b;
           const tstring& key = m_words_table[index].word;
           if (key == word)
           {
               if (pos)
                   *pos = index;
               return true;
           }
           if (key < word)
           {
               b = index+1;
               index = b;
           }
           else
           {
               e = index;
           }
        }
        if (pos)
            *pos = index;
        return false;
    }

    index_ptr find_name(const tstring& name)
    {
        Phrase p(name);
        if (p.len() == 0)
            return 0;
        int pi = 0;
        if (find_pos(p.get(0), &pi))
        {
            tstring n(name);
            tstring_tolower(&n);
            positions_vector &pv = *m_words_table[pi].positions;
            for (int i = 0, e = pv.size(); i < e; ++i) {
                if (pv[i].idx->name == n)
                    return pv[i].idx;
            }
        }
        return 0;
    }

    void add_index(const tstring& name, index_ptr ix, bool teg, bool manual)
    {
       Phrase p(name);
       for (int i=0,len=p.len(); i<len; ++i)
       {
          position word_pos;
          word_pos.word_type = name_teg;
          if (teg) word_pos.word_type = (manual) ? manual_teg : auto_teg;
          word_pos.word_idx = i;
          word_pos.idx = ix;
          positions_ptr positions_vector_ptr;
          tstring part(p.get(i));
          int index = 0;
          if (find_pos(part, &index))
              positions_vector_ptr = m_words_table[index].positions;
          else
          {
              worddata wd;
              wd.word = part;
              wd.positions = std::make_shared<positions_vector>();
              positions_vector_ptr = wd.positions;
              int dc = m_words_table.size() - m_words_table.capacity();
              if (dc == 0)
                  m_words_table.reserve(m_words_table.size()+1000);
              m_words_table.insert(m_words_table.begin()+index, wd);
          }
          positions_vector_ptr->push_back(word_pos);
       }
    }

    index_ptr add_tofile(const tstring& name, const tstring& data, const std::vector<tstring>& tegs)
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

        index_ptr ix = std::make_shared<index>();
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
        if (!fw.write(f.path, name, data, tegs))
           return ix;
        f.size += fw.written;
        ix->file = m_current_file;
        ix->pos_in_file = fw.start_name;
        ix->name_tegs_len = fw.start_data - fw.start_name;
        ix->data_len = fw.written - (fw.start_data - fw.start_name);
        ix->name = name;
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
                    DWORD max_size = max_db_filesize + 16384;
                    if (fd.nFileSizeLow >= max_size)
                        continue;
                    tstring name(fd.cFileName);
                    int len = name.length()-3;
                    name = name.substr(0,len);
                    if (!isOnlyDigits(name))
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
                fileerror(m_files[i].path);
                continue;
            }

            index_ptr ix = std::make_shared<index>();
            ix->file = i;

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
                        ix->data_len = lf.getPosition()-ix->pos_in_file-ix->name_tegs_len;
                        Tokenizer tk(TU2W(name.c_str()), L";");
                        ix->name = tk[0];
                        {
                           for (int i=0,e=tk.size();i<e;++i)
                               add_index(tk[i], ix, (i!=0), false);
                        }
                        ix = std::make_shared<index>();
                        ix->file = i;
                        name.clear();
                    }
                    continue;
                }
                if (find_name_mode) 
                {
                    name.assign(str);
                    find_name_mode = false;
                    ix->pos_in_file = start_pos;
                    ix->name_tegs_len = lf.getPosition() - start_pos;
                }
            }
        }
    }

    void save_manual_tegs()
    {
        if (!m_manual_tegs_changed)
            return;

        class tegs_file_writer : public new_filewriter  {
        public:
        bool write(const tstring& name, const std::vector<tstring>& tegs)
        {
            tstring s(name);
            for (int i=0,e=tegs.size();i<e;++i)
            {
                s.append(L";");
                s.append(tegs[i]);
            }
            DWORD written = 0;
            return write_tofile(s, &written);
        }} file;

        tstring path(m_base_dir);
        path.append(L"usertegs.new");
        if (!file.open(path.c_str()))
        {
            filesave_error(path);
            return;
        }

        words_table_iterator it = m_words_table.begin(), it_end = m_words_table.end();
        for(; it!=it_end;++it)
        {
            positions_vector &pv = *it->positions;
            for (int i=0,e=pv.size();i<e;++i)
            {
                if (pv[i].word_type == manual_teg && pv[i].word_idx == 0)
                {
                    index_ptr p = pv[i].idx;
                    if (!file.write(p->name,p->manual_tegs))
                    {
                        filesave_error(path);
                        return;
                    }
                }
            }
        }
        file.close();
        tstring newpath(m_base_dir);
        newpath.append(L"usertegs.db");
        if (!::MoveFileEx(path.c_str(), newpath.c_str(), MOVEFILE_REPLACE_EXISTING))
            return;
        m_manual_tegs_changed = false;
    }

    void load_manual_tegs()
    {
        tstring path(m_base_dir);
        path.append(L"usertegs.db");
        load_file fr(path);
        if (fr.file_missed)
            return;
        if (!fr.result)
        {
            tstring error(L"������ ��� �������� �����: ");
            error.append(path.c_str());
            fileerror( error.c_str() );
            return;
        }
        std::string s;
        while (fr.readNextString(&s))
        {
            tstring ws(TU2W(s.c_str()));
            Tokenizer tk(ws.c_str(), L";");
            if (tk.size() < 2) continue;
            const tstring& name = tk[0];
            for (int i=1,e=tk.size();i<e;++i)
                teg(name, tk[i]);
        }
        fr.close();
        m_manual_tegs_changed = false;
    }
};

int dict_add(lua_State *L)
{
    if (luaT_check(L, 3, get_dict(L), LUA_TSTRING, LUA_TSTRING) || 
        luaT_check(L, 4, get_dict(L), LUA_TSTRING, LUA_TSTRING, LUA_TTABLE))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        tstring info(luaT_towstring(L, 3));
        std::vector<tstring> tegs;
        if (lua_gettop(L) == 4)
        {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
            {
                tstring teg(luaT_towstring(L, -1));
                tegs.push_back(teg);
                lua_pop(L, 1);
            }
        }
        int result = d->add(id, info, tegs);
        if (result == MapDictonary::MD_OK)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
        lua_pushboolean(L, 0);
        lua_pushstring(L, result == MapDictonary::MD_EXIST ? "exist" : "error");
        return 2;
    }
    return dict_invalidargs(L, "add");
}

int dict_find(lua_State *L)
{
    if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        MapDictonaryMap result;
        typedef MapDictonaryMap::iterator iterator;
        if (d->find(id, &result) && !result.empty())
        {
            lua_newtable(L);
            iterator it = result.begin(), it_end = result.end();
            for (;it!=it_end;++it)
            {
                luaT_pushwstring(L, it->first.c_str());
                lua_newtable(L);

                data_ptr p = it->second;
                lua_pushstring(L, "data");
                lua_pushstring(L, p->data.c_str());
                lua_settable(L, -3);

                const std::vector<tstring>& at = p->auto_tegs;
                lua_pushstring(L, "auto");
                lua_newtable(L);
                for (int i=0,e=at.size();i<e;++i)
                {
                    lua_pushinteger(L, i+1);
                    luaT_pushwstring(L, at[i].c_str());
                    lua_settable(L, -3);
                }                
                lua_settable(L, -3);

                const std::vector<tstring>& mt = p->manual_tegs;
                lua_pushstring(L, "tegs");
                lua_newtable(L);
                for (int i=0,e=mt.size();i<e;++i)
                {
                    lua_pushinteger(L, i+1);
                    luaT_pushwstring(L, mt[i].c_str());
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);
                lua_settable(L, -3);
            }
        }
        else
            lua_pushnil(L);
        return 1;
    }
    return dict_invalidargs(L, "find");
}

int dict_update(lua_State *L)
{
   if (luaT_check(L, 2, get_dict(L), LUA_TFUNCTION))
   {
       MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
       lua_ref r;
       r.createRef(L);
       tstring error;
       bool result = d->update(r, &error);
       r.unref(L);
       if (result) { lua_pushboolean(L, 1); return 1; }
       lua_pushboolean(L, 0);
       luaT_pushwstring(L, error.c_str());
       return 2;
   }
   return dict_invalidargs(L, "update");
}

int dict_wipe(lua_State *L)
{
   if (luaT_check(L, 1, get_dict(L)))
   {
       MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
       d->wipe();
       return 0;
   }
   return dict_invalidargs(L, "wipe");
}

int dict_teg(lua_State *L)
{
    if (luaT_check(L, 3, get_dict(L), LUA_TSTRING, LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring name(luaT_towstring(L, 2));
        tstring teg(luaT_towstring(L, 3));
        MapDictonary::TegResult result = d->teg(name, teg);
        switch (result) {
        case MapDictonary::ABSENT:
            lua_pushstring(L, "absent");
            break;
        case MapDictonary::EXIST:
            lua_pushstring(L, "exist");
            break;
        case MapDictonary::ADDED:
            lua_pushstring(L, "added");
            break;
        case MapDictonary::REMOVED:
            lua_pushstring(L, "removed");
            break;
        default:
            lua_pushnil(L);
        }
        return 1;
    }
    return dict_invalidargs(L, "teg");
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

int dict_new(lua_State *L)
{
    if (!luaT_check(L, 1, LUA_TSTRING))
    {
        luaT_push_args(L, "dictonary");
        return lua_error(L);
    }
    tstring path(luaT_towstring(L, 1));

    if (get_dict(L) == -1)
    {
        int type = luaT_regtype(L, "dictonary");
        if (!type)
            return 0;
        regtype_dict(L, type);
        luaL_newmetatable(L, "dictonary");
        regFunction(L, "add", dict_add);
        regFunction(L, "find", dict_find);
        regFunction(L, "update", dict_update);
        regFunction(L, "wipe", dict_wipe);
        regFunction(L, "teg", dict_teg);

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
    MapDictonary* nd = new MapDictonary(path, L);
    luaT_pushobject(L, nd, get_dict(L));
    return 1;
}
