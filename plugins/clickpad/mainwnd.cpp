﻿#include "stdafx.h"
#include "mainwnd.h"
#include "clickpad.h"

extern SettingsDlg* m_settings;
extern ImageCollection *m_image_collection;

ClickpadMainWnd::ClickpadMainWnd() : m_editmode(false),
m_button_size(64), m_rows(0), m_columns(0), m_backgroundColor(RGB(1,1,1))
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
    if (mode) {
        TTActivate(FALSE);
    } else {
        for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_columns; ++x) {
           updateTooltip(getButton(x, y), true);
        }}
        TTActivate(TRUE);
    }
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
            std::wstring command;
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

bool ClickpadMainWnd::checkRowColumn(int row, int column)
{
    return (row >= 1 && row <= MAX_ROWS && column >= 1 && column <= MAX_COLUMNS) ? true : false;
}

bool ClickpadMainWnd::showRowsColumns(int rows, int columns)
{
    if (!checkRowColumn(rows, columns))
        return false;
    m_rows = rows;
    m_columns = columns;
    setWorkWindowSize();
    return true;
}

bool ClickpadMainWnd::setButton(int row, int column, const ButtonParams& p)
{
    if (!checkRowColumn(row, column))
        return false;
    PadButton* pb = getButton(column-1, row-1);
    if (!pb)
        return false;
    pb->setText(p.text);
    pb->setCommand(p.cmd);
    pb->setTooltip(p.tooltip);
    pb->setTemplate(p.templ);
    ClickpadImage *image = m_image_collection->load(p.imagefile, p.imagex, p.imagey);
    pb->setImage(image);
    showButton(pb,true);
    return true;
}

bool ClickpadMainWnd::updateButton(int row, int column, const ButtonParams& p)
{
    if (!checkRowColumn(row, column))
        return false;
    PadButton* pb = getButton(column-1, row-1);
    if (!pb)
        return false;
    if (p.update & ButtonParams::TEXT)
        pb->setText(p.text);
    if (p.update & ButtonParams::CMD)
        pb->setCommand(p.cmd);
    if (p.update & ButtonParams::TOOLTIP)
        pb->setText(p.tooltip);
    if (p.update & ButtonParams::IMAGE)
    {
        int x = 0; int y = 0;
        if (p.update & ButtonParams::IMAGEXY)
        {
            x = p.imagex; y = p.imagey;        
        }
        ClickpadImage *image = m_image_collection->load(p.imagefile, x, y);
        if (image)
            pb->setImage(image);
    }
    if (p.update & ButtonParams::TEMPLATE)
        pb->setTemplate(p.templ);
    showButton(pb,true);
    return true;
}

bool ClickpadMainWnd::getButton(int row, int column, ButtonParams* p)
{
    if (!checkRowColumn(row, column))
        return false;
    PadButton* pb = getButton(column-1, row-1);
    if (!pb)
        return false;
    pb->getText(&p->text);
    if (!p->text.empty())
        p->update |= ButtonParams::TEXT;
    pb->getCommand(&p->cmd);
    if (!p->cmd.empty())
        p->update |= ButtonParams::CMD;
    pb->getTooltip(&p->tooltip);
    if (!p->tooltip.empty())
        p->update |= ButtonParams::TOOLTIP;
    ClickpadImage *image = pb->getImage();
    if (image)
    {
        const ClickpadImageParams&cp = image->params();
        p->imagefile = cp.atlas_filename;
        p->update |= ButtonParams::IMAGE;
        if (cp.atlas_x > 0 || cp.atlas_y > 0)
        {
            p->imagex = cp.atlas_x; p->imagey = cp.atlas_y;
            p->update |= ButtonParams::IMAGEXY;
        }
        else
        {
            p->imagex = 0; p->imagey = 0;
        }
    }
    bool templ = pb->getTemplate();
    if (templ)
    {
        p->templ = true;
        p->update |= ButtonParams::TEMPLATE;
    }
    return true;
}

bool ClickpadMainWnd::clearButton(int row, int column)
{
    if (!checkRowColumn(row, column))
        return false;
    PadButton* pb = getButton(column-1, row-1);
    if (!pb)
        return false;
    pb->clear();
    showButton(pb,false);
    return true;
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
           showButton(getButton(x, y), true);
    }
    else
    {
        for (int y = count; y < m_rows; ++y)
         for (int x = 0; x < m_columns; ++x)
           showButton(getButton(x, y), false);
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
           showButton(getButton(x, y), true);
    }
    else
    {
        for (int x = count; x < m_columns; ++x)
         for (int y = 0; y < m_rows; ++y)
           showButton(getButton(x, y), false);
    }
    m_columns = count;
}

void ClickpadMainWnd::setFont(LOGFONT font)
{
  m_logfont = font;
  if (!m_buttons_font.IsNull())
      m_buttons_font.DeleteObject();
  HFONT hfont = m_buttons_font.CreateFontIndirect(&m_logfont);
  for (int y = 0; y < MAX_ROWS; ++y) {
  for (int x = 0; x < MAX_COLUMNS; ++x) {
  PadButton *b = getButton(x, y);
    b->setFont(hfont);
  }}
}

void ClickpadMainWnd::getFont(LOGFONT* font) const
{
    *font = m_logfont;
}

void ClickpadMainWnd::setButtonSize(int size)
{
    m_button_size = size;
    for (int y=0; y<MAX_ROWS; ++y) {
    for (int x=0; x<MAX_COLUMNS; ++x) {
       PadButton *b = getButton(x, y);
       int px = x * (m_button_size+2) + 2;
       int py = y * (m_button_size+2) + 2;
       RECT pos = { px, py, px+m_button_size, py+m_button_size };
       b->MoveWindow(&pos);
    }}
    setWorkWindowSize();
}

void ClickpadMainWnd::onCreate()
{
    TTInit();
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

void ClickpadMainWnd::onPaint(HDC dc)
{
    RECT rc; GetClientRect(&rc);
    CDCHandle hdc(dc);
    hdc.FillSolidRect(&rc, m_backgroundColor);
}

void ClickpadMainWnd::showButton(PadButton*b, bool show)
{
    b->ShowWindow(show ? SW_SHOWNOACTIVATE : SW_HIDE);
    updateTooltip(b, show);
}

void ClickpadMainWnd::updateTooltip(PadButton*b, bool show)
{
    std::wstring tooltip;
    b->getTooltip(&tooltip);
    HWND button = b->m_hWnd;
    TTRemove(button);
    if (show && !tooltip.empty()) {
        TTAdd(button);
        TTSetTxt(button, tooltip.c_str());
    }
}

void ClickpadMainWnd::updated()
{
    luaT_Props p(getLuaState());
    COLORREF c = p.backgroundColor();
    if (c == m_backgroundColor) return;
    for (int i=0,e=m_buttons.size();i<e;++i)
        m_buttons[i]->setBackgroundColor(c);
    m_backgroundColor = c;
    Invalidate(FALSE);
}

void ClickpadMainWnd::save(xml::node& node)
{
    node.create(L"font");
    node.set(L"name", m_logfont.lfFaceName);
    node.set(L"height", MulDiv(-m_logfont.lfHeight, 72, GetDeviceCaps(GetDC(), LOGPIXELSY)));
    node.set(L"bold", m_logfont.lfWeight);
    node.set(L"italic", m_logfont.lfItalic);

    node.create(L"/params");
    node.set(L"size", m_button_size);
    node.set(L"columns", m_columns);
    node.set(L"rows", m_rows);
    node.create(L"/buttons");
    xml::node base(node);
    for (int y=0;y<MAX_ROWS;++y) {
    for (int x=0;x<MAX_COLUMNS;++x) {
      PadButton *b = getButton(x, y);
      if (b->isEmptyButton())
          continue;

      std::wstring cmd, text, ttip;
      b->getCommand(&cmd);
      b->getText(&text);
      b->getTooltip(&ttip);
      node.create(L"button");
      node.set(L"x", x);
      node.set(L"y", y);
      node.set(L"text", text);
      node.set(L"command", cmd);
      if (!ttip.empty())
        node.set(L"tooltip", ttip);
      node.set(L"template", b->getTemplate() ? 1 : 0);
      ClickpadImage *image = b->getImage();
      if (image)
      {
          std::wstring image_params;
          m_image_collection->save(image, &image_params);
          if (!image_params.empty())
            node.set(L"image", image_params);
      }
      node = base;
    }}
    node.move(L"/");
}

void ClickpadMainWnd::load(xml::node& node)
{
    LOGFONT lf;
    initLogFont(&lf);
    xml::request fnode(node, L"font");
    if (fnode.size() == 1)
    {
        xml::node font(fnode[0]);
        std::wstring fname; int height = 0; int bold = 0; int italic = 0;
        if (font.get(L"name", &fname) && font.get(L"height", &height) && 
            font.get(L"bold", &bold) && font.get(L"italic", &italic))
        {
            wcscpy(lf.lfFaceName, fname.c_str());
            lf.lfHeight = -MulDiv(height, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
            lf.lfItalic = (italic) ? 1 : 0;
            lf.lfWeight = bold;
        }
    }

    int size = 0;
    node.get(L"params/size", &size);
    ButtonSizeTranslator bt;
    if (!bt.checkSize(size))
       size = bt.getDefaultSize();
    m_button_size = size;

    m_buttons.resize(MAX_COLUMNS*MAX_ROWS, NULL);
    for (int x = 0; x < MAX_COLUMNS; x++) {
        for (int y = 0; y < MAX_ROWS; y++) {
            PadButton *b = new PadButton(WM_USER, MAKELONG(x, y));
            int px = x * (m_button_size+2) + 2;
            int py = y * (m_button_size+2) + 2;
            RECT pos = { px, py, px + m_button_size, py + m_button_size };
            b->Create(m_hWnd, pos, L"", WS_CHILD);
            int index = y*MAX_COLUMNS + x;
            m_buttons[index] = b;
        }
    }
    std::wstring text, cmd;
    xml::request buttons(node, L"buttons/button");
    for (int i=0,e=buttons.size(); i<e; ++i)
    {
        xml::node n = buttons[i];
        int x = 0; int y = 0;
        if (n.get(L"x", &x) && n.get(L"y", &y) && n.get(L"text", &text) && n.get(L"command", &cmd) &&
            (x >= 0 && x <= MAX_COLUMNS-1 && y >= 0 && y <= MAX_ROWS-1))
        {
            int template_flag = 0;
            n.get(L"template", &template_flag);

            PadButton *b = getButton(x, y);
            b->setText(text);
            b->setCommand(cmd);
            b->setTemplate( (template_flag==1) ? true : false);
            std::wstring image_params;
            if (n.get(L"image", &image_params))
            {
                ClickpadImage *image = m_image_collection->load(image_params);
                b->setImage(image);
            }
            std::wstring ttip;
            if (n.get(L"tooltip", &ttip)) {
                b->setTooltip(ttip);
            } else {
                b->setTooltip(L"");
            }
        }
    }
    int rows = 0; int columns = 0;
    if (node.get(L"params/rows", &rows) && node.get(L"params/columns", &columns))
    {
        if (columns >= 1 && columns <=MAX_COLUMNS && rows >= 1 && rows <= MAX_ROWS)
        {
            showRows(rows);
            showColumns(columns);
        }
    }
    setFont(lf);
    PostMessage(WM_USER+1);
}

void ClickpadMainWnd::setWorkWindowSize()
{
    CWindow wnd(getFloatWnd());
    RECT rc; wnd.GetWindowRect(&rc);
    int width = getColumns() * (m_button_size+2) + (GetSystemMetrics(SM_CXFRAME)) * 2 + 2;
    int height = getRows() * (m_button_size+2) + (GetSystemMetrics(SM_CYFRAME)) * 2 + GetSystemMetrics(SM_CYSMCAPTION) + 2;
    rc.right = rc.left + width;
    rc.bottom = rc.top + height;
    wnd.MoveWindow(&rc);
}

void ClickpadMainWnd::initLogFont(LOGFONT *f)
{
    f->lfHeight = -MulDiv(8, GetDeviceCaps(GetDC(), LOGPIXELSY), 72);
    f->lfWidth = 0;
    f->lfEscapement = 0;
    f->lfOrientation = 0;
    f->lfWeight = FW_NORMAL;
    f->lfItalic = 0;
    f->lfUnderline = 0;
    f->lfStrikeOut = 0;
    f->lfCharSet = DEFAULT_CHARSET;
    f->lfOutPrecision = OUT_DEFAULT_PRECIS;
    f->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    f->lfQuality = DEFAULT_QUALITY;
    f->lfPitchAndFamily = DEFAULT_PITCH;
    wcscpy(f->lfFaceName, L"Tahoma");
}
