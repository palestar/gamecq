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

// BCGDockContext.cpp:
//

#include "stdafx.h"

#pragma warning (disable : 4706)
#include "multimon.h"
#pragma warning (default : 4706)

#include "bcgcontrolbar.h"
#include "BCGDockContext.h"
#include "BCGDockBar.h"
#include "BCGSizingControlBar.h"
#include "bcgtoolbar.h"
#include "BCGPopupMenu.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"
#include "MenuImages.h"
#include "BCGToolbarMenuButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGDockContext
//
// this class is needed for 8-dim stretching of floated controlbars

CBCGDockContext::CBCGDockContext(CControlBar* pBar) : 
	CDockContext (pBar)
{
}

CBCGDockContext::~CBCGDockContext()
{
}

// Aliases duplicated from CDockContext 

#define m_rectRequestedSize     m_rectDragHorz
#define m_rectActualSize        m_rectDragVert
#define m_rectActualFrameSize   m_rectFrameDragHorz
#define m_rectFrameBorders      m_rectFrameDragVert


// Duplicated from CDockContext in order to call Stretch()
// because it's not virtual
void CBCGDockContext::StartResize(int nHitTest, CPoint pt)
{
	ASSERT_VALID(m_pBar);
	ASSERT(m_pBar->m_dwStyle & CBRS_SIZE_DYNAMIC);
	m_bDragging = FALSE;

	InitLoop();

	// GetWindowRect returns screen coordinates(not mirrored)
	// So if the desktop is mirrored then turn off mirroring
	// for the desktop dc so that we draw correct focus rect

#if _MSC_VER < 1300
	if (m_pDC->GetLayout() & LAYOUT_RTL)
		m_pDC->SetLayout(m_pDC->GetLayout() & ~LAYOUT_RTL);
#endif

	// get true bar size (including borders)
	CRect rect;
	m_pBar->GetWindowRect(rect);
	m_ptLast = pt;
	m_nHitTest = nHitTest;

	CSize size = m_pBar->CalcDynamicLayout(0, LM_HORZ | LM_MRUWIDTH);
	m_rectRequestedSize = CRect(rect.TopLeft(), size);
	m_rectActualSize = CRect(rect.TopLeft(), size);
	m_rectActualFrameSize = CRect(rect.TopLeft(), size);

	// calculate frame rectangle
	CMiniFrameWnd::CalcBorders(&m_rectActualFrameSize);
	m_rectActualFrameSize.InflateRect(-afxData.cxBorder2, -afxData.cyBorder2);

	m_rectFrameBorders = CRect(CPoint(0,0),
		m_rectActualFrameSize.Size() - m_rectActualSize.Size());

	// initialize tracking state and enter tracking loop
	m_dwOverDockStyle = 0;
	Stretch(pt);   // call it here to handle special keys
	Track();
}


// Mostly duplicated from CDockContext
// changes are marked
void CBCGDockContext::Stretch(CPoint pt)
{
	CPoint ptOffset = pt - m_ptLast;

	//<---------------------------------------------------------->
	//<ET: Horizontally and vertically independently------------->
    CSize size;

	// horizontally
	if (m_nHitTest == HTLEFT 
		|| m_nHitTest == HTTOPLEFT 
		|| m_nHitTest == HTBOTTOMLEFT )
	{
		m_rectRequestedSize.left += ptOffset.x;
        size = m_pBar->CalcDynamicLayout(m_rectRequestedSize.Width(), LM_HORZ);
	}
	if (m_nHitTest == HTRIGHT
		|| m_nHitTest == HTTOPRIGHT 
		|| m_nHitTest == HTBOTTOMRIGHT )
	{
		m_rectRequestedSize.right += ptOffset.x;
        size = m_pBar->CalcDynamicLayout(m_rectRequestedSize.Width(), LM_HORZ);
	}

	// vertically
	if (m_nHitTest == HTTOP 
		|| m_nHitTest == HTTOPLEFT 
		|| m_nHitTest == HTTOPRIGHT )
	{
		m_rectRequestedSize.top += ptOffset.y;
        size = m_pBar->CalcDynamicLayout(m_rectRequestedSize.Height(), LM_HORZ|LM_LENGTHY);
	}
	if (m_nHitTest == HTBOTTOM
		|| m_nHitTest == HTBOTTOMRIGHT 
		|| m_nHitTest == HTBOTTOMLEFT )
	{
		m_rectRequestedSize.bottom += ptOffset.y;
        size = m_pBar->CalcDynamicLayout(m_rectRequestedSize.Height(), LM_HORZ|LM_LENGTHY);
	}
	//<---------------------------------------------------------->
	//<---------------------------------------------------------->


	CRect rectDesk;
	HWND hWndDesk = ::GetDesktopWindow();
	::GetWindowRect(hWndDesk, &rectDesk);
	CRect rectTemp = m_rectActualFrameSize;

	//<---------------------------------------------------------->
	//<ET: cases for HT{X}{Y}------------------------------------>
    // left, top, top left
    if (m_nHitTest == HTLEFT || m_nHitTest == HTTOP || m_nHitTest == HTTOPLEFT)
    {
        rectTemp.left = rectTemp.right -
            (size.cx + m_rectFrameBorders.Width());
        rectTemp.top = rectTemp.bottom -
            (size.cy + m_rectFrameBorders.Height());
        CRect rect;
        if (rect.IntersectRect(rectDesk, rectTemp))
        {
            m_rectActualSize.left = m_rectActualSize.right - size.cx;
            m_rectActualSize.top = m_rectActualSize.bottom - size.cy;
            m_rectActualFrameSize.left = rectTemp.left;
            m_rectActualFrameSize.top = rectTemp.top;
        }
    } 
	else if (m_nHitTest == HTTOPRIGHT) // top right
    {
        rectTemp.top = rectTemp.bottom -
            (size.cy + m_rectFrameBorders.Height());
        rectTemp.right = rectTemp.left +
            (size.cx + m_rectFrameBorders.Width());
        CRect rect;
        if (rect.IntersectRect(rectDesk, rectTemp))
        {
            m_rectActualSize.left = m_rectActualSize.right - size.cx;
            m_rectActualSize.bottom = m_rectActualSize.top + size.cy;
            m_rectActualFrameSize.right = rectTemp.right;
            m_rectActualFrameSize.top = rectTemp.top;
        }
    } 
	else if (m_nHitTest == HTBOTTOMLEFT) // bottom left
    {
        rectTemp.bottom = rectTemp.top +
            (size.cy + m_rectFrameBorders.Height());
        rectTemp.left = rectTemp.right -
            (size.cx + m_rectFrameBorders.Width());
        CRect rect;
        if (rect.IntersectRect(rectDesk, rectTemp))
        {
            m_rectActualSize.right = m_rectActualSize.left + size.cx;
            m_rectActualSize.top = m_rectActualSize.bottom - size.cy;
            m_rectActualFrameSize.left = rectTemp.left;
            m_rectActualFrameSize.bottom = rectTemp.bottom;
        }
    } 
	else // bottom, right, bottom right
    {
        rectTemp.right = rectTemp.left +
            (size.cx + m_rectFrameBorders.Width());
        rectTemp.bottom = rectTemp.top +
            (size.cy + m_rectFrameBorders.Height());
        CRect rect;
        if (rect.IntersectRect(rectDesk, rectTemp))
        {
            m_rectActualSize.right = m_rectActualSize.left + size.cx;
            m_rectActualSize.bottom = m_rectActualSize.top + size.cy;
            m_rectActualFrameSize.right = rectTemp.right;
            m_rectActualFrameSize.bottom = rectTemp.bottom;
        }
    }
	//<---------------------------------------------------------->
	//<---------------------------------------------------------->

	
	m_ptLast = pt;

	// update feedback
	DrawFocusRect();
}

// Duplicated from CDockContext in order to call Stretch()
// because it's not virtual
BOOL CBCGDockContext::Track()
{
	// don't handle if capture already set
	if (::GetCapture() != NULL)
		return FALSE;

	// set capture to the window which received this message
	m_pBar->SetCapture();
	ASSERT(m_pBar == CWnd::GetCapture());

	// get messages until capture lost or cancelled/accepted
	while (CWnd::GetCapture() == m_pBar)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage ((int) msg.wParam);
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
			if (m_bDragging)
			{
				EndDrag();
			}
			else
				EndResize();
			return TRUE;
		case WM_MOUSEMOVE:
			if (m_bDragging)
				Move(msg.pt);
			else
				Stretch(msg.pt);
			break;
		case WM_KEYUP:
			if (m_bDragging)
				OnKey((int)msg.wParam, FALSE);
			break;
		case WM_KEYDOWN:
			if (m_bDragging)
				OnKey((int)msg.wParam, TRUE);
			if (msg.wParam == VK_ESCAPE)
			{
				CancelLoop();
				return FALSE;
			}
			break;
		case WM_RBUTTONDOWN:
			CancelLoop();
			return FALSE;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

	CancelLoop();

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////
// CBCGMiniDockFrameWnd

IMPLEMENT_DYNCREATE(CBCGMiniDockFrameWnd, CMiniDockFrameWnd)

CBCGMiniDockFrameWnd::CBCGMiniDockFrameWnd()
{
#ifndef BCG_NO_SIZINGBAR

	m_nRollingType = BCG_ROLLUP_NONE;
	m_bIsRolled = 0;

	m_rectRollupBox.SetRectEmpty ();

	m_rectCaptionDynamic.left=-1;
	m_rectCaptionDynamic.right=-1;
#endif BCG_NO_SIZINGBAR
}

CBCGMiniDockFrameWnd::~CBCGMiniDockFrameWnd()
{
}

BOOL CBCGMiniDockFrameWnd::Create(CWnd* pParent, DWORD dwBarStyle)
{
	if (!CMiniDockFrameWnd::Create(pParent, dwBarStyle))
		return FALSE;

	ModifyStyle (MFS_4THICKFRAME, 0);
	ModifyStyleEx(0,WS_EX_TOOLWINDOW);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CBCGMiniDockFrameWnd, CMiniDockFrameWnd)
	//{{AFX_MSG_MAP(CBCGMiniDockFrameWnd)
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCMOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGMiniDockFrameWnd::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	CPoint p(point);
	CRect r;
	GetWindowRect(r);
	p.Offset(-r.left, -r.top);

	if (m_rectRollupBox.PtInRect (p))
	{
		Roll ();
	}
	else if (nHitTest == HTCAPTION)
	{
		// special activation for floating toolbars
		ActivateTopParent();
		BringWindowToTop ();

		// initiate toolbar drag for non-CBRS_FLOAT_MULTI toolbars
		if ((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0)
		{
			int nPos = 1;
			CControlBar* pBar = NULL;
			while(pBar == NULL && nPos < m_wndDockBar.m_arrBars.GetSize())
				pBar = reinterpret_cast<CBCGDockBar&>(m_wndDockBar).GetDockedControlBar(nPos++);

			ASSERT(pBar != NULL);
			ASSERT_KINDOF(CControlBar, pBar);
			ASSERT(pBar->m_pDockContext != NULL);
			pBar->m_pDockContext->StartDrag(point);
			return;
		}
	}
	else if (nHitTest >= HTSIZEFIRST && nHitTest <= HTSIZELAST)
	{
		// special activation for floating toolbars
		ActivateTopParent();

		int nPos = 1;
		CControlBar* pBar = NULL;
		while(pBar == NULL && nPos < m_wndDockBar.m_arrBars.GetSize())
			pBar = reinterpret_cast<CBCGDockBar&>(m_wndDockBar).GetDockedControlBar(nPos++);

		ASSERT(pBar != NULL);
		ASSERT_KINDOF(CControlBar, pBar);
		ASSERT(pBar->m_pDockContext != NULL);

		// CBRS_SIZE_DYNAMIC toolbars cannot have the CBRS_FLOAT_MULTI style
		ASSERT((m_wndDockBar.m_dwStyle & CBRS_FLOAT_MULTI) == 0);

#ifndef BCG_NO_SIZINGBAR
		if (!pBar->IsKindOf(RUNTIME_CLASS(CBCGSizingControlBar))) 
		{
			CMiniDockFrameWnd::OnNcLButtonDown(nHitTest,point);
		}
		else 
#endif // BCG_NO_SIZINGBAR
		{
			pBar->m_pDockContext->StartResize(nHitTest, point);
		}
		return;
	}
	CMiniFrameWnd::OnNcLButtonDown(nHitTest, point);
}
//*********************************************************************************
void CBCGMiniDockFrameWnd::OnNcRButtonDown(UINT /*nHitTest*/, CPoint point)
{
	if (CBCGToolBar::IsCustomizeMode ())
	{
		return;
	}

    CWnd* pParentWnd = GetParent ();
	ASSERT_VALID (pParentWnd);

    if (pParentWnd->IsKindOf (RUNTIME_CLASS (CFrameWnd)))
    {
        ((CFrameWnd*)pParentWnd)->SendMessage(BCGM_TOOLBARMENU,
					(WPARAM) GetSafeHwnd (),
					MAKELPARAM (point.x, point.y));
    }
    else
    {
        TRACE(_T("CBCGMiniDockFrameWnd parent is not a CFrameWnd\n"));
        ASSERT (FALSE);
    }
}
//*********************************************************************************
BOOL CBCGMiniDockFrameWnd::StartTearOff (CBCGPopupMenu* pMenu)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMenu);

	HWND hwndMenu = pMenu->GetSafeHwnd ();
	pMenu->ShowWindow (SW_HIDE);

	//----------------------
	// Redraw parent button:
	//----------------------
	CBCGToolbarMenuButton* pParentBtn = pMenu->GetParentButton ();
	if (pParentBtn != NULL)
	{
		CWnd* pWndParent = pParentBtn->GetParentWnd ();
		if (pWndParent != NULL)
		{
			CRect rectBtn = pParentBtn->Rect ();
			rectBtn.InflateRect (4, 4);

			pWndParent->InvalidateRect (rectBtn);
			pWndParent->UpdateWindow ();
		}
	}

	int nPos = 1;
	CControlBar* pBar = NULL;
	while(pBar == NULL && nPos < m_wndDockBar.m_arrBars.GetSize())
		pBar = reinterpret_cast<CBCGDockBar&>(m_wndDockBar).GetDockedControlBar(nPos++);

	ASSERT(pBar != NULL);
	ASSERT_KINDOF(CControlBar, pBar);

	//-----------------------------
	// Disable docking during drag:
	//-----------------------------
	DWORD dwSavedDockStyle = pBar->m_dwDockStyle;
	pBar->EnableDocking (0);

	// handle pending WM_PAINT messages
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			return FALSE;
		DispatchMessage(&msg);
	}

	// don't handle if capture already set
	if (::GetCapture() != NULL)
		return FALSE;

	// set capture to the window which received this message
	pBar->SetCapture();
	ASSERT(pBar == CWnd::GetCapture());

	BOOL bSuccess = FALSE;
	BOOL bStop = FALSE;

	// Move cirsor to the middle of the caption
	CRect rectFrame;
	GetWindowRect (rectFrame);

	int x = (rectFrame.left + rectFrame.right) / 2;
	int xOffset = x - rectFrame.left;

	int y = rectFrame.top + 5;
	int yOffset = y - rectFrame.top;

	::SetCursorPos (x, y);

	// get messages until capture lost or cancelled/accepted
	while (!bStop && CWnd::GetCapture() == pBar)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage ((int) msg.wParam);
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
			bStop = TRUE;
			bSuccess = TRUE;
			break;

		case WM_MOUSEMOVE:
			{
				SetWindowPos (NULL, 
					msg.pt.x - xOffset, msg.pt.y - yOffset,
					-1, -1,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			}
			break;

		case WM_KEYDOWN:
			if (msg.wParam == VK_ESCAPE)
			{
				bStop = TRUE;
			}
			break;

		case WM_RBUTTONDOWN:
			bStop = TRUE;
			break;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

	ReleaseCapture();
	pBar->EnableDocking (dwSavedDockStyle);

	if (::IsWindow (hwndMenu))
	{
		if (bSuccess)
		{
			pMenu->SendMessage (WM_CLOSE);
			if (BCGGetTopLevelFrame (this) != NULL)
			{
				BCGGetTopLevelFrame (this)->SetFocus ();
			}
		}
		else
		{
			pMenu->ShowWindow (SW_SHOWNOACTIVATE);
		}
	}

	if (!bSuccess)
	{
		CFrameWnd* pWndMain = BCGGetTopLevelFrame (this);
		if (pWndMain != NULL)
		{
			CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pWndMain);
			if (pMainFrame != NULL)
			{
				pMainFrame->m_Impl.RemoveTearOffToolbar (pBar);
			}
			else	// Maybe, SDI frame...
			{
				CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pWndMain);
				if (pFrame != NULL)
				{
					pFrame->m_Impl.RemoveTearOffToolbar (pBar);
				}
				else	// Maybe, OLE frame...
				{
					CBCGOleIPFrameWnd* pOleFrame = 
						DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pWndMain);
					if (pOleFrame != NULL)
					{
						pOleFrame->m_Impl.RemoveTearOffToolbar (pBar);
					}
					else
					{
						CBCGOleDocIPFrameWnd* pOleDocFrame = 
							DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pWndMain);
						if (pOleDocFrame != NULL)
						{
							pOleDocFrame->m_Impl.RemoveTearOffToolbar (pBar);
						}
					}
				}
			}
		}

		pBar->DestroyWindow ();
		delete pBar;
	}

	return bSuccess;
}
//**************************************************************************************
void CBCGMiniDockFrameWnd::OnNcPaint() 
{
	CMiniDockFrameWnd::OnNcPaint();

#ifndef BCG_NO_SIZINGBAR

	if (m_nRollingType==BCG_ROLLUP_NONE)
	{
		return;
	}

	CWindowDC dcWin (this);



	dcWin.DrawFrameControl (m_rectRollupBox, DFC_BUTTON, DFCS_BUTTONPUSH);

	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
	if(pSizingBar==NULL) return;

	if(pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMIC_ON || 
		pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_ON){
		CMenuImages::Draw (&dcWin, CMenuImages::IdArowRight , m_rectRollupBox);
	}else{
		CMenuImages::Draw (&dcWin, 
			m_bIsRolled ? CMenuImages::IdArowDown : CMenuImages::IdArowUp , m_rectRollupBox);
	}

#endif // BCG_NO_SIZINGBAR
}
//**************************************************************************************
BOOL CBCGMiniDockFrameWnd::OnNcActivate(BOOL bActive) 
{
	BOOL rc = CMiniDockFrameWnd::OnNcActivate(bActive);
	CMiniDockFrameWnd::OnNcActivate(bActive);
	SendMessage(WM_NCPAINT);	// paint non-client area (frame too)

	if (bActive)
	{
		CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
		if (pSizingBar != NULL)
		{
			ASSERT_VALID (pSizingBar);
			pSizingBar->RedrawWindow (NULL, NULL,
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	return rc;
}
//**************************************************************************************
void CBCGMiniDockFrameWnd::Roll()
{
#ifndef BCG_NO_SIZINGBAR

	if (m_nRollingType!=BCG_ROLLUP_NONE)
	{
// now use the helper...
		CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
		if (pSizingBar == NULL ||
			!pSizingBar->m_nRollingType)
		{
			ASSERT (FALSE);
			return;
		}

		if(pSizingBar->m_nRollingType==BCG_ROLLUP_DYNAMIC_ON){
			pSizingBar->m_nRollingType=BCG_ROLLUP_DYNAMIC_OFF;
			pSizingBar->m_bIsRolled = 0;
			SendMessage(WM_NCPAINT, NULL, NULL);
		}else if(pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMIC_OFF){
			pSizingBar->m_nRollingType=BCG_ROLLUP_DYNAMIC_ON;
			SendMessage(WM_NCPAINT, NULL, NULL);
		}else if(pSizingBar->m_nRollingType==BCG_ROLLUP_DYNAMICSIZED_ON){
			pSizingBar->m_nRollingType=BCG_ROLLUP_DYNAMICSIZED_OFF;
			pSizingBar->m_bIsRolled = 0;
			SendMessage(WM_NCPAINT, NULL, NULL);
		}else if(pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_OFF){
			pSizingBar->m_nRollingType=BCG_ROLLUP_DYNAMICSIZED_ON;
			SendMessage(WM_NCPAINT, NULL, NULL);
		}else{
			pSizingBar->m_bIsRolled = !pSizingBar->m_bIsRolled;
		}
		m_bIsRolled = pSizingBar->m_bIsRolled;
		m_nRollingType=pSizingBar->m_nRollingType;

		DelayRecalcLayout();
	}

#endif // BCG_NO_SIZINGBAR
}
//**************************************************************************************
void CBCGMiniDockFrameWnd::OnSize(UINT nType, int cx, int cy) 
{
	CMiniDockFrameWnd::OnSize(nType, cx, cy);

	m_nRollingType = FALSE;
	m_rectRollupBox.SetRectEmpty ();
	
#ifndef BCG_NO_SIZINGBAR

	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
	if (pSizingBar == NULL ||
		pSizingBar->m_nRollingType==BCG_ROLLUP_NONE)
	{
		return;
	}

	m_nRollingType = pSizingBar->m_nRollingType;
	m_bIsRolled = pSizingBar->m_bIsRolled;

	CRect rectCaption;
	DWORD dwStyle = GetStyle();

	CSize szFrame = (dwStyle & WS_THICKFRAME) ?
		CSize(GetSystemMetrics(SM_CXSIZEFRAME),
			   GetSystemMetrics(SM_CYSIZEFRAME)) :
		CSize(GetSystemMetrics(SM_CXFIXEDFRAME),
				GetSystemMetrics(SM_CYFIXEDFRAME));

	GetWindowRect (&rectCaption);		// window rect in screen coords
	rectCaption -= CPoint (rectCaption.left, rectCaption.top);	// shift origin to (0,0)
	rectCaption.left  += szFrame.cx;				// frame
	rectCaption.right -= szFrame.cx;				// frame
	rectCaption.top   += szFrame.cy;				// top = end of frame
	rectCaption.bottom = rectCaption.top + ::GetSystemMetrics(SM_CYSMCAPTION)  // height of caption
		- GetSystemMetrics(SM_CYSMSIZE);				  // minus gray shadow border

	m_rectRollupBox.SetRect (rectCaption.right-GetSystemMetrics(SM_CXSMSIZE)*2 , 
		rectCaption.top+2, rectCaption.right-GetSystemMetrics(SM_CXSMSIZE)-2, rectCaption.top+GetSystemMetrics(SM_CYSMSIZE)-2);

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE| RDW_ALLCHILDREN);


#endif // BCG_NO_SIZINGBAR
}

// Helper to get the control bar...
CBCGSizingControlBar* CBCGMiniDockFrameWnd::GetSizingControlBar()
{
#ifdef BCG_NO_SIZINGBAR
	return NULL;
#else
	int nPos = 1;
	CControlBar* pBar = NULL;
	while(pBar == NULL && nPos < m_wndDockBar.m_arrBars.GetSize())
		pBar = reinterpret_cast<CBCGDockBar&>(m_wndDockBar).GetDockedControlBar(nPos++);

	CBCGSizingControlBar* pSizingBar = DYNAMIC_DOWNCAST (CBCGSizingControlBar, pBar);
	return pSizingBar;
#endif // BCG_NO_SIZINGBAR
}


// allow smallest control bar when rolled...
void CBCGMiniDockFrameWnd::OnWindowPosChanging  ( WINDOWPOS* lpwndpos )
{
	CMiniDockFrameWnd::OnWindowPosChanging(  lpwndpos );

#ifndef BCG_NO_SIZINGBAR

	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
	if (pSizingBar == NULL ||
		pSizingBar->m_nRollingType==BCG_ROLLUP_NONE)
	{
		return;
	}

	m_nRollingType = pSizingBar->m_nRollingType;
	m_bIsRolled = pSizingBar->m_bIsRolled;

	if(m_bIsRolled){
// allow smallest control bar when rolled...
		lpwndpos->cy=GetSystemMetrics(SM_CYSMCAPTION)+ GetSystemMetrics(SM_CYSIZEFRAME)*2 - 1;

// set rollup smallest width if dynamic...
		if(m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_ON)
		{
			if(m_rectCaptionDynamic.left==-1 && m_rectCaptionDynamic.right==-1) 
			{
				CWindowDC dcWin(this);
				GetWindowRect (&m_rectCaptionDynamic);		// window rect in screen coords
				CString str;
				pSizingBar->GetWindowText(str);
				CSize s=dcWin.GetTextExtent(str);
				m_rectCaptionDynamic.right=m_rectCaptionDynamic.left+s.cx+ GetSystemMetrics(SM_CXSMSIZE)*3 + 4 + GetSystemMetrics(SM_CXSIZEFRAME)*2;
			}
			lpwndpos->cx=m_rectCaptionDynamic.Width();
		}
	}
#endif // BCG_NO_SIZINGBAR
}

// handle dynamic sizing...
void CBCGMiniDockFrameWnd::OnNcMouseMove( UINT nHitTest, CPoint point )
{
	CMiniDockFrameWnd::OnNcMouseMove( nHitTest, point );

#ifndef BCG_NO_SIZINGBAR

	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();

	if(pSizingBar && (pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMIC_ON || 
		pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_ON) 
		&& AfxGetMainWnd()->IsWindowEnabled() 
		&& !CBCGPopupMenu::GetActiveMenu ()
		&& GetFocus())
	{
//		if(pSizingBar->m_bIsRolled){
			pSizingBar->m_bIsRolled = 0;
			m_bIsRolled = pSizingBar->m_bIsRolled;

			BringWindowToTop();

			DelayRecalcLayout();

			SetTimer(pSizingBar->GetDlgCtrlID(),250,NULL);
//		}
		m_RollupDelay=ROLLUP_DELAY;
	}
#endif BCG_NO_SIZINGBAR
}

// timer for dynamic sizing...
void CBCGMiniDockFrameWnd::OnTimer(UINT_PTR nIDEvent)
{
#ifndef BCG_NO_SIZINGBAR
	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
	if(nIDEvent==(UINT)pSizingBar->GetDlgCtrlID()){
		CPoint pt;
		GetCursorPos(&pt);
		// if cursor in the window
		// or child has focus
		// or window has focus...
		// or a menu is shown
		CWnd* wnd=WindowFromPoint(pt);
		int IsMouseInsideTheWindow=wnd->GetSafeHwnd () != NULL && 
			(IsChild(wnd) || wnd->GetSafeHwnd ()==m_hWnd);

		if(IsMouseInsideTheWindow
			|| IsChild(GetFocus())  
			|| pSizingBar->m_NoRollup
			|| (GetCapture() && pSizingBar->m_hWnd == GetCapture()->m_hWnd) 
			|| CBCGPopupMenu::GetActiveMenu () // do not rollup while showing a menu, it's bad when a shadow is used...			
			) 
		{
			m_RollupDelay=ROLLUP_DELAY; 

			if(IsMouseInsideTheWindow 
				&& !CBCGPopupMenu::GetActiveMenu () // avoid menu to be hided by control bar
				&& IsWindowEnabled()  // when you display something modal (in MFC...)
				&& GetFocus()) // test if the app is active
					BringWindowToTop();
			
		}else if(pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_ON 
			|| pSizingBar->m_nRollingType == BCG_ROLLUP_DYNAMIC_ON){

			if(m_RollupDelay<=0){
				CBCGSizingControlBar* pSizingBar = GetSizingControlBar();
				if(!pSizingBar->m_bIsRolled){
					pSizingBar->m_bIsRolled = 1;
					m_bIsRolled = pSizingBar->m_bIsRolled;

					DelayRecalcLayout();
					KillTimer(nIDEvent);
				}
			}
			m_RollupDelay--; // after! to be sure to check a last time before closing...
		}
	}
#endif // BCG_NO_SIZINGBAR
}

// allow bar to be destroyed on close...
void CBCGMiniDockFrameWnd::OnClose() 
{
#ifndef BCG_NO_SIZINGBAR
	CBCGSizingControlBar* pSizingBar = GetSizingControlBar();

	if(!pSizingBar || pSizingBar->QueryClose())
		CMiniDockFrameWnd::OnClose();
#endif // BCG_NO_SIZINGBAR
}

void CBCGMiniDockFrameWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMiniDockFrameWnd::OnWindowPosChanged(lpwndpos);
	
#ifndef BCG_NO_SIZINGBAR
	static BOOL bIsMove = FALSE;

	if (bIsMove || (lpwndpos->flags & SWP_NOMOVE) != 0)
	{
		return;
	}

	CBCGSizingControlBar* pSizingBar = GetSizingControlBar ();
	if (pSizingBar == NULL || !pSizingBar->IsFloating ())
	{
		return;
	}

	CRect rectWindow;
	GetWindowRect (rectWindow);
	
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectWindow.TopLeft (), 
						MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	//---------------------------------------
	// Normalize location in the screen area:
	//---------------------------------------
	BOOL bMove = FALSE;

	if (rectWindow.top < rectScreen.top)
	{
		rectWindow.top = rectScreen.top;
		bMove = TRUE;
	}
	else if (rectWindow.top + ::GetSystemMetrics (SM_CYSMCAPTION) > rectScreen.bottom)
	{
		rectWindow.top = max (rectScreen.top, 
			rectScreen.bottom - ::GetSystemMetrics (SM_CYSMCAPTION));
		bMove = TRUE;
	}

	if (bMove)
	{
		bIsMove = TRUE;
		SetWindowPos (NULL, rectWindow.left, rectWindow.top, -1, -1,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		bIsMove = FALSE;
	}

#endif // BCG_NO_SIZINGBAR
}
