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

// ButtonsTextList.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "ButtonsTextList.h"
#include "afxadv.h"
#include "BCGToolbar.h"
#include "BCGToolbarButton.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "globals.h"
#include "BCGVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonsTextList

CButtonsTextList::CButtonsTextList() :
	m_sizeButton (0, 0)
{
}

CButtonsTextList::~CButtonsTextList()
{
}


BEGIN_MESSAGE_MAP(CButtonsTextList, CListBox)
	//{{AFX_MSG_MAP(CButtonsTextList)
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonsTextList message handlers

void CButtonsTextList::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CListBox::OnLButtonDown(nFlags, point);

	int iIndex = GetCurSel ();
	if (iIndex == LB_ERR)
	{
		return;
	}

	//-------------------------------------------
	// Be sure that we realy click into the item!
	//-------------------------------------------
	CRect rect;
	GetItemRect (iIndex, &rect);

	if (!rect.PtInRect (point))
	{
		return;
	}

	//-----------------------------------------------------------
	// Trigger mouse up event (to change selection notification):
	//-----------------------------------------------------------
	SendMessage (WM_LBUTTONUP, nFlags, MAKELPARAM (point.x, point.y));

	//---------------------
	// Get selected button:
	//---------------------
	CBCGToolbarButton* pButton = (CBCGToolbarButton*) GetItemData (iIndex);
	ASSERT_VALID(pButton);

	//--------------------------------------
	// Prepare clipboard data and start drag:
	//--------------------------------------
	COleDataSource srcItem;

	pButton->m_bDragFromCollection = TRUE;
	pButton->PrepareDrag (srcItem);
	pButton->m_bDragFromCollection = FALSE;

	{
		CBCGLocalResource locaRes;
		::SetCursor (AfxGetApp ()->LoadCursor (IDC_BCGBARRES_DELETE));
	}

	srcItem.DoDragDrop
		(DROPEFFECT_COPY|DROPEFFECT_MOVE, &rect, &CBCGToolBar::m_DropSource);
}
//***************************************************************************************
void CButtonsTextList::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
	ASSERT (lpMIS != NULL);

	UINT uiRowHeight = (UINT) m_sizeButton.cy;
	if (lpMIS->itemHeight < uiRowHeight)
	{
		lpMIS->itemHeight = uiRowHeight;
	}
}
//****************************************************************************************
void CButtonsTextList::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID (pDC);

	CRect rect = lpDIS->rcItem;

	if (lpDIS->itemID != (UINT) -1 && lpDIS->itemID != (UINT) LB_ERR)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) GetItemData (lpDIS->itemID);
		ASSERT_VALID(pButton);

		CString strText = pButton->m_strText;
		GetText (lpDIS->itemID, pButton->m_strText);

		CBCGVisualManager::GetInstance ()->OnFillCommandsListBackground (pDC, rect);

		pButton->OnDrawOnCustomizeList (pDC, rect, lpDIS->itemState & ODS_SELECTED);
		pButton->m_strText = strText;
	}
}
//***************************************************************************************
void CButtonsTextList::PreSubclassWindow() 
{
	CListBox::PreSubclassWindow();

	CSize sizeMenuImage = CBCGToolBar::GetMenuImageSize ();

	m_sizeButton = CSize (	sizeMenuImage.cx + 6,
							sizeMenuImage.cy + 6);
}
//***************************************************************************************
BOOL CButtonsTextList::OnEraseBkgnd(CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGVisualManager::GetInstance ()->OnFillCommandsListBackground (pDC, rectClient);
	return TRUE;
}

#endif // BCG_NO_CUSTOMIZATION
