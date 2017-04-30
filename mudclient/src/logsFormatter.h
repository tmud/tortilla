#pragma once

struct PropertiesData;
struct MudViewString;
class Palette256;
class LogsFormatter
{
public:
    enum PrepareMode { PM_NEW = 0, PM_APPEND };
    LogsFormatter(PropertiesData *pd);
    virtual ~LogsFormatter();
    const tstring& getFilename() const { return m_filename; }
    virtual void normFilename(tstring &name) {}
    virtual void checkExist(tstring& filename);
    virtual bool open(const tstring& filename, PrepareMode pmode);
    virtual bool prepare() { return true; };
    virtual void close();
    virtual void writeString(const MudViewString* str);
    virtual void flushStrings();
    virtual void convertString(const MudViewString* str, std::string* out);
protected:
    void getHeader(std::string* out);
    void write(const std::string &data);
    PropertiesData* m_propData;
    HANDLE hfile;
    WideToUtf8 m_converter;
    tstring m_filename;
    PrepareMode m_mode;
};

class LogsFormatterHtml : public LogsFormatter
{
    MemoryBuffer m_buffer;
    MemoryBuffer m_color_buffer;
    MemoryBuffer m_color_buffer2;
    Palette256* m_palette;
public:
    LogsFormatterHtml(PropertiesData *pd);
    ~LogsFormatterHtml();
    void normFilename(tstring &filename);
    bool prepare();
    void convertString(const MudViewString* str, std::string* out);
    void close();
private:
    const char* color(COLORREF c);
    const char* color2(COLORREF c);
};

class LogsFormatterTxt : public LogsFormatter
{
    WideToUtf8 m_converter;
public:
    LogsFormatterTxt(PropertiesData *pd);
    ~LogsFormatterTxt();
    void normFilename(tstring &filename);
    bool prepare();
    void convertString(const MudViewString* str, std::string* out);
    void close();
};
