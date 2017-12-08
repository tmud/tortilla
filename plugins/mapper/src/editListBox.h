#pragma once

/////////////////////////////////////////////////////////////////////////////
// CEditListBox - Editable List box
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// Modified by me :)

#ifndef __cplusplus
   #error WTL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLCTRLS_H__
   #error EditListBox.h requires atlctrls.h to be included first
#endif

#ifndef __ATLCTRLX_H__
   #error EditListBox.h requires atlctrlx.h to be included first
#endif

// EditListBox Notification messages
#define EDLN_FIRST           (0U-1900U)
#define EDLN_LAST            (0U-1950U)

#define EDLN_BROWSE          (EDLN_FIRST-1)
#define EDLN_ITEMCHANGING    (EDLN_FIRST-2)
#define EDLN_ITEMCHANGED     (EDLN_FIRST-3)
#define EDLN_BEGINLABELEDIT  (EDLN_FIRST-4)
#define EDLN_ENDLABELEDIT    (EDLN_FIRST-5)

typedef struct 
{
    NMHDR hdr;
    int iIndex;
} NMEDITLIST, *PNMEDITLIST;

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CEditListBoxImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
   CListBox m_wndList;
   CContainedWindowT<CEdit> m_wndEdit;
   typedef CEditListBoxImpl< T, TBase, TWinTraits > thisClass;

   BEGIN_MSG_MAP(thisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRightClick)
      COMMAND_CODE_HANDLER(LBN_DBLCLK, OnDblClick)
      COMMAND_CODE_HANDLER(LBN_SELCHANGE, OnChange)
   ALT_MSG_MAP(1) // Edit control
      MESSAGE_HANDLER(WM_GETDLGCODE, OnEditGetDlgCode)
      MESSAGE_HANDLER(WM_CHAR, OnEditKey)
      MESSAGE_HANDLER(WM_KILLFOCUS, OnEditLostFocus)
   END_MSG_MAP()

   CEditListBoxImpl() : m_wndEdit(this, 1)
   {
   }

   ~CEditListBoxImpl()
   {
   }

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd==NULL);
      ATLASSERT(::IsWindow(hWnd));
      BOOL bRet = CWindowImpl< CEditListBox, CWindow, CControlWinTraits >::SubclassWindow(hWnd);
      if(bRet) _Init();
      return bRet;
   }

   int GetCurSel() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return m_wndList.GetCurSel();      
   }

   void SetMaxText(int iMax)
   {
       m_wndEdit.LimitText(iMax);
   }

   BOOL AddItem(LPCTSTR pstrText)
   {
       return InsertItem(-1, pstrText);
   }

   BOOL InsertItem(int iIndex, LPCTSTR pstrText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(!::IsBadStringPtr(pstrText,-1));
      
      if (iIndex > GetItemCount())
         return FALSE;

      if (iIndex == -1) 
          iIndex = GetItemCount();

      int idx = m_wndList.InsertString(iIndex, pstrText);
      if( idx==0 ) 
          m_wndList.SetCurSel(0);
      return (idx >= 0);
   }

   BOOL SelectItem(int iIndex)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return m_wndList.SetCurSel(iIndex) != LB_ERR;
   }

   int FindItem(LPCTSTR pstrText) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(!::IsBadStringPtr(pstrText,-1));
      return m_wndList.FindString(-1, pstrText);
   }

   /*BOOL BrowseSelected()
   {
      if( (GetStyle() & EDLS_BROWSE)==0 ) return FALSE;
      if( m_wndList.GetCurSel()==-1 ) return FALSE;
      BOOL bDummy;
      OnBrowseClick(0, 0, 0, bDummy);
      return TRUE;
   }*/

   void DeleteAllItems()
   {
      m_wndList.ResetContent();
   }

   BOOL DeleteItem(int iIndex)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(iIndex>=0);
      return m_wndList.DeleteString(iIndex) != LB_ERR;
   }

   BOOL SetItemText(int iIndex, LPCTSTR pstrText)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(iIndex>=0);
      if (!DeleteItem(iIndex)) 
          return FALSE;
      return InsertItem(iIndex, pstrText) != LB_ERR;
   }

   BOOL GetItemText(int iIndex, LPTSTR pstrText, UINT cchMax) const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(iIndex>=0);
      ATLASSERT(!::IsBadWritePtr(pstrText, cchMax));
      *pstrText = _T('\0');
      if (iIndex >= GetItemCount()) return FALSE;      
      if ( (UINT)m_wndList.GetTextLen(iIndex)>cchMax ) return FALSE;
      return m_wndList.GetText(iIndex, pstrText);
   }

   int GetItemTextLen(int iIndex) const
   {
       ATLASSERT(::IsWindow(m_hWnd));
       ATLASSERT(iIndex >= 0);
       if (iIndex >= GetItemCount()) return 0;
       return m_wndList.GetTextLen(iIndex);
   }

   int GetItemCount() const
   {
      ATLASSERT(::IsWindow(m_hWnd));
      return m_wndList.GetCount();
   }   

   // Implementation
   void _Init()
   {
      RECT rc;
      GetClientRect(&rc);
      m_wndList.Create(m_hWnd, rcDefault, NULL, 
         WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
         LBS_NOTIFY | LBS_HASSTRINGS | LBS_WANTKEYBOARDINPUT | WS_VSCROLL, 0);
      ATLASSERT(m_wndList.IsWindow());
      m_wndList.SetFont(AtlGetDefaultGuiFont());

      m_wndEdit.Create(this, 1, m_wndList, &rcDefault, NULL, WS_BORDER | WS_CHILD | ES_AUTOHSCROLL);
      ATLASSERT(m_wndEdit.IsWindow());
      m_wndEdit.SetFont(AtlGetDefaultGuiFont());

      // Resize items
      RECT rcEdit;
      m_wndEdit.GetWindowRect(&rcEdit);
      int nHeight;
      nHeight = max( rcEdit.bottom-rcEdit.top, m_wndList.GetItemHeight(0) );
      nHeight += 2;
      m_wndList.SetItemHeight(0, nHeight);

      // Resize control
      //BOOL bDummy;
      //OnSize(WM_SIZE, SIZE_RESTORED, 0, bDummy);
   }

   LRESULT _SendNotifyMsg(UINT code, int iIndex) const
   {
      NMEDITLIST nm = { 0 };
      nm.hdr.hwndFrom = m_hWnd;
      nm.hdr.idFrom = GetDlgCtrlID();
      nm.hdr.code = code;
      nm.iIndex = iIndex;
      return ::SendMessage(GetParent(), WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);
   }

   void _BeginEdit(int iIndex)
   {
      ATLASSERT(iIndex>=0);
      if( iIndex<0 ) return;

      // Hack to make an empty last item
      /*if (iIndex == GetItemCount())
      {
         iIndex = m_wndList.InsertString(iIndex, _T(""));
         m_wndList.SetCurSel(iIndex);
      }*/

      // Allow owner to cancel action
      if( _SendNotifyMsg(EDLN_BEGINLABELEDIT, iIndex) != 0 ) return;
      
      // Copy text to Edit control
      int len = m_wndList.GetTextLen(iIndex)+1;
      LPTSTR pstr = (LPTSTR)_alloca(len*sizeof(TCHAR));
      ATLASSERT(pstr);
      m_wndList.GetText(iIndex, pstr);
      m_wndEdit.SetWindowText(pstr);

      RECT rc;
      m_wndList.GetItemRect(iIndex, &rc);     
      m_wndEdit.SetWindowPos(HWND_TOP, &rc, SWP_SHOWWINDOW);
      m_wndEdit.SetSel(0,-1);
      m_wndEdit.SetFocus();
   }

   // Overridables
   void UpdateLayout()
   {
      RECT rc;
      GetClientRect(&rc);
      m_wndList.SetWindowPos(HWND_TOP, &rc, SWP_NOZORDER|SWP_NOACTIVATE);
   }
  
   // Message handlers
   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      _Init();
      return lRes;
   }

   LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      if( wParam != SIZE_MINIMIZED ) {
         T* pT = static_cast<T*>(this);
         pT->UpdateLayout();
      }
      return 0;
   }

   LRESULT OnDblClick(WORD, WORD, HWND, BOOL&)
   {
      int selection = m_wndList.GetCurSel();
      if (selection != -1)
        _BeginEdit( selection );
      return 0;
   }

   LRESULT OnRightClick(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      return 0;
   }

   LRESULT OnChange(WORD id, WORD, HWND, BOOL&)
   {       
       SendMessage(GetParent(), WM_USER, 0, 0);
       return 0;
   }

   /*LRESULT OnBrowseClick(WORD , WORD , HWND , BOOL& )
   {
      _SendNotifyMsg(EDLN_BROWSE, m_wndList.GetCurSel());
      // We MUST set focus back to the edit control - otherwise
      // the focus logic will be screwed.
      ::SetFocus(m_wndEdit);
      return 0;
   }*/

   // CEdit
   LRESULT OnEditLostFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      int iIndex = m_wndList.GetCurSel();
      _SendNotifyMsg(EDLN_ENDLABELEDIT, iIndex);
      // Hide controls
      m_wndEdit.ShowWindow(SW_HIDE);
      //m_wndBrowse.ShowWindow(SW_HIDE);
      // Ask parent if it accepts the change
      if( _SendNotifyMsg(EDLN_ITEMCHANGING, iIndex)==0 ) {
         // Owner accepted text change.
         // Copy edit text to selected listview item
         int len = m_wndEdit.GetWindowTextLength()+1;
         LPTSTR pstr = (LPTSTR)_alloca(len*sizeof(TCHAR));
         ATLASSERT(pstr);
         m_wndEdit.GetWindowText(pstr, len);        
         SetItemText(iIndex, pstr);
         // Send "Item Changed" notify message
         _SendNotifyMsg(EDLN_ITEMCHANGED, iIndex);
      }
      m_wndList.SetCurSel(iIndex);
      bHandled = FALSE; // Windows needs this to disable cursor
      return 0;
   }

   LRESULT OnEditKey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( wParam ) {
      case VK_ESCAPE:
         {
            // Copy original listview text to edit control.
            // When the edit control looses focus, it will
            // transfer this text back to the list.
            int iIndex = m_wndList.GetCurSel();
            int len = m_wndList.GetTextLen(iIndex)+1;
            LPTSTR pstr = (LPTSTR)_alloca(len*sizeof(TCHAR));
            ATLASSERT(pstr);
            m_wndList.GetText(iIndex, pstr);
            m_wndEdit.SetWindowText(pstr);           
         }
         // FALL THROUGH...
      case VK_RETURN:
         {
            m_wndList.SetFocus(); // Causes Edit WM_KILLFOCUS
            return 0;
         }
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnEditGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS;
   }
};

class CEditListBox : public CEditListBoxImpl<CEditListBox>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_EditListBox"), GetWndClassName())
};
