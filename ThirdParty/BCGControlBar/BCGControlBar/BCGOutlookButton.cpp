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

// BCGOutlookButton.cpp: implementation of the CBCGOutlookButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef BCG_NO_OUTLOOKBAR

#include "bcgcontrolbar.h"
#include "BCGOutlookButton.h"
#include "BCGOutlookBar.h"
#include "MenuImages.h"
#include "BCGWorkspace.h"
#include "BCGVisualManager.h"
#include "BCGDrawManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGOutlookButton, CBCGToolbarButton, 1)

#define BUTTON_OFFSET			10
#define HIGHLIGHT_PERCENTAGE	85

extern CBCGWorkspace* g_pWorkspace;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGOutlookButton::CBCGOutlookButton()
{
	m_pWndParentBar = NULL;
	m_uiPageID = 0;
	m_sizeImage = CSize (0, 0);
	m_bIsWholeText = TRUE;
}
//***************************************************************************************
CBCGOutlookButton::~CBCGOutlookButton()
{
}
//***************************************************************************************
void CBCGOutlookButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz, BOOL bCustomizeMode,
						BOOL bHighlight,
						BOOL /*bDrawBorder*/,
						BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndParentBar);

	CSize csOffset (0, 0);
	if (!bCustomizeMode &&
		(bHighlight && (m_nStyle & TBBS_PRESSED)))
	{
		csOffset = CSize (1, 1);
	}

	CRect rectInternal = rect;
	CRect rectText = rect;

	if (m_bExtraSize)
	{
		CSize sizeExtra = CBCGVisualManager::GetInstance ()->GetButtonExtraBorder ();
		if (sizeExtra != CSize (0, 0))
		{
			rectInternal.DeflateRect (sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);

			if (!bHorz)
			{
				rectText.OffsetRect (0, sizeExtra.cy);
			}
			else
			{
				rectText.OffsetRect (sizeExtra.cx, 0);
			}
		}
	}

	CRect rectBorder = rectInternal;
	rectText.top += BUTTON_OFFSET / 2;

	if (pImages != NULL && GetImage () >= 0)
	{
		int x, y;

		CSize csImage = pImages->GetImageSize ();

		if (!bHorz)
		{
			int iImageHorzOffset = (rectInternal.Width () - csImage.cx) / 2;
			x = rectInternal.left + iImageHorzOffset;
			y = rectInternal.top + BUTTON_OFFSET / 2;

			rectText.top += csImage.cy + 2;
		}
		else
		{
			int iImageVertOffset = (rectInternal.Height () - csImage.cy) / 2;
			x = rectInternal.left + BUTTON_OFFSET / 2;
			y = rectInternal.top + iImageVertOffset;

			rectText.left += csImage.cx + BUTTON_OFFSET;

			CRect rectInternal = rectText;
			int iTextHeight = pDC->DrawText (m_strText, rectInternal, 
				DT_CALCRECT | DT_WORDBREAK);

			rectText.top = rectInternal.top + (rectInternal.Height () - iTextHeight) / 2;
		}

		rectBorder = CRect (CPoint (x, y), csImage);
		rectBorder.InflateRect (2, 2);

		//----------------------
		// Fill button interior:
		//----------------------
		if (m_pWndParentBar->IsDrawShadedHighlight ())
		{
			if (bHighlight && !bCustomizeMode)
			{
				CBCGDrawManager dm (*pDC);
				dm.HighlightRect (rectBorder, HIGHLIGHT_PERCENTAGE);
			}
		}
		else
		{
			if (m_bExtraSize)
			{
				CSize sizeExtra = CBCGVisualManager::GetInstance ()->GetButtonExtraBorder ();
				if (sizeExtra != CSize (0, 0))
				{
					rectBorder.InflateRect (sizeExtra.cx / 2 - 1, sizeExtra.cy / 2 - 1);
				}
			}

			FillInterior (pDC, rectBorder, bHighlight);
		}

		pImages->Draw (pDC, 
			x + csOffset.cx, y + csOffset.cy, GetImage (), FALSE, 
			(m_nStyle & TBBS_DISABLED));
	}
	else
	{
		if (bHighlight &&
			m_pWndParentBar->IsDrawShadedHighlight () && !bCustomizeMode)
		{
			CBCGDrawManager dm (*pDC);
			dm.HighlightRect (rectBorder, HIGHLIGHT_PERCENTAGE);
		}
	}

	if (!bCustomizeMode &&
		(bHighlight || (m_nStyle & TBBS_PRESSED) || (m_nStyle & TBBS_CHECKED)))
	{
		if (((m_nStyle & TBBS_PRESSED) && bHighlight) ||
			(m_nStyle & TBBS_CHECKED))
		{
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGVisualManager::ButtonsIsPressed);
		}
		else
		{							  
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rectBorder, CBCGVisualManager::ButtonsIsHighlighted);
		}
	}

	if (m_bTextBelow && !m_strText.IsEmpty ())
	{
		COLORREF clrText = (COLORREF)-1;
		CBCGVisualManager::BCGBUTTON_STATE state = CBCGVisualManager::ButtonsIsRegular;

		if (bHighlight)
		{
			state = CBCGVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			state = CBCGVisualManager::ButtonsIsPressed;
		}

		if (m_nStyle & TBBS_DISABLED)
		{
			if (m_pWndParentBar->IsBackgroundTexture ())
			{
				clrText = ::GetSysColor (COLOR_GRAYTEXT);
			}
		}
		else
		{
			clrText = m_pWndParentBar->GetRegularColor ();
		}

		if (clrText == (COLORREF)-1)
		{
			if (m_pWndParentBar->IsBackgroundTexture ())
			{
				clrText = ::GetSysColor (COLOR_WINDOWTEXT);
			}
			else
			{
				clrText = CBCGVisualManager::GetInstance ()->GetToolbarButtonTextColor (
														this, state);
			}
		}

		pDC->SetTextColor (clrText);

		if (m_bIsWholeText)
		{
			pDC->DrawText (m_strText, rectText, 
						DT_WORDBREAK | DT_CENTER);
		}
		else
		{
			CString strText = m_strText;
			pDC->DrawText (strText, rectText, DT_WORDBREAK | DT_END_ELLIPSIS);
		}
	}
}
//****************************************************************************************
SIZE CBCGOutlookButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CSize sizeResult = sizeDefault;

	if (!bHorz)
	{
		const int nCXMargin = pDC->GetTextExtent (_T("   ")).cx;
		CRect rectText (0, 0, sizeDefault.cx - nCXMargin, 1);

		const int nImageOffset = 2;

		int iTextHeight = m_bTextBelow ? pDC->DrawText (m_strText, rectText, 
			DT_CALCRECT | DT_WORDBREAK) : 0;

		sizeResult.cy = sizeDefault.cy + iTextHeight + BUTTON_OFFSET;
		sizeResult.cx = max (	m_sizeImage.cx + 2 * nImageOffset, 
								min (sizeDefault.cx, rectText.Width ()));

		m_bIsWholeText = rectText.Width () <= sizeDefault.cx;
	}
	else
	{
		CRect rectText (0, 0, 0, sizeDefault.cy);
		int iTextHeight = 0;

		if (m_bTextBelow)
		{
			do
			{
				rectText.right ++;
				iTextHeight = pDC->DrawText (m_strText, rectText, 
								DT_CALCRECT | DT_WORDBREAK);
			}
			while (iTextHeight < pDC->GetTextExtent (m_strText).cy &&
					rectText.Height () > sizeDefault.cy);
		}

		sizeResult.cx = sizeDefault.cx + rectText.Width () + BUTTON_OFFSET;
		sizeResult.cy = max (m_sizeImage.cy, min (sizeDefault.cy, rectText.Height ()));

		m_bIsWholeText = TRUE;
	}

	return sizeResult;
}
//***************************************************************************************
void CBCGOutlookButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGToolbarButton::OnChangeParentWnd (pWndParent);

	m_pWndParentBar = DYNAMIC_DOWNCAST (CBCGOutlookBar, pWndParent);
	ASSERT_VALID (m_pWndParentBar);
}
//***************************************************************************************
BOOL CBCGOutlookButton::CanBeDropped (CBCGToolBar* pToolbar)
{
	ASSERT_VALID (pToolbar);
	return pToolbar->IsKindOf (RUNTIME_CLASS (CBCGOutlookBar));
}
//***************************************************************************************
void CBCGOutlookButton::SetImage (int iImage)
{
	// Don't add image to hash!
	m_iImage = iImage;
}
//***************************************************************************************
void CBCGOutlookButton::Serialize (CArchive& ar)
{
	CBCGToolbarButton::Serialize (ar);

	if (ar.IsLoading ())
	{
		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x40710)
		{
			ar >> m_uiPageID;
		}
		else
		{
			m_uiPageID = 0;
		}
	}
	else
	{
		ar << m_uiPageID;
	}
}
//************************************************************************************
void CBCGOutlookButton::CopyFrom (const CBCGToolbarButton& src)
{
	CBCGToolbarButton::CopyFrom (src);

	const CBCGOutlookButton& srcOLButton = (const CBCGOutlookButton&) src;
	m_uiPageID = srcOLButton.m_uiPageID;
}
//************************************************************************************
BOOL CBCGOutlookButton::CompareWith (const CBCGToolbarButton& other) const
{
	if (!CBCGToolbarButton::CompareWith (other))
	{
		return FALSE;
	}

	const CBCGOutlookButton& otherOLButton = (const CBCGOutlookButton&) other;
	return m_uiPageID == otherOLButton.m_uiPageID;
}

#endif // BCG_NO_OUTLOOKBAR

