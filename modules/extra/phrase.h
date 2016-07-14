#pragma once
#include "../mudclient/src/common/tokenizer.h"

class Phrase
{
public:
    Phrase(tstring p)
    {
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
              int compared = 0;
              for (int i=0;i<len;++i) {
                  if (p1[i] != p2[i]) break;
                  compared++;
              }
              int notcompared = len - compared;
              if (notcompared > compared)
                  { result = false; break; }
              if (notcompared)
              {
                  notcompared = notcompared + delta;
                  if (notcompared >= compared*2) {
                      result = false; break;
                  }
              }
        }
        return result;
    }
    bool strong(const Phrase& p) const 
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
              if (p1.size() < p2.size())
                  { result = false; break; }
              int len = p2.size();
              if (p1.compare(0, p2.size(), p2))
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
    PhrasesList() {}
    ~PhrasesList() { clear(); }
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

       int dsize = m_phrases.capacity() - m_phrases.size();
       if (dsize == 0)
           m_phrases.reserve(m_phrases.size()+1000);
       m_phrases.insert(m_phrases.begin()+index, p);
       return true;
    }
    bool findPhrase(const Phrase& p, bool strong_mode, std::vector<tstring>* result)
    {
        std::vector<int> indexes;
        find(p, strong_mode, indexes);
        if (indexes.empty())
            return false;
        result->resize(indexes.size());
        for (int i=0,e=indexes.size();i<e;++i)
        {
            int index = indexes[i];
            m_phrases[index]->getFullPhrase( &result->operator[](i) );
        }
        return true;
    }
    bool deletePhrase(const Phrase& p, bool strong_mode)
    {
        std::vector<int> indexes;
        find(p, strong_mode, indexes);
        if (indexes.size() == 1)
        {
            int index = indexes[0];
            m_phrases.erase(m_phrases.begin()+index);
            return true;
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
    void clear()
    {
        std::for_each(m_phrases.begin(), m_phrases.end(), [](Phrase*p) { delete p; });
        m_phrases.clear();
    }
private:
    void find(const Phrase& p, bool strong_mode, std::vector<int>& indexes)
    {
        if (p.len() == 0)
            return;
        int begin = -1;
        tchar c = p.get(0).at(0);
        for (int i=0,e=m_phrases.size();i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c == c2) { begin = i; break; }
        }
        if (begin == -1) return;
        int end = m_phrases.size();
        for (int i=begin,e=end;i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c != c2) { end = i; break; }
        }
        for (int i=begin;i<end;++i)
        {
            bool result = (strong_mode) ? m_phrases[i]->strong(p) : m_phrases[i]->similar(p);
            if (result)
                indexes.push_back(i);
        }
    }
    std::vector<Phrase*> m_phrases;
};

class Dictonary
{
public:
    Dictonary() : m_changed(false) {}
    ~Dictonary() { clear(); }
    bool addPhrase(const tstring& t)
    {
        Phrase *p = new Phrase(t);
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
    bool findPhrase(const tstring& t, std::vector<tstring>* result) const
    {
        Phrase p(t);
        int len = p.len();
        if (len == 0)
            return false;
        iterator it = m_data.find(len);
        if (it == m_data.end())
            return false;
        it->second->findPhrase(p, false, result);
        return !result->empty();
    }
    bool deletePhrase(const tstring& t)
    {
        Phrase p(t);
        int len = p.len();
        if (len == 0)
            return false;
        iterator it = m_data.find(len);
        if (it == m_data.end())
            return false;
        return it->second->deletePhrase(p, true);
    }
    void clear() 
    {
        std::for_each(m_data.begin(), m_data.end(), [](std::pair<int, PhrasesList*> p){ delete p.second; });
        m_data.clear();
        m_changed = true;
    }
    bool check(const tstring& t, int index) const
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
