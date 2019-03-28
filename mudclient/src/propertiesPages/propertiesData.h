﻿#pragma once

#define DEFAULT_CMD_HISTORY_SIZE 30
#define DEFAULT_VIEW_HISTORY_SIZE 5000
#define MIN_CMD_HISTORY_SIZE 10
#define MAX_CMD_HISTORY_SIZE 500
#define MIN_VIEW_HISTORY_SIZE 1000
#define MAX_VIEW_HISTORY_SIZE 300000
#define TOTAL_MAX_VIEW_HISTORY_SIZE 500000

struct Profile
{
    tstring group;
    tstring name;
};

struct PropertiesHighlight
{
    PropertiesHighlight() { reset(); }
    PropertiesHighlight(COLORREF text_color, COLORREF bgnd_color) { reset(); textcolor = text_color; bkgcolor = bgnd_color; }
    COLORREF textcolor;
    COLORREF bkgcolor;
    int underlined;
    int border;
    int italic;

    void reset()
    {
        underlined = border = italic = 0;
        textcolor = RGB(192,192,192);
        bkgcolor = RGB(0,0,0);
    }

    operator tstring() const {
        tstring value;
        getFlags(&value);
        return value;
    }

    void convertToString(tstring *value) const
    {
        WCHAR buffer[64];
        swprintf(buffer, L"txt[%d,%d,%d],bkg[%d,%d,%d],ubi[%d,%d,%d]", GetRValue(textcolor), GetGValue(textcolor), GetBValue(textcolor),
            GetRValue(bkgcolor), GetGValue(bkgcolor), GetBValue(bkgcolor), underlined, border, italic);
        value->assign(buffer);
    }

    void convertFromString(const tstring& str)
    {
        reset();
        value v; COLORREF color;
        if (parseString(str, L"txt[", &v) && checkColor(v, &color))
            textcolor = color;
        if (parseString(str, L"bkg[", &v) && checkColor(v, &color))
            bkgcolor = color;
        if (parseString(str, L"ubi[", &v))
        {
            if (v.a == 1) underlined = 1;
            if (v.b == 1) border = 1;
            if (v.c == 1) italic = 1;
        }
    }

    void getFlags(tstring* flags) const
    {
        if (underlined) flags->append(L"П");
        if (border) flags->append(L"Р");
        if (italic) flags->append(L"К");
        if (flags->empty())
            flags->append(L"-");
    }
private:
    struct value{ int a; int b; int c; };
    bool parseString(const tstring& str, const tstring& label, value *v)
    {
        int pos = str.find(label);
        if (pos == -1)
            return false;
        int pos2 = str.find(L"]", pos);
        if (pos2 == -1)
            return false;
        pos += label.length();
        tstring tmp(str.substr(pos, pos2-pos));
        swscanf(tmp.c_str(), L"%d,%d,%d", &v->a, &v->b, &v->c);
        return true;
    }
    bool checkColor(const value& v, COLORREF *clr)
    {
        if (v.a >= 0 && v.a <= 255 && v.b >= 0 && v.b <= 255 && v.c >=0 && v.c <= 255)
        {
            *clr = RGB(v.a, v.b, v.c);
            return true;
        }
        return false;
    }   
};

struct PropertiesTimer
{
    void convertToString(tstring *value) const
    {
        value->assign(timer);
        value->append(L";");
        value->append(cmd);
    }
    void convertFromString(const tstring& str)
    {
        timer.clear();
        cmd.clear();
        int pos = str.find(L";");
        if (pos != tstring::npos)
        {
           timer.assign(str.substr(0, pos));
           if (isItNumber(timer))
           {
               double delay = 0;
               w2double(timer, &delay);
               setTimer(delay);
               cmd.assign(str.substr(pos+1));
           }
           else
               timer.assign(L"0");
        }
    }
    void setTimer(double timer_delay)
    {
        if (timer_delay <= 0) timer_delay = 0;
        if (timer_delay >= 10000.0f) timer_delay = 9999.9f;
        bool mod = (getMod(timer_delay) >= 0.09f) ? true : false;
        double2w(timer_delay, (mod) ? 1 : 0, &timer);
    }

    tstring timer;
    tstring cmd;
};

template <class T>
class PropertiesValuesT
{
public:
    struct el {
    tstring key;
    T value;
    tstring group;
    };

    int find(const tstring& key) const
    {
        for (int i=0,e=m_values.size(); i<e; ++i)
            if (m_values[i].key == key) { return i; } 
        return -1;
    }

    int find(const tstring& key, const tstring& group)
    {
        for (int i = 0, e = m_values.size(); i < e; ++i)
            if (m_values[i].key == key && m_values[i].group == group ) { return i; } 
        return -1;
    }

    void add(int index, const tstring& key, const T& value, const tstring& group)
    {
        el data; data.key = key; data.value = value; data.group = group;
        if (index == -1)
            m_values.push_back(data);
        else
        {
            int size = m_values.size();
            if (index >= 0 && index < size)
                m_values[index] = data;
            else
                m_values.push_back(data);
        }
    }

    void insert(int index, const tstring& key, const T& value, const tstring& group)
    {
        if (index == -1)
            return add(index, key, value, group);
        el data; data.key = key; data.value = value; data.group = group;
        m_values.insert(m_values.begin()+index, data);
    }

    void del(int index)
    {
        int size = m_values.size();
        if (index >= 0 && index < size)
            m_values.erase(m_values.begin() + index);
        else
           { assert(false); }
    }

    void swap(int index1, int index2)
    {
        int size = m_values.size();
        if (index1 >= 0 && index1 < size && index2 >= 0 && index2 < size && index1 != index2)
        {
           el t = m_values[index1];
           m_values[index1] = m_values[index2];
           m_values[index2] = t;
        }
    }

    void move(int from, int to)
    {
        int size = m_values.size();
        if (from >= 0 && from < size && to >= 0 && to < size && from != to)
        {
            el t = m_values[from];
            m_values.erase(m_values.begin() + from);
            m_values.insert(m_values.begin() + to, t);
        }
    }

    void swap(PropertiesValuesT<T>& p)
    {
        m_values.swap(p.m_values);
    }

    void assign(const PropertiesValuesT<T>& p)
    {
        m_values.assign(p.m_values.begin(), p.m_values.end());
    }

    const el& get(int index) const
    {
        return m_values[index];
    }

    el& getw(int index)
    {
        return m_values[index];
    }

    int size() const
    {
        return m_values.size();
    }

    void clear()
    {
        m_values.clear();
    }

    bool exist(const tstring& key) const
    {
        return (find(key) == -1) ? false : true;
    }

private:
    std::vector<el> m_values;
};

typedef PropertiesValuesT<tstring> PropertiesValues;
typedef PropertiesValues::el property_value;
typedef PropertiesValuesT<PropertiesHighlight> HighlightValues;
typedef HighlightValues::el highlight_value;
typedef PropertiesValuesT<PropertiesTimer> TimerValues;
typedef TimerValues::el timer_value;

class PropertiesList
{
public:
    int find(const tstring& key) const
    {
        for (int i=0,e=m_values.size(); i<e; ++i)
            if (m_values[i] == key) { return i; } 
        return -1;
    }

    int add(int index, const tstring& key)
    {
        if (index == -1) {
            index = m_values.size();
            m_values.push_back(key);
        }
        else
        {
            int size = m_values.size();
            if (index >= 0 && index < size)
                m_values[index] = key;
            else
                { assert(false); }
        }
        return index;
    }

    int insert(int index, const tstring& key)
    {
        if (index == -1)
            return add(index, key);
        m_values.insert(m_values.begin() + index, key);
        return index;
    }

    void del(int index)
    {
        int size = m_values.size();
        if (index >= 0 && index < size)
            m_values.erase(m_values.begin() + index);
        else
           { assert(false); }
    }

    const tstring& get(int index) const
    {
        return m_values[index];
    }

    tstring& getw(int index)
    {
        return m_values[index];
    }

    int size() const
    {
        return m_values.size();
    }

    void clear()
    {
        m_values.clear();
    }

    bool exist(const tstring& key) const
    {
        return (find(key) == -1) ? false : true;
    }

    void assign(const PropertiesList& p)
    {
        m_values.assign(p.m_values.begin(), p.m_values.end());
    }

private:
    std::vector<tstring> m_values;
};

struct OutputWindow
{
    OutputWindow() : side(DOCK_FLOAT), lastside(DOCK_FLOAT)
    {
        pos.left = pos.right = pos.top = pos.bottom = 0;
        size.cx = size.cy = 0;
    }
    void initDefaultPos(int x, int y, int width, int height)
    {
        lastside = side = DOCK_FLOAT;
        size = { width, height };
        pos = { x, y, x + width, y + height };
    }
    void initVisible(bool visible)
    {
        if (!visible)
            side = DOCK_HIDDEN;
        else
        {
            if (IsDocked(side))
                return;
            side = DOCK_FLOAT;
        }
    }

    tstring name;
    RECT pos;
    SIZE size;
    int  side;
    int  lastside;
};

struct PanelWindow
{
    PanelWindow() : side(0), size(0) {}
    int side;
    int size;
};

struct PluginData
{
    PluginData() : state(0) {}
    tstring name;
    int state;
    std::vector<OutputWindow> windows;
    //std::vector<PanelWindow> panels;

    void initDefaultPos(int width, int height, OutputWindow *w)
    {
        int windows_count = windows.size();
        int x = 100 + windows_count * 50;
        int y = 250 + windows_count * 50;
        w->initDefaultPos(x, y, width, height);
    }

    bool findWindow(const tstring& window_name, OutputWindow *w)
    {
        for (int j = 0, je = windows.size(); j < je; ++j)
        {
            if (windows[j].name == window_name)
            {
                *w = windows[j];
                return true;
            }
        }
        return false;
    }
};

struct PluginsDataValues : public std::vector<PluginData>
{
    void saveWindowPos(const tstring& plugin_filename, const OutputWindow&  w)
    {
        for (int i = 0, e = size(); i < e; ++i) {
        if (plugin_filename == at(i).name) 
        {
            PluginData &p = at(i);
            int index = -1;
            for (int j = 0, je = p.windows.size(); j < je; ++j) {
            if (p.windows[j].name == w.name) {
                    index = j; break;
            }}
            if (index != -1) { p.windows[index] = w; }
            else { p.windows.push_back(w); }
            break;
        }}
    }
    void setAllOff()
    {
        for (int i = 0, e = size(); i < e; ++i) {
            PluginData &p = at(i);
            p.state = 0;
        }
    }
};

struct PropertiesDlgPageState
{
    PropertiesDlgPageState() : topitem(-1), filtermode(0), cansave(false) {}
    tstring item;
    int topitem;
    int filtermode;
    bool cansave;
    tstring group;
};

struct PropertiesDlgData
{
    PropertiesDlgData() : current_page(0) {}
    void clear()
    {
        current_page = 0;
        pages.clear();
    }
    void deleteGroup(const tstring& name)
    {
        for (int i=0,e=pages.size();i<e;++i)
        {
            PropertiesDlgPageState& s = pages[i];
            if (s.group == name)
                { s.item = -1; s.topitem = -1; s.filtermode = false; s.group.clear(); }
        }
    }
    void renameGroup(const tstring& oldname, const tstring& newname)
    {
        for (int i=0,e=pages.size();i<e;++i)
        {
            PropertiesDlgPageState& s = pages[i];
            if (s.group == oldname)
                s.group = newname;
        }
    }
public:
    int current_page;
    std::vector<PropertiesDlgPageState> pages;
};

#include "propertiesDisplay.h"

struct PropertiesData
{
private:
    PropertiesData(const PropertiesData&) {}
    PropertiesData& operator=(const PropertiesData&) {}

public:
    PropertiesData() : codepage(L"win"), logformat(L"html"), cmd_separator(L';'), cmd_prefix(L'#'),
        view_history_size(DEFAULT_VIEW_HISTORY_SIZE)
       , cmd_history_size(DEFAULT_CMD_HISTORY_SIZE)
       , show_system_commands(0), newline_commands(0), clear_bar(1), disable_ya(0), disable_osc(1)
       , history_tab(1), timers_on(0), plugins_logs(1), plugins_logs_window(0), recognize_prompt(0)
       , soft_scroll(0), unknown_cmd(0), any_font(0), disable_alt(0), move_totray(0), disable_mccp(0)
       , dpi(1.0f)
    {
        initDefaultColorsAndFont();
        initDisplay();
    }

    void copy(const PropertiesData& p)
    {
        aliases.assign(p.aliases);
        actions.assign(p.actions);
        subs.assign(p.subs);
        hotkeys.assign(p.hotkeys);
        highlights.assign(p.highlights);
        groups.assign(p.groups);
        antisubs.assign(p.antisubs);
        gags.assign(p.gags);
        timers.assign(p.timers);
        variables.assign(p.variables);
        tabwords.assign(p.tabwords);
        //skip tabwords_commands.assign(p.tabwords_commands);

        dlg = p.dlg;
        // skip displays
        messages = p.messages;
        // copy mode
        mode = p.mode;
        // skip cmd_history
        codepage = p.codepage;
        logformat = p.logformat;

        memcpy(&colors, p.colors, sizeof(colors));
        memcpy(&osc_colors, p.osc_colors, sizeof(osc_colors));
        memcpy(&osc_flags, p.osc_flags, sizeof(osc_flags));
        bkgnd = p.bkgnd;
        font_name = p.font_name;
        font_heigth = p.font_heigth;
        font_bold = p.font_bold;
        font_italic = p.font_italic;

        cmd_separator = p.cmd_separator;
        cmd_prefix = p.cmd_prefix;

        view_history_size = p.view_history_size;
        cmd_history_size = p.cmd_history_size;
        show_system_commands = p.show_system_commands;
        newline_commands = p.newline_commands;
        clear_bar = p.clear_bar;
        disable_ya = p.disable_ya;
        disable_osc = p.disable_osc;
        history_tab = p.history_tab;
        //skip timers_on = p.timers_on;
        plugins_logs = p.plugins_logs;
        plugins_logs_window = p.plugins_logs_window;

        recognize_prompt = p.recognize_prompt;
        recognize_prompt_template = p.recognize_prompt_template;

        soft_scroll = p.soft_scroll;
        unknown_cmd = p.unknown_cmd;
        any_font = p.any_font;
        disable_alt = p.disable_alt;
        move_totray = p.move_totray;
        disable_mccp = p.disable_mccp;

        //skip title
        rebar = p.rebar;
        dpi = p.dpi;
    }

public:
    PropertiesValues aliases;
    PropertiesValues actions;
    PropertiesValues subs;
    PropertiesValues hotkeys;
    PropertiesValues highlights;
    PropertiesValues groups;
    PropertiesValues antisubs;
    PropertiesValues gags;
    PropertiesValues timers;
    PropertiesValues variables;
    PropertiesList   tabwords;
    PropertiesList   tabwords_commands;
    PropertiesDlgData dlg;
    PropertiesDisplayManager displays;

    struct message_data { 
    message_data() { initDefault();  }
    void initDefault(int val = 0) { actions = aliases = subs = hotkeys = highlights = groups = antisubs = gags = timers = variables = tabwords = val; }
    int actions;
    int aliases;
    int subs;
    int hotkeys;
    int highlights;
    int groups;
    int antisubs;
    int gags;
    int timers;
    int variables;
    int tabwords;
    } messages;

    struct debug_data {
    debug_data() { initDefault(); }
    void initDefault() { actions = subs = highlights = antisubs = gags = 0; }
    int actions;
    int subs;
    int highlights;
    int antisubs;
    int gags;    
    } debug;

    struct working_mode { 
    working_mode() { initDefault();  }
    void initDefault(int val = 1) { actions = aliases = subs = hotkeys = highlights = antisubs = gags = plugins = val; }
    int actions;
    int aliases;
    int subs;
    int hotkeys;
    int highlights;
    int antisubs;
    int gags;
    int plugins;
    } mode;

    std::vector<tstring> cmd_history;

    tstring  codepage;
    tstring  logformat;

    COLORREF colors[16];
    COLORREF osc_colors[16];
    tbyte    osc_flags[16];
    COLORREF bkgnd;
    tstring  font_name;
    int      font_heigth;
    int      font_bold;
    int      font_italic;

    WCHAR    cmd_separator;
    WCHAR    cmd_prefix;

    int      view_history_size;
    int      cmd_history_size;
    int      show_system_commands;
    int      newline_commands;
    int      clear_bar;
    int      disable_ya;
    int      disable_osc;
    int      history_tab;
    int      timers_on;
    int      plugins_logs;
    int      plugins_logs_window;

    int      recognize_prompt;
    tstring  recognize_prompt_template;

    int      soft_scroll;
    int      unknown_cmd;
    int      any_font;
    int      disable_alt;
    int      move_totray;
    int      disable_mccp;

    tstring title;        // name of main window (dont need to save)
    float   dpi;          // dpi multiplier (dont need to save)
    tstring rebar;

    void initDefaultColorsAndFont()
    {
        bkgnd = RGB(0, 0, 0);
        colors[0] = RGB(64, 64, 64); 
        colors[1] = RGB(128, 0, 0);
        colors[2] = RGB(0, 128, 0);
        colors[3] = RGB(128, 128, 0);
        colors[4] = RGB(0, 64, 128);
        colors[5] = RGB(128, 0, 128);
        colors[6] = RGB(0, 128, 128);
        colors[7] = RGB(192, 192, 192);
        colors[8] = RGB(128, 128, 128); 
        colors[9] = RGB(255, 0, 0);
        colors[10] = RGB(0, 255, 0);
        colors[11] = RGB(255, 255, 0);
        colors[12] = RGB(0, 128, 255);
        colors[13] = RGB(255, 0, 255);
        colors[14] = RGB(0, 255, 255);
        colors[15] = RGB(255, 255, 255);
        resetOSCColors();
        font_heigth = 10;
        font_italic = 0;
        font_name.assign(L"Fixedsys");
        font_bold = FW_NORMAL;
    }

    void resetOSCColors()
    {
        for (int i = 0; i < 16; ++i) { osc_colors[i] = 0; osc_flags[i] = 0; }
    }

    void initAllDefault()
    {
        aliases.clear();
        actions.clear();
        subs.clear();
        hotkeys.clear();
        highlights.clear();
        antisubs.clear();
        gags.clear();
        timers.clear();
        variables.clear();
        cmd_history.clear();
        groups.clear();
        tabwords.clear();
        initDefaultColorsAndFont();
        initDisplay();
        messages.initDefault();
        mode.initDefault();
        timers_on = 0;
        recognize_prompt = 0;
        recognize_prompt_template.clear();
        dlg.clear();
        rebar.clear();
    }

    void initLogFont(HWND hwnd, LOGFONT *f)
    {
        f->lfHeight = -MulDiv(font_heigth, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72);
        f->lfWidth = 0;
        f->lfEscapement = 0;
        f->lfOrientation = 0;
        f->lfWeight = font_bold;
        f->lfItalic = font_italic ? 1 : 0;
        f->lfUnderline = 0;
        f->lfStrikeOut = 0;
        f->lfCharSet = DEFAULT_CHARSET;
        f->lfOutPrecision = OUT_DEFAULT_PRECIS;
        f->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        f->lfQuality = DEFAULT_QUALITY;
        f->lfPitchAndFamily = DEFAULT_PITCH;
        wcscpy(f->lfFaceName, font_name.c_str() );
    }

    void initDisplay()
    {
        displays.clear();
        displays.initDefault();
    }

    void addGroup(const tstring& name)
    {
        int index = groups.find(name);
        groups.add(index, name, L"1", L"");
    }

    void deleteGroup(const tstring& name)
    {
        dlg.deleteGroup(name);
        delGroupInArray(name, &aliases);
        delGroupInArray(name, &actions);
        delGroupInArray(name, &subs);
        delGroupInArray(name, &hotkeys);
        delGroupInArray(name, &highlights);
        delGroupInArray(name, &antisubs);
        delGroupInArray(name, &gags);
        delGroupInArray(name, &timers);
        for (int i=0,e=groups.size(); i<e; ++i)
        {
            const property_value &g = groups.get(i);
            if (g.key == name){
                groups.del(i); break;
            }
        }
    }

    void renameGroup(const tstring& oldname, const tstring& newname)
    {
        dlg.renameGroup(oldname, newname);
        renGroupInArray(oldname, newname, &aliases);
        renGroupInArray(oldname, newname, &actions);
        renGroupInArray(oldname, newname, &subs);
        renGroupInArray(oldname, newname, &hotkeys);
        renGroupInArray(oldname, newname, &highlights);
        renGroupInArray(oldname, newname, &antisubs);
        renGroupInArray(oldname, newname, &gags);
        renGroupInArray(oldname, newname, &timers);
        for (int i=0,e=groups.size(); i<e; ++i)
        {
            property_value &g = groups.getw(i);
            if (g.key == oldname){
                g.key = newname; break;
            }
        }
    }

    void checkGroups()
    {
        checkGroupsInArray(&aliases);
        checkGroupsInArray(&actions);
        checkGroupsInArray(&subs);
        checkGroupsInArray(&hotkeys);
        checkGroupsInArray(&highlights);
        checkGroupsInArray(&antisubs);
        checkGroupsInArray(&gags);
        checkGroupsInArray(&timers);
    }

    void addDefaultGroup()
    {
        if (groups.size() == 0)
            groups.add(-1, L"default", L"1", L"");
    }

private:
    void delGroupInArray(const tstring& name, PropertiesValues* values)
    {
        int last=values->size()-1;
        for (int i=last; i>=0; --i)
        {
            const property_value &v = values->get(i);
            if (v.group == name)
                values->del(i);
        }
    }

    void renGroupInArray(const tstring& oldname, const tstring& newname, PropertiesValues* values)
    {
        for (int i=0,e=values->size(); i<e; ++i)
        {
            property_value &v = values->getw(i);
            if (v.group == oldname)
                v.group = newname;
        }
    }

    void checkGroupsInArray(PropertiesValues* values)
    {
        std::vector<int> todelete;
        for (int i=0,e=values->size(); i<e; ++i)
        {
            const property_value &v = values->get(i);
            bool exist = false;
            for (int j=0,je=groups.size(); j<je; ++j)
            {
                const property_value &g = groups.getw(j);
                if (v.group == g.key)
                    { exist = true; break; }
            }
            if (!exist)
                todelete.push_back(i);
        }

        int last=todelete.size()-1;
        for (int i=last; i>=0; --i)
            values->del(todelete[i]);
    }
};
