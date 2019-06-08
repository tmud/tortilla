﻿#include "stdafx.h"
#include "settingsDlg.h"
#include "mainwnd.h"
#include "libs/common/memoryBuffer.h"
#include "mudclient/src/common/changeDir.h"

extern SettingsDlg* m_settings;
extern SelectImageDlg* m_select_image;
extern luaT_window m_select_image_window;

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
    else if (rows > MAX_ROWS)
        { rows = MAX_ROWS; m_settings->setRows(rows); }
    m_rows.SetCurSel(rows-1);
    int columns = m_settings->getColumns();
    if (columns <= 0)
        { columns = 1; m_settings->setColumns(columns); }
    else if (columns > MAX_COLUMNS)
        { columns = MAX_COLUMNS; m_settings->setColumns(columns); }
    m_columns.SetCurSel(columns-1);

    ButtonSizeTranslator bt;
    int bsize = m_settings->getButtonSize();
    if (!bt.checkSize(bsize))
        { bsize = bt.getDefaultSize(); m_settings->setButtonSize(bsize); }
    m_bsize.SetCurSel(bt.getIndex(bsize));
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
    if (m_editable_button)
        m_editable_button->setSelected(false);
    m_editable_button = button;

    setEditableState(button ? true : false);
    if (!button)
        return;

    std::wstring text, command, ttip;
    button->getText(&text);
    button->getCommand(&command);
    m_edit_text.SetWindowText(text.c_str());
    m_edit_command.SetWindowText(command.c_str());
    button->getTooltip(&ttip);
    m_edit_tooltip.SetWindowText(ttip.c_str());
    m_template_cmd.SetCheck(button->getTemplate() ? BST_CHECKED : BST_UNCHECKED);

    button->setSelected(true);
    if (text.empty())
    {
        m_edit_text.SetFocus();
    }
    else
    {
        int pos = command.length();
        m_edit_command.SetFocus();
        m_edit_command.SetSel(pos, pos);
    }

    ClickpadImage *image = button->getImage();
    m_image_example.setImage(image);
}

LRESULT SettingsDlg::OnTemplate(WORD, WORD, HWND, BOOL&)
{
    bool state = (m_template_cmd.GetCheck() == BST_CHECKED) ? true : false;
    m_editable_button->setTemplate(state);
    return 0;
}

LRESULT SettingsDlg::OnRowsChanged(WORD, WORD, HWND, BOOL&)
{
    resetEditable();
    int rows = m_rows.GetCurSel() + 1;
    m_settings->setRows(rows);
    return 0;
}

LRESULT SettingsDlg::OnColumnsChanged(WORD, WORD, HWND, BOOL&)
{
    resetEditable();
    int columns = m_columns.GetCurSel() + 1;
    m_settings->setColumns(columns);
    return 0;
}

LRESULT SettingsDlg::OnBSizeChanged(WORD, WORD, HWND, BOOL&)
{
    resetEditable();
    ButtonSizeTranslator bt;
    int pos = m_bsize.GetCurSel();
    int bsize = bt.getSize(pos);
    m_settings->setButtonSize(bsize);
    return 0;
}

void getWindowText(HWND handle, std::wstring *string)
{
    int text_len = ::GetWindowTextLength(handle);
    WCHAR *buffer = new WCHAR[text_len+2];
    ::GetWindowText(handle, buffer, text_len+1);
    string->assign(buffer);
    delete []buffer;
}

LRESULT SettingsDlg::OnTextChanged(WORD, WORD, HWND, BOOL&)
{
    if (!m_editable_button)
        return 0;
    std::wstring text;
    getWindowText(m_edit_text, &text);
    m_editable_button->setText(text);
    return 0;
}

LRESULT SettingsDlg::OnCommandChanged(WORD, WORD, HWND, BOOL&)
{
    if (!m_editable_button)
        return 0;
    std::wstring cmd;
    getWindowText(m_edit_command, &cmd);
    m_editable_button->setCommand(cmd);
    return 0;
}

LRESULT SettingsDlg::OnTooltipChanged(WORD, WORD, HWND, BOOL&)
{
    if (!m_editable_button)
        return 0;
    std::wstring ttip;
    getWindowText(m_edit_tooltip, &ttip);
    m_editable_button->setTooltip(ttip);
    return 0;
}

LRESULT SettingsDlg::OnButtonExit(WORD, WORD, HWND, BOOL&)
{
    exitEditMode();
    return 0;
}

LRESULT SettingsDlg::OnDelButton(WORD, WORD, HWND, BOOL&)
{
    if (!m_editable_button)
        return 0;
    m_editable_button->setText(L"");
    m_editable_button->setCommand(L"");
    m_edit_text.SetWindowText(L"");
    m_edit_command.SetWindowText(L"");
    m_edit_text.SetFocus();
    return 0; 
}

void SettingsDlg::setSettingsBlock(bool block)
{
    m_del_hotkey.EnableWindow(FALSE);
    if (block)
    {
        m_close_settings.ShowWindow(SW_SHOWNOACTIVATE);
    }
    else
    {
        m_list.SetFocus();
        m_close_settings.ShowWindow(SW_HIDE);
    }
}

LRESULT SettingsDlg::OnDelHotkey(WORD, WORD, HWND, BOOL&)
{
    int item_selected = m_list.GetSelectedIndex();
    if (item_selected == -1)
        return 0;
    std::wstring key0, cmd0, group0;
    getListItemText(item_selected, 0, &key0);
    getListItemText(item_selected, 1, &cmd0);
    getListItemText(item_selected, 2, &group0);
    m_list.DeleteItem(item_selected);

    luaT_ActiveObjects hk(getLuaState(), "hotkeys");
    std::wstring key, value, group;
    for (int i=1,e=hk.size();i<=e;++i)
    {
        hk.select(i);
        if (hk.get(luaT_ActiveObjects::KEY, &key) && !key0.compare(key.c_str()) &&
            hk.get(luaT_ActiveObjects::VALUE, &value) && !cmd0.compare(value.c_str()) &&
            hk.get(luaT_ActiveObjects::GROUP, &group) && !group0.compare(group.c_str())
            )
        {
            hk.del();
            break;
        }
    }
    return 0;
}

void SettingsDlg::resetEditable()
{
    if (!m_editable_button)
        return;
    m_editable_button->setSelected(false);
    m_editable_button = NULL;
    setEditableState(false);
}

void SettingsDlg::setEditableState(bool state)
{
    BOOL flag = state ? TRUE : FALSE;
    m_edit_text.EnableWindow(flag);
    m_edit_command.EnableWindow(flag);
    m_edit_tooltip.EnableWindow(flag);
    m_del_button.EnableWindow(flag);
    if (!state)
    {
        m_edit_text.SetWindowText(L"");
        m_edit_command.SetWindowText(L"");
        m_edit_tooltip.SetWindowText(L"");
    }
    if (!flag)
        m_template_cmd.SetCheck(BST_UNCHECKED);
    m_template_cmd.EnableWindow(flag);
    m_button_icon.EnableWindow(flag);
    m_button_delicon.EnableWindow(flag);
}

LRESULT SettingsDlg::OnListItemChanged(int , LPNMHDR , BOOL&)
{
    int item_selected = m_list.GetSelectedIndex();
    if (item_selected == -1)
    {
        m_del_hotkey.EnableWindow(FALSE);
    }
    else
    {
        if (m_edit_command.IsWindowEnabled())
        {
            std::wstring text;
            getListItemText(item_selected, 1, &text);
            m_edit_command.SetWindowText(text.c_str());
            if (m_edit_text.GetWindowTextLength() == 0)
            {
                if (text.length() > 5)
                    text = text.substr(0, 5);
                m_edit_text.SetWindowText(text.c_str());
            }
        }

        luaT_Props p(getLuaState());
        if (!p.isPropertiesOpen())
            m_del_hotkey.EnableWindow(TRUE);
    }
    return 0;
}

LRESULT SettingsDlg::OnListKillFocus(int , LPNMHDR , BOOL&)
{
    HWND focus = GetFocus();
    if (focus != m_del_hotkey)
        m_del_hotkey.EnableWindow(FALSE);
    return 0;
}

void SettingsDlg::getListItemText(int item, int subitem, std::wstring* text)
{
     wchar_t buffer[256];
     m_list.GetItemText(item, subitem, buffer, 255);
     text->assign(buffer);
}

LRESULT SettingsDlg::OnIconButton(WORD, WORD, HWND, BOOL&)
{
    if (!m_select_image_window.isVisible())
        m_select_image_window.show();
    else
        m_select_image_window.hide();
    return 0;
}

LRESULT SettingsDlg::OnDelIconButton(WORD, WORD, HWND, BOOL&)
{
    if (m_editable_button)
    {
        m_editable_button->setImage(NULL);
        m_image_example.setImage(NULL);
    }
    return 0;
}

LRESULT SettingsDlg::OnSelectFont(WORD, WORD, HWND, BOOL&)
{
    LOGFONT lf;
    m_settings->getFont(&lf);
    CFontDialog dlg(&lf, CF_SCREENFONTS, NULL, m_hWnd);
    if (dlg.DoModal() == IDOK)
    {
        m_settings->setFont(lf);
    }
    return 0;
}

bool SettingsDlg::isSupportedExt(const wchar_t* file)
{
    const wchar_t *e = wcsrchr(file, L'.');
    if (!e)
        return false;
    std::wstring ext(e + 1);
    return (ext == L"png" || ext == L"bmp" || ext == L"gif" || ext == L"ico" || ext == L"jpg") ? true : false;
}

void SettingsDlg::updateImage()
{
    if (m_editable_button)
    {
        ClickpadImage* image = m_select_image->createImageSelected();
        m_editable_button->setImage(image);   
        m_image_example.setImage(image);
    }
}
