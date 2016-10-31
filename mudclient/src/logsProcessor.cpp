#include "stdafx.h"
#include "logsProcessor.h"

LogsProcessor::LogsProcessor() 
{
}

LogsProcessor::~LogsProcessor()
{
    closeAllLogs();
    stop();
    wait();
    std::for_each(m_free.begin(),m_free.end(),[](MudViewString* s) {delete s;});
}

bool LogsProcessor::init()
{
    return run();
}

void LogsProcessor::calcFileName(tstring& filename)
{
    bool htmlformat = true;
    tstring format(m_propData->logformat);
    if (format == L"txt")
        htmlformat = false;
    int pos = filename.rfind(L'.');
    if (pos == -1)
    {
        filename.append(htmlformat ? L".html" : L".txt");
    }
    else
    {
        tstring ext(filename.substr(pos + 1));
        if (htmlformat) 
        {
            if (ext != L"htm" && ext != L"html")
                filename.append(L".html");
        }
        else
        {
            if (ext != L"txt")
                filename.append(L".txt");
        }
    }
}

int LogsProcessor::openLog(const tstring& filename, bool newlog)
{
    int index = -1;
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
        if (m_logs[i] && m_logs[i]->filename == filename) { index = i; break; }
    if (index != -1)
    {
        log *l = m_logs[index];
        l->close = false;              // dont close (if in closing state)
        if (newlog)
        {
            m_logs[index] = NULL;
            SetFilePointer(l->hfile, 0, NULL, FILE_BEGIN);
            SetEndOfFile(l->hfile);    // clear log and make it as new (dont save exists msgs for closed log)
            m_logs.push_back(l);
            index = m_logs.size() - 1;
        }
        return index;
    }
    DWORD mode = (newlog) ? CREATE_ALWAYS : OPEN_ALWAYS;
    HANDLE hfile = CreateFile(filename.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, mode, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE)
        return -1;
    DWORD hsize = 0;
    DWORD size = GetFileSize(hfile, &hsize);
    if (hsize != 0)
        return -1;

    log *l = new log;
    l->filename = filename;
    l->hfile = hfile;
    if (newlog || size == 0) l->newlog = true;
    else l->append = true;
    l->htmlformat = true;
    tstring format(m_propData->logformat);
    if (format == L"txt")
        l->htmlformat = false;
    m_logs.push_back(l);
    index = m_logs.size()-1;
    return index;
}

void LogsProcessor::closeLog(int id)
{
    CSectionLock _lock(m_cs_logs);
    int last = m_logs.size() - 1;
    if (id >= 0 && id <= last)
    {
        log *l = m_logs[id];
        if (l)
            l->close = true;
    }
}

void LogsProcessor::writeLog(int id, const parseData& pdata)
{
    CSectionLock _lock(m_cs);
    int e=pdata.strings.size()-1;
    for (int i=0; i<=e; ++i)
    {
        MudViewString *src = pdata.strings[i];
        if (src->dropped) continue;
        if (i == 0 && pdata.update_prev_string && !m_msgs.empty())
        {
            int last = m_msgs.size()-1;
            MudViewString* s = m_msgs[last].str;
            s->copy(src);
            continue;
        }
        MudViewString* s = getFreeString();
        s->copy(src);
        msg m; m.log = id; m.str = s;
        m_msgs.push_back(m);
    }
}

tstring LogsProcessor::getLogFile(int id)
{
    CSectionLock _lock(m_cs_logs);
    int last = m_logs.size() - 1;
    if (id >= 0 && id <= last)
    {
        log *l = m_logs[id];
        if (l)
            return l->filename;
    }
    return L"";
}

void LogsProcessor::closeAllLogs()
{
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
    {
        log *l = m_logs[i];
        if (l)
            l->close = true;
    }
}

void LogsProcessor::threadProc()
{
    while (!needStop())
    {
        {
            CSectionLock _lock(m_cs);
            if (!m_msgs.empty())
            {
                int last = m_msgs.size() - 1;
                msg m = m_msgs[last];
                if (!m.str->system)
                {
                  m_msgs.pop_back();
                  m_msgs.swap(m_towrite);
                  m_msgs.push_back(m);
                }
                else
                {
                   m_msgs.swap(m_towrite);
                }
            }
        }
        if (m_towrite.empty())
        {
            closeReqLogs();
            Sleep(250);
            continue;
        }
        saveAll();
        closeReqLogs();
    }
    {
        CSectionLock _lock(m_cs);
        if (!m_msgs.empty())
        {
            m_towrite.insert(m_towrite.end(), m_msgs.begin(), m_msgs.end());
            m_msgs.clear();
        }
    }
    saveAll();
    closeReqLogs();
}

void LogsProcessor::saveAll()
{
    for (int i=0,e=m_towrite.size(); i<e; ++i)
    {
        int id = m_towrite[i].log;
        if (id == -1)
            continue;
        CSectionLock _lock(m_cs_logs);
        int lastid = m_logs.size()-1;
        if (id < 0 || id > lastid || !m_logs[id])
            continue;
        bool htmlformat = m_logs[id]->htmlformat;
        if (htmlformat)
            prepare(id);
        else
            prepare_txt(id);

        for (int j=i; j<e; ++j)
        {
            msg &m = m_towrite[j];
            if (m.log != id) continue;
            m.log = -1;
            if (m.str->dropped)
                continue;
            std::string out;
            if (htmlformat)
                convertString(m.str, &out);
            else
                convertString_txt(m.str, &out);
            write(m_logs[id]->hfile, out);
        }
    }

    CSectionLock _lock(m_cs);
    for (int i=0,e=m_towrite.size(); i<e; ++i)
    {
        MudViewString *s = m_towrite[i].str;
        m_free.push_back(s);
    }
    m_towrite.clear();
}

void LogsProcessor::prepare_txt(int id)
{
    /*log *l = m_logs[id];
    l->opened = true;

    if (l->append)
    {
        l->append = false;
        ::SetFilePointer(l->hfile, 0, NULL, FILE_END);
        std::string bin;
        DWORD hsize = 0;
        DWORD size = GetFileSize(l->hfile, &hsize);
        if (hsize == 0 && size == 0)
        {
            unsigned char bom[4] = { 0xEF, 0xBB, 0xBF, 0 };
            bin.append((char*)bom);
        }
        std::string date;
        getHeader(&date);
        bin.append(date);
        write(l->hfile, bin);
    }
    if (l->newlog)
    {
        l->newlog = false;
        SetFilePointer(l->hfile, 0, NULL, FILE_BEGIN);
        SetEndOfFile(l->hfile);
        unsigned char bom[4] = { 0xEF, 0xBB, 0xBF, 0 };
        std::string bin;
        bin.append((char*)bom);
        std::string date;
        getHeader(&date);
        bin.append(date);
        write(l->hfile, bin);
    }*/
}

void LogsProcessor::prepare(int id)
{
    /*log *l = m_logs[id];
    l->opened = true;

    bool result = true;
    if (l->append)
    {
        l->append = false;
        DWORD inbuffer = 0;
        DWORD hsize = 0;
        DWORD lsize = GetFileSize(l->hfile, &hsize);
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
            if (!ReadFile(l->hfile, p, toread, &readed, NULL) || toread != readed)
                { result = false; break; }
            size -= toread;
            toread += inbuffer;

            // find teg in data
            std::string teg("</body>");
            int teg_len = teg.length();
            char *s = buffer;
            char *e = s + toread;
            while (s != e)
            {
                int len = e-s;
                char *m = (char*)memchr(s, teg.at(0), len);
                if ( m && (e-m)>=teg_len && !memcmp(teg.c_str(), m, teg_len) )
                {
                    fileptr = pos+(m-buffer);
                    break;
                }
                if (!m) break;
                s = s+(m-s)+1;
            }
            if (fileptr != -1)
                break;
            pos += (toread-teg_len);
            inbuffer = teg_len;
            memcpy(buffer, buffer+(toread-teg_len), teg_len);
        }

        if (fileptr == -1)
            result = false;
        else
        {
            SetFilePointer(l->hfile, fileptr, NULL, FILE_BEGIN);
            SetEndOfFile(l->hfile);
            write(l->hfile, "<pre>\r\n");
            std::string date;
            getHeader(&date);
            write(l->hfile, date.c_str());
        }
    }

    if (l->newlog || !result)
    {
        l->newlog = false;
        if (!result)
        {
            SetFilePointer(l->hfile, 0, NULL, FILE_BEGIN);
            SetEndOfFile(l->hfile);
        }

        char *buffer = m_buffer.getData();
        std::string header("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">\r\n<title>Tortilla Mud Client log");
        header.append("</title>\r\n</head>\r\n<style type=\"text/css\">\r\n");
        TW2U font(m_propData->font_name.c_str());
        sprintf(buffer, "body{ background-color: %s; color: %s; font-size: %dpt; font-weight: %d; font-family: %s }\r\n", 
            color(m_propData->bkgnd), color2(m_palette->getColor(7)), m_propData->font_heigth, m_propData->font_bold, (const char*)font);
        header.append(buffer);
        for (int i=0; i<16; ++i)
        {
            COLORREF c = m_palette->getColor(i);
            sprintf(buffer, ".c%d { color: %s } .b%d { background: %s }\r\n", i, color(c), i, color2( i==0 ? m_propData->bkgnd : c ));
            header.append(buffer);
        }
        header.append(".u { text-decoration: underline; }\r\n.b { border: 1px solid; }\r\n.i { font-style: italic; }\r\n");
        header.append("</style>\r\n<body><pre>");
        write(l->hfile, header);
        std::string date;
        getHeader(&date);
        write(l->hfile, date);
    }*/
}

void LogsProcessor::write(HANDLE file, const std::string &data)
{
    DWORD towrite = data.length();
    DWORD written = 0;
    WriteFile(file, data.c_str(), towrite, &written, NULL);
}

void LogsProcessor::closeReqLogs()
{
    CSectionLock _lock(m_cs_logs);
    for (int i=0,e=m_logs.size(); i<e; ++i)
    {
        log *l = m_logs[i];
        if (l && l->close)
        {
            l->ff->close();
            delete l->ff;
            m_logs[i] = NULL;
            delete l;
        }
    }
}

void LogsProcessor::convertString_txt(MudViewString* str, std::string* out)
{
    tstring s;
    str->getText(&s);
    m_converter.convert(s.c_str(), s.length());
    out->assign(m_converter);
    out->append("\r\n");
}

void LogsProcessor::convertString(MudViewString* str, std::string* out)
{
    char* buffer = m_buffer.getData();
    for (int i=0,e=str->blocks.size(); i<e; ++i)
    {
        tstring &s = str->blocks[i].string;
        m_converter.convert(s.c_str(), s.length());
        if (isOnlySpaces(s)) {
            out->append(m_converter);
            continue;
        }

        MudViewStringParams &p = str->blocks[i].params;
        std::string eff;
        if (p.italic_status) eff.append("i ");
        if (p.underline_status) eff.append("u ");
        if (p.blink_status) eff.append("b ");
        if (!eff.empty())
        {
            int len = eff.length();
            eff = eff.substr(0, len-1);
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
            if (!eff.empty())
                sprintf(buffer, "%s c%d", eff.c_str(), txt);
            else
                sprintf(buffer, "c%d", txt);

            std::string tmp(buffer);
            if (p.bkg_color != 0)
            {
                sprintf(buffer, "%s b%d", tmp.c_str(), bkg);
                tmp.assign(buffer);
            }
            sprintf(buffer, "<a class=\"%s\">%s</a>", tmp.c_str(), (const char*)m_converter);
        }
        out->append(buffer);
    }
    out->append("\r\n");
}

void LogsProcessor::updateProps(PropertiesData *pdata)
{
   /* CSectionLock _lock(m_cs_logs);
    m_propData = pdata;
    if (!m_palette)
        m_palette = new Palette256(pdata);
    else
        m_palette->updateProps(pdata);*/
}

MudViewString* LogsProcessor::getFreeString()
{
    if (m_free.empty())
        return new MudViewString;
    int last = m_free.size() - 1;
    MudViewString *s = m_free[last];
    m_free.pop_back();
    return s;
}

/*const char* LogsProcessor::color(COLORREF c)
{
    char *buffer = m_color_buffer.getData();
    sprintf(buffer, "#%.2x%.2x%.2x", GetRValue(c), GetGValue(c), GetBValue(c));
    return buffer;
}

const char* LogsProcessor::color2(COLORREF c)
{
    char *buffer = m_color_buffer2.getData();
    sprintf(buffer, "#%.2x%.2x%.2x", GetRValue(c), GetGValue(c), GetBValue(c));
    return buffer;
}

void LogsProcessor::getHeader(std::string* out)
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
*/