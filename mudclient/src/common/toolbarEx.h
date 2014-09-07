#pragma once

class CommandBarEx
{
    int m_lastpos;                     // last position
    tstring m_lastlabel;               // last position label
    CCommandBarCtrl m_cmdbar;
public:
    CommandBarEx();
    ~CommandBarEx();
    HWND CreateMenu(HWND parent, UINT images);
    bool addMenuItem(const TCHAR* menuitem, int pos, UINT id, HBITMAP image);
    void deleteMenuItem(const wchar_t* menuitem, std::vector<UINT> *ids);
    void deleteMenuItem(UINT id);
    void checkMenuItem(UINT id, BOOL state);
    void enableMenuItem(UINT id, BOOL state);
    int getMenuWidth() const;
private:
    void collectSubMenus(HMENU menu, std::vector<UINT> *ids);
    void deleteEmptySubMenu();
    bool findEmptySubMenu(HMENU menu, bool toplevel);
};

class ToolBar
{
public:
    ToolBar() : m_button_size(0) {}
    void attach(HWND toolbar);
    HWND create(HWND parent, int image_size);
    void addButton(HBITMAP bmp, UINT id, const TCHAR* hover);
    void delButton(UINT id);
    void checkButton(UINT id, BOOL state);
    void enableButton(UINT id, BOOL state);
    int  width() const;
    int  button_width() const;
    void show();
    void hide();
private:
    CToolBarCtrl m_toolbar;
    int m_button_size;
};

template <class T>
class ToolbarEx
{
    T* m_parent;
    CommandBarEx m_cmdbar;
    int m_cmdbar_band;
    struct tb { ToolBar* toolbar; tstring name; int band;  };
    std::vector<tb> m_toolbars;
    int m_currentToolbar;
    int m_bandsCount;
    HWND m_rebar;

public:
    ToolbarEx() : m_parent(NULL), m_cmdbar_band(-1), m_currentToolbar(-1), m_bandsCount(0), m_rebar(NULL) {}
    ~ToolbarEx()  {
        for (int i = 0, e = m_toolbars.size(); i < e; ++i) { delete m_toolbars[i].toolbar; }
    }
    void create(T* parent)
    {
        m_parent = parent; 
        m_rebar = m_parent->CreateSimpleReBarCtrl(m_parent->m_hWnd, ATL_SIMPLE_REBAR_NOBORDER_STYLE);
        m_parent->m_hWndToolBar = m_rebar;
    }
    void createCmdBar(UINT images) 
    {
        HWND cmdbar = m_cmdbar.CreateMenu(m_parent->m_hWnd, images);
        m_parent->AddSimpleReBarBand(cmdbar, NULL, TRUE);
        m_cmdbar_band = m_bandsCount++;        
    }
    bool addMenuItem(const TCHAR* menuitem, int pos, UINT id, HBITMAP image)
    {
        return m_cmdbar.addMenuItem(menuitem, pos, id, image);
    }
    void deleteMenuItem(const wchar_t* menuitem, std::vector<UINT>* ids)
    {
        m_cmdbar.deleteMenuItem(menuitem, ids);
    }
    void deleteMenuItem(UINT id) 
    {
        if (id == -1) return;
        m_cmdbar.deleteMenuItem(id);
    }
    void checkMenuItem(UINT id, BOOL state)
    {
        m_cmdbar.checkMenuItem(id, state);
    }
    void enableMenuItem(UINT id, BOOL state)
    {
        m_cmdbar.enableMenuItem(id, state);
    }
    void createToolbar(UINT images) 
    {
        HWND hWndToolBar = m_parent->CreateSimpleToolBarCtrl(m_parent->m_hWnd, images, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
        tb t; t.toolbar = new ToolBar(); t.band = m_bandsCount++;
        t.toolbar->attach(hWndToolBar);
        m_toolbars.push_back(t);
        m_parent->AddSimpleReBarBand(hWndToolBar, NULL, TRUE, t.toolbar->width());
        m_currentToolbar = m_toolbars.size() - 1;
    }
    bool addToolbarButton(HBITMAP bmp, UINT id, const TCHAR* hover) 
    {
        if (m_currentToolbar == -1) return false;
        if (!bmp) return false;
        ToolBar *tb = m_toolbars[m_currentToolbar].toolbar;
        tb->addButton(bmp, id, hover);
        return true;
    }
    void deleteToolbarButton(UINT id)
    {
        if (id == -1) return;
        if (m_currentToolbar == -1) return;
        ToolBar *tb = m_toolbars[m_currentToolbar].toolbar;
        tb->delButton(id);
    }
    void checkToolbarButton(UINT id, BOOL state)
    {
        if (id == -1) return;
        for (int i = 0, e = m_toolbars.size(); i < e; ++i)
        {
            ToolBar *tb = m_toolbars[i].toolbar;
            tb->checkButton(id, state);
        }        
    }
    void enableToolbarButton(UINT id, BOOL state)
    {
        if (id == -1) return;
        for (int i = 0, e = m_toolbars.size(); i < e; ++i)
        {
            ToolBar *tb = m_toolbars[i].toolbar;
            tb->enableButton(id, state);
        }
    }
    void addToolbar(const wchar_t* toolbar, int image_size)
    {
        if (!wcslen(toolbar)) return;
        int id = findToolbar(toolbar);
        if (id != -1)
            m_currentToolbar = id;
        tb t;
        t.toolbar = new ToolBar();
        t.name = toolbar; t.band = m_bandsCount++;
        HWND hWndToolBar = t.toolbar->create(m_parent->m_hWnd, image_size);
        m_toolbars.push_back(t);
        m_parent->AddSimpleReBarBand(hWndToolBar, NULL, FALSE, t.toolbar->width());
        m_currentToolbar = m_toolbars.size() - 1;
    }

    void deleteToolbar(const wchar_t* toolbar)
    {
        int id = findToolbar(toolbar);
        if (id == -1) return;
        CReBarCtrl rebar = m_rebar;
        rebar.DeleteBand(m_toolbars[id].band);
    }

    void hideToolbar(const wchar_t* toolbar)
    {
        int id = findToolbar(toolbar);
        if (id == -1) return;
        CReBarCtrl rebar = m_rebar;        
        rebar.ShowBand(m_toolbars[id].band, FALSE);
    }

    void showToolbar(const wchar_t* toolbar)
    {
        int id = findToolbar(toolbar);
        if (id == -1) return;
        CReBarCtrl rebar = m_rebar;
        rebar.ShowBand(m_toolbars[id].band, TRUE);
    }

    void resetCurrentToolbar()
    {
        resizeBand(m_cmdbar_band, m_cmdbar.getMenuWidth());
        for (int i = 0, e = m_toolbars.size(); i < e; ++i)
        {
            ToolBar* tb = m_toolbars[i].toolbar;            
            resizeBand(m_toolbars[i].band, tb->width());
        }
        m_currentToolbar = 0;
    }

private:
    int findToolbar(const wchar_t* toolbar)
    {
        if (!wcslen(toolbar)) return -1;
        for (int i = 0, e = m_toolbars.size(); i < e; ++i)
        {
            if (!m_toolbars[i].name.compare(toolbar))
                return i;
        }
        return -1;
    }

    void resizeBand(int band, int size)
    {
        REBARBANDINFO ri; memset(&ri, 0, sizeof(REBARBANDINFO));
        ri.cbSize = sizeof(REBARBANDINFO);
        ri.fMask = RBBIM_IDEALSIZE | RBBIM_CHILDSIZE;
        UINT res = ::SendMessage(m_rebar, RB_GETBANDINFO, band, (LPARAM)&ri);
        ri.cxIdeal =  size;
        ri.cxMinChild = ri.cxIdeal + 2;
        ::SendMessage(m_rebar, RB_SETBANDINFO, band, (LPARAM)&ri);
        BOOL flag = TRUE;
        ::SendMessage(m_rebar, RB_MAXIMIZEBAND, band, (LPARAM)flag);
    }
};
