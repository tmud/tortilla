#pragma once
#include "resource.h"

void tokenize(const std::wstring& s, const std::wstring& tokens, std::vector<std::wstring>& parts)
{	
	const wchar_t *p = s.c_str();
	const wchar_t *e = p + s.length();
	while (p < e)
	{
		size_t len = _tcscspn(p, tokens.c_str());
		parts.push_back(std::wstring(p, len));
		p = p + len + 1;
	}
}

class CEditEx : public CWindowImpl<CEditEx, CEdit>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())
private:
	BEGIN_MSG_MAP(CEditEx)
	  MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()
	LRESULT OnKeyDown(UINT, WPARAM wparam, LPARAM, BOOL&bHandled)
	{
		if (wparam == VK_ESCAPE || wparam == VK_RETURN) {
			::PostMessage(GetParent(), WM_CLOSE, 0, 0);
		}
		bHandled = FALSE;
		return 0;
	}
};

class AlertDlgControl : public CDialogImpl<AlertDlgControl>
{
	 std::wstring m_text;
	 CEditEx m_edit;     
	 void close() { ::SendMessage(GetParent(), WM_CLOSE, 0, 0); }
public:
    enum { IDD = IDD_ALERT };
	void setText(const std::wstring& text) {
		m_text = text;
		if (IsWindow())
			settext();
	}
private:
	void settext() 
	{
		std::vector<std::wstring> v;
		tokenize(m_text, L"\n", v);
				
		/*HFONT font = m_edit.GetFont();
		RECT rc;
		m_edit.GetClientRect(&rc);
		int max_width = rc.right;

		std::wstring result;

		CDC dc(GetDC());
		HFONT oldfont = dc.SelectFont(font);
		for (int i = 0, e = v.size(); i < e; ++i)
		{
			SIZE size = { 0, 0 };
			const std::wstring& t = v[i];
			if (i != 0)
				result.append(L"\r\n");			
			GetTextExtentPoint32(dc, t.c_str(), t.length(), &size);
			if (size.cx > max_width) {
				int x = 1;
			}
			result.append(t);
		}
		dc.SelectFont(oldfont);
		m_edit.SetWindowText(result.c_str());*/

		std::wstring result;
		for (int i = 0, e = v.size(); i < e; ++i)
		{
			const std::wstring& t = v[i];
			if (i != 0)
				result.append(L"\r\n");
			result.append(t);
		}
		m_edit.SetWindowText(result.c_str());
	}

   BEGIN_MSG_MAP(AlertDlgControl)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(IDOK, OnClick)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
   {
		CStatic editpos(GetDlgItem(IDC_STATIC_EDIT));
		RECT rc0, rc;
		GetWindowRect(&rc0);
		editpos.GetWindowRect(&rc);
		LONG w = rc.right - rc.left;
		LONG h = rc.bottom - rc.top;
		rc.left = rc.left - rc0.left;
		rc.top = rc.top - rc0.top;
		rc.right = rc.left + w;
		rc.bottom = rc.top + h;
		editpos.ShowWindow(SW_HIDE);
		m_edit.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			WS_VSCROLL | ES_READONLY | ES_AUTOVSCROLL | ES_MULTILINE, WS_EX_STATICEDGE);
		HFONT font = GetFont();
		m_edit.SetFont(font);			
		settext();
        return 0;
   }
   LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
   {
	   close();
	   return 0;
   }
   LRESULT OnClick(WORD, WORD, HWND, BOOL&)
   {
	   close();
	   return 0;
   }
};

class AlertDlg :  public CWindowImpl<AlertDlg>
{
    AlertDlgControl m_dlg;
	void close() {
		DestroyWindow();
	}
private:
   BEGIN_MSG_MAP(AlertDlg)
	   MESSAGE_HANDLER(WM_CREATE, OnCreate)
	    MESSAGE_HANDLER(WM_CLOSE, OnClose)        
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
   END_MSG_MAP()

   void setText(const std::wstring& text)
   {
	   m_dlg.setText(text);
   }

   LRESULT OnKeyDown(UINT, WPARAM wparam, LPARAM, BOOL&)
   {
	   if (wparam == VK_ESCAPE || wparam == VK_RETURN) {
		   close();
	   }
	   return 0;
   }

   LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
   {
	   close();
	   return 0;
   }

   LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
   {
       m_dlg.Create(m_hWnd);
       RECT rc;
       m_dlg.GetClientRect(&rc);
       int dy = GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYBORDER);
       int dx = 2 * GetSystemMetrics(SM_CXBORDER);
       rc.right += dx;
       rc.bottom += dy;
       MoveWindow(&rc);
	   HICON icon = AtlLoadIcon(IDI_WARNINGMY);
	   SetIcon(icon, FALSE);
	   SetIcon(icon, TRUE);
       return 0;
   }
};
