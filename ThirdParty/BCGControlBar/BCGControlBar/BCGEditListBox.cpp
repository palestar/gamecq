//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //*******************************************************************************

// BCGEditListBox.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGEditListBox.h"
#include "KeyHelper.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iListId = 1;
static const int nTextMargin = 5;
static const int nBrowseButtonWidth = 20;

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListEdit

CBCGEditListEdit::CBCGEditListEdit()
{
	m_bLocked = FALSE;
	m_rectBtn.SetRectEmpty ();
	m_bIsButtonPressed = FALSE;
	m_bIsButtonCaptured = FALSE;
	m_pParentList = NULL;
	m_bBrowseBtn = FALSE;
}

CBCGEditListEdit::~CBCGEditListEdit()
{
}


BEGIN_MESSAGE_MAP(CBCGEditListEdit, CEdit)
	//{{AFX_MSG_MAP(CBCGEditListEdit)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListEdit message handlers

void CBCGEditListEdit::LockSize (CBCGEditListBase* pParent, BOOL bLock)
{
	m_pParentList = pParent;
	m_bLocked = bLock;
}
//*************************************************************************************
void CBCGEditListEdit::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	if (m_bLocked)
	{
		lpwndpos->flags |= SWP_NOSIZE;
	}

	CEdit::OnWindowPosChanging(lpwndpos);
}
//*************************************************************************************
void CBCGEditListEdit::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CEdit::OnWindowPosChanged(lpwndpos);
	
	if (m_bBrowseBtn)
	{
		CRect rectWindow;
		GetWindowRect (m_rectBtn);

		m_rectBtn.left = m_rectBtn.right -  nBrowseButtonWidth;
	}
	else
	{
		m_rectBtn.SetRectEmpty ();
	}
}
//*************************************************************************************
void CBCGEditListEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsButtonCaptured)
	{
		ReleaseCapture ();

		m_bIsButtonPressed = FALSE;
		m_bIsButtonCaptured = FALSE;

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

		CPoint ptScreen = point;
		ClientToScreen (&ptScreen);

		if (m_rectBtn.PtInRect (ptScreen) && m_pParentList != NULL)
		{
			ASSERT_VALID (m_pParentList);
			m_pParentList->OnBrowse ();
		}

		return;
	}
	
	CEdit::OnLButtonUp(nFlags, point);
}
//*************************************************************************************
void CBCGEditListEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bIsButtonCaptured)
	{
		CPoint ptScreen = point;
		ClientToScreen (&ptScreen);

		BOOL bIsButtonPressed = m_rectBtn.PtInRect (ptScreen);
		if (bIsButtonPressed != m_bIsButtonPressed)
		{
			m_bIsButtonPressed = bIsButtonPressed;
			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}

		return;
	}
	
	CEdit::OnMouseMove(nFlags, point);
}
//*************************************************************************************
void CBCGEditListEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (!m_bLocked)
	{
		CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);

		if (m_bBrowseBtn)
		{
			lpncsp->rgrc [0].right -= nBrowseButtonWidth;
		}
	}
}
//*************************************************************************************
void CBCGEditListEdit::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if (m_bBrowseBtn && m_rectBtn.PtInRect (point))
	{
		m_bIsButtonPressed = TRUE;
		m_bIsButtonCaptured = TRUE;

		SetCapture ();

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		return;
	}
	
	CEdit::OnNcLButtonDown(nHitTest, point);
}
//*************************************************************************************
void CBCGEditListEdit::OnNcPaint() 
{
	CWindowDC dc (this);

	CRect rectWindow;
	GetWindowRect (rectWindow);

	CRect rect = m_rectBtn;
	rect.OffsetRect (-rectWindow.left, -rectWindow.top);

	if (m_pParentList == NULL || !m_bBrowseBtn)
	{
		::FillRect (dc.GetSafeHdc (), &rect, ::GetSysColorBrush (COLOR_3DFACE));
		return;
	}

	ASSERT_VALID (m_pParentList);

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect (&rect);

	dc.SelectClipRgn (&rgnClip);
	m_pParentList->OnDrawBrowseButton (&dc, rect, m_bIsButtonPressed, FALSE);
	dc.SelectClipRgn (NULL);
}
//***************************************************************************************
BCGNcHitTestType CBCGEditListEdit::OnNcHitTest(CPoint point)
{
	if (m_rectBtn.PtInRect (point))
	{
		return HTCAPTION;
	}
	
	return CEdit::OnNcHitTest(point);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBase

CBCGEditListBase::CBCGEditListBase()
{
	m_sizeButton = CSize (0, 0);
	m_uiStandardBtns = 0;
	m_bNewItem = FALSE;
	m_bIsActualDelete = TRUE;
	m_bBrowseButton = FALSE;
	m_bGrayDisabledButtons = FALSE;
}
//**************************************************************************
CBCGEditListBase::~CBCGEditListBase()
{
	while (!m_lstButtons.IsEmpty ())
	{
		delete m_lstButtons.RemoveHead ();
	}
}

BEGIN_MESSAGE_MAP(CBCGEditListBase, CStatic)
	//{{AFX_MSG_MAP(CBCGEditListBase)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_SETFONT, OnSetFont)
    ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBase message handlers
void CBCGEditListBase::PreSubclassWindow() 
{
	CStatic::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init ();
	}
}
//**********************************************************************************
int CBCGEditListBase::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init ();
	return 0;
}
//**********************************************************************************
void CBCGEditListBase::Init ()
{
	ModifyStyle (0, SS_USERITEM);

	if (OnCreateList () == NULL)
	{
		TRACE0("CBCGEditListBase::Init (): Can not create list control\n");
		return;
	}

	AdjustLayout ();
}
//**********************************************************************************
BOOL CBCGEditListBase::SetStandardButtons (UINT uiBtns)
{
	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGLocalResource locaRes;

	CString strButton;
	if (uiBtns & BGCEDITLISTBOX_BTN_NEW)
	{
		strButton.LoadString (IDS_BCGBARRES_NEW);
		VERIFY (AddButton (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_NEW32 : IDB_BCGBARRES_NEW, 
			strButton, VK_INSERT,0,BGCEDITLISTBOX_BTN_NEW_ID));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_DELETE)
	{
		strButton.LoadString (IDS_BCGBARRES_DELETE);
		VERIFY (AddButton (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_DELETE32 : IDB_BCGBARRES_DELETE, 
			strButton, VK_DELETE, 0, BGCEDITLISTBOX_BTN_DELETE_ID));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_UP)
	{
		strButton.LoadString (IDS_BCGBARRES_MOVEUP);
		VERIFY (AddButton (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_UP32 : IDB_BCGBARRES_UP, 
			strButton, VK_UP, FALT, BGCEDITLISTBOX_BTN_UP_ID));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_DOWN)
	{
		strButton.LoadString (IDS_BCGBARRES_MOVEDN);
		VERIFY (AddButton (
			globalData.Is32BitIcons () ? IDB_BCGBARRES_DOWN32 : IDB_BCGBARRES_DOWN, 
			strButton, VK_DOWN, FALT, BGCEDITLISTBOX_BTN_DOWN_ID));
	}

	m_uiStandardBtns |= uiBtns;
	return TRUE;
}
//**********************************************************************************
BOOL CBCGEditListBase::AddButton (UINT uiImageResId, 
								 LPCTSTR lpszTooltip/* = NULL*/,
								 WORD wKeyAccelerator/* = 0*/,
								 BYTE fVirt/* = 0*/,
								 UINT uiButtonID/* = 0*/)
{
	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (uiButtonID != 0)
	{
		ASSERT(GetButtonNum(uiButtonID)==-1); // button with this ID still not added
	}

	CRect rectEmpty;
	rectEmpty.SetRectEmpty ();

	CBCGButton* pButton = new CBCGButton ();
	if (!pButton->Create (_T(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW, 
		rectEmpty, this, (int) m_lstButtons.GetCount () + 2))
	{
		return FALSE;
	}

	pButton->m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
	pButton->m_bGrayDisabled = m_bGrayDisabledButtons;
	pButton->m_bDrawFocus = FALSE;

	pButton->SetImage (uiImageResId);
	if (lpszTooltip != NULL)
	{
		CString strTooltip = lpszTooltip;
		if (wKeyAccelerator != 0)
		{
			ACCEL acccel;
			acccel.cmd = 0;
			acccel.fVirt = (BYTE) (fVirt | FVIRTKEY);
			acccel.key = wKeyAccelerator;

			CBCGKeyHelper helper (&acccel);
			CString strAccellKey;
			helper.Format (strAccellKey);

			strTooltip += _T(" (");
			strTooltip += strAccellKey;
			strTooltip += _T(")");
		}

		pButton->SetTooltip (strTooltip);
	}

	pButton->SizeToContent ();
	CRect rectBtn;
	pButton->GetWindowRect (rectBtn);
	CSize sizeButton = rectBtn.Size ();

	if (m_lstButtons.IsEmpty ())
	{
		m_sizeButton = sizeButton;
	}
	else
	{
		ASSERT (m_sizeButton == sizeButton);
	}

	m_lstButtons.AddTail (pButton);

	if (wKeyAccelerator == 0)
	{
		fVirt = 0;
	}

	DWORD dwKey = (fVirt << 16) | wKeyAccelerator;
	m_lstKeyAccell.AddTail (dwKey);

	if (uiButtonID != 0)
	{
		int iButton = (int) m_lstButtons.GetCount () - 1;
		m_mapButtonIDs.SetAt (iButton, uiButtonID);
	}

	AdjustLayout ();
	return TRUE;
}
//**********************************************************************************
void CBCGEditListBase::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect (m_rectCaption, &globalData.brBtnFace);
	dc.Draw3dRect (m_rectCaption, globalData.clrBtnShadow, globalData.clrBtnHilite);

	CRect rectText = m_rectCaption;
	rectText.DeflateRect (nTextMargin, 0);

	dc.SetBkMode (TRANSPARENT);
	dc.SetTextColor (IsWindowEnabled () ? globalData.clrBtnText : globalData.clrGrayedText);

	CFont* pOldFont = NULL;
	
	if (m_font.GetSafeHandle () != NULL)
	{
		pOldFont = dc.SelectObject (&m_font);
	}
	else
	{
		CFont* pParentFont = GetParent ()->GetFont ();

		if (pParentFont != NULL)
		{
			pOldFont = dc.SelectObject (pParentFont);
			ASSERT (pOldFont != NULL);
		}
	}

	CString strCaption;
	GetWindowText (strCaption);
	dc.DrawText (strCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	if (pOldFont != NULL)
	{
		dc.SelectObject (pOldFont);
	}
}
//**********************************************************************************
void CBCGEditListBase::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	AdjustLayout ();
}
//***********************************************************************************
void CBCGEditListBase::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	m_rectCaption = rectClient;

	CClientDC dc (this);

	CFont* pOldFont = NULL;
	if (m_font.GetSafeHandle () != NULL)
	{
		pOldFont = dc.SelectObject (&m_font);
	}
	else
	{
		CFont* pParentFont = GetParent ()->GetFont ();

		if (pParentFont != NULL)
		{
			pOldFont = dc.SelectObject (pParentFont);
			ASSERT (pOldFont != NULL);
		}
	}

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	if (pOldFont != NULL)
	{
		dc.SelectObject (pOldFont);
	}

	m_rectCaption.bottom = m_rectCaption.top +
		max (tm.tmHeight * 4 / 3, m_sizeButton.cy);

	int x = rectClient.right - 1 - m_sizeButton.cx;
	for (POSITION pos = m_lstButtons.GetTailPosition (); pos != NULL;)
	{
		CBCGButton* pButton = m_lstButtons.GetPrev (pos);
		ASSERT (pButton != NULL);

		pButton->MoveWindow (x, rectClient.top + 1, m_sizeButton.cx, 
			m_rectCaption.Height () - 2);
		x -= m_sizeButton.cx;
	}

	CWnd* pWndList = CWnd::FromHandle (GetListHwnd ());
	if (pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pWndList->MoveWindow (	rectClient.left,
							rectClient.top + m_rectCaption.Height (),
							rectClient.Width (),
							rectClient.Height () - m_rectCaption.Height ());
	OnSizeList ();
}
//************************************************************************************
BOOL CBCGEditListBase::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//************************************************************************************
BOOL CBCGEditListBase::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	HWND hwnd = (HWND) lParam;

	int iButton = 0;
	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGButton* pButton = m_lstButtons.GetNext (pos);
		ASSERT (pButton);
	
		if (pButton->GetSafeHwnd () == hwnd)
		{
			CWnd* pWndList = CWnd::FromHandle (GetListHwnd ());
			if (pWndList == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				pWndList->SetFocus ();
			}

			OnClickButton (iButton);
			return TRUE;
		}
	}
	
	return CStatic::OnCommand(wParam, lParam);
}
//***************************************************************************
void CBCGEditListBase::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CWnd* pWndList = CWnd::FromHandle (GetListHwnd ());
	if (pWndList == NULL)
	{
		ASSERT (FALSE);
	}
	else
	{
		pWndList->SetFocus ();
	}
}
//***************************************************************************
void CBCGEditListBase::OnClickButton (int iButton)
{
	if (m_uiStandardBtns == 0)
	{
		return;
	}

	int iSelItem = GetSelItem ();
	UINT uiBtnID = GetButtonID (iButton);

	switch (uiBtnID)
	{ 
	case BGCEDITLISTBOX_BTN_NEW_ID:
		CreateNewItem ();
		return;

	case BGCEDITLISTBOX_BTN_DELETE_ID:
		if (iSelItem >= 0)
		{
			if (OnBeforeRemoveItem (iSelItem))
			{
				RemoveItem (iSelItem);
			}
		}
		break;

	case BGCEDITLISTBOX_BTN_UP_ID:
	case BGCEDITLISTBOX_BTN_DOWN_ID:
		if (iSelItem >= 0)
		{
			BOOL bIsUp = (uiBtnID == BGCEDITLISTBOX_BTN_UP_ID);
			if (bIsUp)
			{
				if (iSelItem == 0)
				{
					return;
				}
			}
			else
			{
				if (iSelItem == GetCount () - 1)
				{
					return;
				}
			}

			// Adjust list control:
			SetRedraw (FALSE);

			CString strLabel = GetItemText (iSelItem);
			DWORD_PTR dwData = GetItemData (iSelItem);

			m_bIsActualDelete = FALSE;
			RemoveItem (iSelItem);
			m_bIsActualDelete = TRUE;
			
			if (bIsUp)
			{
				iSelItem --;
			}
			else
			{
				iSelItem ++;
			}

			AddItem (strLabel, dwData, iSelItem);
			SelectItem (iSelItem);

			SetRedraw ();

			CWnd* pWndList = CWnd::FromHandle (GetListHwnd ());
			if (pWndList == NULL)
			{
				ASSERT (FALSE);
			}
			else
			{
				pWndList->Invalidate ();
			}

			if (bIsUp)
			{
				OnAfterMoveItemUp (iSelItem);
			}
			else
			{
				OnAfterMoveItemDown (iSelItem);
			}
		}
	}
}
//****************************************************************************
int CBCGEditListBase::GetStdButtonNum (UINT uiStdBtn) const
{
	if ((m_uiStandardBtns & uiStdBtn) == 0)
	{
		return -1;
	}

	switch (uiStdBtn)
	{
	case BGCEDITLISTBOX_BTN_NEW:
		return GetButtonNum(BGCEDITLISTBOX_BTN_NEW_ID);	

	case BGCEDITLISTBOX_BTN_DELETE:
		return GetButtonNum(BGCEDITLISTBOX_BTN_DELETE_ID);

	case BGCEDITLISTBOX_BTN_UP:
		return GetButtonNum(BGCEDITLISTBOX_BTN_UP_ID);

	case BGCEDITLISTBOX_BTN_DOWN:
		return GetButtonNum(BGCEDITLISTBOX_BTN_DOWN_ID);
	}

	ASSERT (FALSE);
	return -1;
}
//***********************************************************************************
void CBCGEditListBase::CreateNewItem ()
{
	int iLastItem = AddItem (_T(""));
	ASSERT (iLastItem >= 0);

	m_bNewItem = TRUE;
	EditItem (iLastItem);
}
//**************************************************************************
void CBCGEditListBase::OnKey (WORD wKey, BYTE fFlags)
{
	int iSelItem = GetSelItem ();
	TCHAR cKey = (TCHAR) LOWORD (::MapVirtualKey (wKey, 2));

	if (fFlags == 0 &&	// No Ctrl, Shift or Alt
		iSelItem >= 0 &&
		(cKey == _T(' ') || wKey == VK_F2))
	{
		int iSelItem = GetSelItem ();

		if (iSelItem >= 0)
		{
			EditItem (iSelItem);
		}
	}
}
//**************************************************************************
LRESULT CBCGEditListBase::OnGetFont(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return (LRESULT) m_font.GetSafeHandle ();
}
//**************************************************************************
LRESULT CBCGEditListBase::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();

	CFont* pFont = CFont::FromHandle ((HFONT) wParam);
	if (pFont != NULL)
	{
		LOGFONT lf;
		pFont->GetLogFont (&lf);

		m_font.DeleteObject ();
		m_font.CreateFontIndirect (&lf);
	}

	if (::IsWindow(GetSafeHwnd ()) != NULL)
	{
		AdjustLayout ();

		if (lParam != 0)
		{
			Invalidate ();
			UpdateWindow ();
		}
	}

	return lResult;
}
//**************************************************************************
void CBCGEditListBase::OnEndEditLabel (LPCTSTR lpszLabel)
{
	int iSelItem = GetSelItem ();
	if (iSelItem < 0)
	{
		ASSERT (FALSE);
		return;
	}

	CString strLabel = (lpszLabel != NULL) ? lpszLabel : _T("");

	if (!strLabel.IsEmpty ())
	{
		SetItemText (iSelItem, strLabel);

		if (m_bNewItem)
		{
			OnAfterAddItem (iSelItem);
		}
		else
		{
			OnAfterRenameItem (iSelItem);
		}
	}
	else
	{
		if (m_bNewItem)
		{
			RemoveItem (iSelItem);
		}
	}

	m_bNewItem = FALSE;
}
//****************************************************************************
BOOL CBCGEditListBase::EnableButton (int iButtonNum, BOOL bEnable/* = TRUE*/)
{
	POSITION pos = m_lstButtons.FindIndex (iButtonNum);
	if (pos == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGButton* pButton = m_lstButtons.GetAt (pos);
	ASSERT_VALID (pButton);

	pButton->EnableWindow (bEnable);
	return TRUE;
}
//****************************************************************************
UINT CBCGEditListBase::GetButtonID (int iButtonNum) const
{
	UINT uiID = 0;
	m_mapButtonIDs.Lookup (iButtonNum, uiID);

	return uiID;
}
//****************************************************************************
int CBCGEditListBase::GetButtonNum (UINT uiID) const
{
	for (POSITION pos = m_mapButtonIDs.GetStartPosition (); pos != NULL;)
	{
		int iNum = -1;
		UINT uiButtonID = 0;

		m_mapButtonIDs.GetNextAssoc (pos, iNum, uiButtonID);

		if (uiButtonID == uiID)
		{
			return iNum;
		}
	}

	return -1;
}
//****************************************************************************
void CBCGEditListBase::EnableBrowseButton (BOOL bEnable/* = TRUE*/)
{
	m_bBrowseButton = bEnable;
}
//****************************************************************************
void CBCGEditListBase::SetGrayDisabledButtons (BOOL bOn)
{
	m_bGrayDisabledButtons = bOn;
}
//****************************************************************************
void CBCGEditListBase::OnDrawBrowseButton (CDC* pDC, CRect rectBtn, 
										   BOOL bPressed, BOOL /*bHighlighted*/)
{
	ASSERT (m_bBrowseButton);
	ASSERT_VALID (pDC);

	::FillRect (pDC->GetSafeHdc (), &rectBtn, ::GetSysColorBrush (COLOR_3DFACE));

	COLORREF clrText = pDC->SetTextColor (::GetSysColor (COLOR_BTNTEXT));
	int nTextMode = pDC->SetBkMode (TRANSPARENT);
	CFont* pFont = (CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT);

	CString strText = _T("...");

	CRect rectText = rectBtn;
	rectText.DeflateRect (1, 2);
	rectText.OffsetRect (0, -2);

	if (bPressed)
	{
		rectText.OffsetRect (1, 1);
	}

	pDC->DrawText (strText, rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	pDC->SetTextColor (clrText);
	pDC->SetBkMode (nTextMode);
	pDC->SelectObject (pFont);

	pDC->Draw3dRect (rectBtn, ::GetSysColor (COLOR_3DDKSHADOW), ::GetSysColor (COLOR_3DDKSHADOW));

	rectBtn.DeflateRect (1, 1);
	pDC->DrawEdge (rectBtn, bPressed ? BDR_SUNKENINNER : BDR_RAISEDINNER, BF_RECT);
}
//************************************************************************************
void CBCGEditListBase::OnEnable(BOOL bEnable) 
{
	CStatic::OnEnable(bEnable);
	
	for (POSITION pos = m_lstButtons.GetTailPosition (); pos != NULL;)
	{
		CBCGButton* pButton = m_lstButtons.GetPrev (pos);
		ASSERT_VALID (pButton);

		pButton->m_bGrayDisabled = !bEnable || m_bGrayDisabledButtons;
		pButton->EnableWindow (bEnable);
	}

	CWnd* pWndList = CWnd::FromHandle (GetListHwnd ());
	if (pWndList != NULL)
	{
		pWndList->EnableWindow (bEnable);
	}

	RedrawWindow ();
}
//************************************************************************************
UINT CBCGEditListEdit::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBox

CBCGEditListBox::CBCGEditListBox()
{
	m_pWndList = NULL;
	m_ptClick = CPoint (-1, -1);
}
//**************************************************************************
CBCGEditListBox::~CBCGEditListBox()
{
	if (m_pWndList != NULL)
	{
		ASSERT_VALID (m_pWndList);
		delete m_pWndList;
	}
}

BEGIN_MESSAGE_MAP(CBCGEditListBox, CBCGEditListBase)
	//{{AFX_MSG_MAP(CBCGEditListBox)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, iListId, OnKeyDown)
	ON_NOTIFY(NM_DBLCLK, iListId, OnDblclkList)
	ON_NOTIFY(LVN_GETDISPINFO, iListId, OnGetdispinfo)
	ON_NOTIFY(LVN_ENDLABELEDIT, iListId, OnEndLabelEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, iListId, OnItemChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBox message handlers

CWnd* CBCGEditListBox::OnCreateList ()
{
	if (GetSafeHwnd () == NULL ||
		m_pWndList != NULL)
	{
		return FALSE;
	}

	ASSERT (GetStyle () & WS_CHILD);

	CRect rectEmpty;
	rectEmpty.SetRectEmpty ();

	m_pWndList = new CListCtrl;
	m_pWndList->CWnd::CreateEx (WS_EX_CLIENTEDGE, _T("SysListView32"), _T(""),
						WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | 
						LVS_NOCOLUMNHEADER | LVS_EDITLABELS | LVS_SHOWSELALWAYS,
						rectEmpty, this, iListId);

	m_pWndList->SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_pWndList->InsertColumn (0, _T(""));

	return m_pWndList;
}
//*************************************************************************************
int CBCGEditListBox::AddItem (const CString& strText, DWORD_PTR dwData, int iIndex)
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		return -1;
	}

	ASSERT_VALID (m_pWndList);

	if (iIndex < 0)
	{
		iIndex = m_pWndList->GetItemCount ();
	}

	int iItem = m_pWndList->InsertItem (iIndex, strText, I_IMAGECALLBACK);
	m_pWndList->SetItemData (iItem, dwData);

	if (iItem == 0)
	{
		SelectItem (0);
	}

	return iItem;
}
//*************************************************************************************
int CBCGEditListBox::GetCount () const
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		return -1;
	}

	return m_pWndList->GetItemCount ();
}
//*************************************************************************************
CString CBCGEditListBox::GetItemText (int iIndex) const
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		return _T("");
	}

	ASSERT_VALID (m_pWndList);

	return m_pWndList->GetItemText (iIndex, 0);
}
//*************************************************************************************
DWORD_PTR CBCGEditListBox::GetItemData (int iIndex) const
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	ASSERT_VALID (m_pWndList);

	return (DWORD_PTR) m_pWndList->GetItemData (iIndex);
}
//*************************************************************************************
void CBCGEditListBox::SetItemData (int iIndex, DWORD_PTR dwData)
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pWndList);

	m_pWndList->SetItemData (iIndex, dwData);
}
//*************************************************************************************
#if _MSC_VER >= 1300
void CBCGEditListBox::OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pKeyDown = (LPNMLVKEYDOWN) pNMHDR;
#else
void CBCGEditListBox::OnKeyDown (LPNMLVKEYDOWN pKeyDown, LRESULT* pResult)
{
#endif
	*pResult = 0;

	if (pKeyDown != NULL)
	{
		BYTE fCurrVirt = 0;

		if (::GetAsyncKeyState (VK_CONTROL) & 0x8000)
		{
			fCurrVirt |= FCONTROL;
		}

		if (::GetAsyncKeyState (VK_MENU) & 0x8000)
		{
			fCurrVirt |= FALT;
		}

		if (::GetAsyncKeyState (VK_SHIFT) & 0x8000)
		{
			fCurrVirt |= FSHIFT;
		}

		int iButton = 0;
		for (POSITION pos = m_lstKeyAccell.GetHeadPosition (); pos != NULL; iButton ++)
		{
			DWORD dwKey = m_lstKeyAccell.GetNext (pos);
		
			if (dwKey != 0 && pKeyDown->wVKey == (dwKey & 0xFFFF))
			{
				//-------------------
				// Check state flags:
				//-------------------
				BYTE fVirt = (BYTE) (dwKey >> 16);
				if (fCurrVirt == fVirt)
				{
					OnClickButton (iButton);
					return;
				}
			}
		}

		OnKey (pKeyDown->wVKey, fCurrVirt);
	}
}
//*************************************************************************************
int CBCGEditListBox::GetSelItem () const
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT_VALID (m_pWndList);

	return m_pWndList->GetNextItem (-1, LVNI_SELECTED);
}
//*************************************************************************************
BOOL CBCGEditListBox::SelectItem (int iItem)
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (m_pWndList);

	if (!m_pWndList->SetItemState (iItem,	LVIS_SELECTED | LVIS_FOCUSED, 
								LVIS_SELECTED | LVIS_FOCUSED))
	{
		return FALSE;
	}

	return m_pWndList->EnsureVisible (iItem, FALSE);
}
//*************************************************************************************
void CBCGEditListBox::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	*pResult = 0;

	int iSelItem = GetSelItem ();

	if ((m_uiStandardBtns & BGCEDITLISTBOX_BTN_NEW) && iSelItem == -1)
	{
		CreateNewItem ();
		return;
	}

	if (iSelItem >= 0)
	{
		EditItem (iSelItem);
	}
}
//*************************************************************************************
BOOL CBCGEditListBox::RemoveItem (int iIndex)
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (m_pWndList);

	BOOL bIsSelected = (GetSelItem () == iIndex);

	if (!m_pWndList->DeleteItem (iIndex))
	{
		return FALSE;
	}

	if (!bIsSelected || GetCount () == 0)
	{
		return FALSE;
	}

	//-------------------
	// Restore selection:
	//-------------------
	if (iIndex >= GetCount ())
	{
		iIndex --;
	}

	SelectItem (iIndex);
	return TRUE;
}
//*************************************************************************************
void CBCGEditListBox::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT (pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ASSERT (pItem != NULL);

	if (pItem->mask & LVIF_IMAGE)
	{
		pItem->iImage = OnGetImage (pItem);
	}
	
	*pResult = 0;
}
//**************************************************************************
void CBCGEditListBox::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT (pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ASSERT (pItem != NULL);

	OnEndEditLabel (pItem->pszText);

	for (POSITION pos = m_lstButtons.GetTailPosition (); pos != NULL;)
	{
		CBCGButton* pButton = m_lstButtons.GetPrev (pos);
		ASSERT (pButton != NULL);

		pButton->EnableWindow ();
	}

	*pResult = 0;
}
//**************************************************************************
void CBCGEditListBox::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT (pNMHDR != NULL);

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ASSERT (pNMListView != NULL);

	if (pNMListView->uChanged == LVIF_STATE &&
		(pNMListView->uOldState & LVIS_SELECTED) != (pNMListView->uNewState & LVIS_SELECTED))
	{
		OnSelectionChanged ();
	}

	*pResult = 0;
}
//**************************************************************************
BOOL CBCGEditListBox::EditItem (int iIndex)
{
	m_wndEdit.LockSize (NULL, FALSE);

	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (m_pWndList);

	m_pWndList->SetFocus ();
	CEdit* pEdit = m_pWndList->EditLabel (iIndex);
	if (pEdit == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pEdit);

	m_wndEdit.SubclassWindow (pEdit->GetSafeHwnd ());
	m_wndEdit.EnableBrowse (m_bBrowseButton);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectEdit;
	pEdit->GetClientRect (rectEdit);

	pEdit->SetWindowPos (NULL, -1, -1,
		rectClient.Width () - 2 * afxData.cxBorder2 - ::GetSystemMetrics (SM_CXVSCROLL),
		rectEdit.Height (),
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	m_wndEdit.LockSize (this);

	for (POSITION pos = m_lstButtons.GetTailPosition (); pos != NULL;)
	{
		CBCGButton* pButton = m_lstButtons.GetPrev (pos);
		ASSERT (pButton != NULL);

		pButton->EnableWindow (FALSE);
	}

	return TRUE;
}
//**************************************************************************
void CBCGEditListBox::OnSizeList ()
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pWndList);

	CRect rectClient;
	GetClientRect (rectClient);

	m_pWndList->SetColumnWidth (0, rectClient.Width () -
		2 * ::GetSystemMetrics (SM_CXEDGE) -
		::GetSystemMetrics (SM_CXVSCROLL));
}
//****************************************************************************
void CBCGEditListBox::SetItemText (int iIndex, const CString& strText)
{
	if (GetSafeHwnd () == NULL || m_pWndList == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (m_pWndList);

	m_pWndList->SetItemText (iIndex, 0, strText);
}
//****************************************************************************
BOOL CBCGEditListBox::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDOWN &&
		m_pWndList != NULL &&
		m_pWndList->GetEditControl () == NULL &&
		m_pWndList->GetSafeHwnd () == CWnd::GetFocus ()->GetSafeHwnd ())
	{
		ASSERT_VALID (m_pWndList);

		m_ptClick = CPoint (-1, -1);

		CPoint ptClick = pMsg->pt;
		m_pWndList->ScreenToClient (&ptClick);

		UINT uFlags;
		int iItem = m_pWndList->HitTest (ptClick, &uFlags);
		if (iItem >= 0 && (uFlags & LVHT_ONITEMLABEL))
		{
			UINT uiMask = LVIS_FOCUSED | LVIS_SELECTED;
			if ((m_pWndList->GetItemState (iItem, uiMask) & uiMask) == uiMask)
			{
				// Secondary click on selected item:
				m_ptClick = ptClick;

				SetCapture ();
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_LBUTTONUP && m_ptClick != CPoint (-1, -1))
	{
		ASSERT_VALID (m_pWndList);

		ReleaseCapture ();

		CPoint ptClick = pMsg->pt;
		m_pWndList->ScreenToClient (&ptClick);

		int iItem = m_pWndList->HitTest (ptClick);

		BOOL bEditItem =  
			iItem >= 0 &&
			(abs (ptClick.x - m_ptClick.x) < ::GetSystemMetrics (SM_CXDRAG) &&
			abs (ptClick.y - m_ptClick.y) < ::GetSystemMetrics (SM_CYDRAG));

		m_ptClick = CPoint (-1, -1);

		if (bEditItem)
		{
			EditItem (iItem);
		}

		return TRUE;
	}
	
	return CBCGEditListBase::PreTranslateMessage(pMsg);
}
