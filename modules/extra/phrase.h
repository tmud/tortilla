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
              /*if (delta > 2)
                  { result = false; break; }*/
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
    PhrasesList() {}
    ~PhrasesList() { std::for_each(m_phrases.begin(), m_phrases.end(), [](Phrase*p) { delete p; }); }
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
        int index = find(p);
        if (index != -1)
        {   m_phrases[index]->getFullPhrase(result);
            return true;
        }
        return false;
    }
    bool deletePhrase(const Phrase& p)
    {
        int index = find(p);
        if (index != -1)
        {
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
private:
    int find(const Phrase& p)
    {
        int begin = -1;
        tchar c = p.get(0).at(0);
        for (int i=0,e=m_phrases.size();i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c == c2) { begin = i; break; }
        }
        if (begin == -1) return -1;
        int end = m_phrases.size();
        for (int i=begin,e=m_phrases.size();i<e;++i)
        {
            tchar c2 = m_phrases[i]->get(0).at(0);
            if (c != c2) { end = i; break; }
        }
        for (int i=begin;i<end;++i)
        {
            if (m_phrases[i]->similar(p))
                return i;
        }
        return -1;
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
    bool findPhrase(const tstring& t, tstring* result) const
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
    bool deletePhrase(const tstring& t)
    {
        Phrase p(t);
        int len = p.len();
        if (len == 0)
            return false;
        iterator it = m_data.find(len);
        if (it == m_data.end())
            return false;
        return it->second->deletePhrase(p);
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
