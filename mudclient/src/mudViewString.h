#pragma once

struct MudViewStringParams
{
    MudViewStringParams() { reset(); }
    void reset() 
    {
        text_color = 7;
        bkg_color = 0;
        intensive_status = 0;
        underline_status = 0;
        italic_status = 0;
        blink_status = 0;
        reverse_video = 0;
        use_ext_colors = 0;
        ext_text_color = 0;
        ext_bkg_color = 0;
    }

    bool operator==(const MudViewStringParams& p)
    {
        if (intensive_status == p.intensive_status &&
            underline_status == p.underline_status &&
            italic_status == p.italic_status &&
            blink_status == p.blink_status &&
            reverse_video == p.reverse_video &&
            use_ext_colors == p.use_ext_colors
            )
        {
            if (!use_ext_colors)
            {
                return (text_color == p.text_color &&
                    bkg_color == p.bkg_color) ? true : false;
            }
            if (ext_text_color == p.ext_text_color &&
                ext_bkg_color == p.ext_bkg_color)
                return true;
        }
        return false;
    }

    tbyte text_color;              // support 256 colors (0-15 - from palette)
    tbyte bkg_color;               // 256 colors
    tbyte intensive_status;
    tbyte underline_status;
    tbyte italic_status;
    tbyte blink_status;
    tbyte reverse_video;
    tbyte use_ext_colors;          // flag for use extended colors
    COLORREF ext_text_color;       // extended colors (for highlights)
    COLORREF ext_bkg_color;
};

struct MudViewStringBlock
{
   MudViewStringBlock() { size.cx = 0; size.cy = 0; subs_protected = 0; }
   tstring string;                        // text string
   MudViewStringParams params;            // string params
   SIZE size;                             // size of string (DC size of current font)
   tbyte subs_protected;                  // protected from subs
};
typedef std::vector<MudViewStringBlock> MudViewStringBlocks;

struct MudViewString
{
   MudViewString() : dropped(false), gamecmd(false), system(false), triggered(false), prompt(0), next(false), prev(false), changed(true) {}
   void moveBlocks(MudViewString* src) 
   {
       blocks.insert(blocks.end(), src->blocks.begin(), src->blocks.end());
       gamecmd |= src->gamecmd;
       system |= src->system;
       triggered |= src->triggered;
       if (prompt || src->prompt)
            prompt = getTextLen();
       src->clear();
   }

   void clear()
   {
       blocks.clear();
       dropped = false;
       gamecmd = false;
       system = false;
       triggered = false;
       prompt = 0;
   }

   void copy(MudViewString* src)
   {
       blocks.assign(src->blocks.begin(), src->blocks.end());
       dropped = false;
       gamecmd = src->gamecmd;
       system = src->system;
       triggered = src->triggered;
       prompt = src->prompt;
   }

   void getText(tstring *text) const
   {
       text->clear();
       for (int i=0,e=blocks.size(); i<e; ++i) {
           text->append(blocks[i].string);
       }
   }

   int getTextLen() const
   {
       int size = 0;
       for (int i=0, e=blocks.size(); i<e; ++i)
           size += blocks[i].string.size();
       return size;
   }

   void setPrompt(int index = -1)
   {
       if (index == -1)
           prompt = getTextLen();
       else
           prompt = index;
   }

   void getPrompt(tstring *text) const
   {
       if (prompt <= 0)
           return;
       tstring tmp;
       getText(&tmp);
       text->assign(tmp.substr(0, prompt));
   }

   void getMd5(tstring *crc) const
   {
       MD5 md5;
       for (int i=0,e=blocks.size(); i<e; ++i)
           md5.update(blocks[i].string);
       crc->assign(md5.getCRC());
   }

   MudViewString* divideString(int pos)
   {
       MudViewString *s = new MudViewString;
       int begin = 0; int index = -1;
       for (int i = 0, e = blocks.size(); i < e; ++i)
       {
            int end = begin + blocks[i].string.size();
            if (pos >= begin && pos < end) { index = i; break; }
            begin = end;
       }
       if (index != -1)
       {
           MudViewStringBlock &c = blocks[index];
           MudViewStringBlock newc;
           newc.params = c.params;
           newc.string = c.string.substr(pos - begin);
           c.string = c.string.substr(0, pos - begin);
           s->blocks.push_back(newc);
           for (int i = index + 1, e = blocks.size(); i < e; ++i)
               s->blocks.push_back(blocks[i]);
           blocks.erase(blocks.begin()+index+1, blocks.end());
       }
       return s;
   }

   MudViewStringBlocks blocks;              // all string blocks
   bool dropped;                            // flag for dropping string from view
   bool gamecmd;                            // flag - game cmd
   bool system;                             // flag - system cmd / log
   bool triggered;                          // flag - string triggered, after that string can insert another strings
   int  prompt;                             // prompt-string, index of last symbol of prompt
   bool next;                               // flag - next string in MudView is part of that string
   bool prev;                               // flag - prev string in MudView is part of that string
   bool changed;                            // flag - to recalc string dc (changed in triggers)
};

typedef std::vector<MudViewString*> mudViewStrings;
