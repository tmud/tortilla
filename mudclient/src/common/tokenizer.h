#pragma once

class Tokenizer
{
public:
    Tokenizer(const tchar* string, const tchar* tokens)
    {
        if (!string || !tokens) return;
        const tchar *p = string;
        const tchar *e = p + _tcslen(string);
        while (p < e)
        {
            size_t len = _tcscspn(p, tokens);
            parts.push_back(tstring(p, len));
            p = p + len + 1;
        }
    }
    bool empty() const { return parts.empty(); }
    int  size() const { return parts.size(); }
    const tchar* operator[](int index) { return parts[index].c_str(); }
    void moveto(std::vector<tstring> *v) { v->swap(parts); parts.clear(); }
    void trimempty()
    {
        std::vector<tstring> newparts;
        for (int i=0,e=parts.size();i<e;++i)
        {
          if (!parts[i].empty())
             newparts.push_back(parts[i]);
        }
        parts.swap(newparts);
    }
    void swap(std::vector<tstring>& sa)
    {
        parts.swap(sa);
    }
private:
    std::vector<tstring> parts;
};
