#include "stdafx.h"
#include "settingsDlg.h"
#include "mainwnd.h"

extern SettingsDlg* m_settings;
LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return m_settings->HookGetMsgProc(nCode, wParam, lParam);
}

void SettingsDlg::setSettings(ClickpadSettings *settings)
{
    m_settings = settings;
    int rows = m_settings->getRows();
    if (rows <= 0) 
        { rows = 1; m_settings->setRows(rows); }
    else if (rows > 5)
        { rows = 5; m_settings->setRows(rows); }
    m_rows.SetCurSel(rows-1);
    int columns = m_settings->getColumns();
    if (columns <= 0 || columns > 10)
        { columns = 8; m_settings->setColumns(columns); }
    m_columns.SetCurSel(columns-1);
    int bsize = m_settings->getButtonSize();
    if (bsize != 48 && bsize != 64 && bsize != 80)
        { bsize =  64; m_settings->setButtonSize(bsize); }
    if (bsize == 48)
        m_bsize.SetCurSel(0);
    else if (bsize == 64)
        m_bsize.SetCurSel(1);
    else
        m_bsize.SetCurSel(2);
}

LRESULT SettingsDlg::HookGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    LPMSG lpMsg = (LPMSG) lParam;
    if ( nCode >= 0 && PM_REMOVE == wParam )
    {
       // Don't translate non-input events.
       if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
       {
          if ( IsDialogMessage(lpMsg) )
          {
            // The value returned from this hookproc is ignored, 
            // and it cannot be used to tell Windows the message has been handled.
            // To avoid further processing, convert the message to WM_NULL 
            // before returning.
            lpMsg->message = WM_NULL;
            lpMsg->lParam  = 0;
            lpMsg->wParam  = 0;
          }
       }
    }
    return CallNextHookEx(m_hHook, nCode, wParam, lParam);
}

void SettingsDlg::editButton(PadButton *button)
{
    tstring text, command;
    button->getText(&text);
    button->getCommand(&command);
    m_edit_text.SetWindowText(text.c_str());
    m_edit_command.SetWindowText(command.c_str());
}
