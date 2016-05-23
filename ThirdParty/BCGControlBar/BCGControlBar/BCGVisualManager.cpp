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

// BCGVisualManager.cpp: implementation of the CBCGVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "multimon.h"
#include "bcgcontrolbar.h"
#include "BCGVisualManager.h"
#include "BCGToolbarButton.h"
#include "bcgoutlookbar.h"
#include "BCGOutlookButton.h"
#include "globals.h"
#include "BCGSizingControlBar.h"
#include "BCGToolBar.h"
#include "BCGCaptionBar.h"
#include "BCGTabWnd.h"
#include "BCGDrawManager.h"
#include "BCGShowAllButton.h"
#include "BCGOutlookButton.h"
#include "BCGStatusBar.h"
#include "BCGTasksPane.h"
#include "BCGHeaderCtrl.h"
#include "BCGSpinButtonCtrl.h"
#include "BCGPopupWindow.h"
#include "BCGPropList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE (CBCGVisualManager, CObject)

extern CObList	gAllToolbars;
extern CObList	gAllSizingControlBars;

CBCGVisualManager*	CBCGVisualManager::m_pVisManager = NULL;
CRuntimeClass*		CBCGVisualManager::m_pRTIDefault = NULL;

#if (defined(SCHEMA_STRINGS)) || (! defined(TMSCHEMA_H))

#define TVP_GLYPH		2
#define GLPS_CLOSED		1
#define GLPS_OPENED		2

#define SBP_SIZEBOX		10
#define SZB_RIGHTALIGN	1
#define SZB_LEFTALIGN	2

#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGVisualManager::CBCGVisualManager(BOOL bIsTemporary)
{
	m_bAutoDestroy = FALSE;
	m_bIsTemporary = bIsTemporary;

	if (!bIsTemporary)
	{
		if (m_pVisManager != NULL)
		{
			ASSERT (FALSE);
		}
		else
		{
			m_pVisManager = this;
		}
	}

	m_bLook2000 = FALSE;
	m_bMenuFlatLook = FALSE;
	m_nMenuShadowDepth = 6;
	m_bShadowHighlightedImage = FALSE;
	m_bEmbossDisabledImage = TRUE;
	m_bFadeInactiveImage = FALSE;
	m_bEnableToolbarButtonFill = TRUE;

	m_nVertMargin = 12;
	m_nHorzMargin = 12;
	m_nGroupVertOffset = 15;
	m_nGroupCaptionHeight = 25;
	m_nGroupCaptionHorzOffset = 13;
	m_nGroupCaptionVertOffset = 7;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;

	m_bOfficeXPStyleMenus = FALSE;
	m_nMenuBorderSize = 2;
	m_bIsOutlookToolbarHotBorder = TRUE;

	m_b3DTabWideBorder = TRUE;
	m_bAlwaysFillTab = FALSE;

	if (!bIsTemporary)
	{
		globalData.UpdateSysColors ();
	}

	OnUpdateSystemColors ();
}
//*************************************************************************************
CBCGVisualManager::~CBCGVisualManager()
{
	if (!m_bIsTemporary)
	{
		m_pVisManager = NULL;
	}
}
//*************************************************************************************
void CBCGVisualManager::SetDefaultManager (CRuntimeClass* pRTI)
{
	if (pRTI != NULL &&
		!pRTI->IsDerivedFrom (RUNTIME_CLASS (CBCGVisualManager)))
	{
		ASSERT (FALSE);
		return;
	}

	m_pRTIDefault = pRTI;

	if (m_pVisManager != NULL)
	{
		ASSERT_VALID (m_pVisManager);

		delete m_pVisManager;
		m_pVisManager = NULL;
	}

	globalData.UpdateSysColors ();

	//---------------------
	// Adjust all toolbars:
	//---------------------
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGToolBar, gAllToolbars.GetNext (posTlb));
		if (pToolBar != NULL)
		{
			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);
				pToolBar->OnChangeVisualManager ();
			}
		}
	}

	RedrawAll ();
}
//*************************************************************************************
void CBCGVisualManager::RedrawAll ()
{
	if (AfxGetMainWnd () != NULL)
	{
		AfxGetMainWnd ()->RedrawWindow (NULL, NULL,
					RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CControlBar* pToolBar = (CControlBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID (pToolBar);
			
			pToolBar->RedrawWindow (NULL, NULL,
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	for (POSITION posCb = gAllSizingControlBars.GetHeadPosition (); posCb != NULL;)
	{
		CControlBar* pBar = (CControlBar*) gAllSizingControlBars.GetNext (posCb);
		ASSERT (pBar != NULL);

		if (CWnd::FromHandlePermanent (pBar->m_hWnd) != NULL)
		{
			ASSERT_VALID (pBar);

			pBar->RedrawWindow (NULL, NULL,
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}
}
//*************************************************************************************
void CBCGVisualManager::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CControlBar* pBar)
{
	ASSERT_VALID (pDC);

	const BOOL bSingleGripper = m_bLook2000;

	const int iGripperSize = 3;
	const int iGripperOffset = bSingleGripper ? 0 : 1;
	const int iLinesNum = bSingleGripper ? 1 : 2;

	BOOL bSideBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGSizingControlBar));

	if (bSideBar)
	{
		if (bHorz)
		{
			rectGripper.left = rectGripper.CenterPoint ().x - iGripperSize;
			rectGripper.right = rectGripper.left = + 2 * iGripperSize;
		}
		else
		{
			rectGripper.top = rectGripper.CenterPoint ().y - iGripperSize;
			rectGripper.bottom = rectGripper.top = + 2 * iGripperSize;
		}
	}

	if (bHorz)
	{
		//-----------------
		// Gripper at left:
		//-----------------
		rectGripper.DeflateRect (0, bSingleGripper ? 3 : 2);

		// ET: Center the grippers
		rectGripper.left = iGripperOffset + rectGripper.CenterPoint().x - 
			( iLinesNum*iGripperSize + (iLinesNum-1)*iGripperOffset) / 2;

		rectGripper.right = rectGripper.left + iGripperSize;

 
		for (int i = 0; i < iLinesNum; i ++)
		{
			pDC->Draw3dRect (rectGripper, 
							globalData.clrBarHilite,
							globalData.clrBarShadow);

			// ET: not used for NewFlat look
			if(! bSingleGripper ) {
				//-----------------------------------
				// To look same as MS Office Gripper!
				//-----------------------------------
				pDC->SetPixel (CPoint (rectGripper.left, rectGripper.bottom - 1),
								globalData.clrBarHilite);
			}

			rectGripper.OffsetRect (iGripperSize+1, 0);
		}
	} 
	else 
	{
		//----------------
		// Gripper at top:
		//----------------
		rectGripper.top += iGripperOffset;
		rectGripper.DeflateRect (bSingleGripper ? 3 : 2, 0);

		// ET: Center the grippers
		rectGripper.top = iGripperOffset + rectGripper.CenterPoint().y - 
			( iLinesNum*iGripperSize + (iLinesNum-1)) / 2;

		rectGripper.bottom = rectGripper.top + iGripperSize;

		for (int i = 0; i < iLinesNum; i ++)
		{
			pDC->Draw3dRect (rectGripper,
							globalData.clrBarHilite,
							globalData.clrBarShadow);

			// ET: not used for NewFlat look
			if(! bSingleGripper ) {
				//-----------------------------------
				// To look same as MS Office Gripper!
				//-----------------------------------
				pDC->SetPixel (CPoint (rectGripper.right - 1, rectGripper.top),
								globalData.clrBarHilite);
			}

			rectGripper.OffsetRect (0, iGripperSize+1);
		}
	}
}
//*************************************************************************************
void CBCGVisualManager::SetLook2000 (BOOL bLook2000)
{
	m_bLook2000 = bLook2000;

	if (AfxGetMainWnd () != NULL)
	{
		AfxGetMainWnd()->RedrawWindow (NULL, NULL,
			RDW_INVALIDATE | RDW_ERASENOW | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_FRAME);
	}

	RedrawAll ();
}
//*************************************************************************************
void CBCGVisualManager::OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL /*bNCArea*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (DYNAMIC_DOWNCAST (CReBar, pBar) != NULL ||
		DYNAMIC_DOWNCAST (CReBar, pBar->GetParent ()))
	{
		FillRebarPane (pDC, pBar, rectClient);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGOutlookBar)))
	{
		((CBCGOutlookBar*) pBar)->FillWorkArea (pDC, rectClient);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGCaptionBar)))
	{
		CBCGCaptionBar* pCaptionBar = (CBCGCaptionBar*) pBar;

		pDC->FillSolidRect	(rectClip, pCaptionBar->m_clrBarBackground == -1 ? 
			globalData.clrBarShadow : pCaptionBar->m_clrBarBackground);
		return;
	}

	// By default, control bar background is filled by 
	// the system 3d background color

	pDC->FillRect (rectClip.IsRectEmpty () ? rectClient : rectClip, 
				&globalData.brBarFace);
}
//*************************************************************************************
void CBCGVisualManager::OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	DWORD dwStyle = pBar->m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
		return;

	COLORREF clrBckOld = pDC->GetBkColor ();	// FillSolidRect changes it

	COLORREF clr = (pBar->m_dwStyle & CBRS_BORDER_3D) ? globalData.clrBarHilite : globalData.clrBarShadow;
	if(pBar->m_dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, clr);
	if(pBar->m_dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.Width()-1 , 1, clr);
	if(pBar->m_dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect.right, 0/*RGL~:1*/, -1,
			rect.Height()/*RGL-: - 1*/, globalData.clrBarShadow);	
	if(pBar->m_dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect.bottom, rect.Width()-1, -1, globalData.clrBarShadow);

	// if undockable toolbar at top of frame, apply special formatting to mesh
	// properly with frame menu
	if(!pBar->m_pDockContext) 
	{
		pDC->FillSolidRect(0,0,rect.Width(),1,globalData.clrBarShadow);
		pDC->FillSolidRect(0,1,rect.Width(),1,globalData.clrBarHilite);
	}

	if (dwStyle & CBRS_BORDER_LEFT)
		++rect.left;
	if (dwStyle & CBRS_BORDER_TOP)
		++rect.top;
	if (dwStyle & CBRS_BORDER_RIGHT)
		--rect.right;
	if (dwStyle & CBRS_BORDER_BOTTOM)
		--rect.bottom;

	// Restore Bk color:
	pDC->SetBkColor (clrBckOld);
}
//************************************************************************************
void CBCGVisualManager::OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* /*pMenu*/, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rect, globalData.clrBarLight, globalData.clrBarDkShadow);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarShadow);
}
//************************************************************************************
void CBCGVisualManager::OnFillButtonInterior (CDC* pDC,
				CBCGToolbarButton* pButton, CRect rect,
				CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsKindOf (RUNTIME_CLASS (CBCGShowAllButton)))
	{
		if (state == ButtonsIsHighlighted)
		{
			CBCGDrawManager dm (*pDC);
			dm.HighlightRect (rect);
		}

		return;
	}

	if (!m_bEnableToolbarButtonFill)
	{
		BOOL bIsPopupMenu = FALSE;

		CBCGToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
		if (pMenuButton != NULL)
		{
			bIsPopupMenu = pMenuButton->GetParentWnd () != NULL &&
				pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));
		}

		if (!bIsPopupMenu)
		{
			return;
		}
	}

	if (!pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton)) &&
		!CBCGToolBar::IsCustomizeMode () && state != ButtonsIsHighlighted &&
		(pButton->m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
	{
		CRect rectDither = rect;
		rectDither.InflateRect (-afxData.cxBorder2, -afxData.cyBorder2);

		CBCGToolBarImages::FillDitheredRect (pDC, rectDither);
	}
}
//************************************************************************************
void CBCGVisualManager::OnHighlightMenuItem (CDC*pDC, CBCGToolbarMenuButton* /*pButton*/,
											CRect rect, COLORREF& /*clrText*/)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &globalData.brHilite);
}
//************************************************************************************
COLORREF CBCGVisualManager::GetHighlightedMenuItemTextColor (CBCGToolbarMenuButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		return globalData.clrGrayedText;
	}

	return globalData.clrTextHilite;
}
//************************************************************************************
void CBCGVisualManager::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	CBCGDrawManager dm (*pDC);
	dm.HighlightRect (rectRarelyUsed);

	pDC->Draw3dRect (rectRarelyUsed, globalData.clrBarShadow, globalData.clrBarHilite);
}
//************************************************************************************
void CBCGVisualManager::OnDrawMenuCheck (CDC* pDC, CBCGToolbarMenuButton* /*pButton*/,
		CRect rectCheck, BOOL /*bHighlight*/, BOOL bIsRadio)
{
	ASSERT_VALID (pDC);

	int iImage = bIsRadio ? CMenuImages::IdRadio : CMenuImages::IdCheck;
	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rectCheck);
}
//************************************************************************************
void CBCGVisualManager::OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		return;
	}

	BOOL bIsOutlookButton = pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton));
	COLORREF clrDark = bIsOutlookButton ? 
					   globalData.clrBarDkShadow : globalData.clrBarShadow;

	switch (state)
	{
	case ButtonsIsPressed:
		pDC->Draw3dRect (&rect, clrDark, globalData.clrBarHilite);
		return;

	case ButtonsIsHighlighted:
		pDC->Draw3dRect (&rect, globalData.clrBarHilite, clrDark);
		return;
	}
}
//*************************************************************************************
void CBCGVisualManager::OnDrawButtonSeparator (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state,
		BOOL /*bHorz*/)
{
	ASSERT_VALID (pButton);

	if (!m_bMenuFlatLook || !pButton->IsDroppedDown ())
	{
		OnDrawButtonBorder (pDC, pButton, rect, state);
	}
}
//**********************************************************************************
void CBCGVisualManager::OnDrawSeparator (CDC* pDC, CControlBar* /*pBar*/,
										 CRect rect, BOOL bHorz)
{
	CRect rectSeparator = rect;

	if (bHorz)
	{
		rectSeparator.left += rectSeparator.Width () / 2 - 1;
		rectSeparator.right = rectSeparator.left + 2;
	}
	else
	{
		rectSeparator.top += rectSeparator.Height () / 2 - 1;
		rectSeparator.bottom = rectSeparator.top + 2;
	}

	pDC->Draw3dRect (rectSeparator, globalData.clrBarShadow,
									globalData.clrBarHilite);
}
//***************************************************************************************
void CBCGVisualManager::OnDrawCaptionButton (
						CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz,
						BOOL bMaximized, BOOL bDisabled)
{
	ASSERT_VALID (pDC);
    CRect rc = pButton->GetRect();

	CRect rectIcon = rc;
	if (pButton->m_bPushed && pButton->m_bFocused)
	{
		rectIcon.left += 2;
		rectIcon.top += 2;
	}

	pButton->DrawIcon (pDC, rectIcon, bHorz, bMaximized, bDisabled);

	if (pButton->m_bPushed && pButton->m_bFocused)
	{
		pDC->Draw3dRect (rc, globalData.clrBarDkShadow, globalData.clrBarHilite);
	}
	else if (pButton->m_bFocused || pButton->m_bPushed)
	{
		pDC->Draw3dRect (rc, globalData.clrBarHilite, globalData.clrBarDkShadow);
	}
}
//***********************************************************************************
void CBCGVisualManager::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGTabWnd* /*pTabWnd*/)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &globalData.brBarFace);
}
//***********************************************************************************
void CBCGVisualManager::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);

	CRect rectClip;
	pDC->GetClipBox (rectClip);

	if (pTabWnd->IsFlatTab ())
	{
		//----------------
		// Draw tab edges:
		//----------------
		#define FLAT_POINTS_NUM	4
		POINT pts [FLAT_POINTS_NUM];

		const int nHalfHeight = pTabWnd->GetTabsHeight () / 2;

		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
		{
			rectTab.bottom --;

			pts [0].x = rectTab.left;
			pts [0].y = rectTab.top;

			pts [1].x = rectTab.left + nHalfHeight;
			pts [1].y = rectTab.bottom;

			pts [2].x = rectTab.right - nHalfHeight;
			pts [2].y = rectTab.bottom;

			pts [3].x = rectTab.right;
			pts [3].y = rectTab.top;
		}
		else
		{
			rectTab.top ++;

			pts [0].x = rectTab.left + nHalfHeight;
			pts [0].y = rectTab.top;

			pts [1].x = rectTab.left;
			pts [1].y = rectTab.bottom;

			pts [2].x = rectTab.right;
			pts [2].y = rectTab.bottom;

			pts [3].x = rectTab.right - nHalfHeight;
			pts [3].y = rectTab.top;

			rectTab.left += 2;
		}

		CBrush* pOldBrush = NULL;
		CBrush br (clrTab);

		if (!bIsActive && clrTab != (COLORREF)-1)
		{
			pOldBrush = pDC->SelectObject (&br);
		}

		pDC->Polygon (pts, FLAT_POINTS_NUM);

		if (pOldBrush != NULL)
		{
			pDC->SelectObject (pOldBrush);
		}
	}
	else if (pTabWnd->IsLeftRightRounded ())
	{
		CList<POINT, POINT> pts;

		POSITION posLeft = pts.AddHead (CPoint (rectTab.left, rectTab.top));
		posLeft = pts.InsertAfter (posLeft, CPoint (rectTab.left, rectTab.top + 2));

		POSITION posRight = pts.AddTail (CPoint (rectTab.right, rectTab.top));
		posRight = pts.InsertBefore (posRight, CPoint (rectTab.right, rectTab.top + 2));

		int xLeft = rectTab.left + 1;
		int xRight = rectTab.right - 1;

		int y = 0;

		for (y = rectTab.top + 2; y < rectTab.bottom - 4; y += 2)
		{
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y));
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y + 2));

			posRight = pts.InsertBefore (posRight, CPoint (xRight, y));
			posRight = pts.InsertBefore (posRight, CPoint (xRight, y + 2));

			xLeft++;
			xRight--;
		}

		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_TOP)
		{
			xLeft--;
			xRight++;
		}

		const int nTabLeft = xLeft - 1;
		const int nTabRight = xRight + 1;

		for (;y < rectTab.bottom - 1; y++)
		{
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y));
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 1, y + 1));

			posRight = pts.InsertBefore (posRight, CPoint (xRight, y));
			posRight = pts.InsertBefore (posRight, CPoint (xRight - 1, y + 1));

			if (y == rectTab.bottom - 2)
			{
				posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 1, y + 1));
				posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 3, y + 1));

				posRight = pts.InsertBefore (posRight, CPoint (xRight, y + 1));
				posRight = pts.InsertBefore (posRight, CPoint (xRight - 2, y + 1));
			}

			xLeft++;
			xRight--;
		}

		posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 2, rectTab.bottom));
		posRight = pts.InsertBefore (posRight, CPoint (xRight - 2, rectTab.bottom));

		LPPOINT points = new POINT [pts.GetCount ()];

		int i = 0;

		for (POSITION pos = pts.GetHeadPosition (); pos != NULL; i++)
		{
			points [i] = pts.GetNext (pos);

			if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_TOP)
			{
				points [i].y = rectTab.bottom - (points [i].y - rectTab.top);
			}
		}

		CRgn rgnClip;
		rgnClip.CreatePolygonRgn (points, (int) pts.GetCount (), WINDING);

		pDC->SelectClipRgn (&rgnClip);

		CBrush br (clrTab == (COLORREF)-1 ? globalData.clrBtnFace : clrTab);
		OnFillTab (pDC, rectTab, &br, iTab, bIsActive, pTabWnd);

		pDC->SelectClipRgn (NULL);

		CPen pen (PS_SOLID, 1, globalData.clrBarShadow);
		CPen* pOLdPen = pDC->SelectObject (&pen);

		for (i = 0; i < pts.GetCount (); i++)
		{
			if ((i % 2) != 0)
			{
				int x1 = points [i - 1].x;
				int y1 = points [i - 1].y;

				int x2 = points [i].x;
				int y2 = points [i].y;

				if (x1 > rectTab.CenterPoint ().x && x2 > rectTab.CenterPoint ().x)
				{
					x1--;
					x2--;
				}

				if (y2 >= y1)
				{
					pDC->MoveTo (x1, y1);
					pDC->LineTo (x2, y2);
				}
				else
				{
					pDC->MoveTo (x2, y2);
					pDC->LineTo (x1, y1);
				}
			}
		}

		delete [] points;
		pDC->SelectObject (pOLdPen);

		rectTab.left = nTabLeft;
		rectTab.right = nTabRight;
	}
	else	// 3D Tab
	{
		CRgn rgnClip;

		CRect rectClip;
		pTabWnd->GetTabsRect (rectClip);

		BOOL bIsCutted = FALSE;

		const BOOL bIsOneNote = pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ();

		const int nExtra = bIsOneNote ?
			((iTab == 0 || bIsActive || pTabWnd->IsVS2005Style ()) ? 
			0 : rectTab.Height ()) : 0;

		if (rectTab.left + nExtra + 10 > rectClip.right ||
			rectTab.right - 10 <= rectClip.left)
		{
			return;
		}

		const int iVertOffset = 2;
		const int iHorzOffset = 2;
		const BOOL bIs2005 = pTabWnd->IsVS2005Style ();

		#define POINTS_NUM	8
		POINT pts [POINTS_NUM];

		if (!bIsActive || bIsOneNote || clrTab != (COLORREF)-1 || m_bAlwaysFillTab)
		{
			if (clrTab != (COLORREF)-1 || bIsOneNote || m_bAlwaysFillTab)
			{
				CRgn rgn;
				CBrush br (clrTab == (COLORREF)-1 ? globalData.clrBtnFace : clrTab);

				CRect rectFill = rectTab;

				if (bIsOneNote)
				{
					CRect rectFill = rectTab;

					const int nHeight = rectFill.Height ();

					pts [0].x = rectFill.left;
					pts [0].y = rectFill.bottom;

					pts [1].x = bIs2005 ? rectFill.left : rectFill.left + 2;
					pts [1].y = bIs2005 ? rectFill.bottom : rectFill.bottom - 1;

					pts [2].x = bIs2005 ? rectFill.left + 2 : rectFill.left + 4;
					pts [2].y = bIs2005 ? rectFill.bottom : rectFill.bottom - 2;
					
					pts [3].x = rectFill.left + nHeight;
					pts [3].y = rectFill.top + 2;
					
					pts [4].x = rectFill.left + nHeight + 4;
					pts [4].y = rectFill.top;
					
					pts [5].x = rectFill.right - 2;
					pts [5].y = rectFill.top;
					
					pts [6].x = rectFill.right;
					pts [6].y = rectFill.top + 2;

					pts [7].x = rectFill.right;
					pts [7].y = rectFill.bottom;

					for (int i = 0; i < POINTS_NUM; i++)
					{
						if (pts [i].x > rectClip.right)
						{
							pts [i].x = rectClip.right;
							bIsCutted = TRUE;
						}

						if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
						{
							pts [i].y = rectFill.bottom - pts [i].y + rectFill.top - 1;
						}
					}

					rgn.CreatePolygonRgn (pts, POINTS_NUM, WINDING);
					pDC->SelectClipRgn (&rgn);
				}
				else
				{
					rectFill.DeflateRect (1, 0);

					if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
					{
						rectFill.bottom--;
					}
					else
					{
						rectFill.top++;
					}

					rectFill.right = min (rectFill.right, rectClip.right);
				}

				OnFillTab (pDC, rectFill, &br, iTab, bIsActive, pTabWnd);
				pDC->SelectClipRgn (NULL);

				if (bIsOneNote)
				{
					CRect rectLeft;
					pTabWnd->GetClientRect (rectLeft);
					rectLeft.right = rectClip.left - 1;

					pDC->ExcludeClipRect (rectLeft);

					if (iTab > 0 && !bIsActive && iTab != pTabWnd->GetFirstVisibleTabNum ())
					{
						CRect rectLeftTab = rectClip;
						rectLeftTab.right = rectFill.left + rectFill.Height () - 10;

						const int nVertOffset = bIs2005 ? 2 : 1;

						if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
						{
							rectLeftTab.top -= nVertOffset;
						}
						else
						{
							rectLeftTab.bottom += nVertOffset;
						}

						pDC->ExcludeClipRect (rectLeftTab);
					}

					pDC->Polyline (pts, POINTS_NUM);

					if (bIsCutted)
					{
						pDC->MoveTo (rectClip.right, rectTab.top);
						pDC->LineTo (rectClip.right, rectTab.bottom);
					}

					CRect rectRight = rectClip;
					rectRight.left = rectFill.right;

					pDC->ExcludeClipRect (rectRight);
				}
			}
		}

		CPen penLight (PS_SOLID, 1, globalData.clrBarHilite);
		CPen penShadow (PS_SOLID, 1, globalData.clrBarShadow);
		CPen penDark (PS_SOLID, 1, globalData.clrBarDkShadow);

		CPen* pOldPen = NULL;

		if (bIsOneNote)
		{
			pOldPen = (CPen*) pDC->SelectObject (&penLight);
			ASSERT(pOldPen != NULL);

			if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
			{
				if (!bIsCutted)
				{
					int yTop = bIsActive ? pts [7].y - 1 : pts [7].y;

					pDC->MoveTo (pts [6].x - 1, pts [6].y);
					pDC->LineTo (pts [7].x - 1, yTop);
				}
			}
			else
			{
				pDC->MoveTo (pts [2].x - 1, pts [2].y + 1);
				pDC->LineTo (pts [2].x, pts [2].y + 1);

				pDC->MoveTo (pts [2].x, pts [2].y + 1);
				pDC->LineTo (pts [3].x + 1, pts [3].y);

				pDC->MoveTo (pts [3].x + 1, pts [3].y);
				pDC->LineTo (pts [3].x + 2, pts [3].y);

				pDC->MoveTo (pts [3].x + 2, pts [3].y);
				pDC->LineTo (pts [3].x + 3, pts [3].y);

				pDC->MoveTo (pts [4].x - 1, pts [4].y + 1);
				pDC->LineTo (pts [5].x + 1, pts [5].y + 1);

				if (!bIsActive && !bIsCutted && m_b3DTabWideBorder)
				{
					pDC->SelectObject (&penShadow);

					pDC->MoveTo (pts [6].x - 2, pts [6].y - 1);
					pDC->LineTo (pts [6].x - 1, pts [6].y - 1);
				}

				pDC->MoveTo (pts [6].x - 1, pts [6].y);
				pDC->LineTo (pts [7].x - 1, pts [7].y);
			}
		}
		else
		{
			if (rectTab.right > rectClip.right)
			{
				CRect rectTabClip = rectTab;
				rectTabClip.right = rectClip.right;

				rgnClip.CreateRectRgnIndirect (&rectTabClip);
				pDC->SelectClipRgn (&rgnClip);
			}

			if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
			{
				pOldPen = (CPen*) pDC->SelectObject (&penLight);
				ASSERT(pOldPen != NULL);

				if (!m_b3DTabWideBorder)
				{
					pDC->SelectObject(&penShadow);
				}

				pDC->MoveTo (rectTab.left, rectTab.top);
				pDC->LineTo (rectTab.left, rectTab.bottom - iVertOffset);

				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject (&penDark);
				}

				pDC->LineTo (rectTab.left + iHorzOffset, rectTab.bottom);
				pDC->LineTo (rectTab.right - iHorzOffset, rectTab.bottom);
				pDC->LineTo (rectTab.right, rectTab.bottom - iVertOffset);
				pDC->LineTo (rectTab.right, rectTab.top - 1);

				pDC->SelectObject(&penShadow);

				if (m_b3DTabWideBorder)
				{
					pDC->MoveTo (rectTab.left + iHorzOffset + 1, rectTab.bottom - 1);
					pDC->LineTo (rectTab.right - iHorzOffset, rectTab.bottom - 1);
					pDC->LineTo (rectTab.right - 1, rectTab.bottom - iVertOffset);
					pDC->LineTo (rectTab.right - 1, rectTab.top - 1);
				}
			}
			else
			{
				pOldPen = pDC->SelectObject (
					m_b3DTabWideBorder ? &penDark : &penShadow);

				ASSERT(pOldPen != NULL);

				pDC->MoveTo (rectTab.right, bIsActive ? rectTab.bottom : rectTab.bottom - 1);
				pDC->LineTo (rectTab.right, rectTab.top + iVertOffset);
				pDC->LineTo (rectTab.right - iHorzOffset, rectTab.top);
				
				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject (&penLight);
				}
				
				pDC->LineTo (rectTab.left + iHorzOffset, rectTab.top);
				pDC->LineTo (rectTab.left, rectTab.top + iVertOffset);

				pDC->LineTo (rectTab.left, rectTab.bottom);

				if (m_b3DTabWideBorder)
				{
					pDC->SelectObject (&penShadow);
					
					pDC->MoveTo (rectTab.right - 1, bIsActive ? rectTab.bottom : rectTab.bottom - 1);
					pDC->LineTo (rectTab.right - 1, rectTab.top + iVertOffset - 1);
				}
 			}
		}

		if (bIsActive)
		{
			const int iBarHeight = 1;
			const int y = (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM) ? 
				(rectTab.top - iBarHeight - 1) : (rectTab.bottom);

			CRect rectFill (CPoint (rectTab.left, y), 
							CSize (rectTab.Width (), iBarHeight + 1));

			COLORREF clrActiveTab = pTabWnd->GetTabBkColor (iTab);

			if (bIsOneNote)
			{
				if (bIs2005)
				{
					rectFill.left += 3;
				}
				else
				{
					rectFill.OffsetRect (1, 0);
				}

				if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
				{
					rectFill.left++;
				}

				if (clrActiveTab == (COLORREF)-1)
				{
					clrActiveTab = globalData.clrWindow;
				}
			}

			if (clrActiveTab != (COLORREF)-1)
			{
				CBrush br (clrActiveTab);
				pDC->FillRect (rectFill, &br);
			}
			else
			{
				pDC->FillRect (rectFill, &globalData.brBarFace);
			}
		}

		pDC->SelectObject (pOldPen);

		if (bIsOneNote)
		{
			const int nLeftMargin = pTabWnd->IsVS2005Style () && bIsActive ?
				rectTab.Height () * 3 / 4 : rectTab.Height ();

			const int nRightMargin = pTabWnd->IsVS2005Style () && bIsActive ?
				CBCGTabWnd::TAB_IMAGE_MARGIN * 3 / 4 : CBCGTabWnd::TAB_IMAGE_MARGIN;

			rectTab.left += nLeftMargin;
			rectTab.right -= nRightMargin;

			if (pTabWnd->IsVS2005Style () && bIsActive && pTabWnd->HasImage (iTab))
			{
				rectTab.OffsetRect (CBCGTabWnd::TAB_IMAGE_MARGIN, 0);
			}
		}

		pDC->SelectClipRgn (NULL);
	}

	COLORREF clrText = pTabWnd->GetTabTextColor (iTab);
	
	COLORREF cltTextOld = (COLORREF)-1;
	if (!bIsActive && clrText != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor (clrText);
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rectClip);

	pDC->SelectClipRgn (&rgn);

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	if (cltTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (cltTextOld);
	}

	pDC->SelectClipRgn (NULL);
}
//*********************************************************************************
void CBCGVisualManager::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive,
									 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pbrFill);

	if (bIsActive && !globalData.IsHighContastMode () &&
		(pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded ()) &&
		pTabWnd->GetTabBkColor (iTab) == (COLORREF)-1)
	{
		pDC->FillRect (rectFill, &globalData.brWindow);
	}
	else
	{
		pDC->FillRect (rectFill, pbrFill);
	}
}
//*********************************************************************************
BOOL CBCGVisualManager::OnEraseTabsFrame (CDC* pDC, CRect rect, 
										   const CBCGTabWnd* pTabWnd) 
{	
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());

	if (clrActiveTab == (COLORREF)-1)
	{
		return FALSE;
	}

	pDC->FillSolidRect (rect, clrActiveTab);
	return TRUE;
}
//*********************************************************************************
void CBCGVisualManager::OnDrawTabContent (CDC* pDC, CRect rectTab,
						int iTab, BOOL /*bIsActive*/, const CBCGTabWnd* pTabWnd,
						COLORREF clrText)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	CString strText;
	pTabWnd->GetTabLabel (iTab, strText);

	if (pTabWnd->IsFlatTab ())
	{
		//---------------
		// Draw tab text:
		//---------------
		UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
		if (pTabWnd->IsDrawNoPrefix ())
		{
			nFormat |= DT_NOPREFIX;
		}

		pDC->DrawText (strText, rectTab, nFormat);
	}
	else
	{
		CSize sizeImage = pTabWnd->GetImageSize ();
		UINT uiIcon = pTabWnd->GetTabIcon (iTab);

		if (uiIcon == (UINT)-1)
		{
			sizeImage.cx = 0;
		}

		if (sizeImage.cx + CBCGTabWnd::TAB_IMAGE_MARGIN <= rectTab.Width ())
		{
			const CImageList* pImageList = pTabWnd->GetImageList ();
			if (pImageList != NULL && uiIcon != (UINT)-1)
			{
				//----------------------
				// Draw the tab's image:
				//----------------------
				CRect rectImage = rectTab;

				rectImage.top += (rectTab.Height () - sizeImage.cy) / 2;
				rectImage.bottom = rectImage.top + sizeImage.cy;

				rectImage.left += CBCGTabWnd::TAB_IMAGE_MARGIN;
				rectImage.right = rectImage.left + sizeImage.cx;

				ASSERT_VALID (pImageList);
				((CImageList*) pImageList)->Draw (pDC, uiIcon, rectImage.TopLeft (), ILD_TRANSPARENT);
			}

			//------------------------------
			// Finally, draw the tab's text:
			//------------------------------
			CRect rcText = rectTab;
			rcText.left += sizeImage.cx + 2 * TEXT_MARGIN;
			
			if (clrText != (COLORREF)-1)
			{
				pDC->SetTextColor (clrText);
			}

			CString strText;
			pTabWnd->GetTabLabel (iTab, strText);

			UINT nFormat = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
			if (pTabWnd->IsDrawNoPrefix ())
			{
				nFormat |= DT_NOPREFIX;
			}

			if (pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
			{
				nFormat |= DT_CENTER;
			}
			else
			{
				nFormat |= DT_LEFT;
			}

			pDC->DrawText (strText, rcText, nFormat);
		}
	}
}
//**********************************************************************************
COLORREF CBCGVisualManager::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (bIsSelected)
	{
		pDC->FillRect (rect, &globalData.brHilite);
		
		const int nFrameSize = 1;

		rect.DeflateRect (1, 1);
		rect.right--;
		rect.bottom--;

		pDC->PatBlt (rect.left, rect.top + nFrameSize, nFrameSize, rect.Height (), PATINVERT);
		pDC->PatBlt (rect.left, rect.top, rect.Width (), nFrameSize, PATINVERT);
		pDC->PatBlt (rect.right, rect.top, nFrameSize, rect.Height (), PATINVERT);
		pDC->PatBlt (rect.left + nFrameSize, rect.bottom, rect.Width (), nFrameSize, PATINVERT);

		return globalData.clrTextHilite;
	}

	pDC->FillRect (rect, &globalData.brBtnFace);
	return globalData.clrBarText;
}
//**********************************************************************************
CBCGVisualManager* CBCGVisualManager::CreateVisualManager (
					CRuntimeClass* pVisualManager)
{
	if (pVisualManager == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGVisualManager* pVisManagerOld = m_pVisManager;
	
	CObject* pObj = pVisualManager->CreateObject ();
	if (pObj == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT_VALID (pObj);
	
	if (pVisManagerOld != NULL)
	{
		ASSERT_VALID (pVisManagerOld);
		delete pVisManagerOld;
	}
	
	m_pVisManager = (CBCGVisualManager*) pObj;
	m_pVisManager->m_bAutoDestroy = TRUE;

	return m_pVisManager;
}
//*************************************************************************************
void CBCGVisualManager::DestroyInstance (BOOL bAutoDestroyOnly)
{
	if (m_pVisManager == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pVisManager);

	if (bAutoDestroyOnly && !m_pVisManager->m_bAutoDestroy)
	{
		return;
	}

	delete m_pVisManager;
	m_pVisManager = NULL;
}
//***********************************************************************************
void CBCGVisualManager::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	const int iBorderSize = 2;

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &globalData.brBarFace);
	
	rect.DeflateRect (iBorderSize, 1);

	pDC->FillSolidRect (rect, 
		bIsActive ? 
		::GetSysColor (COLOR_ACTIVECAPTION) :
		::GetSysColor (COLOR_INACTIVECAPTION));
}
//***********************************************************************************
void CBCGVisualManager::OnDrawMenuSystemButton (CDC* pDC, CRect rect, 
												UINT uiSystemCommand, 
												UINT nStyle, BOOL /*bHighlight*/)
{
	ASSERT_VALID (pDC);

	UINT uiState = 0;

	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		uiState |= DFCS_CAPTIONCLOSE;
		break;

	case SC_MINIMIZE:
		uiState |= DFCS_CAPTIONMIN;
		break;

	case SC_RESTORE:
		uiState |= DFCS_CAPTIONRESTORE;
		break;

	default:
		return;
	}

	if (nStyle & TBBS_PRESSED)
	{
		uiState |= DFCS_PUSHED;
	}
	
	if (nStyle & TBBS_DISABLED) // Jan Vasina: Add support for disabled buttons
	{
		uiState |= DFCS_INACTIVE;
	}

	pDC->DrawFrameControl (rect, DFC_CAPTION, uiState);
}
//********************************************************************************
void CBCGVisualManager::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* /*pBar*/,
					CRect rectPane, UINT /*uiID*/, UINT nStyle)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	if (!(nStyle & SBPS_NOBORDERS))
	{
		// draw the borders
		COLORREF clrHilite;
		COLORREF clrShadow;

		if (nStyle & SBPS_POPOUT)
		{
			// reverse colors
			clrHilite = globalData.clrBarShadow;
			clrShadow = globalData.clrBarHilite;
		}
		else
		{
			// normal colors
			clrHilite = globalData.clrBarHilite;
			clrShadow = globalData.clrBarShadow;
		}

		pDC->Draw3dRect (rectPane, clrShadow, clrHilite);
	}
}
//**************************************************************************************
void CBCGVisualManager::OnDrawComboDropButton (CDC* pDC, CRect rect,
											    BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	COLORREF clrText = pDC->GetTextColor ();

	pDC->FillRect (rect, &globalData.brBarFace);
	pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarHilite);

	if (bIsDropped)
	{
		rect.OffsetRect (1, 1);
		pDC->Draw3dRect (&rect, globalData.clrBarShadow, globalData.clrBarHilite);
	}
	else if (bIsHighlighted)
	{
		pDC->Draw3dRect (&rect, globalData.clrBarHilite, globalData.clrBarShadow);
	}

	CMenuImages::Draw (pDC, 
		(bDisabled ? CMenuImages::IdArowDownDsbl : CMenuImages::IdArowDown), 
		rect);

	pDC->SetTextColor (clrText);
}
//*************************************************************************************
void CBCGVisualManager::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL /*bDisabled*/,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID (pDC);

	if (bIsHighlighted || bIsDropped)
	{
		if (m_bMenuFlatLook)
		{
			CRect rectBorder = rect;
			rectBorder.DeflateRect (1, 1);

			pDC->Draw3dRect (&rectBorder, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
		}
		else
		{
			pDC->Draw3dRect (&rect, globalData.clrBarShadow, globalData.clrBarHilite);
		}
	}
}
//*************************************************************************************
void CBCGVisualManager::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL /*bDisabled*/,
												BOOL bIsHighlighted,
												CBCGToolbarEditBoxButton* /*pButton*/)
{
	ASSERT_VALID (pDC);

	if (bIsHighlighted)
	{
		pDC->DrawEdge (rect, EDGE_SUNKEN, BF_RECT);
	}
}
//**************************************************************************************
COLORREF CBCGVisualManager::GetToolbarButtonTextColor (CBCGToolbarButton* pButton,
												  CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
		(!CBCGToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

	if (pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton)))
	{
		if (globalData.IsHighContastMode ())
		{
			return bDisabled ? globalData.clrGrayedText : globalData.clrWindowText;
		}

		return bDisabled ? globalData.clrBtnFace : globalData.clrWindow;
	}

	return	(bDisabled ? globalData.clrGrayedText : 
			(state == ButtonsIsHighlighted) ? 
				CBCGToolBar::GetHotTextColor () : globalData.clrBarText);
}
//***************************************************************************************
BOOL CBCGVisualManager::OnFillOutlookPageButton (CBCGButton* /*pButton*/,
												CDC* /*pDC*/, const CRect& /*rectClient*/,
												COLORREF& /*clrText*/)
{
	return FALSE;	// Default processing
}
//****************************************************************************************
BOOL CBCGVisualManager::OnDrawOutlookPageButtonBorder (CBCGButton* /*pButton*/,
												CDC* /*pDC*/, CRect& /*rectClient*/,
												UINT /*uiState*/)
{
	return FALSE;	// Default processing
}
//**********************************************************************************
COLORREF CBCGVisualManager::GetCaptionBarTextColor (CBCGCaptionBar* /*pBar*/)
{
	return ::GetSysColor (COLOR_WINDOW);
}
//**************************************************************************************
void CBCGVisualManager::OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* /*pStatusBar*/,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
			BOOL bProgressText)
{
	ASSERT_VALID (pDC);

	if (nProgressTotal == 0)
	{
		return;
	}

	CRect rectComplete = rectProgress;
	rectComplete.right = rectComplete.left + 
		nProgressCurr * rectComplete.Width () / nProgressTotal;

	if (clrProgressBarDest == (COLORREF)-1)
	{
		// one-color bar
		CBrush br (clrBar);
		pDC->FillRect (rectComplete, &br);
	}
	else
	{
		// gradient bar:
		CBCGDrawManager dm (*pDC);
		dm.FillGradient (rectComplete, clrBar, clrProgressBarDest, FALSE);
	}

	if (bProgressText)
	{
		CString strText;
		strText.Format (_T("%d%%"), nProgressCurr * 100 / nProgressTotal);

		COLORREF clrText = pDC->SetTextColor (globalData.clrBtnText);

		pDC->DrawText (strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

		CRgn rgn;
		rgn.CreateRectRgnIndirect (rectComplete);
		pDC->SelectClipRgn (&rgn);

		pDC->SetTextColor (clrProgressText == (COLORREF)-1 ?
			::GetSysColor (COLOR_HIGHLIGHTTEXT) : clrProgressText);
		
		pDC->DrawText (strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		pDC->SelectClipRgn (NULL);
		pDC->SetTextColor (clrText);
	}
}
//*********************************************************************************
void CBCGVisualManager::OnFillHeaderCtrlBackground (CBCGHeaderCtrl* pCtrl,
													 CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, 
		pCtrl->IsDialogControl () ? &globalData.brBtnFace : &globalData.brBarFace);
}
//****************************************************************************************
void CBCGVisualManager::OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL /*bIsHighlighted*/)
{
	ASSERT_VALID (pDC);

	if (bIsPressed)
	{
		if (pCtrl->IsDialogControl ())
		{
			pDC->Draw3dRect (rect, globalData.clrBtnShadow, globalData.clrBtnShadow);
		}
		else
		{
			pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
		}

		rect.left++;
		rect.top++;
	}
	else
	{
		if (pCtrl->IsDialogControl ())
		{
			pDC->Draw3dRect (rect, globalData.clrBtnHilite, globalData.clrBtnShadow);
		}
		else
		{
			pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarShadow);
		}
	}
}
//*****************************************************************************************
void CBCGVisualManager::OnDrawHeaderCtrlSortArrow (CBCGHeaderCtrl* pCtrl, 
												   CDC* pDC,
												   CRect& rectArrow, BOOL bIsUp)
{
	CPen penLight (1, PS_SOLID, 
		pCtrl->IsDialogControl () ? globalData.clrBtnHilite : globalData.clrBarHilite);
	CPen penDark (1, PS_SOLID, 
		pCtrl->IsDialogControl () ? globalData.clrBtnDkShadow : globalData.clrBarDkShadow);

	CPen* pPenOld = pDC->SelectObject (&penLight);
	ASSERT_VALID (pPenOld);

	if (!bIsUp)
	{
		pDC->MoveTo (rectArrow.right, rectArrow.top);
		pDC->LineTo (rectArrow.CenterPoint ().x, rectArrow.bottom);

		pDC->SelectObject (&penDark);
		pDC->LineTo (rectArrow.left, rectArrow.top);
		pDC->LineTo (rectArrow.right, rectArrow.top);
	}
	else
	{
		pDC->MoveTo (rectArrow.left, rectArrow.bottom);
		pDC->LineTo (rectArrow.right, rectArrow.bottom);
		pDC->LineTo (rectArrow.CenterPoint ().x, rectArrow.top);

		pDC->SelectObject (&penDark);
		pDC->LineTo (rectArrow.left, rectArrow.bottom);
	}

	pDC->SelectObject (pPenOld);
}
//*****************************************************************************************
void CBCGVisualManager::OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
			CRect rectSizeBox)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pStatBar);

	if (m_pfGetWindowTheme != NULL && 
		(*m_pfGetWindowTheme) (pStatBar->GetSafeHwnd ()) != NULL &&
		m_hThemeScrollBar != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX,
			SZB_RIGHTALIGN, &rectSizeBox, 0);
		return;
	}

	CFont* pOldFont = pDC->SelectObject (&globalData.fontMarlett);
	ASSERT (pOldFont != NULL);

	const CString strSizeBox = 
		(pStatBar->GetExStyle() & WS_EX_LAYOUTRTL) ? 
		_T("x") : _T("o");	// Char of the sizing box in "Marlett" font

	UINT nTextAlign = pDC->SetTextAlign (TA_RIGHT | TA_BOTTOM);
	COLORREF clrText = pDC->SetTextColor (globalData.clrBtnShadow);

	pDC->ExtTextOut (rectSizeBox.right, rectSizeBox.bottom,
		ETO_CLIPPED, &rectSizeBox, strSizeBox, NULL);

	pDC->SelectObject (pOldFont);
	pDC->SetTextColor (clrText);
	pDC->SetTextAlign (nTextAlign);
}
//*********************************************************************************
void CBCGVisualManager::OnEraseTabsButton (CDC* pDC, CRect rect, 
											CBCGButton* /*pButton*/,
											CBCGTabWnd* /*pWndTab*/)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &globalData.brBarFace);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT uiState,
												 CBCGTabWnd* /*pWndTab*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsPressed () || (uiState & ODS_SELECTED))
	{
		pDC->Draw3dRect (rect, globalData.clrBtnDkShadow, globalData.clrBtnHilite);

		rect.left += 2;
		rect.top += 2;
	}
	else
	{
		pDC->Draw3dRect (rect, globalData.clrBtnHilite, globalData.clrBtnDkShadow);
	}

	rect.DeflateRect (2, 2);
}
//*********************************************************************************
void CBCGVisualManager::OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, 
											CBCGButton* /*pButton*/)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &globalData.brBarFace);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT uiState)
{
	pButton->DrawBorder (pDC, rect, uiState);
}
//**********************************************************************************
void CBCGVisualManager::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	::FillRect (pDC->GetSafeHdc (), rectWorkArea, ::GetSysColorBrush (COLOR_WINDOW));
}
//**********************************************************************************
void CBCGVisualManager::OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, 
						BOOL bIsHighlighted, BOOL /*bIsSelected*/, BOOL bCanCollapse)
{
	ASSERT_VALID(pDC);
	ASSERT(pGroup != NULL);

	const int nIconMargin = 10;

	// ---------------------------------
	// Draw caption background (Windows)
	// ---------------------------------
	COLORREF clrBckOld = pDC->GetBkColor ();
	pDC->FillSolidRect(pGroup->m_rect, 
		(pGroup->m_bIsSpecial ? globalData.clrHilite : globalData.clrBtnFace)); 
	pDC->SetBkColor (clrBckOld);

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL 
		&& pGroup->m_sizeIcon.cx < pGroup->m_rect.Width () - pGroup->m_rect.Height());
	if (bShowIcon)
	{
		CPoint pointIcon(pGroup->m_rect.left+1, pGroup->m_rect.bottom - pGroup->m_sizeIcon.cy);
		pDC->DrawIcon(pointIcon, pGroup->m_hIcon);
	}

	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject (&globalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bCanCollapse && bIsHighlighted)
	{
		clrTextOld = pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ?
			(pGroup->m_bIsSpecial ? ::GetSysColor(COLOR_WINDOW) : ::GetSysColor(COLOR_WINDOWTEXT)) :
			pGroup->m_clrTextHot);
	}
	else
	{
		clrTextOld = pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ?
			(pGroup->m_bIsSpecial ? ::GetSysColor(COLOR_WINDOW) : ::GetSysColor(COLOR_WINDOWTEXT)) :
			pGroup->m_clrText);
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();

	CRect rectText = pGroup->m_rect;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	+ nIconMargin: 
		(nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset));
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right - pGroup->m_rect.Height());

	pDC->DrawText (pGroup->m_strName, rectText, DT_SINGLELINE);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse && !pGroup->m_strName.IsEmpty())
	{
		CSize sizeButton = CMenuImages::Size();
		CRect rectButton = pGroup->m_rect;
		rectButton.left = max(rectButton.left, 
			rectButton.right - (rectButton.Height()+1)/2 - (sizeButton.cx+1)/2);
		rectButton.top = max(rectButton.top, 
			rectButton.bottom - (rectButton.Height()+1)/2 - (sizeButton.cy+1)/2);
		rectButton.right = rectButton.left + sizeButton.cx;
		rectButton.bottom = rectButton.top + sizeButton.cy;
		
		if (rectButton.right <= pGroup->m_rect.right && rectButton.bottom <= pGroup->m_rect.bottom)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject (&globalData.brBarFace);
				COLORREF clrBckOld = pDC->GetBkColor ();

				pDC->Draw3dRect(&rectButton, ::GetSysColor(COLOR_WINDOW), globalData.clrBtnShadow);

				pDC->SetBkColor (clrBckOld);
				pDC->SelectObject (pBrushOld);
			}

			if (pGroup->m_bIsSpecial)
			{
				if (!pGroup->m_bIsCollapsed)
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArowUp, rectButton.TopLeft());
				}
				else
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArowDown, rectButton.TopLeft());
				}
			}
			else
			{
				if (!pGroup->m_bIsCollapsed)
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArowUp, rectButton.TopLeft());
				}
				else
				{
					CMenuImages::Draw(pDC, CMenuImages::IdArowDown, rectButton.TopLeft());
				}
			}
		}
	}
}
//**********************************************************************************
void CBCGVisualManager::OnEraseTasksGroupArea(CDC* /*pDC*/, CRect /*rect*/)
{
}
//**********************************************************************************
void CBCGVisualManager::OnFillTasksGroupInterior(CDC* /*pDC*/, CRect /*rect*/, BOOL /*bSpecial*/)
{
}
//**********************************************************************************
void CBCGVisualManager::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial, 
												   BOOL bNoTitle)
{
	ASSERT_VALID(pDC);

	// Draw caption background:
	CPen* pPenOld = (CPen*) pDC->SelectObject (bSpecial ? &globalData.penHilite : &globalData.penBarFace);

	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.left, rect.bottom-1);
	pDC->LineTo (rect.right-1, rect.bottom-1);
	pDC->LineTo (rect.right-1, rect.top);
	if (bNoTitle)
	{
		pDC->LineTo (rect.left, rect.top);
	}
	else
	{
		pDC->LineTo (rect.right-1, rect.top-1);
	}

	pDC->SelectObject (&pPenOld);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
								   BOOL bIsHighlighted, BOOL /*bIsSelected*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT(pTask != NULL);

	CRect rectText = pTask->m_rect;

	if (pTask->m_bIsSeparator)
	{
		CPen* pPenOld = (CPen*) pDC->SelectObject (&globalData.penBarFace);

		pDC->MoveTo (rectText.left, rectText.CenterPoint ().y);
		pDC->LineTo (rectText.right, rectText.CenterPoint ().y);

		pDC->SelectObject (&pPenOld);
		return;
	}

	// ---------
	// Draw icon
	// ---------
	CSize sizeIcon(0, 0);
	::ImageList_GetIconSize (pIcons->m_hImageList, (int*) &sizeIcon.cx, (int*) &sizeIcon.cy);
	if (pTask->m_nIcon >= 0 && sizeIcon.cx > 0)
	{
		pIcons->Draw (pDC, pTask->m_nIcon, rectText.TopLeft (), ILD_TRANSPARENT);
	}
	int nTaskPaneOffset = pTask->m_pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	rectText.left += sizeIcon.cx + (nTaskPaneOffset != -1 ? nTaskPaneOffset : m_nTasksIconHorzOffset);

	// ---------
	// Draw text
	// ---------
	BOOL bIsLabel = (pTask->m_uiCommandID == 0);

	CFont* pFontOld = NULL;
	COLORREF clrTextOld = pDC->GetTextColor ();
	if (bIsLabel)
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			::GetSysColor(COLOR_WINDOWTEXT) : pTask->m_clrText);
	}
	else if (!pTask->m_bEnabled)
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (globalData.clrGrayedText);
	}
	else if (bIsHighlighted)
	{
		pFontOld = pDC->SelectObject (&globalData.fontUnderline);
		pDC->SetTextColor (pTask->m_clrTextHot == (COLORREF)-1 ?
			::GetSysColor(COLOR_WINDOWTEXT) : pTask->m_clrTextHot);
	}
	else
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			::GetSysColor(COLOR_WINDOWTEXT) : pTask->m_clrText);
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	CBCGTasksPane* pTaskPane = pTask->m_pGroup->m_pPage->m_pTaskPane;
	ASSERT_VALID (pTaskPane);

	BOOL bMultiline = bIsLabel ? 
		pTaskPane->IsWrapLabelsEnabled () : pTaskPane->IsWrapTasksEnabled ();

	if (bMultiline)
	{
		pDC->DrawText (pTask->m_strName, rectText, DT_WORDBREAK);
	}
	else
	{
		pDC->DrawText (pTask->m_strName, rectText, DT_SINGLELINE | DT_VCENTER);
	}
	
	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brBarFace);

	if (bHilited)
	{
		CBCGDrawManager dm (*pDC);
		dm.HighlightRect (rect);

		pDC->Draw3dRect (rect,
			globalData.clrBarHilite,
			globalData.clrBarDkShadow);
	}

	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}
//**************************************************************************************
void CBCGVisualManager::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened,
											COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTree != NULL && 
		(*m_pfGetWindowTheme) (pDC->GetWindow ()->GetSafeHwnd ()) != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeTree, pDC->GetSafeHdc(), TVP_GLYPH,
			bIsOpened ? GLPS_OPENED : GLPS_CLOSED, &rect, 0);
		return;
	}

	pDC->Draw3dRect (rect, colorBox, colorBox);

	rect.DeflateRect (2, 2);

	CPen penLine (PS_SOLID, 1, globalData.clrBtnText);
	CPen* pOldPen = pDC->SelectObject (&penLine);

	CPoint ptCenter = rect.CenterPoint ();

	pDC->MoveTo (rect.left, ptCenter.y);
	pDC->LineTo (rect.right, ptCenter.y);

	if (!bIsOpened)
	{
		pDC->MoveTo (ptCenter.x, rect.top);
		pDC->LineTo (ptCenter.x, rect.bottom);
	}

	pDC->SelectObject (pOldPen);
}
//**********************************************************************************
void CBCGVisualManager::GetTabFrameColors (const CBCGTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack)
{
	ASSERT_VALID (pTabWnd);

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());

	if (pTabWnd->IsOneNoteStyle () && clrActiveTab != (COLORREF)-1)
	{
		clrFace = clrActiveTab;
	}
	else
	{
		clrFace = globalData.clrBarFace;
	}

	clrDark = globalData.clrBarShadow;
	clrBlack = globalData.clrBarText;
	clrHighlight = pTabWnd->IsVS2005Style () ? globalData.clrBarShadow : globalData.clrBarHilite;
	clrDarkShadow = globalData.clrBarDkShadow;
	clrLight = globalData.clrBarLight;

	pbrFace = &globalData.brBarFace;
	pbrBlack = &globalData.brBlack;
}
//********************************************************************************
COLORREF CBCGVisualManager::GetPropListGroupColor (CBCGPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	return pPropList->DrawControlBarColors () ?
		globalData.clrBarFace : globalData.clrBtnFace;
}
//********************************************************************************
COLORREF CBCGVisualManager::GetPropListGroupTextColor (CBCGPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	return pPropList->DrawControlBarColors () ?
		globalData.clrBarDkShadow : globalData.clrBtnDkShadow;
}
//********************************************************************************
COLORREF CBCGVisualManager::GetMenuItemTextColor (
	CBCGToolbarMenuButton* /*pButton*/, BOOL bHighlighted, BOOL bDisabled)
{
	if (bHighlighted)
	{
		return bDisabled ? globalData.clrBtnFace : globalData.clrTextHilite;
	}

	return bDisabled ? globalData.clrGrayedText : globalData.clrWindowText;
}


/////////////////////////////////////////////////////////////////////////////////////
// CBCGWinXPThemeManager

CBCGWinXPThemeManager::CBCGWinXPThemeManager ()
{
	m_hThemeToolBar = NULL;
	m_hThemeButton = NULL;
	m_hThemeStatusBar = NULL;
	m_hThemeWindow = NULL;
	m_hThemeRebar = NULL;
	m_hThemeComboBox = NULL;
	m_hThemeProgress = NULL;
	m_hThemeHeader = NULL;
	m_hThemeScrollBar = NULL;
	m_hThemeExplorerBar = NULL;
	m_hThemeTree = NULL;
	m_hThemeStartPanel = NULL;
	m_hThemeTaskBand = NULL;
	m_hThemeTaskBar = NULL;
	m_hThemeSpin = NULL;
	m_hThemeTab = NULL;

	m_hinstUXDLL = LoadLibrary (_T("UxTheme.dll"));

	if (m_hinstUXDLL != NULL)
	{
		m_pfOpenThemeData = (OPENTHEMEDATA)::GetProcAddress (m_hinstUXDLL, "OpenThemeData");
		m_pfCloseThemeData = (CLOSETHEMEDATA)::GetProcAddress (m_hinstUXDLL, "CloseThemeData");
		m_pfDrawThemeBackground = (DRAWTHEMEBACKGROUND)::GetProcAddress (m_hinstUXDLL, "DrawThemeBackground");
		m_pfGetThemeColor = (GETTHEMECOLOR)::GetProcAddress (m_hinstUXDLL, "GetThemeColor");
		m_pfGetThemeSysColor = (GETTHEMESYSCOLOR)::GetProcAddress (m_hinstUXDLL, "GetThemeSysColor");
		m_pfGetCurrentThemeName = (GETCURRENTTHEMENAME)::GetProcAddress (m_hinstUXDLL, "GetCurrentThemeName");
		m_pfGetWindowTheme = (GETWINDOWTHEME)::GetProcAddress (m_hinstUXDLL, "GetWindowTheme");
		m_pfIsAppThemed = (ISAPPTHEMED)::GetProcAddress (m_hinstUXDLL, "IsAppThemed");

		UpdateSystemColors ();
	}
	else
	{
		m_pfOpenThemeData = NULL;
		m_pfCloseThemeData = NULL;
		m_pfDrawThemeBackground = NULL;
		m_pfGetThemeColor = NULL;
		m_pfGetThemeSysColor = NULL;
		m_pfGetCurrentThemeName = NULL;
		m_pfGetWindowTheme = NULL;
		m_pfIsAppThemed = NULL;
	}
}
//*************************************************************************************
CBCGWinXPThemeManager::~CBCGWinXPThemeManager ()
{
	if (m_hinstUXDLL != NULL)
	{
		CleanUpThemes ();
		::FreeLibrary (m_hinstUXDLL);
	}
}
//*************************************************************************************
void CBCGWinXPThemeManager::UpdateSystemColors ()
{
	if (m_hinstUXDLL != NULL)
	{
		CleanUpThemes ();

		if (m_pfOpenThemeData == NULL ||
			m_pfCloseThemeData == NULL ||
			m_pfDrawThemeBackground == NULL)
		{
			ASSERT (FALSE);
		}
		else
		{
			m_hThemeToolBar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"TOOLBAR");
			m_hThemeButton = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"BUTTON");
			m_hThemeStatusBar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"STATUS");
			m_hThemeWindow = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"WINDOW");
			m_hThemeRebar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"REBAR");
			m_hThemeComboBox = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"COMBOBOX");
			m_hThemeProgress = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"PROGRESS");
			m_hThemeHeader = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"HEADER");
			m_hThemeScrollBar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"SCROLLBAR");
			m_hThemeExplorerBar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"EXPLORERBAR");
			m_hThemeTree = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"TREEVIEW");
			m_hThemeStartPanel = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"STARTPANEL");
			m_hThemeTaskBand = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"TASKBAND");
			m_hThemeTaskBar = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"TASKBAR");
			m_hThemeSpin = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"SPIN");
			m_hThemeTab = (*m_pfOpenThemeData)(AfxGetMainWnd ()->GetSafeHwnd (), L"TAB");
		}
	}
}
//************************************************************************************
void CBCGWinXPThemeManager::CleanUpThemes ()
{
	if (m_pfCloseThemeData == NULL)
	{
		return;
	}

	if (m_hThemeToolBar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeToolBar);
	}

	if (m_hThemeRebar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeRebar);
	}

	if (m_hThemeStatusBar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeStatusBar);
	}

	if (m_hThemeButton != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeButton);
	}

	if (m_hThemeWindow != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeWindow);
	}

	if (m_hThemeComboBox != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeComboBox);
	}

	if (m_hThemeProgress != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeProgress);
	}

	if (m_hThemeHeader != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeHeader);
	}

	if (m_hThemeScrollBar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeScrollBar);
	}

	if (m_hThemeExplorerBar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeExplorerBar);
	}

	if (m_hThemeTree != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeTree);
	}

	if (m_hThemeStartPanel != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeStartPanel);
	}

	if (m_hThemeTaskBand != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeTaskBand);
	}

	if (m_hThemeTaskBar != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeTaskBar);
	}

	if (m_hThemeSpin != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeSpin);
	}

	if (m_hThemeTab != NULL)
	{
		(*m_pfCloseThemeData) (m_hThemeTab);
	}
}
//**************************************************************************************
BOOL CBCGWinXPThemeManager::DrawStatusBarProgress (CDC* pDC, CBCGStatusBar* /*pStatusBar*/,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF /*clrBar*/, COLORREF /*clrProgressBarDest*/, COLORREF /*clrProgressText*/,
			BOOL bProgressText)
{
	#define	PP_BAR				1
	#define	PP_CHUNK			3

	if (m_hThemeProgress == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDC);

	(*m_pfDrawThemeBackground) (m_hThemeProgress, pDC->GetSafeHdc(), 
								PP_BAR, 0, &rectProgress, 0);

	if (nProgressTotal == 0)
	{
		return TRUE;
	}

	CRect rectComplete = rectProgress;
	rectComplete.DeflateRect (3, 3);

	rectComplete.right = rectComplete.left + 
		nProgressCurr * rectComplete.Width () / nProgressTotal;

	(*m_pfDrawThemeBackground) (m_hThemeProgress, pDC->GetSafeHdc(), 
								PP_CHUNK, 0, &rectComplete, 0);

	if (bProgressText)
	{
		CString strText;
		strText.Format (_T("%d%%"), nProgressCurr * 100 / nProgressTotal);

		COLORREF clrText = pDC->SetTextColor (globalData.clrBtnText);
		pDC->DrawText (strText, rectProgress, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		pDC->SetTextColor (clrText);
	}

	return TRUE;
}
//***********************************************************************************
void CBCGVisualManager::OnDrawShowAllMenuItems (CDC* pDC, CRect rect, 
												 CBCGVisualManager::BCGBUTTON_STATE /*state*/)
{
	ASSERT_VALID (pDC);

	CMenuImages::Draw (pDC, CMenuImages::IdArowShowAll, rect);
}
//************************************************************************************
int CBCGVisualManager::GetShowAllMenuItemsHeight (CDC* /*pDC*/, const CSize& /*sizeDefault*/)
{
	return CMenuImages::Size ().cy + 2 * TEXT_MARGIN;
}
//********************************************************************************
void CBCGVisualManager::CleanUp ()
{
	if (m_pVisManager != NULL && m_pVisManager->IsAutoDestroy ())
	{
		delete m_pVisManager;
	}
}
//*************************************************************************************
void CBCGVisualManager::OnDrawControlBorder (CWnd* pWndCtrl)
{
	ASSERT_VALID (pWndCtrl);

	CWindowDC dc (pWndCtrl);

	CRect rect;
	pWndCtrl->GetWindowRect (rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;
	
	if (pWndCtrl->GetStyle () & WS_POPUP)
	{
		dc.Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
	}
	else
	{
		dc.Draw3dRect (rect, globalData.clrBarDkShadow, globalData.clrBarHilite);
	}

	rect.DeflateRect (1, 1);
	dc.Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
}
//********************************************************************************
BOOL CBCGWinXPThemeManager::DrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
{
	#define CP_DROPDOWNBUTTON	1

	#define CBXS_NORMAL			1
	#define CBXS_HOT			2
	#define CBXS_PRESSED		3
	#define CBXS_DISABLED		4

	if (m_hThemeComboBox == NULL)
	{
		return FALSE;
	}

	int nState = bDisabled ? CBXS_DISABLED : bIsDropped ? CBXS_PRESSED : bIsHighlighted ? CBXS_HOT : CBXS_NORMAL;

	(*m_pfDrawThemeBackground) (m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, 
		nState, &rect, 0);

	return TRUE;
}
//********************************************************************************
BOOL CBCGWinXPThemeManager::DrawCheckBox (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 BOOL bChecked,
										 BOOL bEnabled)
{
	#define CBS_UNCHECKEDNORMAL	1
	#define CBS_UNCHECKEDHOT	2
	#define CBS_UNCHECKEDPRESSED	3
	#define CBS_UNCHECKEDDISABLED	4
	#define CBS_CHECKEDNORMAL	5
	#define CBS_CHECKEDHOT		6
	#define CBS_CHECKEDPRESSED	7
	#define CBS_CHECKEDDISABLED	8
	#define CBS_MIXEDNORMAL		9
	#define CBS_MIXEDHOT		10
	#define CBS_MIXEDPRESSED	11
	#define CBS_MIXEDDISABLED	12

	#define BP_CHECKBOX			3

	if (m_hThemeButton == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDC);

	int nState = bChecked ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;

	if (!bEnabled)
	{
		nState = bChecked ? CBS_CHECKEDDISABLED : CBS_UNCHECKEDDISABLED;
	}
	else if (bHighlighted)
	{
		nState = bChecked ? CBS_CHECKEDHOT : CBS_UNCHECKEDHOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_CHECKBOX,
		nState, &rect, 0);

	return TRUE;
}
//********************************************************************************
BOOL CBCGWinXPThemeManager::DrawComboBorder (CDC* pDC, CRect rect,
										BOOL /*bDisabled*/,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
{
	if (m_hThemeWindow == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDC);

	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (&rect,  globalData.clrHilite, globalData.clrHilite);
	}

	return TRUE;
}
//********************************************************************************
void CBCGWinXPThemeManager::FillRebarPane (CDC* pDC, CControlBar* pBar, CRect rectClient)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (m_pfDrawThemeBackground == NULL || m_hThemeRebar == NULL)
	{
		pDC->FillRect (rectClient, &globalData.brBarFace);
		return;
	}

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (CPoint (rectClient.right, rectClient.top), 
		MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	pBar->ScreenToClient (&rectScreen);

	rectClient.right = rectScreen.right;
	rectClient.bottom = rectScreen.bottom;

	if (!pBar->IsFloating ())
	{
		rectClient.left = rectScreen.left;
		rectClient.top = rectScreen.top;
	}

	(*m_pfDrawThemeBackground) (m_hThemeRebar, pDC->GetSafeHdc(),
		0, 0, &rectClient, 0);
}
//*******************************************************************************
void CBCGVisualManager::OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rectSplitter, &globalData.brBarFace);
	pDC->Draw3dRect (rectSplitter, globalData.clrBarHilite, globalData.clrBarShadow);
}
//********************************************************************************
void CBCGVisualManager::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, 
												  COLORREF& clrText)
{
	pDC->FillSolidRect	(rectCaption, globalData.clrBarFace);
	pDC->Draw3dRect (rectCaption, globalData.clrBarHilite, globalData.clrBarShadow);

	clrText = globalData.clrBarText;
}
//**********************************************************************************
void CBCGVisualManager::OnDrawSpinButtons (CDC* pDC, CRect rectSpin, 
	int nState, BOOL bOrientation, CBCGSpinButtonCtrl* pSpinCtrl)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);
	ASSERT_VALID (pSpinCtrl);

	rectSpin.DeflateRect (1, 1);

	CRect rect[2];

	rect[0] = rect[1] = rectSpin;

	if (!bOrientation)
	{
		rect[0].DeflateRect(0, 0, 0, rect[0].Height() / 2);
		rect[1].top = rect[0].bottom + 1;
	}
	else
	{
		rect[0].DeflateRect(0, 0, rect[0].Width() / 2, 0);
		rect[1].left = rect[0].right + 1;
	}

	pDC->FillRect (rectSpin, &globalData.brBarFace);
	pDC->Draw3dRect (rectSpin, globalData.clrBarHilite, globalData.clrBarHilite);

	CMenuImages::IMAGES_IDS id[2][2] = {{CMenuImages::IdArowUp, CMenuImages::IdArowDown}, {CMenuImages::IdArowLeft, CMenuImages::IdArowRight}};

	int idxPressed = (nState & (SPIN_PRESSEDUP | SPIN_PRESSEDDOWN)) - 1;

	for (int i = 0; i < 2; i ++)
	{
		if (idxPressed == i)
		{
			pDC->Draw3dRect (&rect[i], globalData.clrBarShadow, globalData.clrBarHilite);
		}
		else
		{
			pDC->Draw3dRect (&rect[i], globalData.clrBarHilite, globalData.clrBarShadow);
		}

		CMenuImages::Draw (pDC, id[bOrientation][i], rect[i]);
	}
}
//********************************************************************************
void CBCGVisualManager::OnDrawSplitterBorder (CDC* pDC, CBCGSplitterWnd* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarHilite);
	rect.InflateRect(-CX_BORDER, -CY_BORDER);
	pDC->Draw3dRect (rect, globalData.clrBarFace, globalData.clrBarFace);
}
//********************************************************************************
void CBCGVisualManager::OnDrawSplitterBox (CDC* pDC, CBCGSplitterWnd* /*pSplitterWnd*/, CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, globalData.clrBarFace, globalData.clrBarShadow);
}
//********************************************************************************
void CBCGVisualManager::OnFillSplitterBackground (CDC* pDC, CBCGSplitterWnd* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillSolidRect (rect, globalData.clrBarFace);
}
//*******************************************************************************
CBCGWinXPThemeManager::WinXpTheme CBCGWinXPThemeManager::GetStandardWinXPTheme ()
{
	WCHAR szName [256];
	WCHAR szColor [256];

	if (m_pfGetCurrentThemeName == NULL ||
		(*m_pfGetCurrentThemeName) (szName, 255, szColor, 255, NULL, 0) != S_OK)
	{
		return WinXpTheme_None;
	}

	CString strThemeName = szName;
	CString strWinXPThemeColor = szColor;

	TCHAR fname[_MAX_FNAME];   
	_tsplitpath (strThemeName, NULL, NULL, fname, NULL);

	strThemeName = fname;

	if (strThemeName.CompareNoCase (_T("Luna")) != 0)
	{
		return WinXpTheme_NonStandard;
	}

	// Check for 3-d party visual managers:
	if (m_pfGetThemeColor != NULL && m_hThemeButton != NULL)
	{
		COLORREF clrTest = 0;
		if ((*m_pfGetThemeColor) (m_hThemeButton, 1, 0, 3823, &clrTest) !=
			S_OK || clrTest == 1)
		{
			return WinXpTheme_NonStandard;
		}
	}

	if (strWinXPThemeColor.CompareNoCase (_T("normalcolor")) == 0)
	{
		return WinXpTheme_Blue;
	}

	if (strWinXPThemeColor.CompareNoCase (_T("homestead")) == 0)
	{
		return WinXpTheme_Olive;
	}

	if (strWinXPThemeColor.CompareNoCase (_T("metallic")) == 0)
	{
		return WinXpTheme_Silver;
	}

	return WinXpTheme_NonStandard;
}
//*********************************************************************************
void CBCGVisualManager::OnDrawAppBarBorder (CDC* pDC, CBCGAppBarWnd* /*pAppBarWnd*/,
									CRect rectBorder, CRect rectBorderSize)
{
	ASSERT_VALID (pDC);

	CBrush* pOldBrush = pDC->SelectObject (&globalData.brBtnFace);
	ASSERT (pOldBrush != NULL);

	pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height (), PATCOPY);
	pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorder.Width (), rectBorderSize.top, PATCOPY);
	pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height (), PATCOPY);
	pDC->PatBlt (rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width (), rectBorderSize.bottom, PATCOPY);

	rectBorderSize.DeflateRect (2, 2);
	rectBorder.DeflateRect (2, 2);

	pDC->SelectObject (&globalData.brLight);

	pDC->PatBlt (rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height () - 2, PATCOPY);
	pDC->PatBlt (rectBorder.left + 1, rectBorder.top, rectBorder.Width () - 2, rectBorderSize.top, PATCOPY);
	pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height () - 2, PATCOPY);
	pDC->PatBlt (rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width () - 2, rectBorderSize.bottom, PATCOPY);

	pDC->SelectObject (pOldBrush);
}
//*********************************************************************************
void CBCGVisualManager::OnDrawAppBarCaption (CDC* pDC, CBCGAppBarWnd* /*pAppBarWnd*/, 
											CRect rectCaption, CString strCaption)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rectCaption, &globalData.brBarFace);

	// Paint caption text:
	int nOldMode = pDC->SetBkMode (TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor (globalData.clrBarText);
	CFont* pOldFont = pDC->SelectObject (&globalData.fontBold);
	ASSERT_VALID (pOldFont);

	CRect rectText = rectCaption;
	rectText.DeflateRect (2, 0);
	pDC->DrawText (strCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldMode);
	pDC->SetTextColor (clrOldText);
}
void CBCGVisualManager::OnFillPopupWindowBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &globalData.brBarFace);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawPopupWindowBorder (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rect, globalData.clrBarLight, globalData.clrBarDkShadow);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarShadow);
}
//**********************************************************************************
COLORREF  CBCGVisualManager::OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* /*pPopupWnd*/)
{
	ASSERT_VALID (pDC);

	BOOL bActive = TRUE;	// TODO

	CBrush br (bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption);
	pDC->FillRect (rectCaption, &br);

    // get the text color
	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//**********************************************************************************
void CBCGVisualManager::OnErasePopupWindowButton (CDC* pDC, CRect rect, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsCaptionButton ())
	{
		pDC->FillRect (rect, &globalData.brBtnFace);
		return;
	}

	CRect rectParent;
	pButton->GetParent ()->GetClientRect (rectParent);

	pButton->GetParent ()->MapWindowPoints (pButton, rectParent);
	OnFillPopupWindowBackground (pDC, rectParent);
}
//**********************************************************************************
void CBCGVisualManager::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rect, CBCGPopupWndButton* pButton)
{
	if (pButton->IsPressed ())
	{
		pDC->Draw3dRect (rect, globalData.clrBarDkShadow, globalData.clrBarLight);
		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarHilite);
	}
	else
	{
		pDC->Draw3dRect (rect, globalData.clrBarLight, globalData.clrBarDkShadow);
		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarShadow);
	}
}
//***************************************************************************************
COLORREF CBCGVisualManager::OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* /*pBar*/, 
			BOOL bActive, CRect rectCaption, CRect /*rectButtons*/)
{
	ASSERT_VALID (pDC);

	CBrush br (bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption);
	pDC->FillRect (rectCaption, &br);

    // get the text color
	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//************************************************************************************
void CBCGVisualManager::OnDrawMenuShadow (CDC* pPaintDC, const CRect& rectClient, const CRect& /*rectExclude*/,
								int nDepth,  int iMinBrightness,  int iMaxBrightness,  
								CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight,
								BOOL bRTL)
{
	ASSERT_VALID (pPaintDC);
	ASSERT_VALID (pBmpSaveBottom);
	ASSERT_VALID (pBmpSaveRight);

	//------------------------------------------------------
	// Simple draw the shadow, ignore rectExclude parameter:
	//------------------------------------------------------
	CBCGDrawManager dm (*pPaintDC);
	dm.DrawShadow (rectClient, nDepth, iMinBrightness, iMaxBrightness,
				pBmpSaveBottom, pBmpSaveRight, !bRTL);
}
//************************************************************************************
COLORREF CBCGVisualManager::GetToolbarHighlightColor ()
{
	return globalData.clrBarFace;
}
