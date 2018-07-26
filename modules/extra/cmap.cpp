#include "stdafx.h"
#include "phrase.h"
#include <memory>

const tchar* maindb_file = L"main.db";
const tchar* patchdb_file = L"patch.db";

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
	tstring filepath;
	DWORD len;
public:
	filewriter() : hfile(INVALID_HANDLE_VALUE), len(0) {}
	~filewriter() { if (hfile != INVALID_HANDLE_VALUE) close(); }
    bool open(const tstring &path)
	{
        filepath = path;
		hfile = CreateFile(filepath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hfile == INVALID_HANDLE_VALUE)
			return false;
		DWORD hsize = 0;
		DWORD size = GetFileSize(hfile, &hsize);
		if (hsize > 0) {
			close();
			return false;
		}
		if (SetFilePointer(hfile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
		{
			close();
			return false;
		}
		return true;
	}
	bool write(const tstring& data)
    {
		u8string tmp(TW2U(data.c_str()));
		return write(tmp);
    }
    void close()
    {
	    CloseHandle(hfile);
	    hfile = INVALID_HANDLE_VALUE;
    }
	void remove()
	{
		close();
		if (!filepath.empty())
			DeleteFile(filepath.c_str());
	}
	DWORD datalen() {
		return len;
	}
private:
    bool write(const std::string& data)
    {
        if (hfile == INVALID_HANDLE_VALUE)
            return false;
        DWORD written = 0;
        DWORD towrite = data.length();
        if (!WriteFile(hfile, data.c_str(), towrite, &written, NULL) || written != towrite)
        {
            SetFilePointer(hfile, len, NULL, FILE_BEGIN);
            return false;
        }
        len += written;
        return true;
    }
};

class database_diff_writer 
{
    filewriter fw;
public:
    bool init(const tstring& filepath)
    {
        return fw.open(filepath);
    }
    bool write_object(const tstring& name, const tstring& data, const std::vector<tstring>& auto_tegs)
    {
        tstring out(L"O;"); out.append(name);
        for (const tstring& t : auto_tegs)
        {
            out.append(L";");
            out.append(t);
        }
        out.append(L"\n");
        out.append(data);
        out.append(L"\n\n");
        return fw.write(out);
    }
    bool write_delete_object(const tstring& name) 
    {
        tstring out(L"o;"); out.append(name);
        out.append(L"\n");
        return fw.write(out);    
    }
    bool write_teg(const tstring& name, const tstring& teg)
    {
        tstring out(L"T;"); out.append(name);
        out.append(L";"); out.append(teg);
        out.append(L"\n");
        return fw.write(out);
    }
    bool write_delete_teg(const tstring& name, const tstring& teg)
    {
        tstring out(L"t;"); out.append(name);
        out.append(L";"); out.append(teg);
        out.append(L"\n");
        return fw.write(out);
    }
    bool write_info(const tstring& name, const tstring& info)
    {
        tstring out(L"I;"); out.append(name);
        out.append(L";"); out.append(info);
        out.append(L"\n");
        return fw.write(out);
    }
    bool write_delete_info(const tstring& name)
    {
        tstring out(L"i;"); out.append(name);
        out.append(L"\n");
        return fw.write(out);
    }
};

struct index
{
	index() {}
	tstring name, comment, data;
	std::vector<tstring> auto_tegs;
	std::vector<tstring> manual_tegs;
};
typedef std::shared_ptr<index> index_ptr;
typedef std::map<tstring, index_ptr> MapDictonaryMap;

class MapDictonary
{
	enum wordtype { name_teg = 0, manual_teg, auto_teg };
    struct position
    {
        index_ptr idx;
		wordtype type;
        int word_idx;
    };
    typedef std::vector<position> positions_vector;
    typedef std::shared_ptr<positions_vector> positions_ptr;
    struct worddata
    {
        tstring word;
        positions_ptr positions;
    };
    std::vector<worddata> m_words_table;

    tstring m_base_dir;
    database_diff_writer m_patch_file;
    lua_State *L;

    void fileerror(const tstring& file) 
    {
        tstring e(L"Ошибка чтения файла: ");
        e.append(file);
        base::log(L, e.c_str());
    }
    void filesave_error(const tstring& file)
    {
        tstring e(L"Ошибка записи файла: ");
        e.append(file);
        base::log(L, e.c_str());
    }
public:
    MapDictonary(const tstring& dir, lua_State *pl) : m_base_dir(dir), L(pl)
    {
        load_db();        
    }
    ~MapDictonary() 
    {
        tstring error;
        save_db(&error);
    }
    enum { MD_OK = 0, MD_EXIST, MD_UPDATED, MD_ERROR };

    bool update(const lua_ref& r, tstring *error)
    {
        bool result = true;
        /*
        // save current words table
        std::vector<worddata> savewt;
        savewt.swap(m_words_table);
        std::vector<DWORD> new_files_size(m_files.size(), 0);

        for (int i=0,e=m_files.size();i<e;++i)
        {
             fileinfo& fi = m_files[i];
             load_file lf(fi.path);
             if (!lf.result)
             {
                 error->assign(L"Ошибка при открытии файла: ");
                 error->append(fi.path);
                 result = false;
                 break;
             }
             if (fi.size != lf.size())
             {
                 error->assign(L"Файл изменен другой программой: ");
                 error->append(fi.path);
                 error->append(L". Перезапустите плагин.");
                 result = false;
                 break;
             }
             tstring newfile_path(fi.path);
             newfile_path.append(L".new");
             database_file_writer fw;
             if (!fw.open(newfile_path))
             {
                 error->assign(L"Ошибка при открытии файла на запись: ");
                 error->append(newfile_path);
                 result = false;
                 break;
             }
             std::string str, name;
             DataQueue data;
             while (lf.readNextString(&str, false))
             {
                 std::string t(str);
                 string_replace(&t, "\r", "");
                 string_replace(&t, "\n", "");
                 if (!t.empty())
                 {
                     if (name.empty())
                         name.assign(t);
                     else
                         data.write(str.data(), str.length());
                     continue;
                 }
                 if (name.empty())
                 {
                     data.clear();
                     continue;
                 }
                 r.pushValue(L);
                 u8string tmp((const char*)data.getData(), data.getSize());
                 lua_pushstring(L, tmp.c_str());
                 if (lua_pcall(L, 1, 1, 0))
                 {
                     error->assign(lua_toerror(L));
                     result = false;
                     break;
                 }
                 // read tegs
                 std::vector<tstring> tegs;
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

                  // write file, make index
                  Tokenizer tk(TU2W(name.c_str()), L";");
                  const MemoryBuffer &mb = data.getMemoryBuffer();
                  database_file_writer::WriteResult r;
                  if (!fw.write(tk[0], mb, tegs, &r))
                  {
                      error->assign(L"Ошибка при записи файла: ");
                      error->append(newfile_path);
                      result = false;
                      break;
                  }

                  index_ptr ix = std::make_shared<index>();
                  ix->file = i;
                  ix->name = tk[0];
                  ix->pos_in_file = r.start;
                  ix->name_tegs_len = r.title_len;
                  ix->data_len = r.data_len;

                  add_index(ix->name, ix, false, false);
                  for (int i=0,e=tegs.size();i<e;++i)
                    add_index(tegs[i], ix, true, false);

                  new_files_size[ix->file] = r.start + (r.data_len+r.title_len);

                  name.clear();
                  data.clear();
             }
        }

        if (!result)
            savewt.swap(m_words_table);

        if (result)
        {
            // переименование старых файлов
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                tstring oldname(name);
                oldname.append(L".old");
                if (!MoveFile(name.c_str(), oldname.c_str()))
                {
                    error->assign(L"Ошибка при переименовании файлов. Переименуйте их вручную.");
                    result = false;
                    break;
                }
            }
        }

        if (result)
        {
            // переименование новых файлов
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                tstring newname(name);
                newname.append(L".new");
                if (!MoveFile(newname.c_str(), name.c_str()))
                {
                    error->assign(L"Ошибка при переименовании файлов. Переименуйте их вручную.");
                    result = false;
                    break;
                }
            }
        }

        if (result)
        {
             // удаление старых файлов
            for (int i=0,e=m_files.size(); i<e; ++i)
            {
                tstring name(m_files[i].path);
                name.append(L".old");
                DeleteFile(name.c_str());
                m_files[i].size = new_files_size[i];
            }

            // перечитываем ручные теги
            load_manual_tegs();
            m_manual_tegs_changed = true;
        }*/
        return result;
    }

    void wipe()
    {
	    DeleteFile(maindb_file);
		DeleteFile(patchdb_file);
        m_words_table.clear();
    }

    enum TegResult { ERR = 0, ABSENT, ADDED, REMOVED };
    TegResult teg(const tstring& name, const tstring& teg)
    {
        if (teg.empty())
            return ERR;
        index_ptr ix = find_by_name(name);
        if (!ix)
            return ABSENT;
        int index = find_manual_teg(ix, teg);
        if (index != -1)
        {
            if (!m_patch_file.write_delete_teg(name, teg))
                return ERR;
            std::vector<tstring>& tegs = ix->manual_tegs;
            tegs.erase(tegs.begin()+index);
            Phrase p(teg);
            for (int i=0,e=p.len();i<e;++i)
            {
                int pos = 0;
                if (find_pos(p.get(i), &pos))
                {
                    positions_vector& pv = *m_words_table[pos].positions;
                    for (int i=0,e=pv.size();i<e;++i) {
                      if (pv[i].idx == ix && pv[i].word_idx == i && pv[i].type == manual_teg){
                          pv.erase(pv.begin()+i); break;
                      }
                    }
                }
            }
            return REMOVED;
        }
        if (!m_patch_file.write_teg(name, teg))
            return ERR;
        add_index(teg, ix, true, true);        
        ix->manual_tegs.push_back(teg);
        return ADDED;
    }

    enum InfoResult { INFO_ERR = 0, INFO_ABSENT, INFO_ADDED, INFO_REMOVED };
    InfoResult info(const tstring& name, const tstring& data) 
    {
        if (name.empty())
            return INFO_ERR;
        index_ptr ix = find_by_name(name);
        if (!ix)
            return INFO_ABSENT;
        tstring text(data);
        tstring_trim(&text);
        if (text.empty()) {
            if (!m_patch_file.write_delete_info(name))
                return INFO_ERR;
        }
        else {
            if (!m_patch_file.write_info(name, text))
                return INFO_ERR;
        }
        ix->comment = text;
        if (text.empty())
            return INFO_REMOVED;
        return INFO_ADDED;
    }

    int add(const tstring& name, const tstring& data, const std::vector<tstring>& tegs)
    {
        if (name.empty() || data.empty())
            return MD_ERROR;
        index_ptr ix = find_by_name(name);
        if (ix) 
        {
            if (ix->data == data && ix->auto_tegs == tegs)
                return MD_EXIST;
            if (!m_patch_file.write_object(name, data, tegs))
                return MD_ERROR;
            create_object(name, data, tegs, ix->manual_tegs);
            return MD_UPDATED;
        }
        if (!m_patch_file.write_object(name, data, tegs))
            return MD_ERROR;
        std::vector<tstring> empty;
        create_object(name, data, tegs, empty);
        return MD_OK;
    }
    
    void create_object(const tstring& name, const tstring& data, const std::vector<tstring>& autotegs, const std::vector<tstring>& manualtegs)
    {
        del_object(name);
        index_ptr ix = std::make_shared<index>();
        ix->name = name;
        ix->data = data;
        ix->auto_tegs = autotegs;
        ix->manual_tegs = manualtegs;
        add_index(name, ix, false, false);
        for (const tstring& t : autotegs)
            add_index(t, ix, true, false);
        for (const tstring& t : manualtegs)
            add_index(t, ix, true, true);
    }

    void del_index(index_ptr ix, const tstring word)
    {
        Phrase p(word);
        for (int j = 0, je = p.len(); j < je; ++j)
        {
            int pos = 0;
            if (find_pos(p.get(j), &pos))
            {
                positions_vector& pv = *m_words_table[pos].positions;
                for (int i = 0, e = pv.size(); i < e; ++i) {
                    if (pv[i].idx == ix){
                        pv.erase(pv.begin() + i); break;
                    }
                }
            }
        }
    }

    bool del_object(const tstring& object)
    {
         if (object.empty())
             return false;
         index_ptr ix = find_by_name(object);
         if (!ix) return true;
         del_index(ix, ix->name);
         for (const tstring& t : ix->manual_tegs)
             del_index(ix, t);
         for (const tstring& t : ix->auto_tegs)
             del_index(ix, t);
         return true;
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

        // удаляем дубликаты
        std::set<index_ptr> result;
        std::for_each(tmp.begin(), tmp.end(), [&result](position& p){ result.insert(p.idx); });

        // выгружаем данные
        std::set<index_ptr>::iterator rt = result.begin(), rt_end = result.end();
        for (; rt != rt_end; ++rt)
        {
            index_ptr ix = *rt;
            values->operator[](ix->name) = ix;
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
                    if (p.type == name_teg || p.word_idx == 0)
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
                        if (words_indexes[j].type == name_teg)
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

    index_ptr find_by_name(const tstring& name)
    {
        Phrase p(name);
        if (p.len() == 0)
            return nullptr;
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
        return nullptr;
    }

    void add_index(const tstring& word, index_ptr ix, bool teg, bool manual)
    {
       Phrase p(word);
       if (p.len()==0)
           return;
       for (int i=0,len=p.len(); i<len; ++i)
       {
          position word_pos;
          word_pos.type = name_teg;
          if (teg) word_pos.type = (manual) ? manual_teg : auto_teg;
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

    int find_manual_teg(index_ptr ix, const tstring& teg)
    {
        tstring t(teg);
        tstring_tolower(&t);
        std::vector<tstring>& tegs = ix->manual_tegs;
        std::vector<tstring>::iterator it = std::find_if(tegs.begin(), tegs.end(), [&](const tstring& s) 
            { tstring t1(s); tstring_tolower(&t1); return (t1 == t); } );
        if (it != tegs.end())
            return (it - tegs.begin());
        return -1;
    }

	void load_old_db()
	{

	}

    void load_db()
    {
       tstring error;
       //load_maindb(&error);

       tstring pf(m_base_dir);
       pf.append(patchdb_file);
       if (load_patchdb(pf))
       {
           save_db(&error);
           if (!error.empty())
               return;
       }
       //DeleteFile(pf.c_str());
       if (!m_patch_file.init(pf))
       {
           error = L"patch file error";
       }
    }

    void load_maindb()
    {
        return;
        tstring mainfile(m_base_dir);
        mainfile.append(maindb_file);

        load_file lf(mainfile);
        if (!lf.result)
            return;

        u8string str, name;
        bool find_name_mode = true;
        index_ptr ix = std::make_shared<index>();
        while (lf.readNextString(&str, true))
        {
            if (str.empty())
            {
                find_name_mode = true;
                if (!name.empty())
                {
                    Tokenizer tk(TU2W(name.c_str()), L";");
                    ix->name = tk[0];
                    {
                        index_ptr copy = find_by_name(tk[0]);
                        if (!copy) {
                          for (int i=0,e=tk.size();i<e;++i)
                             add_index(tk[i], ix, (i!=0), false);
                          }
                        }
                        ix = std::make_shared<index>();
                        name.clear();
                    }
                    continue;
                }
                if (find_name_mode)
                {
                    name.assign(str);
                    find_name_mode = false;
                }
        }
    }

    bool load_patchdb(const tstring& path)
    {
        load_file pf(path);
        if (!pf.result) 
            return false;

        bool reading_object = false;
        std::vector<tstring> object_data;
        u8string str;
        while (pf.readNextString(&str, true))
        {
            if (str.empty()) 
            {
                if (reading_object)
                {
                    Tokenizer tk(object_data[0].c_str(), L";");
                    tstring data;
                    for (int i=1,e=object_data.size(); i<e; ++i) {
                        if (i != 1)
                            data.append(L"\r\n");
                        data.append(object_data[i]);
                    }
                    std::vector<tstring> autotegs, manualtegs;
                    for (int i=1,e=tk.size();i<e;++i) {
                        autotegs.push_back(tk[i]);
                    }
                    create_object(tk[0], data, autotegs, manualtegs);
                    object_data.clear();
                    reading_object = false;
                }
                continue;
            }
            tstring s(TU2W(str.c_str()));
            if (reading_object)
            {
                object_data.push_back(s);
                continue;
            }
            if (str.size() < 2 || s[1] != L';')
                continue;
            tchar t = s[0];
            tstring data(s.substr(2));
            if (t == L'O')
            {
                object_data.push_back(data);
                reading_object = true;
                continue;
            }
            tstring objectname(data), subdata;
            size_t pos = data.find(L';');
            if (pos != tstring::npos)
            {
                objectname = data.substr(0, pos);
                subdata = data.substr(pos+1);
            }
            index_ptr ix = find_by_name(objectname);
            if (!ix)
                continue;
            if (t == L'i')  //delete comment
            {
                ix->comment.clear();
                continue;
            }
            if (t == L'T')  //add teg
            {
                add_index(subdata, ix, true, true);
                if (find_manual_teg(ix, subdata) == -1) {
                    ix->manual_tegs.push_back(subdata);
                }
            }
            if (t == L't')  //delete teg
            {
                del_index(ix, subdata);
                int index = find_manual_teg(ix, subdata);
                if (index != -1) {
                    std::vector<tstring>& tegs = ix->manual_tegs;
                    tegs.erase(tegs.begin()+index);
                }
            }
            if (t == L'I')  //add comment
            {
                ix->comment = subdata;
            }
        }
        return true;
    }

    void save_db(tstring *error)
    {
        std::set<index_ptr> setix;
        typedef std::vector<worddata>::iterator words_table_iterator;
        words_table_iterator it = m_words_table.begin(), it_end = m_words_table.end();
        for (; it!=it_end; ++it)
        {
            positions_ptr ptr  = it->positions;
            positions_vector& pv = *ptr;
            for (const position& p : pv)
                setix.insert(p.idx);
        }

        tstring mainfile(m_base_dir);
        mainfile.append(maindb_file);
        filewriter fw;
        if (!fw.open(mainfile))
        {
            error->assign(L"Ошибка! Не получилось записать файл базы.");
            return;
        }
        for (index_ptr ix : setix)
        {
            tstring out(ix->name); out.append(L"\n");
            if (!ix->auto_tegs.empty())
            {
                out.append(L"A");
                for (const tstring& t : ix->auto_tegs)
                {
                    out.append(L";");
                    out.append(t);
                }
                out.append(L"\n");
            }
            if (!ix->manual_tegs.empty())
            {
                out.append(L"T");
                for (const tstring& t : ix->manual_tegs)
                {
                    out.append(L";");
                    out.append(t);
                }
                out.append(L"\n");
            }
            if (!ix->comment.empty()) 
            {
                out.append(L"C;");
                out.append(ix->comment);
                out.append(L"\n");
            }
            out.append(ix->data);
            out.append(L"\n\n");
            if (!fw.write(out))
            {
                error->assign(L"Ошибка! Ошибка при записи файла базы.");
                return;
            }
        }
    }

    /*void load_manual_tegs()
    {
        tstring path(m_base_dir);
        path.append(L"usertegs.db");
        load_file fr(path);
        if (fr.file_missed)
            return;
        if (!fr.result)
        {
            tstring error(L"Ошибка при открытии файла: ");
            error.append(path.c_str());
            fileerror( error.c_str() );
            return;
        }
        std::string s;
        while (fr.readNextString(&s, true))
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
    }*/
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
        if (result == MapDictonary::MD_EXIST)
        {
            lua_pushstring(L, "exist");            
        }
        else
        {
            lua_pushstring(L, result == MapDictonary::MD_UPDATED ? "updated" : "error");
        }
        return 2;
    }
    return dict_invalidargs(L, "add");
}

int dict_delete(lua_State *L)
{
    if (luaT_check(L, 2, get_dict(L), LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring id(luaT_towstring(L, 2));
        bool result = d->del_object(id);
        lua_pushboolean(L, result?1:0);
        return 1;
    }
    return dict_invalidargs(L, "delete");
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

                index_ptr ix = it->second;
                lua_pushstring(L, "data");
                lua_pushwstring(L, ix->data.c_str());
                lua_settable(L, -3);
                if (!ix->comment.empty()) {
                    lua_pushstring(L, "comment");
                    lua_pushwstring(L, ix->comment.c_str());
                    lua_settable(L, -3);
                }
                const std::vector<tstring>& at = ix->auto_tegs;
                lua_pushstring(L, "auto");
                lua_newtable(L);
                for (int i=0,e=at.size();i<e;++i)
                {
                    lua_pushinteger(L, i+1);
                    luaT_pushwstring(L, at[i].c_str());
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);

                const std::vector<tstring>& mt = ix->manual_tegs;
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
        case MapDictonary::ERR:
            lua_pushstring(L, "invalid args");
            break;
        case MapDictonary::ABSENT:
            lua_pushstring(L, "absent");
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

int dict_comment(lua_State *L)
{
    if (luaT_check(L, 3, get_dict(L), LUA_TSTRING, LUA_TSTRING))
    {
        MapDictonary *d = (MapDictonary*)luaT_toobject(L, 1);
        tstring name(luaT_towstring(L, 2));
        tstring info(luaT_towstring(L, 3));
        MapDictonary::InfoResult result = d->info(name, info);
        switch (result) {
        case MapDictonary::INFO_ERR:
            lua_pushstring(L, "invalid args");
            break;
        case MapDictonary::INFO_ABSENT:
            lua_pushstring(L, "absent");
            break;
        case MapDictonary::INFO_ADDED:
            lua_pushstring(L, "added");
            break;
        case MapDictonary::INFO_REMOVED:
            lua_pushstring(L, "removed");
            break;
        default:
            lua_pushnil(L);
        }
        return 1;
    }
    return dict_invalidargs(L, "comment");
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
        regFunction(L, "delete", dict_delete);
        regFunction(L, "update", dict_update);
        regFunction(L, "wipe", dict_wipe);
        regFunction(L, "teg", dict_teg);
        regFunction(L, "comment", dict_comment);
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
