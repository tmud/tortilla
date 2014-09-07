#pragma once

#define DEFAULT_CMD_HISTORY_SIZE 20
#define DEFAULT_VIEW_HISTORY_SIZE 1000
#define MIN_CMD_HISTORY_SIZE 10
#define MIN_VIEW_HISTORY_SIZE 100
#define MAX_CMD_HISTORY_SIZE 30
#define MAX_VIEW_HISTORY_SIZE 30000

struct PropertiesHighlight
{
    PropertiesHighlight() : textcolor(RGB(192,192,192)), bkgcolor(RGB(0,0,0)),
        underlined(0), border(0), italic(0) {}
    COLORREF textcolor;
    COLORREF bkgcolor;
    int underlined;
    int border;
    int italic;

    void convertToString(tstring *value) const
    {
        WCHAR buffer[64];
        swprintf(buffer, L"txt[%d,%d,%d],bkg[%d,%d,%d],ubi[%d,%d,%d]", GetRValue(textcolor), GetGValue(textcolor), GetBValue(textcolor),
            GetRValue(bkgcolor), GetGValue(bkgcolor), GetBValue(bkgcolor), underlined, border, italic);
        value->assign(buffer);
    }

    void convertFromString(const tstring& str)
    {
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
           if (isOnlyDigits(timer))
               cmd.assign(str.substr(pos+1));
           else
               timer.assign(L"0");
        }
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
                { assert(false); }
        }
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

    void add(int index, const tstring& key)
    {        
        if (index == -1)
            m_values.push_back(key);
        else
        {
            int size = m_values.size();
            if (index >= 0 && index < size)
                m_values[index] = key;
            else
                { assert(false); }
        }
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

private:
    std::vector<tstring> m_values;
};

struct OutputWindow
{
    OutputWindow() : side(0), lastside(0)
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

    tstring name;
    RECT pos;
    SIZE size;
    int  side;
    int  lastside;
};

struct PluginData
{
    PluginData() : state(false) {}
    tstring name;
    int state;
    std::vector<OutputWindow> windows;    

    void initDefaultPos(int width, int height, OutputWindow *w)
    {
        int windows_count = windows.size();
        int x = 100 + windows_count * 50;
        int y = 250 + windows_count * 50;
        w->initDefaultPos(x, y, width, height);                        
    }
};

struct PluginsDataValues : public std::vector<PluginData>
{
    void saveWindowPos(const tstring& plugin_name, const OutputWindow&  w)
    {
        for (int i = 0, e = size(); i < e; ++i) {
        if (plugin_name == at(i).name) 
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
};

struct PropertiesData
{
    PropertiesData() : codepage(L"win"), cmd_separator(L';'), cmd_prefix(L'#'),
        view_history_size(DEFAULT_VIEW_HISTORY_SIZE)
       , cmd_history_size(DEFAULT_CMD_HISTORY_SIZE)
       , show_system_commands(1), clear_bar(1), disable_ya(0)
       , history_tab(0), timers_on(0), plugins_logs(0), plugins_logs_window(0)
    {
        initDefaultColorsAndFont();
        initMainWindow();
    }

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
    PluginsDataValues plugins;

    std::vector<tstring> cmd_history;

    tstring  codepage;

    COLORREF colors[16];
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
    int      clear_bar;
    int      disable_ya;
    int      history_tab;
    int      timers_on;    
    int      plugins_logs;
    int      plugins_logs_window;

    RECT main_window;
    int  main_window_fullscreen;
    int  display_width;
    int  display_height;
    std::vector<OutputWindow> windows;

    tstring title;        // name of main window (dont need to save)

    void initDefaultColorsAndFont()
    {
        bkgnd = RGB(0, 0, 0);
        colors[0] = RGB(64, 64, 64); 
        colors[1] = RGB(128, 0, 0);
        colors[2] = RGB(0, 128, 0);
        colors[3] = RGB(128, 128, 0);
        colors[4] = RGB(0, 0, 128);
        colors[5] = RGB(128, 0, 128);
        colors[6] = RGB(0, 128, 128);
        colors[7] = RGB(192, 192, 192);
        colors[8] = RGB(128, 128, 128); 
        colors[9] = RGB(255, 0, 0);
        colors[10] = RGB(0, 255, 0);
        colors[11] = RGB(255, 255, 0);
        colors[12] = RGB(0, 0, 255);
        colors[13] = RGB(255, 0, 255);
        colors[14] = RGB(0, 255, 255);
        colors[15] = RGB(255, 255, 255);        
        font_heigth = 10;
        font_italic = 0;
        if (isVistaOrHigher()) 
        {
            font_name.assign(L"Consolas");   // Consolas more pretty font (exist only Vista+)
            font_bold = FW_BOLD;
        }
        else
        {
            font_name.assign(L"Fixedsys" );
            font_bold = FW_NORMAL;
        }        
    }

    void initMainWindow()
    {
        main_window_fullscreen = 0;
        display_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        display_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        int primary_width = GetSystemMetrics(SM_CXSCREEN);
        int primary_height = GetSystemMetrics(SM_CYSCREEN);
        int width = (primary_width / 4) * 3;
        int height = (primary_height / 4) * 3;
        main_window.left = (primary_width - width) / 2;
        main_window.top  = (primary_height - height) / 2;
        main_window.right = main_window.left + width;
        main_window.bottom = main_window.top + height;
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
        initDefaultColorsAndFont();
        initOutputWindows();
        timers_on = 0;
        initPlugins();
    }

    void initPlugins()
    {
        // turn off all plugins
        for (int i = 0, e = plugins.size(); i<e; ++i) { plugins[i].state = 0; }
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

    void addGroup(const tstring& name)
    {
        int index = groups.find(name);
        groups.add(index, name, L"1", L"");
    }

    void deleteGroup(const tstring& name)
    {
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

    void initOutputWindows()
    {
        windows.clear();
        for (int i=0; i<OUTPUT_WINDOWS; ++i) 
        {
            OutputWindow w;      
            w.size.cx = 350; w.size.cy = 200;
            int d = (i+1) * 70;
            RECT defpos = { d, d, d+w.size.cx, d+w.size.cy };
            w.pos = defpos;
            w.side = DOCK_HIDDEN;
            w.lastside = DOCK_FLOAT;
            if (w.name.empty())
            {
                WCHAR buffer[8];
                swprintf(buffer, L"Окно %d", i+1);
                w.name.assign(buffer);
            }
            windows.push_back(w);
        }
    }    

    bool isVistaOrHigher() const
    {
        OSVERSIONINFOEX os;
        ZeroMemory(&os, sizeof(OSVERSIONINFOEX));
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx((OSVERSIONINFO*)&os);
        if ((os.wProductType != VER_NT_WORKSTATION) ||
            (os.dwMajorVersion < 6)) // if less Vista/7/8
                return false;
        return true;
    }
};
