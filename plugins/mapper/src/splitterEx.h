#pragma once

#include <atlsplit.h>

template <bool t_bVertical = true, int divider1 = 1, int divider2 = 2>
class CSplitterWindowExT : public CSplitterWindowImpl<CSplitterWindowExT<t_bVertical>, t_bVertical>
{
public:
	DECLARE_WND_CLASS_EX(_T("WTL_SplitterWindowEx"), CS_DBLCLKS, COLOR_WINDOW)
    //typedef CSplitterWindowImpl<CSplitterWindowExT<t_bVertical>, t_bVertical> _baseClass;

    BEGIN_MSG_MAP(CSplitterWindowExT)        
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
        CHAIN_MSG_MAP(_baseClass)
        FORWARD_NOTIFICATIONS()
    END_MSG_MAP()

    void SetDefaultSplitterPos()
    {
        RECT rcWindow;
		GetClientRect(&rcWindow);
        int pos = (t_bVertical) ? (rcWindow.right - rcWindow.left) : (rcWindow.bottom - rcWindow.top);
        pos = (pos * divider1) / (divider1 + divider2);
        SetSplitterPos(pos);
    }

private:
    LRESULT OnLButtonDoubleClick(UINT, WPARAM, LPARAM, BOOL&)    
	{		
		SetDefaultSplitterPos();
        return 0;
	}
};
