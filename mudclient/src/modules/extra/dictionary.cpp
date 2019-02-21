#include "stdafx.h"
#include "phrase.h"
#include <memory>

void regFunction(lua_State *L, const char* name, lua_CFunction f);

const tchar* maindb_file = L"main.ldb";
const tchar* patchdb_file = L"patch.ldb";

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
	~filewriter() { close(); }
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
    bool truncate()
    {
        if (hfile == INVALID_HANDLE_VALUE)
			return false;
        if (SetFilePointer(hfile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return false;
        return SetEndOfFile(hfile) ? true : false;
    }
	bool write(const tstring& data)
    {
		u8string tmp(TW2U(data.c_str()));
		return write(tmp);
    }
    void close()
    {
        if (hfile != INVALID_HANDLE_VALUE)
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
    void delfile() {
        close();
        DeleteFile(filepath.c_str());
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
    bool writed;
    bool write(const tstring& t)
    {
        bool result = fw.write(t);
        if (result) writed = true;
        return result;
    }
public:
    database_diff_writer() : writed(true) {}
    ~database_diff_writer() 
    {
        if (!writed) {
            fw.delfile();
        }
    }
    bool truncate()
    {
        return fw.truncate();
    }
    void close()
    {
        fw.close();
    }
    bool init(const tstring& filepath)
    {
        bool result = (fw.open(filepath) && truncate());
        if (result)
            writed = false;
        return result;
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
        return write(out);
    }
    bool write_delete_object(const tstring& name) 
    {
        tstring out(L"o;"); out.append(name);
        out.append(L"\n");
        return write(out);
    }
    bool write_teg(const tstring& name, const tstring& teg)
    {
        tstring out(L"T;"); out.append(name);
        out.append(L";"); out.append(teg);
        out.append(L"\n");
        return write(out);
    }
    bool write_delete_teg(const tstring& name, const tstring& teg)
    {
        tstring out(L"t;"); out.append(name);
        out.append(L";"); out.append(teg);
        out.append(L"\n");
        return write(out);
    }
    bool write_info(const tstring& name, const tstring& info)
    {
        tstring out(L"I;"); out.append(name);
        out.append(L";"); out.append(info);
        out.append(L"\n");
        return write(out);
    }
    bool write_delete_info(const tstring& name)
    {
        tstring out(L"i;"); out.append(name);
        out.append(L"\n");
        return write(out);
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
public:
    MapDictonary(const tstring& dir, lua_State *pl) : m_base_dir(dir), L(pl)
    {
    }
    ~MapDictonary() 
    {
    }
    enum { MD_OK = 0, MD_EXIST, MD_UPDATED, MD_ERROR };
    void init(tstring *error)
    {
        load_db(error);
    }

    bool update(const lua_ref& r, tstring *error)
    {
        if (!m_patch_file.truncate())
        {
            error->assign(L"Не удалось сбросить патч файл.");
            return false;
        }

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

        for (index_ptr ix : setix)
        {
            std::vector<tstring> newtegs;
            Tokenizer tk(ix->data.c_str(), L"\n");
            for (int i=0,e=tk.size(); i<e; ++i)
            {
                r.pushValue(L);
                luaT_pushwstring(L, tk[i]);
                if (lua_pcall(L, 1, 1, 0))
                {
                     error->assign(lua_toerror(L));
                     return false;
                }
                if (lua_isstring(L, -1)) {
                    tstring newteg(luaT_towstring(L, -1));
                    newtegs.push_back(newteg);
                }
                if (lua_istable(L, -1))
                {
                     lua_pushnil(L);                     // first key
                     while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
                     {
                         if (lua_isstring(L, -1))
                         {
                             tstring teg(luaT_towstring(L, -1));
                             newtegs.push_back(teg);
                         }
                         lua_pop(L, 1);
                     }
                 }
                 lua_pop(L, 1);
            }
            create_object(ix->name, ix->data, newtegs, ix->manual_tegs);
        }
        save_db(error);
        return error->empty();
    }

    void wipe()
    {
        m_patch_file.close();
        tstring mf(m_base_dir); mf.append(maindb_file);
        tstring pf(m_base_dir); pf.append(patchdb_file);
	    DeleteFile(mf.c_str());
		DeleteFile(pf.c_str());
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
            if (ix->data == data && ix->auto_tegs.size() == tegs.size())
            {
                bool equal = true;
                for (int i=0,e=tegs.size(); i<e; ++i) {
                    if (ix->auto_tegs[i] != tegs[i]) { equal = false; break; }
                }
                if (equal)
                    return MD_EXIST;
            }
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

    void create_index(index_ptr ix)
    {
        del_object(ix->name);
        add_index(ix->name, ix, false, false);
        for (const tstring& t : ix->auto_tegs)
            add_index(t, ix, true, false);
        for (const tstring& t : ix->manual_tegs)
            add_index(t, ix, true, true);
    }

    void create_object(const tstring& name, const tstring& data, const std::vector<tstring>& autotegs, const std::vector<tstring>& manualtegs)
    {
        index_ptr ix = std::make_shared<index>();
        ix->name = name;
        ix->data = data;
        ix->auto_tegs = autotegs;
        ix->manual_tegs = manualtegs;
        create_index(ix);
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

    void load_db(tstring* error)
    {
       load_old_db(error);
       if (!error->empty())
            return;
       load_maindb(error);
       if (!error->empty())
            return;
       tstring pf(m_base_dir);
       pf.append(patchdb_file);
       if (load_patchdb(pf, error))
       {
           save_db(error);
       }
       if (!error->empty())
           return;
       if (!m_patch_file.init(pf))
       {
           error->assign(L"patch file не смог создаться.");
           return;
       }
       delete_old_db();
    }

    void load_maindb(tstring *error)
    {
        tstring mainfile(m_base_dir);
        mainfile.append(maindb_file);
        load_file lf(mainfile);
        if (lf.file_missed)
            return;
        if (!lf.result)
        {
            error->assign(L"Не загружается основной файл базы.");
            return;
        }

        std::vector<tstring> data;
        u8string str;
        while (lf.readNextString(&str, true))
        {
            tstring s(TU2W(str.c_str()));
            if (!s.empty())
            {
                data.push_back(s);
                continue;
            }
            if (data.empty())
                continue;
            // processing object
            if (data[0].find(L';') != tstring::npos)
                continue;
            index_ptr ix = std::make_shared<index>();
            ix->name = data[0];
            for (int i=1,e=data.size();i<e;++i)
            {
                const tstring &t = data[i];
                if (t.length() < 2) continue;
                tchar tp = t[0];
                if (t[1] == L';' && (tp == L'A' || tp == L'T')) {
                    Tokenizer tk(t.substr(2).c_str(), L";");
                    if (tp == L'A')
                        tk.moveto(&ix->auto_tegs);
                    else
                        tk.moveto(&ix->manual_tegs);
                    continue;
                }
                if (t[1] == L';' && tp == L'C')
                {
                    ix->comment = t.substr(2);
                    continue;
                }
                tstring& data = ix->data;
                if (!data.empty()) {
                    data.append(L"\n");
                }
                data.append(t);
            }
            data.clear();
            create_index(ix);
        }
    }

    bool load_patchdb(const tstring& path, tstring *error)
    {
        load_file pf(path);
        if (pf.file_missed) 
            return true;
        if (!pf.result)
        {
            error->assign(L"Не загрузился патч для базы.");
            return false;
        }

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
                            data.append(L"\n");
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
        if (!fw.open(mainfile) || !fw.truncate())
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

    void load_old_db(tstring *error)
	{
        std::vector<tstring> files;
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
                    tstring name(fd.cFileName);
                    int len = name.length()-3;
                    name = name.substr(0,len);
                    if (!isOnlyDigits(name))
                        continue;
                    files.push_back(fd.cFileName);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
        std::sort(files.begin(), files.end(),[](const tstring&a, const tstring&b) {
            return a < b;
        });
        std::vector<tstring> empty;
        for (const tstring& f : files)
        {
            // read files in to catalog
            tstring path(m_base_dir);
            path.append(f);
            load_file lf(path);
            if (!lf.result) {
                error->assign(L"Ошибка при загрузке файла базы: " + f);
                return;
            }

            std::vector<tstring> data;
            u8string str;
            bool find_name_mode = false;
            while (lf.readNextString(&str, true))
            {
                tstring s(TU2W(str.c_str()));
                if (!s.empty()) {
                    data.push_back(s);
                    continue;
                }                
                if (data.empty())
                    continue;
                // process object
                Tokenizer tk(data[0].c_str(), L";");
                std::vector<tstring> tegs;
                tk.moveto(&tegs);
                tstring name(tegs[0]); tegs.erase(tegs.begin());
                tstring content;
                for (int i=1,e=data.size();i<e;++i) {
                    if (i != 1)
                        content.append(L"\n");
                    content.append(data[i]);
                }
                create_object(name, content, tegs, empty);
                find_name_mode = true;
                data.clear();
            }
        }
        tstring path(m_base_dir);
        path.append(L"usertegs.db");
        load_file fr(path);
        if (fr.file_missed)
            return;
        if (!fr.result)
        {
            error->assign(L"Ошибка при открытии файла: " + path);
            return;
        }
        u8string us;
        while (fr.readNextString(&us, true))
        {
            tstring s(TU2W(us.c_str()));
            Tokenizer tk(s.c_str(), L";");
            if (tk.size() < 2) continue;
            const tstring& name = tk[0];
            index_ptr ix = find_by_name(name);
            if (!ix)
                continue;
            for (int i=1,e=tk.size();i<e;++i)
            {
                const tstring& teg = tk[i];
                add_index(teg, ix, true, true);
                if (find_manual_teg(ix, teg) == -1) {
                    ix->manual_tegs.push_back(teg);
                }
            }
        }
    }

    void delete_old_db() 
    {
        std::vector<tstring> files;
        tstring mask(m_base_dir);
        mask.append(L"*.db");
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(mask.c_str(), &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do 
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    files.push_back(fd.cFileName);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
        for (const tstring& f : files)
        {
            tstring path(m_base_dir);
            path.append(f);
            DeleteFile(path.c_str());
        }   
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
    tstring error;
    nd->init(&error);
    if (!error.empty()) {
        luaT_pushwstring(L, error.c_str());
        return lua_error(L);
    }
    luaT_pushobject(L, nd, get_dict(L));
    return 1;
}
