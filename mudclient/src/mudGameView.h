#pragma once
#include "accessors.h"
#include "propertiesPages/propertiesDlg.h"
#include "profiles/profileDlgs.h"

#include "network/network.h"
#include "mudView.h"
#include "mudCommandBar.h"
#include "findDlg.h"
#include "modeDlg.h"
#include "logicProcessor.h"

#include "plugins/pluginsApi.h"
#include "plugins/pluginsView.h"
#include "plugins/pluginsManager.h"

#include "AboutDlg.h"
#include "helpManager.h"

#define WS_DEFCHILD WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN

class MudGameView : public CWindowImpl<MudGameView>, public LogicProcessorHost, public CIdleHandler
{
    enum { CPWIN = 0, CPUTF8 };
    PropertiesManager m_manager;
    PropertiesElements m_propElements;
    PropertiesData *m_propData;

    CWindow m_parent;
    CDockingWindow m_dock;
    int m_barHeight;
    MudCommandBar m_bar;
    FindView m_find_dlg;
    int m_last_find_view;
    MudView m_history;
    MudView m_view;
    CSplitterWindowExT<false, 3, 1> m_hSplitter;

    NetworkConnectData m_networkData;
    Network m_network;
    DataQueue m_network_queue;
    HotkeyTable m_hotkeyTable;
    LogicProcessor m_processor;
    std::vector<MudView*> m_views;
    std::vector<PluginsView*> m_plugins_views;
    PluginsManager m_plugins;
    int m_codepage;
    bool m_activated;
    bool m_settings_mode;
    bool m_drag_flag;
    std::vector<MudViewHandler*> m_handlers;

private:
    void onStart();
    void onClose();
    void onNewProfile();
    void onLoadProfile();
    void onNewWorld();
    void loadPlugins();
    void unloadPlugins();

public:
    DECLARE_WND_CLASS(NULL)

    MudGameView() : m_propElements(m_manager.getConfig()), m_propData(m_propElements.propData),
        m_barHeight(32), m_bar(m_propData), m_last_find_view(-1), m_network_queue(2048),
        m_view(&m_propElements, 0), m_history(&m_propElements, -1),
        m_processor(this), m_codepage(CPWIN), m_activated(false), m_settings_mode(false), m_drag_flag(false)
    {
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        UINT msg = pMsg->message;
        if (msg == WM_MOUSEWHEEL)
        {
            BOOL b = FALSE;
            OnWheel(pMsg->message, pMsg->wParam, pMsg->lParam, b);
            return TRUE;
        }
        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
        {
            tstring key;
            m_hotkeyTable.recognize(pMsg->wParam, pMsg->lParam, &key);
            if (m_processor.processHotkey(key))
                return TRUE;
            if (GetKeyState(VK_MENU) < 0)
            {
                if (pMsg->wParam != VK_MENU && pMsg->wParam != VK_F4)
                    return TRUE;
            }
            if (pMsg->wParam == VK_MENU && m_propData->disable_alt)
            {
               return TRUE;
            }
            if (processKey(pMsg->wParam))
                return TRUE;
        }

        if (m_find_dlg.processMsg(pMsg))
        {
            if (msg == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
               SetFocus();
            if (msg == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
               findText();
            return TRUE;
        }

        if (msg == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
        {
            if (m_history.IsWindowVisible())
            {
                closeHistory();
                return TRUE;
            }
            int last = m_view.getLastString();
            if (last != m_view.getViewString())
            {
                m_view.setViewString(last);
                return TRUE;
            }
        }
        if (msg == WM_KEYDOWN  && pMsg->wParam == VK_F12 && checkKeysState(true, false, false))
        {
            // Shift+F12 - hot key for settings
            BOOL b = FALSE;
            OnSettings(0,0,0,b);
            return TRUE;
        }
        if (msg == WM_KEYDOWN  && pMsg->wParam == 'F' && checkKeysState(false, true, false))
        {
            // Ctrl+F - search mode
            PropertiesWindow* main_window = m_propData->displays.main_window();
            if (main_window->visible && m_find_dlg.focused()) { hideFindView(); }
            else { showFindView(true); }
            return TRUE;
        }
        if (m_bar.PreTranslateMessage(pMsg))
            return TRUE;
        return FALSE;
    }

    BOOL OnIdle()
    {
        collectGarbage();
        return FALSE;
    }

    bool initialize();
    HWND createView(HWND parent)
    {
        m_parent = parent;
        RECT rc; ::GetClientRect(m_parent, &rc);
        HWND dock = m_dock.Create(m_parent, rc);

        int height = rc.bottom;
        rc.top = height - m_barHeight;
        HWND bar = m_bar.Create(dock, rc, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP);
        m_dock.SetStatusBar(bar);
        m_dock.SetStatusBarHeight(m_barHeight);

        rc.bottom = rc.top;
        rc.top = 0;
        Create(dock, rc, NULL, WS_DEFCHILD);
        m_dock.SetClient(m_hWnd);
        m_bar.setCommandEventCallback(m_hWnd, WM_USER);
        return dock;
    }

    bool activated() const
    {
        return m_activated;
    }

    bool isPropertiesOpen() const
    {
        return m_settings_mode;
    }

    PluginsView* createPanel(const PanelWindow& w, Plugin* p)
    {
        PluginsView *v = new PluginsView(p);
        v->Create(m_dock, rcDefault, L"", WS_DEFCHILD);
        int dt = (w.side == DOCK_LEFT || w.side == DOCK_RIGHT) ? GetSystemMetrics(SM_CXEDGE) : GetSystemMetrics(SM_CYEDGE);
        m_dock.m_panels.AddWindow(*v, w.side, w.size+dt);
        return v;
    }

    void deletePanel(PluginsView *v)
    {
        HWND hwnd = v->m_hWnd;
        m_dock.m_panels.RemoveWindow(hwnd);
        ::DestroyWindow(hwnd);
        delete v;
    }

    PluginsView* createDockPane(const OutputWindow& w, Plugin* p)
    {
        PluginsView *v = new PluginsView(p);
        v->Create(m_dock, rcDefault, w.name.c_str(), WS_DEFCHILD, 0);
        m_dock.AddWindow(*v);
        if (IsDocked(w.side))
        {
            m_dock.DockWindow(*v, w.side);
            int size = IsDockedVertically(w.side) ? w.pos.right : w.pos.bottom;
            m_dock.SetPaneSize(w.side, size);
        }
        else if (IsFloating(w.side))
        {
            m_dock.FloatWindow(*v, w.pos);
        }
        else
        {
            HWND hwnd = v->m_hWnd;
            DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
            ctx->rcWindow = w.pos;
            ctx->sizeFloat = w.size;
            ctx->Side = w.side; 
            ctx->LastSide = w.lastside;
        }
        m_plugins_views.push_back(v);
        return v;
    }

    void deleteDockPane(PluginsView *v)
    {
        HWND hwnd = v->m_hWnd;
        savePluginWindowPos(hwnd);
        m_dock.RemoveWindow(hwnd);
        for (int i = 0, e = m_plugins_views.size(); i < e; ++i) {
        if (m_plugins_views[i] == v) { 
            m_plugins_views.erase(m_plugins_views.begin() + i); break;
        }}
        ::DestroyWindow(hwnd);
        delete v;
    }

    void dockDockPane(PluginsView* v, int side)
    {
        HWND hwnd = v->m_hWnd;
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        if (ctx && IsFloating(ctx->Side) && side >= 0)
        {
            m_dock._UnFloatWindow(ctx);
            m_dock._DockWindow(ctx, side, 0, FALSE);
        }
    }

    void undockDockPane(PluginsView* v)
    {
        HWND hwnd = v->m_hWnd;
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        if (ctx && IsDocked(ctx->Side))
        {
            RECT pos;
            ::GetWindowRect(ctx->hwndFloated, &pos);
            m_dock._UnDockWindow(ctx);
            ctx->rcWindow = pos;
            m_dock._FloatWindow(ctx);
        }
    }

    void blockDockPane(PluginsView* v, int side)
    {
        HWND hwnd = v->m_hWnd;
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        if (ctx)
        {
            if (ctx->Side == side)
                return;
            DWORD flags = 0;
            switch (side)
            {
              case DOCK_LEFT: flags = DCK_NOLEFT; break;
              case DOCK_RIGHT: flags = DCK_NORIGHT; break;
              case DOCK_TOP: flags = DCK_NOTOP; break;
              case DOCK_BOTTOM: flags = DCK_NOBOTTOM; break;
              case DOCK_FLOAT: flags = DCK_NOFLOAT; break;
            }
            if (flags != 0)
                ctx->dwFlags |= flags;
        }
    }

    void unblockDockPane(PluginsView* v, int side)
    {
        HWND hwnd = v->m_hWnd;
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        if (ctx)
        {
            DWORD flags = 0;
            switch (side)
            {
            case DOCK_LEFT: flags = DCK_NOLEFT; break;
            case DOCK_RIGHT: flags = DCK_NORIGHT; break;
            case DOCK_TOP: flags = DCK_NOTOP; break;
            case DOCK_BOTTOM: flags = DCK_NOBOTTOM; break;
            case DOCK_FLOAT: flags = DCK_NOFLOAT; break;
            }
            if (flags != 0)
            {
                flags ^= 0xffffffff;
                ctx->dwFlags &= flags;
            }
        }
    }

    bool isDockPaneVisible(PluginsView* v)
    {
       return m_dock.IsVisible(v->m_hWnd) ? true : false;
    }

    void hideDockPane(PluginsView* v)
    {
        m_dock.HideWindow(v->m_hWnd);
    }

    void showDockPane(PluginsView* v)
    {
        m_dock.ShowWindow(v->m_hWnd);
    }

    SIZE getDockPaneSize(PluginsView *v)
    {
        return m_dock.GetWindowSize(v->m_hWnd);
    }

    HWND getFloatingWnd(PluginsView* v)
    {
        HWND hwnd = v->m_hWnd;
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        return (ctx) ? ctx->hwndFloated : NULL;
    }

    void addWindowBorder(int &width, int &height)
    {
        width += (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER)) * 2;
        height += (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYBORDER)) * 2 + GetSystemMetrics(SM_CYSMCAPTION);
    }

    void setFixedSize(HWND hwnd, int width, int height)
    {
        DOCKCONTEXT *ctx = m_dock._GetContext(hwnd);
        if (!ctx) return;
        if (width <= 0 || height <= 0)
            return;
        CWindow p(ctx->hwndFloated);
        if (!p.IsWindow()) return;
        addWindowBorder(width, height);
        RECT pos = ctx->rcWindow;
        pos.right = pos.left + width - 1;
        pos.bottom = pos.top + height - 1;
        ctx->bBlockFloatingResizeBox = true;
        ctx->dwFlags |= DCK_NOLEFT|DCK_NORIGHT|DCK_NOTOP|DCK_NOBOTTOM;
        p.MoveWindow(&pos);
    }

    void setCommand(const tstring& cmd) { m_bar.setCommand(cmd); }
    void getCommand(tstring* cmd) { m_bar.getCommandLine(cmd); }
    void showView(int view, bool show) { showWindow(view, show); }
    bool isViewVisible(int view) { return isWindowVisible(view); }

    void findText();

    LogicProcessorMethods *getMethods() { return &m_processor; }
    PropertiesData *getPropData() { return m_propData;  }
    CFont *getStandardFont() { return &m_propElements.standard_font; }
    PropertiesManager* getPropManager() { return &m_manager; }
    Palette256* getPalette() { return &m_propElements.palette;  }
    PluginsManager* getPluginsManager() { return &m_plugins; }
    int convertSideFromString(const wchar_t* side) { return m_dock.GetSideByString(side); }
    const NetworkConnectData* getConnectData() { return &m_networkData; }
    MudViewHandler* getHandler(int view);

private:
    BEGIN_MSG_MAP(MudGameView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_USER, OnUserCommand)
        MESSAGE_HANDLER(WM_USER+1, OnNetwork)
        MESSAGE_HANDLER(WM_USER+2, OnFullScreen)
        MESSAGE_HANDLER(WM_USER+3, OnShowWelcome)
        MESSAGE_HANDLER(WM_USER+4, OnBarSetFocus)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
    ALT_MSG_MAP(1)  // retranslated from MainFrame
        MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_CLOSE, OnParentClose)
        MESSAGE_HANDLER(WM_MOUSEWHEEL, OnWheel)
        COMMAND_ID_HANDLER(ID_NEWPROFILE, OnNewProfile)
        COMMAND_ID_HANDLER(ID_LOADPROFILE, OnLoadProfile)
        COMMAND_ID_HANDLER(ID_NEWWORLD, OnNewWorld)
        COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
        COMMAND_ID_HANDLER(ID_MODE, OnMode)
        COMMAND_RANGE_HANDLER(ID_WINDOW_1, ID_WINDOW_6, OnShowWindow)
        COMMAND_ID_HANDLER(ID_VIEW_FIND, OnViewFind)
        MESSAGE_HANDLER(WM_DOCK_PANE_CLOSE, OnCloseWindow)
        MESSAGE_HANDLER(WM_DOCK_FOCUS, OnBarSetFocus)
        COMMAND_ID_HANDLER(ID_PLUGINS, OnPlugins)
        COMMAND_RANGE_HANDLER(PLUGING_MENUID_START, PLUGING_MENUID_END, OnPluginMenuCmd)
		MESSAGE_HANDLER(WM_ACTIVATEAPP, OnActivateApp)
        CHAIN_MSG_MAP_ALT_MEMBER(m_dock, 1) // processing some system messages
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM lparam, BOOL& bHandled)
    {
        RECT rc;
        GetClientRect(&rc);
        m_hSplitter.Create(m_hWnd, rc, NULL, 0, WS_EX_CLIENTEDGE);
        m_hSplitter.m_cxySplitBar = 3;
        m_hSplitter.SetDefaultSplitterPos();

        RECT pane_top, pane_bottom;
        m_hSplitter.GetSplitterPaneRect(0, &pane_top);
        m_hSplitter.GetSplitterPaneRect(1, &pane_bottom);

        DWORD bstyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL;
        m_history.Create(m_hSplitter, pane_top, NULL, bstyle);
        m_view.Create(m_hSplitter, pane_bottom, NULL, bstyle);
        m_hSplitter.SetSplitterPanes(m_history, m_view);
        m_hSplitter.SetSinglePaneMode(SPLIT_PANE_BOTTOM);

        PropertiesWindow *main_window = m_propData->displays.main_window();
        m_parent.MoveWindow(&main_window->pos);

        // create find panel
        m_find_dlg.Create(m_dock);
        m_find_dlg.SetWindowText(L"Поиск");
        m_dock.AddWindow(m_find_dlg);
        m_find_dlg.setWindowName(0, L"Главное окно");
        m_find_dlg.selectWindow(0);

        // create docking output windows
        OutputWindowsCollection* output_windows = m_propData->displays.output_windows();
        for (int i=0; i < OUTPUT_WINDOWS; ++i)
        {            
            const OutputWindow& w =  output_windows->at(i);
            m_find_dlg.setWindowName(i+1,w.name);

            MudView *v = new MudView(&m_propElements, i+1);
            DWORD style = WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE;
            int menu_id = i+ID_WINDOW_1;
            if (w.side != DOCK_HIDDEN)
                m_parent.SendMessage(WM_USER, menu_id, 1);
            m_parent.SendMessage(WM_USER+1, menu_id, (WPARAM)w.name.c_str());

            v->Create(m_dock, rcDefault, w.name.c_str(), style, 0);
            m_views.push_back(v);
            m_dock.AddWindow(*v);
            if (IsDocked(w.side))
            {
                m_dock.DockWindow(*v, w.side);
                int size = IsDockedVertically(w.side) ? w.pos.right-w.pos.left : w.pos.bottom-w.pos.top;
                m_dock.SetPaneSize(w.side, size+DEFAULT_SPLITTER_SIZE);
            }
            else if (w.side == DOCK_FLOAT)
                m_dock.FloatWindow(*v, w.pos);
        }

        // show find window if required
        PropertiesWindow* find_window = m_propData->displays.find_window();
        if (find_window->visible)
            showFindView(false);

        m_handlers.push_back( new MudViewHandler(&m_view, &m_history) );
        for (int i=0; i<OUTPUT_WINDOWS; ++i)
        {
            MudView *v = m_views[i];
            const OutputWindow& w =  output_windows->at(i);
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            ctx->rcWindow = w.pos;
            ctx->sizeFloat = w.size;
            ctx->LastSide = w.lastside;
            m_handlers.push_back(new MudViewHandler(v, NULL));
        }

        m_dock.SortPanes();
        onStart();
        m_parent.ShowWindow(SW_SHOW);

        PostMessage(WM_USER+2, main_window->fullscreen ? 1 :0);

        if (m_manager.isFirstStartup())
            PostMessage(WM_USER+3);

        SetTimer(1, 25);
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnFullScreen(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        if (wparam)
            m_parent.ShowWindow(SW_SHOWMAXIMIZED);
        else
            m_parent.ShowWindow(SW_SHOWNORMAL);
        return 0;
    }

    LRESULT OnShowWelcome(UINT, WPARAM, LPARAM, BOOL&)
    {
        // show help
        openHelp(m_parent, L"");
        return 0;
    }

    LRESULT OnParentClose(UINT, WPARAM, LPARAM lparam, BOOL&bHandled)
    {
        saveClientWindowPos();
        saveFindWindowPos();
        unloadPlugins();
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        pLoop->RemoveIdleHandler(this);

        KillTimer(1);
        std::for_each(m_handlers.begin(), m_handlers.end(), [](MudViewHandler *obj){ delete obj; });        
        for (int i=0,e=m_views.size(); i<e; ++i)
            delete m_views[i];
        for (int i = 0, e = m_plugins_views.size(); i<e; ++i)
            delete m_plugins_views[i];
        onClose();
        unloadModules();
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM lparam, BOOL& bHandled)
    {
        if (!m_view.IsWindow())
            return 0;
        RECT pos = { 0, 0, LOWORD(lparam), HIWORD(lparam) };
        m_hSplitter.MoveWindow(&pos);
        return 0;
    }

    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        return 1;
    }

    LRESULT OnWheel(UINT, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        if (checkKeysState(false, true, false))
        {
            if (m_history.IsWindowVisible()) {
                ::SendMessage(m_history, WM_MOUSEWHEEL, wparam, lparam);
                bool last = (m_history.getViewString() == m_history.getLastString());
                if (last)
                    closeHistory();
                return 0;
            }
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            if (delta > 0) {
                bool canscroll = (m_view.getStringsCount() > m_view.getStringsOnDisplay());
                if (canscroll) {
                    int vs = m_view.getViewString();
                    showHistory(vs-1, 1);
                }
            }
            return 0;
        }

        POINT pt; RECT rc;
        if (GetCursorPos(&pt))
        {
            for (int i=0,e=m_views.size(); i<e; ++i)
            {
                MudView *v = m_views[i];
                if (!v->IsWindowVisible())
                    continue;
                v->GetWindowRect(&rc);
                if (PtInRect(&rc, pt))
                {
                    ::SendMessage(*v, WM_MOUSEWHEEL, wparam, lparam);
                    return 0;
                }
            }
        }

        if (m_history.IsWindowVisible())
        {
            m_history.GetWindowRect(&rc);
            if (PtInRect(&rc, pt))
            {
                ::SendMessage(m_history, WM_MOUSEWHEEL, wparam, lparam);
                bool last = (m_history.getViewString() == m_history.getLastString());
                if (last)
                    closeHistory();
                return 0;
            }
        }

        m_view.GetWindowRect(&rc);
        if (PtInRect(&rc, pt))
            ::SendMessage(m_view, WM_MOUSEWHEEL, wparam, lparam);

        m_bar.GetWindowRect(&rc);
        if (PtInRect(&rc, pt))
            ::SendMessage(m_bar, WM_MOUSEWHEEL, wparam, lparam);

        return 0;
    }

    void setCmdBarFocus()
    {
        PostMessage(WM_USER+4);
    }

    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        setCmdBarFocus();
        return 0;
    }

    LRESULT OnBarSetFocus(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        m_bar.setFocus();
        return 0;
    }

    LRESULT OnActivateApp(UINT, WPARAM wparam, LPARAM, BOOL&bHandled)
    {
        m_activated = (wparam) ? true : false;
        m_plugins.processPluginsMethod(m_activated ? "activated" : "deactivated", 0);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnPlugins(WORD, WORD, HWND, BOOL&)
    {
        if (m_plugins.pluginsPropsDlg())
        {
            if (!m_manager.saveProfile())
                msgBox(m_hWnd, IDS_ERROR_SAVEPROFILE_FAILED, MB_OK | MB_ICONSTOP);
        }
        return 0;
    }

    LRESULT OnPluginMenuCmd(WORD, WORD id, HWND, BOOL&)
    {
        pluginsMenuCmd(id);
        return 0;
    }

    LRESULT OnNetwork(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        NetworkEvent event = m_network.translateEvent(lparam);
        if (event == NE_NEWDATA)
        {
            // msdp data
            MsdpNetwork* msdp = m_plugins.getMsdp();
            msdp->translateReceived(m_network.receivedMsdp());
            DataQueue& md = msdp->getSendData();
            if (md.getSize() > 0)
            {
                m_network.sendplain((tbyte*)md.getData(), md.getSize());
                md.clear();
            }

            // game data
            DataQueue &data = m_network.received();
            int text_len = data.getSize();

            if (text_len == 0)
                return 0;

            //OUTPUT_BYTES(data.getData(), text_len, text_len, "DATA");

            MemoryBuffer wide;
            if (m_codepage == CPWIN)
            {
                AnsiToWideConverter a2wc;
                a2wc.convert(&wide, (char*)data.getData(), text_len);
                data.clear();
            }
            else
            {
                Utf8ToWideConverter u2w;
                int converted = u2w.convert(&wide, (char*)data.getData(), text_len);
                data.truncate(converted);
            }

            if (m_propData->mode.plugins)
                m_plugins.processStreamData(&wide);
            const WCHAR* processeddata = (const WCHAR*)wide.getData();
            m_processor.processNetworkData(processeddata, wcslen(processeddata));
        }
        else if (event == NE_CONNECT)
        {
            m_processor.processNetworkConnect();
            m_plugins.processConnectEvent();
        }
        else if (event == NE_DISCONNECT)
        {
            m_network.disconnect();
            m_processor.processNetworkDisconnect();
            m_plugins.processDisconnectEvent();
        }
        else if (event == NE_ERROR)
        {
            m_network.disconnect();
            m_processor.processNetworkError();
            m_plugins.processDisconnectEvent();
        }
        else if (event == NE_ERROR_CONNECT)
        {
            m_network.disconnect();
            m_processor.processNetworkConnectError();
            m_plugins.processDisconnectEvent();
        }
        else if (event == NE_ERROR_MCCP)
        {
            m_network.disconnect();
            m_processor.processNetworkMccpError();
            m_plugins.processDisconnectEvent();
        }
        return 0;
    }

    void disconnectFromNetwork()
    {
        m_network.disconnect();
        m_plugins.processDisconnectEvent();
    }

    void sendToNetwork(const tstring& data)
    {
        MemoryBuffer buffer;
        if (m_codepage == CPWIN)
        {
            WideToAnsiConverter w2a;
            w2a.convert(&buffer, data.c_str(), data.length());
        }
        else
        {
            WideToUtf8Converter w2u;
            w2u.convert(&buffer, data.c_str(), data.length());
        }
        m_network.send((tbyte*)buffer.getData(), buffer.getSize() - 1); // don't send last byte(0) of string
    }

    LRESULT OnTimer(UINT, WPARAM id, LPARAM, BOOL&)
    {
        static int count = 0;
        count = count + 1;
        if (count == 8)
        {
            count = 0;
            m_processor.processTick();
            m_plugins.processTick();
        }

        static int seconds_count = 0;
        seconds_count = seconds_count + 1;
        if (seconds_count == 40)
        {
            seconds_count = 0;
            if (m_processor.getConnectionState())
              m_plugins.processSecondTick();
            m_plugins.processDebugTick();
        }

        m_processor.processStackTick();
        m_view.updateSoftScrolling();
        for (int i=0,e=m_views.size();i<e;++i)
           m_views[i]->updateSoftScrolling();
        return 0;
    }

    void processUserCommand(const tstring& cmd, bool process_history)
    {
        InputPlainCommands cmds(cmd);
        int count = cmds.size();
        if (count > 1)
        {
            int last = cmd.size() - 1;
            if (last > 0)
            {
                tchar last_char = cmd.at(last);
                if (last_char != L'\r' && last_char != L'\n')
                {
                    int last_cmd = cmds.size() - 1;
                    m_bar.setText(cmds[last_cmd], -1, false);
                    cmds.erase(last_cmd);
                }
            }
        }
        else if (count == 1)
        {
            if (process_history) {
            InputPlainCommands history;
            if (m_propData->mode.plugins)
                m_plugins.processHistoryCmds(cmds, &history);
            for (int i=0,e=history.size(); i<e; ++i)
                m_bar.addToHistory(history[i]);
            }
        }
        else
        {
            assert(false);
            return;
        }

        InputTemplateParameters p;
        p.prefix = m_propData->cmd_prefix;
        p.separator = m_propData->cmd_separator;

        // разбиваем команду на подкомманды для обработки в barcmd
        InputTemplateCommands tcmds;
        tcmds.init(cmds, p);
        tcmds.extract(&cmds);

        if (m_propData->mode.plugins)
            m_plugins.processBarCmds(&cmds);

        m_processor.processUserCommand(cmds);
    }

    LRESULT OnUserCommand(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        tstring cmd;
        m_bar.getCommand(&cmd);
        processUserCommand(cmd, true);
        return 0;
    }

    LRESULT OnCopyData(UINT, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        tstring window, cmd;
        if (readCommandToWindow(wparam, lparam, &window, &cmd))
        {
            PropertiesManager *pmanager = tortilla::getPropertiesManager();
            if (window.empty() || window == pmanager->getProfileName())
                processUserCommand(cmd, false);
        }
        return 0;
    }

    void initCommandBar()
    {
        m_barHeight = m_propElements.font_height + 4 + 7; // +7 - доп. высота, чтобы Пуск не наезжал на буквы когда окно максимизировано
        m_bar.setParams(m_barHeight, m_propElements.standard_font);
        m_dock.SetStatusBarHeight(m_barHeight);
        RECT pos; GetClientRect(&pos);
        m_hSplitter.MoveWindow(&pos);
    }

    LRESULT OnNewProfile(WORD, WORD, HWND, BOOL&)
    {
        onNewProfile();
        return 0;
    }

    LRESULT OnLoadProfile(WORD, WORD, HWND, BOOL&)
    {
        onLoadProfile();
        return 0;
    }

    LRESULT OnNewWorld(WORD, WORD, HWND, BOOL&)
    {
        onNewWorld();
        return 0;
    }

    LRESULT OnMode(WORD, WORD, HWND, BOOL&)
    {
        ModeDlg dlg(m_propData);
        if (dlg.DoModal() == IDOK)
        {
            m_plugins.processPluginsMethod("compsupdated", 0);
        }
        return 0;
    }

    LRESULT OnSettings(WORD, WORD, HWND, BOOL&)
    {
        m_settings_mode = true;
        m_plugins.processPluginsMethod("propsblocked", 0);

        PropertiesData& data = *m_manager.getConfig();
        PropertiesData tmp;
        tmp.copy(data);

        bool font_updated = false;
        PropertiesDlg propDlg(&tmp);
        if (propDlg.DoModal() == IDOK)
        {
            if (data.font_name != tmp.font_name || data.font_bold != tmp.font_bold ||
                data.font_heigth != tmp.font_heigth || data.font_italic != tmp.font_italic)
            {
                font_updated = true;
            }
            data.copy(tmp);
            updateProps();
            if (!m_manager.saveProfile())
                msgBox(m_hWnd, IDS_ERROR_SAVEPROFILE_FAILED, MB_OK|MB_ICONSTOP);
        }
        else
        {
            data.dlg = tmp.dlg;
        }
        m_settings_mode = false;
        m_plugins.processPluginsMethod("propsupdated", 0);
        if (font_updated)
            m_plugins.processPluginsMethod("fontupdated", 0);
        return 0;
    }

    LRESULT OnShowWindow(WORD, WORD control_id, HWND, BOOL&)
    {
        int id = (control_id - ID_WINDOW_1) + 1;
        assert(id >=1 && id <=OUTPUT_WINDOWS);
        HWND wnd = m_views[id-1]->m_hWnd;
        bool new_state = !isWindowShown(wnd);
        showWindowEx(wnd, new_state);
        setCmdBarFocus();
        return 0;
    }

    void showFindView(bool set_focus)
    {
        PropertiesWindow *find_window = m_propData->displays.find_window();
        DOCKCONTEXT *ctx = m_dock._GetContext(m_find_dlg);
        if (ctx->Side == DOCK_HIDDEN)
        {
            RECT& p = find_window->pos;
            SIZE sz =  m_find_dlg.getSize();
            int w = 0; int h = 0;
            addWindowBorder(w, h);
            p.right = p.left + sz.cx + w;
            p.bottom = p.top + sz.cy + h;
            setFixedSize(m_find_dlg, p.right, p.bottom); // - p.left, p.bottom - p.top);
            m_dock.FloatWindow(m_find_dlg, p);
        }
        m_parent.SendMessage(WM_USER, ID_VIEW_FIND, 1);
        find_window->visible = true;
        if (set_focus)
            m_find_dlg.setFocus();
    }

    void hideFindView()
    {
        PropertiesWindow *find_window = m_propData->displays.find_window();
        find_window->visible = false;
        DOCKCONTEXT *ctx = m_dock._GetContext(m_find_dlg);
        //m_propData->find_window = ctx->rcWindow;
        m_dock.HideWindow(m_find_dlg);
        m_parent.SendMessage(WM_USER, ID_VIEW_FIND, 0);
        if (m_last_find_view == 0)
            m_history.clearFind();
        else if (m_last_find_view > 0) {
            MudView *v = m_views[m_last_find_view-1];
            v->clearFind();
        }
        m_last_find_view = -1;
    }

    LRESULT OnViewFind(WORD, WORD, HWND, BOOL&)
    {
        PropertiesWindow *find_window = m_propData->displays.find_window();
        if (find_window->visible) { hideFindView(); }
        else { showFindView(true); }
        return 0;
    }

    LRESULT OnCloseWindow(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        HWND wnd = (HWND)wparam;
        if (wnd == m_find_dlg)
        {
            hideFindView();
            return 0;
        }
        for (int i = 0, e = m_plugins_views.size(); i < e; ++i)
        {
            HWND hwnd = *m_plugins_views[i];
            if (hwnd == wnd)
            {
                savePluginWindowPos(hwnd);
                Plugin *p = m_plugins.findPlugin(wnd);
                assert(p);
                if (p) 
                    p->closeWindow(wnd);
                setCmdBarFocus();
                return 0;
            }
        }
        showWindowEx(wnd, false);
        setCmdBarFocus();
        return 0;
    } 

    bool processKey(int vkey)
    {
        if ((vkey == VK_UP || vkey == VK_DOWN) && checkKeysState(false, true, false))
        {
            BOOL hv = m_history.IsWindowVisible();
            if (vkey == VK_UP && !hv)
            {
                int vs = m_view.getViewString();
                if (vs > 1)
                    showHistory(vs-1, 1);
                return true;
            }
            if (vkey == VK_DOWN && !hv)
                return true;
            MudView &view = m_history;
            int visible_string = view.getViewString();
            if (vkey == VK_UP)
                visible_string -= 1;
            else
                visible_string += 1;
            if (vkey == VK_DOWN)
            {
                int next_last = view.getLastString() + 1;
                if (next_last == visible_string)
                {
                    closeHistory();
                    return true;
                }
            }
            view.setViewString(visible_string);
            return true;
        }

        if ((vkey == VK_PRIOR || vkey == VK_NEXT) && checkKeysState(false, false, false)) // PAGEUP & PAGEDOWN
        {
            if (vkey == VK_PRIOR && !m_history.IsWindowVisible())
            {
                int vs = m_view.getViewString();
                int page = m_view.getStringsOnDisplay();
                if (vs > page)
                    showHistory(vs-page, -1);
                return true;
            }

            BOOL hv = m_history.IsWindowVisible();
            MudView &view = (hv) ? m_history : m_view;
            int visible_string = view.getViewString();
            int page = view.getStringsOnDisplay();
            if (vkey == VK_PRIOR)
                visible_string -= page;
            else
                visible_string += page;
            if (vkey == VK_NEXT)
            {
                int next_last = view.getLastString() + 1;
                if (visible_string >= next_last)
                {
                    if (hv)
                    {
                        closeHistory();
                        return true;
                    }
                    visible_string = next_last;
                }
            }
            view.setViewString(visible_string);
            return true;
        }
        return false;
    }

    void updateProps()
    {
       m_propElements.updateProps(m_hWnd);
       initCommandBar();
       m_view.updateProps();
       m_view.setSoftScrollingMode(m_propData->soft_scroll ? true : false);
       m_history.updateProps();
       for (int i=0,e=m_views.size(); i<e; ++i)
           m_views[i]->updateProps();
       updateTitle();
       m_processor.updateProps();
       m_network.setSendDoubleIACmode(m_propData->disable_ya ? false : true);
       m_plugins.updateProps();
       if (m_propData->codepage == L"utf8") m_codepage = CPUTF8;
       else m_codepage = CPWIN;
       bool utf8_encoding = m_codepage == CPUTF8 ? true : false;
       m_network.setUtf8Encoding(utf8_encoding);
       m_plugins.getMsdp()->setUtf8Encoding(utf8_encoding);
    }

    void updateTitle()
    {
        tstring title;
        title.append(m_manager.getProfileGroup());
        title.append(L" - ");
        title.append(m_manager.getProfileName());
        title.append(L" - ");
        tstring appname;
        loadString(IDR_MAINFRAME, &appname);
        appname.append(L" v");
        appname.append(TORTILLA_VERSION);
        title.append(appname);
        ::SetWindowText(m_dock.GetParent(), title.c_str());
        m_propData->title = title;
    }

    // LogicProcessorListener
    void connectToNetwork(const tstring& address, int port)
    {
        WideToAnsi w2a(address.c_str());
        m_networkData.address = w2a;
        m_networkData.port = port;
        m_networkData.notifyMsg = WM_USER+1;
        m_networkData.wndToNotify = m_hWnd;
        m_network.connect(m_networkData);
    }

    bool saveViewData(int view, tstring& filename)
    {
        if (view >= 0 && view <= OUTPUT_WINDOWS)
        {
            PropertiesData* pdata = tortilla::getProperties();
            MudView *v = (view == 0) ? &m_view : m_views[view-1];
            LogsFormatter *f = NULL;
            int pos = filename.rfind(L'.');
            if (pos != -1) {
              tstring ext(filename.substr(pos + 1));
              if (ext == L"txt") f = new LogsFormatterTxt(pdata);
              if (ext == L"htm" || ext == L"html") f = new LogsFormatterHtml(pdata);
            }
            if (!f) {
                if (pdata->logformat == L"txt")
                    f = new LogsFormatterTxt(pdata);
                else
                    f = new LogsFormatterHtml(pdata);
                f->normFilename(filename);
            }
            bool result = false;
            f->checkExist(filename);
            if (f->open(filename, LogsFormatter::PM_NEW) && f->prepare())
            {
                for (int i=0,e=v->getStringsCount();i<e;++i) {
                    f->writeString(v->getString(i));
                }
                f->close();
                result = true;
            }
            delete f;
            return result;
        }
        return false;
    }

    MudViewString* getLastString(int view)
    {
        MudViewString *s = NULL;
        MudView* v = NULL;
        if (view == 0)
            v = &m_view;
        if (view >= 1 && view <= OUTPUT_WINDOWS)
            v = m_views[view - 1];
        if (v) {
            int last = v->getStringsCount() - 1;
            s = (last > 0) ? v->getString(last) : NULL;
        }
        return s;
    }

    void accLastString(int view, parseData* parse_data)
    {
        if (view == 0)
            m_view.accLastString(parse_data);
        else if (view >= 1 && view <= OUTPUT_WINDOWS)
        {
            MudView* v = m_views[view - 1];
            v->accLastString(parse_data);
        }
    }

    void preprocessText(int view, parseData* parse_data)
    {
        m_plugins.processViewData("before", view, parse_data);
    }

    void postprocessText(int view, parseData* parse_data)
    {
        m_plugins.processViewData("after", view, parse_data);
    }

    PluginsTriggersHandler* getPluginsTriggers() 
    {
        return &m_plugins;
    }

    void clearDropped(int view)
    {
        if (view == 0)
            m_view.clearDropped();
        if (view >= 1 && view <= OUTPUT_WINDOWS)
        {
            MudView* v = m_views[view-1];
            v->clearDropped();
        }
    }
    void loadProfile(const tstring& name, const tstring& group, tstring* error);
    
    void addText(int view, parseData* parse_data)
    {
        if (view == 0)
        {
            int vs = m_view.getViewString();
            bool last = (m_view.getViewString() == m_view.getLastString());

            parseData history;
            bool in_soft_scrolling = m_view.inSoftScrolling();
            int limited = 0;

            bool last_deleted = m_view.lastStringDeleted();
            m_view.addText(parse_data, &history, &limited);
            bool empty = history.strings.empty();
            m_history.pushText(&history, last_deleted);
            if (empty)
                return;
            vs = vs - limited;
            checkHistorySize();
            bool history_visible = m_history.IsWindowVisible() ? true : false;
            bool soft_scroll = m_propData->soft_scroll ? true : false;
            if (!history_visible && !last)
            {
                if (!soft_scroll || !in_soft_scrolling)
                {
                    bool skip_history = false;
                    if (m_view.isDragMode())
                    {
                        m_drag_flag = true;
                        return;
                    }
                    else
                    {
                        if (m_drag_flag) { m_drag_flag = false; skip_history = true; }
                    }
                    if (!skip_history)
                    {
                        showHistory(vs, 1);
                    }
                    if (soft_scroll || skip_history) {
                      int last = m_view.getLastString();
                      m_view.setViewString(last);
                    }
                }
            }
            else if (history_visible)
            {
                int vs = m_history.getViewString();
                m_history.setViewString(vs);
            }
            return;
        }
        if (view >= 1 && view <= OUTPUT_WINDOWS)
        {
            MudView* v = m_views[view-1];
            v->addText(parse_data, NULL);
        }
    }

    void clearText(int view)
    {
        if (view == 0)
        {
           m_view.clearText();
           m_history.clearText();
        }
        else if (view >= 1 && view <= OUTPUT_WINDOWS)
        {
            MudView* v = m_views[view-1];
            v->clearText();
        }
    }

    void showWindow(int view, bool show)
    {
        assert(view >=1 && view <= OUTPUT_WINDOWS);
        showWindowEx(*m_views[view-1], show);
    }

    bool isWindowVisible(int view)
    {
        assert(view >=1 && view <= OUTPUT_WINDOWS);
        MudView* v = m_views[view-1];
        return isWindowShown(*v);
    }

    void setWindowName(int view, const tstring& name)
    {
        assert(view >=1 && view <= OUTPUT_WINDOWS);
        MudView* v = m_views[view-1];
        m_dock.SetWindowName(*v, name.c_str());
        v->SetWindowText(name.c_str());
        int menu_id = view-1+ID_WINDOW_1;
        m_parent.SendMessage(WM_USER+1, menu_id, (WPARAM)name.c_str());
        m_find_dlg.setWindowName(view, name);
    }

    void showWindowEx(HWND wnd, bool show)
    {
        int id = -1;
        for (int i=0,e=m_views.size(); i<e; ++i)
        {
            HWND w = *m_views[i];
            if (wnd == w)
                { id = i+1; break; }
        }
        if (id == -1) {
            assert(false);  return;
        }

        int menu_id = id-1+ID_WINDOW_1;
        if (show)
        {
            if (!isWindowShown(wnd))
                { m_dock.ShowWindow(wnd); m_parent.SendMessage(WM_USER, menu_id, 1);  setCmdBarFocus(); }
        }
        else
        {
            if (isWindowShown(wnd))
                { m_dock.HideWindow(wnd); m_parent.SendMessage(WM_USER, menu_id, 0); setCmdBarFocus(); }
        }
    }

    bool isWindowShown(HWND wnd)
    {
        DOCKCONTEXT *ctx = m_dock._GetContext(wnd);
        assert(ctx != NULL);
        return (ctx->Side != DOCK_HIDDEN) ? true : false;
    }

    void getMccpStatus(MccpStatus *status)
    {
        m_network.getMccpStatus(status);
    }

    HWND getMainWindow()
    {
        return m_parent;
    }

    void preprocessCommand(InputCommand cmd);

    void checkHistorySize()
    {
        int hs = m_propData->view_history_size;
        if (!m_history.IsWindowVisible())
            m_history.truncateStrings(hs);
        else
        {
            int count = m_history.getLastString();
            int maxcount = (hs*3);
            if (maxcount > TOTAL_MAX_VIEW_HISTORY_SIZE)
                maxcount = TOTAL_MAX_VIEW_HISTORY_SIZE;
            if (count > maxcount)
                closeHistory();
        }
    }

    void closeHistory()
    {
        m_hSplitter.SetSinglePaneMode(SPLIT_PANE_BOTTOM);
        if (m_last_find_view == 0)
        {
            m_history.clearFind();
            m_last_find_view = -1;
        }
        m_history.truncateStrings(m_propData->view_history_size);        
    }

    void showHistory(int vs, int dt)
    {
        CDC dc(m_view.GetDC());
        HFONT current_font = dc.SelectFont(m_propElements.standard_font);
        SIZE sz = { 0, 0 };
        GetTextExtentPoint32(dc, L"W", 1, &sz);         // sz.cy = height of font
        dc.SelectFont(current_font);

        RECT rc; m_hSplitter.GetClientRect(&rc);
        int lines0 = rc.bottom / sz.cy;
        int dy0 = rc.bottom - (lines0 * sz.cy);
        int curpos = m_hSplitter.GetSplitterPos();
        int lines = curpos / sz.cy;
        curpos = dy0 + (lines * sz.cy);
        vs = vs - (lines0 - lines) * dt + (m_history.getLastString() - m_view.getLastString());

        m_hSplitter.SetSplitterPos(curpos);
        m_hSplitter.SetSinglePaneMode(SPLIT_PANE_NONE);
        m_history.setViewString(vs);
    }

    void saveFindWindowPos()
    {
        DOCKCONTEXT *ctx = m_dock._GetContext(m_find_dlg);
        PropertiesWindow *find_window = m_propData->displays.find_window();
        find_window->pos = ctx->rcWindow;
        find_window->visible = (ctx->Side == DOCK_FLOAT) ? true : false;
    }

    void saveClientWindowPos()
    {
        OutputWindowsCollection* windows = m_propData->displays.output_windows();
        tstring buffer;
        for (int i = 0, e = m_views.size(); i<e; ++i)
        {
            MudView *v = m_views[i];
            getWindowText(*v, &buffer);
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            OutputWindow& w = windows->at(i);
            w.name = tstring(buffer);
            w.side = ctx->Side;
            w.lastside = ctx->LastSide;
            w.pos = ctx->rcWindow;
            w.size = ctx->sizeFloat;
        }

        WINDOWPLACEMENT wp;
        m_parent.GetWindowPlacement(&wp);
        PropertiesWindow *main_window = m_propData->displays.main_window();
        main_window->pos = wp.rcNormalPosition;
        main_window->fullscreen = (wp.showCmd == SW_SHOWMAXIMIZED) ? true : false;
        saveFindWindowPos();
    }

    void loadClientWindowPos()
    {
        m_parent.ShowWindow(SW_RESTORE);
        PropertiesWindow *main_window = m_propData->displays.main_window();
        m_parent.MoveWindow(&main_window->pos);

        for (int i = 0; i<OUTPUT_WINDOWS; ++i)
        {
            MudView *v = m_views[i];
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            int menu_id = i + ID_WINDOW_1;
            if (IsFloating(ctx->Side))
                m_dock._UnFloatWindow(ctx);
            else if (IsDocked(ctx->Side))
                m_dock._UnDockWindow(ctx);
        }

        // order for creating windows
        std::vector<int> sides = { DOCK_TOP, DOCK_BOTTOM, DOCK_LEFT, DOCK_RIGHT, DOCK_FLOAT, DOCK_HIDDEN };

        // recreate docking output windows
        OutputWindowsCollection* windows = m_propData->displays.output_windows();
        for (int j=0,je=sides.size(); j<je; ++j)
        {
            typedef std::pair<int,int> wd;
            std::vector<wd> wds;
            for (int i=0; i < OUTPUT_WINDOWS; ++i)
            {
                const OutputWindow& w =  windows->at(i);
                if (w.side == sides[j]) {
                  wd d; d.first = i; d.second = 0;
                  if (IsDockedVertically(w.side)) d.second = w.pos.top;
                  if (IsDockedHorizontally(w.side)) d.second = w.pos.left;
                  wds.push_back(d);
                }
            }
            struct { bool operator() (const wd& i,const wd& j) { return (i.second<j.second);}} comparator;
            std::sort(wds.begin(), wds.end(), comparator);

            for (int i=0,e=wds.size(); i<e; ++i)
            {
                int index = wds[i].first;
                MudView *v = m_views[index];
                const OutputWindow& w = windows->at(index);

                int menu_id = index + ID_WINDOW_1;
                if (IsDocked(w.side))
                {
                    m_dock.DockWindow(*v, w.side);
                    int size = IsDockedVertically(w.side) ? w.pos.right-w.pos.left : w.pos.bottom-w.pos.top;
                    int border = IsDockedVertically(w.side) ? m_dock.m_sizeBorder.cx : m_dock.m_sizeBorder.cy;
                    m_dock.SetPaneSize(w.side, size+border*2);
                    m_parent.SendMessage(WM_USER, menu_id, 1);
                }
                else if (w.side == DOCK_FLOAT)
                {
                    m_dock.FloatWindow(*v, w.pos);
                    m_parent.SendMessage(WM_USER, menu_id, 1);
                }
                else
                {
                    m_parent.SendMessage(WM_USER, menu_id, 0);
                }
            }
        }
        for (int i = 0; i<OUTPUT_WINDOWS; ++i)
        {
            MudView *v = m_views[i];
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            const OutputWindow& w =  windows->at(i);
            ctx->Side = w.side;
            ctx->LastSide = w.lastside;
            ctx->rcWindow = w.pos;
            ctx->sizeFloat = w.size;
            ctx->bKeepSize = false;
        }

        // find window
        PropertiesWindow *find_window = m_propData->displays.find_window();
        bool state = find_window->visible;
        hideFindView();
        m_dock.UpdatePanes();
        if (state)
            showFindView(false);

        // maximize main window
        PostMessage(WM_USER+2, main_window->fullscreen ? 1 :0);
    }

    void savePluginWindowPos(HWND wnd = NULL)
    {
        PluginsDataValues* pdv =  tortilla::pluginsData();
        for (int i = 0, e = m_plugins_views.size(); i < e; ++i)
        {
            PluginsView *v = m_plugins_views[i];
            if (wnd == NULL || *v == wnd)
            {
                DOCKCONTEXT *ctx = m_dock._GetContext(*v);
                OutputWindow w;
                getWindowText(*v, &w.name);
                w.side = ctx->Side;
                w.lastside = ctx->LastSide;
                w.pos = ctx->rcWindow;
                w.size = ctx->sizeFloat;
                Plugin *p = v->getPlugin();
                pdv->saveWindowPos(p->get(Plugin::FILE), w);
            }
        }
    }

    void setOscColor(int index, COLORREF color);
    void resetOscColors();
};
