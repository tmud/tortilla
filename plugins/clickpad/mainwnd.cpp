#include "stdafx.h"
#include "mainwnd.h"
#include "clickpad.h"

extern SettingsDlg* m_settings;
ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_button_size(64), m_rows(0), m_columns(0)
{
}

ClickpadMainWnd::~ClickpadMainWnd()
{
}

void ClickpadMainWnd::setEditMode(bool mode)
{
    m_editmode = mode;
    m_settings->editButton(NULL);
    int hrows = m_buttons.size();
    int hcolumns = (hrows == 0) ? 0 : m_buttons[0].size();
    for (int y=0; y<hrows; ++y) {
    for (int x=0; x<hcolumns; ++x) {
    PadButton *b = m_buttons[y][x];
    if (b->isEmptyButton())
        b->ShowWindow(mode ? SW_SHOWNOACTIVATE : SW_HIDE);
    }}
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
            processGameCommand(command, button->getTemplate());
        }
        return;
    }
    PadButton *button = m_buttons[y][x];
    m_settings->editButton(button);
}

int ClickpadMainWnd::getRows() const
{
    return m_rows;
}

int ClickpadMainWnd::getColumns() const
{
    return m_columns;
}

void ClickpadMainWnd::setRows(int count)
{
    if (count <= 0) return;
    m_rows = count;
    setRowsInArray(count);
    setWorkWindowSize();
}

void ClickpadMainWnd::setColumns(int count)
{
    if (count <= 0) return;
    m_columns = count;
    setColumnsInArray(count);
    setWorkWindowSize();
}

void ClickpadMainWnd::setRowsInArray(int count)
{    
    int hrows = m_buttons.size();
    int hcolumns = (hrows == 0) ? 0 : m_buttons[0].size();
    int add = count - hrows;
    if (add > 0)
    {  // add new rows
        std::vector<PadButton*> newrow(hcolumns, NULL);
        for (;add > 0; --add) 
        {
            int y = m_buttons.size();
            m_buttons.push_back(newrow);
            for (int x=0; x<hcolumns; ++x)
                createButton(x, y);
        }
    }
}

void ClickpadMainWnd::setColumnsInArray(int count)
{
    int hrows = m_buttons.size();
    int hcolumns = (hrows == 0) ? 0 : m_buttons[0].size();
    int add = count - hcolumns;
    if (add > 0) 
    {   // add new columns
        for (int i=0; i<hrows; ++i)
        {
            m_buttons[i].resize(count, NULL);
            for (int j=hcolumns; j<count; ++j)
                createButton(j, i);
        }
    }
}

void ClickpadMainWnd::setButtonSize(int size)
{
    m_button_size = size;
    int hrows = m_buttons.size();
    int hcolumns = (hrows == 0) ? 0 : m_buttons[0].size();
    for (int y=0; y<hrows; ++y) {
    for (int x=0; x<hcolumns; ++x) {
       PadButton *b = m_buttons[y][x];
       if (!b) continue;
       int px = x * m_button_size;
       int py = y * m_button_size;
       RECT pos = { px, py, px+m_button_size, py+m_button_size };
       b->MoveWindow(&pos);
    }}
    setWorkWindowSize();
}

int ClickpadMainWnd::getButtonSize() const
{
    return m_button_size;
}

void ClickpadMainWnd::onCreate()
{
}

void ClickpadMainWnd::onDestroy()
{
}

void ClickpadMainWnd::onSize()
{
}

void ClickpadMainWnd::createButton(int x, int y)
{
    if (m_buttons[y][x])
        return;
    PadButton *b = new PadButton(WM_USER, MAKELONG(x, y));
    int px = x * m_button_size + 2;
    int py = y * m_button_size + 2;
    RECT pos = { px, py, px+m_button_size, py+m_button_size };
    b->Create(m_hWnd, pos, L"", WS_CHILD|WS_VISIBLE);
    m_buttons[y][x] = b;
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create("params");
    node.set("size", m_button_size);
    node.set("columns", m_columns);
    node.set("rows", m_rows);
    int hrows = m_buttons.size();
    int hcolumns = (hrows == 0) ? 0 : m_buttons[0].size();
    node.create("/buttons");
    xml::node base(node);
    for (int y=0;y<hrows;++y) {
    for (int x=0;x<hcolumns;++x) {
      PadButton *b = m_buttons[y][x];
      tstring text, cmd;
      b->getText(&text);
      b->getCommand(&cmd);
      if (text.empty() && cmd.empty()) continue;
      node.create("button");
      node.set("x", x);
      node.set("y", y);
      node.set("text", text);
      node.set("command", cmd);
      node.set("template", b->getTemplate() ? 1 : 0);
      ClickpadImage *image = b->getImage();
      if (image)
      {
          tstring image_params;
          image->save(&image_params);
          if (!image_params.empty())
            node.set("image", image_params);
      }
      node = base;
    }}
    node.move("/");
}

void ClickpadMainWnd::load(xml::node& node)
{
    int size = 0;
    node.get("params/size", &size);
    ButtonSizeTranslator bt;
    if (!bt.checkSize(size))
       size = bt.getDefaultSize();
    m_button_size = size;

    tstring text, cmd;
    xml::request buttons(node, "buttons/button");
    for (int i=0,e=buttons.size(); i<e; ++i)
{
        xml::node n = buttons[i];
        int x = 0; int y = 0;
        if (n.get("x", &x) && n.get("y", &y) && n.get("text", &text) && n.get("command", &cmd) &&
            (x >= 0 && x <= MAX_COLUMNS-1 && y >= 0 && y <= MAX_ROWS-1))
        {
            int template_flag = 0;
            n.get("template", &template_flag);
            
            setRowsInArray(y+1);
            setColumnsInArray(x+1);
            PadButton *b = m_buttons[y][x];
            b->setText(text);
            b->setCommand(cmd);
            b->setTemplate( (template_flag==1) ? true : false);
            tstring image_params;
            if (n.get("image", &image_params))
                b->setImage(image_params);
        }
    }
    int rows = 0; int columns = 0;
    if (node.get("params/rows", &rows) && node.get("params/columns", &columns))
    {
        if (columns >= 1 && columns <=MAX_COLUMNS && rows >= 1 && rows <= MAX_ROWS)
        {
            setRowsInArray(rows);
            setColumnsInArray(columns);
            m_rows = rows;
            m_columns = columns;
        }
    }
}

void ClickpadMainWnd::setWorkWindowSize()
{    
    CWindow wnd(getFloatWnd());
    RECT rc; wnd.GetWindowRect(&rc);
    int width = getColumns() * m_button_size + (GetSystemMetrics(SM_CXFRAME) /*+ GetSystemMetrics(SM_CXBORDER)*/) * 2;
    int height = getRows() * m_button_size + (GetSystemMetrics(SM_CYFRAME) /*+ GetSystemMetrics(SM_CYBORDER)*/) * 2 + GetSystemMetrics(SM_CYSMCAPTION);
    rc.right = rc.left + width + 4;
    rc.bottom = rc.top + height + 4;
    wnd.MoveWindow(&rc);
}
