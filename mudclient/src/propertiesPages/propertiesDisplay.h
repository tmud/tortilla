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
    OutputWindowsCollection output_windows;
    std::map<tstring, OutputWindowsCollection*> plugins_windows;
    typedef std::map<tstring, OutputWindowsCollection*>::iterator iterator;
public:
    PropertiesWindow* mainWindow() { return &main_window; }
    PropertiesWindow* findWindow() { return &find_window; }
    OutputWindowsCollection* outputWindows() { return &output_windows; }
    OutputWindowsCollection* pluginWindows(const tstring& plugin) {
       iterator it = plugins_windows.find(plugin);
       if (it == plugins_windows.end()) return NULL;
       return it->second;
    }
    PropertiesDisplay();
    ~PropertiesDisplay();
    void initDefault();
    bool load(xml::node root_node);
    void save(xml::node root_node);
    bool current(int width, int height) const;
    int width() const { return display_width; }
    int height() const { return display_height; }
private:
    void initMainWindow();
    void initFindWindow();
    void initOutputWindows();
    bool loadWindow(xml::node parent, OutputWindow* w);
    bool loadDisplaySize(xml::node n);
    bool loadRECT(xml::node n, RECT *rc);
    void saveRECT(xml::node n, const RECT &rc);
    void saveOutputWindows(xml::node n, const OutputWindowsCollection &owc);
    bool loadOutputWindows(xml::request& r, OutputWindowsCollection *owc);
};

class PropertiesDisplayManager
{
public:
    PropertiesDisplayManager();
    ~PropertiesDisplayManager();
    void initDefault();
    void load(xml::node root_node);
    void save(xml::node root_node);
    PropertiesWindow* main_window() const {
        return current_display->mainWindow();
    }
    PropertiesWindow* find_window() const {
        return current_display->findWindow();
    }
    OutputWindowsCollection* output_windows() const {
        return current_display->outputWindows();
    }
    OutputWindowsCollection* plugin_windows(const tstring& name) {
        return current_display->pluginWindows(name);
    }
private:
    int findCurrentDisplay();
    int findDisplay(int cx, int cy);
    PropertiesDisplay *current_display;
    std::vector<PropertiesDisplay*> m_displays;
};