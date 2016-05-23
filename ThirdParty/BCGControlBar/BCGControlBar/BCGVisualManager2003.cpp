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
// BCGVisualManager2003.cpp: implementation of the CBCGVisualManager2003 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGVisualManager2003.h"
#include "BCGDrawManager.h"
#include "BCGPopupMenuBar.h"
#include "BCGMenuBar.h"
#include "globals.h"
#include "BCGToolbarMenuButton.h"
#include "CustomizeButton.h"
#include "MenuImages.h"
#include "BCGCaptionBar.h"
#include "BCGTabWnd.h"
#include "BCGColorBar.h"
#include "BCGTasksPane.h"
#include "BCGStatusBar.h"
#include "BCGCaptionBar.h"
#include "BCGHeaderCtrl.h"
#include "BCGSizingControlBar.h"
#include "BCGPopupWindow.h"
#include "BCGDropDown.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGVisualManager2003, CBCGVisualManagerXP)

BOOL CBCGVisualManager2003::m_bUseGlobalTheme = TRUE;
BOOL CBCGVisualManager2003::m_bStatusBarOfficeXPLook = TRUE;
BOOL CBCGVisualManager2003::m_bDefaultWinXPColors = TRUE;

#if (defined(SCHEMA_STRINGS)) || (! defined(TMSCHEMA_H))

#define TVP_GLYPH		2
#define GLPS_CLOSED		1
#define GLPS_OPENED		2

#define SBP_SIZEBOX		10
#define SZB_RIGHTALIGN	1
#define SZB_LEFTALIGN	2

#define HIS_NORMAL			1
#define HIS_HOT				2
#define HIS_PRESSED			3

#define HP_HEADERITEM		1
#define HP_HEADERSORTARROW	4

#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGVisualManager2003::CBCGVisualManager2003()
{
	m_WinXPTheme = WinXpTheme_None;

	m_bShadowHighlightedImage = FALSE;
	m_bFadeInactiveImage = FALSE;
	m_nMenuShadowDepth = 3;

	m_nVertMargin = 8;
	m_nHorzMargin = 8;
	m_nGroupVertOffset = 8;
	m_nGroupCaptionHeight = 18;
	m_nGroupCaptionHorzOffset = 3;
	m_nGroupCaptionVertOffset = 3;
	m_nTasksHorzOffset = 8;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;
	m_bIsOutlookToolbarHotBorder = FALSE;
	
	OnUpdateSystemColors ();
}
//********************************************************************************
CBCGVisualManager2003::~CBCGVisualManager2003()
{
}
//****************************************************************************************
void CBCGVisualManager2003::DrawCustomizeButton (CDC* pDC, CRect rect, BOOL bIsHorz,
						  CBCGVisualManager::BCGBUTTON_STATE state,
						  BOOL bIsCustomize, BOOL bIsMoreButtons)
{
	ASSERT_VALID (pDC);

    COLORREF clrDark = state == ButtonsIsRegular ?
		m_clrCustomizeButtonGradientDark : m_clrHighlightGradientDark;

	COLORREF clrLight = state == ButtonsIsRegular ?
		m_clrCustomizeButtonGradientLight : m_clrHighlightGradientLight;

	#define PTS_NUM 6
	POINT pts [PTS_NUM];

	if (bIsHorz)
	{
		pts [0] = CPoint (rect.left, rect.top);
		pts [1] = CPoint (rect.left + 2, rect.top + 1);
		pts [2] = CPoint (rect.left + 3, rect.bottom - 3);
		pts [3] = CPoint (rect.left, rect.bottom);
		pts [4] = CPoint (rect.right, rect.bottom);
		pts [5] = CPoint (rect.right, rect.top);
	}
	else
	{
		pts [0] = CPoint (rect.left, rect.top);
		pts [1] = CPoint (rect.left + 3, rect.top + 2);
		pts [2] = CPoint (rect.right - 3, rect.top + 3);
		pts [3] = CPoint (rect.right, rect.top);
		pts [4] = CPoint (rect.right, rect.bottom);
		pts [5] = CPoint (rect.left, rect.bottom);
	}

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn (pts, PTS_NUM, WINDING);

	pDC->SelectClipRgn (&rgnClip);

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rect, clrDark, clrLight, bIsHorz);

	//---------------------
	// Draw button content:
	//---------------------
	const int nEllipse = 2;

	if (bIsHorz)
	{
		rect.DeflateRect (0, nEllipse);
		rect.left += nEllipse;
	}
	else
	{
		rect.DeflateRect (nEllipse, 0);
		rect.top += nEllipse;
	}

	const int nMargin = GetToolBarCustomizeButtonMargin ();

	CSize sizeImage = CMenuImages::Size ();

	if (CBCGToolBar::IsLargeIcons ())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	if (bIsCustomize)
	{
		//-----------------
		// Draw menu image:
		//-----------------
		CRect rectMenu = rect;
		if (bIsHorz)
		{
			rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
		}
		else
		{
			rectMenu.left = rectMenu.right - sizeImage.cx - 2 * nMargin;
			rectMenu.top++;
		}

		rectMenu.DeflateRect (
			(rectMenu.Width () - sizeImage.cx) / 2,
			(rectMenu.Height () - sizeImage.cy) / 2);

		rectMenu.OffsetRect (1, 1);

		CMenuImages::IMAGES_IDS id = bIsHorz ? 
			CMenuImages::IdCustomizeArowDownWhite : CMenuImages::IdCustomizeArowLeftWhite;

		CMenuImages::Draw (	pDC, id, rectMenu.TopLeft (), sizeImage);

		id = bIsHorz ? 
			CMenuImages::IdCustomizeArowDown : CMenuImages::IdCustomizeArowLeft;

		rectMenu.OffsetRect (-1, -1);

		CMenuImages::Draw (	pDC, id, rectMenu.TopLeft (), sizeImage);
	}

	if (bIsMoreButtons)
	{
		//-------------------
		// Draw "more" image:
		//-------------------
		CRect rectMore = rect;
		if (bIsHorz)
		{
			rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
		}
		else
		{
			rectMore.right = rectMore.left + sizeImage.cx + 2 * nMargin;
			rectMore.top++;
		}

		rectMore.DeflateRect (
			(rectMore.Width () - sizeImage.cx) / 2,
			(rectMore.Height () - sizeImage.cy) / 2);

		CMenuImages::IMAGES_IDS id = 
			bIsHorz ? 
				CMenuImages::IdCustomizeMoreButtonsHorzWhite : 
				CMenuImages::IdCustomizeMoreButtonsVertWhite;

		rectMore.OffsetRect (1, 1);
		CMenuImages::Draw (pDC, id, rectMore.TopLeft (), sizeImage);

		id = 
			bIsHorz ? 
				CMenuImages::IdCustomizeMoreButtonsHorz : 
				CMenuImages::IdCustomizeMoreButtonsVert;

		rectMore.OffsetRect (-1, -1);
		CMenuImages::Draw (pDC, id, rectMore.TopLeft (), sizeImage);
	}

	pDC->SelectClipRgn (NULL);
}
//***********************************************************************************
BOOL CBCGVisualManager2003::IsToolbarRoundShape (CBCGToolBar* pToolBar)
{
	ASSERT_VALID (pToolBar);
	return !pToolBar->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));
}
//***********************************************************************************
void CBCGVisualManager2003::OnFillBarBackground (CDC* pDC, CControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (DYNAMIC_DOWNCAST (CReBar, pBar) != NULL ||
		DYNAMIC_DOWNCAST (CReBar, pBar->GetParent ()))
	{
		FillRebarPane (pDC, pBar, rectClient);
		return;
	}

	CRuntimeClass* pBarClass = pBar->GetRuntimeClass ();

	if (globalData.m_nBitsPerPixel <= 8 ||
		globalData.IsHighContastMode () ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGColorBar)) ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGSizingControlBar)))
	{
		CBCGVisualManagerXP::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGStatusBar)))
	{
		if (m_bStatusBarOfficeXPLook && m_hThemeStatusBar != NULL)
		{
			(*m_pfDrawThemeBackground) (m_hThemeStatusBar, 
				pDC->GetSafeHdc (),
				0, 0, &rectClient, 0);
			return;
		}
	}

	if (rectClip.IsRectEmpty ())
	{
		rectClip = rectClient;
	}

	CBCGDrawManager dm (*pDC);

    if (pBar->IsKindOf (RUNTIME_CLASS (CBCGCaptionBar)))
	{
		dm.FillGradient (rectClient, 
			m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
		return;
	}

    if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
	{
		pDC->FillRect (rectClip, &m_brMenuLight);

		BOOL bQuickMode = FALSE;

		CBCGPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPopupMenuBar, pBar);
		if (pMenuBar != NULL && !pMenuBar->m_bDisableSideBarInXPMode)
		{
			CWnd* pWnd = pMenuBar->GetParent();

			if(pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CBCGPopupMenu)))
			{
				CBCGPopupMenu* pMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, pWnd);

				if(pMenu->IsCustomizePane())
				{
					bQuickMode = TRUE;
				}
			}

			CRect rectImages = rectClient;
			rectImages.DeflateRect (0, 1);

			if (bQuickMode)
			{
				rectImages.right = rectImages.left + 2*CBCGToolBar::GetMenuImageSize ().cx + 
							4 * GetMenuImageMargin () + 4;
			}
			else
			{
				rectImages.right = rectImages.left + CBCGToolBar::GetMenuImageSize ().cx + 
							2 * GetMenuImageMargin () + 2;
			}

			dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE,
				35);
		}
		return;
	}

	BOOL bIsHorz = (pBar->GetBarStyle () & CBRS_ORIENT_HORZ);
	BOOL bIsToolBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGToolBar)) &&
						!pBar->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));

	COLORREF clr1 = bIsHorz ? m_clrToolBarGradientDark : m_clrToolBarGradientVertLight;
	COLORREF clr2 = bIsHorz ? m_clrToolBarGradientLight : m_clrToolBarGradientVertDark;

	if (!bIsToolBar)
	{
		bIsHorz = !bIsHorz;

		clr1 = m_clrBarGradientDark;
		clr2 = m_clrBarGradientLight;

		rectClient.right = rectClient.left + globalData.m_rectVirtual.Width () + 10;
	}

	const int nStartFlatPercentage = bIsToolBar ? 25 : 0;

	BOOL bRoundedCorners = FALSE;

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGDropDownToolBar)))
	{
		bNCArea = FALSE;
	}

	CBCGToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGToolBar, pBar);
	if (bNCArea && pToolBar != NULL && !pToolBar->IsFloating () &&
		pToolBar->m_pDockBar != NULL &&
		!pToolBar->IsKindOf (RUNTIME_CLASS (CBCGMenuBar)))
	{
		bRoundedCorners = TRUE;

		CControlBar* pParentBar = DYNAMIC_DOWNCAST (CControlBar,
			pBar->GetParent ());

		if (pParentBar != NULL)
		{
			CPoint pt (0, 0);
			pBar->MapWindowPoints (pParentBar, &pt, 1);
			pt = pDC->OffsetWindowOrg (pt.x, pt.y);

			CRect rectParent;
			pParentBar->GetClientRect (rectParent);

			OnFillBarBackground (pDC, pParentBar, rectParent, rectParent);

			pDC->SetWindowOrg(pt.x, pt.y);
		}

		CRect rectFill = rectClient;
		rectFill.DeflateRect (1, 0);

		dm.FillGradient (rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);

		CRect rectLeft = rectClient;
		rectLeft.top ++;
		rectLeft.right = rectLeft.left + 1;

		dm.FillGradient (rectLeft, clr1, clr2, bIsHorz);

		CRect rectRight = rectClient;
		rectLeft.top ++;
		rectRight.left = rectRight.right - 1;

		dm.FillGradient (rectRight, clr1, clr2, bIsHorz);
	}
	else
	{
		CRect rectFill = rectClient;

		if (bIsToolBar && pBar->m_pDockBar != NULL)
		{
			ASSERT_VALID (pToolBar);

			rectFill.left -= pToolBar->m_cxLeftBorder;
			rectFill.right += pToolBar->m_cxRightBorder;
			rectFill.top -= pToolBar->m_cyTopBorder;
			rectFill.bottom += pToolBar->m_cyBottomBorder;
		}

		dm.FillGradient (rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);
	}

	if (bNCArea)
	{
		CCustomizeButton* pCustomizeButton = NULL;
		
		CRect rectCustomizeBtn;
		rectCustomizeBtn.SetRectEmpty ();

		if (pToolBar != NULL && pToolBar->GetCount () > 0)
		{
			pCustomizeButton = DYNAMIC_DOWNCAST (CCustomizeButton, 
				pToolBar->GetButton (pToolBar->GetCount () - 1));

			if (pCustomizeButton != NULL)
			{
				rectCustomizeBtn = pCustomizeButton->Rect ();
			}
		}

		if (bRoundedCorners)
		{
			//------------------------
			// Draw bottom/right edge:
			//------------------------
			CPen* pOldPen = pDC->SelectObject (&m_penBottomLine);
			ASSERT (pOldPen != NULL);

			if (bIsHorz)
			{
				pDC->MoveTo (rectClient.left + 2, rectClient.bottom - 1);
				pDC->LineTo (rectClient.right - rectCustomizeBtn.Width (), rectClient.bottom - 1);
			}
			else
			{
				pDC->MoveTo (rectClient.right - 1, rectClient.top + 2);
				pDC->LineTo (rectClient.right - 1, rectClient.bottom - 2 - rectCustomizeBtn.Height ());
			}

			pDC->SelectObject (pOldPen);
		}

		if (pToolBar != NULL && pToolBar->GetCount () > 0)
		{
			if (pCustomizeButton != NULL && !rectCustomizeBtn.IsRectEmpty () &&
				pCustomizeButton->IsPipeStyle ())
			{
				BOOL bIsRTL = pBar->GetExStyle() & WS_EX_LAYOUTRTL;

				//----------------------------------------
				// Special drawing for "Customize" button:
				//----------------------------------------
				CRect rectWindow;
				pBar->GetWindowRect (rectWindow);

				pBar->ClientToScreen (&rectCustomizeBtn);

				CRect rectButton = rectClient;

				if (pToolBar->GetBarStyle () & CBRS_ORIENT_HORZ)
				{
					if (bIsRTL)
					{
						int nButtonWidth = rectCustomizeBtn.Width ();

						nButtonWidth -= 
							rectWindow.left - rectCustomizeBtn.left;
						rectButton.left = rectButton.right - nButtonWidth;
					}
					else
					{
						rectButton.left = rectButton.right - rectCustomizeBtn.Width () - 
							rectWindow.right + rectCustomizeBtn.right;
					}

					pCustomizeButton->SetExtraSize (
						0,
						rectWindow.bottom - rectCustomizeBtn.bottom);
				}
				else
				{
					rectButton.top = rectButton.bottom - rectCustomizeBtn.Height () - 
						rectWindow.bottom + rectCustomizeBtn.bottom;
					pCustomizeButton->SetExtraSize (
						rectWindow.right - rectCustomizeBtn.right,
						0);
				}

				BCGBUTTON_STATE state = ButtonsIsRegular;

				if (pToolBar->IsButtonHighlighted (pToolBar->GetCount () - 1) ||
					pCustomizeButton->IsDroppedDown ())
				{
					state = ButtonsIsHighlighted;
				}
				else if (pCustomizeButton->m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
				{
					//-----------------------
					// Pressed in or checked:
					//-----------------------
					state = ButtonsIsPressed;
				}

				DrawCustomizeButton (pDC, rectButton,
					pToolBar->GetBarStyle () & CBRS_ORIENT_HORZ, state,
					(int) pCustomizeButton->GetCustomizeCmdId () > 0,
					!pCustomizeButton->GetInvisibleButtons ().IsEmpty ());
			}
		}
	}
}
//****************************************************************************************
void CBCGVisualManager2003::OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect)
{
	ASSERT_VALID (pBar);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawBarBorder (pDC, pBar, rect);
	}
}
//***************************************************************************************
void CBCGVisualManager2003::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CControlBar* pBar)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

	if (pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGSizingControlBar)))
	{
		CBCGVisualManagerXP::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

	const int nBoxSize = 4;

	CBCGToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGToolBar, pBar);

	if (pToolBar != NULL)
	{
		if (bHorz)
		{
			rectGripper.left = rectGripper.right - nBoxSize;
			rectGripper.top += nBoxSize / 2;

			const int nHeight = CBCGToolBar::IsLargeIcons () ? 
				pToolBar->GetRowHeight () : pToolBar->GetButtonSize ().cy;

			const int nDelta = max (0, (nHeight - pToolBar->GetImageSize ().cy) / 2);
			rectGripper.DeflateRect (0, nDelta);
		}
		else
		{
			rectGripper.top = rectGripper.bottom - nBoxSize;
			rectGripper.left += nBoxSize / 2;

			const int nWidth = CBCGToolBar::IsLargeIcons () ? 
				pToolBar->GetColumnWidth () : pToolBar->GetButtonSize ().cx;

			const int nDelta = max (0, (nWidth - pToolBar->GetImageSize ().cx) / 2);
			rectGripper.DeflateRect (nDelta, 0);
		}
	}
	else
	{
		if (bHorz)
		{
			rectGripper.left = rectGripper.CenterPoint ().x - nBoxSize / 2;
			rectGripper.right = rectGripper.left + nBoxSize;
		}
		else
		{
			rectGripper.top = rectGripper.CenterPoint ().y - nBoxSize / 2;
			rectGripper.bottom = rectGripper.top + nBoxSize;
		}
	}

	const int nBoxesNumber = bHorz ?
		(rectGripper.Height () - nBoxSize) / nBoxSize : 
		(rectGripper.Width () - nBoxSize) / nBoxSize;

	int nOffset = bHorz ? 
		(rectGripper.Height () - nBoxesNumber * nBoxSize) / 2 :
		(rectGripper.Width () - nBoxesNumber * nBoxSize) / 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		int x = bHorz ? 
			rectGripper.left : 
			rectGripper.left + nOffset;

		int y = bHorz ? 
			rectGripper.top + nOffset : 
			rectGripper.top;

		pDC->FillSolidRect (x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, 
			globalData.clrBtnHilite);
		pDC->FillSolidRect (x, y, nBoxSize / 2, nBoxSize / 2, 
			m_clrGripper);

		nOffset += nBoxSize;
	}
}
//**************************************************************************************
void CBCGVisualManager2003::OnDrawComboBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawComboBorder (pDC, rect,
												bDisabled,
												bIsDropped,
												bIsHighlighted,
												pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped || bDisabled)
	{
		rect.DeflateRect (1, 1);

		COLORREF colorBorder = bDisabled ? globalData.clrBarShadow : m_clrMenuItemBorder;
		pDC->Draw3dRect (&rect, colorBorder, colorBorder);
	}
}
//*********************************************************************************
BOOL CBCGVisualManager2003::OnFillOutlookPageButton (CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnFillOutlookPageButton (pButton,
											pDC, rectClient,
											clrText);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGDrawManager dm (*pDC);

	CRect rect = rectClient;

	if (pButton->IsPressed () ||
		(pButton->IsHighlighted () && pButton->IsChecked ()))
	{
		dm.FillGradient (rect,	m_clrHighlightDnGradientDark,
								m_clrHighlightDnGradientLight,
								TRUE);
	}
	else if (pButton->IsHighlighted () || pButton->IsChecked ())
	{
		dm.FillGradient (rect,	m_clrHighlightGradientDark,
								m_clrHighlightGradientLight,
								TRUE);
	}
	else
	{
		dm.FillGradient (rect,	m_clrBarGradientDark,
								m_clrBarGradientLight,
								TRUE);
	}

	clrText = globalData.clrBarText;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGVisualManager2003::OnDrawOutlookPageButtonBorder (CBCGButton* pButton, 
												CDC* pDC, CRect& rectClient, UINT uiState)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnDrawOutlookPageButtonBorder (pButton, 
												pDC, rectClient, uiState);
	}

	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rectClient, globalData.clrBarHilite, m_clrGripper);
	return TRUE;
}
//**********************************************************************************
void CBCGVisualManager2003::OnFillButtonInterior (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state)
{

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle () || globalData.m_nBitsPerPixel <= 8 ||
		globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	CBCGToolBar* pToolBar = pCustButton->GetParentToolbar ();
	if (pToolBar != NULL)
	{
		ASSERT_VALID (pToolBar);

		CRect rectToolbar;
		pToolBar->GetWindowRect (rectToolbar);
		pToolBar->ScreenToClient (rectToolbar);

		if (pToolBar->GetBarStyle () & CBRS_ORIENT_HORZ)
		{
			rect.right = rectToolbar.right;
		}
		else
		{
			rect.bottom = rectToolbar.bottom;
		}
	}

	CSize sizeExtra = pCustButton->GetExtraSize ();

	rect.InflateRect (sizeExtra);
	DrawCustomizeButton (pDC, rect, pToolBar->GetBarStyle () & CBRS_ORIENT_HORZ, 
		state, (int) pCustButton->GetCustomizeCmdId () > 0,
		!pCustButton->GetInvisibleButtons ().IsEmpty ());

	pCustButton->SetDefaultDraw (FALSE);
}
//**************************************************************************************
void CBCGVisualManager2003::OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state)
{
	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle () || globalData.m_nBitsPerPixel <= 8 ||
		globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
	}

	// Do nothing - the border is already painted in OnFillButtonInterior
}
//**************************************************************************************
void CBCGVisualManager2003::OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pBar);

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)) ||
		globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	CBCGToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGToolBar, pBar);
	if (pToolBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
	ASSERT (pOldPen != NULL);

	if (bHorz)
	{
		const int nDelta = max (0, (pToolBar->GetButtonSize ().cy - pToolBar->GetImageSize ().cy) / 2);
		rect.DeflateRect (0, nDelta);

		int x = rect.left += rect.Width () / 2 - 1;

		pDC->MoveTo (x, rect.top);
		pDC->LineTo (x, rect.bottom - 1);

		pDC->SelectObject (&m_penSeparatorLight);

		pDC->MoveTo (x + 1, rect.top + 1);
		pDC->LineTo (x + 1, rect.bottom);

	}
	else
	{
		const int nDelta = max (0, (pToolBar->GetButtonSize ().cx - pToolBar->GetImageSize ().cx) / 2);
		rect.DeflateRect (nDelta, 0);

		int y = rect.top += rect.Height () / 2 - 1;

		pDC->MoveTo (rect.left, y);
		pDC->LineTo (rect.right - 1, y);

		pDC->SelectObject (&m_penSeparatorLight);

		pDC->MoveTo (rect.left + 1, y + 1);
		pDC->LineTo (rect.right, y + 1);
	}

	pDC->SelectObject (pOldPen);
}
//***********************************************************************************
void CBCGVisualManager2003::OnUpdateSystemColors ()
{
	CBCGWinXPThemeManager::UpdateSystemColors ();

	BOOL bIsAppThemed = m_bUseGlobalTheme || (m_pfGetWindowTheme != NULL && 
		(*m_pfGetWindowTheme) (AfxGetMainWnd ()->GetSafeHwnd ()) != NULL);

	m_WinXPTheme = bIsAppThemed ? GetStandardWinXPTheme () : WinXpTheme_None;

	if (!m_bDefaultWinXPColors && m_WinXPTheme != WinXpTheme_None)
	{
		m_WinXPTheme = WinXpTheme_NonStandard;
	}

	m_bIsStandardWinXPTheme = 
		m_WinXPTheme == WinXpTheme_Blue ||
		m_WinXPTheme == WinXpTheme_Olive ||
		m_WinXPTheme == WinXpTheme_Silver;

	//----------------------
	// Modify global colors:
	//----------------------
	ModifyGlobalColors ();

	CBCGVisualManagerXP::OnUpdateSystemColors ();

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		m_clrTaskPaneGradientDark  = globalData.clrWindow;
		m_clrTaskPaneGradientLight  = globalData.clrWindow;
		m_clrTaskPaneGroupCaptionDark  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionLight  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecDark  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecLight  = globalData.clrBarFace;
		m_clrTaskPaneGroupAreaLight  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaDark  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecLight  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecDark  = globalData.clrWindow;
		m_clrTaskPaneGroupBorder  = globalData.clrBtnShadow;

		m_clrBarGradientLight = m_clrToolBarGradientLight = globalData.clrBarLight;

		m_clrTaskPaneText = globalData.clrWindowText;
		m_clrTaskPaneTextHot = globalData.clrHotText;

		m_penTaskPaneGroupBorder.DeleteObject ();
		m_penTaskPaneGroupBorder.CreatePen (PS_SOLID, 1, m_clrTaskPaneGroupBorder);

		m_clrToolbarDisabled = globalData.clrBtnHilite;
		return;
	}

	//--------------------------------------------------
	// Calculate control bars bakground gradient colors:
	//--------------------------------------------------
	COLORREF clrBase = GetBaseThemeColor ();

	if (m_WinXPTheme == WinXpTheme_Olive)
	{
		m_clrToolBarGradientDark = CBCGDrawManager::PixelAlpha (
			clrBase, 120);

		m_clrBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			.87, 1, 3);

		m_clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1., 2, 1);

		m_clrBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Silver)
	{
		m_clrToolBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.75, 2);

		m_clrBarGradientDark = CBCGDrawManager::PixelAlpha (
			clrBase, 120);

		m_clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DHIGHLIGHT),
			.98);

		m_clrBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Blue)
	{
		m_clrToolBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.93, 2);


		m_clrBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DLIGHT),
			.99, 2, 1);

		m_clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);

		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}
	else
	{
		m_clrToolBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.93, 2);

		m_clrBarGradientDark = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DLIGHT),
			.99, 2, 1);

		m_clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1., 1, 4);

		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}

	m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;

	m_clrToolBarGradientVertDark = CBCGDrawManager::PixelAlpha (
			m_clrToolBarGradientDark, 98);

	COLORREF clrSeparatorDark;

	//-------------------------------------
	// Calculate highlight gradient colors:
	//-------------------------------------
	if (m_bIsStandardWinXPTheme)
	{
		ASSERT (m_pfGetThemeColor != NULL);

		COLORREF clr1, clr2, clr3;

		(*m_pfGetThemeColor) (m_hThemeButton, 1, 0, 3823, &clr1);
		(*m_pfGetThemeColor) (m_hThemeButton, 2, 0, 3823, &clr2);
		(*m_pfGetThemeColor) (m_hThemeWindow, 18, 0, 3821, &clr3);

		m_clrHighlightMenuItem = CBCGDrawManager::SmartMixColors (
			clr1, 
			clr2,
			1.3, 1, 1);

		m_clrHighlightGradientLight = CBCGDrawManager::SmartMixColors (
			clr1, 
			clr3,
			1.55, 2, 1);

		m_clrHighlightGradientDark = CBCGDrawManager::SmartMixColors (
			clr1, 
			clr2,
			1.03, 2, 1);

		m_clrHighlightDnGradientLight = CBCGDrawManager::SmartMixColors (
			clr1, 
			clr3,
			1.03, 1, 2);

		COLORREF clrCustom;
		(*m_pfGetThemeColor) (m_hThemeButton, 2, 0, 3822, &clrCustom);

		if (m_WinXPTheme == WinXpTheme_Blue || m_WinXPTheme == WinXpTheme_Silver)
		{
			m_clrCustomizeButtonGradientDark = CBCGDrawManager::PixelAlpha (
				globalData.clrActiveCaption, 60);

			const double k = (m_WinXPTheme == WinXpTheme_Blue) ? 1.61 : 1;

			m_clrCustomizeButtonGradientLight = CBCGDrawManager::SmartMixColors (
				m_clrCustomizeButtonGradientDark,
				clrBase, k, 3, 1);

			(*m_pfGetThemeColor) (m_hThemeButton, 1, 5, 3823, &clrCustom);
		}
		else
		{
			m_clrCustomizeButtonGradientDark = CBCGDrawManager::SmartMixColors (
				clrCustom, 
				clrBase,
				0.63, 1, 3);

			(*m_pfGetThemeColor) (m_hThemeButton, 1, 5, 3823, &clrCustom);

			m_clrCustomizeButtonGradientLight = CBCGDrawManager::SmartMixColors (
				clrCustom,
				clrBase,
				1.2, 1, 3);
		}

		globalData.clrBarDkShadow = m_clrCustomizeButtonGradientDark;

		if (m_WinXPTheme != WinXpTheme_Silver)
		{
			globalData.clrBarShadow = CBCGDrawManager::SmartMixColors (
				clrCustom,
				clrBase,
				1.4, 1, 3);
		}

		m_clrToolBarBottomLine = m_WinXPTheme == WinXpTheme_Silver ?
			CBCGDrawManager::PixelAlpha (m_clrToolBarGradientDark, 80) :
			CBCGDrawManager::PixelAlpha (m_clrToolBarGradientDark, 50);


		m_colorToolBarCornerTop = CBCGDrawManager::PixelAlpha (m_clrToolBarGradientLight, 92);
		m_colorToolBarCornerBottom = CBCGDrawManager::PixelAlpha (m_clrToolBarGradientDark, 97);

		m_clrGripper = 
			CBCGDrawManager::PixelAlpha (m_clrToolBarGradientVertDark, 40);

		clrSeparatorDark = 
			CBCGDrawManager::PixelAlpha (m_clrToolBarGradientVertDark, 81);

		m_clrMenuItemBorder = m_clrGripper;
		
		m_clrMenuBorder = 
			CBCGDrawManager::PixelAlpha (clrBase, 80);

		m_clrCaptionBarGradientDark = m_clrCustomizeButtonGradientDark;
		m_clrCaptionBarGradientLight = m_clrCustomizeButtonGradientLight;

		m_clrMenuLight = CBCGDrawManager::PixelAlpha (
			globalData.clrWindow, .96, .96, .96);

		m_brMenuLight.DeleteObject ();
		m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
	}
	else
	{
		m_clrHighlightMenuItem = (COLORREF)-1;

		m_clrHighlightGradientLight = m_clrHighlight;
		m_clrHighlightGradientDark = m_clrHighlightDn;
		m_clrHighlightDnGradientLight = 
			CBCGDrawManager::PixelAlpha (m_clrHighlightGradientLight, 120);

		m_clrCustomizeButtonGradientDark = globalData.clrBarShadow;
		m_clrCustomizeButtonGradientLight = CBCGDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark,
			globalData.clrBarFace, 1, 1, 1);

		m_clrToolBarBottomLine = CBCGDrawManager::PixelAlpha (m_clrToolBarGradientDark, 75);
		m_colorToolBarCornerTop = globalData.clrBarLight;
		m_colorToolBarCornerBottom = m_clrToolBarGradientDark;

		m_clrGripper = globalData.clrBarShadow;
		clrSeparatorDark = globalData.clrBarShadow;

		m_clrCaptionBarGradientLight = globalData.clrBarShadow;
		m_clrCaptionBarGradientDark = globalData.clrBarDkShadow;
	}

	m_clrHighlightDnGradientDark = m_clrHighlightGradientDark;

	m_clrHighlightCheckedGradientLight = m_clrHighlightDnGradientDark;

	m_clrHighlightCheckedGradientDark = 
		CBCGDrawManager::PixelAlpha (m_clrHighlightDnGradientLight, 120);

	m_brTabBack.DeleteObject ();
	m_brTabBack.CreateSolidBrush (m_clrToolBarGradientLight);

	m_penSeparatorLight.DeleteObject ();
	m_penSeparatorLight.CreatePen (PS_SOLID, 1, globalData.clrBarHilite);

	m_brTearOffCaption.DeleteObject ();
	m_brTearOffCaption.CreateSolidBrush (globalData.clrBarFace);

	m_brFace.DeleteObject ();
	m_brFace.CreateSolidBrush (m_clrToolBarGradientLight);

	m_penSeparator.DeleteObject ();
	m_penSeparator.CreatePen (PS_SOLID, 1, clrSeparatorDark);

	m_clrMenuShadowBase = globalData.clrBarFace;

	m_clrToolbarDisabled = CBCGDrawManager::SmartMixColors (
		m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92);

	m_penBottomLine.DeleteObject ();
	m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);

	// --------------------------
	// Calculate TaskPane colors:
	// --------------------------
	if (m_bIsStandardWinXPTheme && m_hThemeExplorerBar != NULL)
	{
		ASSERT (m_pfGetThemeColor != NULL);

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, 3810, &m_clrTaskPaneGradientLight);// GRADIENTCOLOR1
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, 3811, &m_clrTaskPaneGradientDark);	// GRADIENTCOLOR2

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 5, 0, 3802, &m_clrTaskPaneGroupCaptionDark);		// FILLCOLOR
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 12, 0, 3802, &m_clrTaskPaneGroupCaptionSpecDark);// FILLCOLOR
		m_clrTaskPaneGroupCaptionSpecLight = m_clrTaskPaneGroupCaptionDark;

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 5, 0, 3802, &m_clrTaskPaneGroupAreaLight);	// FILLCOLOR
		m_clrTaskPaneGroupAreaDark = m_clrTaskPaneGroupAreaLight;
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 9, 0, 3821, &m_clrTaskPaneGroupAreaSpecLight);	// FILLCOLORHINT
		m_clrTaskPaneGroupAreaSpecDark = m_clrTaskPaneGroupAreaSpecLight;
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 5, 0, 3801, &m_clrTaskPaneGroupBorder);	// BORDERCOLOR
		m_clrTaskPaneGroupCaptionLight = m_clrTaskPaneGroupBorder;
		
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 5, 0, 3803, &m_clrTaskPaneText);	// TEXTCOLOR
	}
	else
	{
		m_clrTaskPaneGradientDark  = m_clrBarGradientDark;
		m_clrTaskPaneGradientLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupCaptionDark  = m_clrBarGradientDark;
		m_clrTaskPaneGroupCaptionLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupCaptionSpecDark = CBCGDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark, 
			m_clrCustomizeButtonGradientLight);
		m_clrTaskPaneGroupCaptionSpecLight  = m_clrCustomizeButtonGradientLight;
		m_clrTaskPaneGroupAreaLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupBorder  = m_clrToolBarGradientLight;
		m_clrTaskPaneText = globalData.clrWindowText;
	}

	m_clrTaskPaneTextHot = CBCGDrawManager::PixelAlpha (m_clrTaskPaneText, 150);

	m_penTaskPaneGroupBorder.DeleteObject ();
	m_penTaskPaneGroupBorder.CreatePen (PS_SOLID, 1, m_clrTaskPaneGroupBorder);
}
//***********************************************************************************
void CBCGVisualManager2003::OnFillHighlightedArea (CDC* pDC, CRect rect, 
							CBrush* pBrush, CBCGToolbarButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);

	BOOL bIsHorz = TRUE;
	BOOL bIsPopupMenu = FALSE;

	COLORREF clr1 = (COLORREF)-1;
	COLORREF clr2 = (COLORREF)-1;

	if (pButton != NULL)
	{
		ASSERT_VALID (pButton);

		bIsHorz = pButton->IsHorizontal ();

		CBCGToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

		bIsPopupMenu = pMenuButton != NULL &&
			pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));

		if (bIsPopupMenu && pBrush == &m_brHighlight)
		{
			if (m_clrHighlightMenuItem != (COLORREF)-1)
			{
				CBrush br (m_clrHighlightMenuItem);
				pDC->FillRect (&rect, &br);
				return;
			}
		}

		if (pMenuButton != NULL &&
			!bIsPopupMenu &&
			pMenuButton->IsDroppedDown ())
		{
			clr1 = CBCGDrawManager::PixelAlpha (
				m_clrToolBarGradientDark, 
				m_bIsStandardWinXPTheme ? 101 : 120);

			clr2 = CBCGDrawManager::PixelAlpha (
				m_clrToolBarGradientLight, 110);
		}
	}

	if (pBrush == &m_brHighlight && m_bIsStandardWinXPTheme)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = bIsPopupMenu ? clr1 : m_clrHighlightGradientLight;
	}
	else if (pBrush == &m_brHighlightDn && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
		clr2 = m_clrHighlightDnGradientLight;
	}
	else if (pBrush == &m_brHighlightChecked && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightCheckedGradientLight : m_clrHighlightCheckedGradientDark;
		clr2 = m_clrHighlightCheckedGradientLight;
	}

	if (clr1 == (COLORREF)-1 || clr2 == (COLORREF)-1)
	{
		CBCGVisualManagerXP::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, bIsHorz);
}
//***********************************************************************************
void CBCGVisualManager2003::OnDrawShowAllMenuItems (CDC* pDC, CRect rect, 
												 CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		const int nRadius = 8;
		rect = CRect (rect.CenterPoint () - CSize (nRadius - 1, nRadius - 1), CSize (nRadius * 2, nRadius * 2));

		CBCGDrawManager dm (*pDC);
		dm.DrawGradientRing (rect, m_clrToolBarGradientDark, m_clrMenuLight,
			(COLORREF)-1,
			45, nRadius);
	}

	CBCGVisualManager::OnDrawShowAllMenuItems (pDC, rect, state);
}
//************************************************************************************
int CBCGVisualManager2003::GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault)
{
	int nHeight = CBCGVisualManager::GetShowAllMenuItemsHeight (pDC, sizeDefault);
	return nHeight + 4;
}
//***********************************************************************************
void CBCGVisualManager2003::OnDrawCaptionBarBorder (CDC* pDC, 
	CBCGCaptionBar* /*pBar*/, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder)
{
	ASSERT_VALID (pDC);

	if (clrBarBorder == (COLORREF) -1)
	{
		clrBarBorder = globalData.clrBarFace;
	}

	CBrush brBorder (clrBarBorder);
	pDC->FillRect (rect, &brBorder);

	if (!bFlatBorder)
	{
		pDC->Draw3dRect (rect, m_clrBarGradientLight, m_clrToolBarBottomLine);
	}
}
//**************************************************************************************
void CBCGVisualManager2003::OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawComboDropButton (pDC, rect,
												bDisabled, bIsDropped,
												bIsHighlighted, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	if (bIsDropped || bIsHighlighted)
	{
		OnFillHighlightedArea (pDC, rect, 
			bIsDropped ? &m_brHighlightDn : &m_brHighlight,
			NULL);

		CPen pen (PS_SOLID, 1, m_clrMenuItemBorder);
		CPen* pOldPen = pDC->SelectObject (&pen);
		ASSERT (pOldPen != NULL);

		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.left, rect.bottom);

		pDC->SelectObject (pOldPen);
	}
	else if (!bDisabled)
	{
		CBCGDrawManager dm (*pDC);
		dm.FillGradient (rect, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);

		pDC->Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
	}

	if (bDisabled)
	{
		rect.InflateRect (1, -1, -1, -1);
		pDC->FillRect (rect, &globalData.brBtnFace);
	}

	CMenuImages::IMAGES_IDS idImage = 
		(bDisabled ? CMenuImages::IdArowDownDsbl : CMenuImages::IdArowDown);

	CMenuImages::Draw (pDC, idImage, rect);
}
//***********************************************************************************
void CBCGVisualManager2003::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawTearOffCaption (pDC, rect, bIsActive);
		return;
	}

	const int iBorderSize = 1;
	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brMenuLight);

	rect.DeflateRect (iBorderSize, 1);

	if (bIsActive)
	{
		OnFillHighlightedArea (pDC, rect, bIsActive ? &m_brHighlight : &m_brBarBkgnd,
			NULL);
	}
	else
	{
		pDC->FillRect (rect, &m_brTearOffCaption);
	}
	
	// Draw gripper:
	OnDrawBarGripper (pDC, rect, FALSE, NULL);

	if (bIsActive)
	{
		pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
	}
}
//***********************************************************************************
void CBCGVisualManager2003::OnDrawMenuBorder (CDC* pDC, 
		CBCGPopupMenu* pMenu, CRect rect)
{
	BOOL bConnectMenuToParent = m_bConnectMenuToParent;

	if (DYNAMIC_DOWNCAST (CCustomizeButton, pMenu->GetParentButton ()) != NULL)
	{
		m_bConnectMenuToParent = FALSE;
	}

	CBCGVisualManagerXP::OnDrawMenuBorder (pDC, pMenu, rect);
	m_bConnectMenuToParent = bConnectMenuToParent;
}
//***********************************************************************************
COLORREF CBCGVisualManager2003::GetThemeColor (HTHEME hTheme, int nIndex) const
{
	if (hTheme != NULL && m_pfGetThemeSysColor != NULL)
	{
		return (*m_pfGetThemeSysColor) (hTheme, nIndex);
	}
	
	return ::GetSysColor (nIndex);
}
//***********************************************************************************
void CBCGVisualManager2003::OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsDialogControl ())
	{
		pDC->FillRect (rect, &globalData.brBtnFace);
		return;
	}

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	CBCGDrawManager dm (*pDC);

	COLORREF clr1 = m_clrToolBarGradientDark;
	COLORREF clr2 = m_clrToolBarGradientLight;

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		dm.FillGradient (rect, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient (rect, clr2, clr1, TRUE);
	}
}
//*************************************************************************************
void CBCGVisualManager2003::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (!pTabWnd->IsOneNoteStyle () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsLeftRightRounded ())
	{
		CBCGVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	CRect rectClip;
	pTabWnd->GetTabsRect (rectClip);

	const int nExtra = (iTab == 0 || bIsActive) ? 0 : rectTab.Height ();

	if (rectTab.left + nExtra + 10 > rectClip.right ||
		rectTab.right - 10 <= rectClip.left)
	{
		return;
	}

	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab ();

	COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);
	if (clrTab == (COLORREF)-1 && bIsActive)
	{
		clrTab = globalData.clrWindow;
	}

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		rectTab.OffsetRect (0, -1);
	}

	CRect rectFill = rectTab;

	#define POINTS_NUM	8
	POINT pts [POINTS_NUM];

	const int nHeight = rectFill.Height ();

	pts [0].x = rectFill.left;
	pts [0].y = rectFill.bottom;

	pts [1].x = rectFill.left + 2;
	pts [1].y = rectFill.bottom - 1;

	pts [2].x = rectFill.left + 4;
	pts [2].y = rectFill.bottom - 2;
	
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

	BOOL bIsCutted = FALSE;

	for (int i = 0; i < POINTS_NUM; i++)
	{
		if (pts [i].x > rectClip.right)
		{
			pts [i].x = rectClip.right;
			bIsCutted = TRUE;
		}

		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
		{
			pts [i].y = rectFill.bottom - pts [i].y + rectFill.top;
		}
	}

	CRgn rgn;
	rgn.CreatePolygonRgn (pts, POINTS_NUM, WINDING);

	pDC->SelectClipRgn (&rgn);

	CRect rectLeft;
	pTabWnd->GetClientRect (rectLeft);
	rectLeft.right = rectClip.left - 1;

	pDC->ExcludeClipRect (rectLeft);

	CBCGDrawManager dm (*pDC);

	COLORREF clrFill = bIsHighlight ? m_clrHighlightMenuItem : clrTab;
	COLORREF clr2;

	if (clrFill != (COLORREF)-1)
	{
		clr2 = CBCGDrawManager::PixelAlpha (clrFill, 150);
	}
	else
	{
		clrFill = m_clrToolBarGradientDark;
		clr2 = m_clrToolBarGradientLight;
	}

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		COLORREF clr = clrFill;
		clrFill = clr2;
		clr2 = clr;

		rectFill.top++;
	}

	dm.FillGradient (rectFill, clrFill, clr2);
	pDC->SelectClipRgn (NULL);

	pDC->ExcludeClipRect (rectLeft);

	if (iTab > 0 && !bIsActive && iTab != pTabWnd->GetFirstVisibleTabNum ())
	{
		CRect rectLeftTab = rectClip;
		rectLeftTab.right = rectFill.left + rectFill.Height () - 10;

		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
		{
			rectLeftTab.top -= 2;
		}
		else
		{
			rectLeftTab.bottom++;
		}

		pDC->ExcludeClipRect (rectLeftTab);
	}

	CPen penGray (PS_SOLID, 1, globalData.clrBarDkShadow);
	CPen penShadow (PS_SOLID, 1, globalData.clrBarShadow);

	CPen* pOldPen = pDC->SelectObject (&penGray);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->Polyline (pts, POINTS_NUM);

	if (bIsCutted)
	{
		pDC->MoveTo (rectClip.right, rectTab.top);
		pDC->LineTo (rectClip.right, rectTab.bottom);
	}

	CRect rectRight = rectClip;
	rectRight.left = rectFill.right;

	pDC->ExcludeClipRect (rectRight);

	CPen penLight (PS_SOLID, 1, bIsHighlight ?
		globalData.clrBarDkShadow : globalData.clrBarHilite);

	pDC->SelectObject (&penLight);

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
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

		if (!bIsActive && !bIsCutted)
		{
			pDC->SelectObject (&penShadow);

			pDC->MoveTo (pts [6].x - 2, pts [6].y - 1);
			pDC->LineTo (pts [6].x - 1, pts [6].y - 1);
		}

		pDC->MoveTo (pts [6].x - 1, pts [6].y);
		pDC->LineTo (pts [7].x - 1, pts [7].y);
	}

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	if (bIsActive)
	{
		const int iBarHeight = 1;
		const int y = (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM) ? 
			(rectTab.top - iBarHeight) : (rectTab.bottom);

		CRect rectFill (CPoint (rectTab.left + 2, y), 
						CSize (rectTab.Width () - 1, iBarHeight));
		
		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
		{
			rectFill.OffsetRect (-1, 1);
		}

		rectFill.right = min (rectFill.right, rectClip.right);

		CBrush br (clrTab);
		pDC->FillRect (rectFill, &br);
	}

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		rectTab.left += rectTab.Height () + CBCGTabWnd::TAB_IMAGE_MARGIN;
	}
	else
	{
		rectTab.left += rectTab.Height ();
		rectTab.right -= CBCGTabWnd::TAB_IMAGE_MARGIN;
	}

	COLORREF clrText = pTabWnd->GetTabTextColor (iTab);
	
	COLORREF cltTextOld = (COLORREF)-1;
	if (!bIsActive && clrText != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor (clrText);
	}

	rectTab.right = min (rectTab.right, rectClip.right - 2);

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	if (cltTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (cltTextOld);
	}

	pDC->SelectClipRgn (NULL);
}
//*********************************************************************************
void CBCGVisualManager2003::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl ())
	{
		CBCGVisualManagerXP::OnFillTab (pDC, rectFill, pbrFill,
									 iTab, bIsActive, pTabWnd);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	COLORREF clr1 = CBCGDrawManager::PixelAlpha (m_clrBarGradientDark, 105);
	
	if (pTabWnd->GetTabBkColor (iTab) != (COLORREF)-1)
	{
		clr1 = pTabWnd->GetTabBkColor (iTab);
		
		if (clr1 == globalData.clrWindow && bIsActive)
		{
			pDC->FillRect (rectFill, &globalData.brWindow);
			return;
		}
	}
	else 
	{
		if (m_bAlwaysFillTab)
		{
			if (bIsActive)
			{
				pDC->FillRect (rectFill, &globalData.brWindow);
				return;
			}
		}
		else
		{
			if (pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded ())
			{
				if (bIsActive)
				{
					pDC->FillRect (rectFill, &globalData.brWindow);
					return;
				}
			}
			else if (!bIsActive)
			{
				return;
			}
		}
	}

	COLORREF clr2 = CBCGDrawManager::PixelAlpha (clr1, 120);

	CBCGDrawManager dm (*pDC);

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_TOP)
	{
		dm.FillGradient (rectFill, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient (rectFill, clr2, clr1, TRUE);
	}
}
//***********************************************************************************
BOOL CBCGVisualManager2003::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGTabWnd* pTabWnd)
{	
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl ())
	{
		return CBCGVisualManagerXP::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());
	if (clrActiveTab == (COLORREF)-1 && 
		(pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ()))
	{
		pDC->FillRect (rect, &globalData.brWindow);
		return TRUE;
	}

	CBCGDrawManager dm (*pDC);

	COLORREF clr1 = m_clrBarGradientDark;

	if (clrActiveTab != (COLORREF)-1)
	{
		clr1 = clrActiveTab;
	}

	COLORREF clr2 = CBCGDrawManager::PixelAlpha (clr1, 130);

	if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		COLORREF clr = clr1;
		clr1 = clr2;
		clr2 = clr;
	}

	dm.FillGradient2 (rect, clr1, clr2, 45);
	return TRUE;
}
//*********************************************************************************
void CBCGVisualManager2003::OnEraseTabsButton (CDC* pDC, CRect rect, CBCGButton* pButton,
									CBCGTabWnd* pWndTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pWndTab == NULL || pWndTab->IsFlatTab () || 
		globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pWndTab->IsDialogControl ())
	{
		CBCGVisualManagerXP::OnEraseTabsButton (pDC, rect, pButton, pWndTab);
		return;
	}

	ASSERT_VALID (pWndTab);

	if ((pWndTab->IsOneNoteStyle () || pWndTab->IsVS2005Style ()) && 
		(pButton->IsPressed () || pButton->IsHighlighted ()))
	{
		CBCGDrawManager dm (*pDC);

		if (pButton->IsPressed ())
		{
			dm.FillGradient (rect, m_clrHighlightDnGradientDark, m_clrHighlightDnGradientLight);
		}
		else
		{
			dm.FillGradient (rect, m_clrHighlightGradientDark, m_clrHighlightGradientLight);
		}

		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CRect rectTabs;
	pWndTab->GetClientRect (&rectTabs);

	CRect rectTabArea;
	pWndTab->GetTabsRect (rectTabArea);

	if (pWndTab->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		rectTabs.top = rectTabArea.top;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints (pButton, rectTabs);
	OnEraseTabsArea (pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn (NULL);
}
//**********************************************************************************
void CBCGVisualManager2003::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT /*uiState*/,
												 CBCGTabWnd* /*pWndTab*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsPushed () || pButton->IsHighlighted ())
	{
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//************************************************************************************
void CBCGVisualManager2003::ModifyGlobalColors ()
{
	if (globalData.m_nBitsPerPixel <= 8 || !m_bIsStandardWinXPTheme || globalData.IsHighContastMode ())
	{
		//----------------------------------------------
		// Theme color may differ from the system color:
		//----------------------------------------------
		globalData.clrBarFace = GetThemeColor (m_hThemeButton, COLOR_3DFACE);
		globalData.clrBarShadow = GetThemeColor (m_hThemeButton, COLOR_3DSHADOW);
		globalData.clrBarHilite = GetThemeColor (m_hThemeButton, COLOR_3DHIGHLIGHT);
		globalData.clrBarDkShadow = GetThemeColor (m_hThemeButton, COLOR_3DDKSHADOW);
		globalData.clrBarLight = GetThemeColor (m_hThemeButton, COLOR_3DLIGHT);
	}
	else
	{
		COLORREF clrBase = GetBaseThemeColor ();

		if (m_WinXPTheme == WinXpTheme_Olive)
		{
			COLORREF clrToolBarGradientDark = CBCGDrawManager::PixelAlpha (
				clrBase, 120);

			COLORREF clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1., 2, 1);

			globalData.clrBarFace = CBCGDrawManager::SmartMixColors (
				clrToolBarGradientDark,
				clrToolBarGradientLight, 1., 2, 1);
		}
		else if (m_WinXPTheme == WinXpTheme_Silver)
		{
			COLORREF clrToolBarGradientDark = CBCGDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
				0.75, 2);

			COLORREF clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1.03);

			globalData.clrBarFace = CBCGDrawManager::PixelAlpha (CBCGDrawManager::SmartMixColors (
				clrToolBarGradientDark,
				clrToolBarGradientLight), 95);
		}
		else
		{
			globalData.clrBarFace = CBCGDrawManager::SmartMixColors (
				GetThemeColor (m_hThemeWindow, /*COLOR_HIGHLIGHT*/29),
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW));
		}

		globalData.clrBarShadow = CBCGDrawManager::PixelAlpha (
			globalData.clrBarFace, 70);
		globalData.clrBarHilite = CBCGDrawManager::PixelAlpha (
			globalData.clrBarFace, 130);
		globalData.clrBarDkShadow = CBCGDrawManager::PixelAlpha (
			globalData.clrBarFace, 50);
		globalData.clrBarLight = CBCGDrawManager::PixelAlpha (
			globalData.clrBarFace, 110);
	}

	globalData.brBarFace.DeleteObject ();
	globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
}
//************************************************************************************
void CBCGVisualManager2003::SetUseGlobalTheme (BOOL bUseGlobalTheme/* = TRUE*/)
{
	m_bUseGlobalTheme = bUseGlobalTheme;

	CBCGVisualManager::GetInstance ()->OnUpdateSystemColors ();
	CBCGVisualManager::GetInstance ()->RedrawAll ();
}
//************************************************************************************
void CBCGVisualManager2003::SetStatusBarOfficeXPLook (BOOL bStatusBarOfficeXPLook/* = TRUE*/)
{
	m_bStatusBarOfficeXPLook = bStatusBarOfficeXPLook;

	CBCGVisualManager::GetInstance ()->RedrawAll ();
}
//***********************************************************************************
void CBCGVisualManager2003::SetDefaultWinXPColors (BOOL bDefaultWinXPColors/* = TRUE*/)
{
	m_bDefaultWinXPColors = bDefaultWinXPColors;

	CBCGVisualManager::GetInstance ()->OnUpdateSystemColors ();
	CBCGVisualManager::GetInstance ()->RedrawAll ();
}
//***********************************************************************************
void CBCGVisualManager2003::GetTabFrameColors (const CBCGTabWnd* pTabWnd,
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
	
	CBCGVisualManagerXP::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (pTabWnd->IsOneNoteStyle () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl () ||
		!m_bIsStandardWinXPTheme)
	{
		return;
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());

	if (clrActiveTab == (COLORREF)-1)
	{
		clrFace = globalData.clrWindow;
	}

	clrDark = globalData.clrBarShadow;
	clrBlack = globalData.clrBarDkShadow;
	clrHighlight = pTabWnd->IsVS2005Style () ? globalData.clrBarShadow : globalData.clrBarLight;
	clrDarkShadow = globalData.clrBarShadow;
	clrLight = globalData.clrBarFace;
}
//************************************************************************************
void CBCGVisualManager2003::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillTasksPaneBackground (pDC, rectWorkArea);
		return;
	}

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rectWorkArea, m_clrTaskPaneGradientDark, m_clrTaskPaneGradientLight, TRUE);
}
//************************************************************************************
void CBCGVisualManager2003::OnDrawTasksGroupCaption(
										CDC* pDC, CBCGTasksGroup* pGroup, 
										BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected/* = FALSE*/, 
										BOOL bCanCollapse /*= FALSE*/)
{
	ASSERT_VALID(pDC);
	ASSERT(pGroup != NULL);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawTasksGroupCaption(
										pDC, pGroup, 
										bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

	// -----------------------
	// Draw caption background
	// -----------------------
	POINT pts [7];

	const int nLeft = pGroup->m_rect.left;
	const int nTop = pGroup->m_rect.top;

	pts [0].x = nLeft;
	pts [0].y = pGroup->m_rect.bottom;

	pts [1].x = nLeft;
	pts [1].y = nTop + 4;

	pts [2].x = nLeft + 1;
	pts [2].y = nTop + 2;
	
	pts [3].x = nLeft + 2;
	pts [3].y = nTop + 1;
	
	pts [4].x = nLeft + 4;
	pts [4].y = nTop;

	pts [5].x = pGroup->m_rect.right;
	pts [5].y = nTop;

	pts [6].x = pGroup->m_rect.right;
	pts [6].y = pGroup->m_rect.bottom;

	CRgn rgn;
	rgn.CreatePolygonRgn (pts, 7, WINDING);

	pDC->SelectClipRgn (&rgn);

	CBCGDrawManager dm (*pDC);
	if (pGroup->m_bIsSpecial)
	{
		dm.FillGradient (pGroup->m_rect, m_clrTaskPaneGroupCaptionSpecDark, 
			m_clrTaskPaneGroupCaptionSpecLight, FALSE);
	}
	else
	{
		dm.FillGradient (pGroup->m_rect, m_clrTaskPaneGroupCaptionLight, 
			m_clrTaskPaneGroupCaptionDark, FALSE);
	}

	pDC->SelectClipRgn (NULL);

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
			(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : m_clrTaskPaneTextHot) :
			pGroup->m_clrTextHot);
	}
	else
	{
		clrTextOld = pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ?
			(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : m_clrTaskPaneText) :
			pGroup->m_clrText);
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);
	
	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();

	CRect rectText = pGroup->m_rect;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	: 
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

				pDC->Draw3dRect(&rectButton, globalData.clrWindow, globalData.clrBarShadow);

				pDC->SetBkColor (clrBckOld);
				pDC->SelectObject (pBrushOld);
			}

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
//************************************************************************************
void CBCGVisualManager2003::OnFillTasksGroupInterior(
								CDC* pDC, CRect rect, BOOL bSpecial /*= FALSE*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillTasksGroupInterior(pDC, rect, bSpecial);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGDrawManager dm (*pDC);
	if (bSpecial)
	{
		dm.FillGradient (rect, m_clrTaskPaneGroupCaptionSpecDark, 
			m_clrTaskPaneGroupCaptionSpecLight, TRUE);
	}
	else
	{
		dm.FillGradient (rect, m_clrTaskPaneGroupAreaDark, 
			m_clrTaskPaneGroupAreaLight, TRUE);
	}
}
//************************************************************************************
void CBCGVisualManager2003::OnDrawTasksGroupAreaBorder(
											CDC* pDC, CRect rect, BOOL /*bSpecial = FALSE*/,
											BOOL /*bNoTitle = FALSE*/)
{
	ASSERT_VALID(pDC);

	// Draw underline
	CPen* pPenOld = (CPen*) pDC->SelectObject (&m_penTaskPaneGroupBorder);

	rect.right -= 1;
	rect.bottom -= 1;
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right, rect.top);
	pDC->LineTo (rect.right, rect.bottom);
	pDC->LineTo (rect.left, rect.bottom);
	pDC->LineTo (rect.left, rect.top);

	pDC->SelectObject (pPenOld);
}
//************************************************************************************
void CBCGVisualManager2003::OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/)
{
	ASSERT_VALID (pTask);
	ASSERT_VALID (pDC);
	
	if (pTask->m_bIsSeparator)
	{
		CRect rectText = pTask->m_rect;

		CPen* pPenOld = (CPen*) pDC->SelectObject (&m_penSeparator);

		pDC->MoveTo (rectText.left, rectText.CenterPoint ().y);
		pDC->LineTo (rectText.right, rectText.CenterPoint ().y);

		pDC->SelectObject (pPenOld);
		return;
	}

	COLORREF clrOld = globalData.clrHotText;
	globalData.clrHotText = bIsHighlighted ? m_clrTaskPaneTextHot : m_clrTaskPaneText;

	CBCGVisualManagerXP::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, bIsSelected);

	globalData.clrHotText = clrOld;
}
//**********************************************************************************
void CBCGVisualManager2003::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brBarFace);

	if (bHilited)
	{
		CBrush br (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ?
			globalData.clrWindow : m_clrHighlightMenuItem == (COLORREF)-1 ? 
			m_clrHighlight : m_clrHighlightMenuItem);

		pDC->FillRect (rect, &br);
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
	else
	{
		pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
	}

	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}
//**********************************************************************************
COLORREF CBCGVisualManager2003::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF clrText = globalData.clrBarText;
	int iImageWidth = CBCGToolBar::GetMenuImageSize ().cx + GetMenuImageMargin ();

	if (bIsSelected)
	{
		rect.left = 0;

		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ?
			m_clrHighlight : m_clrHighlightMenuItem;
		
		CBrush br (color);
		pDC->FillRect (&rect, &br);

		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

		// Now, we should define a menu text color...
		if (GetRValue (color) > 128 &&
			GetGValue (color) > 128 &&
			GetBValue (color) > 128)
		{
			clrText = RGB (0, 0, 0);
		}
		else
		{
			clrText = RGB (255, 255, 255);
		}
	}
	else
	{
		pDC->FillRect (rect, &m_brMenuLight);

		CRect rectImages = rect;
		rectImages.right = rectImages.left + iImageWidth + GetMenuImageMargin ();

		CBCGDrawManager dm (*pDC);
		dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE);

		clrText = globalData.clrBarText;
	}

	return clrText;
}
//*********************************************************************************
void CBCGVisualManager2003::OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, 
											CBCGButton* pButton)
{
	ASSERT_VALID (pDC);

	if (pButton->IsHighlighted () || pButton->IsPushed ())
	{
		OnFillHighlightedArea (pDC, rect, 
			pButton->IsHighlighted () && pButton->IsPushed () ? 
				&m_brHighlightDn : &m_brHighlight, NULL);
	}
	else
	{
		pDC->FillRect (rect, &globalData.brBarFace);
	}
}
//*******************************************************************************
void CBCGVisualManager2003::OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnDrawOutlookBarSplitter (pDC, rectSplitter);
		return;
	}

	CBCGDrawManager dm (*pDC);

	dm.FillGradient (rectSplitter,
					m_clrCaptionBarGradientDark,
					m_clrCaptionBarGradientLight,
					TRUE);

	const int nBoxesNumber = 10;
	const int nBoxSize = rectSplitter.Height () - 3;

	int x = rectSplitter.CenterPoint ().x - nBoxSize * nBoxesNumber / 2;
	int y = rectSplitter.top + 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		pDC->FillSolidRect (x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, 
			globalData.clrBtnHilite);
		pDC->FillSolidRect (x, y, nBoxSize / 2, nBoxSize / 2, 
			m_clrGripper);

		x += nBoxSize;
	}
}
//*******************************************************************************
void CBCGVisualManager2003::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillOutlookBarCaption (pDC, rectCaption, clrText);
		return;
	}

	CRect rectTop = rectCaption;
	rectTop.bottom = rectTop.top + 3;
	pDC->FillSolidRect	(rectCaption, globalData.clrBarFace);

	rectCaption.top = rectTop.bottom;

	CBCGDrawManager dm (*pDC);

	dm.FillGradient (rectCaption,
		m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
	clrText = globalData.clrBarHilite;
}
//*********************************************************************************
COLORREF CBCGVisualManager2003::GetWindowColor () const
{
	return GetThemeColor (m_hThemeWindow, COLOR_WINDOW);
}
//************************************************************************************
void CBCGVisualManager2003::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnHighlightRarelyUsedMenuItems (pDC, rectRarelyUsed);
		return;
	}

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rectRarelyUsed, m_clrMenuRarelyUsed, m_clrToolBarGradientDark, FALSE);
}
//*************************************************************************************
void CBCGVisualManager2003::OnDrawControlBorder (CWnd* pWndCtrl)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGVisualManagerXP::OnDrawControlBorder (pWndCtrl);
		return;
	}

	ASSERT_VALID (pWndCtrl);

	CWindowDC dc (pWndCtrl);

	CRect rect;
	pWndCtrl->GetWindowRect (rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	COLORREF clrBorder = (COLORREF)-1;

	if ((*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, 3801, &clrBorder) != S_OK)
	{
		CBCGVisualManagerXP::OnDrawControlBorder (pWndCtrl);
		return;
	}

	dc.Draw3dRect (&rect, clrBorder, clrBorder);
	rect.DeflateRect (1, 1);
	dc.Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
}
//****************************************************************************************
COLORREF CBCGVisualManager2003::GetBaseThemeColor ()
{
	return m_bIsStandardWinXPTheme && m_hThemeWindow != NULL ?
		GetThemeColor (m_hThemeWindow, 29) :
		globalData.clrBarFace;
}
//*****************************************************************************************
void CBCGVisualManager2003::OnDrawStatusBarSizeBox (CDC* pDC, CBCGStatusBar* pStatBar,
			CRect rectSizeBox)
{
	if (m_hThemeScrollBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawStatusBarSizeBox (pDC, pStatBar, rectSizeBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX,
		SZB_RIGHTALIGN, &rectSizeBox, 0);
}
//************************************************************************************
void CBCGVisualManager2003::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTree == NULL)
	{
		CBCGVisualManagerXP::OnDrawExpandingBox (pDC, rect, bIsOpened, colorBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeTree, pDC->GetSafeHdc(), TVP_GLYPH,
		bIsOpened ? GLPS_OPENED : GLPS_CLOSED, &rect, 0);
}
//**************************************************************************************
void CBCGVisualManager2003::OnDrawStatusBarProgress (CDC* pDC, CBCGStatusBar* pStatusBar,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
			BOOL bProgressText)
{
	if (!DrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText))
	{
		CBCGVisualManagerXP::OnDrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText);
	}
}
//**************************************************************************************
void CBCGVisualManager2003::OnHighlightQuickCustomizeMenuButton (CDC* pDC, 
	CBCGToolbarMenuButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.IsHighContastMode ())
	{
		pDC->FillRect (rect, &m_brBarBkgnd);
	}
	else
	{
		CBrush br (m_clrToolBarGradientLight);
		pDC->FillRect (rect, &br);
	}

	pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
}
//****************************************************************************************
void CBCGVisualManager2003::OnDrawHeaderCtrlBorder (CBCGHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (m_hThemeHeader == NULL)
	{
		CBCGVisualManagerXP::OnDrawHeaderCtrlBorder (pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	int nState = HIS_NORMAL;

	if (bIsPressed)
	{
		nState = HIS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = HIS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERITEM, nState, &rect, 0);
}
//****************************************************************************************
void CBCGVisualManager2003::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle)
{
	if (!m_bStatusBarOfficeXPLook || m_hThemeStatusBar == NULL)
	{
		CBCGVisualManagerXP::OnDrawStatusBarPaneBorder (pDC, pBar,
						rectPane, uiID, nStyle);
	}

	if (m_hThemeStatusBar != NULL &&
		!(nStyle & SBPS_NOBORDERS))
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), 1 /*SP_PANE*/,
			0, &rectPane, 0);
	}
}
//****************************************************************************************
void CBCGVisualManager2003::OnFillPopupWindowBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnFillPopupWindowBackground (pDC, rect);
		return;
	}

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight);
}
//**********************************************************************************
void CBCGVisualManager2003::OnDrawPopupWindowBorder (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
}
//**********************************************************************************
COLORREF  CBCGVisualManager2003::OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* pPopupWnd)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnDrawPopupWindowCaption (pDC, rectCaption, pPopupWnd);
	}

	CBCGDrawManager dm (*pDC);
	dm.FillGradient (rectCaption, 
		m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

	if (pPopupWnd->HasSmallCaption ())
	{
		CRect rectGripper = rectCaption;

		int xCenter = rectGripper.CenterPoint ().x;
		int yCenter = rectGripper.CenterPoint ().y;

		rectGripper.left = xCenter - 20;
		rectGripper.right = xCenter + 20;

		rectGripper.top = yCenter - 4;
		rectGripper.bottom = yCenter + 2;

		OnDrawBarGripper (pDC, rectGripper, FALSE, NULL);
	}

    // get the text color
	return globalData.clrBarHilite;
}
//**********************************************************************************
void CBCGVisualManager2003::OnErasePopupWindowButton (CDC* pDC, CRect rc, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnErasePopupWindowButton (pDC, rc, pButton);
		return;
	}

	if (pButton->IsPressed ())
	{
		COLORREF color = m_clrHighlightDnGradientLight == (COLORREF)-1 ?
			m_clrHighlightDn : m_clrHighlightDnGradientLight;
		
		CBrush br (color);
		pDC->FillRect (&rc, &br);
		return;
	}
	else if (pButton->IsHighlighted () || pButton->IsPushed ())
	{
		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ?
			m_clrHighlight : m_clrHighlightMenuItem;
		
		CBrush br (color);
		pDC->FillRect (&rc, &br);
		return;
	}

	CRect rectParent;
	pButton->GetParent ()->GetClientRect (rectParent);

	pButton->GetParent ()->MapWindowPoints (pButton, rectParent);
	OnFillPopupWindowBackground (pDC, rectParent);
}
//**********************************************************************************
void CBCGVisualManager2003::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rc, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsHighlighted () || pButton->IsPushed () ||
		pButton->IsCaptionButton ())
	{
		pDC->Draw3dRect (rc, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//**********************************************************************************
COLORREF CBCGVisualManager2003::GetPropListGroupColor (CBCGPropList* pPropList)
{
	return CBCGVisualManager::GetPropListGroupColor (pPropList);
}
//********************************************************************************
COLORREF CBCGVisualManager2003::GetPropListGroupTextColor (CBCGPropList* pPropList)
{
	return CBCGVisualManager::GetPropListGroupTextColor (pPropList);
}
//**************************************************************************************
COLORREF CBCGVisualManager2003::OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar,
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	CBCGDrawManager dm (*pDC);

	if (pBar->IsVertDocked () || CBCGSizingControlBar::IsCaptionAlwaysOnTop ())
	{
		if (!bActive)
		{
			dm.FillGradient (rectCaption, 
							m_clrToolBarGradientDark, 
							m_clrToolBarGradientLight, 
							TRUE);
		}
		else
		{
			dm.FillGradient (rectCaption,	
							m_clrHighlightGradientDark,
							m_clrHighlightGradientLight,
							TRUE);
		}
	}
	else
	{
		if (!bActive)
		{
			dm.FillGradient (rectCaption, 
							m_clrToolBarGradientLight, 
							m_clrToolBarGradientDark,
							FALSE);
		}
		else
		{
			dm.FillGradient (rectCaption,	
							m_clrHighlightGradientLight,
							m_clrHighlightGradientDark,
							FALSE);
		}
	}
	return globalData.clrBarText;
}
//***************************************************************************************
void CBCGVisualManager2003::OnDrawCaptionButton (
						CDC* pDC, CBCGSCBButton* pButton, BOOL bHorz,
						BOOL bMaximized, BOOL bDisabled)
{
	ASSERT_VALID (pDC);
    CRect rc = pButton->GetRect();

	if (pButton->m_bPushed && pButton->m_bFocused && !bDisabled)
	{
		OnFillHighlightedArea (pDC, rc, &m_brHighlightDn, NULL);
	}
	else if (pButton->m_bPushed || pButton->m_bFocused)
	{
		if (!bDisabled)
		{
			OnFillHighlightedArea (pDC, rc, &m_brHighlight, NULL);
		}
	}

	pButton->DrawIcon (pDC, rc, bHorz, bMaximized, bDisabled);

	if ((pButton->m_bPushed || pButton->m_bFocused) && !bDisabled)
	{
		COLORREF clrDark = globalData.clrBarDkShadow;
		pDC->Draw3dRect (rc, clrDark, clrDark);
	}
}
