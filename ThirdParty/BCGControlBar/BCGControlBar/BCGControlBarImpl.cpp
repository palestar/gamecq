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

// BCGControlBarImpl.cpp: implementation of the CBCGControlBarImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGControlBarImpl.h"
#include "BCGToolBar.h"
#include "BCGVisualManager.h"
#include "globals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFrameWnd* g_pTopLevelFrame = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGControlBarImpl::CBCGControlBarImpl(CControlBar* pBar) :
	m_pBar (pBar)
{
	ASSERT_VALID (m_pBar);
}
//*****************************************************************************************
CBCGControlBarImpl::~CBCGControlBarImpl()
{
}
//****************************************************************************************
void CBCGControlBarImpl::DrawNcArea ()
{
	CWindowDC dc(m_pBar);

	CRect rectClient;
	m_pBar->GetClientRect(rectClient);

	CRect rectWindow;
	m_pBar->GetWindowRect(rectWindow);

	m_pBar->ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect (rectClient);

	BOOL bRTL = m_pBar->GetExStyle() & WS_EX_LAYOUTRTL;

	{
		MSG* pMsg = &AfxGetThreadState()->m_lastSentMsg;

		ASSERT (pMsg->hwnd == m_pBar->m_hWnd);
		ASSERT (pMsg->message == WM_NCPAINT);

		CRgn* pRgn = NULL;
		if (pMsg->wParam != 1 && 
			(pRgn = CRgn::FromHandle ((HRGN) pMsg->wParam)) != NULL)
		{
			CRect rect;
			m_pBar->GetWindowRect (rect);

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

	// draw borders in non-client area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	CBCGVisualManager::GetInstance ()->OnDrawBarBorder (&dc, m_pBar, rectWindow);

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	CBCGVisualManager::GetInstance ()->OnFillBarBackground  (&dc, m_pBar, rectWindow,
		CRect (0, 0, 0, 0), TRUE /* NC area */);

	// draw gripper in non-client area
	if ((m_pBar->m_dwStyle & (CBRS_GRIPPER|CBRS_FLOATING)) != CBRS_GRIPPER)
	{
		dc.SelectClipRgn (NULL);
		return;
	}

	BOOL bHorz = (m_pBar->m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;
	CRect rectGripper;
	
	m_pBar->GetWindowRect (&rectGripper);

	CRect rcClient;
	m_pBar->GetClientRect(&rcClient);
	m_pBar->ClientToScreen(&rcClient);

	if(bHorz) 
	{
		if (bRTL)
			rectGripper.right = min( rectGripper.right, rectGripper.left + rectGripper.right - rcClient.right);
		else
			rectGripper.right = min( rectGripper.right, rcClient.left - 1);
	}
	else 
	{
		rectGripper.bottom = min( rectGripper.bottom, rcClient.top - 1);
	}
	////////////////////////////////////////////////////////

	rectGripper.OffsetRect (-rectGripper.left, -rectGripper.top);
	CBCGVisualManager::GetInstance ()->OnDrawBarGripper (
		&dc, rectGripper, bHorz, m_pBar);
	dc.SelectClipRgn (NULL);
}
//**********************************************************************************************

#define CX_GRIPPER  3
#define CX_BORDER_GRIPPER 2

void CBCGControlBarImpl::CalcNcSize (NCCALCSIZE_PARAMS FAR* lpncsp)
{
	ASSERT_VALID(m_pBar);

	CRect rect; rect.SetRectEmpty();
	BOOL bHorz = (m_pBar->m_dwStyle & CBRS_ORIENT_HORZ) != 0;

	m_pBar->CalcInsideRect(rect, bHorz);
	if (bHorz && (m_pBar->GetExStyle() & WS_EX_LAYOUTRTL) && ((m_pBar->m_dwStyle & (CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER))
	{
		rect.OffsetRect(-(CX_BORDER_GRIPPER+CX_GRIPPER+CX_BORDER_GRIPPER), 0);
	}

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;

	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}
//*************************************************************************************
BOOL CBCGControlBarImpl::GetBackgroundFromParent (CDC* pDC)
{
	return globalData.DrawParentBackground (m_pBar, pDC);
}
