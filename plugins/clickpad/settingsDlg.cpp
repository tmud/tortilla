#include "stdafx.h"
#include "settingsDlg.h"
#include "mainwnd.h"
#include "libs/common/memoryBuffer.h"
#include "mudclient/src/common/changeDir.h"

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

    tstring text, command;
    button->getText(&text);
    button->getCommand(&command);
    m_edit_text.SetWindowText(text.c_str());
    m_edit_command.SetWindowText(command.c_str());

    /*tstring img; int img_index = -1;
    button->getImage(&img, &img_index);
    if (!img.empty())
    {        
    }*/

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

void getWindowText(HWND handle, tstring *string)
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
    tstring text;
    getWindowText(m_edit_text, &text);
    m_editable_button->setText(text);
    return 0;
}

LRESULT SettingsDlg::OnCommandChanged(WORD, WORD, HWND, BOOL&)
{
    if (!m_editable_button)
        return 0;
    tstring cmd;
    getWindowText(m_edit_command, &cmd);
    m_editable_button->setCommand(cmd);
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

LRESULT SettingsDlg::OnDelHotkey(WORD, WORD, HWND, BOOL&)
{
    luaT_Props p(getLuaState());
    if (p.settingsWnd())
    {
        m_close_settings.ShowWindow(SW_SHOWNOACTIVATE);
        return 0;
    }

    int item_selected = m_list.GetSelectedIndex();
    if (item_selected == -1)
        return 0;
    tstring key0, cmd0, group0;
    getListItemText(item_selected, 0, &key0);
    getListItemText(item_selected, 1, &cmd0);
    getListItemText(item_selected, 2, &group0);
    m_list.DeleteItem(item_selected);

    luaT_ActiveObjects hk(getLuaState(), "hotkeys");
    u8string key, value, group;
    for (int i=1,e=hk.size();i<=e;++i)
    {
        hk.select(i);
        if (hk.get(luaT_ActiveObjects::KEY, &key) && !key0.compare(TU2W(key.c_str())) &&
            hk.get(luaT_ActiveObjects::VALUE, &value) && !cmd0.compare(TU2W(value.c_str())) &&
            hk.get(luaT_ActiveObjects::GROUP, &group) && !group0.compare(TU2W(group.c_str()))
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
    m_del_button.EnableWindow(flag);
    m_images_list.EnableWindow(flag);
    if (!flag)
        m_images_list.SetCurSel(-1);
    if (!state)
    {
        m_edit_text.SetWindowText(L"");
        m_edit_command.SetWindowText(L"");
    }
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
            tstring text;
            getListItemText(item_selected, 1, &text);
            m_edit_command.SetWindowText(text.c_str());
        }
        luaT_Props p(getLuaState());
        if (!p.settingsWnd())
        {
            m_del_hotkey.EnableWindow(TRUE);
            m_close_settings.ShowWindow(SW_HIDE);
        }
        else
            m_close_settings.ShowWindow(SW_SHOWNOACTIVATE);
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

void SettingsDlg::getListItemText(int item, int subitem, tstring* text)
{
     wchar_t buffer[256];
     m_list.GetItemText(item, subitem, buffer, 255);
     text->assign(buffer);
}

LRESULT SettingsDlg::OnIconChanged(WORD, WORD, HWND, BOOL&)
{
    /*int item = m_images_list.GetCurSel();
    int len = m_images_list.GetTextLen(item);
    MemoryBuffer mb( (len+1)*sizeof(wchar_t) );
    wchar_t *p = (wchar_t *)mb.getData();
    m_images_list.GetText(item, p);
    tstring fpath(m_images_path);
    fpath.append(p);*/
   // m_editable_button->setImage(fpath);
    return 0;
}

void SettingsDlg::setIconsFileList()
{
    u8string tmp;
    base::getResource(getLuaState(), "", &tmp);
    if (tmp.empty())
        return;
    tstring path ( TU2W(tmp.c_str()) );

    ChangeDir cd;
    if (!cd.changeDir(path))
       return;

    std::vector<tstring> sets;
    WIN32_FIND_DATA fd;
    memset(&fd, 0, sizeof(WIN32_FIND_DATA));
    HANDLE file = FindFirstFile(L"*.*", &fd);
    if (file != INVALID_HANDLE_VALUE)
    {
        do
        {
           if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && isSupportedExt(fd.cFileName))
           {
               image_file el; el.path = fd.cFileName;
               m_image_files.push_back(el);
           }
           if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
           {
               tstring dir(fd.cFileName);
               bool only_numbers = (wcsspn(dir.c_str(), L"0123456789") != dir.length()) ? false : true;
               if (only_numbers)
                  sets.push_back(dir);
           }
       } while (::FindNextFile(file, &fd));
       ::FindClose(file);
   }
   for (int s=0,se=sets.size();s<se;++s)
   {
       tstring dir(sets[s]);
       dir.append(L"\\*.*");
       file = FindFirstFile(dir.c_str(), &fd);
       do
       {
           if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && isSupportedExt(fd.cFileName))
           {
              image_file el; el.path = fd.cFileName;
              el.set_size = _wtoi(sets[s].c_str());
              m_image_files.push_back(el);
           }
       } while (::FindNextFile(file, &fd));
       ::FindClose(file);
   }

   m_images_list.AddString(L"Без иконки");
   for (int i=0,e=m_image_files.size();i<e;++i)
   {
       image_file &f = m_image_files[i];
       if (f.set_size == -1)
           m_images_list.AddString(f.path.c_str());
       else
       {
           wchar_t buffer[16];
           swprintf(buffer, L"(%d) ", f.set_size);
           tstring item(buffer);
           item.append(f.path);
           m_images_list.AddString(item.c_str());
       }
   }
}

bool SettingsDlg::isSupportedExt(const wchar_t* file)
{
    const wchar_t *e = wcsrchr(file, L'.');
    if (!e)
        return false;
    tstring ext(e + 1);
    return (ext == L"png" || ext == L"bmp" || ext == L"gif" || ext == L"ico" || ext == L"jpg") ? true : false;
}
