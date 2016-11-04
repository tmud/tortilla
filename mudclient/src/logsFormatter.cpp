#include "stdafx.h"
#include "logsFormatter.h"
#include "mudViewString.h"
#include "propertiesPages/propertiesData.h"
#include "palette256.h"

const tchar* month[12] = { L"Янв", L"Фев", L"Мар", L"Апр", L"Май", L"Июн", L"Июл", L"Авг", L"Сен", L"Окт", L"Ноя", L"Дек" };
LogsFormatter::LogsFormatter(PropertiesData *pd) : hfile(INVALID_HANDLE_VALUE), m_propData(pd)
{
}

LogsFormatter::~LogsFormatter()
{
    if (hfile != INVALID_HANDLE_VALUE)
        CloseHandle(hfile);
}

bool LogsFormatter::open(const tstring& filename, PrepareMode pmode)
{
    if (hfile != INVALID_HANDLE_VALUE)
    {
        if (pmode == PM_NEW)
            SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
        SetEndOfFile(hfile);            // clear log and make it as new (dont save exists msgs for closed log)    
    }
    else
    {
        DWORD mode = (pmode == PM_NEW) ? CREATE_ALWAYS : OPEN_ALWAYS;
        hfile = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, mode, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile == INVALID_HANDLE_VALUE)
            return false;
        DWORD hsize = 0;
        DWORD size = GetFileSize(hfile, &hsize);
        if (hsize != 0) {
            CloseHandle(hfile);
            hfile = INVALID_HANDLE_VALUE;
            return false;
        }
    }
    m_mode = pmode;
    m_filename = filename;
    return true;
}

void LogsFormatter::close()
{
    CloseHandle(hfile);
    hfile = INVALID_HANDLE_VALUE;
}

void LogsFormatter::writeString(const MudViewString* str)
{
    std::string data;
    convertString(str, &data);
    write(data);
}

void LogsFormatter::flushStrings()
{
    FlushFileBuffers(hfile);
}

void LogsFormatter::convertString(const MudViewString* str, std::string* out)
{
    assert(false);
}

void LogsFormatter::write(const std::string &data)
{
    DWORD towrite = data.length();
    DWORD written = 0;
    WriteFile(hfile, data.c_str(), towrite, &written, NULL);
}

void LogsFormatter::getHeader(std::string* out)
{
    tstring tu;
    tu.assign(m_propData->title);
    size_t pos = tu.rfind(L"-");
    if (pos != tstring::npos)
        tu = tu.substr(0, pos);
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    tchar buffer[64];
    swprintf(buffer, L"- %d %s %d, %d:%02d:%02d\r\n", tm.wDay, month[tm.wMonth], tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
    tu.append(buffer);
    out->assign(TW2U(tu.c_str()));
}

LogsFormatterHtml::LogsFormatterHtml(PropertiesData *pd) : LogsFormatter(pd)
{
    m_palette = new Palette256(pd);
    m_buffer.alloc(4096);
    m_color_buffer.alloc(10);
    m_color_buffer2.alloc(10);
}

LogsFormatterHtml::~LogsFormatterHtml()
{
    delete m_palette;
}

void LogsFormatterHtml::close()
{
    std::string finish;
    getHeader(&finish);
    std::string closed(TW2U(L"Лог закрыт.\r\n"));
    if (hfile != INVALID_HANDLE_VALUE)
    {
        write(finish);
        write(closed);
        std::string close = "</pre></body></html>";
        write(close);
    }
    LogsFormatter::close();
}

bool LogsFormatterHtml::prepare()
{
    bool result = true;
    if (m_mode == PM_APPEND)
    {
        DWORD inbuffer = 0;
        DWORD hsize = 0;
        DWORD lsize = GetFileSize(hfile, &hsize);
        unsigned __int64 size = hsize; size <<= 32; size |= lsize;

        DWORD buffer_len = m_buffer.getSize();
        char *buffer = m_buffer.getData();

        int pos = 0;
        int fileptr = -1;
        while (size > 0)
        {
            DWORD toread = buffer_len - inbuffer;
            if (toread > size) toread = (DWORD)size;
            char *p = buffer + inbuffer;
            DWORD readed = 0;
            if (!ReadFile(hfile, p, toread, &readed, NULL) || toread != readed)
            {
                result = false; break;
            }
            size -= toread;
            toread += inbuffer;

            // find teg in data
            std::string teg("</body>");
            int teg_len = teg.length();
            char *s = buffer;
            char *e = s + toread;
            while (s != e)
            {
                int len = e - s;
                char *m = (char*)memchr(s, teg.at(0), len);
                if (m && (e - m) >= teg_len && !memcmp(teg.c_str(), m, teg_len))
                {
                    fileptr = pos + (m - buffer);
                    break;
                }
                if (!m) break;
                s = s + (m - s) + 1;
            }
            if (fileptr != -1)
                break;
            pos += (toread - teg_len);
            inbuffer = teg_len;
            memcpy(buffer, buffer + (toread - teg_len), teg_len);
        }

        if (fileptr == -1)
            result = false;
        else
        {
            SetFilePointer(hfile, fileptr, NULL, FILE_BEGIN);
            SetEndOfFile(hfile);
            write("<pre>\r\n");
            std::string date;
            getHeader(&date);
            write(date.c_str());
        }
    }

    if (m_mode == PM_NEW || !result)
    {
        if (!result)
        {
            SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
            SetEndOfFile(hfile);
        }

        char *buffer = m_buffer.getData();
        std::string header("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">\r\n<title>Tortilla Mud Client log");
        header.append("</title>\r\n</head>\r\n<style type=\"text/css\">\r\n");
        TW2U font(m_propData->font_name.c_str());
        sprintf(buffer, "body{ background-color: %s; color: %s; font-size: %dpt; font-weight: %d; font-family: %s }\r\n",
            color(m_propData->bkgnd), color2(m_palette->getColor(7)), m_propData->font_heigth, m_propData->font_bold, (const char*)font);
        header.append(buffer);
        for (int i = 0; i < 16; ++i)
        {
            COLORREF c = m_palette->getColor(i);
            sprintf(buffer, ".c%d { color: %s } .b%d { background: %s }\r\n", i, color(c), i, color2(i == 0 ? m_propData->bkgnd : c));
            header.append(buffer);
        }
        header.append(".u { text-decoration: underline; }\r\n.b { border: 1px solid; }\r\n.i { font-style: italic; }\r\n");
        header.append("</style>\r\n<body><pre>");
        write(header);
        std::string date;
        getHeader(&date);
        write(date);
    }
    return true;
}

void LogsFormatterHtml::convertString(const MudViewString* str, std::string* out)
{
    char* buffer = m_buffer.getData();
    for (int i = 0, e = str->blocks.size(); i < e; ++i)
    {
        const tstring &s = str->blocks[i].string;
        m_converter.convert(s.c_str(), s.length());
        if (isOnlySpaces(s)) {
            out->append(m_converter);
            continue;
        }

        const MudViewStringParams &p = str->blocks[i].params;
        std::string eff;
        if (p.italic_status) eff.append("i ");
        if (p.underline_status) eff.append("u ");
        if (p.blink_status) eff.append("b ");
        if (!eff.empty())
        {
            int len = eff.length();
            eff = eff.substr(0, len - 1);
        }

        if (p.use_ext_colors)
        {
            std::string text(m_converter);
            if (!eff.empty())
            {
                sprintf(buffer, "<a class=\"%s\">%s</a>", eff.c_str(), text.c_str());
                text.assign(buffer);
            }
            COLORREF c1 = p.ext_text_color; COLORREF c2 = p.ext_bkg_color;
            if (p.reverse_video) { c1 = p.ext_bkg_color;  c2 = p.ext_text_color; }
            sprintf(buffer, "<span style=\"color:%s;background-color:%s\">%s</span>", color(c1), color2(c2), text.c_str());
        }
        else
        {
            tbyte txt = p.text_color;
            tbyte bkg = p.bkg_color;
            if (p.reverse_video) { tbyte x = txt; txt = bkg; bkg = x; }
            if (txt <= 7 && p.intensive_status) { txt += 8; } // txt >= 0 always
            if (txt < 16 && p.bkg_color < 16)
            {
                if (!eff.empty())
                    sprintf(buffer, "%s c%d", eff.c_str(), txt);
                else
                    sprintf(buffer, "c%d", txt);
                std::string tmp(buffer);
                if (p.bkg_color != 0)  {
                    sprintf(buffer, "%s b%d", tmp.c_str(), bkg);
                    tmp.assign(buffer);
                }
                sprintf(buffer, "<a class=\"%s\">%s</a>", tmp.c_str(), (const char*)m_converter);
            }
            else
            {
                std::string text(m_converter);
                if (!eff.empty()) {
                    sprintf(buffer, "<a class=\"%s\">%s</a>", eff.c_str(), text.c_str());
                    text.assign(buffer);
                }
                COLORREF c1 = m_palette->getColor(txt);
                if (p.bkg_color != 0) {
                   COLORREF c2 = m_palette->getColor(p.bkg_color);
                   sprintf(buffer, "<span style=\"color:%s;background-color:%s\">%s</span>", color(c1), color2(c2), text.c_str());
                } else {
                   sprintf(buffer, "<span style=\"color:%s\">%s</span>", color(c1), text.c_str());
                }
            }
        }
        out->append(buffer);
    }
    out->append("\r\n");
}

const char* LogsFormatterHtml::color(COLORREF c)
{
    char *buffer = m_color_buffer.getData();
    sprintf(buffer, "#%.2x%.2x%.2x", GetRValue(c), GetGValue(c), GetBValue(c));
    return buffer;
}

const char* LogsFormatterHtml::color2(COLORREF c)
{
    char *buffer = m_color_buffer2.getData();
    sprintf(buffer, "#%.2x%.2x%.2x", GetRValue(c), GetGValue(c), GetBValue(c));
    return buffer;
}

LogsFormatterTxt::LogsFormatterTxt(PropertiesData *pd) : LogsFormatter(pd)
{
}

LogsFormatterTxt::~LogsFormatterTxt()
{
}

void LogsFormatterTxt::close()
{
    std::string finish;
    getHeader(&finish);
    std::string closed(TW2U(L"Лог закрыт.\r\n"));
    if (hfile != INVALID_HANDLE_VALUE)
    {
        write(finish);
        write(closed);
    }
    LogsFormatter::close();
}

bool LogsFormatterTxt::prepare()
{
    if (m_mode == PM_APPEND)
    {
        // append
        ::SetFilePointer(hfile, 0, NULL, FILE_END);
        std::string bin;
        DWORD hsize = 0;
        DWORD size = GetFileSize(hfile, &hsize);
        if (hsize == 0 && size == 0)
        {
            unsigned char bom[4] = { 0xEF, 0xBB, 0xBF, 0 };
            bin.append((char*)bom);
        }
        std::string date;
        getHeader(&date);
        bin.append(date);
        write(bin);
    }
    if (m_mode == PM_NEW)
    {   // new
        SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
        SetEndOfFile(hfile);
        unsigned char bom[4] = { 0xEF, 0xBB, 0xBF, 0 };
        std::string bin;
        bin.append((char*)bom);
        std::string date;
        getHeader(&date);
        bin.append(date);
        write(bin);
    }
    return true;
}

void LogsFormatterTxt::convertString(const MudViewString* str, std::string* out)
{
    tstring s;
    str->getText(&s);
    m_converter.convert(s.c_str(), s.length());
    out->assign(m_converter);
    out->append("\r\n");
}
