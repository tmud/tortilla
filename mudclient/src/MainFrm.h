#pragma once

#include "mudGameView.h"
#include "helpManager.h"
#include "plugins/pluginsApi.h"
#include "resource.h"

template <class T> // stub for DECLARE_FRAME_WND_CLASS macro (without stub not working)
class ATL_NO_VTABLE CCommonFrameImpl : public CMessageMap {
    BEGIN_MSG_MAP(CCommonFrameImpl<T>)
    END_MSG_MAP()
};

class CMainFrame : public CFrameWindowImpl<CMainFrame>,
    public CUpdateUI<CMainFrame>, public CMessageFilter, public CIdleHandler,
    public CCommonFrameImpl<CMainFrame>
{
public:
    ToolbarEx<CMainFrame> m_toolBar;
    MudGameView m_gameview;
    tstring m_cmdLine;
    DECLARE_FRAME_WND_CLASS(MAINWND_CLASS_NAME, IDR_MAINFRAME)
    CMainFrame() : m_trayIconSet(0),m_maximized(false) {}
private:
    int  m_trayIconSet;
    bool m_maximized;
public:
    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_1, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_2, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_3, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_4, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_5, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_6, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_FIND, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

private:
    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (m_gameview.PreTranslateMessage(pMsg))
           return TRUE;
        return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        UIUpdateToolBar();
        return FALSE;
    }

    BEGIN_MSG_MAP(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnMinMaxInfo)
        COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
        //COMMAND_ID_HANDLER(ID_MUDCLIENT_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_MUDCLIENT_EXIT, OnAppExit)
        MESSAGE_HANDLER(WM_USER, OnSetMenuCheck);
        MESSAGE_HANDLER(WM_USER+1, OnSetMenuText);
        MESSAGE_HANDLER(WM_USER+2, OnTrayIcon);
        COMMAND_ID_HANDLER(ID_MUDCLIENT_HELP, OnHelp)
        COMMAND_ID_HANDLER(ID_CHECK_UPDATES, OnCheckUpdates)
        MESSAGE_HANDLER(WM_SIZE, OnSize);
        CHAIN_MSG_MAP_ALT_MEMBER(m_gameview, 1)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
        CHAIN_MSG_MAP(CCommonFrameImpl<CMainFrame>)
    END_MSG_MAP()

    void processCmdline()
    {
        if (m_cmdLine.empty()) return;
        Pcre16 pcre, pcre2;
        pcre.setRegExp(L"((?:\".*\")|[^ ]+)", true);
        pcre.findAllMatches(m_cmdLine);
        pcre2.setRegExp(L"\"(.*)\"", true);

        std::vector<tstring> profiles;
        for (int i=1,e=pcre.getSize(); i<e; ++i)
        {
            tstring profile;
            pcre.getString(i, &profile);
            tstring_trim(&profile);
            if (profile.empty()) continue;
            pcre2.find(profile);
            if (pcre2.getSize() > 0) {
                pcre2.getString(1, &profile);
            }
            profiles.push_back(profile);
        }

        tstring path;
        getClientExePath(&path);
        for (int i=1,e=profiles.size(); i<e; ++i)
        {
            tstring p(path);
            p.append(L" ");
            p.append(profiles[i]);
            WCHAR *tmppath = new WCHAR[p.length() + 1];
            wcscpy(tmppath, p.c_str());
            STARTUPINFO cif;
            ZeroMemory(&cif, sizeof(STARTUPINFO));
            PROCESS_INFORMATION pi;
            CreateProcess(NULL, tmppath, NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi);
            delete[]tmppath;
        }
        if (profiles.empty())
            m_cmdLine.clear();
        else
            m_cmdLine = profiles[0];
    }

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        HDC dc = GetDC();
        float logpy = static_cast<float>(GetDeviceCaps(dc, LOGPIXELSY));
        ReleaseDC(dc);
        float dpi = logpy / 96;
        PropertiesData *pdata = m_gameview.getPropData();
        pdata->dpi = dpi;
        pdata->displays.setDpi(dpi);

        processCmdline();
        if (!m_gameview.initialize(m_cmdLine))
        {
            DestroyWindow();
            PostQuitMessage(0);
            return 0;
        }

        setTaskbarName();
        m_toolBar.create(this);

        const int count = 5;
        UINT ti[count] = { ID_SELECTPROFILE, ID_SETTINGS, ID_PLUGINS, ID_MODE, ID_MUDCLIENT_HELP };
        HBITMAP images = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME));
        BitmapMethods bmpm(images);
        BITMAP bm;
        GetObject(images, sizeof(BITMAP), &bm);

        int src_size = bm.bmHeight;
        float float_size = static_cast<float>(src_size);
        float_size = float_size * dpi;
        int size = static_cast<int>(float_size);

        std::vector<CommandBarExImage> items;
        std::vector<ToolbarExButton> buttons;
        for (int i = 0; i < count; ++i)
        {
            CommandBarExImage im;
            int imageindex = i;
            im.image = bmpm.cutNewBitmap(src_size * imageindex, 0, src_size, src_size, size, size);
            im.commandid = ti[i];
            items.push_back(im);
            ToolbarExButton b;
            b.image = bmpm.cutNewBitmap(src_size * imageindex, 0, src_size, src_size, size, size);
            b.commandid = im.commandid;
            loadString(b.commandid, &b.hover);
            tstring_trimsymbols(&b.hover, L"\n");
            buttons.push_back(b);
        }
        ToolbarExButton separator;
        buttons.insert(buttons.begin() + 4, separator);
        buttons.insert(buttons.begin() + 1, separator);
        m_toolBar.createCmdBar(items);
        m_toolBar.createToolbar(buttons);
        DeleteObject(images);

        CReBarCtrl rebar = m_hWndToolBar;
        CReBarSettings rbs;
        rbs.Load(rebar, pdata->rebar);

        bool toolbar = rbs.IsVisible(rebar, ATL_IDW_BAND_FIRST+1);
        UISetCheck(ID_VIEW_TOOLBAR,toolbar);

        m_hWndClient = m_gameview.createView(m_hWnd);

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        CReBarCtrl rebar = m_hWndToolBar;
        if (rebar.IsWindow()) {
        CReBarSettings rbs;
        tstring param;
        rbs.Save(rebar, &param);
        PropertiesData *pdata = m_gameview.getPropData();
        pdata->rebar = param;
        }

        // unregister message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);
        bHandled = FALSE;
        return 1;
    }

    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnMinMaxInfo(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        MINMAXINFO *minmax = (MINMAXINFO*) lparam;
        minmax->ptMinTrackSize.x = 300;
        minmax->ptMinTrackSize.y = 250;
        return 0;
    }

    void showTrayIcon()
    {
        NOTIFYICONDATA niData; 
        ZeroMemory(&niData,sizeof(NOTIFYICONDATA));
        niData.cbSize = sizeof(NOTIFYICONDATA);
        niData.uID = 0;
        niData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
        niData.hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
        niData.hWnd = m_hWnd;
        niData.uCallbackMessage = WM_USER+2;

        PropertiesManager *pmgr = m_gameview.getPropManager();
        tstring tip;
        tip.append(pmgr->getProfileGroup());
        tip.append(L" - ");
        tip.append(pmgr->getProfileName());
        wcscpy(niData.szTip, tip.c_str());
        Shell_NotifyIcon(NIM_ADD,&niData);
        m_trayIconSet = 1;
    }

    void hideTrayIcon()
    {
        NOTIFYICONDATA niData; 
        ZeroMemory(&niData,sizeof(NOTIFYICONDATA));
        niData.cbSize = sizeof(NOTIFYICONDATA);
        niData.uID = 0;
        niData.hWnd = m_hWnd;
        Shell_NotifyIcon(NIM_DELETE,&niData);
        m_trayIconSet = 0;
    }

    LRESULT OnTrayIcon(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        WORD event = LOWORD(lparam);
        if (event == WM_LBUTTONDOWN)
        {
            int set = m_trayIconSet;
            hideTrayIcon();
            if (set == 2)
                ShowWindow(SW_SHOWMAXIMIZED);
            else
                ShowWindow(SW_SHOWDEFAULT);
            SetForegroundWindow(m_hWnd);
        }
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM wparam, LPARAM, BOOL&bHandled)
    {
        bool mode = (m_gameview.getPropData()->move_totray) ? true : false;
        if (wparam == SIZE_MINIMIZED)
        {
            if (!m_trayIconSet && mode)
            {
                WINDOWPLACEMENT wp;
                GetWindowPlacement(&wp);
                ShowWindow(SW_HIDE);
                showTrayIcon();
                if (m_maximized) m_trayIconSet = 2;
            }
            m_maximized = false;
        }
        else
        {
            m_maximized = (wparam == SIZE_MAXIMIZED) ? true : false;
            if (m_trayIconSet)
                hideTrayIcon();
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnViewToolBar(WORD, WORD, HWND, BOOL&)
    {
        CReBarCtrl rebar = m_hWndToolBar;
        CReBarSettings rbs;
        BOOL bVisible = rbs.IsVisible(rebar, ATL_IDW_BAND_FIRST + 1) ? FALSE : TRUE;
        int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
        rebar.ShowBand(nBandIndex, bVisible);
        UISetCheck(ID_VIEW_TOOLBAR, bVisible);
        UpdateLayout();
        return 0;
    }

    LRESULT OnSetMenuCheck(UINT, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        UISetCheck(wparam, lparam);
        return 0;
    }

    LRESULT OnSetMenuText(UINT, WPARAM wparam, LPARAM lparam, BOOL&)
    {
        const WCHAR *text = (const WCHAR*)lparam;
        UISetText(wparam, text);
        return 0;
    }

    LRESULT OnAppExit(WORD, WORD, HWND, BOOL&)
    {
        SendMessage(WM_CLOSE, 0, 0);
        return 0;
    }

    LRESULT OnHelp(WORD, WORD, HWND, BOOL&)
    {
        openHelp(m_hWnd, L"");
        return 0;
    }

    LRESULT OnCheckUpdates(WORD, WORD, HWND, BOOL&)
    {
        openURL(L"http://tmud.github.io");
        return 0;
    }

    LRESULT OnAppAbout(WORD, WORD, HWND, BOOL&)
    {
        //CWelcomeDlg dlg;
        //dlg.DoModal();
        return 0;
    }

    void setTaskbarName()
    {
        /*if (!isVistaOrHigher())
            return;
        tstring name(L"Tortilla");
        HKEY key = 0;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache", 
            0, KEY_SET_VALUE, &key) == ERROR_SUCCESS)
        {
            WCHAR buffer[MAX_PATH+1];
            GetModuleFileName(NULL, buffer, MAX_PATH);
            RegSetValueEx(key, buffer, 0, REG_SZ, (const BYTE*)name.c_str(), name.length()*sizeof(WCHAR));
            RegCloseKey(key);
        }*/
    }
};
