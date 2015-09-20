#pragma once
#include "splitterEx.h"

class ImageCollection
{
public:
    void scanImages();

private:
    std::vector<tstring> m_files;
};

class SelectImageCategory : public CWindowImpl<SelectImageCategory>
{
public:
private:
    BEGIN_MSG_MAP(SelectImageCategory)
        //MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()
};

class SelectImage : public CWindowImpl<SelectImage>
{
private:
    BEGIN_MSG_MAP(SelectImage)
        //MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()
};

class SelectImageDlg : public CWindowImpl<SelectImageDlg>
{
    CSplitterWindowExT<true, 1, 4> m_vSplitter;
    //SelectImageCategory m_category;
    CListBox m_category;
    SelectImage m_atlas;
    ImageCollection m_images;
public:
    SelectImageDlg() {}

private:
    BEGIN_MSG_MAP(SelectImageDlg)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      //MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) 
    {
        RECT rc;
        GetClientRect(&rc);
        m_vSplitter.Create(m_hWnd, rc);
        m_vSplitter.m_cxySplitBar = 3;
        m_vSplitter.SetSplitterRect();
        m_vSplitter.SetDefaultSplitterPos();

        RECT pane_left, pane_right;
        m_vSplitter.GetSplitterPaneRect(0, &pane_left); 
        pane_left.right -= 3;
        m_vSplitter.GetSplitterPaneRect(1, &pane_right);

        DWORD style = WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE;
        m_category.Create(m_vSplitter, pane_left, NULL, style, WS_EX_CLIENTEDGE);
        m_atlas.Create(m_vSplitter, pane_right, NULL, style); // | WS_VSCROLL | WS_HSCROLL);
        m_vSplitter.SetSplitterPanes(m_category, m_atlas);

        m_images.scanImages();
        return 0;
    }

    LRESULT OnShowWindow(UINT, WPARAM, LPARAM, BOOL&)
    {

        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rc;
        GetClientRect(&rc);
        m_vSplitter.MoveWindow(&rc, FALSE);
        m_vSplitter.SetSplitterRect();
        return 0;
    }
};
