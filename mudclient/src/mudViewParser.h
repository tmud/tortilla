#pragma once 
#include "mudViewString.h"

typedef std::vector<MudViewString*> parseDataStrings;
struct parseData
{
    parseData() : update_prev_string(false), last_finished(true) {}
    ~parseData() { autodel<MudViewString> z1(strings); }
    bool update_prev_string;
    bool last_finished;
    parseDataStrings strings;
};

#ifdef MARKERS_IN_VIEW
void markBlink(parseDataStrings& strings);
void markInversed(parseDataStrings& strings);
void markInversedColor(parseDataStrings& strings, int color);
void markItalic(parseDataStrings& strings);
void printByIndex(const parseDataStrings& strings, int index);
void markPromptUnderline(parseDataStrings& strings);
#define MARKBLINK(x) markBlink(x)
#define MARKINVERSED(x) markInversed(x)
#define MARKITALIC(x) markItalic(x)
#define MARKPROMPTUNDERLINE(x) markPromptUnderline(x)
#define PRINTBYINDEX(x, i) printByIndex(x, i);
#define MARKINVERSEDCOLOR(x, c) markInversedColor(x, c);
#else
#define MARKBLINK(x) {}
#define MARKINVERSED(x) {}
#define MARKITALIC(x) {}
#define MARKPROMPTUNDERLINE(x) {}
#define PRINTBYINDEX(x, i) {}
#define MARKINVERSEDCOLOR(x, c) {}
#endif

class MudViewParser
{
public:
    MudViewParser();
    ~MudViewParser();
    void parse(const WCHAR* text, int len, bool newline_iacga, parseData* data);

private:
    enum parserResultCode {
    PARSE_NO_ERROR = 0,
    PARSE_LOW_DATA,        // low data for parsing
    PARSE_ERROR_DATA,      // data parsed, data with errors (skipped errors)
    PARSE_NOT_SUPPORTED,   // data parsed, commands not supported (skipped)
    PARSE_BLOCK_FINISHED,
    PARSE_STRING_FINISHED, // string finalized with new line
    PARSE_STRING_IACGA
    };

    struct parserResult
    {
        parserResult(parserResultCode r, int p) : result(r), processed(p) {}
        parserResultCode result;
        int processed;
    };

    parserResult process(const WCHAR* b, int len);
    parserResult process_string(const WCHAR* b, int len);
    parserResult process_0xd(const WCHAR* b, int len);
    parserResult process_0xa(const WCHAR* b, int len);
    parserResult process_esc(const WCHAR* b, int len);
    parserResult process_csi(const WCHAR* b, int len);
    parserResult process_csr(const WCHAR* b, int len);    

private:
    DataQueue m_buffer;
    MudViewString *m_current_string;
    MudViewStringBlock m_current_block;
    bool m_last_finished;
};

class ColorsCollector
{
public:
    void process(parseData *data);
};
