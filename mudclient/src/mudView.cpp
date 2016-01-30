#include "stdafx.h"
#include "propertiesPages/propertiesData.h"
#include "mudView.h"
#pragma warning(disable: 4996)

MudView::MudView(PropertiesElements *elements) : 
propElements(elements),
m_lines_count(0),
m_last_visible_line(-1),
m_last_string_updated(false),
m_use_softscrolling(false),
m_start_softscroll(-1),
drag_begin(-1), drag_end(-1),
drag_left(-1), drag_right(-1),
m_find_string_index(-1), m_find_start_pos(-1), m_find_end_pos(-1)
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

    bool space_in_prompt = false;
    if (last_string->prompt && string->blocks.size() == 1)
    {
        const tstring &tmp = string->blocks[0].string;
        if (tmp.empty() || tmp.find_first_not_of(L' ') == -1)
            space_in_prompt = true;
    }

    if (!space_in_prompt) {
    if (last_string->prompt && last_string->gamecmd)
        return;
    if (string->gamecmd && !last_string->prompt)
        return;
    if (!string->gamecmd && last_string->prompt)
        return;
    if (string->gamecmd && string->blocks.empty())
    {
        MudViewStringBlock empty;
        last_string->blocks.push_back(empty);
    }}

    last_string->moveBlocks(string);
    delete string;
    parse_data->strings[0] = last_string;
    m_strings.pop_back();                                   // remove last string from view
    m_last_string_updated = true;
}

int MudView::getStringsCount() const
{
    return m_strings.size();
}

void MudView::addText(parseData* parse_data, parseData *copy_data)
{
    if (parse_data->strings.empty())
        return;

    m_last_string_updated = false;

    removeDropped(parse_data);
    calcStringsSizes(parse_data->strings);
    if (copy_data)
    {
        copy_data->update_prev_string = parse_data->update_prev_string;
        copy_data->last_finished = parse_data->last_finished;
        parseDataStrings &s = parse_data->strings;
        int count = s.size();
        copy_data->strings.resize(count);
        for (int i=0;i<count;++i)
        {
            MudViewString *hs = new MudViewString;
            hs->copy(s[i]);
            copy_data->strings[i] = hs;
        }
    }
    pushText(parse_data);

    if (m_use_softscrolling) {
        if (m_start_softscroll == -1)
            m_start_softscroll = (m_last_visible_line == -1) ? 0 : m_last_visible_line;
        return;
    }

    if (isDragMode())
    {
        updateScrollbar(m_last_visible_line);
        return;
    }

    checkLimit();
    int new_visible_line = m_strings.size() - 1;
    updateScrollbar(new_visible_line);
    Invalidate(FALSE);
}

void MudView::pushText(parseData* parse_data)
{
    parseDataStrings &pds = parse_data->strings;
    m_strings.insert(m_strings.end(), pds.begin(), pds.end());
    pds.clear();
}

void MudView::updateSoftScrolling()
{
    if (!m_use_softscrolling || m_start_softscroll == -1)
        return;
    if (isDragMode())
    {
       updateScrollbar(m_last_visible_line);
       return;
    }

    int last_string = getLastString();
    int new_visible_line = last_string;
    if (m_last_visible_line != last_string)
    {
        int count = last_string - m_last_visible_line;
        int p = min((count / 10)+1 , 3);
        if (count >= (m_lines_count * 2))
            new_visible_line = last_string - m_lines_count;
        else if (count > p*3)
            new_visible_line = m_last_visible_line + p*3;
        else if (count > p*2)
            new_visible_line = m_last_visible_line + p*2;
        else if (count > p)
            new_visible_line = m_last_visible_line + p;
    }
    if (new_visible_line == last_string)
    {
        stopSoftScroll();
        checkLimit();
    }
    updateScrollbar(new_visible_line);
    Invalidate(FALSE);
}

void MudView::setSoftScrollingMode(bool mode)
{
    m_use_softscrolling = mode;
}

void MudView::clearText()
{
    mudViewStrings::iterator it = m_strings.begin(), it_end = m_strings.end();
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
    deleteBeginStrings(count);
}

void MudView::setViewString(int index)
{
    stopSoftScroll();
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

bool MudView::isLastStringUpdated() const
{
    return m_last_string_updated;
}

void MudView::deleteLastString()
{
    if (m_strings.empty())
        return;
    int last = m_strings.size() - 1;
    delete m_strings[last];
    m_strings.pop_back();
}

int MudView::getStringsOnDisplay() const
{
    return m_lines_count;
}

int MudView::getSymbolsOnDisplay() const
{
    int font_width = propElements->font_width;
    if (font_width < 1)
        return 0;
    RECT rc;
    GetClientRect(&rc);
    int count = rc.right / font_width;
    int dc = count / 40; // погрешность
    if (dc < 3) dc = 3;
    return count - dc;
}

MudViewString* MudView::getString(int idx) const
{
    return m_strings[idx];
}

void MudView::updateProps()
{
    if (m_strings.empty())
    {
        Invalidate(FALSE);
        return;
    }
    calcStringsSizes(m_strings);
    initRenderParams();
    Invalidate(FALSE);
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

void MudView::calcStringsSizes(mudViewStrings& pds)
{
    if (pds.empty())
        return;
    CDC dc(GetDC());
    HFONT oldfont = dc.SelectFont(propElements->standard_font);
    for (int i=0,e=pds.size(); i<e; ++i) {
    MudViewString *string = pds[i];
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
    }}
    dc.SelectFont(oldfont);
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

struct DragParamsChecker
{
    DragParamsChecker(int &l, int &r, int &s, int &e) : left(l), right(r), start_sym(s), end_sym(e) {}
    bool check() 
    {
        if ((left >= start_sym && left < end_sym) ||
            (right >= start_sym && right < end_sym) ||
            (left < start_sym && right >= end_sym))
        {
            if (left >= start_sym && left < end_sym) left -= start_sym;
            else left = 0;
            if (right >= start_sym  && right < end_sym) right -= start_sym;
            else right = end_sym - 1;
            return true;
        }
        return false;
    }
    int left, right, start_sym, end_sym;
};

void MudView::renderString(CDC *dc, MudViewString *s, int left_x, int bottom_y, int index)
{
    int line_heigth = propElements->font_height;
    RECT pos = { left_x, bottom_y - line_heigth, left_x, bottom_y };
    if (checkDragging(index, true) && s->getTextLen()==0)
    {
        // dragging empty line
        pos.right += 16;
        COLORREF bkg = invertColor(propElements->propData->bkgnd);
        dc->FillSolidRect(&pos, bkg);
        return;
    }

    int start_sym = 0;
    std::vector<MudViewStringBlock> &b = s->blocks;
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

        bool dragging = false;
        const tstring &str = b[i].string;
        if (checkDragging(index, false))
        {
            dragging = true;
            COLORREF bkg =  propElements->propData->bkgnd;
            bkg_color = invertColor(bkg);
            text_color = bkg;
        }
        else if (checkDragging(index, true))
        {
            dragging = true;
            const std::vector<int> &ld = (index == drag_begin) ? m_drag_beginline_len : m_drag_endline_len;
            int last = ld.size() - 1;
            int left = drag_left;
            int right = drag_right;

            if (drag_begin == drag_end)
            {
                if (left == -1) {
                    if (right != -1) { left = right; right = last; }
                    else { left = 0; right = last; }
                } else if (right == -1) {
                    if (isDragCursorLeft()) { right = left; left = 0; }
                    else { right = last; }
                }
                else if (left > right) { int t = left; left = right; right = t; }
            }
            else if (drag_begin < drag_end) // сверху вниз
            {
                if (index == drag_begin) {
                    if (left == -1) left = 0;
                    right = last;
                } else {
                    if (right == -1) right = last;
                    left = 0;
                }
            }
            else // drag_begin > drag_end  снизу вверх
            {
                if (index == drag_begin) {
                    if (left == -1) { right = last; }
                    else { right = left; }
                    left = 0;
                } else {
                    if (right == -1) { left = 0; }
                    else { left = right; }
                    right = last;
                }
            }

            // проверка что блок попадает в вычисленный диапазон (частью или целиком)
            // и вычисляем в left right - что выделено, но в символьных координатах блока
            int end_sym = start_sym + str.size();
            DragParamsChecker dpc(left, right, start_sym, end_sym);
            if (dpc.check())
            {
                if (left != 0)
                {
                    RECT side = pos; side.right = ld[start_sym + left - 1];
                    renderDragSym(dc, str.substr(0, left), side, text_color, bkg_color);
                }
                // selected
                {
                    RECT side = pos; if (left != 0) side.left = ld[start_sym + left - 1];
                    if (right != (end_sym - 1)) side.right = ld[start_sym + right];
                    COLORREF txt0 = propElements->propData->bkgnd;
                    COLORREF bkg0 = invertColor(txt0);
                    renderDragSym(dc, str.substr(left, right-left+1), side, txt0, bkg0);
                }
                if (right != (end_sym - 1))
                {
                    RECT side = pos; side.left = ld[start_sym + right];
                    renderDragSym(dc, str.substr(right+1), side, text_color, bkg_color);
                }
                start_sym = end_sym;
                continue;
            }
        }

        if (!dragging && index == m_find_string_index)
        {
            int left = m_find_start_pos;
            int right = m_find_end_pos;
            int end_sym = start_sym + str.size();

            DragParamsChecker dpc(left, right, start_sym, end_sym);
            if (dpc.check())
            {
                std::vector<int> ld;
                calcDragArray(s, ld);

                if (left != 0)
                {
                    RECT side = pos; side.right = ld[start_sym + left - 1];
                    renderDragSym(dc, str.substr(0, left), side, text_color, bkg_color);
                }
                // selected
                {
                    RECT side = pos; if (left != 0) side.left = ld[start_sym + left - 1];
                    if (right != (end_sym - 1)) side.right = ld[start_sym + right];
                    COLORREF txt0 = propElements->propData->bkgnd;
                    COLORREF bkg0 = invertColor(txt0);
                    renderDragSym(dc, str.substr(left, right - left + 1), side, txt0, bkg0);
                }
                if (right != (end_sym - 1))
                {
                    RECT side = pos; side.left = ld[start_sym + right];
                    renderDragSym(dc, str.substr(right + 1), side, text_color, bkg_color);
                }
                start_sym = end_sym;
                continue;
           }
        }

        dc->FillSolidRect(&pos, bkg_color);
        dc->SetBkColor(bkg_color);
        dc->SetTextColor(text_color);
        dc->DrawText(str.c_str(), -1, &pos, DT_CENTER|DT_SINGLELINE|DT_VCENTER);

        if (p.underline_status || p.italic_status)
            dc->SelectFont(propElements->standard_font);

        if (p.blink_status && !dragging)
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
        start_sym += str.length();
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
    stopSoftScroll();
    updateScrollbar(visible_line);
    Invalidate();
}

void MudView::mouseWheel(WORD position)
{
    int direction = (position & 0x8000) ? 3 : -3;
    int visible_line = m_last_visible_line + direction;
    stopSoftScroll();
    updateScrollbar(visible_line);
    Invalidate();
}

void MudView::checkLimit()
{
    int size = m_strings.size();
    if (size > propElements->propData->view_history_size)
    {
        size = size - propElements->propData->view_history_size;
        deleteBeginStrings(size);
    }
}

void MudView::deleteBeginStrings(int count_from_begin)
{
    mudViewStrings::iterator it = m_strings.begin(), it_end = m_strings.begin()+count_from_begin;
    for (; it != it_end; ++it)
        delete (*it);
    m_strings.erase(m_strings.begin(), m_strings.begin()+count_from_begin);
    if (drag_begin < 0)
        return;
    drag_begin -= count_from_begin;
    drag_end -= count_from_begin;
    if (drag_begin < 0) drag_begin = 0;
    if (drag_end < 0) drag_end = 0;
    if (drag_begin == drag_end && drag_begin == 0)
    {
        ReleaseCapture();
        drag_begin = -1;
    }
}

void MudView::startDraging()
{
    m_dragpt = getCursor();
    int line = getCursorLine(m_dragpt.y);
    if (line < 0) return;
    SetCapture();
    drag_begin = line;
    drag_end = line;
    calcDragLine(line, BEGINLINE);
    drag_left = calcDragSym(m_dragpt.x, BEGINLINE);
    drag_right = drag_left;
    Invalidate(FALSE);
}

void MudView::stopDraging()
{
    if (drag_begin < 0)
        return;
    ReleaseCapture();

    if (drag_end == -1 && drag_begin >= 0) { drag_end = 0; drag_right = 0; }
    if (drag_begin > drag_end) { int t=drag_begin; drag_begin=drag_end; drag_end=t;
    t = drag_left; drag_left = drag_right; drag_right = t; }

    tstring text, tmp;
    if (drag_begin == drag_end)
    {
        m_strings[drag_begin]->getText(&tmp);
        if (drag_left == -1)
        {
            if (drag_right != -1)
                text.assign ( tmp.substr(drag_right) );
            else
                text.assign(tmp);
        }
        else if (drag_right == -1)
        {
            if (isDragCursorLeft())
                text.assign( tmp.substr(0, drag_left+1) );
            else
                text.assign( tmp.substr(drag_left) );
        }
        else
        {
            if (drag_left > drag_right) { int t = drag_left; drag_left = drag_right; drag_right = t; }
            text.assign ( tmp.substr(drag_left, drag_right - drag_left + 1) );
        }
    }
    else
    {
        tstring eol(L"\r\n");

        // begin line
        m_strings[drag_begin]->getText(&tmp);
        if (drag_begin < drag_end)
        {
            int left = drag_left;
            if (left == -1) { text.append(tmp); }
            else { 
                tstring sp(left, L' ');
                text.append(sp);
                text.append(tmp.substr(left)); 
            }
        }
        else
        {
            int left = drag_left;
            if (left == -1) { text.append(tmp); }
            else { text.append(tmp.substr(left)); }
        }
        text.append(eol);

        // middle lines
        for (int i = drag_begin+1; i < drag_end; ++i)
        {
            m_strings[i]->getText(&tmp);
            text.append(tmp);
            text.append(eol);
        }

        // endline
        m_strings[drag_end]->getText(&tmp);
        if (drag_begin < drag_end)
        {
            int right = drag_right;
            if (right == -1) { text.append(tmp); }
            else { text.append(tmp.substr(0, right+1)); }
        }
        else
        {
            int right = drag_right;
            if (right == -1) { text.append(tmp); }
            else { text.append(tmp.substr(right+1)); }
        }
    }

    sendToClipboard(m_hWnd, text);
    drag_begin = -1;
    Invalidate(FALSE);
}

void MudView::doDraging()
{
    if (drag_begin < 0)
        return;
    POINT pt = getCursor();
    m_dragpos = pt;
    int new_drag_end = getCursorLine(pt.y);
    if (drag_begin != new_drag_end)
    {
        if (new_drag_end > m_last_visible_line)
            new_drag_end = m_last_visible_line;
        int min_line = m_last_visible_line - m_lines_count;
        if (new_drag_end < min_line)
            new_drag_end = min_line;
        if (new_drag_end != drag_end)
        {
            drag_end = new_drag_end;
            calcDragLine(drag_end, ENDLINE);
        }
        drag_right = calcDragSym(pt.x, ENDLINE);
    }
    else
    {
        drag_end = new_drag_end;
        drag_right = calcDragSym(pt.x, BEGINLINE);
    }
    Invalidate(FALSE);
}

bool MudView::checkDragging(int line, bool incborder)
{
    if (drag_begin < 0)
        return false;
    if (drag_end < drag_begin)
    {
        if (incborder) {
        if (line >= drag_end && line <= drag_begin)
            return true;
        }
        else {
        if (line > drag_end && line < drag_begin)
            return true;
        }
    }
    else
    {
        if (incborder) {
        if (line >= drag_begin && line <= drag_end)
            return true;
        }
        else {
        if (line > drag_begin && line < drag_end)
            return true;
        }
    }
    return false;
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

bool MudView::isDragCursorLeft() const
{
    int dx = m_dragpos.x - m_dragpt.x;
    return (dx < 0) ? true : false;
}

int MudView::calcDragSym(int x, dragline type) const
{
    const std::vector<int> &ld = (type == BEGINLINE) ? m_drag_beginline_len : m_drag_endline_len;
    int left = 0;
    for (int i = 0, e = ld.size(); i < e; ++i)
    {
        int right = ld[i];
        if (x >= left && x < right) { return i; }
        left = right;
    }
    return -1;
}

void MudView::calcDragLine(int line, dragline type)
{
    if (line < 0) return;
    std::vector<int> &ld = (type == BEGINLINE) ? m_drag_beginline_len : m_drag_endline_len;
    MudViewString *s = m_strings[line];
    calcDragArray(s, ld);
}

void MudView::calcDragArray(MudViewString* s, std::vector<int> &ld)
{
    int blocks = s->blocks.size();
    if (!blocks) { ld.clear(); return; }
    int dc_size = 0;
    for (int i = 0; i < blocks; ++i)
    {
        const MudViewStringBlock &b = s->blocks[i];
        dc_size += b.size.cx;
    }
    tstring text;
    s->getText(&text);
    CDC dc(GetDC());
    HFONT current_font = dc.SelectFont(propElements->standard_font);
    int maxchars = text.size();
    ld.resize(maxchars);
    SIZE sz = { 0 };
    if (maxchars > 0)
        GetTextExtentExPoint(dc, text.c_str(), text.length(), dc_size, &maxchars, &ld[0], &sz);
    dc.SelectFont(current_font);
}

void MudView::stopSoftScroll()
{
    m_start_softscroll = -1;
}

bool MudView::inSoftScrolling() const
{
    return (m_start_softscroll == -1) ? false : true;
}

bool MudView::isDragMode() const
{
    return (drag_begin == -1) ? false : true;
}

int MudView::findAndSelectText(int from, int direction, const tstring& text)
{
    if (text.empty())
        return -1;
    if (direction != -1 && direction != 1)
        return -1;
    if (from == -1 && direction > 0)
        from = 0;
    if (from < 0)
        return -1;
    int count = m_strings.size();
    if (from >= count)
        return -1;

    int start = m_find_start_pos;
    int i = from;
    int end = (direction==1) ? count : -1;
    while (i != end)
    {
        tstring str;
        m_strings[i]->getText(&str);
        size_t pos = -1;
        if (direction == 1)
        {
           size_t start_pos = (start==-1) ? 0 : start+text.length();
           pos = str.find(text, start_pos);
        }
        else
        {
           tstring p( (start>=0) ? str.substr(0, start) : str);
           pos = p.rfind(text);
        }
        if (pos != -1)
        {
            m_find_string_index = i;
            m_find_start_pos = pos;
            m_find_end_pos = pos + text.size() - 1;
            return i;
        }
        else
        {
            start = -1;
        }
        i = i + direction;
    }
    return -1;
}

int MudView::getCurrentFindString()
{
    return m_find_string_index;
}

void MudView::clearFind()
{
    m_find_string_index = m_find_start_pos = m_find_end_pos = -1;
    Invalidate(FALSE);
}
