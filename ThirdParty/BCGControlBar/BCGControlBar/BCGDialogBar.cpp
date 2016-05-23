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

// BCGDialogBar.cpp : implementation file
//

#include "stdafx.h"
#if _MSC_VER >= 1300
	#include <afxocc.h>
#else
	#include <../src/occimpl.h>
#endif

#ifndef BCG_NO_SIZINGBAR

#include "bcgcontrolbar.h"
#include "globals.h"
#include "BCGDialogBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGDialogBar

IMPLEMENT_SERIAL(CBCGDialogBar, CBCGSizingControlBar, VERSIONABLE_SCHEMA | 1)

CBCGDialogBar::CBCGDialogBar()
{
	m_bAllowSizing = FALSE;

#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = NULL;
	m_pOccDialogInfo = NULL;
#endif
}

CBCGDialogBar::~CBCGDialogBar()
{
//	DestroyWindow();    // avoid PostNcDestroy problems
}

/////////////////////////////////////////////////////////////////////////////
// CBCGDialogBar message handlers

//****************************************************************************************
CSize CBCGDialogBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if(m_bAllowSizing)
	{
		return CBCGSizingControlBar::CalcDynamicLayout(nLength, dwMode);
	}
	else
	{
		return CControlBar::CalcDynamicLayout(nLength, dwMode);
	}
}
//****************************************************************************************
CSize CBCGDialogBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	if (m_bAllowSizing)
	{
		return CBCGSizingControlBar::CalcFixedLayout(bStretch, bHorz);
	}

	CSize sizeResult = m_sizeDefault;

	if (bStretch) // if not docked stretch to fit
	{
		if (bHorz)
		{
			sizeResult.cx = 32767;
		}
		else
		{
			sizeResult.cy = 32767;
		}
	}

	// Jasc Begin
	// Adjust for our "non-client" space
	sizeResult.cx += m_nBorderSize * 2;
	sizeResult.cy += m_nBorderSize * 2;
	
	// We need to adjust for the gripper and for the separator line.
	switch (m_nDockBarID)
	{
	case AFX_IDW_DOCKBAR_TOP:
	case AFX_IDW_DOCKBAR_BOTTOM:
		sizeResult.cx += m_cyGripper;
		if (IsEdgeVisible (HTTOP) || IsEdgeVisible (HTBOTTOM))
		{
			sizeResult.cy += m_cxEdge;
		}
		break;
		
	case AFX_IDW_DOCKBAR_LEFT:
	case AFX_IDW_DOCKBAR_RIGHT:
		sizeResult.cy += m_cyGripper;
		if (IsEdgeVisible (HTRIGHT) || IsEdgeVisible (HTLEFT))
		{
			sizeResult.cx += m_cxEdge;
		}
		break;
		
	default:
		break;
	}
	
	CSize size = CControlBar::CalcFixedLayout (bStretch, bHorz);
	// Jasc End

	sizeResult.cx = max(sizeResult.cx, size.cx);
	sizeResult.cy = max(sizeResult.cy, size.cy);

	return sizeResult;
}
//****************************************************************************************
BOOL CBCGDialogBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper, 
						   UINT nIDTemplate, UINT nStyle, UINT nID)
{ 
	return Create(lpszWindowName, pParentWnd, bHasGripper, MAKEINTRESOURCE(nIDTemplate), nStyle, nID); 
}
//****************************************************************************************
BOOL CBCGDialogBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper, 
						   LPCTSTR lpszTemplateName, UINT nStyle, UINT nID)
{
	ASSERT(pParentWnd != NULL);
	ASSERT(lpszTemplateName != NULL);

	//------------------------------------------------------
    // cannot be both fixed and dynamic
    // (CBRS_SIZE_DYNAMIC is used for resizng when floating)
	//------------------------------------------------------
    ASSERT (!((nStyle & CBRS_SIZE_FIXED) &&
              (nStyle & CBRS_SIZE_DYNAMIC)));

	if (bHasGripper)
	{
		m_cyGripper = max (12, globalData.GetTextHeight ());
	}
	else
	{
		m_cyGripper = 0;
	}

	m_bAllowSizing = nStyle & CBRS_SIZE_DYNAMIC ? TRUE : FALSE;

	//------------------------------
	// allow chance to modify styles
	//------------------------------
	m_dwStyle = (nStyle & CBRS_ALL);

	CREATESTRUCT cs;
	memset(&cs, 0, sizeof(cs));
	cs.lpszClass = AFX_WNDCONTROLBAR;
	cs.lpszName = lpszWindowName;
	cs.style = (DWORD)nStyle | WS_CHILD;
	cs.hMenu = (HMENU)(UINT_PTR) nID;
	cs.hInstance = AfxGetInstanceHandle();
	cs.hwndParent = pParentWnd->GetSafeHwnd();

	if (!PreCreateWindow(cs))
	{
		return FALSE;
	}

#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = lpszTemplateName;
#endif

	//----------------------------
	// initialize common controls
	//----------------------------
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));
	AfxDeferRegisterClass(AFX_WNDCOMMCTLSNEW_REG);

	//--------------------------
	// create a modeless dialog
	//--------------------------
	if (!CreateDlg (lpszTemplateName, pParentWnd))
	{
		TRACE(_T("Can't create dialog: %s\n"), lpszTemplateName);
		return FALSE;
	}

#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = NULL;
#endif

#pragma warning (disable : 4311)
	SetClassLongPtr (m_hWnd, GCLP_HBRBACKGROUND, (long)::GetSysColorBrush(COLOR_BTNFACE));
#pragma warning (default : 4311)

	//----------------------------------------------
	// dialog template MUST specify that the dialog
	// is an invisible child window
	//----------------------------------------------
	SetDlgCtrlID(nID);

	CRect rect;
	GetWindowRect(&rect);
	m_sizeDefault = rect.Size();    // set fixed size

    m_szHorz = m_sizeDefault; // set the size members
    m_szVert = m_sizeDefault;
    m_szFloat = m_sizeDefault;

	//-----------------------
	// force WS_CLIPSIBLINGS
	//-----------------------
	ModifyStyle(0, WS_CLIPSIBLINGS);

	if (!ExecuteDlgInit(lpszTemplateName))
		return FALSE;

	//--------------------------------------------------------
	// force the size to zero - resizing bar will occur later
	//--------------------------------------------------------
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);

	if (lpszWindowName != NULL)
	{
		SetWindowText (lpszWindowName);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGDialogBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

BEGIN_MESSAGE_MAP(CBCGDialogBar, CBCGSizingControlBar)
	//{{AFX_MSG_MAP(CBCGDialogBar)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
#ifndef _AFX_NO_OCC_SUPPORT
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
#endif //!_AFX_NO_OCC_SUPPORT
END_MESSAGE_MAP()

#ifndef _AFX_NO_OCC_SUPPORT

LRESULT CBCGDialogBar::HandleInitDialog(WPARAM, LPARAM)
{
	Default();  // allow default to initialize first (common dialogs/etc)

	// create OLE controls
	COccManager* pOccManager = afxOccManager;
	if ((pOccManager != NULL) && (m_pOccDialogInfo != NULL))
	{
		if (!pOccManager->CreateDlgControls(this, m_lpszTemplateName,
			m_pOccDialogInfo))
		{
			TRACE (_T("Warning: CreateDlgControls failed during dialog bar init.\n"));
			return FALSE;
		}
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGDialogBar::SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo)
{
	m_pOccDialogInfo = pOccDialogInfo;
	return TRUE;
}

#endif //!_AFX_NO_OCC_SUPPORT

BOOL CBCGDialogBar::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (rectClient);

	pDC->FillRect (rectClient, &globalData.brBtnFace);
	return TRUE;
}

#endif // BCG_NO_SIZINGBAR
