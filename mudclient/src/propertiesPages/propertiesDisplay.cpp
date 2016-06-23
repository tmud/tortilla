#include "stdafx.h"
#include "propertiesData.h"
#include "propertiesDisplay.h"

PropertiesDisplay::PropertiesDisplay() : display_width(0), display_height(0)
{
    initOutputWindows();
}

PropertiesDisplay::~PropertiesDisplay()
{
}

void PropertiesDisplay::initDefault()
{
    initMainWindow();
    initFindWindow();
    initOutputWindows();
    plugins_data.setAllOff();
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
        xml::node w = fw[0]; int visible = 0;
        if (!w.get(L"visible", &visible) || !loadRECT(w, &find_window.pos))
            initFindWindow();
        else
            find_window.visible = (visible==1) ? true : false;
    }

    xml::request ow(root_node, L"windows/window");
    if (!loadOutputWindows(ow, &output_windows))
        initOutputWindows();

    return loadOnlyPlugins(root_node);
}

bool PropertiesDisplay::loadOnlyPlugins(xml::node root_node)
{
    xml::request p(root_node, L"plugins/plugin");
    for (int i=0,e=p.size();i<e;++i)
    {
        tstring name;
        if (!p[i].get(L"key", &name) || name.empty())
           continue;
        PluginData pd;
        pd.name = name;
        int state = 0;
        if (p[i].get(L"value", &state) && state == 1)
            pd.state = 1;
        xml::request pw(p[i], L"windows/window");
        if (!pw.empty())
        {
            std::vector<OutputWindow> tmp;
            if (loadOutputWindows(pw, &tmp))
                pd.windows.swap(tmp);
        }
        plugins_data.push_back(pd);
    }
    return true;
}

void PropertiesDisplay::save(xml::node root_node)
{
    xml::node r = root_node.createsubnode(L"display");
    r.set(L"width", display_width);
    r.set(L"height", display_height);

    xml::node mw = r.createsubnode(L"mainwindow");
    saveRECT(mw, main_window.pos);
    mw.set(L"fullscreen", main_window.fullscreen ? 1 : 0);

    xml::node fw = r.createsubnode(L"findwindow");
    saveRECT(fw, find_window.pos);
    fw.set(L"visible", find_window.visible ? 1 : 0);

    xml::node w = r.createsubnode(L"windows");
    saveOutputWindows(w, output_windows);

    xml::node p = r.createsubnode(L"plugins");
    for (int i=0,e=plugins_data.size();i<e;++i)
    {
        PluginData &pd = plugins_data[i];
        xml::node pn = p.createsubnode(L"plugin");
        pn.set(L"key", pd.name);
        pn.set(L"value", pd.state);
        if (!pd.windows.empty())
        {
            xml::node w = pn.createsubnode(L"windows");
            saveOutputWindows(w, pd.windows);
        }
    }
}

bool PropertiesDisplay::current(int width, int height) const
{
    return (display_width == width && display_height == height) ? true : false;
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
    output_windows.clear();
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
        output_windows.push_back(w);
    }
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

void PropertiesDisplay::saveOutputWindows(xml::node n, const OutputWindowsCollection &owc)
{
    for (int i=0,e=owc.size();i<e;++i)
    {
        const OutputWindow& ow = owc[i];
        xml::node w = n.createsubnode(L"window");
        w.set(L"name", ow.name);
        saveRECT(w, ow.pos);
        w.set(L"side", ow.side);
        w.set(L"lastside", ow.lastside);
        w.set(L"width", ow.size.cx);
        w.set(L"height", ow.size.cy);
    }
}

bool PropertiesDisplay::loadOutputWindows(xml::request& r, OutputWindowsCollection *owc)
{
    int maxcount = owc->size();
    int to = (maxcount == 0) ? r.size() : maxcount;
    for (int i=0;i<to;++i)
    {
        OutputWindow w;
        xml::node n = r[i];
        if (!n.get(L"side", &w.side))
            w.side = DOCK_FLOAT;
        if (!n.get(L"lastside", &w.lastside))
            w.lastside = DOCK_FLOAT;
        tstring name;
        if (!loadRECT(n, &w.pos) || !n.get(L"name", &name))
            return false;
        w.name = name;
        int width = 0; int height = 0;
        if (!n.get(L"width", &width))
           width = w.pos.right - w.pos.left;
        if (!n.get(L"height", &height))
           height = w.pos.bottom - w.pos.top;
        w.size.cx = width; w.size.cy = height;
        if (maxcount > 0)
            owc->at(i) = w;
        else
            owc->push_back(w);
    }
    return true;
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

PropertiesDisplayManager::PropertiesDisplayManager() : current_display(NULL)
{
}

PropertiesDisplayManager::~PropertiesDisplayManager()
{
    clear();
}

void PropertiesDisplayManager::clear()
{
    std::for_each(m_displays.begin(), m_displays.end(), [](PropertiesDisplay* d) { delete d; });
    m_displays.clear();
    current_display = NULL;
}

void PropertiesDisplayManager::initDefault()
{
    if (!current_display)
    {
        int index = findCurrentDisplay();
        if (index != -1)
            current_display = m_displays[index];
    }
    if (!current_display)
    {
        current_display = new PropertiesDisplay();
        m_displays.push_back(current_display);
    }
    current_display->initDefault();
}

void PropertiesDisplayManager::load(xml::node root_node)
{
    clear();
    xml::request disp(root_node, L"displays/display");
    for (int i=0,e=disp.size();i<e;++i)
    {
        PropertiesDisplay *d = new PropertiesDisplay();
        if (!d->load(disp[i]))
            { delete d; continue; }
        int index = findDisplay(d->width(), d->height());
        if (index != -1)
        {
            delete m_displays[index];
            m_displays.erase(m_displays.begin()+index);
        }
        m_displays.push_back(d);
    }

    int index = findCurrentDisplay();
    if (index != -1)
       current_display =  m_displays[index];
    else
    {
        PropertiesDisplay *d = new PropertiesDisplay();
        if (!d->load(root_node))
        {
            d->initDefault();
            d->loadOnlyPlugins(root_node);
        }
        current_display = d;
        m_displays.push_back(d);
    }
}

void PropertiesDisplayManager::save(xml::node root_node)
{
    xml::node d = root_node.createsubnode(L"displays");
    for (int i=0,e=m_displays.size();i<e;++i)
      m_displays[i]->save(d);
}

int PropertiesDisplayManager::findCurrentDisplay()
{
    int display_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int display_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);    
    return findDisplay(display_width, display_height);
}

int PropertiesDisplayManager::findDisplay(int cx, int cy)
{
    int index = -1;
    for (int i=0,e=m_displays.size();i<e;++i)
    {
        PropertiesDisplay *d = m_displays[i];
        if (d->current(cx, cy))
        {
            index = i;
            break;
        }
    }
    return index;
}
