#include "stdafx.h"
#include "propertiesData.h"
#include "propertiesDisplay.h"

PropertiesWindow* PropertiesDisplay::mainWindow()
{
    return &main_window;
}
PropertiesWindow* PropertiesDisplay::findWindow()
{
    return &find_window;
}
OutputWindowsCollection* PropertiesDisplay::outputWindows()
{
    return &client_windows;
}
OutputWindowsCollection* PropertiesDisplay::pluginWindows(const tstring& name)
{
    std::map<tstring, OutputWindowsCollection*>::iterator it = plugins_windows.find(name);
    return (it == plugins_windows.end()) ? NULL : it->second;
}

PropertiesDisplay::PropertiesDisplay() : display_width(0), display_height(0)
{
    client_windows.resize(OUTPUT_WINDOWS);
}

PropertiesDisplay::~PropertiesDisplay()
{
    std::for_each( plugins_windows.begin(), plugins_windows.end(),
        [](std::pair<const tstring, OutputWindowsCollection*>& o) { delete o.second; } );
}

void PropertiesDisplay::initDefault()
{
    initMainWindow();
    initFindWindow();
    initOutputWindows();
    plugins_windows.clear();
}

bool PropertiesDisplay::load(xml::node root_node)
{
    std::wstring name;
    root_node.getname(&name);
    bool display_loaded = false;
    if (name == L"display") {
       display_loaded = loadDisplaySize(root_node);
       if (!display_loaded) return false;
    }
    xml::request mw(root_node, L"mainwindow");
    if (!mw.size())
        return false;
    xml::node w = mw[0];
    if (!display_loaded && !loadDisplaySize(w))
       return false;
    int full_screen = 0;
    w.get(L"fullscreen", &full_screen);
    main_window.fullscreen = (full_screen==1) ? true : false;

    if (!loadRECT(w, &main_window.pos))
        initMainWindow();
    xml::request fw(root_node, L"findwindow");
    if (!fw.size())
        initFindWindow();
    else
    {
        xml::node w = fw[0];
        int visible = 0;
        if (!w.get(L"visible", &visible) || !loadRECT(w, &find_window.pos))
        {
            initFindWindow();
        }
        else
        {
            find_window.visible = (visible==1) ? true : false;
        }
    }

    if (!display_loaded)
    {
    
    }
    else
    {
        
    }

    return true;
}

void PropertiesDisplay::save(xml::node root_node)
{
    
}

bool PropertiesDisplay::current() const
{
    return (display_width == GetSystemMetrics(SM_CXVIRTUALSCREEN) && display_height == GetSystemMetrics(SM_CYVIRTUALSCREEN)) ? true : false;
}

void PropertiesDisplay::initMainWindow()
{
    main_window.fullscreen = false;
    display_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    display_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int primary_width = GetSystemMetrics(SM_CXSCREEN);
    int primary_height = GetSystemMetrics(SM_CYSCREEN);
    int width = (primary_width / 4) * 3;
    int height = (primary_height / 4) * 3;
    RECT &pos = main_window.pos;
    pos.left = (primary_width - width) / 2;
    pos.top = (primary_height - height) / 2;
    pos.right = pos.left + width;
    pos.bottom = pos.top + height;
}

void PropertiesDisplay::initFindWindow()
{
    RECT &pos = find_window.pos;
    pos.left = 200;
    pos.top = 100;
    pos.right = pos.left;
    pos.bottom = pos.top;
    find_window.visible = false;
}

void PropertiesDisplay::initOutputWindows()
{
    client_windows.clear();
    for (int i = 0; i < OUTPUT_WINDOWS; ++i)
    {
        OutputWindow w;
        w.size.cx = 350; w.size.cy = 200;
        int d = (i + 1) * 70;
        RECT defpos = { d, d, d + w.size.cx, d + w.size.cy };
        w.pos = defpos;
        w.side = DOCK_HIDDEN;
        w.lastside = DOCK_FLOAT;
        if (w.name.empty())
        {
            WCHAR buffer[8];
            swprintf(buffer, L"Окно %d", i + 1);
            w.name.assign(buffer);
        }
        client_windows.push_back(w);
    }
}

bool PropertiesDisplay::loadWindow(xml::node parent, OutputWindow* w)
{
    if (!parent.get(L"side", &w->side))
        w->side = DOCK_FLOAT;
    if (!parent.get(L"lastside", &w->lastside))
        w->lastside = DOCK_FLOAT;
    tstring name;
    if (loadRECT(parent, &w->pos) && parent.get(L"name", &name))
    {
        w->name = name;
        int width = 0; int height = 0;
        if (!parent.get(L"width", &width))
            width = w->pos.right - w->pos.left;
        if (!parent.get(L"height", &height))
            height = w->pos.bottom - w->pos.top;
        w->size.cx = width; w->size.cy = height;
        return true;
    }
    return false;
}

void PropertiesDisplay::saveWindow(xml::node parent, const OutputWindow& w)
{
    xml::node xw = parent.createsubnode(L"window");
    xw.set(L"name", w.name.c_str());
    saveRECT(xw, w.pos);
    xw.set(L"side", w.side);
    xw.set(L"lastside", w.lastside);
    xw.set(L"width", w.size.cx);
    xw.set(L"height", w.size.cy);
}

bool PropertiesDisplay::loadDisplaySize(xml::node n)
{
    return (n.get(L"width", &display_width) && n.get(L"height", &display_height)) ? true : false;
}

bool PropertiesDisplay::loadRECT(xml::node n, RECT *rc)
{
    int left = 0; int right = 0; int top = 0; int bottom = 0;
    if (!n.get(L"left", &left) ||
        !n.get(L"right", &right) ||
        !n.get(L"top", &top) ||
        !n.get(L"bottom", &bottom))
        return false;

    rc->left = left;
    rc->right = right;
    rc->top = top;
    rc->bottom = bottom;
    return true;
}

void PropertiesDisplay::saveRECT(xml::node n, const RECT &rc)
{
    n.set(L"left", rc.left);
    n.set(L"right", rc.right);
    n.set(L"top", rc.top);
    n.set(L"bottom", rc.bottom);
}

PropertiesDisplayManager::PropertiesDisplayManager() : current_display(NULL)
{
}

PropertiesDisplayManager::~PropertiesDisplayManager()
{
    std::for_each(m_displays.begin(), m_displays.end(), [](PropertiesDisplay* d) { delete d; });
}

void PropertiesDisplayManager::load(xml::node root_node)
{
    bool default_display = true;
    PropertiesDisplay *d = new PropertiesDisplay();
    if (!d->load(root_node))
        d->initDefault();
    current_display = d;    
    xml::request disp(root_node, L"displays/display");
    for (int i=0,e=disp.size();i<e;++i)
    {
        d = new PropertiesDisplay();
        if (d->load(disp[i]))
        {            
            if (d->current()) {
                if (!default_display) { delete d; continue; }
                default_display = false;
                delete current_display;
                current_display = d;
            }
            m_displays.push_back(d);
        } 
        else
        {
            delete d;
        }
    }
}

bool PropertiesDisplayManager::save(xml::node root_node)
{
    xml::node d = root_node.createsubnode(L"displays");
    if (!d) return false;
    return true;
}
