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
// BCGHeaderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGHeaderCtrl.h"
#include "BCGVisualManager.h"
#include "trackmouse.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGHeaderCtrl

CBCGHeaderCtrl::CBCGHeaderCtrl()
{
	m_bIsMousePressed = FALSE;
	m_bMultipleSort = FALSE;
	m_bAscending = TRUE;
	m_nHighlightedItem = -1;
	m_bTracked = FALSE;
	m_bIsDlgControl = FALSE;
}

CBCGHeaderCtrl::~CBCGHeaderCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CBCGHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGHeaderCtrl message handlers

void CBCGHeaderCtrl::OnDrawItem (CDC* pDC, int iItem, CRect rect, BOOL bIsPressed,
								 BOOL bIsHighlighted)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	const int nTextMargin = 5;

	//-------------
	// Draw border:
	//-------------
	CBCGVisualManager::GetInstance ()->OnDrawHeaderCtrlBorder (this, pDC,
		rect, bIsPressed, bIsHighlighted);

	if (iItem < 0)
	{
		return;
	}

	int nSortVal = 0;
	if (m_mapColumnsStatus.Lookup (iItem, nSortVal) &&
		nSortVal != 0)
	{
		//-----------------
		// Draw sort arrow:
		//-----------------
		CRect rectArrow = rect;
		rectArrow.DeflateRect (5, 5);
		rectArrow.left = rectArrow.right - rectArrow.Height ();

		if (bIsPressed)
		{
			rectArrow.right++;
			rectArrow.bottom++;
		}

		rect.right = rectArrow.left - 1;

		int dy2 = (int) (.134 * rectArrow.Width ());
		rectArrow.DeflateRect (0, dy2);

		m_bAscending = nSortVal > 0;
		OnDrawSortArrow (pDC, rectArrow);
	}

	HD_ITEM hdItem;
	memset (&hdItem, 0, sizeof (hdItem));
	hdItem.mask = HDI_FORMAT | HDI_BITMAP | HDI_TEXT | HDI_IMAGE;

	TCHAR szText [256];
	hdItem.pszText = szText;
	hdItem.cchTextMax = 255;

	if (!GetItem (iItem, &hdItem))
	{
		return;
	}

	//-----------------------
	// Draw bitmap and image:
	//-----------------------
	if ((hdItem.fmt & HDF_IMAGE) && hdItem.iImage >= 0) 
	{
		// By Max Khiszinsky:

		//---------------------------------------
		// The column has a image from imagelist:
		//---------------------------------------
		CImageList* pImageList = GetImageList ();
		if (pImageList != NULL)
		{			
			int cx = 0;
			int cy = 0;

			VERIFY (::ImageList_GetIconSize (*pImageList, &cx, &cy));

			CPoint pt = rect.TopLeft ();
			pt.x ++;
			pt.y = (rect.top + rect.bottom - cy) / 2;

			VERIFY (pImageList->Draw (pDC, hdItem.iImage, pt, ILD_NORMAL));

			rect.left += cx;
		}
	}

	if ((hdItem.fmt & (HDF_BITMAP | HDF_BITMAP_ON_RIGHT)) && hdItem.hbm != NULL)
	{
		CBitmap* pBmp = CBitmap::FromHandle (hdItem.hbm);
		ASSERT_VALID (pBmp);

		BITMAP bmp;
		pBmp->GetBitmap (&bmp);

		CRect rectBitmap = rect;
		if (hdItem.fmt & HDF_BITMAP_ON_RIGHT)
		{
			rectBitmap.right--;
			rect.right = rectBitmap.left = rectBitmap.right - bmp.bmWidth;
		}
		else
		{
			rectBitmap.left++;
			rect.left = rectBitmap.right = rectBitmap.left + bmp.bmWidth;
		}

		rectBitmap.top += max (0, (rectBitmap.Height () - bmp.bmHeight) / 2);
		rectBitmap.bottom = rectBitmap.top + bmp.bmHeight;

		pDC->DrawState (rectBitmap.TopLeft (), rectBitmap.Size (), pBmp, DSS_NORMAL);
	}

	//-----------
	// Draw text:
	//-----------
	if (hdItem.fmt & HDF_STRING)
	{
		CRect rectLabel = rect;
		rectLabel.DeflateRect (nTextMargin, 0);

		CString strLabel = hdItem.pszText;

		UINT uiTextFlags = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
		if (hdItem.fmt & HDF_CENTER)
		{
			uiTextFlags |= DT_CENTER;
		}
		else if (hdItem.fmt & HDF_RIGHT)
		{
			uiTextFlags |= DT_RIGHT;
		}

		pDC->DrawText (strLabel, rectLabel, uiTextFlags);
	}
}
//***************************************************************************************
void CBCGHeaderCtrl::SetSortColumn (int iColumn, BOOL bAscending, BOOL bAdd)
{
	ASSERT_VALID (this);

	if (iColumn < 0)
	{
		m_mapColumnsStatus.RemoveAll ();
		return;
	}

	if (bAdd)
	{
		if (!m_bMultipleSort)
		{
			ASSERT (FALSE);
			bAdd = FALSE;
		}
	}

	if (!bAdd)
	{
		m_mapColumnsStatus.RemoveAll ();
	}

	m_mapColumnsStatus.SetAt (iColumn, bAscending ? 1 : -1);
	RedrawWindow ();
}
//***************************************************************************************
void CBCGHeaderCtrl::RemoveSortColumn (int iColumn)
{
	ASSERT_VALID (this);
	m_mapColumnsStatus.RemoveKey (iColumn);
	RedrawWindow ();
}
//***************************************************************************************
BOOL CBCGHeaderCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
//***************************************************************************************
void CBCGHeaderCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rectClip;
	dc.GetClipBox (rectClip);

	CRect rectClient;
	GetClientRect (rectClient);

	CDC*		pDC = &dc;
	BOOL		bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC (&dc) &&
		bmp.CreateCompatibleBitmap (&dc, rectClient.Width (),
								  rectClient.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

	OnFillBackground (pDC);

	CFont* pOldFont = (CFont*) pDC->SelectObject (&globalData.fontRegular);
	ASSERT_VALID (pOldFont);

	pDC->SetTextColor (globalData.clrBtnText);
	pDC->SetBkMode (TRANSPARENT);

	CRect rect;
	GetClientRect(rect);

	CRect rectItem;
	int nCount = GetItemCount ();

	int xMax = 0;

	for (int i = 0; i < nCount; i++)
	{
		//------------------
		// Is item pressed?
		//------------------
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		HDHITTESTINFO hdHitTestInfo;
		hdHitTestInfo.pt = ptCursor;

		int iHit = (int) SendMessage (HDM_HITTEST, 0, (LPARAM) &hdHitTestInfo);

		BOOL bIsHighlighted = iHit == i && (hdHitTestInfo.flags & HHT_ONHEADER);
		BOOL bIsPressed = m_bIsMousePressed && bIsHighlighted;

		GetItemRect (i, rectItem);

		CRgn rgnClip;
		rgnClip.CreateRectRgnIndirect (&rectItem);
		pDC->SelectClipRgn (&rgnClip);

		//-----------
		// Draw item:
		//-----------
		OnDrawItem (pDC, i, rectItem, bIsPressed, m_nHighlightedItem == i);

		pDC->SelectClipRgn (NULL);

		xMax = max (xMax, rectItem.right);
	}

	//--------------------
	// Draw "tail border":
	//--------------------
	if (nCount == 0)
	{
		rectItem = rect;
		rectItem.right++;
	}
	else
	{
		rectItem.left = xMax;
		rectItem.right = rect.right + 1;
	}

	OnDrawItem (pDC, -1, rectItem, FALSE, FALSE);

	pDC->SelectObject (pOldFont);

	if (bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		dc.BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
					   &dcMem, rectClip.left, rectClip.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}
}
//***************************************************************************************
void CBCGHeaderCtrl::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGVisualManager::GetInstance ()->OnFillHeaderCtrlBackground (this, pDC,
		rectClient);
}
//***************************************************************************************
void CBCGHeaderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bIsMousePressed = TRUE;
	CHeaderCtrl::OnLButtonDown(nFlags, point);
}
//***************************************************************************************
void CBCGHeaderCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bIsMousePressed = FALSE;
	CHeaderCtrl::OnLButtonUp(nFlags, point);
}
//***************************************************************************************
void CBCGHeaderCtrl::OnDrawSortArrow (CDC* pDC, CRect rectArrow)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CBCGVisualManager::GetInstance ()->OnDrawHeaderCtrlSortArrow (this, pDC,
		rectArrow, m_bAscending);
}
//*********************************************************************************
void CBCGHeaderCtrl::EnableMultipleSort (BOOL bEnable)
{
	ASSERT_VALID (this);

	if (m_bMultipleSort == bEnable)
	{
		return;
	}

	m_bMultipleSort = bEnable;

	if (!m_bMultipleSort)
	{
		m_mapColumnsStatus.RemoveAll ();

		if (GetSafeHwnd () != NULL)
		{
			RedrawWindow ();
		}
	}
}
//*********************************************************************************
int CBCGHeaderCtrl::GetSortColumn () const
{
	ASSERT_VALID (this);

	if (m_bMultipleSort)
	{
		TRACE0("Call CBCGHeaderCtrl::GetColumnState for muliple sort\n");
		ASSERT (FALSE);
		return -1;
	}

	int nCount = GetItemCount ();
	for (int i = 0; i < nCount; i++)
	{
		int nSortVal = 0;
		if (m_mapColumnsStatus.Lookup (i, nSortVal) &&
			nSortVal != 0)
		{
			return i;
		}
	}

	return -1;
}
//*********************************************************************************
BOOL CBCGHeaderCtrl::IsAscending () const
{
	ASSERT_VALID (this);

	if (m_bMultipleSort)
	{
		TRACE0("Call CBCGHeaderCtrl::GetColumnState for muliple sort\n");
		ASSERT (FALSE);
		return -1;
	}

	int nCount = GetItemCount ();
	int i = 0;

	for (; i < nCount; i++)
	{
		int nSortVal = 0;
		if (m_mapColumnsStatus.Lookup (i, nSortVal) &&
			nSortVal != 0)
		{
			return nSortVal > 0;
		}
	}

	return i;
}
//*********************************************************************************
int CBCGHeaderCtrl::GetColumnState (int iColumn) const
{
	int nSortVal = 0;
	m_mapColumnsStatus.Lookup (iColumn, nSortVal);

	return nSortVal;
}
//**********************************************************************************
void CBCGHeaderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_LBUTTON) == 0)
	{
		HDHITTESTINFO hdHitTestInfo;
		hdHitTestInfo.pt = point;

		int nPrevHighlightedItem = m_nHighlightedItem;
		m_nHighlightedItem = (int) SendMessage (HDM_HITTEST, 0, (LPARAM) &hdHitTestInfo);

		if ((hdHitTestInfo.flags & HHT_ONHEADER) == 0)
		{
			m_nHighlightedItem = -1;
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
		
		if (nPrevHighlightedItem != m_nHighlightedItem)
		{
			RedrawWindow ();
		}
	}
	
	CHeaderCtrl::OnMouseMove(nFlags, point);
}
//*****************************************************************************************
LRESULT CBCGHeaderCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_nHighlightedItem >= 0)
	{
		m_nHighlightedItem = -1;
		RedrawWindow ();
	}

	return 0;
}
//*****************************************************************************************
void CBCGHeaderCtrl::OnCancelMode() 
{
	CHeaderCtrl::OnCancelMode();
	
	if (m_nHighlightedItem >= 0)
	{
		m_nHighlightedItem = -1;
		RedrawWindow ();
	}
}
//********************************************************************************
int CBCGHeaderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CHeaderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CommonInit ();
	
	return 0;
}
//*********************************************************************************
void CBCGHeaderCtrl::PreSubclassWindow() 
{
	CommonInit ();
	CHeaderCtrl::PreSubclassWindow();
}
//********************************************************************************
void CBCGHeaderCtrl::CommonInit ()
{
	ASSERT_VALID (this);

	for (CWnd* pParentWnd = GetParent (); pParentWnd != NULL;
		pParentWnd = pParentWnd->GetParent ())
	{
		if (pParentWnd->IsKindOf (RUNTIME_CLASS (CDialog)))
		{
			m_bIsDlgControl = TRUE;
			break;
		}
	}
}

