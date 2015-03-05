#pragma once

#include "propertiesPages/propertiesDlg.h"
#include "propertiesPages/propertiesManager.h"
#include "profiles/profileDlgs.h"

#include "network/network.h"
#include "mudView.h"
#include "mudCommandBar.h"
#include "logicProcessor.h"

#include "plugins/pluginsApi.h"
#include "plugins/pluginsView.h"
#include "plugins/pluginsManager.h"

#include "AboutDlg.h"

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
    MudView m_history;
    MudView m_view;
    CSplitterWindowExT<false, 3, 1> m_hSplitter;

    NetworkConnectData m_networkData;
    Network m_network;
    HotkeyTable m_hotkeyTable;
    LogicProcessor m_processor;
    std::vector<MudView*> m_views;
    std::vector<PluginsView*> m_plugins_views;
    PluginsManager m_plugins;
    int m_codepage;

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
        m_barHeight(32), m_bar(m_propData),
        m_view(&m_propElements), m_history(&m_propElements),
        m_processor(m_propData, this), m_plugins(m_propData), 
        m_codepage(CPWIN)
    {
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        UINT msg = pMsg->message;
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
            if (processKey(pMsg->wParam))
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

    PluginsView* createPanel(const PanelWindow& w, Plugin* p)
    {
        PluginsView *v = new PluginsView(p);
        v->Create(m_dock, rcDefault, L"", WS_DEFCHILD|WS_VISIBLE);
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
        v->Create(m_dock, rcDefault, w.name.c_str(), WS_DEFCHILD, WS_EX_STATICEDGE);
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

    void hideDockPane(PluginsView* v)
    {
        HWND hwnd = v->m_hWnd;
        m_dock.HideWindow(hwnd);
    }

    void showDockPane(PluginsView* v)
    {
        HWND hwnd = v->m_hWnd;
        m_dock.ShowWindow(hwnd);
    }

    LogicProcessorMethods *getMethods() { return &m_processor; }
    PropertiesData *getPropData() { return m_propData;  }
    CFont *getStandardFont() { return &m_propElements.standard_font; }
    PropertiesManager* getPropManager() { return &m_manager; }
    Palette256* getPalette() { return &m_propElements.palette;  }
    int convertSideFromString(const wchar_t* side) { return m_dock.GetSideByString(side); }

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
        MESSAGE_HANDLER(WM_USER+4, OnSetFocus)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
    ALT_MSG_MAP(1)  // retranslated from MainFrame
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_CLOSE, OnParentClose)
        MESSAGE_HANDLER(WM_MOUSEWHEEL, OnWheel)
        COMMAND_ID_HANDLER(ID_NEWPROFILE, OnNewProfile)
        COMMAND_ID_HANDLER(ID_LOADPROFILE, OnLoadProfile)
        COMMAND_ID_HANDLER(ID_NEWWORLD, OnNewWorld)
        COMMAND_ID_HANDLER(ID_SETTINGS, OnSettings)
        COMMAND_RANGE_HANDLER(ID_WINDOW_1, ID_WINDOW_6, OnShowWindow)
        MESSAGE_HANDLER(WM_DOCK_PANE_CLOSE, OnCloseWindow)
        MESSAGE_HANDLER(WM_DOCK_FOCUS, OnSetFocus)
        COMMAND_ID_HANDLER(ID_PLUGINS, OnPlugins)
        COMMAND_RANGE_HANDLER(PLUGING_MENUID_START, PLUGING_MENUID_END, OnPluginMenuCmd)
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

        m_parent.MoveWindow(&m_propData->main_window);

        // create docking output windows
        for (int i=0; i < OUTPUT_WINDOWS; ++i)
        {
            const OutputWindow& w =  m_propData->windows[i];
            MudView *v = new MudView(&m_propElements);
            DWORD style = WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE;
            int menu_id = i+ID_WINDOW_1;
            if (w.side != DOCK_HIDDEN)
                m_parent.SendMessage(WM_USER, menu_id, 1);
            m_parent.SendMessage(WM_USER+1, menu_id, (WPARAM)w.name.c_str());

            v->Create(m_dock, rcDefault, w.name.c_str(), style, WS_EX_CLIENTEDGE);
            m_views.push_back(v);
            m_dock.AddWindow(*v);
            if (IsDocked(w.side))
            {
                m_dock.DockWindow(*v, w.side);
                int size = IsDockedVertically(w.side) ? w.pos.right : w.pos.bottom;
                m_dock.SetPaneSize(w.side, size);
            }
            else if (w.side == DOCK_FLOAT)
            {
                m_dock.FloatWindow(*v, w.pos);
            }
        }

        for (int i=0; i<OUTPUT_WINDOWS; ++i)
        {
            MudView *v = m_views[i];
            const OutputWindow& w =  m_propData->windows[i];
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            ctx->rcWindow = w.pos;
            ctx->sizeFloat = w.size;
            ctx->LastSide = w.lastside;
        }

        m_dock.SortPanes();
        onStart();
        m_parent.ShowWindow(SW_SHOW);
        if (m_propData->main_window_fullscreen)
            PostMessage(WM_USER+2);

        if (m_propElements.global.welcome)
        {
            m_propElements.global.welcome = 0;
            PostMessage(WM_USER + 3);
        }

        SetTimer(1, 200);
        SetTimer(2, 50);
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnFullScreen(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_parent.ShowWindow(SW_SHOWMAXIMIZED);
        return 0;
    }

    LRESULT OnShowWelcome(UINT, WPARAM, LPARAM, BOOL&)
    {
        CWelcomeDlg dlg;
        dlg.DoModal();
        return 0;
    }

    LRESULT OnParentClose(UINT, WPARAM, LPARAM lparam, BOOL&bHandled)    
    {
        saveClientWindowPos();
        savePluginWindowPos();
        unloadPlugins();
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        pLoop->RemoveIdleHandler(this);

        KillTimer(2);
        KillTimer(1);
        for (int i=0,e=m_views.size(); i<e; ++i)
            delete m_views[i];
        for (int i = 0, e = m_plugins_views.size(); i<e; ++i)
            delete m_plugins_views[i];
        onClose();
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
                return 0;
            }
        }

        m_view.GetWindowRect(&rc);
        if (PtInRect(&rc, pt))
            ::SendMessage(m_view, WM_MOUSEWHEEL, wparam, lparam);

        return 0;
    }

    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_bar.setFocus();
        return 0;
    }

    LRESULT OnPlugins(WORD, WORD, HWND, BOOL&)
    {
        m_plugins.pluginsPropsDlg();
        return 0;
    }

    LRESULT OnPluginMenuCmd(WORD, WORD id, HWND, BOOL&)
    {
        pluginsMenuCmd(id);
        return 0;
    }

    LRESULT OnNetwork(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        NetworkEvents result = m_network.processMsg(lparam);
        if (result == NE_NEWDATA)
        {
            DataQueue* data = m_network.receive();
            int text_len = data->getSize();
            if (text_len == 0)
                return 0;

            MemoryBuffer wide;
            if (m_codepage == CPWIN)
            {
                AnsiToWideConverter a2wc;
                a2wc.convert(&wide, (char*)data->getData(), text_len);
            }
            else
            {
                Utf8ToWideConverter u2w;
                u2w.convert(&wide, (char*)data->getData(), text_len);
            }
            data->truncate(text_len);

            m_plugins.processStreamData(&wide);
            const WCHAR* processeddata = (const WCHAR*)wide.getData();
            m_processor.processNetworkData(processeddata, wcslen(processeddata));
        }
        else if (result == NE_CONNECT)
        {
            m_processor.processNetworkConnect();
        }
        else if (result == NE_DISCONNECT)
        {
            m_network.disconnect();
            m_processor.processNetworkDisconnect();
        }
        else if (result == NE_ERROR)
        {
            m_network.disconnect();
            m_processor.processNetworkError();
        }
        else if (result == NE_ERROR_CONNECT)
        {
            m_network.disconnect();
            m_processor.processNetworkConnectError();
        }
        else if (result == NE_ERROR_MCCP)
        {
            m_network.disconnect();
            m_processor.processNetworkMccpError();
        }
        return 0;
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
        if (id == 1)
        {
            m_processor.processTick();
            if (m_history.IsWindowVisible() && m_history.isLastString())
            {
               m_hSplitter.SetSinglePaneMode(SPLIT_PANE_BOTTOM);
               m_history.truncateStrings(m_propData->view_history_size);
            }
        }
        else if (id == 2)
        {
            m_processor.processStackTick();
        }
        return 0;
    }

    LRESULT OnUserCommand(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        tstring cmd;
        m_bar.getCommand(&cmd);
        m_plugins.processBarCmd(&cmd);
        tstring history(cmd);
        m_plugins.processHistoryCmd(&history);
        m_bar.addToHistory(history);
        m_processor.processCommand(cmd);
        return 0;
    }

    void initCommandBar()
    {
        m_barHeight = m_propElements.font_height + 4;
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

    LRESULT OnSettings(WORD, WORD, HWND, BOOL&)
    {
        PropertiesData& data = *m_manager.getConfig();
        PropertiesData tmp(data);
        PropertiesDlg propDlg(&tmp);
        if (propDlg.DoModal() == IDOK)
        {
            data = tmp;
            updateProps();
            if (!m_manager.saveProfile())
                msgBox(m_hWnd, IDS_ERROR_SAVEPROFILE_FAILED, MB_OK|MB_ICONSTOP);
        }
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

    LRESULT OnCloseWindow(UINT, WPARAM wparam, LPARAM, BOOL&)
    {
        HWND wnd = (HWND)wparam;
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
        if (vkey == VK_PRIOR || vkey == VK_NEXT) // PAGEUP & PAGEDOWN
        {
            MudView &view = (m_history.IsWindowVisible()) ? m_history : m_view;
            int visible_string = view.getViewString();
            int page = view.getStringsOnDisplay();
            if (vkey == VK_PRIOR)
                visible_string -= page;
            else
                visible_string += page;
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
       m_history.updateProps();
       for (int i=0,e=m_views.size(); i<e; ++i)
           m_views[i]->updateProps();
       updateTitle();
       m_processor.updateProps();
       m_network.setSendDoubleIACmode(m_propData->disable_ya ? false : true);
       m_plugins.updateProps();
       if (m_propData->codepage == L"utf8") m_codepage = CPUTF8;
       else m_codepage = CPWIN;
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
        if (!m_network.connect(m_networkData))
            m_processor.processNetworkConnectError();
    }

    void setCmdBarFocus()
    {
        PostMessage(WM_USER + 4);
    }

    void disconnectFromNetwork()
    {
        m_network.disconnect();
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

    void addText(int view, parseData* parse_data)
    {
        if (parse_data->strings.empty())
            return;
        if (view == 0)
        {
            bool last = m_view.isLastString();
            int vs = m_view.getViewString();
            m_view.addText(parse_data, &m_history);
            checkHistorySize();

            if (!m_history.IsWindowVisible() && !last)
            {
                CDC dc(m_view.GetDC());
                HFONT current_font = dc.SelectFont(m_propElements.standard_font);
                SIZE sz = {0,0};
                GetTextExtentPoint32(dc, L"W", 1, &sz);         // sz.cy = height of font
                dc.SelectFont(current_font);

                RECT rc; m_hSplitter.GetClientRect(&rc);
                int lines0 = rc.bottom / sz.cy;
                int dy0 = rc.bottom - (lines0 * sz.cy);
                int curpos = m_hSplitter.GetSplitterPos();
                int lines = curpos / sz.cy;
                curpos = dy0 + (lines * sz.cy);
                vs = vs - (lines0 - lines) + (m_history.getLastString() - m_view.getLastString());

                m_hSplitter.SetSplitterPos(curpos);
                m_hSplitter.SetSinglePaneMode(SPLIT_PANE_NONE);
                m_history.setViewString(vs);
            }
            else
            {
                int vs = m_history.getViewString();
                m_history.setViewString(vs);
            }
            return;
        }
        if (view >= 1 && view <= OUTPUT_WINDOWS)
        {
            MudView* v = m_views[view-1];
            v->addText(parse_data);
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

    void setWindowName(int view, const tstring& name)
    {
        assert(view >=1 && view <= OUTPUT_WINDOWS);
        MudView* v = m_views[view-1];
        m_dock.SetWindowName(*v, name.c_str());
        v->SetWindowText(name.c_str());
        int menu_id = view-1+ID_WINDOW_1;
        m_parent.SendMessage(WM_USER+1, menu_id, (WPARAM)name.c_str());
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
        m_network.getMccpRatio(status);
    }

    HWND getMainWindow()
    {
        return m_parent;
    }

    void preprocessGameCmd(tstring* cmd);

    void checkHistorySize()
    {
        int hs = m_propData->view_history_size;
        if (!m_history.IsWindowVisible())
            m_history.truncateStrings(hs);
        else
        {
            int count = m_history.getLastString();
            int maxcount = (hs*3);
            if (maxcount > MAX_VIEW_HISTORY_SIZE) 
                    maxcount = MAX_VIEW_HISTORY_SIZE;
            if (count > maxcount)
            {
                m_hSplitter.SetSinglePaneMode(SPLIT_PANE_BOTTOM);
                m_history.truncateStrings(hs);
            }
        }
    }

    void saveClientWindowPos()
    {
        m_propData->windows.clear();
        tstring buffer;
        for (int i = 0, e = m_views.size(); i<e; ++i)
        {
            MudView *v = m_views[i];
            getWindowText(*v, &buffer);
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            OutputWindow w;
            w.name = tstring(buffer);
            w.side = ctx->Side;
            w.lastside = ctx->LastSide;
            w.pos = ctx->rcWindow;
            w.size = ctx->sizeFloat;
            m_propData->windows.push_back(w);
        }

        WINDOWPLACEMENT wp;
        m_parent.GetWindowPlacement(&wp);
        m_propData->main_window = wp.rcNormalPosition;
        m_propData->main_window_fullscreen = (wp.showCmd == SW_SHOWMAXIMIZED) ? 1 : 0;
    }

    void loadClientWindowPos()
    {
        for (int i = 0; i<OUTPUT_WINDOWS; ++i)
        {
            MudView *v = m_views[i];
            DOCKCONTEXT *ctx = m_dock._GetContext(*v);
            int menu_id = i + ID_WINDOW_1;
            if (IsFloating(ctx->Side))
                m_dock._UnFloatWindow(ctx);
            else if (IsDocked(ctx->Side))
                m_dock._UnDockWindow(ctx);

            const OutputWindow& w = m_propData->windows[i];
            if (IsDocked(w.side))
            {
                m_dock.DockWindow(*v, w.side);
                int size = IsDockedVertically(w.side) ? w.pos.right : w.pos.bottom;
                m_dock.SetPaneSize(w.side, size);
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
            ctx->rcWindow = w.pos;
            ctx->sizeFloat = w.size;
            ctx->LastSide = w.lastside;
        }
        m_dock.SortPanes();
    }

    void savePluginWindowPos(HWND wnd = NULL)
    {
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
                m_propData->plugins.saveWindowPos(v->getPluginName(), w);
            }
        }
    }
};
