#include "stdafx.h"
#include "toolbarEx.h"

class MenuHelper : protected CMenuHandle
{
public:
    MenuHelper() {}
    MenuHelper(HMENU menu) { Attach(menu); }
    void attach(HMENU hmenu) { Attach(hmenu); }
    HMENU getMenu() { return operator HMENU(); }

    int findMenuItem(const tchar* text)
    {
        tchar buffer[64];
        for (int i = 0, e = GetMenuItemCount(); i < e; ++i)
        {
            UINT state = GetMenuState(i, MF_BYPOSITION);
            if ((state&MF_SEPARATOR) && !wcscmp(text, L"-"))
               return i;
            GetMenuString(i, buffer, 63, MF_BYPOSITION);
            tstring label(buffer);
            tstring_replace(&label, L"&", L"");
            if (!wcscmp(label.c_str(), text))
                return i;
        }
        return -1;
    }

    void addItem(int pos, const tchar* item, UINT id)
    {
        addSubItem(pos, item, id);
    }

    HMENU addSubMenu(int pos, const tchar* item)
    {
        return addSubItem(pos, item, -1);
    }

    void addSeparator(int pos)
    {
        MENUITEMINFO mi;
        memset(&mi, 0, sizeof(MENUITEMINFO));
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask = MIIM_FTYPE;
        mi.fType = MFT_SEPARATOR; 
        InsertMenuItem(pos, TRUE, &mi);
    }

    void deleteItem(int pos)
    {
        DeleteMenu(pos, MF_BYPOSITION);        
    }

    void deleteItemById(UINT id)
    {
        DeleteMenu(id, MF_BYCOMMAND); 
    }

    bool isEmpty()
    {
        return (GetMenuItemCount() == 0) ? true : false;
    }

    void checkItem(UINT id, BOOL checked)
    {
        CheckMenuItem(id, (checked) ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
    }

    void enableItem(UINT id, BOOL status)
    {
        EnableMenuItem(id, (status) ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_DISABLED);
    }

    HMENU getSubMenu(int pos)
    {
        return GetSubMenu(pos);
    }

    UINT getMenuItemId(int pos)
    {
        return GetMenuItemID(pos);
    }

    void validatePosition(int* pos)
    {
        int last = GetMenuItemCount();
        if (*pos >= 0 && *pos <= last) return;
        *pos = last;
    }

    int getMenuItemsCount()
    {
        return GetMenuItemCount();
    }

private:
    HMENU addSubItem(int pos, const tchar* item, UINT id)
    {
        if (pos == -1)
            pos = GetMenuItemCount();
        int len = _tcslen(item);
        tchar *buffer = new tchar[len + 1];
        _tcscpy(buffer, item);
        MENUITEMINFO mi;
        memset(&mi, 0, sizeof(MENUITEMINFO));
        mi.cbSize = sizeof(MENUITEMINFO);
        mi.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE;
        if (id == -1) { mi.fMask |= MIIM_SUBMENU; mi.hSubMenu = ::CreatePopupMenu(); }
        else { mi.fMask |= MIIM_ID; mi.wID = id; }
        mi.fType = MFT_STRING;
        mi.fState = MFS_ENABLED;
        mi.dwTypeData = buffer;
        mi.cch = len;
        InsertMenuItem(pos, TRUE, &mi);
        delete[]buffer;
        return mi.hSubMenu;
    }
};

CommandBarEx::CommandBarEx() : m_lastpos(-1)
{
}

CommandBarEx::~CommandBarEx()
{
}

HWND CommandBarEx::CreateMenu(HWND parent, const std::vector<CommandBarExImage>& items)
{
    HWND toolbar = m_cmdbar.Create(parent, CWindow::rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    m_cmdbar.AttachMenu(::GetMenu(parent));
    ::SetMenu(parent, NULL);
    if (items.empty())
        return toolbar;
    BITMAP bm;
    GetObject(items[0].image, sizeof(BITMAP), &bm);
    int size = bm.bmHeight;
    int count = items.size();
    m_cmdbar.SetImageSize(size, size);
    m_cmdbar.SetButtonSize(size, size);
    m_cmdbar.SetImageMaskColor(RGB(192, 192, 192));
    for (const CommandBarExImage& im : items)
        m_cmdbar.AddBitmap(im.image, im.commandid);
    return toolbar;
}

bool CommandBarEx::addMenuItem(const TCHAR* menuitem, int pos, UINT id, HBITMAP image)
{
    Tokenizer tk(menuitem, L"/");
    if (tk.empty())
        return false;

    if (image)
        m_cmdbar.AddBitmap(image, id);

    if (m_lastlabel.compare(tk[0]) != 0)
    {
        m_lastlabel.assign(tk[0]);
        m_lastpos = -1;
    }

    bool toplevel = false;
    MenuHelper menu(m_cmdbar.GetMenu());
    for (int i = 0, e = tk.size() - 1; i <= e; ++i)
    {
        bool last = (i == e) ? true : false;
        int index = menu.findMenuItem(tk[i]);
        if (index == -1)
        {
            if (pos == -1)
            {
                if (m_lastpos != -1)
                    pos = m_lastpos + 1;
            }
            else
                m_lastpos = pos;
            menu.validatePosition(&pos);
            if (last)
            {
                if (!wcscmp(tk[i], L"-")) { menu.addSeparator(pos); }
                else { menu.addItem(pos, tk[i], id); }
            }
            else {
                menu.attach(menu.addSubMenu(pos, tk[i]));
            }
            pos = -1;
            if (i == 0) toplevel = true;
        }
        else
        {
            menu.attach(menu.getSubMenu(index));
        }
    }
    if (toplevel) { HMENU h = m_cmdbar.GetMenu(); m_cmdbar.AttachMenu(h); }
    return true;
}

void CommandBarEx::deleteMenuItem(const wchar_t* menuitem, std::vector<UINT> *ids)
{
    Tokenizer tk(menuitem, L"/");
    if (tk.empty())
        return;

    bool toplevel = false;
    MenuHelper menu(m_cmdbar.GetMenu());
    for (int i = 0, e = tk.size() - 1; i <= e; ++i)
    {
        int index = menu.findMenuItem(tk[i]);
        if (index == -1)
            return;

        bool last = (i == e) ? true : false;
        if (i == 0 && last) toplevel = true;
        if (last)
        {
            UINT id = menu.getMenuItemId(index);
            if (id == -1)
              collectSubMenus(menu.getSubMenu(index), ids);
            else
              ids->push_back(id);
            menu.deleteItem(index);
            if (menu.isEmpty())
                deleteEmptySubMenu();
        }
        else
            menu.attach(menu.getSubMenu(index));
    }

    for (int i = 0, e = ids->size(); i < e; ++i)
        m_cmdbar.RemoveImage(ids->at(i));

    // redraw top level menu
    if (toplevel) { HMENU h = m_cmdbar.GetMenu(); m_cmdbar.AttachMenu(h); }
}

void CommandBarEx::deleteMenuItem(UINT id)
{
    MenuHelper menu(m_cmdbar.GetMenu());
    menu.deleteItemById(id);
    if (menu.isEmpty())
        deleteEmptySubMenu();
}

void CommandBarEx::deleteEmptySubMenu()
{
    findEmptySubMenu(m_cmdbar.GetMenu(), true);
}

bool CommandBarEx::findEmptySubMenu(HMENU menu, bool toplevel)
{
    MenuHelper mh(menu);
    for (int i = 0, e = mh.getMenuItemsCount(); i < e; ++i)
    {
        UINT id = mh.getMenuItemId(i);
        if (id == -1) // submenu
        {
            MenuHelper submenu = mh.getSubMenu(i);
            if (submenu.isEmpty()) {
                mh.deleteItem(i);
                // redraw top level menu
                if (toplevel) { HMENU h = m_cmdbar.GetMenu(); m_cmdbar.AttachMenu(h); }                
                return true;
            }
            else
            {
                if (findEmptySubMenu(submenu.getMenu(), false))
                    return true;
            }
        }
    }
    return false;
}

void CommandBarEx::collectSubMenus(HMENU menu, std::vector<UINT> *ids)
{
    MenuHelper mh(menu);
    for (int i = 0, e = mh.getMenuItemsCount(); i < e; ++i)
    {
        UINT id = mh.getMenuItemId(i);
        if (id == -1) // submenu
            collectSubMenus(mh.getSubMenu(i), ids);
        else 
            if (id > 0) ids->push_back(id);
    }
}

void CommandBarEx::checkMenuItem(UINT id, BOOL state)
{
    MenuHelper menu(m_cmdbar.GetMenu());
    menu.checkItem(id, state);
}

void CommandBarEx::enableMenuItem(UINT id, BOOL state)
{
    MenuHelper menu(m_cmdbar.GetMenu());
    menu.enableItem(id, state);
}

int CommandBarEx::getMenuWidth() const
{
    RECT rc;
    int last = m_cmdbar.GetButtonCount() - 1;
    m_cmdbar.GetItemRect(last, &rc);
    return rc.right;
}

HWND ToolBar::createEmpty(HWND parent, int imagesize)
{
    int cxyButtonMargin = imagesize + 7;
    RECT rc = { 0, 0, 0, cxyButtonMargin };
    HWND hWnd = m_toolbar.Create(parent, rc, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, TBSTYLE_EX_MIXEDBUTTONS);
    ::SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);
    ::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(imagesize, imagesize));
    ::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(cxyButtonMargin, cxyButtonMargin));
    m_button_size = cxyButtonMargin;
    return hWnd;
}

HWND ToolBar::create(HWND parent, const std::vector<ToolbarExButton>& items)
{
    int imagesize = 16;
    if (!items.empty())
    {
        BITMAP bm;
        GetObject(items[0].image, sizeof(BITMAP), &bm);
        imagesize = bm.bmHeight;
    }
    HWND hWnd = createEmpty(parent, imagesize);
    for (const ToolbarExButton& tb : items)
    {
        if (tb.commandid == ID_SEPARATOR)
        {
            m_toolbar.AddButton(ID_SEPARATOR, TBSTYLE_SEP, 0, 0, NULL, NULL);
            continue;
        }

        addButton(tb.image, tb.commandid, tb.hover.c_str());
    }
    return hWnd;
}

void ToolBar::addButton(HBITMAP bmp, UINT id, const TCHAR* hover)
{
    BITMAP bm;
    GetObject(bmp, sizeof(BITMAP), &bm);
    CImageList il = m_toolbar.GetImageList();
    if (!il.m_hImageList)
    {
        il.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24 | ILC_MASK, 0, 0);
        m_toolbar.SetImageList(il);
    }
    else
    {
        int cx = 0; int cy = 0;
        il.GetIconSize(cx, cy);
        if (bm.bmWidth != cx || bm.bmHeight != cy)
        {
            BitmapMethods bm(bmp);
            HBITMAP resized = bm.createNewResized(cx, cy);
            DeleteObject(bmp);
            bmp = resized;
        }
    }

    il.Add(bmp, RGB(192, 192, 192));
    int image = il.GetImageCount() - 1;
    if (m_toolbar.AddButton(id, TBSTYLE_BUTTON | BTNS_SHOWTEXT , TBSTATE_ENABLED, image, 0, 0))
    {
        RECT pos;
        int last = m_toolbar.GetButtonCount() - 1;
        m_toolbar.GetItemRect(last, &pos);
        CToolTipCtrl ttip = m_toolbar.GetToolTips();
        ttip.AddTool(m_toolbar, hover, &pos, id);
    }
}

void ToolBar::delButton(UINT id)
{
    CToolTipCtrl ttip = m_toolbar.GetToolTips();
    ttip.DelTool(m_toolbar, id);
    TBBUTTON tb; int index = -1;
    for (int i = 0, e = m_toolbar.GetButtonCount(); i < e; ++i)
    {
        m_toolbar.GetButton(i, &tb);
        if (tb.idCommand == id)
            {  m_toolbar.DeleteButton(i); break; }
    }
}

void ToolBar::checkButton(UINT id, BOOL state)
{
    m_toolbar.CheckButton(id, state);
}

void ToolBar::enableButton(UINT id, BOOL state)
{
    m_toolbar.EnableButton(id, state);
}

int ToolBar::width() const
{
    int size = 0;
    SIZE sz; m_toolbar.GetButtonSize(sz);
    TBBUTTON tb;
    for (int i = 0, e = m_toolbar.GetButtonCount(); i < e; ++i)
    {
        m_toolbar.GetButton(i, &tb);
        if (tb.fsStyle & TBSTYLE_SEP)
            size += 8;
        else
            size += sz.cx;
    }
    return size;
}

int ToolBar::button_width() const
{
    return m_button_size;
}

