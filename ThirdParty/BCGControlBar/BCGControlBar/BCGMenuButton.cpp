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

// BCGMenuButton.cpp : implementation file
//

#include "stdafx.h"
#include "MenuImages.h"
#include "bcgcontrolbar.h"
#include "BCGMenuButton.h"
#include "BCGContextMenuManager.h"
#include "BCGPopupMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nImageHorzMargin = 10;

/////////////////////////////////////////////////////////////////////////////
// CBCGMenuButton

IMPLEMENT_DYNAMIC(CBCGMenuButton, CBCGButton)

CBCGMenuButton::CBCGMenuButton()
{
	m_bRightArrow = FALSE;
	m_hMenu = NULL;
	m_nMenuResult = 0;
	m_bMenuIsActive = FALSE;
	m_bStayPressed = FALSE;
	m_bOSMenu = TRUE;
	m_bDefaultClick = FALSE;
	m_bClickOnMenu = FALSE;
	m_bRightAlign = FALSE;
}
//*****************************************************************************************
CBCGMenuButton::~CBCGMenuButton()
{
}


BEGIN_MESSAGE_MAP(CBCGMenuButton, CBCGButton)
	//{{AFX_MSG_MAP(CBCGMenuButton)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGMenuButton message handlers

CSize CBCGMenuButton::SizeToContent (BOOL bCalcOnly)
{
	CSize size = CBCGButton::SizeToContent (FALSE);
	size.cx += CMenuImages::Size ().cx;

	if (!bCalcOnly)
	{
		SetWindowPos (NULL, -1, -1, size.cx, size.cy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return size;
}
//*****************************************************************************************
void CBCGMenuButton::OnDraw (CDC* pDC, const CRect& rect, UINT uiState)
{
	ASSERT_VALID (pDC);

	CSize sizeArrow = CMenuImages::Size ();

	CRect rectParent = rect;
	rectParent.right -= sizeArrow.cx + nImageHorzMargin;

	CBCGButton::OnDraw (pDC, rectParent, uiState);

	int iImage;
	if (!m_bRightArrow)
	{
		iImage = (uiState & ODS_DISABLED) ? CMenuImages::IdArowDownLargeDsbl : CMenuImages::IdArowDownLarge;
	}
	else
	{
		iImage = (uiState & ODS_DISABLED) ? CMenuImages::IdArowLeftLargeDsbl : CMenuImages::IdArowLeftLarge;
	}

	CRect rectArrow = rect;

	CString strText;
	GetWindowText (strText);

	if (m_Image.GetCount () != 0 || !strText.IsEmpty ())
	{
		rectArrow.left = rectParent.right;
	}

	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rectArrow);

	if (m_bDefaultClick)
	{
		//----------------
		// Draw separator:
		//----------------
		CRect rectSeparator = rectArrow;
		rectSeparator.right = rectSeparator.left + 2;
		rectSeparator.DeflateRect (0, 2);

		if (!m_bWinXPTheme || m_bDontUseWinXPTheme)
		{
			rectSeparator.left += m_sizePushOffset.cx;
			rectSeparator.top += m_sizePushOffset.cy;
		}

		pDC->Draw3dRect (rectSeparator, globalData.clrBtnDkShadow, globalData.clrBtnHilite);
	}

}
//*****************************************************************************************
void CBCGMenuButton::OnShowMenu () 
{
	if (m_hMenu == NULL || m_bMenuIsActive)
	{
		return;
	}

	CRect rectWindow;
	GetWindowRect (rectWindow);

	int x, y;

	if (m_bRightArrow)
	{
		x = rectWindow.right;
		y = rectWindow.top;
	}
	else if (m_bRightAlign)
	{
		x = rectWindow.right;
		y = rectWindow.bottom;
	}
	else
	{
		x = rectWindow.left;
		y = rectWindow.bottom;
	}

	if (m_bStayPressed)
	{
		m_bPushed = TRUE;
		m_bHighlighted = TRUE;
	}

	m_bMenuIsActive = TRUE;
	Invalidate ();

	if (!m_bOSMenu && g_pContextMenuManager != NULL)
	{
		m_nMenuResult = g_pContextMenuManager->TrackPopupMenu (
			m_hMenu, x, y, this, m_bRightAlign && !m_bRightArrow);
		SetFocus ();
	}
	else
	{
		UINT nAlign = m_bRightAlign && !m_bRightArrow ? TPM_RIGHTALIGN : TPM_LEFTALIGN;

		m_nMenuResult = ::TrackPopupMenu (m_hMenu, 
			 nAlign | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
			x, y, 0, GetSafeHwnd (), NULL);
	}

	if (m_nMenuResult != 0)
	{
		//-------------------------------------------------------
		// Trigger mouse up event (to button click notification):
		//-------------------------------------------------------
		CWnd* pParent = GetParent ();
		if (pParent != NULL)
		{
			pParent->SendMessage (	WM_COMMAND,
									MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
									(LPARAM) m_hWnd);
		}
	}

	m_bPushed = FALSE;
	m_bHighlighted = FALSE;
	m_bMenuIsActive = FALSE;
	
	Invalidate ();
	UpdateWindow ();

	if (m_bCaptured)
	{
		ReleaseCapture ();
		m_bCaptured = FALSE;
	}
}
//*****************************************************************************************
void CBCGMenuButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE || nChar == VK_DOWN)
	{
		m_bClickOnMenu = TRUE;
		OnShowMenu ();
		return;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGMenuButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bMenuIsActive)
	{
		Default ();
		return;
	}

	m_bClickOnMenu = TRUE;

	if (m_bDefaultClick)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		CRect rectArrow = rectClient;
		rectArrow.left = rectArrow.right - CMenuImages::Size ().cx - nImageHorzMargin;

		if (!rectArrow.PtInRect (point))
		{
			m_bClickOnMenu = FALSE;
			m_nMenuResult = 0;
			CBCGButton::OnLButtonDown (nFlags, point);
			return;
		}
	}

	SetFocus ();
	OnShowMenu ();
}
//*****************************************************************************************
UINT CBCGMenuButton::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}
//****************************************************************************************
void CBCGMenuButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bStayPressed && m_bMenuIsActive && m_bPushed)
	{
		m_bClickiedInside = FALSE;

		CButton::OnLButtonUp(nFlags, point);

		if (m_bCaptured)
		{
			ReleaseCapture ();
			m_bCaptured = FALSE;
		}
	}
	else if (!m_bClickOnMenu)
	{
		CBCGButton::OnLButtonUp(nFlags, point);
	}
}
//***************************************************************************************
void CBCGMenuButton::OnKillFocus(CWnd* pNewWnd) 
{
	if (m_bStayPressed && m_bMenuIsActive && m_bPushed)
	{
		CButton::OnKillFocus(pNewWnd);
		
		if (m_bCaptured)
		{
			ReleaseCapture ();
			m_bCaptured = FALSE;
		}
		
		m_bClickiedInside = FALSE;
		m_bHover = FALSE;
	}
	else
	{
		CBCGButton::OnKillFocus(pNewWnd);
	}
}
//***************************************************************************************
BOOL CBCGMenuButton::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN &&
		CBCGPopupMenu::GetActiveMenu () == NULL)
	{
		m_bClickOnMenu = TRUE;
		OnShowMenu ();
		return TRUE;
	}
	
	return CBCGButton::PreTranslateMessage(pMsg);
}
//***************************************************************************************
void CBCGMenuButton::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (!m_bMenuIsActive)
	{
		CBCGButton::OnLButtonDblClk(nFlags, point);
	}
}
