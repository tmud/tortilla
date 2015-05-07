#include "stdafx.h"
#include "mainwnd.h"
#include "clickpad.h"

SettingsDlg* m_settings;
ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_button_size(64), m_settings_dlg(NULL)
{
}

ClickpadMainWnd::~ClickpadMainWnd()
{
}

HWND ClickpadMainWnd::createSettingsDlg(HWND parent)
{
    m_settings_dlg = new SettingsDlg();
    m_settings_dlg->Create(parent);
    m_settings = m_settings_dlg;
    return m_settings_dlg->m_hWnd;
}

void ClickpadMainWnd::setEditMode(bool mode)
{
    m_editmode = mode;
}

void ClickpadMainWnd::onClickButton(int x, int y, bool up)
{
    if (!m_editmode)
    {
        if (up)
            setFocusToMudClient();
        else
        {
            PadButton *button = m_buttons[y][x];
            assert(button);
            tstring command;
            button->getCommand(&command);
            runGameCommand(command);
        }
        return;
    }
    PadButton *button = m_buttons[y][x];
    m_settings_dlg->editButton(button);
}

void ClickpadMainWnd::setButtonsNet(int rows, int columns)
{
    createNewRows(rows);
    createNewColumns(columns);
    for (int y=0; y<rows; ++y) {
    for (int x=0; x<columns; ++x) {
       createButton(x, y);
    }}
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

void ClickpadMainWnd::setRows(int count)
{
}

void ClickpadMainWnd::setColumns(int count)
{
}

void ClickpadMainWnd::setButtonSize(int size)
{
    m_button_size = size;
}

int ClickpadMainWnd::getButtonSize() const
{
    return m_button_size;
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
    //m_settings_wnd.Create(m_hWnd, rcDefault, WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    //m_button_wnd.Create(m_hWnd, rcDefault, WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
}

void ClickpadMainWnd::onDestroy()
{
    if (m_settings_dlg)
      { m_settings_dlg->DestroyWindow(); delete m_settings_dlg; m_settings = NULL; }
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

void ClickpadMainWnd::createButton(int x, int y)
{
    PadButton *b = new PadButton(WM_USER, MAKELONG(x, y));
    int px = x * m_button_size;
    int py = y * m_button_size;
    RECT pos = { px, py, px+m_button_size, py+m_button_size };
    b->Create(m_hWnd, pos, L"-", WS_CHILD|WS_VISIBLE);
    m_buttons[y][x] = b;
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create("params");
    node.set("size", m_button_size);
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
    CWindow wnd(getFloatWnd());
    RECT rc; wnd.GetWindowRect(&rc);
    int width = getColumns() * m_button_size + (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXBORDER)) * 2;
    int height = getRows() * m_button_size + (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYBORDER)) * 2 + GetSystemMetrics(SM_CYSMCAPTION);
    rc.right = rc.left + width - 1;
    rc.bottom = rc.top + height - 1;
    wnd.MoveWindow(&rc);
}
