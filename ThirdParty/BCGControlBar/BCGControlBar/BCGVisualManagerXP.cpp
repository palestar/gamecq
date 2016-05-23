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
// BCGVisualManagerXP.cpp: implementation of the CBCGVisualManagerXP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGVisualManagerXP.h"
#include "BCGDrawManager.h"
#include "BCGSizingControlBar.h"
#include "BCGMenuBar.h"
#include "BCGPopupMenu.h"
#include "BCGToolbarMenuButton.h"
#include "BCGOutlookBar.h"
#include "BCGOutlookButton.h"
#include "BCGColorBar.h"
#include "BCGTabWnd.h"
#include "BCGToolbarEditBoxButton.h"
#include "Globals.h"
#include "BCGTasksPane.h"
#include "BCGPopupWindow.h"
#include "BCGPropList.h"
#include "CustomizeButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGVisualManagerXP, CBCGVisualManager)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGVisualManagerXP::CBCGVisualManagerXP(BOOL bIsTemporary) :
	CBCGVisualManager (bIsTemporary)
{
	m_bMenuFlatLook = TRUE;
	m_bShadowHighlightedImage = TRUE;
	m_bEmbossDisabledImage = FALSE;
	m_bFadeInactiveImage = TRUE;
	m_bLook2000 = TRUE;
	m_nMenuShadowDepth = 4;
	m_bConnectMenuToParent = TRUE;

	m_nVertMargin = 4;
	m_nHorzMargin = 4;
	m_nGroupVertOffset = 4;
	m_nGroupCaptionHeight = 0;
	m_nGroupCaptionHorzOffset = 0;
	m_nGroupCaptionVertOffset = 0;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = FALSE;
	m_nMenuBorderSize = 1;
	m_bShdowDroppedDownMenuButton = TRUE;

	m_bOfficeXPStyleMenus = TRUE;
	m_bDrawLastTabLine = TRUE;
	
	globalData.UpdateSysColors ();
	OnUpdateSystemColors ();
}
//**********************************************************************************
CBCGVisualManagerXP::~CBCGVisualManagerXP()
{
}
//****************************************************************************************
void CBCGVisualManagerXP::OnUpdateSystemColors ()
{
	CBCGVisualManager::OnUpdateSystemColors ();

	m_brBarBkgnd.DeleteObject ();
	m_brMenuRarelyUsed.DeleteObject ();
	m_brMenuLight.DeleteObject ();
	m_brHighlight.DeleteObject ();
	m_brHighlightDn.DeleteObject ();
	m_brHighlightChecked.DeleteObject ();
	m_penSeparator.DeleteObject ();
	m_brTabBack.DeleteObject ();

	COLORREF clrTabBack;
	COLORREF clrSeparator;

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		COLORREF clrWindow = GetWindowColor ();
		COLORREF clrFace = globalData.clrBarFace;

		m_clrMenuLight = RGB (
			(219 * GetRValue (clrWindow) + 36 * GetRValue (clrFace)) / 255,
			(219 * GetGValue (clrWindow) + 36 * GetGValue (clrFace)) / 255,
			(219 * GetBValue (clrWindow) + 36 * GetBValue (clrFace)) / 255);

		double H, S, L;
		CBCGDrawManager::RGBtoHSL (clrFace, &H, &S, &L);

		double S1;
		double L1;

		if (S < 0.1)
		{
			L1 = min (1., L + (1. - L) * .5);
			S1 = S == 0 ? 0 : min (1., S + .1);
		}
		else
		{
			L1 = min (1., 0.5 * L + 0.5);
			S1 = min (1., S * 2);
		}

		clrTabBack = CBCGDrawManager::HLStoRGB_ONE (H, L1, S1);

		m_clrBarBkgnd = RGB (
			(40 * GetRValue (clrWindow) + 215 * GetRValue (clrFace)) / 255,
			(40 * GetGValue (clrWindow) + 215 * GetGValue (clrFace)) / 255,
			(40 * GetBValue (clrWindow) + 215 * GetBValue (clrFace)) / 255);

		m_clrInactiveTabText = CBCGDrawManager::PixelAlpha (clrFace, 55);

		m_clrMenuRarelyUsed = CBCGDrawManager::PixelAlpha (
			m_clrBarBkgnd, 94);

		COLORREF clrHL = globalData.clrHilite;
		CBCGDrawManager::RGBtoHSL (clrHL, &H, &S, &L);

		COLORREF clrMix = RGB (
			(77 * GetRValue (clrHL) + 178 * GetRValue (m_clrMenuLight)) / 255,
			(77 * GetGValue (clrHL) + 178 * GetGValue (m_clrMenuLight)) / 255,
			(77 * GetBValue (clrHL) + 178 * GetBValue (m_clrMenuLight)) / 255);

		if (L > .8)	// The highlight color is very light
		{
			m_clrHighlight = CBCGDrawManager::PixelAlpha (clrMix, 91);
			m_clrHighlightDn = CBCGDrawManager::PixelAlpha (clrMix, 98);
			m_clrMenuItemBorder = CBCGDrawManager::PixelAlpha (globalData.clrHilite, 84);
		}
		else
		{
			m_clrHighlight = CBCGDrawManager::PixelAlpha (clrMix, 105);
			m_clrHighlightDn = CBCGDrawManager::PixelAlpha (m_clrHighlight, 87);
			m_clrMenuItemBorder = globalData.clrHilite;
		}

		m_clrHighlightChecked = CBCGDrawManager::PixelAlpha (RGB (
			(GetRValue (clrHL) + 5 * GetRValue (m_clrMenuLight)) / 6,
			(GetGValue (clrHL) + 5 * GetGValue (m_clrMenuLight)) / 6,
			(GetBValue (clrHL) + 5 * GetBValue (m_clrMenuLight)) / 6), 
			100);

		clrSeparator = CBCGDrawManager::PixelAlpha (
			globalData.clrBarFace, .86, .86, .86);

		m_clrPaneBorder = CBCGDrawManager::PixelAlpha (
				globalData.clrBarFace, 108);

		m_clrMenuBorder = CBCGDrawManager::PixelAlpha (clrFace, 55);

		m_clrGripper = CBCGDrawManager::PixelAlpha (
			globalData.clrBarShadow, 110);
	}
	else
	{
		m_clrMenuLight = globalData.clrWindow;
		m_clrBarBkgnd = globalData.clrBtnFace;

		if (globalData.m_bIsBlackHighContrast)
		{
			m_clrHighlightChecked = m_clrHighlightDn = m_clrHighlight = globalData.clrHilite;
			m_clrMenuRarelyUsed = globalData.clrBtnFace;
		}
		else
		{
			m_clrHighlightDn = m_clrHighlight = globalData.clrBtnFace;
			m_clrHighlightChecked = globalData.clrWindow;
			m_clrMenuRarelyUsed = globalData.clrBarLight;
		}

		clrTabBack = globalData.clrBtnFace;
		m_clrInactiveTabText = globalData.clrBtnDkShadow;
		clrSeparator = globalData.clrBtnShadow;
		m_clrPaneBorder = globalData.clrBtnShadow;
		m_clrMenuBorder = globalData.clrBtnDkShadow;
		m_clrGripper = globalData.clrBtnShadow;

		m_clrMenuItemBorder = globalData.IsHighContastMode () ?
			globalData.clrBtnDkShadow : globalData.clrHilite;
	}

	m_brBarBkgnd.CreateSolidBrush (m_clrBarBkgnd);
	m_brMenuRarelyUsed.CreateSolidBrush (m_clrMenuRarelyUsed);
	m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
	m_brHighlight.CreateSolidBrush (m_clrHighlight);
	m_brHighlightDn.CreateSolidBrush (m_clrHighlightDn);
	m_brHighlightChecked.CreateSolidBrush (m_clrHighlightChecked);
	m_brTabBack.CreateSolidBrush (clrTabBack);
	m_penSeparator.CreatePen (PS_SOLID, 1, clrSeparator);

	m_clrMenuShadowBase = (COLORREF)-1;	// Used in derived classes
	m_clrPressedButtonBorder = (COLORREF)-1;	// Used in derived classes

	m_penMenuItemBorder.DeleteObject ();
	m_penMenuItemBorder.CreatePen (PS_SOLID, 1, m_clrMenuItemBorder);
}
//***************************************************************************************
void CBCGVisualManagerXP::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CControlBar* pBar)
{
	if (m_brGripperHorz.GetSafeHandle () == NULL)
	{
		CreateGripperBrush ();
	}

	BOOL bSideBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGSizingControlBar));
	BOOL bMenuBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGMenuBar));

	CRect rectFill = rectGripper;

	if (!bSideBar)
	{
		if (bHorz)
		{
			rectFill.DeflateRect (rectFill.Width () / 2 + 2, 6);
		}
		else
		{
			rectFill.DeflateRect (6, rectFill.Height () / 2 + 2);
		}
	}
	else
	{
		if (bHorz)
		{
			rectFill.DeflateRect (4, 0);
		}
		else
		{
			rectFill.DeflateRect (0, 4);
		}

		bHorz = !bHorz;
	}

	COLORREF clrTextOld = pDC->SetTextColor (m_clrGripper);
	COLORREF clrBkOld = pDC->SetBkColor (bSideBar || bMenuBar ? 
		globalData.clrBarFace : m_clrBarBkgnd);

	pDC->FillRect (rectFill, bHorz ? &m_brGripperHorz : &m_brGripperVert);

	if (bSideBar)
	{
		//------------------
		// Draw bar caption:
		//------------------
		int nOldBkMode = pDC->SetBkMode (OPAQUE);
		pDC->SetTextColor (globalData.clrBarText);

		const CFont& font = CBCGMenuBar::GetMenuFont (bHorz);

		CFont* pOldFont = pDC->SelectObject ((CFont*) &font);

		CString strCaption;
		pBar->GetWindowText (strCaption);
		strCaption = _T(" ") + strCaption + _T(" ");

		CRect rectText = rectGripper;
		UINT uiTextFormat = 0;

		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);

		CPoint ptTextOffset (0, 0);
		if (bHorz)
		{
			ptTextOffset.y = (rectGripper.Height () - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptTextOffset.x = (rectGripper.Width () - tm.tmHeight + 1) / 2;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;
			pDC->DrawText (strCaption, &rectText, uiTextFormat);
		}
		else
		{
			rectText.left = rectText.right - ptTextOffset.x;
			rectText.top = rectGripper.top + ptTextOffset.y;
			rectText.bottom = rectGripper.top + 3 * ptTextOffset.y;

			uiTextFormat |= DT_NOCLIP;

			pDC->DrawText (strCaption, &rectText, uiTextFormat);
		}

		pDC->SelectObject(pOldFont);
		pDC->SetBkMode(nOldBkMode);
	}

	pDC->SetTextColor (clrTextOld);
	pDC->SetBkColor (clrBkOld);
}
//****************************************************************************************
void CBCGVisualManagerXP::OnDrawMenuBorder (CDC* pDC, CBCGPopupMenu* pMenu, CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pMenu);

	const BOOL bRTL = pMenu->GetExStyle() & WS_EX_LAYOUTRTL;

	pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, m_clrMenuLight, m_clrMenuLight);

	CRect rectLeft (1, 1, 2, rect.bottom - 1);
	pDC->FillRect (rectLeft, &m_brBarBkgnd);

	//------------------------------------------------
	// Quick Customize Office XP like draw popup menu
	//------------------------------------------------
	CBCGPopupMenu* pParentPopup = pMenu->GetParentPopupMenu();
	if (pParentPopup != NULL)
	{
		if (pParentPopup->IsQuickCustomize () && !bRTL)
		{
			CBCGToolbarMenuButton* pParentBtn = pMenu->GetParentButton();
			if ((pParentBtn != NULL) && (pParentBtn->IsQuickMode()))
			{
				CBCGPopupMenu* pParent = (CBCGPopupMenu*)pMenu->GetParentPopupMenu();

				CRect rcParent;
				pParent->GetWindowRect(rcParent);

				CRect rcCurrent;
				pMenu->GetWindowRect(rcCurrent);

				CBCGToolbarMenuButton* pBtn = pMenu->GetMenuItem(0);
				CRect rcButton = pBtn->Rect();

				CRect rectBorder;
				rectBorder.SetRectEmpty();

				if (rcParent.left > rcCurrent.left) 
				{
					if (rcParent.top <= rcCurrent.top)
					{
						rectBorder.SetRect (
							rect.right - 1,
							rect.top,
							rect.right + 1,
							rcButton.bottom);
					}
					else
					{
						// up
						rectBorder.SetRect (
							rect.right - 1,
							rect.bottom - rcButton.Height (), 
							rect.right + 1,
							rect.bottom);
					}
				}
				else
				{
					if (rcParent.top <= rcCurrent.top)
					{
						rectBorder.SetRect (
							rect.left - 1,
							rect.top, 
							rect.left + 1,
							rcButton.bottom);
					}
					else
					{
						// up
						rectBorder.SetRect (
							rect.left - 1, 
							rect.bottom - rcButton.Height (),
							rect.left + 1,
							rect.bottom);
					}
				}
				
				if (!rectBorder.IsRectEmpty ())
				{
					pDC->FillRect (rectBorder, &m_brBarBkgnd);
				}
			}
		}
	}

	if (!CBCGToolBar::IsCustomizeMode ())
	{
		//-------------------------------------
		// "Connect" menu to the parent button:
		//-------------------------------------
		CBCGToolbarMenuButton* pParentMenuBtn = pMenu->GetParentButton ();
		if (m_bConnectMenuToParent &&
			pParentMenuBtn != NULL && pMenu->GetParentPopupMenu () == NULL && 
			pParentMenuBtn->IsBorder ())
		{
			CRect rectConnect;
			rectConnect.SetRectEmpty ();

			CRect rectParent = pParentMenuBtn->Rect ();
			CWnd* pWnd = pParentMenuBtn->GetParentWnd();
			pWnd->ClientToScreen(rectParent);
			pMenu->ScreenToClient(&rectParent);

			switch (pMenu->GetDropDirection ())
			{
			case CBCGPopupMenu::DROP_DIRECTION_BOTTOM:
				rectConnect = CRect (rectParent.left + 1, rect.top - 1, rectParent.right - 1, rect.top);
				
				if (rectConnect.Width () > rect.Width () + 2)
				{
					return;
				}

				break;

			case CBCGPopupMenu::DROP_DIRECTION_TOP:
				rectConnect = CRect (rectParent.left + 1, rect.bottom, rectParent.right - 1, rect.bottom + 1);

				if (rectConnect.Width () > rect.Width () + 2)
				{
					return;
				}

				break;

			case CBCGPopupMenu::DROP_DIRECTION_RIGHT:
				rectConnect = CRect (rect.left - 1, rectParent.top + 1, rect.left, rectParent.bottom - 1);

				if (rectConnect.Height () > rect.Height () + 2)
				{
					return;
				}

				break;

			case CBCGPopupMenu::DROP_DIRECTION_LEFT:
				rectConnect = CRect (rect.right, rectParent.top + 1, rect.right + 1, rectParent.bottom - 1);

				if (rectConnect.Height () > rect.Height () + 2)
				{
					return;
				}

				break;
			}

			CRect rectBorder = rect;
			rectBorder.InflateRect (1, 1);
			rectConnect.IntersectRect (&rectConnect, &rectBorder);
			rectParent.InflateRect (1, 1);
			rectConnect.IntersectRect (&rectConnect, &rectParent);

			pDC->FillRect (rectConnect, &m_brBarBkgnd);
		}
	}
}
//****************************************************************************************
void CBCGVisualManagerXP::OnDrawMenuShadow (CDC* pPaintDC, const CRect& rectClient, const CRect& rectExclude,
								int nDepth,  int iMinBrightness,  int iMaxBrightness,  
								CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight, BOOL bRTL)
{
	ASSERT_VALID (pPaintDC);
	ASSERT_VALID (pBmpSaveBottom);
	ASSERT_VALID (pBmpSaveRight);

	if (rectExclude.IsRectNull())
	{
		//------------------------
		// Simple draw the shadow:
		//------------------------
		CBCGDrawManager dm (*pPaintDC);
		dm.DrawShadow (rectClient, nDepth, iMinBrightness, iMaxBrightness,
					pBmpSaveBottom, pBmpSaveRight, !bRTL, m_clrMenuShadowBase);
	}
	else
	{
		//--------------------------------------------
		// Copy screen content into the memory bitmap:
		//--------------------------------------------
		CDC dcMem;
		if (!dcMem.CreateCompatibleDC (pPaintDC))
		{
			ASSERT (FALSE);
			return;
		}

		//--------------------------------------------
		// Gets the whole menu and changes the shadow.
		//--------------------------------------------
		CRect rectBmp (0, 0, rectClient.Width(), rectClient.Height());
		int cx = rectBmp.Width() + nDepth;
		int cy = rectBmp.Height() + nDepth;
		CBitmap	bmpMem;
		if (!bmpMem.CreateCompatibleBitmap (pPaintDC, cx, cy))
		{
			ASSERT (FALSE);
			return;
		}

		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
		ASSERT (pOldBmp != NULL);

		dcMem.BitBlt (0, 0, cx, cy, pPaintDC, rectClient.left, rectClient.top, SRCCOPY);

		//-----------------
		// Draw the shadow:
		//-----------------
		CBCGDrawManager dm (dcMem);
		dm.DrawShadow (rectBmp, nDepth, iMinBrightness, iMaxBrightness,
					pBmpSaveBottom, pBmpSaveRight,
					!bRTL, m_clrMenuShadowBase);

		//------------------------------------------
		// Do not cover rectExclude with the shadow:
		//------------------------------------------
		dcMem.BitBlt (rectExclude.left - rectClient.left, rectExclude.top - rectClient.top,
			rectExclude.Width(), rectExclude.Height(), 
			pPaintDC, rectExclude.left, rectExclude.top, SRCCOPY);

		//-----------------------------------------
		// Copy shadowed bitmap back to the screen:
		//-----------------------------------------
		pPaintDC->BitBlt (rectClient.left, rectClient.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}
}
//****************************************************************************************
void CBCGVisualManagerXP::OnDrawBarBorder (CDC* pDC, CControlBar* pBar, CRect& rect)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	DWORD dwStyle = pBar->m_dwStyle;
	if (!(dwStyle & CBRS_BORDER_ANY))
	{
		return;
	}

	COLORREF clrBckOld = pDC->GetBkColor ();	// FillSolidRect changes it

	if(pBar->m_dwStyle & CBRS_BORDER_LEFT)
		pDC->FillSolidRect(0, 0, 1, rect.Height() - 1, globalData.clrBarFace);
	if(pBar->m_dwStyle & CBRS_BORDER_TOP)
		pDC->FillSolidRect(0, 0, rect.Width()-1 , 1, globalData.clrBarFace);
	if(pBar->m_dwStyle & CBRS_BORDER_RIGHT)
		pDC->FillSolidRect(rect.right, 0/*RGL~:1*/, -1,
			rect.Height()/*RGL-: - 1*/, globalData.clrBarFace);	
	if(pBar->m_dwStyle & CBRS_BORDER_BOTTOM)
		pDC->FillSolidRect(0, rect.bottom, rect.Width()-1, -1, globalData.clrBarFace);

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
//****************************************************************************************
void CBCGVisualManagerXP::OnFillBarBackground (CDC* pDC, CControlBar* pBar,
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

	if (rectClip.IsRectEmpty ())
	{
		rectClip = rectClient;
	}

	CRuntimeClass* pBarClass = pBar->GetRuntimeClass ();

	if (pBarClass == NULL || pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGMenuBar)))
	{
		CBCGVisualManager::OnFillBarBackground (pDC, pBar, rectClient, rectClip);
		return;
	}

	if (pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGOutlookBar)))
	{
		CBCGOutlookBar* pOlBar = DYNAMIC_DOWNCAST (CBCGOutlookBar, pBar);
		ASSERT_VALID (pOlBar);

		if (pOlBar->IsBackgroundTexture ())
		{
			CBCGVisualManager::OnFillBarBackground (pDC, pBar, rectClient, rectClip);
			return;
		}
	}

	if (pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGColorBar)))
	{
		pDC->FillRect (rectClip, 
			((CBCGColorBar*) pBar)->IsTearOff () ? &m_brBarBkgnd : &m_brMenuLight);
		return;
	}

	if (pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPopupMenuBar)))
	{
		pDC->FillRect (rectClip, &m_brMenuLight);

		BOOL bQuickMode = FALSE;

		CBCGPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPopupMenuBar, pBar);
		if (pMenuBar != NULL && !pMenuBar->m_bDisableSideBarInXPMode)
		{
			CWnd* pWnd = pMenuBar->GetParent();

			if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CBCGPopupMenu)))
			{
				CBCGPopupMenu* pMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, pWnd);

				if (pMenu != NULL && pMenu->IsCustomizePane ())
				{
					bQuickMode = TRUE;
				}
			}

			CRect rectImages = rectClient;

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

			rectImages.DeflateRect (0, 1);
			pDC->FillRect (rectImages, &m_brBarBkgnd);
		}

		return;
	}

	if (pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGToolBar)))
	{
		pDC->FillRect (rectClip, &m_brBarBkgnd);
		return;
	}

	CBCGVisualManager::OnFillBarBackground (pDC, pBar, rectClient, rectClip);
}
//**************************************************************************************
void CBCGVisualManagerXP::OnDrawSeparator (CDC* pDC, CControlBar* pBar,
										 CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pDC);

	CRect rectSeparator = rect;

	CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
	ASSERT (pOldPen != NULL);

	int x1, x2;
	int y1, y2;

	if (bHorz)
	{
		x1 = x2 = (rect.left + rect.right) / 2;
		y1 = rect.top;
		y2 = rect.bottom - 1;
	}
	else
	{
		y1 = y2 = (rect.top + rect.bottom) / 2;
		x1 = rect.left;
		x2 = rect.right;

		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)) &&
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGColorBar)))
		{
			x1 = rect.left + CBCGToolBar::GetMenuImageSize ().cx + 
				GetMenuImageMargin () + 1;

			CRect rectBar;
			pBar->GetClientRect (rectBar);

			if (rectBar.right - x2 < 50) // Last item in row
			{
				x2 = rectBar.right;
			}

			if (((CBCGPopupMenuBar*) pBar)->m_bDisableSideBarInXPMode)
			{
				x1 = 0;
			}

			//---------------------------------
			//	Maybe Quick Customize separator
			//---------------------------------
			if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
			{
				CWnd* pWnd = pBar->GetParent();
				if (pWnd != NULL && pWnd->IsKindOf (RUNTIME_CLASS (CBCGPopupMenu)))
				{
					CBCGPopupMenu* pMenu = (CBCGPopupMenu*)pWnd;
					if (pMenu->IsCustomizePane())
					{
						x1 = rect.left + 2*CBCGToolBar::GetMenuImageSize ().cx + 
								3*GetMenuImageMargin () + 2;
					}
				}
			}
		}
	}

	pDC->MoveTo (x1, y1);
	pDC->LineTo (x2, y2);

	pDC->SelectObject (pOldPen);
}
//**************************************************************************************
void CBCGVisualManagerXP::OnDrawButtonBorder (CDC* pDC,
		CBCGToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	COLORREF clrBorder = m_clrMenuItemBorder;

	CBCGToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
	BOOL bIsMenuButton = pMenuButton != NULL;

	BOOL bIsPopupMenu = bIsMenuButton &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));

	BOOL bIsPressedBorder = !bIsPopupMenu;

	if (bIsMenuButton && !bIsPopupMenu && 
		pMenuButton->IsDroppedDown ())
	{
		bIsPressedBorder = FALSE;

		CBCGPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu ();
		if (pPopupMenu != NULL && 
			(pPopupMenu->IsWindowVisible () || pPopupMenu->IsShown()))
		{
			clrBorder = m_clrMenuBorder;
			ExtendMenuButton (pMenuButton, rect);

			BOOL bRTL = pPopupMenu->GetExStyle() & WS_EX_LAYOUTRTL;

			if (m_bShdowDroppedDownMenuButton && !bRTL && 
				CBCGMenuBar::IsMenuShadows () &&
				!CBCGToolBar::IsCustomizeMode () &&
				globalData.m_nBitsPerPixel > 8 &&
				!globalData.IsHighContastMode () &&
				!pPopupMenu->IsRightAlign ())
			{
				CBCGDrawManager dm (*pDC);

				dm.DrawShadow (rect, m_nMenuShadowDepth, 100, 75, NULL, NULL,
					TRUE, m_clrMenuShadowBase);
			}
		}
	}

	const BOOL bIsChecked = (pButton->m_nStyle & TBBS_CHECKED);

	switch (state)
	{
	case ButtonsIsPressed:
		if (bIsPressedBorder && m_clrPressedButtonBorder != (COLORREF)-1 &&
			!bIsChecked &&
			rect.Width () > 5 && rect.Height () > 5)
		{
			clrBorder = m_clrPressedButtonBorder;
		}

	case ButtonsIsHighlighted:
		if (bIsPopupMenu && bIsChecked)
		{
			rect.bottom ++;
		}

		pDC->Draw3dRect (rect, clrBorder, clrBorder);
	}
}
//*****************************************************************************************
void CBCGVisualManagerXP::OnFillButtonInterior (CDC* pDC,
	CBCGToolbarButton* pButton, CRect rect, CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	if (CBCGToolBar::IsCustomizeMode () && 
		!CBCGToolBar::IsAltCustomizeMode () && !pButton->IsLocked ())
	{
		return;
	}

	CBCGToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
	BOOL bIsMenuButton = pMenuButton != NULL;

	BOOL bIsPopupMenu = bIsMenuButton &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));

	if (!bIsPopupMenu && !m_bEnableToolbarButtonFill)
	{
		return;
	}

	CBrush* pBrush = ((pButton->m_nStyle & TBBS_PRESSED) && !bIsPopupMenu) ? 
		&m_brHighlightDn : &m_brHighlight;

	if (bIsMenuButton && !bIsPopupMenu && pMenuButton->IsDroppedDown ())
	{
		ExtendMenuButton (pMenuButton, rect);
		pBrush = &m_brBarBkgnd;
	}
	
	if (pButton->m_nStyle & TBBS_CHECKED)
	{
		pBrush = (state == ButtonsIsHighlighted) ? 
				&m_brHighlightDn : &m_brHighlightChecked;
	}

	if (bIsMenuButton && (pButton->m_nStyle & TBBS_DISABLED))
	{
		pBrush = &m_brMenuLight;
	}

	switch (state)
	{
	case ButtonsIsPressed:
	case ButtonsIsHighlighted:
		if ((pButton->m_nStyle & TBBS_CHECKED) == 0)
		{
			rect.DeflateRect (1, 1);
		}

		OnFillHighlightedArea (pDC, rect, pBrush, pButton);
	}
}
//************************************************************************************
void CBCGVisualManagerXP::OnHighlightMenuItem (CDC* pDC, CBCGToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBrush* pBrush = (pButton->m_nStyle & TBBS_DISABLED) ? 
					&m_brMenuLight : &m_brHighlight;

	rect.DeflateRect (1, 0);

	OnFillHighlightedArea (pDC, rect, pBrush, pButton);
	pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

	clrText = GetHighlightedMenuItemTextColor (pButton);
}
//*****************************************************************************
COLORREF CBCGVisualManagerXP::GetHighlightedMenuItemTextColor (CBCGToolbarMenuButton* pButton)
{
	ASSERT_VALID (pButton);

	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		return globalData.clrGrayedText;
	}

	if (GetRValue (m_clrHighlight) > 128 &&
		GetGValue (m_clrHighlight) > 128 &&
		GetBValue (m_clrHighlight) > 128)
	{
		return RGB (0, 0, 0);
	}
	else
	{
		return RGB (255, 255, 255);
	}
}
//**************************************************************************************
void CBCGVisualManagerXP::OnHighlightQuickCustomizeMenuButton (CDC* pDC, 
	CBCGToolbarMenuButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brBarBkgnd);
	pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
}
//************************************************************************************
void CBCGVisualManagerXP::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	pDC->FillRect (rectRarelyUsed, &m_brMenuRarelyUsed);
}
//***********************************************************************************
void CBCGVisualManagerXP::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd)
{
	#define TEXT_MARGIN				4
	#define IMAGE_MARGIN			4

	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () ||
		pTabWnd->IsColored () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsLeftRightRounded ())
	{
		CBCGVisualManager::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	COLORREF	clrDark;
	COLORREF	clrBlack;
	COLORREF	clrHighlight;
	COLORREF	clrFace;
	COLORREF	clrDarkShadow;
	COLORREF	clrLight;
	CBrush*		pbrFace = NULL;
	CBrush*		pbrBlack = NULL;
				   
	GetTabFrameColors (
		pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight,
		pbrFace, pbrBlack);

	CPen penGray (PS_SOLID, 1, clrDark);
	CPen penDkGray (PS_SOLID, 1, clrBlack);
	CPen penHiLight (PS_SOLID, 1, clrHighlight);
	
	CPen* pOldPen = pDC->SelectObject (&penGray);
	ASSERT (pOldPen != NULL);
	
	if (iTab != pTabWnd->GetActiveTab () - 1)
	{
		if (iTab < pTabWnd->GetVisibleTabsNum () - 1 || m_bDrawLastTabLine)
		{
			pDC->MoveTo (rectTab.right, rectTab.top + 3);
			pDC->LineTo (rectTab.right, rectTab.bottom - 3);
		}
	}

	if (bIsActive)
	{
		if (pTabWnd->GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
		{
			CRect rectFace = rectTab;
			rectFace.top --;
			
			OnFillTab (pDC, rectFace, pbrFace, iTab, bIsActive, pTabWnd);
			
			pDC->SelectObject (&penDkGray);

			pDC->MoveTo (rectTab.right, rectTab.top);
			pDC->LineTo (rectTab.right, rectTab.bottom);
			pDC->LineTo (rectTab.left, rectTab.bottom);

			pDC->SelectObject (&penHiLight);
			pDC->LineTo (rectTab.left, rectTab.top - 2);
		}
		else
		{
			CPen penLight (PS_SOLID, 1, m_clrMenuLight);

			CRect rectFace = rectTab;
			rectFace.bottom ++;
			rectFace.left ++;
			
			OnFillTab (pDC, rectFace, pbrFace, iTab, bIsActive, pTabWnd);
			
			pDC->SelectObject (&penDkGray);
			pDC->MoveTo (rectTab.right, rectTab.bottom);
			pDC->LineTo (rectTab.right, rectTab.top);

			pDC->SelectObject (&penHiLight);

			pDC->LineTo (rectTab.right, rectTab.top);
			pDC->LineTo(rectTab.left, rectTab.top);
			pDC->LineTo(rectTab.left, rectTab.bottom);
		}
	}

	pDC->SelectObject (pOldPen);

	COLORREF clrText;

	if (pTabWnd->IsDialogControl ())
	{
		clrText = globalData.clrBtnText;
	}
	else
	{
		clrText = bIsActive ? globalData.clrBarText : m_clrInactiveTabText;
	}

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, clrText);
}
//*********************************************************************************
void CBCGVisualManagerXP::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pbrFill);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->GetTabBkColor (iTab) != (COLORREF)-1 && !bIsActive)
	{
		CBrush br (pTabWnd->GetTabBkColor (iTab));
		pDC->FillRect (rectFill, &br);
		return;
	}

	if (pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsLeftRightRounded ())
	{
		CBCGVisualManager::OnFillTab (pDC, rectFill, pbrFill,
									 iTab, bIsActive, pTabWnd);
	}
	else if (bIsActive)
	{
		pDC->FillRect (rectFill, pbrFill);
	}
}
//***********************************************************************************
void CBCGVisualManagerXP::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab ())
	{
		CBCGVisualManager::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsDialogControl ())
	{
		pDC->FillRect (rect, &globalData.brBtnFace);
		return;
	}

	pDC->FillRect (rect, &m_brTabBack);
}
//**********************************************************************************
COLORREF CBCGVisualManagerXP::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF clrText = globalData.clrBarText;

	int iImageWidth = CBCGToolBar::GetMenuImageSize ().cx + GetMenuImageMargin ();

	if (bIsSelected)
	{
		if (m_bEnableToolbarButtonFill)
		{
			rect.left = 0;
		}

		OnFillHighlightedArea (pDC, rect, &m_brHighlight, NULL);

		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

		// Now, we should define a menu text color...
		if (GetRValue (m_clrHighlight) > 128 &&
			GetGValue (m_clrHighlight) > 128 &&
			GetBValue (m_clrHighlight) > 128)
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
		const int MENU_IMAGE_MARGIN = 2;
		rectImages.right = rectImages.left + iImageWidth + MENU_IMAGE_MARGIN;

		pDC->FillRect (rectImages, &m_brBarBkgnd);

		clrText = globalData.clrBarText;
	}

	return clrText;
}
//***********************************************************************************
void CBCGVisualManagerXP::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	const int iBorderSize = 1;
	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brMenuLight);

	rect.DeflateRect (iBorderSize, 1);
	OnFillHighlightedArea (pDC, rect, bIsActive ? &m_brHighlight : &m_brBarBkgnd,
		NULL);
	
	// Draw gripper:
	int nGripperWidth = max (20, CBCGToolBar::GetMenuImageSize ().cx * 2);

	CRect rectGripper = rect;
	rectGripper.DeflateRect ((rectGripper.Width () - nGripperWidth) / 2, 1);

	if (m_brGripperHorz.GetSafeHandle () == NULL)
	{
		CreateGripperBrush ();
	}

	COLORREF clrTextOld = pDC->SetTextColor (bIsActive ?
		globalData.clrBarDkShadow : globalData.clrBarShadow);
	COLORREF clrBkOld = pDC->SetBkColor (
		bIsActive ? m_clrHighlight : m_clrBarBkgnd);

	if (bIsActive)
	{
		rectGripper.DeflateRect (0, 1);
	}

	pDC->FillRect (rectGripper, &m_brGripperHorz);

	pDC->SetTextColor (clrTextOld);
	pDC->SetBkColor (clrBkOld);

	if (bIsActive)
	{
		pDC->Draw3dRect (rect, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
	}
}
//************************************************************************************
void CBCGVisualManagerXP::CreateGripperBrush ()
{
	ASSERT (m_brGripperHorz.GetSafeHandle () == NULL);
	ASSERT (m_brGripperVert.GetSafeHandle () == NULL);

	WORD horzHatchBits [8] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };

	CBitmap bmpGripperHorz;
	bmpGripperHorz.CreateBitmap (8, 8, 1, 1, horzHatchBits);

	m_brGripperHorz.CreatePatternBrush (&bmpGripperHorz);

	WORD vertHatchBits[8] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

	CBitmap bmpGripperVert;
	bmpGripperVert.CreateBitmap (8, 8, 1, 1, vertHatchBits);

	m_brGripperVert.CreatePatternBrush (&bmpGripperVert);
}
//***********************************************************************************
void CBCGVisualManagerXP::ExtendMenuButton (CBCGToolbarMenuButton* pMenuButton,
											CRect& rect)
{
	ASSERT_VALID (pMenuButton);

	CBCGPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu ();
	if (pPopupMenu == NULL || pPopupMenu->GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rectMenu;
	pPopupMenu->GetWindowRect (rectMenu);

	if (DYNAMIC_DOWNCAST (CCustomizeButton, pMenuButton) != NULL)
	{
		CControlBar* pParentBar = DYNAMIC_DOWNCAST (
			CControlBar, pMenuButton->GetParentWnd ());

		if (pParentBar != NULL)
		{
			BOOL bIsHorz = (pParentBar->GetBarStyle () & CBRS_ORIENT_HORZ);

			CRect rectScreen = rect;
			pParentBar->ClientToScreen (&rectScreen);

			if (bIsHorz)
			{
				rectScreen.top = rectMenu.top;
				rectScreen.bottom = rectMenu.bottom;
			}
			else
			{
				rectScreen.left = rectMenu.left;
				rectScreen.right = rectMenu.right;
				rectScreen.bottom++;
			}

			CRect rectInter;
			if (!rectInter.IntersectRect (rectScreen, rectMenu))
			{
				return;
			}
		}
	}

	int nGrow = 4;

	switch (pPopupMenu->GetDropDirection ())
	{
	case CBCGPopupMenu::DROP_DIRECTION_BOTTOM:
		if (rectMenu.Width () < rect.Width ())
		{
			nGrow = 1;
		}

		rect.bottom += nGrow;
		break;

	case CBCGPopupMenu::DROP_DIRECTION_TOP:
		if (rectMenu.Width () < rect.Width ())
		{
			nGrow = 1;
		}

		rect.top -= nGrow;
		break;

	case CBCGPopupMenu::DROP_DIRECTION_RIGHT:
		if (rectMenu.Height () < rect.Height ())
		{
			nGrow = 1;
		}

		rect.right += nGrow;
		break;

	case CBCGPopupMenu::DROP_DIRECTION_LEFT:
		if (rectMenu.Height () < rect.Height ())
		{
			nGrow = 1;
		}

		rect.left -= nGrow;
		break;
	}
}
//***********************************************************************************
void CBCGVisualManagerXP::OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight)
{
	ASSERT_VALID (pDC);

	BOOL bIsDisabled = (nStyle & TBBS_DISABLED);
	BOOL bIsPressed = (nStyle & TBBS_PRESSED);

	CMenuImages::IMAGES_IDS imageID;

	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		imageID = (bHighlight && bIsPressed) ? CMenuImages::IdCloseWhite :
			bIsDisabled ? CMenuImages::IdCloseDsbl : CMenuImages::IdClose;
		break;

	case SC_MINIMIZE:
		imageID = (bHighlight && bIsPressed) ? CMenuImages::IdMinimizeWhite :
			bIsDisabled ? CMenuImages::IdMinimizeDsbl : CMenuImages::IdMinimize;
		break;

	case SC_RESTORE:
		imageID = (bHighlight && bIsPressed) ? CMenuImages::IdRestoreWhite :
			bIsDisabled ? CMenuImages::IdRestoreDsbl : CMenuImages::IdRestore;
		break;

	default:
		return;
	}

	if (bHighlight && !bIsDisabled)
	{
		OnFillHighlightedArea (pDC, rect, 
			bIsPressed ? &m_brHighlightDn : &m_brHighlight, NULL);

		COLORREF clrBorder = m_clrMenuItemBorder;
		pDC->Draw3dRect (rect, clrBorder, clrBorder);
	}

	CMenuImages::Draw (pDC, imageID, rect);
}
//********************************************************************************
void CBCGVisualManagerXP::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGStatusBar* /*pBar*/,
					CRect rectPane, UINT /*uiID*/, UINT nStyle)
{
	if (!(nStyle & SBPS_NOBORDERS))
	{
		if (nStyle & SBPS_POPOUT)
		{
			CBCGDrawManager dm (*pDC);
			dm.HighlightRect (rectPane);
		}

		// Draw pane border:
		pDC->Draw3dRect (rectPane, m_clrPaneBorder, m_clrPaneBorder);
	}
}
//**************************************************************************************
void CBCGVisualManagerXP::OnDrawComboDropButton (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* /*pButton*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	COLORREF clrText = pDC->GetTextColor ();

	CMenuImages::IMAGES_IDS idImage = 
		(bDisabled ? CMenuImages::IdArowDownDsbl : CMenuImages::IdArowDown);

	if (bIsDropped || bIsHighlighted)
	{
		OnFillHighlightedArea (pDC, rect, 
			bIsDropped ? &m_brHighlightDn : &m_brHighlight,
			NULL);

		CPen* pOldPen = pDC->SelectObject (&m_penMenuItemBorder);
		ASSERT (pOldPen != NULL);

		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.left, rect.bottom);

		pDC->SelectObject (pOldPen);

		if (bIsDropped)
		{
			idImage = CMenuImages::IdArowDownWhite;
		}
	}
	else
	{
		pDC->FillRect (rect, &globalData.brBarFace);
		pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarHilite);
	}

	CMenuImages::Draw (pDC, idImage, rect);
	pDC->SetTextColor (clrText);
}
//*************************************************************************************
void CBCGVisualManagerXP::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL /*bDisabled*/,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGToolbarComboBoxButton* /*pButton*/)
{
	if (bIsHighlighted || bIsDropped)
	{
		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (&rect,  m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//*************************************************************************************
void CBCGVisualManagerXP::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsHighlighted,
												CBCGToolbarEditBoxButton* pButton)
{
	if (!CBCGToolbarEditBoxButton::IsFlatMode ())
	{
		CBCGVisualManager::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted)
	{
		pDC->Draw3dRect (&rect,  m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//**************************************************************************************
COLORREF CBCGVisualManagerXP::GetToolbarButtonTextColor (CBCGToolbarButton* pButton, 
														CBCGVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	if (!globalData.IsHighContastMode ())
	{
		BOOL bDisabled = (CBCGToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
			(!CBCGToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

		if (pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton)))
		{
			if (bDisabled)
			{
				return globalData.clrGrayedText;
			}

			return globalData.IsHighContastMode () ? 
				globalData.clrWindowText : globalData.clrBarText;
		}

		if (state == ButtonsIsHighlighted && 
			(pButton->m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)))
		{
			return globalData.clrTextHilite;
		}
	}

	return	CBCGVisualManager::GetToolbarButtonTextColor (pButton, state);
}
//*********************************************************************************
void CBCGVisualManagerXP::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGButton* pButton,
											  CBCGTabWnd* pWndTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pWndTab);

	if (pWndTab->IsFlatTab ())
	{
		CBrush* pBrush = pButton->IsPressed () ? 
			&m_brHighlightDn : pButton->IsHighlighted () ? &m_brHighlight : &globalData.brBarFace;

		pDC->FillRect (rect, pBrush);
		OnFillHighlightedArea (pDC, rect, pBrush, NULL);
	}
	else
	{
		pDC->FillRect (rect, &m_brTabBack);
	}
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT /*uiState*/,
												 CBCGTabWnd* pWndTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pWndTab);

	if (pWndTab->IsFlatTab ())
	{
		if (pButton->IsPushed () || pButton->IsHighlighted ())
		{
			COLORREF clrDark = globalData.clrBarDkShadow;
			pDC->Draw3dRect (rect, clrDark, clrDark);
		}
	}
	else
	{
		if (pButton->IsPushed () || pButton->IsHighlighted ())
		{
			if (pButton->IsPressed ())
			{
				pDC->Draw3dRect (rect, globalData.clrBarDkShadow, m_clrGripper);
			}
			else
			{
				pDC->Draw3dRect (rect, m_clrGripper, globalData.clrBarDkShadow);
			}
		}
	}
}
//*********************************************************************************
void CBCGVisualManagerXP::OnEraseOutlookCaptionButton (CDC* pDC, CRect rect, 
											CBCGButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBrush* pBrush = pButton->IsPressed () ? 
		&m_brHighlightDn : pButton->IsHighlighted () ? &m_brHighlight : &globalData.brBarFace;

	pDC->FillRect (rect, pBrush);
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawOutlookCaptionButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGButton* pButton, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsPushed () || pButton->IsHighlighted ())
	{
		COLORREF clrDark = globalData.clrBarDkShadow;
		pDC->Draw3dRect (rect, clrDark, clrDark);
	}
}
//**********************************************************************************
void CBCGVisualManagerXP::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	::FillRect (pDC->GetSafeHdc (), rectWorkArea, ::GetSysColorBrush (COLOR_WINDOW));
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawTasksGroupCaption(CDC* pDC, CBCGTasksGroup* pGroup, 
						BOOL bIsHighlighted, BOOL /*bIsSelected*/, BOOL bCanCollapse)
{
	ASSERT_VALID(pDC);
	ASSERT(pGroup != NULL);

	const int nIconMargin = 10;

	// ------------------------------
	// Draw group caption (Office XP)
	// ------------------------------
	
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
			::GetSysColor(COLOR_WINDOWTEXT) : pGroup->m_clrTextHot);
	}
	else
	{
		clrTextOld = pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ?
			::GetSysColor(COLOR_WINDOWTEXT) : pGroup->m_clrText);
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
		rectButton.left = max(rectButton.left, rectButton.right - sizeButton.cx);
		rectButton.top = max(rectButton.top, rectButton.bottom - sizeButton.cy);
		
		if (rectButton.Width () >= sizeButton.cx && rectButton.Height () >= sizeButton.cy)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CPen* pPenOld = (CPen*) pDC->SelectObject (&globalData.penHilite);
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject (&m_brHighlight);
				COLORREF clrBckOld = pDC->GetBkColor ();

				pDC->Rectangle(&rectButton);

				pDC->SetBkColor (clrBckOld);
				pDC->SelectObject (pPenOld);
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
//**********************************************************************************
void CBCGVisualManagerXP::OnEraseTasksGroupArea(CDC* /*pDC*/, CRect /*rect*/)
{
}
//**********************************************************************************
void CBCGVisualManagerXP::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL /*bSpecial*/)
{
	ASSERT_VALID(pDC);

	// Draw underline
	CPen* pPenOld = (CPen*) pDC->SelectObject (&globalData.penBarShadow);
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right, rect.top);
	pDC->SelectObject (&pPenOld);

}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawTasksGroupAreaBorder(CDC* /*pDC*/, CRect /*rect*/, 
													 BOOL /*bSpecial*/, BOOL /*bNoTitle*/)
{
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawTask(CDC* pDC, CBCGTask* pTask, CImageList* pIcons, BOOL bIsHighlighted, BOOL /*bIsSelected*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT(pTask != NULL);

	CRect rectText = pTask->m_rect;

	if (pTask->m_bIsSeparator)
	{
		CPen* pPenOld = (CPen*) pDC->SelectObject (&globalData.penBarShadow);

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
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bIsLabel)
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			globalData.clrWindowText : pTask->m_clrText);
	}
	else if (!pTask->m_bEnabled)
	{
		pDC->SetTextColor (globalData.clrGrayedText);
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
	}
	else if (bIsHighlighted)
	{
		pDC->SetTextColor (pTask->m_clrTextHot == (COLORREF)-1 ?
			globalData.clrHotText : pTask->m_clrTextHot);
		pFontOld = pDC->SelectObject (&globalData.fontUnderline);
	}
	else
	{
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			globalData.clrHotText : pTask->m_clrText);
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
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
void CBCGVisualManagerXP::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brWindow);

	if (bHilited)
	{
		pDC->FillRect (rect, &m_brHighlight);
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}
//***********************************************************************************
void CBCGVisualManagerXP::OnFillHighlightedArea (CDC* pDC, CRect rect, 
		CBrush* pBrush, CBCGToolbarButton* /*pButton*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);

	pDC->FillRect (rect, pBrush);
}
//*******************************************************************************
void CBCGVisualManagerXP::OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rectSplitter, &globalData.brBarFace);
	pDC->Draw3dRect (rectSplitter, globalData.clrBarLight, globalData.clrBarShadow);

	int nGripperWidth = max (20, CBCGToolBar::GetMenuImageSize ().cx * 2);

	CRect rectGripper = rectSplitter;
	rectGripper.DeflateRect ((rectGripper.Width () - nGripperWidth) / 2, 2);

	if (m_brGripperHorz.GetSafeHandle () == NULL)
	{
		CreateGripperBrush ();
	}

	COLORREF clrTextOld = pDC->SetTextColor (globalData.clrBarShadow);
	COLORREF clrBkOld = pDC->SetBkColor (m_clrBarBkgnd);

	pDC->FillRect (rectGripper, &m_brGripperHorz);

	pDC->SetTextColor (clrTextOld);
	pDC->SetBkColor (clrBkOld);
}
//********************************************************************************
void CBCGVisualManagerXP::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, 
												  COLORREF& clrText)
{
	CRect rectTop = rectCaption;
	rectTop.bottom = rectTop.top + 3;
	pDC->FillSolidRect	(rectTop, globalData.clrBarFace);

	rectCaption.top = rectTop.bottom;

	pDC->FillSolidRect	(rectCaption, globalData.clrBarShadow);
	clrText = globalData.clrBarHilite;
}
//***************************************************************************************
BOOL CBCGVisualManagerXP::OnFillOutlookPageButton (CBCGButton* pButton,
											CDC* pDC, const CRect& rectClient,
											COLORREF& clrText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGOutlookBar* pBar = DYNAMIC_DOWNCAST (CBCGOutlookBar,
		pButton->GetParent ());
	if (pBar == NULL || !pBar->IsMode2003 ())
	{
		return FALSE;
	}

	CRect rect = rectClient;

	if (pButton->IsPressed () ||
		(pButton->IsHighlighted () && pButton->IsChecked ()))
	{
		pDC->FillRect (rect, &m_brHighlightDn);
	}
	else if (pButton->IsHighlighted ())
	{
		pDC->FillRect (rect, &m_brHighlight);
	}
	else if (pButton->IsChecked ())
	{
		pDC->FillRect (rect, &m_brHighlightChecked);
	}
	else
	{
		pDC->FillRect (rect, &globalData.brBarFace);
	}

	clrText = globalData.clrBarText;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGVisualManagerXP::OnDrawOutlookPageButtonBorder (CBCGButton* pButton,
												CDC* pDC, CRect& rectClient,
												UINT /*uiState*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGOutlookBar* pBar = DYNAMIC_DOWNCAST (CBCGOutlookBar,
		pButton->GetParent ());
	if (pBar == NULL || !pBar->IsMode2003 ())
	{
		return FALSE;
	}

	if (pButton->IsPressed () ||
		pButton->IsHighlighted () ||
		pButton->IsChecked ())
	{
		pDC->Draw3dRect (rectClient, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	return TRUE;
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawSpinButtons (CDC* pDC, CRect rectSpin, 
	int nState, BOOL bOrientation, CBCGSpinButtonCtrl* /*pSpinCtrl*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	CRect rect [2];
	rect[0] = rect[1] = rectSpin;

	if (!bOrientation) 
	{
		rect[0].DeflateRect(0, 0, 0, rect[0].Height() / 2);
		rect[1].top = rect[0].bottom ;
	}
	else
	{
		rect[0].DeflateRect(0, 0, rect[0].Width() / 2, 0);
		rect[1].left = rect[0].right ;
	}

	CMenuImages::IMAGES_IDS id[2][2] = {{CMenuImages::IdArowUp, CMenuImages::IdArowDown}, {CMenuImages::IdArowLeft, CMenuImages::IdArowRight}};

	int idxPressed = (nState & (SPIN_PRESSEDUP | SPIN_PRESSEDDOWN)) - 1;
	
	int idxHighlighted = -1;
	if (nState & SPIN_HIGHLIGHTEDUP)
	{
		idxHighlighted = 0;
	}
	else if (nState & SPIN_HIGHLIGHTEDDOWN)
	{
		idxHighlighted = 1;
	}

	for (int i = 0; i < 2; i ++)
	{
		if (idxPressed == i || idxHighlighted == i)
		{
			OnFillHighlightedArea (pDC, rect [i], 
				(idxPressed == i) ? &m_brHighlightDn : &m_brHighlight, NULL);
		}
		else
		{
			pDC->FillRect (rect[i], &globalData.brBarFace);
			pDC->Draw3dRect (rect[i], globalData.clrBarHilite, globalData.clrBarHilite);
		}

		CMenuImages::Draw (pDC, id[bOrientation][i], rect[i]);
	}

	if (idxHighlighted >= 0)
	{
		CRect rectHot = rect [idxHighlighted];
		pDC->Draw3dRect (rectHot, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//*********************************************************************************
COLORREF CBCGVisualManagerXP::GetWindowColor () const
{
	return globalData.clrWindow;
}
//********************************************************************************
void CBCGVisualManagerXP::OnDrawSplitterBorder (CDC* pDC, CBCGSplitterWnd* /*pSplitterWnd*/, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
	rect.InflateRect(-CX_BORDER, -CY_BORDER);
	pDC->Draw3dRect (rect, globalData.clrBarFace, globalData.clrBarFace);
}
//********************************************************************************
void CBCGVisualManagerXP::OnDrawSplitterBox (CDC* pDC, CBCGSplitterWnd* /*pSplitterWnd*/, CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rect, globalData.clrBarFace, globalData.clrBarFace);
}
void CBCGVisualManagerXP::OnDrawPopupWindowBorder (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, m_clrMenuLight, m_clrMenuLight);
}
//**********************************************************************************
COLORREF  CBCGVisualManagerXP::OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPopupWindow* /*pPopupWnd*/)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rectCaption, &m_brHighlight);

    // get the text color
	return globalData.clrBarText;
}
//**********************************************************************************
void CBCGVisualManagerXP::OnErasePopupWindowButton (CDC* pDC, CRect rc, CBCGPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsPressed ())
	{
		CBrush br (m_clrHighlightDn);
		pDC->FillRect (&rc, &br);
		return;
	}
	else if (pButton->IsHighlighted () || pButton->IsPushed ())
	{
		CBrush br (m_clrHighlight);
		pDC->FillRect (&rc, &br);
		return;
	}

	CRect rectParent;
	pButton->GetParent ()->GetClientRect (rectParent);

	pButton->GetParent ()->MapWindowPoints (pButton, rectParent);
	OnFillPopupWindowBackground (pDC, rectParent);
}
//**********************************************************************************
void CBCGVisualManagerXP::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rc, CBCGPopupWndButton* pButton)
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
void CBCGVisualManagerXP::OnFillPopupWindowBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &m_brMenuLight);
}
//**********************************************************************************
COLORREF CBCGVisualManagerXP::GetPropListGroupColor (CBCGPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (globalData.m_nBitsPerPixel <= 8)
	{
		return CBCGVisualManager::GetPropListGroupColor (pPropList);
	}

	return CBCGDrawManager::PixelAlpha (
			pPropList->DrawControlBarColors () ? 
			globalData.clrBarFace : globalData.clrBtnFace, 94);
}
//**********************************************************************************
COLORREF CBCGVisualManagerXP::GetPropListGroupTextColor (CBCGPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	return pPropList->DrawControlBarColors () ?
		globalData.clrBarShadow : globalData.clrBtnShadow;
}
//**************************************************************************************
COLORREF CBCGVisualManagerXP::OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* /*pBar*/, 
			BOOL bActive, CRect rectCaption, CRect /*rectButtons*/)
{
	ASSERT_VALID (pDC);

	rectCaption.DeflateRect (0, 1);

	CPen pen (PS_SOLID, 1, 
		bActive ? globalData.clrBarLight : globalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject (&pen);

	CBrush* pOldBrush = (CBrush*) pDC->SelectObject (
		bActive ? &globalData.brActiveCaption : &globalData.brBarFace);

	if (bActive)
	{
		rectCaption.InflateRect (1, 1);
	}

	pDC->RoundRect (rectCaption, CPoint (2, 2));

	pDC->SelectObject (pOldBrush);
	pDC->SelectObject (pOldPen);

    // get the text color
    COLORREF clrCptnText = bActive ?
        globalData.clrCaptionText :
        globalData.clrBarText;

	return clrCptnText;
}
//*************************************************************************************
void CBCGVisualManagerXP::OnDrawButtonSeparator (CDC* pDC,
		CBCGToolbarButton* /*pButton*/, CRect rect, CBCGVisualManager::BCGBUTTON_STATE /*state*/,
		BOOL bHorz)
{
	CPen* pOldPen = pDC->SelectObject (&m_penMenuItemBorder);
	ASSERT (pOldPen != NULL);

	if (bHorz)
	{
		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.left, rect.bottom);
	}
	else
	{
		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.right, rect.top);
	}

	pDC->SelectObject (pOldPen);
}
