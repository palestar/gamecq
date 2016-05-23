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

/////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998, 1999 by Cristi Posea
// All rights reserved
//
// THIS SOURCE CODE WAS INCLUDED INTO THE BCGCONTROLBAR LIBRARY UNDER THE
// PERSONAL PERMISSION OF CRISTI POSEA.
//
// Use and distribute freely, except: don't remove my name from the
// source or documentation (don't take credit for my work), mark your
// changes (don't get me blamed for your possible bugs), don't alter
// or remove this notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// This class is intended to be used as a base class. Do not simply add
// your code to this file - instead create a new class derived from
// CBCGSizingControlBar and put there what you need.
// Modify this file only to fix bugs, and don't forget to send me a copy.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc.,
// and I'll try to keep a version up to date.  I can be reached at:
//    cristip@dundas.com
//
// More details at MFC Programmer's SourceBook
// http://www.codeguru.com/docking/docking_window.shtml or search
// www.codeguru.com for my name if the article was moved.
//
/////////////////////////////////////////////////////////////////////////
//
// Acknowledgements:
//  o   Thanks to Harlan R. Seymour (harlans@dundas.com) for his continuous
//      support during development of this code.
//  o   Thanks to Dundas Software for the opportunity to test this code
//      on real-life applications.
//      If you don't know who they are, visit them at www.dundas.com .
//      Their award winning components and development suites are
//      a pile of gold.
//  o   Thanks to Chris Maunder (chrism@dundas.com) who came with the
//      simplest way to query "Show window content while dragging" system
//      setting.
//  o   Thanks to Zafir Anjum (zafir@codeguru.com) for publishing this
//      code on his cool site (www.codeguru.com).
//  o   Some ideas for the gripper came from the CToolBarEx flat toolbar
//      by Joerg Koenig (Joerg.Koenig@rhein-neckar.de). Also he inspired
//      me on writing this notice:) . Thanks, Joerg!
//  o   Thanks to Jakawan Ratiwanich (jack@alpha.fsec.ucf.edu) and to
//      Udo Schaefer (Udo.Schaefer@vcase.de) for the dwStyle bug fix under
//      VC++ 6.0.
//  o   And, of course, many thanks to all of you who used this code,
//      for the invaluable feedback I received.
//      
// Partialy modified by Stas Levin - bcgsoft@yahoo.com.
// The following features were added:
//
//	o	Drawing splitter bar between the control bars
//	o	Change cursor when mouse is over splitter bar
//	o	Expand/Contract control bars
//	o	Button tooltips
//
/////////////////////////////////////////////////////////////////////////


// BCGSizingControlBar.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_SIZINGBAR

#include "BCGSizingControlBar.h"
#include "BCGControlBarImpl.h"
#include "BCGtoolbar.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "BCGDockBar.h"
#include "globals.h"
#include "RegPath.h"
#include "BCGDockContext.h"
#include "BCGVisualManager.h"
#include "BCGDrawManager.h"
#include "bcgtabwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGSizingControlBar

// gradient defines (if not already defined)
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION     27
#define COLOR_GRADIENTINACTIVECAPTION   28
#define SPI_GETGRADIENTCAPTIONS         0x1008
#endif

static const CString strSCBProfile = _T("BCGSizingControlBar");

CBCGSCBArray CBCGSizingControlBar::m_arrBars; // static member
BOOL CBCGSizingControlBar::m_bCaptionText = FALSE;
BOOL CBCGSizingControlBar::m_bCaptionGradient = FALSE;
BOOL CBCGSizingControlBar::m_bHideDisabledButtons = FALSE;
BOOL CBCGSizingControlBar::m_bCaptionAlwaysOnTop = FALSE;

/////////////////////////////////////////////////////////////////////////////
// All CBCGSizingControlBar collection:
BCGCONTROLBARDLLEXPORT CObList gAllSizingControlBars;

IMPLEMENT_DYNAMIC(CBCGSizingControlBar, CControlBar);

CBCGSizingControlBar::CBCGSizingControlBar()
{
    m_szMax = CSize (-1, -1);

    m_szMin = CSize (40, 40);
    m_szHorz = CSize(200, 200);
    m_szVert = CSize(200, 200);
    m_szFloat = CSize(200, 200);
    m_bTracking = FALSE;
    m_bKeepSize = FALSE;
    m_bParentSizing = FALSE;
	m_cxEdge = 6;
    m_nDockBarID = 0;
    m_dwSCBStyle = 0;
	m_bMaximized = FALSE;

	m_Buttons [0].m_nHit = HTCLOSE_BCG;
	m_Buttons [1].m_nHit = HTMAXBUTTON;

	m_Buttons [0].m_uiTT = IDS_BCGBARRES_HIDE_BAR;
	m_Buttons [1].m_uiTT = IDS_BCGBARRES_EXPAND_BAR;

	m_bIsSingleInRow = FALSE;

	m_ptOld = CPoint (0, 0);
	m_szMaxT = CSize (0, 0);
	m_szOld = CSize (0, 0);

	m_rectRedraw.SetRectEmpty ();

	m_bActive = FALSE;
	m_nBorderSize = 2;

	m_nRollingType = BCG_ROLLUP_NONE;
	m_bIsRolled = FALSE;

	m_NoRollup = 0;
	m_bAutoSaveActiveTab = TRUE;
}

CBCGSizingControlBar::~CBCGSizingControlBar()
{
}

BEGIN_MESSAGE_MAP(CBCGSizingControlBar, CControlBar)
    //{{AFX_MSG_MAP(CBCGSizingControlBar)
    ON_WM_CREATE()
    ON_WM_NCPAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_CAPTURECHANGED()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_NCRBUTTONUP()
    ON_WM_NCHITTEST()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT,OnSetText)
END_MESSAGE_MAP()

BOOL CBCGSizingControlBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd,
                               CSize sizeDefault, BOOL bHasGripper,
                               UINT nID, DWORD dwStyle)
{
    // must have a parent
    ASSERT_VALID(pParentWnd);
    // cannot be both fixed and dynamic
    // (CBRS_SIZE_DYNAMIC is used for resizng when floating)
    ASSERT (!((dwStyle & CBRS_SIZE_FIXED) &&
              (dwStyle & CBRS_SIZE_DYNAMIC)));

    m_dwStyle = dwStyle & CBRS_ALL; // save the control bar styles

    m_szHorz = sizeDefault; // set the size members
    m_szVert = sizeDefault;
    m_szFloat = sizeDefault;

	m_bHasGripper = bHasGripper;
	m_cyGripper = 0;

    // register and create the window - skip CControlBar::Create()
    CString wndclass = ::AfxRegisterWndClass(CS_DBLCLKS,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1), 0);

    dwStyle &= ~CBRS_ALL; // keep only the generic window styles
    dwStyle |= WS_CLIPCHILDREN; // prevents flashing
    if (!CWnd::Create(wndclass, lpszWindowName, dwStyle,
        CRect(0, 0, 0, 0), pParentWnd, nID))
        return FALSE;

    SetBarStyle (GetBarStyle() | CBRS_SIZE_DYNAMIC);

    return TRUE;
}
//**********************************************************************************
void CBCGSizingControlBar::EnableDocking(DWORD dwDockStyle)
{
	CControlBar::EnableDocking(dwDockStyle);

	if (m_pDockContext != NULL) 
	{
		delete m_pDockContext;
		m_pDockContext = NULL;
	}

	if (m_pDockContext == NULL)
	{
		m_pDockContext = new CBCGDockContext (this);
	}
}

/////////////////////////////////////////////////////////////////////////
// CBCGSizingControlBar message handlers

int CBCGSizingControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CControlBar::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_arrBars.Add(this);        // register
    
    m_dwSCBStyle |= SCBS_SHOWEDGES;

	EnableToolTips ();

	m_wndToolTip.Create (this);
	m_wndToolTip.Activate (TRUE);
	if(globalData.m_nMaxToolTipWidth != -1)
	{
		m_wndToolTip.SetMaxTipWidth(globalData.m_nMaxToolTipWidth);
	}

	CBCGLocalResource localRes;
	for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
	{
		CBCGSCBButton& btn = m_Buttons [i];

		btn.m_pParentBar = this;

		CRect rectDummy;
		rectDummy.SetRectEmpty ();

		m_wndToolTip.AddTool (this, btn.m_uiTT, &rectDummy, i + 1);
	}

	if (globalData.m_hcurStretch == NULL)
	{
		globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
	}

	if (globalData.m_hcurStretchVert == NULL)
	{
		globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
	}

	gAllSizingControlBars.AddTail (this);
    return 0;
}

void CBCGSizingControlBar::OnDestroy() 
{
	for (POSITION pos = gAllSizingControlBars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGSizingControlBar* pBar = (CBCGSizingControlBar*) gAllSizingControlBars.GetNext (pos);
		ASSERT (pBar != NULL);

		if (CWnd::FromHandlePermanent (pBar->m_hWnd) != NULL)
		{
			ASSERT_VALID (pBar);

			if (pBar == this)
			{
				gAllSizingControlBars.RemoveAt (posSave);
				break;
			}
		}
	}

	CControlBar::OnDestroy();
}

BOOL CBCGSizingControlBar::DestroyWindow() 
{
    int nPos = FindSizingBar(this);
    ASSERT(nPos >= 0);

    m_arrBars.RemoveAt(nPos);   // unregister

    return CControlBar::DestroyWindow();
}

BOOL CBCGSizingControlBar::IsFloating() const
{
    return !IsHorzDocked() && !IsVertDocked();
}

BOOL CBCGSizingControlBar::IsHorzDocked() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

BOOL CBCGSizingControlBar::IsVertDocked() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT ||
        m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT);
}

BOOL CBCGSizingControlBar::IsSideTracking() const
{
    // don't call this when not tracking
    ASSERT(m_bTracking && !IsFloating());

    return (m_htEdge == HTLEFT || m_htEdge == HTRIGHT) ?
        IsHorzDocked() : IsVertDocked();
}

CSize CBCGSizingControlBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    if (bStretch) // the bar is stretched (is not the child of a dockbar)
        if (bHorz)
            return CSize(32767, m_szHorz.cy);
        else
            return CSize(m_szVert.cx, 32767);

    // dirty cast - using CBCGDockBar to access protected CDockBar members
    CBCGDockBar* pDockBar = (CBCGDockBar*) m_pDockBar;

    // force imediate RecalcDelayShow() for all sizing bars on the row
    // with delayShow/delayHide flags set to avoid IsVisible() problems
    CBCGSCBArray arrSCBars;
    GetRowSizingBars(arrSCBars);
    AFX_SIZEPARENTPARAMS layout;
    layout.hDWP = pDockBar->m_bLayoutQuery ?
        NULL : ::BeginDeferWindowPos ((int) arrSCBars.GetSize());

	BOOL bIsMinimized = FALSE;

    for (int i = 0; i < arrSCBars.GetSize(); i++)
	{
		arrSCBars[i]->RecalcDelayShow(&layout);
		if (arrSCBars[i] != this && arrSCBars[i]->m_bMaximized)
		{
			bIsMinimized = TRUE;
		}
	}

	m_bIsSingleInRow = (arrSCBars.GetSize() < 2);

    if (layout.hDWP != NULL)
        ::EndDeferWindowPos(layout.hDWP);

    // get available length
    CRect rc = pDockBar->m_rectLayout;
    if (rc.IsRectEmpty())
        m_pDockSite->GetClientRect(&rc);

    int nLengthAvail = bHorz ? rc.Width(): rc.Height() - 2;

    if (IsVisible() && !IsFloating() &&
        m_bParentSizing && arrSCBars[0] == this)
        if (NegociateSpace(nLengthAvail, (bHorz != FALSE)))
            AlignControlBars();

    m_bParentSizing = FALSE;
    
    CSize szRet = bHorz ? m_szHorz : m_szVert;

    if(m_szMax.cx > 0 && !bHorz)
    	szRet.cx = min(m_szMax.cx, szRet.cx);
    if(m_szMax.cy > 0 && bHorz)
    	szRet.cy = min(m_szMax.cy, szRet.cy);

	szRet.cx = max(m_szMin.cx, szRet.cx);
	szRet.cy = max(m_szMin.cy, szRet.cy);

	if (bIsMinimized)
	{
		if (bHorz)
		{
			szRet.cx = m_szMin.cx;
		}
		else
		{
			szRet.cy = m_szMin.cy;
		}
	}
	else if (m_bMaximized)
	{
		if (bHorz)
		{
			szRet.cx = nLengthAvail - m_szMin.cx * ((int) arrSCBars.GetSize() - 1);
		}
		else
		{
			szRet.cy = nLengthAvail - m_szMin.cy * ((int) arrSCBars.GetSize() - 1);
		}
	}

    return szRet;
}

CSize CBCGSizingControlBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if (m_bIsRolled && !IsChild(GetFocus()))
	{
		return CSize (m_szFloat.cx, GetSystemMetrics(SM_CYSIZEFRAME) - 1);
	}

    if (dwMode & (LM_HORZDOCK | LM_VERTDOCK)) // docked ?
    {
        if (nLength == -1)
            m_bParentSizing = TRUE;

        return CControlBar::CalcDynamicLayout(nLength, dwMode);
    }

    if (dwMode & LM_MRUWIDTH) return m_szFloat;
    if (dwMode & LM_COMMIT) return m_szFloat; // already committed

    ((dwMode & LM_LENGTHY) ? m_szFloat.cy : m_szFloat.cx) = nLength;

    if(m_szMax.cx > 0)
    	m_szFloat.cx = min(m_szMax.cx, m_szFloat.cx);
    if(m_szMax.cy > 0)
    	m_szFloat.cy = min(m_szMax.cy, m_szFloat.cy);

    m_szFloat.cx = max(m_szFloat.cx, m_szMin.cx);
    m_szFloat.cy = max(m_szFloat.cy, m_szMin.cy);

    return m_szFloat;
}

void CBCGSizingControlBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
    // force non-client recalc if moved or resized
    lpwndpos->flags |= SWP_FRAMECHANGED;

    CControlBar::OnWindowPosChanging(lpwndpos);

    // find on which side are we docked
    m_nDockBarID = GetParent()->GetDlgCtrlID();

    if (!IsFloating())
        if (lpwndpos->flags & SWP_SHOWWINDOW)
            m_bKeepSize = TRUE;
}

/////////////////////////////////////////////////////////////////////////
// Mouse Handling
//
void CBCGSizingControlBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
    if (m_pDockBar != NULL)
    {
		if (m_cyGripper > 0)
		{
			for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
			{
				CBCGSCBButton& btn = m_Buttons [i];

				if (btn.m_bFocused || btn.m_bPushed)
				{
					btn.m_bFocused = btn.m_bPushed = FALSE;
					ReleaseCapture ();
					RedrawButton (btn);
				}
			}
		}

		CRect rcBar;
		GetWindowRect (rcBar);

		CRect rcClient;
	    GetClientRect(rcClient);
        ClientToScreen(&rcClient);
  
        // start the drag
        ASSERT(m_pDockContext != NULL);
        ClientToScreen(&point);

		if( rcClient.PtInRect(point) ) 
		{
	        m_pDockContext->StartDrag(point);
		}
		else if ( rcBar.PtInRect(point) ) 
		{
			BCGNcHitTestType nHitTest = OnNcHitTest(point);
			OnNcLButtonDown ((UINT) nHitTest, point);
		}
    }
    else
        CWnd::OnLButtonDown(nFlags, point);
}

void CBCGSizingControlBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    if (m_pDockBar != NULL)
    {
        // toggle docking
        ASSERT(m_pDockContext != NULL);
        m_pDockContext->ToggleDocking();
    }
    else
        CWnd::OnLButtonDblClk(nFlags, point);
}

void CBCGSizingControlBar::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
    if (IsFloating())
    {
        CControlBar::OnNcLButtonDown(nHitTest, point);
        return;
    }

    if (m_bTracking) return;

	if (m_cyGripper > 0)
	{
		for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			CBCGSCBButton& btn = m_Buttons [i];

			if (btn.m_nHit == nHitTest)
			{
				if (nHitTest != HTMAXBUTTON || !m_bIsSingleInRow)
				{
					btn.m_bFocused = btn.m_bPushed = TRUE;
					RedrawButton (btn);
					SetCapture ();
				}

				break;
			}
		}
	}

    if ((nHitTest >= HTSIZEFIRST) && (nHitTest <= HTSIZELAST))
	{
        StartTracking(nHitTest); // sizing edge hit
	}

	if (nHitTest == HTCAPTION) 
	{
		CWnd* pWndChild = GetWindow (GW_CHILD);

		while (pWndChild != NULL)
		{
			if (!pWndChild->IsKindOf (RUNTIME_CLASS (CControlBar)))
			{
				pWndChild->SetFocus ();
				break;
			}

			pWndChild = pWndChild->GetNextWindow ();
		}

		m_pDockContext->StartDrag(point);
	}
}

void CBCGSizingControlBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if (m_bTracking)
	{
        StopTracking ();

        CBCGSCBArray arrSCBars;
        GetRowSizingBars(arrSCBars);

		BOOL bHorz = IsHorzDocked();

		for (int i = 0; i < arrSCBars.GetSize(); i++)
		{
			CBCGSizingControlBar* pBar = arrSCBars[i];

			BOOL bMaximized = pBar->m_bMaximized;

			if (bHorz)
			{
				if (pBar->m_szHorz.cx == m_szMaxT.cx &&
					arrSCBars.GetSize() == 2)
				{
					pBar->m_bMaximized = TRUE;

					if (pBar->m_szOld.cx == pBar->m_szHorz.cx)
					{
						pBar->m_szOld.cx = pBar->m_szHorz.cx / (int) arrSCBars.GetSize();
					}
				}
				else
				{
					pBar->m_bMaximized = FALSE;
				}
			}
			else
			{
				if (pBar->m_szVert.cy == m_szMaxT.cy &&
					arrSCBars.GetSize() == 2)
				{
					pBar->m_bMaximized = TRUE;

					if (pBar->m_szOld.cy == pBar->m_szVert.cy)
					{
						pBar->m_szOld.cy = pBar->m_szVert.cy / (int) arrSCBars.GetSize();
					}
				}
				else
				{
					pBar->m_bMaximized = FALSE;
				}
			}

			if (bMaximized != pBar->m_bMaximized)
			{
				pBar->RedrawButton (pBar->m_Buttons [1]);
			}
		}
	}
	else if (m_cyGripper > 0)
	{
		for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			CBCGSCBButton& btn = m_Buttons [i];

			if (btn.m_bPushed)
			{
				BOOL bFocused = btn.m_bFocused;

				ReleaseCapture ();
				btn.m_bFocused = btn.m_bPushed = FALSE;

				RedrawButton (btn);

				if (bFocused)
				{
					switch (btn.m_nHit)
					{
					case HTCLOSE_BCG:
						if (QueryClose ())
						{
							m_pDockSite->ShowControlBar(this, FALSE, FALSE); // hide
						}
						break;

					case HTMAXBUTTON:
						Expand (TRUE);
						break;
					}
				}
				break;
			}
		}
	}

    CControlBar::OnLButtonUp(nFlags, point);
}

void CBCGSizingControlBar::OnRButtonDown(UINT nFlags, CPoint point) 
{
    if (m_bTracking)
	{
        StopTracking();
	}
	else if (!CBCGToolBar::IsCustomizeMode ())
	{
		CFrameWnd* pParentFrame = (m_pDockSite == NULL) ?
			GetDockingFrame() : m_pDockSite;
		ASSERT_VALID(pParentFrame);

		ClientToScreen (&point);
		pParentFrame->SendMessage (BCGM_TOOLBARMENU,
			(WPARAM) GetSafeHwnd (),
			MAKELPARAM(point.x, point.y));
		return;
	}
    
    CControlBar::OnRButtonDown(nFlags, point);
}

void CBCGSizingControlBar::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_bTracking)
	{
        OnTrackUpdateSize(point);
	}
	else 
	{
		CPoint ptScreen = point;
		ClientToScreen (&ptScreen);

		TrackButtons (ptScreen);
	}
    
    CControlBar::OnMouseMove(nFlags, point);
}

void CBCGSizingControlBar::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
    if (!m_bTracking)
	{
		TrackButtons (point);
	}

	CControlBar::OnNcMouseMove(nHitTest, point);
}

void CBCGSizingControlBar::TrackButtons (const CPoint& ptScreen)
{
	if (m_cyGripper <= 0)
	{
		return;
	}

	CRect rcBar;
	GetWindowRect (rcBar);

	int iPushedButton = -1;
	int iFocusedButton = -1;
	int i = 0;

	for (; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
	{
		CBCGSCBButton& btn = m_Buttons [i];
		if (btn.m_bPushed)
		{
			iPushedButton = i;
		}

		if (btn.m_bFocused)
		{
			iFocusedButton = i;
		}
	}

	int iNewFocusedButton = -1;

	for (i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
	{
		CBCGSCBButton& btn = m_Buttons [i];
		BOOL bIsDisabled = m_bIsSingleInRow && btn.m_nHit == HTMAXBUTTON;

		CRect rectBtn = btn.GetRect();
		rectBtn.OffsetRect (rcBar.TopLeft());

		if (GetExStyle() && WS_EX_LAYOUTRTL)
			rectBtn.OffsetRect(rcBar.right - rectBtn.right + rcBar.left - rectBtn.left, 0);

		BOOL bFocused = !bIsDisabled && rectBtn.PtInRect (ptScreen);
		if (iPushedButton >= 0 && iPushedButton != i)
		{
			bFocused = FALSE;
		}

		if (bFocused)
		{
			iNewFocusedButton = i;
		}

		if (btn.m_bFocused != bFocused)
		{
			btn.m_bFocused = bFocused;
			RedrawButton (btn);
		}
	}

	if (iPushedButton == -1)
	{
		if (iFocusedButton != -1  && iNewFocusedButton == -1)
		{
			::ReleaseCapture();
		}
		else if (iFocusedButton == -1  && iNewFocusedButton != -1)
		{
			SetCapture ();
		}
	}
}

void CBCGSizingControlBar::OnCaptureChanged(CWnd *pWnd) 
{
    if (m_bTracking && (pWnd != this))
	{
        StopTracking();
	}

    CControlBar::OnCaptureChanged(pWnd);

	if (m_cyGripper > 0)
	{
		for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			CBCGSCBButton& btn = m_Buttons [i];
			if (btn.m_bFocused || btn.m_bPushed)
			{
				btn.m_bFocused = btn.m_bPushed = FALSE;
				RedrawButton (btn);
			}
		}
	}
}

void CBCGSizingControlBar::OnNcCalcSize(BOOL /*bCalcValidRects*/,
                                     NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	const int nBorderSize = max (1, m_nBorderSize);

    // compute the the client area
    CRect rcClient = lpncsp->rgrc[0];
    rcClient.DeflateRect (nBorderSize, nBorderSize);

	if (m_bHasGripper)
	{
		m_cyGripper = max (12, globalData.GetTextHeight ());
		if (m_bCaptionText)
		{
			m_cyGripper += 4;
		}
	}
	else
	{
		m_cyGripper = 0;
	}

    m_dwSCBStyle &= ~SCBS_EDGEALL;
	BOOL bRTL = (GetExStyle() & WS_EX_LAYOUTRTL);

	switch(m_nDockBarID)
	{
	case AFX_IDW_DOCKBAR_TOP:
		m_dwSCBStyle |= SCBS_EDGEBOTTOM;

		if (m_bCaptionAlwaysOnTop)
		{
			rcClient.DeflateRect(0, m_cyGripper, 0, 0);
		}
		else
		{
			rcClient.DeflateRect(m_cyGripper, 0, 0, 0);

			if (bRTL)
				rcClient.OffsetRect(-m_cyGripper, 0);
		}
		break;

	case AFX_IDW_DOCKBAR_BOTTOM:
		m_dwSCBStyle |= SCBS_EDGETOP;

		if (m_bCaptionAlwaysOnTop)
		{
			rcClient.DeflateRect(0, m_cyGripper, 0, 0);
		}
		else
		{
			rcClient.DeflateRect(m_cyGripper, 0, 0, 0);

			if (bRTL)
				rcClient.OffsetRect(-m_cyGripper, 0);
		}
		break;

	case AFX_IDW_DOCKBAR_LEFT:
		m_dwSCBStyle |= SCBS_EDGERIGHT;
		rcClient.DeflateRect(0, m_cyGripper, 0, 0);
		break;
	case AFX_IDW_DOCKBAR_RIGHT:
		m_dwSCBStyle |= SCBS_EDGELEFT;
		rcClient.DeflateRect(0, m_cyGripper, 0, 0);
		break;
	default:
		break;
	}

	BOOL bIsFirst = FALSE;

    if (!IsFloating() && m_pDockBar != NULL)
    {
        CBCGSCBArray arrSCBars;
        GetRowSizingBars(arrSCBars);

        for (int i = 0; i < arrSCBars.GetSize(); i++)
		{
            if (arrSCBars[i] == this)
            {
				if (i == 0)
				{
					bIsFirst = TRUE;
				}

                if (i > 0)
                    m_dwSCBStyle |= IsHorzDocked() ?
                        SCBS_EDGELEFT : SCBS_EDGETOP;
                if (i < arrSCBars.GetSize() - 1)
                    m_dwSCBStyle |= IsHorzDocked() ?
                        SCBS_EDGERIGHT : SCBS_EDGEBOTTOM;
            }
		}
    }

    // make room for edges only if they will be painted
    if (m_dwSCBStyle & SCBS_SHOWEDGES)
	{
        rcClient.DeflateRect(
            IsEdgeVisible (bRTL ? HTRIGHT : HTLEFT) ? m_cxEdge : 0,
            IsEdgeVisible (HTTOP) ? m_cxEdge : 0,
            IsEdgeVisible (bRTL ? HTLEFT : HTRIGHT) ? m_cxEdge : 0,
            IsEdgeVisible (HTBOTTOM) ? m_cxEdge : 0);
	}

	rcClient.right = max(rcClient.right, rcClient.left);
	rcClient.bottom = max(rcClient.bottom, rcClient.top);

    lpncsp->rgrc[0] = rcClient;

	if (!m_bTracking && GetCapture() == this)
	{
		::ReleaseCapture();
	}
}

void CBCGSizingControlBar::ReposCaptionButtons ()
{
	CSize sizeButton = CBCGSCBButton::GetSize ();
    CPoint ptOrgBtn;

    CRect rcClient, rcBar;
    GetClientRect(rcClient);
    ClientToScreen(rcClient);
    GetWindowRect(rcBar);

    rcClient.OffsetRect(-rcBar.TopLeft());
    rcBar.OffsetRect(-rcBar.TopLeft());

	BOOL bRTL = (GetExStyle() & WS_EX_LAYOUTRTL);

	if (bRTL)
		rcClient.OffsetRect(rcBar.right - rcClient.right - (rcClient.left - rcBar.left), 0);

	CRect rectGripper = GetGripperRect (rcClient, TRUE);

    if (IsHorzDocked() && !m_bCaptionAlwaysOnTop)
	{
		rectGripper.DeflateRect (0, 2);
		const int dx = max (0, (rectGripper.Width () - sizeButton.cx) / 2);

        ptOrgBtn = CPoint (
			rectGripper.left + dx, rectGripper.top);
	}
    else
	{
		rectGripper.DeflateRect (2, 0);
		const int dy = max (0, (rectGripper.Height () - sizeButton.cy) / 2);

        ptOrgBtn = CPoint (
			rectGripper.right - sizeButton.cx, rectGripper.top + dy);
	}

    GetWindowRect(rcBar);
	ScreenToClient (rcBar);

	if (m_cyGripper > 0)
	{
		for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			CBCGSCBButton& btn = m_Buttons [i];
			btn.m_clrForeground = (COLORREF)-1;

			BOOL bHide = (m_bHideDisabledButtons &&
						m_bIsSingleInRow && btn.m_nHit == HTMAXBUTTON);

			btn.m_bFocused = btn.m_bPushed = FALSE;

			if ((IsVertDocked () || m_bCaptionAlwaysOnTop) && bHide)
			{
				btn.Move (ptOrgBtn + CSize (sizeButton.cx, 0), TRUE);
			}
			else
			{
				btn.Move (ptOrgBtn, bHide);
			}

			if (m_wndToolTip.GetSafeHwnd () != NULL)
			{
				CRect rectTT = btn.GetRect ();
				rectTT.OffsetRect (rcBar.TopLeft());
				m_wndToolTip.SetToolRect (this, i + 1, rectTT);
			}

			if (!bHide)
			{
				if (IsHorzDocked() && !m_bCaptionAlwaysOnTop)
				{
					ptOrgBtn.Offset (0, sizeButton.cy + 1);
				}
				else
				{
					ptOrgBtn.Offset (- sizeButton.cx - 1, 0);
				}
			}
		}
	}

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}

void CBCGSizingControlBar::OnNcPaint() 
{
	if (m_bTracking)
	{
		return;
	}

    // get window DC that is clipped to the non-client area
    CWindowDC dc(this);

    CRect rcClient, rcBar;
    GetClientRect(rcClient);
    ClientToScreen(rcClient);
    GetWindowRect(rcBar);

	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

    rcClient.OffsetRect(-rcBar.TopLeft());
    rcBar.OffsetRect(-rcBar.TopLeft());
	if (bRTL)
		rcClient.OffsetRect(rcBar.right - rcClient.right - (rcClient.left - rcBar.left), 0);

    // client area is not our bussiness :)
	dc.ExcludeClipRect(rcClient);

	if (!m_rectRedraw.IsRectEmpty ())
	{
		CRgn rgn;
		rgn.CreateRectRgnIndirect (m_rectRedraw);

		dc.SelectClipRgn (&rgn);
	}

    // erase parts not drawn
    dc.IntersectClipRect(rcBar);

	if (ClipPaint ())
	{
		MSG* pMsg = &AfxGetThreadState()->m_lastSentMsg;

		ASSERT (pMsg->hwnd == m_hWnd);
		ASSERT (pMsg->message == WM_NCPAINT);

		CRgn* pRgn = NULL;
		if (pMsg->wParam != 1 && 
			(pRgn = CRgn::FromHandle ((HRGN) pMsg->wParam)) != NULL)
		{
			CRect rect;
			GetWindowRect (rect);

			if (bRTL)
			{
				CRect rect2;
				pRgn->GetRgnBox(&rect2);
				rect2.OffsetRect(rect.right - rect2.right - rect2.left, -rect.top);
				CRgn rgn;
				rgn.CreateRectRgnIndirect(&rect2);
				dc.SelectClipRgn(&rgn, RGN_AND);
			}
			else
			{
				pRgn->OffsetRgn (- rect.TopLeft ());
				dc.SelectClipRgn (pRgn, RGN_AND);
			}
		}
	}

    // erase NC background the hard way
	CBCGVisualManager::GetInstance ()->OnFillBarBackground (&dc, this, rcBar, rcBar, 
		TRUE /* NC area */);

	CRect rcBorder = rcBar;
	rcBorder.InflateRect (IsHorzDocked() && !m_bCaptionAlwaysOnTop, 
		IsVertDocked () || m_bCaptionAlwaysOnTop);
	
    if (m_dwSCBStyle & SCBS_SHOWEDGES)
    {
        CRect rcEdge; // paint the sizing edges
        for (int i = 3; i >= 0; --i)
		{
			UINT nHitTest = GetEdgeHTCode(i);

            if (GetEdgeRect (rcBar, nHitTest, rcEdge))
			{
				CRect rcEdgeInternal = rcEdge;

				switch (nHitTest)
				{
				case HTLEFT:
					rcEdge.InflateRect( 0, IsVertDocked()?2:0 );

					rcEdge.OffsetRect(-IsVertDocked(), 0);
					rcEdgeInternal = rcEdge;
					rcEdgeInternal.InflateRect (1, 0);

					if(IsVertDocked())
						rcBorder.left	= rcEdge.right;
					break;

				case HTRIGHT:
					rcEdge.InflateRect( 0, IsVertDocked () || m_bCaptionAlwaysOnTop ?2:1 );

					rcEdge.OffsetRect(IsVertDocked () || m_bCaptionAlwaysOnTop, 0);
					rcEdgeInternal = rcEdge;
					rcEdgeInternal.InflateRect (1, 0);

					if(IsVertDocked () || m_bCaptionAlwaysOnTop)
						rcBorder.right = rcEdge.left;
					break;

				case HTTOP:
					rcEdge.InflateRect( IsHorzDocked()?2:0, 0 );
					rcEdge.OffsetRect(0, -(m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM) );

					rcEdgeInternal = rcEdge;
					rcEdgeInternal.InflateRect (0, 1);

					if(IsHorzDocked())
						rcBorder.top	= rcEdge.bottom;
					break;

				case HTBOTTOM:
					rcEdge.InflateRect( IsHorzDocked()?2:1, 0 );

					rcEdge.OffsetRect(0, IsHorzDocked());
					rcEdgeInternal = rcEdge;
					rcEdgeInternal.InflateRect (0, 1);

					if(IsHorzDocked())
						rcBorder.bottom	= rcEdge.top;
					break;

				default:
					if( IsEdgeVisible (nHitTest))

					ASSERT(FALSE); // invalid hit test code
				}

				if (CBCGVisualManager::GetInstance ()->IsDrawControlBarEdges ())
				{
					if (CBCGVisualManager::GetInstance ()->IsLook2000 ())
					{
						dc.Draw3dRect (rcEdgeInternal, globalData.clrBarShadow,
							globalData.clrBarHilite);
					}

					dc.Draw3dRect (rcEdge, globalData.clrBarHilite,
						CBCGVisualManager::GetInstance ()->IsLook2000 () ? 
							globalData.clrBarShadow : globalData.clrBarDkShadow);

					rcEdge.DeflateRect ( nHitTest==HTLEFT || nHitTest==HTRIGHT,
						nHitTest==HTTOP || nHitTest==HTBOTTOM);
				}
			}
		}
    }

    if (m_cyGripper > 0 && !IsFloating())
	{
		// Paint gripper and buttons:
		CRect rectGripper = GetGripperRect (rcClient, m_bCaptionText);

		if (m_bCaptionText)
		{
			DrawCaption (&dc, rectGripper);
		}
		else
		{
			//----------------------
			// Draw regular gripper:
			//----------------------
			CBCGVisualManager::GetInstance ()->OnDrawBarGripper (&dc, rectGripper, IsHorzDocked() && !m_bCaptionAlwaysOnTop, this);
		}

		if (m_rectRedraw.IsRectEmpty ())
		{
			//---------------------------------------------------
			// Clip buttons area (to limit Visualization Manger):
			//---------------------------------------------------
			CRect rectClipButtons (0, 0, 0, 0);
			for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
			{
				rectClipButtons += m_Buttons [i].GetRect ();
			}

			CRgn rgn;
			rgn.CreateRectRgnIndirect (rectClipButtons);

			dc.SelectClipRgn (&rgn, RGN_AND);
		}

		for (int i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			m_Buttons [i].Paint (&dc, IsHorzDocked(), m_bMaximized, 
				m_bIsSingleInRow && m_Buttons [i].m_nHit == HTMAXBUTTON);
		}
	}

	dc.SelectClipRgn (NULL);
}

CRect CBCGSizingControlBar::GetGripperRect (const CRect& rcClient,
											BOOL bIncludeButtons)
{
	CRect rectGripper;

	GetWindowRect (&rectGripper);
	ScreenToClient (&rectGripper);

	rectGripper.OffsetRect (-rectGripper.left, -rectGripper.top);

	CRect rcbtn = m_Buttons [CBCGSIZINGCONTROLBAR_BUTTONS_NUM - 1].GetRect();

	rectGripper.DeflateRect(1, 1);

	if (IsHorzDocked() && !m_bCaptionAlwaysOnTop)
	{   // gripper at left
		rectGripper.top = bIncludeButtons ? rcClient.top - 3 : rcbtn.bottom + 3;
		rectGripper.bottom = rcClient.bottom;
		rectGripper.left ++;
		rectGripper.right = rectGripper.left + m_cyGripper;
	}
	else
	{   // gripper at top
		rectGripper.left = rcClient.left;
		rectGripper.right = bIncludeButtons ? rcClient.right : rcbtn.left - 3;

		if (IsHorzDocked () && 
			(!m_bCaptionAlwaysOnTop || m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM))
		{
			rectGripper.top += 6;
		}
		else
		{
			rectGripper.top++;
		}

		rectGripper.bottom = rectGripper.top + m_cyGripper;
	}

	return rectGripper;
}

void CBCGSizingControlBar::OnPaint()
{
    // overridden to skip border painting based on clientrect
    CPaintDC dc(this);
}

BCGNcHitTestType CBCGSizingControlBar::OnNcHitTest(CPoint point)
{
    CRect rcBar, rcEdge;
    GetWindowRect(rcBar);

	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;
	if (bRTL)
	{
		// In order to get the 'real' hit, we need to 'flip' the point!
		point.x = rcBar.right - (point.x - rcBar.left);
	}

    if (IsFloating()) {
		for (int i = 0; i < 4; i++)
			if (GetEdgeRect(rcBar, GetEdgeHTCode(i), rcEdge))
				if (rcEdge.PtInRect(point)) return GetEdgeHTCode(i);

        return CControlBar::OnNcHitTest(point);
	}
	////////

	int i = 0;

    for (i = 0; i < 4; i++)
        if (GetEdgeRect(rcBar, GetEdgeHTCode(i), rcEdge))
            if (rcEdge.PtInRect(point)) return GetEdgeHTCode(i);

	if (m_cyGripper > 0)
	{
		for (i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
		{
			CBCGSCBButton& btn = m_Buttons [i];

			CRect rc = btn.GetRect();
			rc.OffsetRect(rcBar.TopLeft());
			if (rc.PtInRect(point))
			{
				return btn.m_nHit;
			}
		}
	}

	// Maybe on caption?
	CRect rcClient;
	GetClientRect(rcClient);
	ClientToScreen(&rcClient);

	if (bRTL)
	{
		// 'Flip' the point back, as we need the real window for this check
		point.x = rcBar.right - (point.x - rcBar.left);
	}


	if( !rcClient.PtInRect(point))
		return HTCAPTION;

    return HTCLIENT;
}

/////////////////////////////////////////////////////////////////////////
// CBCGSizingControlBar implementation helpers

void CBCGSizingControlBar::StartTracking(UINT nHitTest)
{
    SetCapture();

	int i = 0;

    // make sure no updates are pending
    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW);
    
    BOOL bHorz = IsHorzDocked();

    m_szOld = bHorz ? m_szHorz : m_szVert;

    CRect rc;
    GetWindowRect(&rc);
	m_ptOld = ::GetMessagePos();
	ScreenToClient(&m_ptOld);

    m_htEdge = nHitTest;
    m_bTracking = TRUE;

    CBCGSCBArray arrSCBars;
    GetRowSizingBars(arrSCBars);

    // compute the minsize as the max minsize of the sizing bars on row
    m_szMinT = m_szMin;
    CSize szMaxRow(0,0);
    for (i = 0; i < arrSCBars.GetSize(); i++) {
        if (bHorz) {
            m_szMinT.cy = max(m_szMinT.cy, arrSCBars[i]->m_szMin.cy);
            szMaxRow.cy = (szMaxRow.cy == -1 || arrSCBars[i]->m_szMax.cy == -1) 
                ? (-1) : max(szMaxRow.cy,arrSCBars[i]->m_szMax.cy);
        }
        else {
            m_szMinT.cx = max(m_szMinT.cx, arrSCBars[i]->m_szMin.cx);
            szMaxRow.cx = (szMaxRow.cx == -1 || arrSCBars[i]->m_szMax.cx == -1) 
                ? (-1) : max(szMaxRow.cx,arrSCBars[i]->m_szMax.cx);
        }
    }
        
    if (!IsSideTracking())
    {
        // the control bar cannot grow with more than the size of 
        // remaining client area of the mainframe
        m_pDockSite->RepositionBars(0, 0xFFFF, AFX_IDW_PANE_FIRST,
            reposQuery, &rc, NULL, TRUE);
		m_pDockSite->SendMessage(WM_IDLEUPDATECMDUI);

        m_szMaxT = m_szOld + rc.Size() - CSize(4, 4);

        // Support for maximum size
        if( !bHorz && szMaxRow.cx > 0 && m_szMaxT.cx > szMaxRow.cx) {
            m_szMaxT.cx = szMaxRow.cx;
        }
        else if(szMaxRow.cy > 0 && m_szMaxT.cy > szMaxRow.cy) {
            m_szMaxT.cy = szMaxRow.cy;
        }
    }
    else
    {
        // side tracking: max size is the actual size plus the amount
        // the neighbour bar can be decreased to reach its minsize
		BOOL bFound = FALSE;
        for (i = 0; i < arrSCBars.GetSize (); i++)
		{
            if (arrSCBars [i] == this) 
			{
				bFound = TRUE;
				break;
			}
		}

		if (bFound)
		{
			CBCGSizingControlBar* pBar = arrSCBars[i +
				((m_htEdge == HTTOP || m_htEdge == HTLEFT) ? -1 : 1)];

			m_szMaxT = m_szOld + (bHorz ? pBar->m_szHorz :
				pBar->m_szVert) - pBar->m_szMin;
		}
    }

    OnTrackInvertTracker(); // draw tracker
}

void CBCGSizingControlBar::StopTracking()
{
    OnTrackInvertTracker(); // erase tracker

    m_bTracking = FALSE;
    ReleaseCapture();

    m_pDockSite->DelayRecalcLayout();
}

void CBCGSizingControlBar::OnTrackUpdateSize(CPoint& point)
{
    ASSERT(!IsFloating());

    CSize szDelta = point - m_ptOld;

    if (szDelta == CSize(0, 0)) return; // no size change

    CSize sizeNew = m_szOld;
    switch (m_htEdge)
    {
    case HTLEFT:    sizeNew -= CSize(szDelta.cx, 0); break;
    case HTTOP:     sizeNew -= CSize(0, szDelta.cy); break;
    case HTRIGHT:   sizeNew += CSize(szDelta.cx, 0); break;
    case HTBOTTOM:  sizeNew += CSize(0, szDelta.cy); break;
    }

    // enforce the limits
    sizeNew.cx = max(m_szMinT.cx, min(m_szMaxT.cx, sizeNew.cx));
    sizeNew.cy = max(m_szMinT.cy, min(m_szMaxT.cy, sizeNew.cy));

    BOOL bHorz = IsHorzDocked();
    szDelta = sizeNew - (bHorz ? m_szHorz : m_szVert);
    
    OnTrackInvertTracker(); // erase tracker

    (bHorz ? m_szHorz : m_szVert) = sizeNew; // save the new size

    CBCGSCBArray arrSCBars;
    GetRowSizingBars(arrSCBars);

    for (int i = 0; i < arrSCBars.GetSize(); i++)
	{
		CBCGSizingControlBar* pBar = NULL;

        if (!IsSideTracking())
        {   // track simultaneously
            pBar = arrSCBars[i];
			ASSERT_VALID (pBar);

            (bHorz ? pBar->m_szHorz.cy : pBar->m_szVert.cx) =
                bHorz ? sizeNew.cy : sizeNew.cx;
        }
        else
        {   // adjust the neighbour's size too
            if (arrSCBars[i] != this) continue;

            pBar = arrSCBars[i +
                ((m_htEdge == HTTOP || m_htEdge == HTLEFT) ? -1 : 1)];
			ASSERT_VALID (pBar);

            (bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy) -=
                bHorz ? szDelta.cx : szDelta.cy;
        }
	}

    OnTrackInvertTracker(); // redraw tracker at new pos
}

void CBCGSizingControlBar::OnTrackInvertTracker()
{
    ASSERT(m_bTracking);

	if (m_pDockBar == NULL)
	{
		return;
	}

    BOOL bHorz = IsHorzDocked();
    CRect rc, rcBar, rcDock, rcFrame;
    GetWindowRect(rcBar);
    m_pDockBar->GetWindowRect(rcDock);
    m_pDockSite->GetWindowRect(rcFrame);
    VERIFY(GetEdgeRect(rcBar, m_htEdge, rc));
    if (!IsSideTracking())
        rc = bHorz ? 
            CRect(rcDock.left + 1, rc.top, rcDock.right - 1, rc.bottom) :
            CRect(rc.left, rcDock.top + 1, rc.right, rcDock.bottom - 1);

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		if ((rc.left >= rcBar.left) && (rc.right <= rcBar.right))
		{
			rc.OffsetRect(rcBar.right + rcBar.left - rc.right - rc.left, 0);
			rc.OffsetRect(rcFrame.right + rcFrame.left - rc.right - rc.left, 0);
		}
	}

    rc.OffsetRect(-rcFrame.TopLeft());

    CSize sizeNew = bHorz ? m_szHorz : m_szVert;
    CSize sizeDelta = sizeNew - m_szOld;

    if (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT && m_htEdge == HTTOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT && m_htEdge != HTBOTTOM ||
        m_nDockBarID == AFX_IDW_DOCKBAR_TOP && m_htEdge == HTLEFT ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM && m_htEdge != HTRIGHT)
        sizeDelta = -sizeDelta;
    rc.OffsetRect(sizeDelta);

    CDC *pDC = m_pDockSite->GetDCEx(NULL,
        DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);

	CBrush* pBrush = CDC::GetHalftoneBrush();
    CBrush* pBrushOld = pDC->SelectObject(pBrush);

    pDC->PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATINVERT);
    
    pDC->SelectObject(pBrushOld);
    m_pDockSite->ReleaseDC(pDC);
}
//************************************************************************************
BOOL CBCGSizingControlBar::GetEdgeRect(CRect rcWnd, UINT nHitTest,
                                    CRect& rcEdge)
{
	if (!IsEdgeVisible (nHitTest))
	{
		rcEdge.SetRectEmpty ();
		return FALSE;
	}

    rcEdge = rcWnd;
    if (m_dwSCBStyle & SCBS_SHOWEDGES)
	{
        rcEdge.DeflateRect(1, 1);
	}

    BOOL bHorz = IsHorzDocked();

    switch (nHitTest)
    {
    case HTLEFT:
        rcEdge.right = rcEdge.left + m_cxEdge;
        rcEdge.DeflateRect(0, bHorz ? m_cxEdge: 0);

		if (m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM)
		{
			rcEdge.bottom = rcWnd.bottom + 1;
		}

		if (m_nDockBarID == AFX_IDW_DOCKBAR_TOP)
		{
			rcEdge.top = rcWnd.top - 1;
		}

        break;

    case HTTOP:
        rcEdge.bottom = rcEdge.top + m_cxEdge;
        rcEdge.DeflateRect(bHorz ? 0 : m_cxEdge, 0);

		if (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT)
		{
			rcEdge.left = rcWnd.left - 1;
		}

		if (m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT)
		{
			rcEdge.right = rcWnd.right + 1;
		}
        break;

    case HTRIGHT:
        rcEdge.left = rcEdge.right - m_cxEdge;
        rcEdge.DeflateRect(0, bHorz ? m_cxEdge: 0);

		if (m_nDockBarID == AFX_IDW_DOCKBAR_TOP)
		{
			rcEdge.top = rcWnd.top - 1;
		}

		if (m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM)
		{
			rcEdge.bottom = rcWnd.bottom + 1;
		}
        break;

    case HTBOTTOM:
        rcEdge.top = rcEdge.bottom - m_cxEdge;
        rcEdge.DeflateRect(bHorz ? 0 : m_cxEdge, 0);

		if (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT)
		{
			rcEdge.left = rcWnd.left - 1;
		}

		if (m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT)
		{
			rcEdge.right = rcWnd.right + 1;
		}
        break;

    default:
        ASSERT(FALSE); // invalid hit test code
    }

    return TRUE;
}

UINT CBCGSizingControlBar::GetEdgeHTCode(int nEdge)
{
	switch (nEdge)
	{
	case 0: return HTLEFT;
	case 1: return HTTOP;
	case 2: return HTRIGHT;
	case 3: return HTBOTTOM;
	}

    ASSERT(FALSE); // invalid edge no
    return HTNOWHERE;
}

void CBCGSizingControlBar::GetRowInfo(int& nFirst, int& nLast, int& nThis)
{
	if (m_pDockBar == NULL)
	{
		return;
	}

    ASSERT_VALID(m_pDockBar); // verify bounds

    nThis = m_pDockBar->FindBar(this);
    ASSERT(nThis != -1);

    int i, nBars = (int) m_pDockBar->m_arrBars.GetSize();

    // find the first and the last bar in row
    for (nFirst = -1, i = nThis - 1; i >= 0 && nFirst == -1; i--)
        if (m_pDockBar->m_arrBars[i] == NULL)
            nFirst = i + 1;
    for (nLast = -1, i = nThis + 1; i < nBars && nLast == -1; i++)
        if (m_pDockBar->m_arrBars[i] == NULL)
            nLast = i - 1;

    ASSERT((nLast != -1) && (nFirst != -1));
}

void CBCGSizingControlBar::GetRowSizingBars(CBCGSCBArray& arrSCBars)
{
    arrSCBars.RemoveAll();

	if (m_pDockBar == NULL)
	{
		return;
	}

    int nFirst, nLast, nThis;
    GetRowInfo(nFirst, nLast, nThis);

    for (int i = nFirst; i <= nLast; i++)
    {
        CControlBar* pBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pBar) == 0) continue; // placeholder
        if (!pBar->IsVisible()) continue;
        if (FindSizingBar(pBar) >= 0)
            arrSCBars.Add((CBCGSizingControlBar*)pBar);
    }
}

const int CBCGSizingControlBar::FindSizingBar(CControlBar* pBar) const
{
    for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
        if (m_arrBars[nPos] == pBar)
            return nPos; // got it

    return -1; // not found
}

BOOL CBCGSizingControlBar::NegociateSpace(int nLengthAvail, BOOL bHorz)
{
	if (m_pDockBar == NULL)
	{
		return TRUE;
	}

    ASSERT(bHorz == IsHorzDocked());

    int nFirst, nLast, nThis;
	int i = 0;
    GetRowInfo(nFirst, nLast, nThis);

    // step 1: subtract the visible fixed bars' lengths
    for (i = nFirst; i <= nLast; i++)
    {
        CControlBar* pFBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pFBar) == 0) continue; // placeholder
        if (!pFBar->IsVisible() || (FindSizingBar(pFBar) >= 0)) continue;

        CRect rcBar;
        pFBar->GetWindowRect(&rcBar);

        nLengthAvail -= (bHorz ? rcBar.Width()  : rcBar.Height() );
    }

    CBCGSCBArray arrSCBars;
    GetRowSizingBars(arrSCBars);
    CBCGSizingControlBar* pBar;

    // step 2: compute actual and min lengths; also the common width
    int nActualLength = 0;
    int nMinLength = 2;
    int nWidth = 0;
    for (i = 0; i < arrSCBars.GetSize(); i++)
    {
        pBar = arrSCBars[i];

		nActualLength += bHorz ? pBar->m_szHorz.cx  :
			pBar->m_szVert.cy ;
		nMinLength += bHorz ? pBar->m_szMin.cx :
			pBar->m_szMin.cy ;
		nWidth = max(nWidth, bHorz ? pBar->m_szHorz.cy :
			pBar->m_szVert.cx);
    }
    
    // step 3: pop the bar out of the row if not enough room
    if (nMinLength > nLengthAvail)
    {
        if (nFirst < nThis || nThis < nLast)
        {   // not enough room - create a new row
            m_pDockBar->m_arrBars.InsertAt(nLast + 1, this);
            m_pDockBar->m_arrBars.InsertAt(nLast + 1, (CControlBar*) NULL);
            m_pDockBar->m_arrBars.RemoveAt(nThis);
        }
        return FALSE;
    }

    // step 4: make the bars same width
    for (i = 0; i < arrSCBars.GetSize(); i++)
	{
		if (bHorz)
			arrSCBars[i]->m_szHorz.cy = nWidth;
		else
			arrSCBars[i]->m_szVert.cx = nWidth;
	}

    if (nActualLength == nLengthAvail)
        return TRUE; // no change

    // step 5: distribute the difference between the bars, but
    //         don't shrink them below minsize
    int nDelta = nLengthAvail - nActualLength;
    
    while (nDelta != 0)
    {
		int nDeltaOld = nDelta;
        for (i = 0; i < arrSCBars.GetSize(); i++)
        {
            pBar = arrSCBars[i];
            int nLMin = bHorz ? pBar->m_szMin.cx : pBar->m_szMin.cy;
            int nL = bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy;
            
            if ((nL == nLMin) && (nDelta < 0) || // already at min length
                pBar->m_bKeepSize) // or wants to keep its size
			{
                continue;
			}
            
            // sign of nDelta
            int nDelta2 = (nDelta == 0) ? 0 : ((nDelta < 0) ? -1 : 1);
			(bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy) += nDelta2;

            nDelta -= nDelta2;
            if (nDelta == 0) break;
        }

        // clear m_bKeepSize flags
        if ((nDeltaOld == nDelta) || (nDelta == 0))
            for (i = 0; i < arrSCBars.GetSize(); i++)
                arrSCBars[i]->m_bKeepSize = FALSE;
    }

    return TRUE;
}

void CBCGSizingControlBar::AlignControlBars()
{
	if (m_pDockBar == NULL)
	{
		return;
	}

    int nFirst, nLast, nThis;
    GetRowInfo(nFirst, nLast, nThis);

    BOOL bHorz = IsHorzDocked();
    BOOL bNeedRecalc = FALSE;
    int nPos, nAlign = bHorz ? /*-2*/0 : 0;

    CRect rc, rcDock;
    m_pDockBar->GetWindowRect(&rcDock);

	CControlBar* pPrevBar = 0;
    for (int i = nFirst; i <= nLast; i++)
    {
        CControlBar* pBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pBar) == 0) continue; // placeholder
        if (!pBar->IsVisible()) continue;

        pBar->GetWindowRect(&rc);
        rc.OffsetRect(-rcDock.TopLeft());

        if ((nPos = FindSizingBar(pBar)) >= 0)
            rc = CRect(rc.TopLeft(), bHorz ?
                m_arrBars[nPos]->m_szHorz : m_arrBars[nPos]->m_szVert);

        if ((bHorz ? rc.left : rc.top) != nAlign)
        {
            if (!bHorz)
                rc.OffsetRect(0, nAlign - rc.top);
            else if (m_nDockBarID == AFX_IDW_DOCKBAR_TOP)
                rc.OffsetRect(nAlign - rc.left, 0);
            else
                rc.OffsetRect(nAlign - rc.left, 0);

			pBar->SetWindowPos(NULL,
				rc.left, rc.top,
				rc.Width(), rc.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER);

            bNeedRecalc = TRUE;
        }
        nAlign += (bHorz ? rc.Width() : rc.Height()) ;

		//--------------------------------------------------------------------
		// Erwin Tratar: Z-Order must be defined for overlapping control bars 
		// in order to be able to draw correct sizing edges
		//--------------------------------------------------------------------
		if(pPrevBar && bNeedRecalc) {
			pPrevBar->SetWindowPos(pBar,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
		}
		pPrevBar = pBar;
    }

    if (bNeedRecalc)
    {
        m_pDockSite->DelayRecalcLayout();
    }
}

void CBCGSizingControlBar::LoadState(LPCTSTR lpszProfileName)
{
    ASSERT_VALID(this);
    ASSERT(GetSafeHwnd()); // must be called after Create()

	CString strProfileName = ::BCGGetRegPath (strSCBProfile, lpszProfileName);

    CWinApp* pApp = AfxGetApp();

    TCHAR szSection[256];
    wsprintf(szSection, _T("%s-SCBar-%d"), strProfileName,
        GetDlgCtrlID());

    m_szHorz.cx = max(m_szMin.cx, (int) pApp->GetProfileInt(szSection,
        _T("sizeHorzCX"), m_szHorz.cx));
    m_szHorz.cy = max(m_szMin.cy, (int) pApp->GetProfileInt(szSection, 
        _T("sizeHorzCY"), m_szHorz.cy));

    m_szVert.cx = max(m_szMin.cx, (int) pApp->GetProfileInt(szSection, 
        _T("sizeVertCX"), m_szVert.cx));
    m_szVert.cy = max(m_szMin.cy, (int) pApp->GetProfileInt(szSection, 
        _T("sizeVertCY"), m_szVert.cy));

    m_szFloat.cx = max(m_szMin.cx, (int) pApp->GetProfileInt(szSection,
        _T("sizeFloatCX"), m_szFloat.cx));
    m_szFloat.cy = max(m_szMin.cy, (int) pApp->GetProfileInt(szSection,
        _T("sizeFloatCY"), m_szFloat.cy));

	BOOL bMaximized = pApp->GetProfileInt (szSection, _T("Expanded"), FALSE);
	if (bMaximized)
	{
		Expand (TRUE);
	}

	m_bIsRolled = pApp->GetProfileInt(szSection, _T("RolledFloat"), m_bIsRolled);
	m_nRollingType = pApp->GetProfileInt(szSection, _T("RolledFloatDynamicState"), m_nRollingType);

	if (m_nRollingType==BCG_ROLLUP_DYNAMIC_ON || m_nRollingType==BCG_ROLLUP_DYNAMICSIZED_ON) m_bIsRolled=1;

	if (m_nRollingType!=BCG_ROLLUP_NONE)
	{
		CBCGMiniDockFrameWnd* pFrame =
			DYNAMIC_DOWNCAST (CBCGMiniDockFrameWnd, GetParentFrame ());

		if (pFrame != NULL)
		{
			ASSERT_VALID (pFrame);

			pFrame->m_bIsRolled = m_bIsRolled;
			pFrame->m_nRollingType = m_nRollingType;
		}
	}

	if (m_bAutoSaveActiveTab)
	{
		CBCGTabWnd* pTabWnd = GetTabWnd ();
		if (pTabWnd != NULL)
		{
			int nActiveTab = pApp->GetProfileInt (szSection, _T("ActiveTab"), 0);
			pTabWnd->SetActiveTab (nActiveTab);
		}
	}
}
//**************************************************************************************
void CBCGSizingControlBar::SaveState(LPCTSTR lpszProfileName)
{
    // place your SaveState or GlobalSaveState call in
    // CMainFrame::DestroyWindow(), not in OnDestroy()
    ASSERT_VALID(this);
    ASSERT(GetSafeHwnd());

	CString strProfileName = ::BCGGetRegPath (strSCBProfile, lpszProfileName);

    CWinApp* pApp = AfxGetApp();

    TCHAR szSection[256];
    wsprintf(szSection, _T("%s-SCBar-%d"), strProfileName,
        GetDlgCtrlID());

	CSize szHorz = m_szHorz;
	CSize szVert = m_szVert;

	if (m_bMaximized)
	{
		if (IsHorzDocked())
		{
			szHorz = m_szOld;
		}
		else
		{
			szVert = m_szOld;
		}
	}

    pApp->WriteProfileInt(szSection, _T("sizeHorzCX"), szHorz.cx);
    pApp->WriteProfileInt(szSection, _T("sizeHorzCY"), szHorz.cy);

    pApp->WriteProfileInt(szSection, _T("sizeVertCX"), szVert.cx);
    pApp->WriteProfileInt(szSection, _T("sizeVertCY"), szVert.cy);

    pApp->WriteProfileInt(szSection, _T("sizeFloatCX"), m_szFloat.cx);
    pApp->WriteProfileInt(szSection, _T("sizeFloatCY"), m_szFloat.cy);

	pApp->WriteProfileInt(szSection, _T("Expanded"), m_bMaximized);

	if(m_nRollingType != BCG_ROLLUP_NONE){
		pApp->WriteProfileInt(szSection, _T("RolledFloat"), m_bIsRolled);
		pApp->WriteProfileInt(szSection, _T("RolledFloatDynamicState"), m_nRollingType);
	}

	if (m_bAutoSaveActiveTab)
	{
		CBCGTabWnd* pTabWnd = GetTabWnd ();
		if (pTabWnd != NULL)
		{
			pApp->WriteProfileInt (szSection, _T("ActiveTab"), pTabWnd->GetActiveTab ());
		}
	}
}

void CBCGSizingControlBar::GlobalLoadState(LPCTSTR lpszProfileName)
{
    for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
        ((CBCGSizingControlBar*) m_arrBars[i])->LoadState(lpszProfileName);
	}
}

void CBCGSizingControlBar::GlobalSaveState(LPCTSTR lpszProfileName)
{
    for (int i = 0; i < m_arrBars.GetSize(); i++)
        ((CBCGSizingControlBar*) m_arrBars[i])->SaveState(lpszProfileName);
}

/////////////////////////////////////////////////////////////////////////
// CBCGSCBButton

CBCGSCBButton::CBCGSCBButton()
{
    m_bPushed = FALSE;
    m_bFocused = FALSE;
	m_nHit = (UINT) -1;
	m_bHidden = FALSE;
	m_pParentBar = NULL;
	m_clrForeground = (COLORREF)-1;
}

void CBCGSCBButton::Paint(CDC* pDC, BOOL bHorz, BOOL bMaximized, BOOL bDisabled)
{
	ASSERT_VALID (pDC);

	if (m_bHidden)
	{
		return;
	}

	COLORREF clrTextOld = pDC->GetTextColor ();
	pDC->SetBkMode (TRANSPARENT);

	if (bDisabled)
	{
		pDC->SetTextColor (globalData.clrGrayedText);
	}
	else if (m_clrForeground != (COLORREF)-1)
	{
		if (GetRValue (m_clrForeground) <= 192 &&
			GetGValue (m_clrForeground) <= 192 &&
			GetBValue (m_clrForeground) <= 192)
		{
			pDC->SetTextColor (RGB (0, 0, 0));
		}
		else
		{
			pDC->SetTextColor (RGB (255, 255, 255));
		}
	}

	CBCGVisualManager::GetInstance ()->OnDrawCaptionButton (
		pDC, this, bHorz, bMaximized, bDisabled);

	pDC->SetTextColor (clrTextOld);
}
//************************************************************************************
void CBCGSCBButton::DrawIcon (CDC* pDC, CRect rect, BOOL bHorz, BOOL bMaximized, BOOL bDisabled)
{
	COLORREF clrTxt = pDC->GetTextColor ();

	BOOL bIsBlack =
		(GetRValue (clrTxt) <= 192 &&
		GetGValue (clrTxt) <= 192 &&
		GetBValue (clrTxt) <= 192);

	CMenuImages::IMAGES_IDS id;

	if (m_nHit == HTCLOSE_BCG)
	{
		id = bIsBlack ? CMenuImages::IdClose : CMenuImages::IdCloseWhite;
	}
	else
	{
		if (bDisabled)
		{
			id = bHorz ? CMenuImages::IdArowLeftDsbl : CMenuImages::IdArowUpDisabled;
		}
		else
		{
			if (bHorz)
			{
				if (bMaximized)
				{
					id = bIsBlack ? CMenuImages::IdArowLeft : CMenuImages::IdArowLeftWhite;
				}
				else
				{
					id = bIsBlack ? CMenuImages::IdArowRight : CMenuImages::IdArowRightWhite;
				}
			}
			else
			{
				if (bMaximized)
				{
					id = bIsBlack ? CMenuImages::IdArowUp : CMenuImages::IdArowUpWhite;
				}
				else
				{
					id = bIsBlack ? CMenuImages::IdArowDown : CMenuImages::IdArowDownWhite;
				}
			}
		}
	}

	CMenuImages::Draw (pDC, id, rect);
}
//************************************************************************************
BOOL CBCGSizingControlBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!IsFloating ())
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);

		switch (OnNcHitTest (ptCursor))
		{
		case HTLEFT:
		case HTRIGHT:
			::SetCursor (globalData.m_hcurStretch);
			return TRUE;

		case HTTOP:
		case HTBOTTOM:
			::SetCursor (globalData.m_hcurStretchVert);
			return TRUE;
		}
	}
	
	return CControlBar::OnSetCursor(pWnd, nHitTest, message);
}
//*************************************************************************************
BOOL CBCGSizingControlBar::IsAlmostRight ()
{
	if (m_pDockBar == NULL)
	{
		return FALSE;
	}

    int nFirst, nLast, nThis;
    GetRowInfo(nFirst, nLast, nThis);

	return nLast == nThis;
}
//*************************************************************************************
BOOL CBCGSizingControlBar::IsNotFirst ()
{
	if (m_pDockBar == NULL)
	{
		return FALSE;
	}

    int nFirst, nLast, nThis;
    GetRowInfo(nFirst, nLast, nThis);

	return nFirst != nThis;
}
//*************************************************************************************
BOOL CBCGSizingControlBar::IsEdgeVisible (UINT nHitTest)
{
	switch (nHitTest)
    {
    case HTLEFT:
		return (m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT) &&
			(m_dwSCBStyle & SCBS_EDGELEFT);

    case HTTOP:
		if (m_bCaptionAlwaysOnTop)
		{
			return m_dwSCBStyle & SCBS_EDGETOP;
		}

		return (m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM) &&
			(m_dwSCBStyle & SCBS_EDGETOP);

    case HTRIGHT:
		return (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT || !IsAlmostRight ()) &&
			(m_dwSCBStyle & SCBS_EDGERIGHT);

    case HTBOTTOM:
		if (m_bCaptionAlwaysOnTop)
		{
			return m_dwSCBStyle & SCBS_EDGEBOTTOM;
		}

		return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP || !IsAlmostRight () &&
			(m_dwSCBStyle & SCBS_EDGEBOTTOM));

    default:
        ASSERT(FALSE); // invalid hit test code
    }

	return FALSE;
}
//************************************************************************************
void CBCGSizingControlBar::Expand (BOOL bToggle)
{
	if (IsFloating () || m_bIsSingleInRow)
	{
		return;
	}

    BOOL bHorz = IsHorzDocked();

	if (bToggle)
	{
		m_bMaximized = !m_bMaximized;
	}

    CBCGSCBArray arrSCBars;
    GetRowSizingBars(arrSCBars);

    // dirty cast - using CBCGDockBar to access protected CDockBar members
    CBCGDockBar* pDockBar = (CBCGDockBar*) m_pDockBar;

	CRect rc = pDockBar->m_rectLayout;
	if (rc.IsRectEmpty())
	{
		m_pDockSite->GetClientRect(&rc);
	}

	for (int i = 0; i < arrSCBars.GetSize(); i++)
	{
		CBCGSizingControlBar* pBar = arrSCBars[i];

		if (m_bMaximized && pBar != this && pBar->m_bMaximized)
		{
			pBar->m_bMaximized = FALSE;

			CBCGLocalResource localRes;
			pBar->m_wndToolTip.UpdateTipText (IDS_BCGBARRES_EXPAND_BAR, pBar, 2);
			pBar->RedrawButton (pBar->m_Buttons [1]);
		}

		if (m_bMaximized || !bToggle)
		{
			pBar->m_szOld = bHorz ? pBar->m_szHorz : pBar->m_szVert;

			if (pBar == this)
			{
				if (bHorz)
				{
					m_szHorz.cx = rc.Width() + 2;
				}
				else
				{
					m_szVert.cy = rc.Height() - 2;
				}
			}
			else
			{
				if (bHorz)
				{
					pBar->m_szHorz.cx = pBar->m_szMin.cx;
				}
				else
				{
					pBar->m_szVert.cy = pBar->m_szMin.cy;
				}
			}
		}
		else
		{
			if (bHorz)
			{
				pBar->m_szHorz = pBar->m_szOld;
			}
			else
			{
				pBar->m_szVert = pBar->m_szOld;
			}
		}
	}

	{
		CBCGLocalResource localRes;
		m_wndToolTip.UpdateTipText (
			m_bMaximized ? IDS_BCGBARRES_CONTRACT_BAR : IDS_BCGBARRES_EXPAND_BAR,
			this, 2);
	}

    m_pDockSite->DelayRecalcLayout();
}

void CBCGSizingControlBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
    UpdateDialogControls(pTarget, bDisableIfNoHndler);

	if (IsFloating ())
	{
		return;
	}

    CWnd* pFocus = GetFocus();
    BOOL bActiveOld = m_bActive;

    m_bActive = (pFocus->GetSafeHwnd () != NULL && IsChild (pFocus));

    if (m_bActive != bActiveOld)
	{
        SendMessage (WM_NCPAINT);
	}
}

BOOL CBCGSizingControlBar::PreTranslateMessage(MSG* pMsg) 
{
	m_wndToolTip.RelayEvent (pMsg);
	return CControlBar::PreTranslateMessage(pMsg);
}

void CBCGSizingControlBar::SetCustomizationMode (BOOL bCustMode)
{
	ASSERT_VALID (this);

	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		pWndChild->EnableWindow (!bCustMode);
		pWndChild = pWndChild->GetNextWindow ();
	}
}

// Erwin Tratar:
void CBCGSizingControlBar::OnNcLButtonDblClk(UINT nHitTest, CPoint point) 
{
    if (m_pDockBar != NULL)
    {
		if ((nHitTest < HTSIZEFIRST) || (nHitTest > HTSIZELAST))
		{
			// Not on edges,
			// toggle docking
			ASSERT(m_pDockContext != NULL);
			m_pDockContext->ToggleDocking();
		}
    }
    else
	{
        CControlBar::OnNcLButtonDblClk(nHitTest, point);
	}
}

///////////////////////////////////////////////////////////////////////////
void CBCGSizingControlBar::OnNcRButtonUp(UINT nHitTest, CPoint point) 
{
	if (!CBCGToolBar::IsCustomizeMode ())
	{
		CFrameWnd* pParentFrame = (m_pDockSite == NULL) ?
			GetDockingFrame() : m_pDockSite;
		ASSERT_VALID(pParentFrame);

		pParentFrame->SendMessage (BCGM_TOOLBARMENU,
			(WPARAM) GetSafeHwnd (),
			MAKELPARAM(point.x, point.y));
		return;
	}
	
	CControlBar::OnNcRButtonUp(nHitTest, point);
}
//*****************************************************************************************
void CBCGSizingControlBar::SetCaptionStyle (BOOL bDrawText, BOOL /*bForceGradient*/,
											BOOL bHideDisabledButtons,
											BOOL bAlwaysOnTop)
{
	m_bCaptionText = bDrawText;
	m_bHideDisabledButtons = bHideDisabledButtons;

	m_bCaptionGradient = FALSE;
	m_bCaptionAlwaysOnTop = bAlwaysOnTop;

	for (POSITION pos = gAllSizingControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGSizingControlBar* pBar = (CBCGSizingControlBar*) gAllSizingControlBars.GetNext (pos);
		ASSERT (pBar != NULL);

		if (CWnd::FromHandlePermanent (pBar->m_hWnd) != NULL)
		{
			pBar->SetWindowPos (NULL, 0, 0, 0, 0,
				SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			pBar->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}
}
//*****************************************************************************************
void CBCGSizingControlBar::DrawCaption (CDC* pDC, const CRect& rectCaption)
{
	ASSERT_VALID (pDC);

	BOOL bHorz = IsHorzDocked() && !m_bCaptionAlwaysOnTop;

	ASSERT_VALID (pDC);

	CRect rcbtnRight = CRect (rectCaption.BottomRight (), CSize (0, 0));
	int i = 0;

	for (i = CBCGSIZINGCONTROLBAR_BUTTONS_NUM - 1; i >= 0; i --)
	{
		if (!m_Buttons [i].m_bHidden)
		{
			rcbtnRight = m_Buttons [i].GetRect();
			break;
		}
	}

	CRect rcbtnLeft = CRect (rectCaption.TopLeft (), CSize (0, 0));
	for (i = CBCGSIZINGCONTROLBAR_BUTTONS_NUM - 1; i >= 0; i --)
	{
		if (!m_Buttons [i].m_bHidden)
		{
			rcbtnLeft = m_Buttons [i].GetRect();
			break;
		}
	}

	COLORREF clrCptnText = CBCGVisualManager::GetInstance ()->OnDrawControlBarCaption (
		pDC, this, m_bActive, rectCaption, rcbtnRight);

	for (i = 0; i < CBCGSIZINGCONTROLBAR_BUTTONS_NUM; i ++)
	{
		CBCGSCBButton& btn = m_Buttons [i];
		btn.m_clrForeground = clrCptnText;
	}

    int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
    COLORREF clrOldText = pDC->SetTextColor (clrCptnText);

    CFont* pOldFont = pDC->SelectObject
		(bHorz ? &globalData.fontVertCaption : &globalData.fontRegular);

    CString sTitle;
    GetWindowText(sTitle);

    CPoint ptOrg = bHorz ?
        CPoint(rectCaption.left, rectCaption.bottom - 3) :
        CPoint(rectCaption.left + 3, rectCaption.top + 2);

	CRect rectClip = rectCaption;

	if (bHorz)
	{
		rectClip.top = rcbtnRight.bottom;
	}
	else
	{
		rectClip.right = rcbtnLeft.left;
	}

	if (rectClip.Width () > 8 && rectClip.Height () > 8)
	{
		pDC->ExtTextOut (ptOrg.x, ptOrg.y,
			ETO_CLIPPED, rectClip, sTitle, NULL);
	}

    pDC->SelectObject(pOldFont);
    pDC->SetBkMode(nOldBkMode);
    pDC->SetTextColor(clrOldText);
}
//*****************************************************************************************
void CBCGSizingControlBar::RedrawButton (const CBCGSCBButton& btn)
{
	m_rectRedraw = btn.GetRect ();
	SendMessage (WM_NCPAINT);
	m_rectRedraw.SetRectEmpty ();

	UpdateWindow ();
}
//*****************************************************************************************
LRESULT CBCGSizingControlBar::OnSetText (WPARAM /*wParam*/, LPARAM lParam)
{
	LRESULT lResult = Default();

	if (IsFloating () && GetParentFrame () != NULL)
	{
		GetParentFrame ()->SetWindowText ((LPCTSTR) lParam);
	}

	if (m_pDockBar != NULL)
	{
		m_pDockBar->SetWindowText ((LPCTSTR) lParam);
	}

	SendMessage(WM_NCPAINT);
	return lResult;
}
//***************************************************************************************
void CBCGSizingControlBar::EnableRollUp (int nRollingType/* = BCG_ROLLUP_NORMAL*/)
{
	ASSERT (m_pDockContext == NULL);	// "RollUp" controlbar can't be docked


	m_nRollingType = nRollingType;

	if(m_nRollingType==BCG_ROLLUP_DYNAMIC_ON 
		|| m_nRollingType==BCG_ROLLUP_DYNAMICSIZED_ON)
	{
		m_bIsRolled=1;
	}


	CBCGMiniDockFrameWnd* pFrame =
		DYNAMIC_DOWNCAST (CBCGMiniDockFrameWnd, GetParentFrame ());
	if (pFrame != NULL)
	{
		pFrame->m_nRollingType = m_nRollingType;
		pFrame->m_bIsRolled = m_bIsRolled;
		pFrame->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		pFrame->SendMessage(WM_NCPAINT, NULL,NULL);
	}
}
//***************************************************************************************
void CBCGSizingControlBar::SetMinSize (CSize minSize)
{
	m_szMin = minSize;

    // check limits for floated size
	m_szFloat.cx = max(m_szMin.cx, m_szFloat.cx);
	m_szFloat.cy = max(m_szMin.cy, m_szFloat.cy);
}
//***************************************************************************************
void CBCGSizingControlBar::SetMaxSize (CSize maxSize)
{
	m_szMax = maxSize;
    
    // check limits for floated size
    if(m_szMax.cx > 0)
    	m_szFloat.cx = min(m_szMax.cx, m_szFloat.cx);
    if(m_szMax.cy > 0)
    	m_szFloat.cy = min(m_szMax.cy, m_szFloat.cy);    
}
//***************************************************************************************
void CBCGSizingControlBar::ToggleDocking()
{
    if ((m_pDockBar != NULL) && (m_pDockContext != NULL))
    {
        // toggle docking
        m_pDockContext->ToggleDocking();
    }
}
//***************************************************************************************
void CBCGSizingControlBar::SetRollupState(int state)
{
	// added to force unroll of the control bar
	// or to force the control bar to stay unrolled
	CBCGMiniDockFrameWnd* pFrame =
		DYNAMIC_DOWNCAST (CBCGMiniDockFrameWnd, GetParentFrame ());

	if (pFrame == NULL) return;

	if(state==BCG_ROLLUP_STATE_FORCESTAYUNROLL){// force the control bar to stay visible (if unroll, has no direct effect if the bar is rolled). (useful if the controlbar lost focus but must stay visible)
		m_NoRollup=1;
	}else if(state==BCG_ROLLUP_STATE_LETROLL ){// to allow again the controlbar to be rolled 
		m_NoRollup=0;
	}else if(state== BCG_ROLLUP_STATE_UNROLL ){// to unroll (make visible) the controlbar... (it will roll again, as if the user moved the mouse on the caption).


		if((m_nRollingType == BCG_ROLLUP_DYNAMIC_ON 
			|| m_nRollingType == BCG_ROLLUP_DYNAMICSIZED_ON)	
		    	&& AfxGetMainWnd()->IsWindowEnabled()){
		
			m_bIsRolled = 0;
			pFrame->m_bIsRolled = m_bIsRolled;

			pFrame->DelayRecalcLayout();
	
			BringWindowToTop();

			pFrame->SetTimer(GetDlgCtrlID(),250,NULL);
			pFrame->m_RollupDelay=ROLLUP_DELAY;
		}
	}
}
//*********************************************************************************
CBCGTabWnd* CBCGSizingControlBar::GetTabWnd () const
{
	CBCGTabWnd* pTabWnd = NULL;

	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		if (pWndChild->IsKindOf (RUNTIME_CLASS (CBCGTabWnd)))
		{
			if (pTabWnd != NULL)
			{
				return NULL;
			}
			else
			{
				pTabWnd = DYNAMIC_DOWNCAST (CBCGTabWnd, pWndChild);
			}
		}

		pWndChild = pWndChild->GetNextWindow ();
	}

	return pTabWnd;
}

void CBCGSizingControlBar::OnSize(UINT nType, int cx, int cy) 
{
	CControlBar::OnSize(nType, cx, cy);
	ReposCaptionButtons ();
	
}

#endif // BCG_NO_SIZINGBAR

