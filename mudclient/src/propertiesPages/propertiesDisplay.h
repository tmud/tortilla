#pragma once

struct PropertiesWindow
{
    PropertiesWindow() { pos.left = pos.right = pos.top = pos.bottom = 0; fullscreen = false; visible = false; }
    RECT pos;
    bool fullscreen;
    bool visible;
};

typedef std::vector<OutputWindow> OutputWindowsCollection;
class PropertiesDisplay
{    
    int display_width, display_height;
    PropertiesWindow main_window;
    PropertiesWindow find_window;
    /*RECT main_window;
    int  full_screen;
    RECT find_window;
    int  find_window_visible;*/
    OutputWindowsCollection client_windows;
    std::map<tstring, OutputWindowsCollection*> plugins_windows;

public:
    PropertiesDisplay();
    ~PropertiesDisplay();
    void initDefault();
    bool load(xml::node root_node);
    void save(xml::node root_node);
    bool current() const;

    PropertiesWindow* mainWindow();
    PropertiesWindow* findWindow();
    OutputWindowsCollection* outputWindows();
    OutputWindowsCollection* pluginWindows(const tstring& name);

private:
    void initMainWindow();
    void initFindWindow();
    void initOutputWindows();
    bool loadWindow(xml::node parent, OutputWindow* w);
    void saveWindow(xml::node parent, const OutputWindow& w);
    bool loadDisplaySize(xml::node n);
    bool loadRECT(xml::node n, RECT *rc);
    void saveRECT(xml::node n, const RECT &rc);
};

class PropertiesDisplayManager
{
public:
    PropertiesDisplayManager();
    ~PropertiesDisplayManager();
    void load(xml::node root_node);
    bool save(xml::node root_node);
    PropertiesDisplay* display() const {
        return current_display;
    }
private:
    PropertiesDisplay *current_display;
    std::vector<PropertiesDisplay*> m_displays;
};