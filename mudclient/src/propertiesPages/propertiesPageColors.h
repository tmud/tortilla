#pragma once

class ColorExampleWindow :  public CWindowImpl<ColorExampleWindow, CWindow>
{
    PropertiesData *propData;
    LOGFONT m_logfont;
    CBrush m_backgroundBrush;
    CFont  m_font;
    CPen   m_cursorPen;
    int    m_selectedItem;
    bool   m_mouseleave;

    struct label
    {
        RECT pos;
        tstring text;
    };
    std::vector<label> m_labels;

public:
    ColorExampleWindow(PropertiesData *data) : propData(data), m_backgroundBrush(NULL), m_selectedItem(-1), m_mouseleave(false) {}
    DECLARE_WND_CLASS(NULL)
    void onBackroundColor()
    {
        COLORREF color = propData->bkgnd;
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();
            propData->bkgnd = color;
            initBackground();
            Invalidate();
        }
    }

    void onResetColors()
    {
        if (msgBox(GetParent(), IDS_RESET_COLORS_CONFIRM, MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            propData->initDefaultColorsAndFont();
            initBackground();
            initFont();
            initBase();
            Invalidate();
        }
    }

    void onResetOscColors()
    {
        if (msgBox(GetParent(), IDS_RESET_OSC_CONFIRM, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            propData->resetOSCColors();
            Invalidate();
        }
    }

    void onSelectFont(bool any_fonts)
    {
        LOGFONT lf = m_logfont;
        DWORD dwFlags = CF_SCREENFONTS;
        if (!any_fonts) dwFlags|= CF_FIXEDPITCHONLY;
        CFontDialog dlg(&lf, dwFlags, NULL, GetParent());
        if (dlg.DoModal() == IDOK)
        {
            propData->font_name.assign(lf.lfFaceName);
            propData->font_heigth = MulDiv(-lf.lfHeight, 72, GetDeviceCaps(GetDC(), LOGPIXELSY));
            propData->font_bold = lf.lfWeight;
            propData->font_italic = lf.lfItalic ? 1 : 0;
            initFont();
            initBase();
            Invalidate();
        }
    }

private:
    BEGIN_MSG_MAP(ColorExampleWindow)
       MESSAGE_HANDLER(WM_CREATE, OnCreate)
       MESSAGE_HANDLER(WM_PAINT, OnPaint)
       MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBknd)
       MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
       MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
       MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseClick)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        initFont();
        initBackground();
        initBase();
        return 0;
    }

    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT pos;
        GetClientRect(&pos);
        CPaintDC dc(m_hWnd);
        CMemoryDC mdc(dc, pos);
        mdc.FillRect(&pos, m_backgroundBrush);
        HFONT oldfont=mdc.SelectFont(m_font);

        mdc.SetBkColor(propData->bkgnd);
        for (int i=0; i<16; ++i)
        {
            mdc.SetTextColor(getColor(i));
            mdc.DrawText(m_labels[i].text.c_str(), m_labels[i].text.length(), &m_labels[i].pos, 
                DT_CENTER|DT_SINGLELINE|DT_VCENTER);
        }
        mdc.SelectFont(oldfont);
        int cursor = m_selectedItem;
        if (cursor != -1)
        {
            const RECT &p = m_labels[cursor].pos;
            HPEN oldpen = mdc.SelectPen(m_cursorPen);
            MoveToEx(mdc, p.left, p.top, NULL);
            LineTo(mdc, p.right-1, p.top);
            LineTo(mdc, p.right-1, p.bottom-1);
            LineTo(mdc, p.left, p.bottom-1);
            LineTo(mdc, p.left, p.top);
            mdc.SelectPen(oldpen);
        }
        return 0;
    }

    LRESULT OnEraseBknd(UINT, WPARAM, LPARAM, BOOL&)
    {
        return 1;
    }

    LRESULT OnMouseMove(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        if (!m_mouseleave)
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);
            m_mouseleave = true;
        }

        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        int index = -1;
        for (int i=0; i<16; ++i)
        {
            if (PtInRect(&m_labels[i].pos, pt))
                { index = i; break; }
        }
        m_selectedItem = index;
        Invalidate();
        return 0;
    }

    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        m_mouseleave = false;
        m_selectedItem = -1;
        Invalidate();
        return 0;
    }

    LRESULT OnMouseClick(UINT, WPARAM, LPARAM lparam, BOOL&)
    {
        if (m_selectedItem == -1)
            return 0;

        int item = m_selectedItem;
        COLORREF color = getColor(item);
        CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            color = dlg.GetColor();
            propData->colors[item] = color;
            propData->osc_flags[item] = 0;
            ::PostMessage(GetParent(), WM_USER, 0, 0);
            Invalidate();
        }
        return 0;
    }

private:
    COLORREF getColor(int item)
    {
        return propData->osc_flags[item] ? propData->osc_colors[item] : propData->colors[item];
    }

    void initBackground()
    {
        COLORREF bkgnd = propData->bkgnd;
        if (!m_backgroundBrush.IsNull())
            m_backgroundBrush.DeleteObject();
        m_backgroundBrush = CreateSolidBrush(bkgnd);

        COLORREF cursor = RGB( 255-GetRValue(bkgnd), 255-GetGValue(bkgnd), 255-GetBValue(bkgnd) );
        if (!m_cursorPen.IsNull())
            m_cursorPen.DeleteObject();
        m_cursorPen.CreatePen(PS_SOLID, 1, cursor);
    }

    void initFont()
    {
        propData->initLogFont(m_hWnd, &m_logfont);
        if (!m_font.IsNull())
            m_font.DeleteObject();
        m_font.CreateFontIndirect(&m_logfont);
    }

    void initBase()
    {
        m_labels.clear();

        WCHAR* labels_name[16] = {
            L"Черный", L"Красный", L"Зеленый", L"Коричневый", L"Синий", L"Фиолетовый", L"Голубой", L"Серый",
            L"Угольный", L"Ярко Красный", L"Ярко Зеленый", L"Желтый", L"Ярко синий", L"Розовый", L"Ярко голубой", L"Белый"
        };
        CDC dc(GetDC());
        HFONT oldfont = dc.SelectFont(m_font);
        for (int i=0; i<16; ++i)
        {
            label new_label;
            new_label.text.assign(labels_name[i]);
            SIZE sz = {0, 0};
            GetTextExtentPoint32(dc, labels_name[i], _tcslen(labels_name[i]), &sz);
            RECT pos = { 0, 0, sz.cx, sz.cy };
            new_label.pos = pos;
            m_labels.push_back(new_label);
        }
        dc.SelectFont(oldfont);

        // calc elements position
        int text_label_heigth = m_labels[0].pos.bottom;
        int delimeter = text_label_heigth / 2;

        int text_label_width = 0;
        for (int i=0; i<16; ++i)
        {
            int width = m_labels[i].pos.right;
            if (width > text_label_width)
                text_label_width = width;
        }
        text_label_width += delimeter*2;

        RECT pos; GetClientRect(&pos);
        int free_width = pos.right - (text_label_width * 2);
        if (free_width < 0)
            free_width = 0;
        int free_heigth = pos.bottom - ((8 * text_label_heigth) + (7 * delimeter));
        if (free_heigth < 0)
            free_heigth = 0;

        pos.left = free_width / 3;
        pos.top = free_heigth / 2;
        pos.right = pos.left + text_label_width;
        pos.bottom = pos.top + text_label_heigth;

        for (int i=0; i<16; ++i)
        {
            if (i == 8)
            { 
                pos.left = pos.right + free_width / 3;
                pos.right = pos.left + text_label_width;
                pos.top = free_heigth / 2;
                pos.bottom = pos.top + text_label_heigth;
            }
            m_labels[i].pos = pos;
            pos.top = pos.bottom + delimeter;
            pos.bottom = pos.top + text_label_heigth;
        }
    }
};

class PropertyColors :  public CDialogImpl<PropertyColors>
{
    PropertiesData *propData;
    ColorExampleWindow m_colors;
    CBevelLine m_bl1, m_bl2;
    CButton m_reset_osc;
    CButton m_any_font;
public:
    enum { IDD = IDD_PROPERTY_COLORS_FONT };
    PropertyColors(PropertiesData *data) : propData(data), m_colors(data) {}

private:
    BEGIN_MSG_MAP(PropertyColors)
       MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
       COMMAND_ID_HANDLER(IDC_BUTTON_BKGND_COLOR, OnBackgroundColor)
       COMMAND_ID_HANDLER(IDC_BUTTON_COLOR_RESET, OnResetColors)
       COMMAND_ID_HANDLER(IDC_BUTTON_FONT, OnSelectFont)
       COMMAND_ID_HANDLER(IDC_BUTTON_COLOR_RESETOSC, OnResetOscColors)
       COMMAND_ID_HANDLER(IDC_CHECK_ANYFONTS, OnAnyFont)
       MESSAGE_HANDLER(WM_USER, OnChangedOscColor)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_reset_osc.Attach(GetDlgItem(IDC_BUTTON_COLOR_RESETOSC));
        m_any_font.Attach(GetDlgItem(IDC_CHECK_ANYFONTS));

        RECT pos;
        CStatic colorspos(GetDlgItem(IDC_STATIC_COLORS_WINDOW));
        colorspos.GetWindowRect(&pos);
        ScreenToClient(&pos);
        colorspos.ShowWindow(SW_HIDE);
        m_colors.Create(m_hWnd, pos, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

        m_any_font.SetCheck(propData->any_font ? BST_CHECKED : BST_UNCHECKED);

        setResetOscButtonState();
        return 0;
    }

    void setResetOscButtonState()
    {
        BOOL osc_enable = FALSE;
        for (int i=0; i<16; ++i) {
            if (propData->osc_flags[i]) { osc_enable = TRUE; break; }
        }
        m_reset_osc.EnableWindow(osc_enable);
    }

    LRESULT OnChangedOscColor(UINT, WPARAM, LPARAM, BOOL&)
    {
        setResetOscButtonState();
        return 0;
    }

    LRESULT OnBackgroundColor(WORD, WORD, HWND, BOOL&)
    {
        m_colors.onBackroundColor();
        return 0;
    }

    LRESULT OnResetColors(WORD, WORD, HWND, BOOL&)
    {
        propData->any_font = 0;
        m_any_font.SetCheck(BST_UNCHECKED);
        m_colors.onResetColors();
        return 0;
    }

    LRESULT OnResetOscColors(WORD, WORD, HWND, BOOL&)
    {
        m_colors.onResetOscColors();
        return 0;
    }

    LRESULT OnSelectFont(WORD, WORD, HWND, BOOL&)
    {
        bool any_font = (propData->any_font == 1) ? true : false;
        m_colors.onSelectFont(any_font);
        return 0;
    }

    LRESULT OnAnyFont(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_any_font.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->any_font = state;
        return 0;
    }

};
