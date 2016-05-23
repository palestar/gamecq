// BCGCaptionBar.cpp: implementation of the CBCGCaptionBar class.
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGCaptionBar.h"
#include "BCGVisualManager.h"
#include "BCGToolBar.h"
#include "trackmouse.h"
#include "Globals.h"

const int nMenuArrowWidth = 10;

IMPLEMENT_DYNCREATE(CBCGCaptionBar, CControlBar)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGCaptionBar::CBCGCaptionBar() 
{
	m_clrBarText			= (COLORREF)-1;
	m_clrBarBackground		= (COLORREF)-1;
	m_clrBarBorder			= (COLORREF)-1;

	m_clrTransparent		= (COLORREF)-1;

	m_nBorderSize			= 4;
	m_nMargin				= 4;
	m_nHorzElementOffset	= 4;

	m_hIcon					= NULL;
	m_hFont					= NULL;

	m_nDefaultHeight		= -1;
	m_nCurrentHeight		= 0;

	m_btnAlignnment			= ALIGN_LEFT;
	m_iconAlignment			= ALIGN_LEFT;
	m_textAlignment			= ALIGN_LEFT;

	m_bStretchImage			= FALSE;

	m_bFlatBorder			= FALSE;
	m_uiBtnID				= 0;

	m_bIsBtnPressed			= FALSE;
	m_bIsBtnForcePressed	= FALSE;
	m_bIsBtnHighlighted		= FALSE;

	m_bTracked				= FALSE;
	m_bBtnEnabled			= TRUE;

	m_rectImage.SetRectEmpty ();
	m_rectText.SetRectEmpty ();
	m_rectDrawText.SetRectEmpty ();
	m_rectButton.SetRectEmpty ();
}
//*****************************************************************************
CBCGCaptionBar::~CBCGCaptionBar()
{
}
//*****************************************************************************
BOOL CBCGCaptionBar::Create (DWORD dwStyle, CWnd* pParentWnd, UINT uID, int nHeight)
{
	SetBarStyle (CBRS_ALIGN_TOP);
	m_nDefaultHeight = nHeight;

	return CControlBar::Create (NULL, NULL, dwStyle, CRect (0, 0, 0, 0), pParentWnd, uID);
}

BEGIN_MESSAGE_MAP(CBCGCaptionBar, CControlBar)
	//{{AFX_MSG_MAP(CBCGCaptionBar)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

void CBCGCaptionBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}
//*****************************************************************************
int CBCGCaptionBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CControlBar::OnCreate (lpCreateStruct) == -1)
		return -1;

	SetWindowText (_T("Caption Bar"));
	return 0;
}
//*****************************************************************************
void CBCGCaptionBar::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);
	RecalcLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	lpncsp->rgrc[0].bottom -= m_nBorderSize;
	lpncsp->rgrc[0].top  += m_nBorderSize;
}
//*****************************************************************************
void CBCGCaptionBar::OnPaint() 
{
	CPaintDC dc(this);

	int nOldBkMode = dc.SetBkMode (TRANSPARENT);
	COLORREF clrOldText = 
		dc.SetTextColor (m_clrBarText != (COLORREF) -1 ? m_clrBarText : 
		CBCGVisualManager::GetInstance ()->GetCaptionBarTextColor (this));

	CFont* pOldFont = dc.SelectObject (
		m_hFont == NULL ? &globalData.fontRegular : CFont::FromHandle (m_hFont));

	OnDrawButton (&dc, m_rectButton, m_strBtnText, m_bBtnEnabled);
	OnDrawText (&dc, m_rectDrawText, m_strText);
	OnDrawImage (&dc, m_rectImage);

	dc.SelectObject (pOldFont);
	dc.SetTextColor (clrOldText);
	dc.SetBkMode (nOldBkMode);
}
//*****************************************************************************
void CBCGCaptionBar::OnNcPaint() 
{
	CWindowDC	dcWin (this);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectWindow;
	GetWindowRect(rectWindow);

	CRect rectBorder = rectWindow;

	ScreenToClient(rectWindow);

	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dcWin.ExcludeClipRect (rectClient);
	
	rectBorder.OffsetRect(-rectBorder.left, -rectBorder.top);

	int nTop = rectBorder.top;
	rectBorder.top = rectBorder.bottom - m_nBorderSize;
	OnDrawBorder  (&dcWin, rectBorder);

	rectBorder.top = nTop;
	rectBorder.bottom = rectBorder.top + m_nBorderSize;

	OnDrawBorder  (&dcWin, rectBorder);
	dcWin.SelectClipRgn (NULL);
}
//*****************************************************************************
void CBCGCaptionBar::OnDrawBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	CBCGVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
		rect, rect);
}
//*****************************************************************************
void CBCGCaptionBar::OnDrawBorder (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	rect.InflateRect (2, 0);

	if (m_clrBarBorder == (COLORREF) -1)
	{
		pDC->FillRect (rect, &globalData.brBarFace);
	}
	else
	{
		CBrush brBorder;
		brBorder.CreateSolidBrush (m_clrBarBorder);
		pDC->FillRect (rect, &brBorder);
	}

	if (!m_bFlatBorder)
	{
		pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarShadow);
	}
}
//*****************************************************************************
void CBCGCaptionBar::OnDrawButton (CDC* pDC, CRect rect, 
								   const CString& strButton, BOOL bEnabled)
{
	ASSERT_VALID (pDC);

	CRect rectText = rect;
	rectText.DeflateRect (m_nHorzElementOffset, 0);

	if (m_uiBtnID != 0 && bEnabled)
	{
		rectText.right -= nMenuArrowWidth;
	}

	pDC->DrawText (strButton, rectText,
					DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);

	if (m_uiBtnID != 0 && bEnabled)
	{
		if (m_uiBtnID != 0)
		{
			// Draw menu triangle:
			CRect rectArrow = rect;
			rectArrow.bottom -= m_nMargin;
			rectArrow.top = rectArrow.bottom - nMenuArrowWidth;
			rectArrow.left = rectText.right;

			int iXMiddle = rectArrow.left + rectArrow.Width () / 2;

			rectArrow.DeflateRect (0, rectArrow.Height () / 3);
			rectArrow.DeflateRect (rectArrow.Height () / 3, rectArrow.Height () / 3);
			rectArrow.left = iXMiddle - rectArrow.Height () - 1;
			rectArrow.right = iXMiddle + rectArrow.Height () + 1;

			int iHalfWidth =	(rectArrow.Width () % 2 != 0) ?
								(rectArrow.Width () - 1) / 2 :
								rectArrow.Width () / 2;

			CPoint pts [3];
			pts[0].x = rectArrow.left;
			pts[0].y = rectArrow.top;
			pts[1].x = rectArrow.right;
			pts[1].y = rectArrow.top;
			pts[2].x = rectArrow.left + iHalfWidth;
			pts[2].y = rectArrow.bottom + 1;

			CBrush brArrow (pDC->GetTextColor ());

			CPen* pOldPen = (CPen*) pDC->SelectStockObject (NULL_PEN);
			CBrush* pOldBrush = (CBrush*) pDC->SelectObject(&brArrow);

			pDC->SetPolyFillMode (WINDING);
			pDC->Polygon (pts, 3);

			pDC->SelectObject (pOldBrush);
			pDC->SelectObject (pOldPen);
		}

		if (m_bIsBtnPressed || m_bIsBtnForcePressed)
		{
			pDC->Draw3dRect (rect, globalData.clrBarDkShadow, globalData.clrBarHilite);
		}
		else if (m_bIsBtnHighlighted)
		{
			pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarDkShadow);
		}
	}
}	
//*****************************************************************************
void CBCGCaptionBar::OnDrawText	(CDC* pDC, CRect rect, const CString& strText)
{
	ASSERT_VALID (pDC);

	pDC->DrawText (strText, rect, 
					DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
}	
//*****************************************************************************
void CBCGCaptionBar::OnDrawImage (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (m_hIcon != NULL)
	{
		DrawIconEx (pDC->GetSafeHdc(), rect.left, rect.top, m_hIcon, 
					rect.Width (), rect.Height (), NULL, (HBRUSH)NULL, 
					DI_NORMAL) ;
	}
	else if (m_Bitmap.GetCount () > 0)
	{
		CSize sizeDest;
		if (m_bStretchImage)
		{
			sizeDest = rect.Size ();
		}
		else
		{
			sizeDest = m_rectImage.Size ();
		}

		CBCGDrawState ds;
		m_Bitmap.PrepareDrawImage (ds, sizeDest);
		m_Bitmap.Draw (pDC, rect.left, rect.top, 0);
		m_Bitmap.EndDrawImage (ds);
	}
}
//*****************************************************************************
void CBCGCaptionBar::OnSysColorChange() 
{
}
//*****************************************************************************
CSize CBCGCaptionBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	RecalcLayout ();
	return CSize (32767, m_nCurrentHeight);
}
//*****************************************************************************
void CBCGCaptionBar::SetButton (LPCTSTR lpszLabel, UINT uiCmdUI, BarElementAlignment btnAlignmnet)
{
	ASSERT (lpszLabel != NULL);

	m_strBtnText = lpszLabel;
	m_uiBtnID = uiCmdUI;
	m_btnAlignnment = btnAlignmnet;

	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::EnableButton (BOOL bEnable)
{
	m_bBtnEnabled = bEnable;

	if (GetSafeHwnd () != NULL)
	{
		CRect rectButton = m_rectButton;

		RecalcLayout ();

		InvalidateRect (rectButton);
		InvalidateRect (m_rectButton);

		UpdateWindow ();
	}
}
//*****************************************************************************
void CBCGCaptionBar::SetButtonPressed (BOOL bPresed)
{
	m_bIsBtnForcePressed = bPresed;

	if (GetSafeHwnd () != NULL)
	{
		InvalidateRect (m_rectButton);
		UpdateWindow ();
	}
}
//*****************************************************************************
void CBCGCaptionBar::RemoveButton ()
{
	m_strBtnText.Empty ();
	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::SetIcon (HICON hIcon, BarElementAlignment iconAlignment)
{
	m_Bitmap.Clear ();

	m_hIcon = hIcon;
	m_iconAlignment = iconAlignment;

	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::RemoveIcon ()
{
	m_hIcon = NULL;
	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::SetBitmap (HBITMAP hBitmap, COLORREF clrTransparent, 
								BOOL bStretch, BarElementAlignment bmpAlignment)
{
	ASSERT (hBitmap != NULL);

	m_hIcon = NULL;
	m_Bitmap.Clear ();

	BITMAP bmp;
	::GetObject (hBitmap, sizeof (BITMAP), (LPVOID) &bmp);

	m_Bitmap.SetImageSize (CSize (bmp.bmWidth, bmp.bmHeight));
	m_clrTransparent = clrTransparent;
	m_Bitmap.SetTransparentColor (m_clrTransparent);
	m_Bitmap.AddImage (hBitmap);
	
	m_bStretchImage = bStretch;

	m_iconAlignment = bmpAlignment;

	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::RemoveBitmap ()
{
	m_Bitmap.Clear ();

	AdjustLayout ();
}
//*****************************************************************************
void CBCGCaptionBar::SetText (const CString& strText, BarElementAlignment textAlignment)
{
	BOOL bWasEmptyText = m_strText.IsEmpty ();

	m_strText = strText;
	m_textAlignment = textAlignment;

	if (m_nCurrentHeight == 0 || m_strText.IsEmpty () || bWasEmptyText)
	{
		AdjustLayout ();
	}
	else
	{
		RecalcLayout ();
		RedrawWindow ();
	}
}
//*****************************************************************************
void CBCGCaptionBar::RemoveText ()
{
	m_strText.Empty ();

	AdjustLayout ();
}
//*****************************************************************************
afx_msg LRESULT CBCGCaptionBar::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	AdjustLayout ();
	return 0;
}
//*****************************************************************************
afx_msg LRESULT CBCGCaptionBar::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) m_hFont;
}
//*****************************************************************************
CBCGCaptionBar::BarElementAlignment CBCGCaptionBar::GetAlignment (BarElement elem)
{
	switch (elem)
	{
	case ELEM_BUTTON:
		return m_btnAlignnment;

	case ELEM_TEXT:
		return m_textAlignment;

	case ELEM_ICON:
		return m_iconAlignment;
	}

	ASSERT (FALSE);
	return ALIGN_INVALID;
}
//*****************************************************************************
void CBCGCaptionBar::RecalcLayout ()
{
	CClientDC dc (NULL);

	CFont* pOldFont = dc.SelectObject (
		m_hFont == NULL ? &globalData.fontRegular : CFont::FromHandle (m_hFont));
	ASSERT (pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	int nTextHeight = tm.tmHeight + 2;
	CSize sizeImage = GetImageSize ();

	//-------------------------------------------------------------------
	// the height is set to the default (provided by the user in Create)
	// or calculated if it is -1
	//-------------------------------------------------------------------
	if (m_nDefaultHeight != -1)
	{
		m_nCurrentHeight = m_nDefaultHeight;
	}
	else
	{
		m_nCurrentHeight = max (nTextHeight, sizeImage.cy) + 
			m_nMargin * 2 + m_nBorderSize;
	}

	// for left and center alignment: icon, button, text
	// for right alignment: text, button, icon

	CRect rectClient;
	GetClientRect (rectClient);
	if (rectClient.IsRectEmpty ())
	{
		return;
	}

	BOOL bButtonLeftOfIcon = FALSE;
	BOOL bTextLeftOfButton = FALSE;
	BOOL bTextLeftOfIcon = FALSE;

	BOOL bIconCenter = FALSE;
	BOOL bButtonCenter = FALSE;
	BOOL bTextCenter = FALSE;

	// differs from the current height, because the border size is non-client area
	int nBaseLine = rectClient.Height () / 2;
	int nCenterOffset = rectClient.Width () / 2;

	int nNextXOffsetLeft  = rectClient.left + m_nMargin;
	int nNextXOffsetRight = rectClient.right - m_nMargin;
	int nNextXOffsetCenter = nCenterOffset;

	if (IsImageSet ())
	{
		if (sizeImage.cy < rectClient.Height ())
		{
			// center the icon if its height lesser than client area height
			m_rectImage.top = nBaseLine - sizeImage.cy / 2;
		}
		else
		{
			// otherwise, clip it from the buttom
			m_rectImage.top = rectClient.top + m_nMargin;
		}

		if (!m_bStretchImage)
		{
			m_rectImage.bottom = m_rectImage.top + sizeImage.cy;
		}
		else
		{
			m_rectImage.bottom = rectClient.bottom - m_nMargin;
		}

		switch (m_iconAlignment)
		{
		case ALIGN_LEFT:
			m_rectImage.left = nNextXOffsetLeft;
			m_rectImage.right = m_rectImage.left + sizeImage.cx;
			nNextXOffsetLeft = m_rectImage.right + m_nHorzElementOffset;
			break;

		case ALIGN_RIGHT:
			m_rectImage.left = nNextXOffsetRight - sizeImage.cx;
			m_rectImage.right = m_rectImage.left + sizeImage.cx;
			nNextXOffsetRight = m_rectImage.left - m_nHorzElementOffset;
			// only in this case button and text is at the left side of the icon
			bButtonLeftOfIcon = TRUE; 
			bTextLeftOfIcon = TRUE;
			break;

		case ALIGN_CENTER:
			bIconCenter = TRUE;
			nNextXOffsetCenter -= sizeImage.cx / 2;

			if (m_btnAlignnment == ALIGN_LEFT)
			{
				bButtonLeftOfIcon = TRUE;
			}

			if (m_textAlignment == ALIGN_LEFT)
			{
				bTextLeftOfIcon = TRUE;
			}
			break;

		default:
			ASSERT (FALSE);
		}
	}

	int nButtonWidth = 0;

	if (!m_strBtnText.IsEmpty ())
	{
		nButtonWidth = dc.GetTextExtent (m_strBtnText).cx + 2 * m_nHorzElementOffset;
		if (m_uiBtnID != 0 && m_bBtnEnabled)
		{
			nButtonWidth += nMenuArrowWidth;
		}

		// the button always has a height equivalent to the bar's height
		m_rectButton.top = rectClient.top;
		m_rectButton.bottom = rectClient.bottom;

		switch (m_btnAlignnment)
		{
		case ALIGN_LEFT:
			m_rectButton.left = nNextXOffsetLeft;

			if (nNextXOffsetLeft == rectClient.left + m_nMargin)
			{
				m_rectButton.left = rectClient.left;
			}

			m_rectButton.right = m_rectButton.left + nButtonWidth;
			nNextXOffsetLeft = m_rectButton.right + m_nHorzElementOffset;
			break;

		case ALIGN_RIGHT:
			m_rectButton.left = nNextXOffsetRight - nButtonWidth;

			if (nNextXOffsetRight == rectClient.right - m_nMargin)
			{
				m_rectButton.left = rectClient.right - nButtonWidth;
			}

			m_rectButton.right = m_rectButton.left + nButtonWidth;
			nNextXOffsetRight = m_rectButton.left - m_nHorzElementOffset;
			// only in this case text at the left side of the button
			bTextLeftOfButton = TRUE;
			break;

		case ALIGN_CENTER:
			bButtonCenter = TRUE;
			nNextXOffsetCenter -= nButtonWidth / 2;

			if (m_textAlignment == ALIGN_LEFT)
			{
				bTextLeftOfButton = TRUE;
			}
			break;

		default:
			ASSERT (FALSE);
			return;
		}
	}

	CSize sizeText (0, 0);
	if (!m_strText.IsEmpty ())
	{
		CRect rectText = rectClient;
		
		sizeText.cy = dc.DrawText (m_strText, rectText, DT_CALCRECT | DT_VCENTER | DT_NOPREFIX);
		sizeText.cx = rectText.Width ();

		m_rectText.top = nBaseLine - sizeText.cy / 2;
		m_rectText.bottom = m_rectText.top + sizeText.cy;

		switch (m_textAlignment)
		{
			case ALIGN_LEFT:
				m_rectText.left = nNextXOffsetLeft;
				break;

			case ALIGN_RIGHT:
				m_rectText.left = nNextXOffsetRight - sizeText.cx;
				break;

			case ALIGN_CENTER:
				bTextCenter = TRUE;
				nNextXOffsetCenter -= sizeText.cx / 2;
				break;

			default:
				ASSERT (FALSE);
				return;
		}

		m_rectText.right = m_rectText.left + sizeText.cx;
		AdjustRectToMargin (m_rectText, rectClient, m_nMargin);
		m_rectDrawText = m_rectText;
	}

	if (bIconCenter)
	{
		m_rectImage.left = nNextXOffsetCenter;
		m_rectImage.right = m_rectImage.left + sizeImage.cx;
		nNextXOffsetCenter = m_rectImage.right + m_nHorzElementOffset;
	}

	if (bButtonCenter)
	{
		m_rectButton.left = nNextXOffsetCenter;
		m_rectButton.right = m_rectButton.left + nButtonWidth;
		nNextXOffsetCenter = m_rectButton.right + m_nHorzElementOffset;
	}

	if (bTextCenter)
	{
		m_rectText.left = nNextXOffsetCenter;
		m_rectText.right = m_rectText.left + sizeText.cx; 
		AdjustRectToMargin (m_rectText, rectClient, m_nMargin);
		m_rectDrawText = m_rectText;
	}

	if (IsImageSet ())
	{
		// do not retain image size if it should be stretched
		AdjustRectToMargin (m_rectImage, rectClient, m_nMargin, !m_bStretchImage);
	}

	CRect rectButtonTemp = m_rectButton;
	if (!m_strBtnText.IsEmpty () && IsImageSet ())
	{
		CheckRectangle (rectButtonTemp, m_rectImage, bButtonLeftOfIcon);
	}

	if (!m_strBtnText.IsEmpty ())
	{
		AdjustRectToMargin (rectButtonTemp, rectClient, m_nMargin);
	}

	if (!m_strText.IsEmpty ())
	{
		CheckRectangle (m_rectDrawText, m_rectImage, bTextLeftOfIcon); 
		CheckRectangle (m_rectDrawText, rectButtonTemp, bTextLeftOfButton);
	}

	dc.SelectObject (pOldFont);
}
//*****************************************************************************
BOOL CBCGCaptionBar::CheckRectangle (CRect& rectSrc, const CRect& rectOther, 
									 BOOL bLeftOf)
{
	if (rectSrc.IsRectEmpty () || rectOther.IsRectEmpty ())
	{
		return FALSE;
	}

	CRect rectOtherWithOffset = rectOther;
	rectOtherWithOffset.InflateRect (m_nHorzElementOffset, m_nHorzElementOffset);

	if (rectSrc.left <= rectOtherWithOffset.right && 
		rectSrc.left >= rectOtherWithOffset.left)
	{
		rectSrc.left = rectOtherWithOffset.right;
	}

	if (rectSrc.right >= rectOtherWithOffset.left &&
		rectSrc.right <= rectOtherWithOffset.right)
	{
		rectSrc.right = rectOtherWithOffset.left;
	}

	if (rectSrc.left >= rectOtherWithOffset.left && 
		rectSrc.right <= rectOtherWithOffset.right)
	{
		rectSrc.right = rectSrc.left;
	}

	if (rectSrc.left <= rectOtherWithOffset.left && 
		rectSrc.right >= rectOtherWithOffset.right)
	{
		if (bLeftOf)
		{
			rectSrc.right = rectOtherWithOffset.left;
		}
		else
		{
			rectSrc.left = rectOtherWithOffset.right;
		}
	}

	if (bLeftOf && rectSrc.left >= rectOtherWithOffset.right ||
		!bLeftOf && rectSrc.right <= rectOtherWithOffset.left)
	{
		rectSrc.left = rectSrc.right;
	}

	return FALSE;
}
//*****************************************************************************
void CBCGCaptionBar::AdjustRectToMargin (CRect& rectSrc, const CRect& rectClient, 
										 int nMargin, BOOL bRetainSize)
{
	BOOL bLeftChanged = FALSE;
	BOOL bRightChanged = FALSE;
	int nWidth = rectSrc.Width ();
	if (rectSrc.left < rectClient.left + nMargin)
	{
		rectSrc.left = rectClient.left + nMargin;
		bLeftChanged = TRUE;
	}

	if (rectSrc.right > rectClient.right - nMargin)
	{
		rectSrc.right = rectClient.right - nMargin;
		bRightChanged = TRUE;
	}

	if (bRetainSize)
	{
		if (bLeftChanged)   
		{
			rectSrc.right = rectSrc.left + nWidth;
		}
		else if (bRightChanged)
		{
			rectSrc.left = 	rectSrc.right - nWidth;
		}
	}
}
//*****************************************************************************
CSize CBCGCaptionBar::GetImageSize () const
{
	if (m_Bitmap.GetCount () > 0)
	{
		ASSERT (m_hIcon == NULL);
		return m_Bitmap.GetImageSize ();
	}

	if (m_hIcon == NULL)
	{
		return CSize (0, 0);
	}

	ICONINFO info;
	memset (&info, 0, sizeof (ICONINFO));

	::GetIconInfo (m_hIcon, &info);
	HBITMAP hBmp = info.hbmColor;

	BITMAP bmp;
	::GetObject (hBmp, sizeof (BITMAP), (LPVOID) &bmp);

	::DeleteObject (info.hbmColor);
	::DeleteObject (info.hbmMask);

	return CSize (bmp.bmWidth, bmp.bmHeight);
}
//*****************************************************************************
BOOL CBCGCaptionBar::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect (&rectClient);

	OnDrawBackground (pDC, rectClient);
	return TRUE;
}
//*************************************************************************************
void CBCGCaptionBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CControlBar::OnLButtonDown(nFlags, point);

	if (m_uiBtnID == 0 || !m_bBtnEnabled || !m_bIsBtnHighlighted)
	{
		return;
	}

	m_bIsBtnPressed = TRUE;
	InvalidateRect (m_rectButton);
	UpdateWindow ();

	ASSERT_VALID (GetOwner ());
	GetOwner()->SendMessage (WM_COMMAND, m_uiBtnID);
}
//*************************************************************************************
void CBCGCaptionBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsBtnPressed)
	{
		m_bIsBtnPressed = FALSE;
		InvalidateRect (m_rectButton);
		UpdateWindow ();
	}
	
	CControlBar::OnLButtonUp(nFlags, point);
}
//*************************************************************************************
void CBCGCaptionBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	CControlBar::OnMouseMove(nFlags, point);
	if (m_uiBtnID == 0 || !m_bBtnEnabled)
	{
		return;
	}

	BOOL bIsBtnHighlighted = m_rectButton.PtInRect (point);
	if (m_bIsBtnHighlighted != bIsBtnHighlighted)
	{
		m_bIsBtnHighlighted = bIsBtnHighlighted;
		m_bIsBtnPressed = (nFlags & MK_LBUTTON) && m_bIsBtnHighlighted;

		InvalidateRect (m_rectButton);
		UpdateWindow ();
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
afx_msg LRESULT CBCGCaptionBar::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_bIsBtnPressed || m_bIsBtnHighlighted)
	{
		m_bIsBtnPressed = FALSE;
		m_bIsBtnHighlighted = FALSE;

		InvalidateRect (m_rectButton);
		UpdateWindow ();
	}

	return 0;
}
//***************************************************************************************
void CBCGCaptionBar::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CFrameWnd* pParent = GetParentFrame ();
	if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
	{
		pParent->RecalcLayout ();
	}

	RecalcLayout ();
}
//*********************************************************************************
void CBCGCaptionBar::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (!CBCGToolBar::IsCustomizeMode ())
	{
		ASSERT_VALID (GetOwner ());

		ClientToScreen (&point);
		GetOwner ()->SendMessage (BCGM_TOOLBARMENU,
			(WPARAM) GetSafeHwnd (),
			MAKELPARAM(point.x, point.y));
		return;
	}
	
	CControlBar::OnRButtonUp(nFlags, point);
}
