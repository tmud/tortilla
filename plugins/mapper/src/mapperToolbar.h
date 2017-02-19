#pragma once

class MapperToolbar : public CDialogImpl<MapperToolbar>, public CMessageFilter
{
public:
    enum { IDD = IDD_MAPPER_TOOLBAR };    
    BOOL PreTranslateMessage(MSG* pMsg) {
        return CWindow::IsDialogMessage(pMsg); 
    }

	BEGIN_MSG_MAP(MapperToolbar)
		//MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{   
        bHandled = FALSE;
        return 0;
	}

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
        bHandled = FALSE;
        return 0;
    }
};

class ToolbarViewContainer : public CWindowImpl<ToolbarViewContainer>
{
    int m_size;
    CWindow m_first, m_second;
public:
    ToolbarViewContainer() : m_size(0) {}
    void attach(int size, HWND first, HWND second) {
        m_size = size;
        m_first.Attach(first);
        m_second.Attach(second);
        Invalidate();
    }
private:
    BEGIN_MSG_MAP(ToolbarViewContainer)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()
    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return 1; }
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&){ onSize();  return 0; }
    void onSize() {
        RECT rc; GetClientRect(&rc);
        int bottom = rc.bottom;
        rc.bottom = m_size;
        if (m_first.IsWindow())
            m_first.MoveWindow(&rc);
        rc.top = rc.bottom;
        rc.bottom = bottom;
        if (m_second.IsWindow())
           m_second.MoveWindow(&rc);
    }
};
