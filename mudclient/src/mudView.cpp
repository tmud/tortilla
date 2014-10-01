#include "stdafx.h"
#include "propertiesPages/propertiesData.h"
#include "mudView.h"
#pragma warning(disable: 4996)

MudView::MudView(PropertiesElements *elements) : 
propElements(elements),
m_lines_count(0),
m_last_visible_line(-1),
m_last_string_updated(false),
drag_begin(-1), drag_end(-1),
drag_left(-1), drag_right(-1)
{
    m_dragpt.x = m_dragpt.y = 0;
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

    int last = m_strings.size() - 1;
    MudViewString *last_string = m_strings[last];    
    MudViewString *string = parse_data->strings[0];
    if (string->gamecmd && !last_string->ga)
        return;
    if (!string->gamecmd && last_string->ga)
        return;
    last_string->moveBlocks(string);
    delete string;
    parse_data->strings[0] = last_string;
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
    RECT pos = { left_x, bottom_y - line_heigth, left_x, bottom_y };
    if (b.empty() && checkDragging(index, true))
    {
        // dragging empty line
        pos.right += 16;
        COLORREF bkg = invertColor(propElements->propData->bkgnd);
        dc->FillSolidRect(&pos, bkg);
        return;
    }

    int start_sym = 0;
    dc->SelectFont(propElements->standard_font);
    for (int i = 0, e = b.size(); i < e; ++i)
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
        if (checkDraggingSym(index))
        {
            int end_sym = start_sym + s.length();
            int left = drag_left; int right = drag_right;
            if (left == -1) { left = drag_right; right = drag_left; }
            if (right == -1) right = m_drag_line_len.size()-1;
            if (left > right) { left = drag_right; right = drag_left; }
            if ((left >= start_sym && left < end_sym) ||        
                (right >= start_sym && right < end_sym) ||
                (left < start_sym && right >= end_sym)
               )
            {
                if (left >= start_sym && left < end_sym) left -= start_sym;
                else left = 0;
                if (right >= start_sym  && right < end_sym) right -= start_sym;
                else right = end_sym - 1;
              
                if (left != 0)
                {
                    RECT side = pos; side.right = m_drag_line_len[start_sym + left - 1];
                    renderDragSym(dc, s.substr(0, left), side, text_color, bkg_color);
                }
                // selected
                {
                    RECT side = pos; if (left != 0) side.left = m_drag_line_len[start_sym + left - 1];
                    if (right != (end_sym - 1)) side.right = m_drag_line_len[start_sym + right];
                    COLORREF txt0 = propElements->propData->bkgnd;
                    COLORREF bkg0 = invertColor(txt0);
                    renderDragSym(dc, s.substr(left, right-left+1), side, txt0, bkg0);                    
                }
                if (right != (end_sym - 1))
                {
                    RECT side = pos; side.left = m_drag_line_len[start_sym + right];                    
                    renderDragSym(dc, s.substr(right+1), side, text_color, bkg_color);
                }
                start_sym = end_sym;
                continue;
            }
        }

        if (checkDragging(index, false))
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
        start_sym += s.length();
    }
}

void MudView::renderDragSym(CDC *dc, const tstring& str, RECT& pos, COLORREF text, COLORREF bkg)
{
    dc->FillSolidRect(&pos, bkg);
    dc->SetBkColor(bkg);
    dc->SetTextColor(text);
    dc->DrawText(str.c_str(), -1, &pos, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
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
    m_dragpt = getCursor();
    int line = getCursorLine(m_dragpt.y);
    if (line < 0) return;
    SetCapture();
    drag_begin = line;
    drag_end = line;
    calcDragLine(line);
    drag_left = getCursorSym(m_dragpt.x);
    drag_right = drag_left;
    Invalidate(FALSE);
}

void MudView::stopDraging()
{
    if (drag_begin < 0)
        return;
    ReleaseCapture();

    if (drag_begin == drag_end)
    {
        tstring text;
        m_strings[drag_begin]->getText(&text);
        if (drag_left == -1)
        {
            if (drag_right != -1)
                text = text.substr(drag_right);
        }
        else if (drag_right == -1)
        {
            text = text.substr(drag_left);
        }
        else
        {
            if (drag_left > drag_right) { int t = drag_left; drag_left = drag_right; drag_right = t; }
            text = text.substr(drag_left, drag_right - drag_left + 1);
        }           
        sendToClipboard(m_hWnd, text);
    }
    else
    {
        if (drag_end < drag_begin) { int t = drag_end; drag_end = drag_begin; drag_begin = t; }
        tstring data;
        tstring text;
        for (int i = drag_begin; i <= drag_end; ++i)
        {
            m_strings[i]->getText(&text);
            data.append(text);
            data.append(L"\r\n");
        }
        sendToClipboard(m_hWnd, data);
    }
    drag_begin = -1;    
    Invalidate(FALSE);
}

void MudView::doDraging()
{
    if (drag_begin < 0)
        return;
    POINT pt = getCursor();
    drag_end = getCursorLine(pt.y);
    if (drag_begin != drag_end)
    {
        if (drag_end > m_last_visible_line)
            drag_end = m_last_visible_line;
        int min_line = m_last_visible_line - m_lines_count;
        if (drag_end < min_line)
            drag_end = min_line;
    }
    if (drag_begin == drag_end)
    {
        drag_right = getCursorSym(pt.x);
    }
    Invalidate(FALSE);
}

bool MudView::checkDragging(int line, bool accept_emptyline)
{
    if (drag_begin < 0)
        return false;
    if (!accept_emptyline && drag_begin == drag_end)
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
bool MudView::checkDraggingSym(int line)
{
    if (drag_begin < 0)
        return false;
    if (drag_begin != drag_end)
        return false;
    if (drag_begin != line)
        return false;
    return true;
}

POINT MudView::getCursor() const
{
    POINT pt; GetCursorPos(&pt);
    RECT pos; GetWindowRect(&pos);
    pt.y = pt.y - pos.top;
    pt.x = pt.x - pos.left;
    return pt;
}

int MudView::getCursorLine(int y) const
{
    RECT rc; GetClientRect(&rc);
    y = rc.bottom - y;
    int line_heigth = propElements->font_height;
    int line = y / line_heigth;
    return m_last_visible_line - line;
}

int MudView::getCursorSym(int x) const
{
    int left = 0;
    for (int i = 0, e = m_drag_line_len.size(); i < e; ++i)
    {
        int right = m_drag_line_len[i];
        if (x >= left && x < right) { return i; }
        left = right;
    }
    return -1;
}

void MudView::calcDragLine(int line)
{
    MudViewString *s = m_strings[line];
    int blocks = s->blocks.size();
    if (!blocks) { m_drag_line_len.clear(); return; }
    int dc_size = 0;
    for (int i = 0; i <blocks; ++i)
    {
        const MudViewStringBlock &b = s->blocks[i];
        dc_size += b.size.cx;
    }
    tstring text;
    s->getText(&text);
    CDC dc(GetDC());
    HFONT current_font = dc.SelectFont(propElements->standard_font);
    int maxchars = text.size();
    m_drag_line_len.resize(maxchars);
    SIZE sz = { 0 };
    GetTextExtentExPoint(dc, text.c_str(), text.length(), dc_size, &maxchars, &m_drag_line_len[0], &sz);
    dc.SelectFont(current_font);
}
