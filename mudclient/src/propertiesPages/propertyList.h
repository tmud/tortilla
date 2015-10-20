#pragma once

struct PropertyListItemData
{
    int item;
    int subitem;
    COLORREF text;
    COLORREF bkg;
};

class PropertyListCtrlHandler
{
public:
    virtual bool drawPropertyListItem(PropertyListItemData *pData) { return false; }
};

class PropertyListCtrl : public CWindowImpl<PropertyListCtrl, CListViewCtrl>
{
    float m_width_percent;
    PropertyListCtrlHandler *m_pHandler;
    bool m_doubleclick;
    wchar_t m_temp_buffer[256];

public:
    DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())
    PropertyListCtrl() : m_width_percent(0.0f), m_pHandler(NULL), m_doubleclick(false) {}

    void Attach(HWND hWnd) 
    {
        SubclassWindow(hWnd);
        RECT pos;
        GetClientRect(&pos);
        float width = static_cast<float>(pos.right - (GetSystemMetrics(SM_CXVSCROLL)+2) );
        m_width_percent = width / 100.0f;
    }

    void addColumn(const tstring& text, int width_percents)
    {
        float width = m_width_percent * width_percents;
        int count = GetHeader().GetItemCount();
        InsertColumn(count, text.c_str(), LVCFMT_LEFT, static_cast<int>(width));
    }

    void setHandler(PropertyListCtrlHandler* handler)
    {
        m_pHandler = handler;
    }

    void addItem(int item, int subitem, const tstring& text)
    {
        AddItem(item, subitem, text.c_str());
    }

    void setItem(int item, int subitem, const tstring& text)
    {
        SetItem(item, subitem, LVIF_TEXT, text.c_str(),  0, 0, 0, NULL);
    }

    void supportDoubleClick()
    {
        m_doubleclick = true;
    }

    bool isSelected(int index) const
    {
        int selected = GetSelectedCount();
        if (selected == 0)
            return false;
        int item = -1;
        for (int i = 0; i < selected; ++i)
        {
            item = GetNextItem(item, LVNI_SELECTED);
            if (item == index) return true;
        }
        return false;
    }

    void getSelected(std::vector<int>* selected) const
    {
        selected->clear();
        int item = -1;
        for (int i = 0, items = GetSelectedCount(); i < items; ++i)
        {
            item = GetNextItem(item, LVNI_SELECTED);
            selected->push_back(item);
        }
        std::sort(selected->rbegin(), selected->rend());
    }

    int getOnlySingleSelection() const
    {
        if (GetSelectedCount() != 1)
            return -1;
        return GetNextItem(-1, LVNI_SELECTED);
    }

    void getItemText(int item, int subitem, tstring *text)
    {
        tchar *buffer = NULL;
        for (int nLen = 256;; nLen *= 2)
        {
            buffer = new tchar[nLen];
            int res = GetItemText(item, subitem, buffer, nLen - 1);
            if (res < nLen - 1)
                break;
            delete[]buffer;
        }
        text->assign(buffer);
        delete[]buffer;
    }

    int getTopItem() const
    {
        return GetTopIndex();
    }

    void setTopItem(int index)
    {
        EnsureVisible (index, FALSE);
        RECT rect;
        if (GetItemRect (index, &rect, LVIR_LABEL))
        {
            CSize size;
            size.cx = 0;
            size.cy = rect.bottom - rect.top;
            size.cy *= index - GetTopIndex();
            if (index != GetTopIndex())
                Scroll(size);
        }
    }

private:
    BEGIN_MSG_MAP(PropertyListCtrl)
       NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
       REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
    END_MSG_MAP()

    LRESULT OnCustomDraw(int, LPNMHDR pnmh, BOOL&)
    {
        LRESULT result = CDRF_DODEFAULT;
        NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pnmh );
        DWORD stage = pLVCD->nmcd.dwDrawStage;

        // First thing - check the draw stage. If it's the control's prepaint
        // stage, then tell Windows we want messages for every item.
        if ( CDDS_PREPAINT == stage )
        {
            result = CDRF_NOTIFYITEMDRAW;
        }
        else if ( CDDS_ITEMPREPAINT == stage )
        {
            // This is the notification message for an item.  We'll request
            // notifications before each subitem's prepaint stage.
            result = CDRF_NOTIFYSUBITEMDRAW;
        }
        else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM)  == stage )
        {
            // This is the prepaint stage for a subitem. Here's where we set the
            // item's text and background colors. Our return value will tell 
            // Windows to draw the subitem itself, but it will use the new colors
            // we set here.
            PropertyListItemData pdata;
            pdata.item = pLVCD->nmcd.dwItemSpec;
            pdata.subitem = pLVCD->iSubItem;
            bool selected = isSelected(pdata.item);

            // Used instead of GetSubItemRect(), which returns the entire
            // row-rect for label-column (nCol==0)
            RECT pos;
            GetItemRect(pdata.item, &pos, LVIR_BOUNDS);
            RECT header;
            GetHeader().GetItemRect(pdata.subitem, &header);
            int hscroll = GetScrollPos(SB_HORZ);
            pos.left = header.left - hscroll + 1;
            pos.right = header.right - hscroll - 1;
            pos.top += 1; pos.bottom -= 1;

            if (pdata.subitem == 0) pos.left += 2;
            COLORREF crBackground = GetSysColor (COLOR_HIGHLIGHT);
            COLORREF crText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            if (!selected)
            {
                crBackground = GetBkColor();
                crText = GetTextColor();
            }

            if (m_pHandler && m_pHandler->drawPropertyListItem(&pdata))
            {
                crBackground = pdata.bkg;
                crText = pdata.text;
            }
            CDC dc(GetDC());

            CBrush bkg; bkg.CreateSolidBrush(crBackground);
            FillRect(dc, &pos,  bkg);

            COLORREF old_bk = dc.SetBkColor(crBackground);
            COLORREF old_text = dc.SetTextColor(crText);

            HFONT old_font = dc.GetCurrentFont();
            dc.SelectFont(GetFont());

            GetItemText(pdata.item, pdata.subitem, m_temp_buffer, 255 );
            pos.left += 4;
            DrawText(dc, m_temp_buffer, -1, &pos, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_EXTERNALLEADING);

            dc.SelectFont(old_font);
            dc.SetBkColor(old_bk);
            dc.SetTextColor(old_text);
            result = CDRF_SKIPDEFAULT;
        }
        return result;
    }
};
