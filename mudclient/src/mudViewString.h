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

struct MudViewString
{
   MudViewString() : dropped(false), gamecmd(false), prompt(0) {}
   void moveBlocks(MudViewString* src) 
   {
       for (int i=0,e=src->blocks.size(); i<e; ++i)
           blocks.push_back(src->blocks[i]);
       gamecmd = src->gamecmd;
       src->clear();
   }
   void clear()
   {
       blocks.clear();
       dropped = false;
       gamecmd = false;
       prompt = 0;
   }
   void copy(MudViewString* src)
   {
       clear();
       for (int i=0,e=src->blocks.size(); i<e; ++i)
           blocks.push_back(src->blocks[i]);
       gamecmd = src->gamecmd;
       prompt = src->prompt;
   }
   void getText(tstring *text) const
   {
       text->clear();
       for (int i=0,e=blocks.size(); i<e; ++i) {
           text->append(blocks[i].string);
       }
   }
   void setPrompt(int index = -1)
   {
       if (index == -1)
       {
           int size = 0;
           for (int i = 0, e = blocks.size(); i < e; ++i)
               size += blocks[i].string.size();
           prompt = size;
       }
       else
       {
           prompt = index;
       }
   }

   void getPrompt(tstring *text) const
   {
       tstring tmp;
       getText(&tmp);
       text->assign(tmp.substr(0, prompt));       
   }
   std::vector<MudViewStringBlock> blocks;  // all string blocks
   bool dropped;                            // flag for dropping string from view
   bool gamecmd;                            // flag - game cmd in string
   int  prompt;                             // prompt-string, index of last symbol of prompt
};
