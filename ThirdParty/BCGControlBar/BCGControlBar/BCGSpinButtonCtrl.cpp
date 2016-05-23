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
//
// BCGSpinButtonCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "trackmouse.h"
#include "BCGVisualManager.h"
#include "BCGSpinButtonCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGSpinButtonCtrl

CBCGSpinButtonCtrl::CBCGSpinButtonCtrl()
{
	m_bTracked = FALSE;

	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;

	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;
}
//*****************************************************************************************
CBCGSpinButtonCtrl::~CBCGSpinButtonCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGSpinButtonCtrl, CSpinButtonCtrl)
	//{{AFX_MSG_MAP(CBCGSpinButtonCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGSpinButtonCtrl message handlers

void CBCGSpinButtonCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect (rect);

	int nState = 0;

	if (m_bIsButtonPressedUp)
	{
		nState |= SPIN_PRESSEDUP;
	}

	if (m_bIsButtonPressedDown)
	{
		nState |= SPIN_PRESSEDDOWN;
	}

	if (m_bIsButtonHighligtedUp)
	{
		nState |= SPIN_HIGHLIGHTEDUP;
	}

	if (m_bIsButtonHighligtedDown)
	{
		nState |= SPIN_HIGHLIGHTEDDOWN;
	}

	if (!IsWindowEnabled ())
	{
		nState |= SPIN_DISABLED;
	}

	CBCGVisualManager::GetInstance ()->OnDrawSpinButtons (
		&dc, rect, nState, FALSE, this);
}
//*****************************************************************************************
void CBCGSpinButtonCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect (rect);

	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint ().y;

	CRect rectDown = rect;
	rectDown.top = rectUp.bottom;

	m_bIsButtonPressedUp = rectUp.PtInRect (point);
	m_bIsButtonPressedDown = rectDown.PtInRect (point);
	
	CSpinButtonCtrl::OnLButtonDown(nFlags, point);
}
//*****************************************************************************************
void CBCGSpinButtonCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;
	
	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;

	CSpinButtonCtrl::OnLButtonUp(nFlags, point);
}
//*****************************************************************************************
void CBCGSpinButtonCtrl::OnCancelMode() 
{
	CSpinButtonCtrl::OnCancelMode();

	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;

	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;
}
//*****************************************************************************************
void CBCGSpinButtonCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	BOOL bIsButtonHighligtedUp = m_bIsButtonHighligtedUp;
	BOOL bIsButtonHighligtedDown = m_bIsButtonHighligtedDown;

	CRect rect;
	GetClientRect (rect);

	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint ().y;

	CRect rectDown = rect;
	rectDown.top = rectUp.bottom;

	m_bIsButtonHighligtedUp = rectUp.PtInRect (point);
	m_bIsButtonHighligtedDown = rectDown.PtInRect (point);

	if (nFlags & MK_LBUTTON)
	{
		m_bIsButtonPressedUp = m_bIsButtonHighligtedUp;
		m_bIsButtonPressedDown = m_bIsButtonHighligtedDown;
	}
	
	CSpinButtonCtrl::OnMouseMove(nFlags, point);

	if (bIsButtonHighligtedUp != m_bIsButtonHighligtedUp ||
		bIsButtonHighligtedDown != m_bIsButtonHighligtedDown)
	{
		RedrawWindow ();
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGTrackMouse (&trackmouseevent);	
	}
}
//*****************************************************************************************
LRESULT CBCGSpinButtonCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_bIsButtonPressedUp || m_bIsButtonPressedDown ||
		m_bIsButtonHighligtedUp || m_bIsButtonHighligtedDown)
	{
		m_bIsButtonHighligtedUp = FALSE;
		m_bIsButtonHighligtedDown = FALSE;

		RedrawWindow ();
	}

	return 0;
}
