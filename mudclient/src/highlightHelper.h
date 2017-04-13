#pragma once

class HighlightHelperImpl
{
    Pcre16 pcre_colors;
    Pcre16 pcre_rgb;
    Pcre16 pcre_prefix;
    tstring colors;

public:
    HighlightHelperImpl()
    {
        colors.assign(L"(\\bblack\\b|\\bred\\b|\\bgreen\\b|\\bbrown\\b|\\bblue\\b|\\bmagenta\\b|\\bcyan\\b|\\bgray\\b|\\bcoal\\b|\\blight red\\b|\\blight green\\b|\\byellow\\b|\\blight blue\\b|\\bpurple\\b|\\blight cyan\\b|\\bwhite\\b|\\blight magenta\\b|\\blight brown\\b|\\bgrey\\b|\\bcharcoal\\b|\\blight yellow\\b)");
        pcre_colors.setRegExp(colors, true);
        pcre_rgb.setRegExp(L"rgb([0-9]+,[0-9]+,[0-9]+)");
        pcre_prefix.setRegExp(L"^border|line|italic|b$");
    }

    bool checkText(tstring* param)
    {
        tstring tmp(*param);
        tstring_tolower(&tmp);
        preprocessColors(tmp);

        Separator s(tmp);
        int size = s.getSize();
        if (!size)
            return false;

        std::vector<tstring> parts(size);
        for (int i=0; i<size; ++i)
        {
            tstring tmp(s[i]);
            if (checkPrefix(tmp) || checkColors(&tmp))
                parts[i] = tmp;
            else
                return false;
        }

        bool border = false; bool line = false; bool italic = false;
        tstring text_color, bkg_color;

        for (int i=0; i<size; ++i)
        {
            const tstring& s = parts[i];
            if (s == L"border")
                border = true;
            if (s == L"line")
                line = true;
            if (s == L"italic")
                italic = true;

            if (checkPrefix(s))
                continue;

            if (i != 0 && parts[i-1] == L"b")
            {
                if (!bkg_color.empty())
                    return false;
                bkg_color = s;
                continue;
            }
            if (!text_color.empty())
                return false;
            text_color = s;
        }

        param->clear();
        if (!text_color.empty())
        {
            param->append(L"txt[");
            param->append(text_color);
            param->append(L"],");
        }
        if (!bkg_color.empty())
        {
            param->append(L"bkg[");
            param->append(bkg_color);
            param->append(L"],");
        }
        param->append(L"ubi[");
        param->append(line ? L"1" : L"0");
        param->append(L",");
        param->append(border ? L"1" : L"0");
        param->append(L",");
        param->append(italic ? L"1" : L"0");
        param->append(L"]");
        return true;
    }
private:
    void preprocessColors(tstring& str)
    {
        Pcre16 &p = pcre_colors;
        p.findAllMatches(str);
        if (p.getSize() == 0)
           return;
        WCHAR* table[16] = { L"64,64,64", L"128,0,0", L"0,128,0", L"128,128,0", L"0,64,128", L"128,0,128", L"0,128,128", L"192,192,192",
           L"128,128,128", L"255,0,0", L"0,255,0", L"255,255,0", L"0,128,255", L"255,0,255", L"0,255,255", L"255,255,255" };
        tstring name;
        for (int i=p.getSize()-1; i>=1; --i)
        {
            p.getString(i, &name);
            int pos = colors.find(name);
            int colorid = 0;
            for (int j = 0; j < pos; ++j)
            {
               if (colors.at(j) == L'|') 
                   colorid++;
            }
            if (colorid == 16) colorid = 13; // light magenta -> purple
            if (colorid == 17) colorid = 11; // light brown -> yellow
            if (colorid == 18) colorid = 7;  // grey -> gray
            if (colorid == 19) colorid = 8;  // charcoal -> coal
            if (colorid == 20) colorid = 11; // light yellow -> yellow

            tstring tmp(str.substr(0,p.getFirst(i)));
            tmp.append(L"rgb");
            tmp.append(table[colorid]);
            tmp.append(str.substr(p.getLast(i)));
            str = tmp;
        }
     }

     bool checkColors(tstring* str)
     {
        pcre_rgb.find(*str);
        if (pcre_rgb.getSize() == 0)
            return false;
        tchar e = 0;
        int first = pcre_rgb.getFirst(1);
        if (first != 3)
        {
            if (first != 4)
              return false;
            tchar b = str->at(0);
            if (b == L'{') { e = L'}'; }
            else if (b == L'\'' || b == L'"') { e = b; }
            else { return false; }
        }
        int len = str->length();
        int last = pcre_rgb.getLast(1);
        if (len == last) 
        {
            if (e != 0)
                return false;
        }
        else
        {
            len = len - 1;
            if (len == last && e != 0) 
            {
                tchar s = str->at(last);
                if (s != e)
                    return false;
            }
            else
            {
                return false;
            }
        }
        pcre_rgb.getString(1, str);
        return true;
     }

     bool checkPrefix(const tstring& str)
     {
         Pcre16 &p = pcre_prefix;
         p.find(str);
         if (p.getSize() != 1)
             return false;
         int len = str.length();
         return (p.getFirst(0) == 0 && p.getLast(0) == len) ? true : false;
     }
};

class HighlightHelper
{
   static HighlightHelperImpl m_impl;
public:
   bool checkText(tstring* param);
};
