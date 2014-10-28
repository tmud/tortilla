#pragma once

#include "mudGameView.h"
#include "helpManager.h"
#include "plugins/pluginsApi.h"
#include "aboutdlg.h"

class CMainFrame : public CFrameWindowImpl<CMainFrame>,
    public CUpdateUI<CMainFrame>, public CMessageFilter, public CIdleHandler
{
public:
    ToolbarEx<CMainFrame> m_toolBar;
    MudGameView m_gameview;
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
    CMainFrame() {}

public:
    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_1, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_2, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_3, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_4, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_5, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_WINDOW_6, UPDUI_MENUPOPUP)
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
        COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER(ID_MUDCLIENT_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_MUDCLIENT_EXIT, OnAppExit)
        MESSAGE_HANDLER(WM_USER, OnSetMenuCheck);
        MESSAGE_HANDLER(WM_USER+1, OnSetMenuText);
        COMMAND_ID_HANDLER(ID_MUDCLIENT_HELP, OnHelp)
        COMMAND_ID_HANDLER(ID_CHECK_UPDATES, OnCheckUpdates)
        CHAIN_MSG_MAP_ALT_MEMBER(m_gameview, 1)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()   

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        if (!m_gameview.initialize())
        {
            DestroyWindow();
            PostQuitMessage(0);
            return 0;
        }

        setTaskbarName();

        m_toolBar.create(this);
        m_toolBar.createCmdBar(IDR_MAINFRAME);
        m_toolBar.createToolbar(IDR_MAINFRAME);

        m_hWndClient = m_gameview.createView(m_hWnd);
        //UIAddToolBar(hWndToolBar);
        UISetCheck(ID_VIEW_TOOLBAR, 1);

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
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

    LRESULT OnViewToolBar(WORD, WORD, HWND, BOOL&)
    {
        static BOOL bVisible = TRUE;	// initially visible
        bVisible = !bVisible;
        CReBarCtrl rebar = m_hWndToolBar;
        int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
        rebar.ShowBand(nBandIndex, bVisible);
        UISetCheck(ID_VIEW_TOOLBAR, bVisible);
        UpdateLayout();
        return 0;
    }

    LRESULT OnViewStatusBar(WORD, WORD, HWND, BOOL&)
    {
        BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
        ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
        UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
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
        openURL(L"https://github.com/tmud/tortilla/releases");
        return 0;
    }
   
    LRESULT OnAppAbout(WORD, WORD, HWND, BOOL&)
    {
        CWelcomeDlg dlg;
        dlg.DoModal();
        return 0;
    }

    void setTaskbarName()
    {
        if (!isVistaOrHigher())
            return;
        tstring name(L"Tortilla Mud Client");
        HKEY key = 0;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache", 
            0, KEY_SET_VALUE, &key) == ERROR_SUCCESS)
        {
            WCHAR buffer[MAX_PATH+1];
            GetModuleFileName(NULL, buffer, MAX_PATH);
            RegSetValueEx(key, buffer, 0, REG_SZ, (const BYTE*)name.c_str(), name.length()*sizeof(WCHAR));
            RegCloseKey(key);
        }
    }
};
