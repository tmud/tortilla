#include "stdafx.h"
#include "propertiesPages/propertiesData.h"
#include "mudView.h"
#pragma warning(disable: 4996)

MudView::MudView(PropertiesElements *elements) : 
propElements(elements),
m_lines_count(0),
m_last_visible_line(-1),
m_last_string_updated(false),
drag_begin(-1), drag_end(-1)
{
}

MudView::~MudView()
{
    autodel<MudViewString> z(m_strings);
}

void MudView::accLastString(parseData *parse_data)
{
    if (!parse_data->update_prev_string ||
        m_strings.empty() ||
        parse_data->strings.empty())
        return;

    MudViewString *string = parse_data->strings[0];
    int last = m_strings.size() - 1;
    m_strings[last]->moveBlocks(string);
    delete string;

    parse_data->strings[0] = m_strings[last];
    m_strings.pop_back();                                   // remove last string from view
    m_last_string_updated = true;
}

void MudView::addText(parseData* parse_data, MudView* mirror)
{
    if (parse_data->strings.empty())
        return;

    bool lsu = m_last_string_updated;
    m_last_string_updated = false;

    removeDropped(parse_data);
    parseDataStrings &pds = parse_data->strings;
    for (int i=0,e=pds.size(); i<e; ++i)
    {
        MudViewString *string = pds[i];
        calcStringSizes(string);
        m_strings.push_back(string);
        if (mirror)
        {
            std::vector<MudViewString*> &ms = mirror->m_strings;            
            if (lsu)
            {
                lsu = false;
                int last_mirror = ms.size() - 1;
                delete ms[last_mirror];
                ms.pop_back();
            }
            MudViewString *ns = new MudViewString();
            ns->copy(string);
            ms.push_back(ns);
        }
    }
    pds.clear();
    checkLimit();

    int new_visible_line = m_strings.size() - 1;
    updateScrollbar(new_visible_line);
    Invalidate(FALSE);
}

void MudView::clearText()
{
    std::vector<MudViewString*>::iterator it = m_strings.begin(), it_end = m_strings.end();
    for (; it != it_end; ++it)
        delete (*it);
    m_strings.clear();
    setViewString(-1);
}

void MudView::truncateStrings(int maxcount)
{
    int size = m_strings.size();
    if (size <= maxcount)
        return;
    int count = size - maxcount;
    deleteStrings(count);
}

void MudView::setViewString(int index)
{
    updateScrollbar(index);
    Invalidate(FALSE);
}

int MudView::getViewString() const
{
    return m_last_visible_line;
}

int MudView::getLastString() const
{
    int last = m_strings.size() - (m_last_string_updated ? 0 : 1);
    return last;
}

bool MudView::isLastString() const
{
    return (getLastString() == getViewString()) ? true : false;
}

int MudView::getStringsOnDisplay() const
{
    return m_lines_count;
}

MudViewString* MudView::getString(int idx) const
{
    return m_strings[idx];
}

void MudView::updateProps()
{
    if (m_strings.empty())
        return;    
    for (int i=0,e=m_strings.size(); i<e; ++i)    
        calcStringSizes(m_strings[i]);
    initRenderParams();
    Invalidate();
}

void MudView::removeDropped(parseData* parse_data)
{
    parseDataStrings &pds = parse_data->strings;
    for (int i=pds.size()-1; i>=0; --i)
    {
        MudViewString *string = pds[i];
        if (string->dropped)
        {
            pds.erase(pds.begin() + i);
            delete string;
        }
    }
}

void MudView::calcStringSizes(MudViewString *string)
{
    CDC dc(GetDC());
    dc.SelectFont(propElements->standard_font);
    std::vector<MudViewStringBlock> &b = string->blocks;
    for (int j=0,je=b.size(); j<je; ++j)
    {
        const tstring &s = b[j].string;
        MudViewStringParams &p = b[j].params;
        if (p.underline_status || p.italic_status)
        {
             if (p.underline_status && p.italic_status)
                 dc.SelectFont(propElements->italic_underlined_font);
             else if (p.underline_status)
                 dc.SelectFont(propElements->underlined_font);
             else
                 dc.SelectFont(propElements->italic_font);
        }
        SIZE sz = {0,0};
        GetTextExtentPoint32(dc, s.c_str(), s.length(), &sz);
        if (p.italic_status)
            sz.cx += 2;
        b[j].size = sz;
        if (p.underline_status || p.italic_status)
            dc.SelectFont(propElements->standard_font);
    }
}

void MudView::renderView()
{
    RECT pos; GetClientRect(&pos);
    CPaintDC dc(m_hWnd);
    CMemoryDC mdc(dc, pos);
    mdc.FillRect(&pos, propElements->background_brush);
    if (m_strings.empty())
        return;
    if (m_last_visible_line == -1)
        return;

    int line_heigth = propElements->font_height;
    int index = m_last_visible_line - (m_last_string_updated ? 1 : 0);
    int count = m_lines_count + 1;
    int y = pos.bottom;                
    while (index >= 0 && count > 0)
    {
        renderString(&mdc, m_strings[index], 0, y, index);
        index--; count--;
        y = y - line_heigth;
    }
}

void MudView::renderString(CDC *dc, MudViewString *s, int left_x, int bottom_y, int index)
{
    int line_heigth = propElements->font_height;
    std::vector<MudViewStringBlock> &b = s->blocks;
    RECT pos = { left_x, bottom_y-line_heigth, left_x, bottom_y };
    dc->SelectFont(propElements->standard_font);
    if (b.empty() && checkDragging(index))
    {
        // dragging empty line
        pos.right = 16;
        COLORREF bkg =  invertColor(propElements->propData->bkgnd);
        dc->FillSolidRect(&pos, bkg);
        return;
    }

    for (int i=0,e=b.size(); i<e; ++i)
    {
        pos.left = pos.right;
        pos.right += b[i].size.cx;

        MudViewStringParams &p = b[i].params;
        COLORREF text_color = p.ext_text_color;
        COLORREF bkg_color = p.ext_bkg_color;
        if (!p.use_ext_colors)
        {
            tbyte txt = p.text_color;
            tbyte bkg = p.bkg_color;
            if (p.reverse_video) { tbyte x = txt; txt = bkg; bkg = x; }    
            if (txt <= 7 && p.intensive_status) // txt >= 0 always
                txt += 8;
            text_color = propElements->palette.getColor(txt);
            if (bkg == 0)
                bkg_color = propElements->propData->bkgnd;
            else
                bkg_color = propElements->palette.getColor(bkg);
        }
        else
        {
            if (p.reverse_video) {
                text_color = p.ext_bkg_color;
                bkg_color = p.ext_text_color;
            }
        }

        if (p.underline_status || p.italic_status)
        {
            if (p.underline_status && p.italic_status)
                dc->SelectFont(propElements->italic_underlined_font);
            else if (p.underline_status)                         
                dc->SelectFont(propElements->underlined_font);
            else
                dc->SelectFont(propElements->italic_font);
        }

        const tstring &s = b[i].string;
        if (checkDragging(index))
        {
            COLORREF bkg =  propElements->propData->bkgnd;
            bkg_color = invertColor(bkg);
            text_color = bkg;
        }

        dc->FillSolidRect(&pos, bkg_color);
        dc->SetBkColor(bkg_color);
        dc->SetTextColor(text_color);
        dc->DrawText(s.c_str(), -1, &pos, DT_CENTER|DT_SINGLELINE|DT_VCENTER);
        
        if (p.underline_status || p.italic_status)
            dc->SelectFont(propElements->standard_font);

        if (p.blink_status)
        {
                CPen pen;
                pen.CreatePen(PS_SOLID, 1, text_color);
                HPEN oldpen = dc->SelectPen(pen);
                dc->MoveTo(pos.left, pos.top, NULL); 
                dc->LineTo(pos.right-1, pos.top);
                dc->LineTo(pos.right-1, pos.bottom-1);
                dc->LineTo(pos.left, pos.bottom-1);
                dc->LineTo(pos.left, pos.top);
                dc->SelectPen(oldpen);
        }
    }
}

void MudView::initRenderParams()
{
    int line_heigth = propElements->font_height;
    RECT rc; GetClientRect(&rc);
    if (rc.right <= 0 || rc.bottom <= 0)        // check width and heigth
    {
        m_lines_count = 0;
        return;
    }
    m_lines_count = rc.bottom / line_heigth;
}

void MudView::updateScrollbar(int new_visible_line)
{
    int lines = m_strings.size();
    if (lines <= m_lines_count || m_lines_count == 0)
    {
        SetScrollRange(SB_VERT, 0, 0);
        SetScrollPos(SB_VERT, 0);
        m_last_visible_line = lines-1;
    }
    else
    {
        int max_sb = lines - m_lines_count;
        SetScrollRange(SB_VERT, 0, max_sb, FALSE);

        int max_visible = lines - 1;
        int min_visible = m_lines_count - 1;

        if (new_visible_line < min_visible)
            new_visible_line = min_visible;
        if (new_visible_line > max_visible)
            new_visible_line = max_visible;       

        m_last_visible_line = new_visible_line;
        int pos_sb = new_visible_line - m_lines_count + 1;        
        SetScrollPos(SB_VERT, pos_sb);
    }
}

void MudView::setScrollbar(DWORD position)
{
    int thumbpos = HIWORD(position);
    int action = LOWORD(position);
    
    int visible_line = m_last_visible_line;
    switch(action) {
    case SB_LINEUP:
         visible_line = visible_line - 1;
    break;
    case SB_LINEDOWN:
         visible_line = visible_line + 1;
    break;
    case SB_PAGEUP:
        visible_line = visible_line - m_lines_count;
    break;
    case SB_PAGEDOWN:
        visible_line = visible_line + m_lines_count;
    break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:        
        visible_line = (thumbpos + m_lines_count) - 1;
    break;
    }
    updateScrollbar(visible_line);
    Invalidate();    
}

void MudView::mouseWheel(WORD position)
{
    int direction = (position & 0x8000) ? 3 : -3;
    int visible_line = m_last_visible_line + direction;
    updateScrollbar(visible_line);
    Invalidate();
}

void MudView::checkLimit()
{
    int size = m_strings.size();
    if (size > propElements->propData->view_history_size)
    {
        size = size - propElements->propData->view_history_size;
        deleteStrings(size);
    }    
}

void MudView::deleteStrings(int count_from_begin)
{
    std::vector<MudViewString*>::iterator it = m_strings.begin(), it_end = m_strings.begin()+count_from_begin;
    for (; it != it_end; ++it)
        delete (*it);
    m_strings.erase(m_strings.begin(), m_strings.begin()+count_from_begin);
}

void MudView::startDraging()
{
    int line = getCurrentDraggingLine();
    if (line < 0)
        return;
    SetCapture();
    drag_begin = line;
    drag_end = line;
    Invalidate(FALSE);
}

void MudView::stopDraging()
{
    if (drag_begin < 0)
        return;

    ReleaseCapture();

    // copy to swap buffer
    if (drag_end < 0)
        drag_end = 0;
    if (drag_end < drag_begin)
        { int t = drag_end; drag_end = drag_begin; drag_begin = t; }

    tstring data;
    tstring text;
    for (int i=drag_begin; i<=drag_end; ++i)
    {
        m_strings[i]->getText(&text);
        data.append(text);
        data.append(L"\r\n");        
    }
    sendToClipboard(m_hWnd, data);
    drag_begin = -1;
    Invalidate(FALSE);
}

void MudView::doDraging()
{
    if (drag_begin < 0)
        return;
    POINT pt; GetCursorPos(&pt);        
    drag_end = getCurrentDraggingLine();
    if (drag_end > m_last_visible_line)
        drag_end = m_last_visible_line;
    int min_line = m_last_visible_line-m_lines_count;
    if (drag_end < min_line)
        drag_end = min_line;

    Invalidate(FALSE);
}

int MudView::getCurrentDraggingLine() const
{
     POINT pt; GetCursorPos(&pt);
     RECT pos; GetWindowRect(&pos);
     int y = pos.bottom - pt.y;
     int line_heigth = propElements->font_height;
     int line = y / line_heigth;
     return m_last_visible_line - line;    
}

bool MudView::checkDragging(int line)
{
    if (drag_begin < 0)
        return false;

    if (drag_end < drag_begin)
    {
        if (line >= drag_end && line <= drag_begin)
            return true;
    }
    else
    {
        if (line >= drag_begin && line <= drag_end)
            return true;
    }
    return false;
}
