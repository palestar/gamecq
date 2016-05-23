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
// BCGVisualManagerVS2005.cpp: implementation of the CBCGVisualManagerVS2005 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGVisualManagerVS2005.h"
#include "BCGDrawManager.h"
#include "BCGTabWnd.h"
#include "BCGToolBar.h"
#include "BCGSizingControlBar.h"
#include "BCGToolbarMenuButton.h"
#include "BCGStatusBar.h"
#include "BCGPropList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if (defined(SCHEMA_STRINGS)) || (! defined(TMSCHEMA_H))
#define SP_PANE				1
#endif

BOOL CBCGVisualManagerVS2005::m_bRoundedAutohideButtons = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGVisualManagerVS2005, CBCGVisualManager2003)

CBCGVisualManagerVS2005::CBCGVisualManagerVS2005()
{
	m_bAlwaysFillTab = TRUE;
	m_b3DTabWideBorder = FALSE;
	m_bShdowDroppedDownMenuButton = TRUE;
	m_bDrawLastTabLine = FALSE;
	m_colorActiveTabBorder = (COLORREF)-1;
}

CBCGVisualManagerVS2005::~CBCGVisualManagerVS2005()
{

}

void CBCGVisualManagerVS2005::OnUpdateSystemColors ()
{
	BOOL bDefaultWinXPColors = m_bDefaultWinXPColors;

	m_clrPressedButtonBorder = (COLORREF)-1;

	m_CurrAppTheme = GetStandardWinXPTheme ();

	if (m_CurrAppTheme != WinXpTheme_Silver)
	{
		m_bDefaultWinXPColors = FALSE;
	}

	CBCGVisualManager2003::OnUpdateSystemColors ();

	if (!bDefaultWinXPColors)
	{
		return;
	}

	COLORREF clrMenuButtonDroppedDown = m_clrBarBkgnd;
	COLORREF clrMenuItemCheckedHighlight = m_clrHighlightDn;

	if (m_hThemeComboBox == NULL ||
		m_pfGetThemeColor == NULL ||
		(*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, 3801, &m_colorActiveTabBorder) != S_OK)
	{
		m_colorActiveTabBorder = (COLORREF)-1;
	}

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		m_clrCustomizeButtonGradientLight = CBCGDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark,
			globalData.clrBarFace, 1.5, 1, 1);

		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarGradientDark = CBCGDrawManager::PixelAlpha (
				m_clrToolBarGradientDark, 83);

			m_clrToolBarGradientLight = CBCGDrawManager::SmartMixColors (
				GetBaseThemeColor (), 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1., 3, 2);
		}

		if (m_CurrAppTheme == WinXpTheme_Blue)
		{

			m_clrCustomizeButtonGradientDark = CBCGDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientDark, 90);

			m_clrCustomizeButtonGradientLight = CBCGDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientLight, 115);

			m_clrToolBarBottomLine = CBCGDrawManager::PixelAlpha (
				m_clrToolBarBottomLine, 85);
		}
		else if (m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarBottomLine = CBCGDrawManager::PixelAlpha (
				m_clrToolBarBottomLine, 110);

			m_clrCustomizeButtonGradientDark = m_clrToolBarBottomLine;

			m_clrCustomizeButtonGradientLight = CBCGDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientLight, 120);

			m_clrHighlightDn = globalData.clrHilite;

			m_clrHighlight = CBCGDrawManager::PixelAlpha (
				m_clrHighlightDn, 124);

			m_clrHighlightChecked = CBCGDrawManager::PixelAlpha (
				GetThemeColor (m_hThemeWindow, 27 /*COLOR_GRADIENTACTIVECAPTION*/), 98);

			m_brHighlight.DeleteObject ();
			m_brHighlightDn.DeleteObject ();

			m_brHighlight.CreateSolidBrush (m_clrHighlight);
			m_brHighlightDn.CreateSolidBrush (m_clrHighlightDn);

			m_brHighlightChecked.DeleteObject ();
			m_brHighlightChecked.CreateSolidBrush (m_clrHighlightChecked);
		}

		clrMenuButtonDroppedDown = CBCGDrawManager::PixelAlpha (
			m_clrBarBkgnd, 107);

		clrMenuItemCheckedHighlight = GetThemeColor (m_hThemeWindow, COLOR_HIGHLIGHT);

		if (m_CurrAppTheme != WinXpTheme_Silver)
		{
			m_clrBarGradientLight = CBCGDrawManager::PixelAlpha (
					m_clrToolBarGradientLight, 95);

			m_clrBarGradientDark = CBCGDrawManager::PixelAlpha (
				m_clrBarGradientDark, 97);
		}

		m_clrPressedButtonBorder = CBCGDrawManager::SmartMixColors (
				m_clrMenuItemBorder, 
				globalData.clrBarDkShadow,
				.8, 1, 2);
	}

	m_brMenuButtonDroppedDown.DeleteObject ();
	m_brMenuButtonDroppedDown.CreateSolidBrush (clrMenuButtonDroppedDown);

	m_brMenuItemCheckedHighlight.DeleteObject ();
	m_brMenuItemCheckedHighlight.CreateSolidBrush (clrMenuItemCheckedHighlight);

	m_penActiveTabBorder.DeleteObject ();

	if (m_colorActiveTabBorder != (COLORREF)-1)
	{
		m_penActiveTabBorder.CreatePen (PS_SOLID, 1, m_colorActiveTabBorder);
	}

	m_bDefaultWinXPColors = bDefaultWinXPColors;

	m_clrInactiveTabText = globalData.clrBtnDkShadow;
}
//**************************************************************************************
void CBCGVisualManagerVS2005::OnDrawCaptionButton (CDC* pDC, CBCGSCBButton* pButton,
									BOOL bHorz, BOOL bMaximized, BOOL bDisabled)
{
	ASSERT_VALID (pDC);

	if (pButton->m_pParentBar->IsActive ())
	{
		CBCGVisualManager2003::OnDrawCaptionButton (pDC, pButton, bHorz, bMaximized, bDisabled);
		return;
	}

    CRect rc = pButton->GetRect ();

	const BOOL bHighlight = 
		(pButton->m_bPushed || pButton->m_bFocused) && !bDisabled;
	const BOOL bPressed = pButton->m_bPushed && pButton->m_bFocused;

	COLORREF clrForeground = pButton->m_clrForeground;
	if (clrForeground == (COLORREF)-1)
	{
		clrForeground = globalData.clrBarText;
	}

	if (bHighlight)
	{
		pDC->FillRect (rc, bPressed ? &globalData.brHilite : &globalData.brBarFace);
		clrForeground = bPressed ? globalData.clrTextHilite : globalData.clrBarText;
	}

	if (!bDisabled)
	{
		if (GetRValue (clrForeground) <= 192 &&
			GetGValue (clrForeground) <= 192 &&
			GetBValue (clrForeground) <= 192)
		{
			pDC->SetTextColor (RGB (0, 0, 0));
		}
		else
		{
			pDC->SetTextColor (RGB (255, 255, 255));
		}
	}

	pButton->DrawIcon (pDC, rc, bHorz, bMaximized, bDisabled);

	if (bHighlight)
	{
		pDC->Draw3dRect (rc, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
	}
}
//***********************************************************************************
void CBCGVisualManagerVS2005::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode ())
	{
		CBCGVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		if (pTabWnd->IsDialogControl ())
		{
			pDC->FillRect (rect, &globalData.brBtnFace);
		}
		else
		{
			pDC->FillRect (rect, &globalData.brBarFace);
		}
	}
	else
	{
		CControlBar* pParentBar = DYNAMIC_DOWNCAST (CControlBar,
			pTabWnd->GetParent ());

		if (pParentBar == NULL)
		{
			pDC->FillRect (rect, &globalData.brBtnFace);
		}
		else
		{
			CRect rectScreen = globalData.m_rectVirtual;
			pTabWnd->ScreenToClient (&rectScreen);

			CRect rectFill = rect;
			rectFill.left = min (rectFill.left, rectScreen.left);

			OnFillBarBackground (pDC, pParentBar, rectFill, rect);
		}
	}
}
//*************************************************************************************
void CBCGVisualManagerVS2005::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () ||
		pTabWnd->IsVS2005Style ())
	{
		CPen* pOldPen = NULL;

		if (bIsActive && pTabWnd->IsVS2005Style () &&
			m_penActiveTabBorder.GetSafeHandle () != NULL)
		{
			pOldPen = pDC->SelectObject (&m_penActiveTabBorder);
		}

		CBCGVisualManager2003::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);

		if (pOldPen != NULL)
		{
			pDC->SelectObject (pOldPen);
		}

		return;
	}

	COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);
	COLORREF clrTextOld = (COLORREF)-1;

	if (bIsActive && clrTab == (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (globalData.clrWindowText);
		((CBCGTabWnd*)pTabWnd)->SetTabBkColor (iTab, globalData.clrWindow);
	}

	CBCGVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);

	((CBCGTabWnd*)pTabWnd)->SetTabBkColor (iTab, clrTab);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//*********************************************************************************
void CBCGVisualManagerVS2005::GetTabFrameColors (const CBCGTabWnd* pTabWnd,
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
	
	CBCGVisualManager2003::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (pTabWnd->IsVS2005Style () && m_colorActiveTabBorder != (COLORREF)-1)
	{
		clrHighlight = m_colorActiveTabBorder;
	}

	clrBlack = clrDarkShadow;
}
//************************************************************************************
void CBCGVisualManagerVS2005::OnDrawSeparator (CDC* pDC, CControlBar* pBar, CRect rect, BOOL bHorz)
{
	CBCGToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGToolBar, pBar);
	if (pToolBar != NULL)
	{
		ASSERT_VALID (pToolBar);

		if (bHorz)
		{
			const int nDelta = max (0, (pToolBar->GetButtonSize ().cy - pToolBar->GetImageSize ().cy) / 2);
			rect.DeflateRect (0, nDelta);
		}
		else
		{
			const int nDelta = max (0, (pToolBar->GetButtonSize ().cx - pToolBar->GetImageSize ().cx) / 2);
			rect.DeflateRect (nDelta, 0);
		}
	}

	CBCGVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
}
//***********************************************************************************
void CBCGVisualManagerVS2005::OnFillHighlightedArea (CDC* pDC, CRect rect, 
							CBrush* pBrush, CBCGToolbarButton* pButton)
{
	if (pButton != NULL && 
		(m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive))
	{
		ASSERT_VALID (pButton);

		CBCGToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);

		BOOL bIsPopupMenu = pMenuButton != NULL &&
			pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar));

		if (bIsPopupMenu &&
			(pButton->m_nStyle & TBBS_CHECKED) &&
			pBrush == &m_brHighlightDn)
		{
			pDC->FillRect (rect, &m_brMenuItemCheckedHighlight);
			return;
		}

		if (pMenuButton != NULL && !bIsPopupMenu && pMenuButton->IsDroppedDown ())
		{
			pDC->FillRect (rect, &m_brMenuButtonDroppedDown);
			return;
		}
	}

	CBCGVisualManager2003::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
}
//***********************************************************************************
void CBCGVisualManagerVS2005::OnFillBarBackground (CDC* pDC, CControlBar* pBar,
								CRect rectClient, CRect rectClip,
								BOOL bNCArea)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGStatusBar)) &&
		globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode () && m_hThemeStatusBar != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(),
			0, 0, &rectClient, 0);
		return;
	}

	CBCGVisualManager2003::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}
//********************************************************************************
COLORREF CBCGVisualManagerVS2005::GetPropListGroupColor (CBCGPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (m_bDefaultWinXPColors)
	{
		return CBCGVisualManager2003::GetPropListGroupColor (pPropList);
	}

	return pPropList->DrawControlBarColors () ?
		globalData.clrBarLight : globalData.clrBtnLight;
}
//**************************************************************************************
COLORREF CBCGVisualManagerVS2005::OnDrawControlBarCaption (CDC* pDC, CBCGSizingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGVisualManagerXP::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	COLORREF clrFill;

	if (!bActive)
	{
		clrFill = CBCGDrawManager::PixelAlpha (m_clrBarGradientDark, 87);

		CBrush brFill (clrFill);
		pDC->FillRect (rectCaption, &brFill);

		pDC->Draw3dRect (rectCaption, globalData.clrBarShadow, globalData.clrBarShadow);
	}
	else
	{
		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive ||
			m_CurrAppTheme == WinXpTheme_Silver)
		{
			COLORREF clrLight = 
				CBCGDrawManager::PixelAlpha (globalData.clrHilite, 130);

			CBCGDrawManager dm (*pDC);

			if (pBar->IsVertDocked () || CBCGSizingControlBar::IsCaptionAlwaysOnTop ())
			{
				dm.FillGradient (rectCaption, globalData.clrHilite, clrLight, TRUE);
			}
			else
			{
				dm.FillGradient (rectCaption, clrLight, globalData.clrHilite, FALSE);
			}

			return globalData.clrTextHilite;
		}
		else
		{
			pDC->FillRect (rectCaption, &globalData.brActiveCaption);
			return globalData.clrCaptionText;
		}
	}

	if (GetRValue (clrFill) <= 192 &&
		GetGValue (clrFill) <= 192 &&
		GetBValue (clrFill) <= 192)
	{
		return RGB (255, 255, 255);
	}
	else
	{
		return RGB (0, 0, 0);
	}
}
