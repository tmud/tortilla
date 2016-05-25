#include "stdafx.h"
#include "mudViewParser.h"

MudViewParser::MudViewParser() : m_current_string(NULL), m_last_finished(true), m_palette(NULL)
{
}

MudViewParser::~MudViewParser()
{
    delete m_current_string;
}

void MudViewParser::parse(const WCHAR* text, int len, bool newline_iacga, parseData* data, MudViewParserOscPalette *palette)
{
    m_palette = palette;

    if (!m_last_finished)
        data->update_prev_string = true;

    m_buffer.write(text, len * sizeof(WCHAR));
    while (m_buffer.getSize() > 0)
    {
        const WCHAR *b = (WCHAR*)m_buffer.getData();
        int len = m_buffer.getSize() / sizeof(WCHAR);

        if (!m_current_string)
            m_current_string = new MudViewString();

        parserResult result = process (b, len);
        parserResultCode r = result.result;

        if (r == PARSE_STRING_IACGA)
        {
            r = (newline_iacga) ? PARSE_STRING_FINISHED : PARSE_NO_ERROR;
        }

        if (r == PARSE_BLOCK_FINISHED || r == PARSE_STRING_FINISHED)
        {
            if (!m_current_block.string.empty())
            {
                m_current_string->blocks.push_back(m_current_block);
                m_current_block.string.clear();
            }
        }

        if (r == PARSE_STRING_FINISHED)
        {
            data->strings.push_back(m_current_string);
            m_current_string = NULL;
        }

        m_buffer.truncate(result.processed * sizeof(WCHAR));
        if (r == PARSE_LOW_DATA)
            break;
    }

    m_last_finished = true;
    if (m_current_string)
    {
        data->strings.push_back(m_current_string);
        m_current_string = NULL;
        m_last_finished = false;
    }
    data->last_finished = m_last_finished;
}

MudViewParser::parserResult MudViewParser::process(const WCHAR* b, int len)
{
    /*MemoryBuffer out;
    WideToAnsiConverter tmp;
    int slen = tmp.convert(&out, b, len);
    OUTPUT_BYTES(out.getData(), slen, slen, "parser");*/

    if (*b >= 0x20)
        return process_string(b, len);
    if (*b == 0x1b)
        return process_esc(b, len);
    if (*b == 0xd)
        return process_0xd(b, len);
    if (*b == 0xa)
        return process_0xa(b, len);
    // skip not supported tag
    return parserResult(PARSE_NOT_SUPPORTED, 1);
}

MudViewParser::parserResult MudViewParser::process_string(const WCHAR* b, int len)
{
    // so we can translate string
    // find - next control code
    const WCHAR* e = b;
    for (; len > 0; --len) {
        if (*e < 0x20)
            break;
        e++;
    }
    int string_len = e-b;
    m_current_block.string.append(b, string_len);
    return parserResult(PARSE_BLOCK_FINISHED, string_len);
}

MudViewParser::parserResult MudViewParser::process_0xd(const WCHAR* b, int len)
{
    return parserResult(PARSE_NO_ERROR, 1);
}

MudViewParser::parserResult MudViewParser::process_0xa(const WCHAR* b, int len)
{
    return parserResult(PARSE_STRING_FINISHED, 1);
}

MudViewParser::parserResult MudViewParser::process_esc(const WCHAR* b, int len)
{
    if (len < 2)
        return parserResult(PARSE_LOW_DATA, 0);

    if (b[1] == L'[') //0x5b
       return process_csi(b, len);

    if (b[1] == L']') //0x5d
        return process_osc(b, len);

    if (b[1] == 0x5c) // string terminator (GA)
    {
        m_current_string->setPrompt();
        return parserResult(PARSE_STRING_IACGA, 2);
    }
    // skip all other esc codes (0x1b + code)
    return parserResult(PARSE_NOT_SUPPORTED, 2);
}

MudViewParser::parserResult MudViewParser::process_csi(const WCHAR* b, int len)
{
    // csi: b[0]=0x1b, b[1]='['
    // find end of csi macro
    if (len < 3)
        return parserResult(PARSE_LOW_DATA, 0);

    const WCHAR *p = b+2;
    if (*p == L'm')
    {
        m_current_block.params.reset();
        return parserResult(PARSE_NO_ERROR, 3);
    }

    const WCHAR *be = b+len;
    const WCHAR *macro_end = NULL;
    while (p != be)
    {
        macro_end = wcschr(L"m@ABCDEFGHJKLMPXr", *p);// find end of csi macro
        if (macro_end != NULL)
            break;
        const WCHAR *check_symbol = wcschr(L";0123456789", *p);
        p++;
        if (!check_symbol)
            return parserResult(PARSE_ERROR_DATA, p-b);
    }

    if (!macro_end)
        return parserResult(PARSE_LOW_DATA, 0);          // not found of end csi macro

    if (*p == L'm')
        return process_csr(b, (p-b)+1);

    return parserResult(PARSE_NOT_SUPPORTED, (p-b)+1);   // skip all other csi codes
}

MudViewParser::parserResult MudViewParser::process_csr(const WCHAR* b, int len)
{
    // b[0]=0x1b, b[1]='[', b[len]='m'
    const WCHAR *e = b+len;
    const WCHAR *p = b+2;

    while (p != e)
    {
        const WCHAR *c = p;
        while (*c != L';' && *c != L'm')
            c++;

        if (p == c)
            {   p++; continue; }

        tstring code_str(p, c-p);
        int code = _wtoi(code_str.c_str());
        p = c;

        MudViewStringParams &pa = m_current_block.params;

        // xterm 256 colors teg
        if (code == 38 || code == 48)
        {
            if ((e-p) < 5)
                return parserResult(PARSE_ERROR_DATA, len);

            if (p[0] == L';' && p[1] == L'5' && p[2] == L';')
            {
                 code_str.assign(&p[3], (e-p)-4);
                 int color = _wtoi(code_str.c_str());
                 if (code == 38)
                 {
                    if (color < 0 || color > 255) color = 7; //default color
                    pa.text_color = color;
                 }
                 else
                 {
                    if (color < 0 || color > 255) color = 0;
                    pa.bkg_color = color;
                 }
                 p = p + 5;
             }
             else
                 return parserResult(PARSE_ERROR_DATA, len);
         }
         if (code >= 30 && code <= 37)
         {
             pa.text_color = code - 30;
             p++;
         }
         else if (code >= 40 && code <= 47)
         {
             pa.bkg_color = code - 40;
             p++;
         }
         else
         {
             switch(code) {
             case 0:
                 pa.reset();
             break;
             case 1:
                 pa.intensive_status = 1;
             break;
             case 4:
                 pa.underline_status = 1;
             break;
             case 5:
                 pa.blink_status = 1;
             break;
             case 7:
                 pa.reverse_video = 1;
             break;
             case 22:
                 pa.intensive_status = 0;
             break;
             case 24:
                 pa.underline_status = 0;
             break;
             case 25:
                 pa.blink_status = 0;
             break;
             case 27:
                 pa.reverse_video = 0;
             break;
             case 39:
             {
                 MudViewStringParams _default;
                 pa.text_color = _default.text_color;
             }
             break;
             case 49:
             {
                 MudViewStringParams _default;
                 pa.bkg_color = _default.bkg_color;
             }
             break;
             default:
             break;
            }
            p++;
         }
    }
    return parserResult(PARSE_NO_ERROR, len);
}

MudViewParser::parserResult MudViewParser::process_osc(const WCHAR* b, int len)
{
    // osc: b[0]=0x1b, b[1]=']'
    if (len < 3)
        return parserResult(PARSE_LOW_DATA, 0);
    const WCHAR *p = b+2;
    if (*p == L'R')
    {   // reset all osc colors
        if (m_palette)
            m_palette->reset_colors = true;
        return parserResult(PARSE_NO_ERROR, 3);
    }
    if (*p == L'P')
    {
        if (len < 10)
             return parserResult(PARSE_LOW_DATA, 0);
        // set osc color
        if (m_palette)
        {
            int index = 0;
            std::string _i((const char*)&p[1], 1);
            sscanf(_i.c_str(), "%x", &index);
            if (index >= 0 && index <= 15)
            {
                COLORREF color = 0;
                std::string _c((const char*)&p[2], 6);
                sscanf(_c.c_str(), "%x", &color);
                m_palette->colors[index] = RGB((color>>16)&0xff,(color>>8)&0xff,color&0xff);
            }
        }
        return parserResult(PARSE_NO_ERROR, 10);
    }
     return parserResult(PARSE_NOT_SUPPORTED, 3);   // skip all other codes
}

// collect strings in parse_data in one with same colors 
// MudViewStringParams(s1) = MudViewStringParams(s2)
void ColorsCollector::process(parseDataStrings* pds)
{
   for (int i=0,e=pds->size(); i<e; ++i)
   {
       //collect same strings color
       std::vector<MudViewStringBlock> &b = pds->at(i)->blocks;
       int j=0, je=b.size()-1;
       while (j<je)
       {
           if (b[j].params == b[j+1].params)
           {
               b[j].string.append(b[j+1].string);
               b.erase(b.begin() + (j+1));
               je--;
           }
           else
           {
               j++;
           }
       }
   }
}

#ifdef _DEBUG
void markBlink(parseDataStrings& strings)
{
    int count = strings.size();
    for (int i = 0; i < count; ++i)
    {
        int blocks = strings[i]->blocks.size();
        for (int j = 0; j < blocks; ++j)
            strings[i]->blocks[j].params.blink_status = 1;
    }
}
void markInversed(parseDataStrings& strings)
{
    int count = strings.size();
    for (int i = 0; i < count; ++i)
    {
        int blocks = strings[i]->blocks.size();
        for (int j = 0; j < blocks; ++j)
            strings[i]->blocks[j].params.reverse_video = 1;
    }
}
void markInversedColor(parseDataStrings& strings, int color)
{
    int count = strings.size();
    for (int i = 0; i < count; ++i)
    {
        int blocks = strings[i]->blocks.size();
        for (int j = 0; j < blocks; ++j)
        {
            MudViewStringParams &p = strings[i]->blocks[j].params;
            p.reverse_video = 1;
            p.text_color = color;
        }
    }
}
void markItalic(parseDataStrings& strings)
{
    int count = strings.size();
    for (int i = 0; i < count; ++i)
    {
        int blocks = strings[i]->blocks.size();
        for (int j = 0; j < blocks; ++j)
            strings[i]->blocks[j].params.italic_status = 1;
    }
}
void printByIndex(const parseDataStrings& strings, int index)
{
    tstring text;
    strings[index]->getText(&text);
}

void markPromptUnderline(parseDataStrings& strings)
{
    int count = strings.size();
    for (int i = 0; i < count; ++i)
    {
        if (!strings[i]->prompt)
            continue;
        int blocks = strings[i]->blocks.size();
        for (int j = 0; j < blocks; ++j)
        {
            MudViewStringParams &p = strings[i]->blocks[j].params;
            p.underline_status = 1;
        }
    }
}
#endif
