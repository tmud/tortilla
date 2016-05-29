#pragma once
#include "propertiesData.h"

class PropertiesDisplay
{
    typedef std::vector<OutputWindow> OutputWindowsCollection;
    int display_width, display_height, full_screen;
    RECT main_window;
    RECT find_window;
    int  find_window_visible;
    OutputWindowsCollection client_windows;    
    std::map<tstring, OutputWindowsCollection> plugins_windows;

public:
    PropertiesDisplay();
    void initDefault();
    bool load(xml::node root_node);
    void save(xml::node root_node);
    bool current() const;

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
    const PropertiesDisplay* display() const {
        return current_display;
    }
private:
    PropertiesDisplay *current_display;
    std::vector<PropertiesDisplay*> m_displays;
};