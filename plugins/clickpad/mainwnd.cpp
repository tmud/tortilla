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
    m_settings_wnd.Create(m_hWnd, rcDefault, WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    setButtonsNet(2, 5);
}

void ClickpadMainWnd::onSize()
{
    RECT pos;
    GetClientRect(&pos);
    if (!m_editmode)
        return;
    pos.top = pos.bottom / 2;
    m_settings_wnd.MoveWindow(&pos);
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
    int height = pos.bottom;
    floatwnd.GetWindowRect(&pos);
    
    RECT settings_pos;
    m_settings_wnd.GetClientRect(&settings_pos);
    pos.bottom += settings_pos.bottom;
    floatwnd.MoveWindow(&pos);

    settings_pos.top += height;
    settings_pos.bottom += height;
    m_settings_wnd.MoveWindow(&settings_pos);
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
    b->Create(m_hWnd, pos, L"test", WS_CHILD|WS_VISIBLE);
    m_buttons[y][x] = b;
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create("params");
    node.set("width", m_button_width);
    node.set("height", m_button_height);
    node.set("columns", getColumns());
    node.set("rows", getRows());
    node.create("/buttons");
    for (int y=0,rows=getRows();y<rows;++y) {
    for (int x=0,columns=getColumns();x<columns;++x) {
      PadButton *b = m_buttons[y][x];
      if (!b) continue;

    }}
    node.move("/");
}

void ClickpadMainWnd::initDefault()
{

}

void ClickpadMainWnd::load(xml::node& node)
{

}