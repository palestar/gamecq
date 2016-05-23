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
// BCGPPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGPopupDlg.h"

#include "BCGPopupWindow.h"
#include "bcglocalres.h"
#include "bcgbarres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_CLASS_NAME	255
#define STATIC_CLASS	_T("Static")
#define BUTTON_CLASS	_T("Button")

#define MAX_TEXT_LEN	512

IMPLEMENT_DYNCREATE (CBCGPopupDlg, CBCGDialog)

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupDlg

CBCGPopupDlg::CBCGPopupDlg()
{
	m_pParentPopup = NULL;
	m_bDefault = FALSE;
	m_sizeDlg = CSize (0, 0);
	m_bDontSetFocus = FALSE;
	m_bMenuIsActive = FALSE;
}

CBCGPopupDlg::~CBCGPopupDlg()
{
}


BEGIN_MESSAGE_MAP(CBCGPopupDlg, CBCGDialog)
	//{{AFX_MSG_MAP(CBCGPopupDlg)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPopupDlg message handlers

HBRUSH CBCGPopupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);

		if (globalData.IsHighContastMode ())
		{
			pDC->SetTextColor (globalData.clrWindowText);
		}

		return (HBRUSH) ::GetStockObject (HOLLOW_BRUSH);
	}

	return CBCGDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CBCGPopupDlg::OnEraseBkgnd(CDC* pDC)
{
	if (!globalData.IsWinXPDrawParentBackground ())
	{
		CRect rectClient;
		GetClientRect (&rectClient);
		
		CBCGVisualManager::GetInstance ()->OnFillPopupWindowBackground (
			pDC, rectClient);
	}

	return TRUE;
}

void CBCGPopupDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	OnDraw (pDC);
}

void CBCGPopupDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBCGDialog::OnLButtonDown(nFlags, point);

	GetParent ()->SendMessage (WM_LBUTTONDOWN, 0, MAKELPARAM (point.x, point.y));
	SetFocus ();
}

BOOL CBCGPopupDlg::HasFocus () const
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (m_bMenuIsActive)
	{
		return TRUE;
	}

	CWnd* pWndMain = AfxGetMainWnd ();
	if (pWndMain->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (pWndMain->IsIconic () ||
		!pWndMain->IsWindowVisible() ||
		pWndMain != GetForegroundWindow ())
	{
		return FALSE;
	}

    CWnd* pFocus = GetFocus();

    BOOL bActive = (pFocus->GetSafeHwnd () != NULL && 
		(IsChild (pFocus) || pFocus->GetSafeHwnd () == GetSafeHwnd ()));

	return bActive;
}

BOOL CBCGPopupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	ASSERT_VALID (m_pParentPopup);

	if (m_pParentPopup->ProcessCommand ((HWND)lParam))
	{
		return TRUE;
	}
	
	return CBCGDialog::OnCommand(wParam, lParam);
}

int CBCGPopupDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pParentPopup = DYNAMIC_DOWNCAST (CBCGPopupWindow, GetParent ());
	ASSERT_VALID (m_pParentPopup);
	
	return 0;
}

BOOL CBCGPopupDlg::OnInitDialog() 
{
	CBCGDialog::OnInitDialog();
	
	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);

		CBCGButton* pButton = DYNAMIC_DOWNCAST(CBCGButton, pWndChild);
		if (pButton != NULL)
		{
			pButton->m_bDrawFocus = FALSE;
		}
		else
		{
			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName (pWndChild->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == STATIC_CLASS && (pWndChild->GetStyle () & SS_ICON))
			{
				pWndChild->ShowWindow (SW_HIDE);
			}
		}

		pWndChild = pWndChild->GetNextWindow ();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CBCGPopupDlg::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle ((HDC) wp);
		ASSERT_VALID (pDC);

		OnDraw (pDC);
	}

	return 0;
}

void CBCGPopupDlg::OnDraw (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (&rectClient);
	
	CBCGVisualManager::GetInstance ()->OnFillPopupWindowBackground (
		pDC, rectClient);

	CWnd* pWndChild = GetWindow (GW_CHILD);

	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);

		TCHAR lpszClassName [MAX_CLASS_NAME + 1];

		::GetClassName (pWndChild->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
		CString strClass = lpszClassName;

		if (strClass == STATIC_CLASS && (pWndChild->GetStyle () & SS_ICON))
		{
			CRect rectIcon;
			pWndChild->GetWindowRect (rectIcon);
			ScreenToClient (rectIcon);

			HICON hIcon = ((CStatic*) pWndChild)->GetIcon ();
			pDC->DrawIcon (rectIcon.TopLeft (), hIcon);
		}

		pWndChild = pWndChild->GetNextWindow ();
	}
}

CSize CBCGPopupDlg::GetOptimalTextSize (CString str)
{
	if (str.IsEmpty ())
	{
		return CSize (0, 0);
	}

	CRect rectScreen;

	CRect rectDlg;
	GetWindowRect (rectDlg);

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectDlg.TopLeft (), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);
	ASSERT_VALID (pOldFont);

	int nStepY = globalData.GetTextHeight ();
	int nStepX = nStepY * 3;

	CRect rectText (0, 0, nStepX, nStepY);

	for (;;)
	{
		CRect rectTextSaved = rectText;

		int nHeight = dc.DrawText (str, rectText, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
		int nWidth = rectText.Width ();

		rectText = rectTextSaved;

		if (nHeight <= rectText.Height () ||
			rectText.Width () > rectScreen.Width () ||
			rectText.Height () > rectScreen.Height ())
		{
			rectText.bottom = rectText.top + nHeight + 5;
			rectText.right = rectText.left + nWidth + 5;
			break;
		}

		rectText.right += nStepX;
		rectText.bottom += nStepY;
	}

	dc.SelectObject (pOldFont);
	return rectText.Size ();
}

BOOL CBCGPopupDlg::CreateFromParams (CBCGPopupWndParams& params, CBCGPopupWindow* pParent)
{
	CBCGLocalResource	lr;

	if (!Create (IDD_BCGBARRES_POPUP_DLG, pParent))
	{
		return FALSE;
	}

	m_Params = params;

	int xMargin = 10;
	int yMargin = 10;

	int x = xMargin;
	int y = yMargin;

	int cxIcon = 0;

	CString strText = m_Params.m_strText;
	if (strText.GetLength () > MAX_TEXT_LEN)
	{
		strText = strText.Left (MAX_TEXT_LEN - 1);
	}

	CString strURL = m_Params.m_strURL;
	if (strURL.GetLength () > MAX_TEXT_LEN)
	{
		strURL = strURL.Left (MAX_TEXT_LEN - 1);
	}

	CSize sizeText = GetOptimalTextSize (strText);
	CSize sizeURL = GetOptimalTextSize (strURL);

	int cx = max (sizeText.cx, sizeURL.cx);

	if (m_Params.m_hIcon != NULL)
	{
		ICONINFO iconInfo;
		::GetIconInfo (m_Params.m_hIcon, &iconInfo);

		BITMAP bitmap;
		::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

		::DeleteObject (iconInfo.hbmColor);
		::DeleteObject (iconInfo.hbmMask);

		CRect rectIcon = CRect (xMargin, yMargin, 
								bitmap.bmWidth + xMargin, bitmap.bmHeight + yMargin);

		m_wndIcon.Create (_T(""), WS_CHILD | SS_ICON | SS_NOPREFIX, rectIcon, this);
		m_wndIcon.SetIcon (m_Params.m_hIcon);

		cxIcon = rectIcon.Width () + xMargin;
		x += cxIcon;
	}

	if (!strText.IsEmpty ())
	{
		CRect rectText (CPoint (x, y), CSize (cx, sizeText.cy));

		m_wndText.Create (strText, WS_CHILD | WS_VISIBLE, rectText, this);;
		m_wndText.SetFont (&globalData.fontRegular);

		y = rectText.bottom + yMargin;
	}

	if (!strURL.IsEmpty ())
	{
		CRect rectURL (CPoint (x, y), CSize (cx, sizeURL.cy));

		m_btnURL.Create (strURL, WS_VISIBLE | WS_CHILD, 
			rectURL, this, m_Params.m_nURLCmdID);

		m_btnURL.m_bMultilineText = TRUE;
		m_btnURL.m_bAlwaysUnderlineText = FALSE;
		m_btnURL.m_bDefaultClickProcess = TRUE;
		m_btnURL.m_bDrawFocus = FALSE;

		y = rectURL.bottom + yMargin;
	}

	m_sizeDlg = CSize (cxIcon + cx + 2 * xMargin, y);
	return TRUE;
}

CSize CBCGPopupDlg::GetDlgSize ()
{
	if (!m_bDefault)
	{
		ASSERT (FALSE);
		return CSize (0, 0);
	}

	return m_sizeDlg;
}

void CBCGPopupDlg::OnSetFocus(CWnd* pOldWnd) 
{
	if (m_bDontSetFocus && pOldWnd->GetSafeHwnd () != NULL)
	{
		pOldWnd->SetFocus ();
		return;
	}

	CBCGDialog::OnSetFocus(pOldWnd);
}

BOOL CBCGPopupDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDOWN &&
		m_pParentPopup->GetSafeHwnd () != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);
		if (pWnd != NULL)
		{
			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName (pWnd->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == STATIC_CLASS || pWnd->GetSafeHwnd () == GetSafeHwnd ())
			{
				m_pParentPopup->StartWindowMove ();
			}
		}
	}

	return CBCGDialog::PreTranslateMessage(pMsg);
}
