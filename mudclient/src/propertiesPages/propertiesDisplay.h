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
    PluginsDataValues plugins_data;
public:
    PropertiesWindow* mainWindow() { return &main_window; }
    PropertiesWindow* findWindow() { return &find_window; }
    OutputWindowsCollection* outputWindows() { return &output_windows; }
    PluginsDataValues* pluginsData() { return &plugins_data; }

    PropertiesDisplay();
    ~PropertiesDisplay();
    void initDefault();
    bool load(xml::node root_node);
    bool loadOnlyPlugins(xml::node root_node);
    void save(xml::node root_node);
    bool current(int width, int height) const;
    int width() const { return display_width; }
    int height() const { return display_height; }
    void setres(int width, int height) { display_width = width; display_height = height; }
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
    void clear();
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
    PluginsDataValues* plugins_data() const {
        return current_display->pluginsData();
    }
private:
    int findCurrentDisplay();
    int findDisplay(int cx, int cy);
    PropertiesDisplay *current_display;
    std::vector<PropertiesDisplay*> m_displays;
};