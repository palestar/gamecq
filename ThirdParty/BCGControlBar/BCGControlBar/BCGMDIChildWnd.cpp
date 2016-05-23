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

// BCGMDIChildWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGMDIFrameWnd.h"
#include "BCGMDIChildWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGMDIChildWnd

IMPLEMENT_DYNCREATE(CBCGMDIChildWnd, CMDIChildWnd)

CBCGMDIChildWnd::CBCGMDIChildWnd()
{
	m_pMDIFrame = NULL;
	m_bToBeDestroyed = FALSE;
	m_rectOriginal.SetRectEmpty ();
}

CBCGMDIChildWnd::~CBCGMDIChildWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGMDIChildWnd, CMDIChildWnd)
	//{{AFX_MSG_MAP(CBCGMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT,OnSetText)
	ON_MESSAGE(WM_SETICON,OnSetIcon)
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGMDIChildWnd message handlers

BOOL CBCGMDIChildWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST &&
		m_pMDIFrame != NULL &&
		m_pMDIFrame->GetActivePopup () != NULL)
	{
		// Don't process accelerators if popup window is active
		return FALSE;
	}

	return CMDIChildWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************
int CBCGMDIChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if ((GetStyle () & WS_SYSMENU) == 0)
	{
		GetParent ()->SetRedraw (FALSE);

		m_rectOriginal = CRect (CPoint (lpCreateStruct->x, lpCreateStruct->y),
			CSize (lpCreateStruct->cx, lpCreateStruct->cy));

		SetWindowPos (NULL, -1, -1, INT_MAX, INT_MAX,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		ShowWindow (SW_SHOWMAXIMIZED);

		GetParent ()->SetRedraw (TRUE);
	}

	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pMDIFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, GetMDIFrame ());
	ASSERT_VALID (m_pMDIFrame);
	
	return 0;
}
//*************************************************************************************
void CBCGMDIChildWnd::DockControlBarLeftOf(CControlBar* pBar, CControlBar* pLeftOf)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_VALID (pLeftOf);

	CRect rect;
	DWORD dw;
	UINT n;
	
	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout(TRUE);
	
	pLeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,1);
	dw=pLeftOf->GetBarStyle();

	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
	
	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. By calculating a rectangle, we
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar (pBar,n,&rect);
}
//*************************************************************************************
void CBCGMDIChildWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (bActivate && m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.SetActiveTab (pActivateWnd->GetSafeHwnd ());
	}
}
//*************************************************************************************
LRESULT CBCGMDIChildWnd::OnSetText(WPARAM,LPARAM)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs (TRUE);
	}

	return lRes;
}
//*************************************************************************************
LRESULT CBCGMDIChildWnd::OnSetIcon(WPARAM,LPARAM)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs ();
	}

	return lRes;
}
//*************************************************************************************
CString CBCGMDIChildWnd::GetFrameText () const
{
	ASSERT_VALID (this);

	CString strText;
	GetWindowText (strText);

	return strText;
}
//*************************************************************************************
HICON CBCGMDIChildWnd::GetFrameIcon () const
{
	ASSERT_VALID (this);

	HICON hIcon = GetIcon (FALSE);
	if (hIcon == NULL)
	{
		hIcon = (HICON) (LONG_PTR) GetClassLongPtr (GetSafeHwnd (), GCLP_HICONSM);
	}

	return hIcon;
}
//*************************************************************************************
void CBCGMDIChildWnd::OnUpdateFrameTitle (BOOL bAddToTitle)
{
	CMDIChildWnd::OnUpdateFrameTitle (bAddToTitle);

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs ();
	}
}
//*************************************************************************************
void CBCGMDIChildWnd::ActivateFrame(int nCmdShow) 
{
    if ((GetStyle () & WS_SYSMENU) == 0)
    {
        nCmdShow = SW_SHOWMAXIMIZED;
    }

	CMDIChildWnd::ActivateFrame(nCmdShow);
}
//*********************************************************************************
void CBCGMDIChildWnd::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CMDIChildWnd::OnStyleChanged (nStyleType, lpStyleStruct);

	BOOL bWasSysMenu = (lpStyleStruct->styleOld & WS_SYSMENU);
	BOOL bIsSysMenu = (lpStyleStruct->styleNew & WS_SYSMENU);

	if (bWasSysMenu == bIsSysMenu)
	{
		return;
	}

	if (bWasSysMenu)
	{
		if ((GetStyle () & WS_MAXIMIZE) == 0 &&
			(GetStyle () & WS_MINIMIZE) == 0)
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);

			GetParent()->ScreenToClient (&rectWindow);

			m_rectOriginal = rectWindow;
		}

		SetWindowPos (NULL, -1, -1, INT_MAX, INT_MAX,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		ShowWindow (SW_SHOWMAXIMIZED);
	}
	else
	{
		SetWindowPos (NULL, m_rectOriginal.left, m_rectOriginal.top, 
			m_rectOriginal.Width (), m_rectOriginal.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}
}
