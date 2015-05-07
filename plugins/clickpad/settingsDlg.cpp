#include "stdafx.h"
#include "settingsDlg.h"

extern SettingsDlg* m_settings;
LRESULT FAR PASCAL GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return m_settings->HookGetMsgProc(nCode, wParam, lParam);
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
