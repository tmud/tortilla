#include "stdafx.h"
#include "mainwnd.h"
#include "clickpad.h"

extern SettingsDlg* m_settings;
extern ImageCollection *m_image_collection;

ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_button_size(64), m_rows(0), m_columns(0)
{
}

ClickpadMainWnd::~ClickpadMainWnd()
{
}

PadButton* ClickpadMainWnd::getButton(int x, int y)
{
   int index = y*MAX_COLUMNS + x;
   assert(index >=0 && index<(int)m_buttons.size());
   PadButton *b = m_buttons[index];
   assert(b);
   return b;
}

void ClickpadMainWnd::setEditMode(bool mode)
{
    m_editmode = mode;
    m_settings->editButton(NULL);
    for (int y=0; y<m_rows; ++y) {
    for (int x=0; x<m_columns; ++x) {
    PadButton *b = getButton(x, y);
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
            PadButton *button = getButton(x, y);
            tstring command;
            button->getCommand(&command);
            processGameCommand(command, button->getTemplate());
        }
        return;
    }
    PadButton *button = getButton(x, y);
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
    showRows(count);
    setWorkWindowSize();
}

void ClickpadMainWnd::setColumns(int count)
{
    if (count <= 0) return;
    showColumns(count);
    setWorkWindowSize();
}

int ClickpadMainWnd::getButtonSize() const
{
    return m_button_size;
}

void ClickpadMainWnd::showRows(int count)
{
    if (count == m_rows) return;
    if (count > m_rows)
    {
        for (int y=m_rows; y<count; ++y)
         for (int x = 0; x < m_columns; ++x)
           showButton(x, y, true);
    }
    else
    {
        for (int y = count; y < m_rows; ++y)
         for (int x = 0; x < m_columns; ++x)
           showButton(x, y, false);
    }
    m_rows = count; 
}

void ClickpadMainWnd::showColumns(int count)
{
    if (count == m_columns) return;
    if (count > m_columns)
    {
        for (int x = m_columns; x < count; ++x)
         for (int y = 0; y < m_rows; ++y)
           showButton(x, y, true);
    }
    else
    {
        for (int x = count; x < m_columns; ++x)
         for (int y = 0; y < m_rows; ++y)
           showButton(x, y, false);
    }
    m_columns = count;
}

void ClickpadMainWnd::setButtonSize(int size)
{
    m_button_size = size;
    for (int y=0; y<MAX_ROWS; ++y) {
    for (int x=0; x<MAX_COLUMNS; ++x) {
       PadButton *b = getButton(x, y);
       int px = x * m_button_size + 2;
       int py = y * m_button_size + 2;
       RECT pos = { px, py, px+m_button_size, py+m_button_size };
       b->MoveWindow(&pos);
    }}
    setWorkWindowSize();
}

void ClickpadMainWnd::onCreate()
{
    m_buttons.resize(MAX_COLUMNS*MAX_ROWS, NULL);
    for (int x=0; x<MAX_COLUMNS; x++) {
    for (int y=0; y<MAX_ROWS; y++) {
      PadButton *b = new PadButton(WM_USER, MAKELONG(x, y));
      int px = x * m_button_size + 2;
      int py = y * m_button_size + 2;
      RECT pos = { px, py, px + m_button_size, py + m_button_size };
      b->Create(m_hWnd, pos, L"", WS_CHILD);
      int index = y*MAX_COLUMNS + x;
      m_buttons[index] = b;
    }}
}

void ClickpadMainWnd::onDestroy()
{
    for (int i=0,e=m_buttons.size();i<e;++i)
    {
        PadButton *b = m_buttons[i];
        b->DestroyWindow();
        delete b;
    }
    m_buttons.clear();
}

void ClickpadMainWnd::onSize()
{
}

void ClickpadMainWnd::showButton(int x, int y, bool show)
{
    getButton(x, y)->ShowWindow(show ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create("params");
    node.set("size", m_button_size);
    node.set("columns", m_columns);
    node.set("rows", m_rows);
    node.create("/buttons");
    xml::node base(node);
    for (int y=0;y<MAX_ROWS;++y) {
    for (int x=0;x<MAX_COLUMNS;++x) {
      PadButton *b = getButton(x, y);
      if (b->isEmptyButton())
          continue;

      tstring cmd, text;
      b->getCommand(&cmd);      
      b->getText(&text);
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
          m_image_collection->save(image, &image_params);
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
    
    for (int y = 0; y < MAX_ROWS; ++y) {
     for (int x = 0; x < MAX_COLUMNS; ++x) {
       getButton(x, y)->clear();
    }}

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

            PadButton *b = getButton(x, y);
            b->setText(text);
            b->setCommand(cmd);
            b->setTemplate( (template_flag==1) ? true : false);
            tstring image_params;
            if (n.get("image", &image_params))
            {
                ClickpadImage *image = m_image_collection->load(image_params);                
                b->setImage(image);
            }
        }
    }
    int rows = 0; int columns = 0;
    if (node.get("params/rows", &rows) && node.get("params/columns", &columns))
    {
        if (columns >= 1 && columns <=MAX_COLUMNS && rows >= 1 && rows <= MAX_ROWS)
        {
            showRows(rows);
            showColumns(columns);
        }
    }
}

void ClickpadMainWnd::setWorkWindowSize()
{    
    CWindow wnd(getFloatWnd());
    RECT rc; wnd.GetWindowRect(&rc);
    int width = getColumns() * m_button_size + (GetSystemMetrics(SM_CXFRAME) /*+ GetSystemMetrics(SM_CXBORDER)*/) * 2;
    int height = getRows() * m_button_size + (GetSystemMetrics(SM_CYFRAME) /*+ GetSystemMetrics(SM_CYBORDER)*/) * 2 + GetSystemMetrics(SM_CYSMCAPTION);
    
    //rc.right = rc.left + width + 4;
    //rc.bottom = rc.top + height + 4;
    rc.right = rc.left + 900;
    rc.bottom = rc.top + 500;

    wnd.MoveWindow(&rc);
}
