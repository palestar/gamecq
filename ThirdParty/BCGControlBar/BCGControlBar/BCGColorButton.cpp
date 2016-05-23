// BCGColorButton.cpp : implementation file
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//

#include "stdafx.h"

#ifndef BCG_NO_COLOR

#include "bcgcontrolbar.h"
#include "BCGColorButton.h"
#include "BCGColorBar.h"
#include "ColorPopup.h"
#include "MenuImages.h"
#include "BCGWinXPVisualManager.h"
#include "BCGToolbarComboBoxButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nImageHorzMargin = 8;

/////////////////////////////////////////////////////////////////////////////
// CBCGColorButton

IMPLEMENT_DYNAMIC(CBCGColorButton, CBCGButton)

CBCGColorButton::CBCGColorButton()
{
	m_Color = RGB (0, 0, 0);
	m_ColorAutomatic = (COLORREF)-1;
	m_nColumns = -1;
	m_pPopup = NULL;
	m_bAltColorDlg = TRUE;
	m_pPalette = NULL;
	m_bEnabledInCustomizeMode = FALSE;
	m_bAutoSetFocus = TRUE;
}

CBCGColorButton::~CBCGColorButton()
{
	if (m_pPalette != NULL)
	{
		delete m_pPalette;
	}
}


BEGIN_MESSAGE_MAP(CBCGColorButton, CBCGButton)
	//{{AFX_MSG_MAP(CBCGColorButton)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGColorButton message handlers

CSize CBCGColorButton::SizeToContent (BOOL bCalcOnly)
{
	CSize size = CBCGButton::SizeToContent (FALSE);
	size.cx += CMenuImages::Size ().cx;

	if (!bCalcOnly)
	{
		SetWindowPos (NULL, -1, -1, size.cx, size.cy,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return size;
}
//*****************************************************************************************
void CBCGColorButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	if (!IsDrawXPTheme ())
	{
		CBCGButton::OnFillBackground (pDC, rectClient);
		return;
	}

	ASSERT_VALID (pDC);
	::FillRect (pDC->GetSafeHdc (), &rectClient, ::GetSysColorBrush (COLOR_WINDOW));
}
//****************************************************************************************
void CBCGColorButton::OnDraw (CDC* pDC, const CRect& rect, UINT uiState)
{
	ASSERT_VALID (pDC);

	if (m_pPalette == NULL)
	{
		RebuildPalette (NULL);
	}

	CPalette* pCurPalette = pDC->SelectPalette (m_pPalette, FALSE);
	pDC->RealizePalette();

	CSize sizeArrow = CMenuImages::Size ();

	CRect rectColor = rect;
	rectColor.right -= sizeArrow.cx + nImageHorzMargin;

	CRect rectArrow = rect;
	rectArrow.left = rectColor.right;

	COLORREF color = m_Color;
	if (color == (COLORREF) -1)	// Automatic
	{
		//---------------------------
		// Draw automatic text label:
		//---------------------------
		color = m_ColorAutomatic;
		
		if (!m_strAutoColorText.IsEmpty ())
		{
			rectColor.right = rectColor.left + rectColor.Height ();

			CRect rectText = rect;
			rectText.left = rectColor.right;
			rectText.right = rectArrow.left;

			CFont* pOldFont = SelectFont (pDC);
			ASSERT(pOldFont != NULL);

			pDC->SetBkMode (TRANSPARENT);
			pDC->SetTextColor (globalData.clrBtnText);

			pDC->DrawText (m_strAutoColorText, rectText,
							DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			pDC->SelectObject (pOldFont);
		}
	}

	//----------------
	// Draw color box:
	//----------------
	rectColor.DeflateRect (2, 2);
	pDC->Draw3dRect (rectColor, globalData.clrBtnHilite, globalData.clrBtnHilite);
	rectColor.DeflateRect (1, 1);
	pDC->Draw3dRect (rectColor, globalData.clrBtnDkShadow, globalData.clrBtnDkShadow);
	rectColor.DeflateRect (1, 1);

	if (color != (COLORREF)-1 && (uiState & ODS_DISABLED) == 0)
	{
		if (globalData.m_nBitsPerPixel == 8) // 256 colors
		{
			ASSERT_VALID (m_pPalette);
			color =  PALETTEINDEX (m_pPalette->GetNearestPaletteIndex (color));
		}

		CBrush br (color);
		pDC->FillRect (rectColor, &br);
	}

	//----------------------
	// Draw drop-down arrow:
	//----------------------
	CRect rectArrowWinXP = rectArrow;
	rectArrowWinXP.DeflateRect (2, 2);

	if (!m_bWinXPTheme || !CBCGVisualManager::GetInstance ()->DrawComboDropButtonWinXP (
									pDC, rectArrowWinXP,
									(uiState & ODS_DISABLED), m_bPushed,
									m_bHighlighted))
	{
		pDC->FillRect (rectArrow, &globalData.brBarFace);

		int iImage = (uiState & ODS_DISABLED) ? CMenuImages::IdArowDownLargeDsbl : CMenuImages::IdArowDownLarge;
		CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rectArrow);

		pDC->Draw3dRect (rectArrow, globalData.clrBtnLight, globalData.clrBtnDkShadow);
		rectArrow.DeflateRect (1, 1);
		pDC->Draw3dRect (rectArrow, globalData.clrBtnHilite, globalData.clrBtnShadow);
	}

	if (pCurPalette != NULL)
	{
		pDC->SelectPalette (pCurPalette, FALSE);
	}
}
//*****************************************************************************************
void CBCGColorButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);
	ASSERT (m_nFlatStyle != BUTTONSTYLE_NOBORDERS);	// Always has borders

	if (!m_bWinXPTheme || !CBCGVisualManager::GetInstance ()->DrawComboBorderWinXP (
			pDC, rectClient, !IsWindowEnabled (), FALSE, TRUE))
	{
		pDC->Draw3dRect (rectClient,
					globalData.clrBtnDkShadow, globalData.clrBtnHilite);

		rectClient.DeflateRect (1, 1);

		if (m_nFlatStyle == BUTTONSTYLE_3D || m_bHighlighted)
		{
			pDC->Draw3dRect (rectClient,
						globalData.clrBtnShadow, globalData.clrBtnLight);
		}
	}
	
	rectClient.DeflateRect (1, 1);
}
//*****************************************************************************************
void CBCGColorButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	CSize sizeArrow = CMenuImages::Size ();

	CRect rectColor = rectClient;
	rectColor.right -= sizeArrow.cx + nImageHorzMargin;

	CBCGButton::OnDrawFocusRect (pDC, rectColor);
}
//*****************************************************************************************
void CBCGColorButton::OnShowColorPopup () 
{
	if (m_pPopup != NULL)
	{
		m_pPopup->SendMessage (WM_CLOSE);
		m_pPopup = NULL;
		return;
	}

	if (m_Colors.GetSize () == 0)
	{
		// Use default pallete:
		CBCGColorBar::InitColors (NULL, m_Colors);
	}

	m_pPopup = new CColorPopup (this, m_Colors, m_Color,
		m_strAutoColorText, m_strOtherText, m_strDocColorsText, m_lstDocColors,
		m_nColumns, m_ColorAutomatic);
	m_pPopup->m_bEnabledInCustomizeMode = m_bEnabledInCustomizeMode;

	CRect rectWindow;
	GetWindowRect (rectWindow);

	if (!m_pPopup->Create (this, rectWindow.left, rectWindow.bottom, NULL,
		m_bEnabledInCustomizeMode))
	{
		ASSERT (FALSE);
		m_pPopup = NULL;

		TRACE(_T("Color menu can't be used in the customization mode. You need to set CBCGColorButton::m_bEnabledInCustomizeMode\n"));
	}
	else
	{
		if (m_bEnabledInCustomizeMode)
		{
			CBCGColorBar* pColorBar = DYNAMIC_DOWNCAST (
				CBCGColorBar, m_pPopup->GetMenuBar());

			if (pColorBar != NULL)
			{
				ASSERT_VALID (pColorBar);
				pColorBar->m_bInternal = TRUE;
			}
		}

		if (m_bAutoSetFocus)
		{
			m_pPopup->GetMenuBar()->SetFocus ();
		}
	}

	if (m_bCaptured)
	{
		ReleaseCapture ();
		m_bCaptured = FALSE;
	}
}
//*****************************************************************************************
void CBCGColorButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_SPACE || nChar == VK_DOWN)
	{
		OnShowColorPopup ();
		return;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGColorButton::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/) 
{
	SetFocus ();
	OnShowColorPopup ();
}
//****************************************************************************************
void CBCGColorButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	FlatStyle nFlatStyle = m_nFlatStyle;
	if (IsDrawXPTheme ())
	{
		m_nFlatStyle = BUTTONSTYLE_SEMIFLAT;
	}

	CBCGButton::OnMouseMove(nFlags, point);
	m_nFlatStyle = nFlatStyle;
}
//*****************************************************************************************
UINT CBCGColorButton::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}
//*****************************************************************************************
void CBCGColorButton::EnableAutomaticButton (LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	m_strAutoColorText = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_ColorAutomatic = colorAutomatic;
}
//*****************************************************************************************
void CBCGColorButton::EnableOtherButton (LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	m_strOtherText = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_bAltColorDlg = bAltColorDlg;
}
//*****************************************************************************************
void CBCGColorButton::SetDocumentColors (LPCTSTR lpszLabel, CList<COLORREF,COLORREF>& lstColors)
{
	m_lstDocColors.RemoveAll ();
	m_strDocColorsText = (lpszLabel == NULL) ? _T("") : lpszLabel;

	if (!m_strDocColorsText.IsEmpty ())
	{
		m_lstDocColors.AddTail (&lstColors);
	}
}
//*****************************************************************************************
void CBCGColorButton::SetPalette (CPalette* pPalette)
{
	if (m_Colors.GetSize () != 0)
	{
		m_Colors.SetSize (0);
		m_Colors.FreeExtra ();
	}

	CBCGColorBar::InitColors (pPalette, m_Colors);
	RebuildPalette (pPalette);
}
//*****************************************************************************************
void CBCGColorButton::SetColor (COLORREF color /* -1 - automatic*/)
{
	m_Color = color;

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*****************************************************************************************
void CBCGColorButton::UpdateColor (COLORREF color)
{
	SetColor (color);

		//-------------------------------------------------------
	// Trigger mouse up event (to button click notification):
	//-------------------------------------------------------
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{
		pParent->SendMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}
}
//****************************************************************************************
void CBCGColorButton::RebuildPalette (CPalette* pPal)
{
	if (m_pPalette != NULL)
	{
		delete m_pPalette;
	}

	m_pPalette = new CPalette ();

	// Create palette:
	CClientDC dc (this);

	if (pPal == NULL)
	{
		int nColors = 256;	// Use 256 first entries
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

		::GetSystemPaletteEntries (dc.GetSafeHdc (), 0, nColors, pLP->palPalEntry);

		pLP->palVersion = 0x300;
		pLP->palNumEntries = (USHORT) nColors;

		m_pPalette->CreatePalette (pLP);

		delete[] pLP;
	}
	else
	{
		ASSERT_VALID (pPal);
		int nColors = pPal->GetEntryCount ();
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

		pPal->GetPaletteEntries (0, nColors, pLP->palPalEntry);

		pLP->palVersion = 0x300;
		pLP->palNumEntries = (USHORT) nColors;

		m_pPalette->CreatePalette (pLP);

		delete[] pLP;
	}
}
//*****************************************************************************************
void CBCGColorButton::OnSysColorChange() 
{
	CBCGButton::OnSysColorChange();
	RebuildPalette (NULL);

	Invalidate ();
	UpdateWindow ();
}
//*****************************************************************************************
BOOL CBCGColorButton::IsDrawXPTheme () const
{
	return m_bWinXPTheme &&
		CBCGVisualManager::GetInstance ()->IsWinXPThemeSupported ();
}

#endif // BCG_NO_COLOR

