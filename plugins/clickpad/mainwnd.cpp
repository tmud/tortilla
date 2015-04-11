#include "stdafx.h"
#include "mainwnd.h"
extern luaT_window m_parent_window;
extern HWND m_hwnd_client;

ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_button_width(64), m_button_height(64)
{
}

ClickpadMainWnd::~ClickpadMainWnd()
{
}


void ClickpadMainWnd::setButtonsNet(int rows, int columns)
{
    createNewRows(rows);
    createNewColumns(columns);

    for (int y=0; y<rows; ++y)
    {
        for (int x=0; x<columns; ++x)
        {
            createButton(x, y);
        }
    }
}

int ClickpadMainWnd::getRows() const
{
    return m_buttons.size();
}

int ClickpadMainWnd::getColumns() const
{
    int rows = m_buttons.size();
    return (rows > 0) ? m_buttons[0].size() : 0;
}

void ClickpadMainWnd::createNewRows(int count)
{
    std::vector<PadButton*> newrow(getColumns(), NULL);
    for (int i=0; i<count; ++i)
        m_buttons.push_back(newrow);
}

void ClickpadMainWnd::createNewColumns(int count)
{
    int rows = getRows();
    if (rows == 0) return;
    int columns = getColumns() + count;
    for (int i=0; i<rows; ++i)
        m_buttons[i].resize(columns, NULL);
}

void ClickpadMainWnd::onCreate()
{    
    //RECT pos = { 100, 100, 400, 400 };
    m_settings_wnd.Create(m_hWnd, rcDefault, WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    m_button_wnd.Create(m_hWnd, rcDefault, WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
}

void ClickpadMainWnd::onSize()
{
    /*RECT pos;
    GetClientRect(&pos);
    if (!m_editmode)
        return;
    pos.top = pos.bottom / 2;
    m_settings_wnd.MoveWindow(&pos);*/
}

void ClickpadMainWnd::switchEditMode()
{
    if (!m_editmode)
        beginEditMode();
    else
        endEditMode();
}

void ClickpadMainWnd::beginEditMode()
{
    assert(!m_editmode);
    if (m_editmode)
        return;

    CWindow floatwnd(m_parent_window.floathwnd());
    RECT pos;
    floatwnd.GetClientRect(&pos);
    int height = pos.bottom; int width = pos.right;
    floatwnd.GetWindowRect(&pos);
    
    RECT settings_pos;
    m_settings_wnd.GetClientRect(&settings_pos);
    pos.bottom += settings_pos.bottom;
    if (width < settings_pos.right)
        pos.right = pos.left + settings_pos.right;
    else if (width > settings_pos.right)
        settings_pos.right += (width - settings_pos.right);
    floatwnd.MoveWindow(&pos);

    settings_pos.top += height;
    settings_pos.bottom += height;
    m_settings_wnd.MoveWindow(&settings_pos);
    m_button_wnd.MoveWindow(&settings_pos);
    m_settings_wnd.ShowWindow(SW_SHOW);
    m_editmode = true;
}

void ClickpadMainWnd::endEditMode()
{
    m_editmode = false;
}

void ClickpadMainWnd::onSetParentFocus()
{
    ::SetFocus(m_hwnd_client);
}

void ClickpadMainWnd::createButton(int x, int y)
{
    PadButton *b = new PadButton();
    int px = x * m_button_width;
    int py = y * m_button_height;
    RECT pos = { px, py, px+m_button_width, py+m_button_height };
    b->Create(m_hWnd, pos, L"-", WS_CHILD|WS_VISIBLE);
    m_buttons[y][x] = b;
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create("params");
    node.set("size", m_button_width);
    node.set("columns", getColumns());
    node.set("rows", getRows());
    node.create("/buttons");
    xml::node base(node);
    for (int y=0,rows=getRows();y<rows;++y) {
    for (int x=0,columns=getColumns();x<columns;++x) {
      PadButton *b = m_buttons[y][x];
      if (!b) continue;
      tstring tmp;
      b->getCommand(&tmp);
      //if (tmp.empty()) continue;
      node.create("button");
      node.set("x", x);
      node.set("y", y);
      node.set("command", tmp);
      b->getText(&tmp);
      node.set("text", tmp);
      node = base;
    }}
    node.move("/");
}

void ClickpadMainWnd::initDefault()
{
    setButtonsNet(1, 8);
    setWorkWindowSize();
}

void ClickpadMainWnd::load(xml::node& node)
{
    setWorkWindowSize();
}

void ClickpadMainWnd::setWorkWindowSize()
{    
    CWindow wnd(m_parent_window.floathwnd());
    RECT rc; wnd.GetWindowRect(&rc);
    int width = getColumns() * m_button_width + (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER)) * 2;
    int height = getRows() * m_button_height + (GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYBORDER)) * 2 + GetSystemMetrics(SM_CYCAPTION);
    rc.right = rc.left + width;
    rc.bottom = rc.top + height;
    wnd.MoveWindow(&rc);
}
