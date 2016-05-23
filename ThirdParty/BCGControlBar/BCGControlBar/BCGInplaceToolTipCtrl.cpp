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

// BCGPInplaceToolTipCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGControlBar.h"
#include "BCGInplaceToolTipCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGInplaceToolTipCtrl

CString CBCGInplaceToolTipCtrl::m_strClassName;

IMPLEMENT_DYNAMIC(CBCGInplaceToolTipCtrl, CWnd)

CBCGInplaceToolTipCtrl::CBCGInplaceToolTipCtrl()
{
	m_rectLast.SetRectEmpty ();
	m_nTextMargin = 10;
	m_hFont	= NULL;
	m_pWndParent = NULL;
}

CBCGInplaceToolTipCtrl::~CBCGInplaceToolTipCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGInplaceToolTipCtrl, CWnd)
	//{{AFX_MSG_MAP(CBCGInplaceToolTipCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGInplaceToolTipCtrl message handlers

BOOL CBCGInplaceToolTipCtrl::Create (CWnd* pWndParent) 
{
	ASSERT_VALID (pWndParent);
	m_pWndParent = pWndParent;

	if (m_strClassName.IsEmpty ())
	{
		m_strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1));
	}  

	return CreateEx (0,
					m_strClassName, _T (""), WS_POPUP,
					0, 0, 0, 0,
					pWndParent->GetSafeHwnd (), (HMENU) NULL);
}

BOOL CBCGInplaceToolTipCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGInplaceToolTipCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect (rect);

	dc.FillSolidRect (&rect, ::GetSysColor (COLOR_INFOBK));
	dc.Draw3dRect (rect, GetSysColor (COLOR_INFOTEXT), GetSysColor (COLOR_INFOTEXT));

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (CFont::FromHandle (m_hFont));
	ASSERT (pPrevFont != NULL);

	dc.SetBkMode (TRANSPARENT);
	dc.SetTextColor(::GetSysColor (COLOR_INFOTEXT));

	rect.DeflateRect (m_nTextMargin, 0);
	dc.DrawText (m_strText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	dc.SelectObject (pPrevFont);
}
//*******************************************************************************************
void CBCGInplaceToolTipCtrl::Track (CRect rect, const CString& strText)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_rectLast == rect && m_strText == strText)
	{
		return;
	}

	ASSERT_VALID (m_pWndParent);

	m_rectLast = rect;
	m_strText = strText;

	CClientDC dc (this);

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (CFont::FromHandle (m_hFont));
	ASSERT (pPrevFont != NULL);

	int nTextWidth = dc.GetTextExtent (m_strText).cx + 2 * m_nTextMargin;

	dc.SelectObject (pPrevFont);

	if (m_pWndParent->GetExStyle () & WS_EX_LAYOUTRTL)
	{
		rect.left = rect.right - nTextWidth;
	}
	else
	{
		rect.right = rect.left + nTextWidth;
	}

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);

	CRect rectScreen;

	if (GetMonitorInfo (MonitorFromPoint (rect.TopLeft (), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.Width () > rectScreen.Width ())
	{
		rect.left = rectScreen.left;
		rect.right = rectScreen.right;
	}
	else if (rect.right > rectScreen.right)
	{
		rect.right = rectScreen.right;
		rect.left = rect.right - nTextWidth;
	}
	else if (rect.left < rectScreen.left)
	{
		rect.left = rectScreen.left;
		rect.right = rect.left + nTextWidth;
	}

	SetWindowPos (&wndTop, rect.left, rect.top, 
		rect.Width (), rect.Height (), SWP_NOACTIVATE);
  
    ShowWindow (SW_SHOWNOACTIVATE);
	Invalidate ();
	UpdateWindow ();

	SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_ARROW));
}
//*******************************************************************************************
void CBCGInplaceToolTipCtrl::Hide ()
{
	if (GetSafeHwnd () != NULL)
	{
		ShowWindow (SW_HIDE);
	}
}
//*******************************************************************************************
void CBCGInplaceToolTipCtrl::Deactivate ()
{
	m_strText.Empty ();
	m_rectLast.SetRectEmpty ();

	Hide ();
}
//*****************************************************************************
LRESULT CBCGInplaceToolTipCtrl::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD (lParam);

	m_hFont = (HFONT) wParam;

	if (bRedraw)
	{
		Invalidate ();
		UpdateWindow ();
	}

	return 0;
}
//***************************************************************************
BOOL CBCGInplaceToolTipCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_MOUSEFIRST &&
		pMsg->message <= WM_MOUSELAST)
	{
		if (pMsg->message != WM_MOUSEMOVE)
		{
			Hide ();
		}

		ASSERT_VALID (m_pWndParent);

		// the parent should receive the mouse message in its client coordinates
		CPoint pt (LOWORD (pMsg->lParam), HIWORD (pMsg->lParam));
		MapWindowPoints (m_pWndParent, &pt, 1);
		LPARAM lParam = MAKELPARAM (pt.x, pt.y);

		m_pWndParent->SendMessage (pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}
