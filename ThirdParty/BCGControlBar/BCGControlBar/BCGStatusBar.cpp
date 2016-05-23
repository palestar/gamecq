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
// BCGStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "globals.h"
#include "BCGStatusBar.h"
#include "BCGVisualManager.h"
#include "BCGPopupMenu.h"
#include "BCGDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar

/////////////////////////////////////////////////////////////////////////////
class CBCGStatusBarPaneInfo
{
	friend class CBCGStatusBar;

	RECT		rect;			// pane rectangle

	UINT		nID;			// IDC of indicator: 0 => normal text area
	UINT		nStyle;			// style flags (SBPS_*)
	int			cxText;			// width of string area in pixels
								//		on both sides there is a 1 pixel gap and
								//		a one pixel border, making a pane 4 
								//		pixels wider
	COLORREF	clrText;		// text color
	COLORREF	clrBackground;	// background color
	int			cxIcon;			// width of icon area
	int			cyIcon;			// height of icon area
	LPCTSTR		lpszText;		// text in the pane
	LPCTSTR		lpszToolTip;	// pane tooltip
	HIMAGELIST	hImage;			// pane icon or animation

	// Animation parameters
	int			nFrameCount;	// Number of animation frames
	int			nCurrFrame;		// Current frame

	// Progress bar properties
	long		nProgressCurr;	// Current progress value
	long		nProgressTotal;	// Total progress value	(-1 - no progress bar)
	BOOL		bProgressText;	// Display text: "x%"
	COLORREF	clrProgressBar;
	COLORREF	clrProgressBarDest;
	COLORREF	clrProgressText;

	CBCGStatusBarPaneInfo ()
	{
		Init ();
	}

	void Init ()
	{
		nID = 0;
		nStyle = 0;
		lpszText = NULL;
		lpszToolTip = NULL;
		clrText = (COLORREF)-1;
		clrBackground = (COLORREF)-1;
		hImage = NULL;
		cxIcon = 0;
		cyIcon = 0;
		rect = CRect (0, 0, 0, 0);
		nFrameCount = 0;
		nCurrFrame = 0;
		nProgressCurr = 0;
		nProgressTotal = -1;
		clrProgressBar = (COLORREF)-1;
		clrProgressBarDest = (COLORREF)-1;
		clrProgressText = (COLORREF)-1;
		bProgressText = FALSE;
	}
};

inline CBCGStatusBarPaneInfo* CBCGStatusBar::_GetPanePtr(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_nCount)
	{
		ASSERT (FALSE);
		return NULL;
	}

	if (m_pData == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	return ((CBCGStatusBarPaneInfo*)m_pData) + nIndex;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

const int nTextMargin = 4;	// Gap between image and text

CBCGStatusBar::CBCGStatusBar()
{
	m_hFont = NULL;

	// setup correct margins
	m_cxRightBorder = m_cxDefaultGap;
	m_cxSizeBox = 0;
	m_bHideSizeBox = FALSE;

	m_cxLeftBorder = 4;
	m_cyTopBorder = 2;
	m_cyBottomBorder = 0;
	m_cxRightBorder = 0;

	m_bPaneDoubleClick = FALSE;

	m_rectSizeBox.SetRectEmpty ();
}
//********************************************************************************
void CBCGStatusBar::OnSettingChange(UINT /*uFlags*/, LPCTSTR /* lpszSection */)
{
	RecalcLayout ();
}
//********************************************************************************
CBCGStatusBar::~CBCGStatusBar()
{
}
//********************************************************************************
void CBCGStatusBar::OnDestroy() 
{
	for (int i = 0; i < m_nCount; i++)
	{
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update
		SetTipText(i, NULL);
		SetPaneIcon(i, NULL, FALSE);
	}

	CControlBar::OnDestroy();
}
//********************************************************************************
BOOL CBCGStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	// in Win4, status bars do not have a border at all, since it is
	//  provided by the client area.
	if ((m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
	{
		m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
	}

	return CControlBar::PreCreateWindow(cs);
}
//********************************************************************************
BOOL CBCGStatusBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, nID);
}
//********************************************************************************
BOOL CBCGStatusBar::CreateEx (CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, 
							 UINT nID)
{
	ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	m_dwStyle = (dwStyle & CBRS_ALL);
	dwStyle |= dwCtrlStyle;

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();
	LPCTSTR lpszClass = AfxRegisterWndClass (CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE+1), NULL);

	if (!CWnd::Create(lpszClass, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	RecalcLayout ();
	return TRUE;
}
//********************************************************************************
BOOL CBCGStatusBar::SetIndicators(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// free strings before freeing array of elements
	for (int i = 0; i < m_nCount; i++)
		VERIFY(SetPaneText(i, NULL, FALSE));    // no update

	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(CBCGStatusBarPaneInfo)))
		return FALSE;

	ASSERT(nIDCount == m_nCount);

	HFONT hFont = GetCurrentFont ();

	BOOL bOK = TRUE;
	if (lpIDArray != NULL)
	{
		ASSERT(hFont != NULL);        // must have a font !
		CString strText;
		CClientDC dcScreen(NULL);
		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);

		for (int i = 0; i < nIDCount; i++)
		{
			CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(i);
			if (pSBP == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			pSBP->Init ();

			pSBP->nID = *lpIDArray++;
			if (pSBP->nID != 0)
			{
				if (!strText.LoadString(pSBP->nID))
				{
					TRACE1("Warning: failed to load indicator string 0x%04X.\n",
						pSBP->nID);
					bOK = FALSE;
					break;
				}

				pSBP->cxText = dcScreen.GetTextExtent(strText,
						strText.GetLength()).cx;
				ASSERT(pSBP->cxText >= 0);

				if (!SetPaneText(i, strText, FALSE))
				{
					bOK = FALSE;
					break;
				}
			}
			else
			{
				// no indicator (must access via index)
				// default to 1/4 the screen width (first pane is stretchy)
				pSBP->cxText = ::GetSystemMetrics(SM_CXSCREEN) / 4;

				if (i == 0)
				{
					pSBP->nStyle |= (SBPS_STRETCH | SBPS_NOBORDERS);
				}
			}
		}

		dcScreen.SelectObject(hOldFont);
	}

	RecalcLayout ();
	return bOK;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar attribute access

int CBCGStatusBar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	if (m_nCount <= 0)
	{
		return -1;
	}

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(0);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		if (pSBP->nID == nIDFind)
		{
			return i;
		}
	}

	return -1;
}
//*******************************************************************************
UINT CBCGStatusBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);
	
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pSBP->nID;
}
//*******************************************************************************
void CBCGStatusBar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	*lpRect = pSBP->rect;
}
//*******************************************************************************
UINT CBCGStatusBar::GetPaneStyle(int nIndex) const
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	return pSBP->nStyle;
}
//*******************************************************************************
void CBCGStatusBar::SetPaneStyle(int nIndex, UINT nStyle)
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->nStyle != nStyle)
	{
		// just change the style of 1 pane, and invalidate it
		pSBP->nStyle = nStyle;
		InvalidateRect (&pSBP->rect, FALSE);
		UpdateWindow ();
	}
}
//*******************************************************************************
int CBCGStatusBar::GetPaneWidth (int nIndex) const
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	CRect rect = pSBP->rect;
	return rect.Width ();
}
//********************************************************************************
void CBCGStatusBar::SetPaneWidth (int nIndex, int cx)
{
	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CRect rect = pSBP->rect;
	int cxCurr = rect.Width () - CX_BORDER * 4;

	int cxTextNew = cx - pSBP->cxIcon;
	if (pSBP->cxIcon > 0)
	{
		cxTextNew -= nTextMargin;
	}

	pSBP->cxText = max (0, cxTextNew);

	if (cx != cxCurr)
	{
		RecalcLayout ();
		Invalidate();
		UpdateWindow ();
	}
}
//********************************************************************************
void CBCGStatusBar::GetPaneInfo(int nIndex, UINT& nID, UINT& nStyle,
	int& cxWidth) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	nID = pSBP->nID;
	nStyle = pSBP->nStyle;

	CRect rect = pSBP->rect;
	cxWidth = rect.Width ();
}
//*******************************************************************************
void CBCGStatusBar::SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pSBP->nID = nID;
	SetPaneStyle(nIndex, nStyle);
	SetPaneWidth (nIndex, cxWidth);
}
//*******************************************************************************
void CBCGStatusBar::GetPaneText(int nIndex, CString& s) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
}
//*******************************************************************************
CString CBCGStatusBar::GetPaneText(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	CString s = pSBP->lpszText == NULL ? _T("") : pSBP->lpszText;
	return s;
}
//*******************************************************************************
BOOL CBCGStatusBar::SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (pSBP->lpszText != NULL)
	{
		if (lpszNewText != NULL && lstrcmp(pSBP->lpszText, lpszNewText) == 0)
		{
			return TRUE;        // nothing to change
		}

		free((LPVOID)pSBP->lpszText);
	}
	else if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		return TRUE; // nothing to change
	}

	BOOL bOK = TRUE;
	if (lpszNewText == NULL || *lpszNewText == '\0')
	{
		pSBP->lpszText = NULL;
	}
	else
	{
		pSBP->lpszText = _tcsdup(lpszNewText);
		if (pSBP->lpszText == NULL)
			bOK = FALSE; // old text is lost and replaced by NULL
	}

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}

	return bOK;
}
//*******************************************************************************
void CBCGStatusBar::SetPaneIcon (int nIndex, HICON hIcon, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	// Disable animation (if exist):
	SetPaneAnimation (nIndex, NULL, 0, FALSE);

	if (hIcon == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy (pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	ICONINFO iconInfo;
	::GetIconInfo (hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

	::DeleteObject (iconInfo.hbmColor);
	::DeleteObject (iconInfo.hbmMask);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, 
										ILC_MASK | ILC_COLORDDB, 1, 0);
		::ImageList_AddIcon (pSBP->hImage, hIcon);

		RecalcLayout ();
	}
	else
	{
		ASSERT (pSBP->cxIcon == bitmap.bmWidth);
		ASSERT (pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_ReplaceIcon (pSBP->hImage, 0, hIcon);
	}

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}
}
//*******************************************************************************
void CBCGStatusBar::SetPaneIcon (int nIndex, HBITMAP hBmp,
								 COLORREF clrTransparent, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	// Disable animation (if exist):
	SetPaneAnimation (nIndex, NULL, 0, FALSE);

	if (hBmp == NULL)
	{
		if (pSBP->hImage != NULL)
		{
			::ImageList_Destroy (pSBP->hImage);
		}

		pSBP->hImage = NULL;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	BITMAP bitmap;
	::GetObject (hBmp, sizeof (BITMAP), &bitmap);

	if (pSBP->hImage == NULL)
	{
		pSBP->cxIcon = bitmap.bmWidth;
		pSBP->cyIcon = bitmap.bmHeight;

		pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, 
										ILC_MASK | ILC_COLORDDB, 1, 0);
		RecalcLayout ();
	}
	else
	{
		ASSERT (pSBP->cxIcon == bitmap.bmWidth);
		ASSERT (pSBP->cyIcon == bitmap.bmHeight);

		::ImageList_Remove (pSBP->hImage, 0);
	}

	//---------------------------------------------------------
	// Because ImageList_AddMasked changes the original bitmap,
	// we need to create a copy:
	//---------------------------------------------------------
	HBITMAP hbmpCopy = (HBITMAP) ::CopyImage (hBmp, IMAGE_BITMAP, 0, 0, 0);
	::ImageList_AddMasked (pSBP->hImage, hbmpCopy, clrTransparent);
	::DeleteObject (hbmpCopy);

	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}
}
//*******************************************************************************
void CBCGStatusBar::SetPaneAnimation (int nIndex, HIMAGELIST hImageList, 
							UINT nFrameRate, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->nFrameCount > 0)
	{
		KillTimer (pSBP->nID);
	}

	if (pSBP->hImage != NULL)
	{
		::ImageList_Destroy (pSBP->hImage);
		pSBP->hImage = NULL;
	}

	pSBP->nCurrFrame = 0;
	pSBP->nFrameCount = 0;

	if (hImageList == NULL)
	{
		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	pSBP->nFrameCount = ::ImageList_GetImageCount (hImageList);
	if (pSBP->nFrameCount == 0)
	{
		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}

		return;
	}

	::ImageList_GetIconSize (hImageList, &pSBP->cxIcon, &pSBP->cyIcon);

	pSBP->hImage = ::ImageList_Create (pSBP->cxIcon, pSBP->cyIcon, 
									ILC_MASK | ILC_COLORDDB, 1, 1);

	for (int i =0; i < pSBP->nFrameCount; i++)
	{
		HICON hIcon = ::ImageList_GetIcon (hImageList, i, ILD_TRANSPARENT);
		ASSERT (hIcon != NULL);

		::ImageList_AddIcon (pSBP->hImage, hIcon);

		::DestroyIcon (hIcon);
	}

	RecalcLayout ();
	if (bUpdate)
	{
		InvalidatePaneContent (nIndex);
	}

	SetTimer (pSBP->nID, nFrameRate, NULL);
}
//*******************************************************************************
void CBCGStatusBar::EnablePaneProgressBar (int nIndex, long nTotal, 
										   BOOL bDisplayText,
										   COLORREF clrBar, COLORREF clrBarDest,
										   COLORREF clrProgressText)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pSBP->bProgressText = bDisplayText;
	pSBP->clrProgressBar = clrBar;
	pSBP->clrProgressBarDest = clrBarDest;
	pSBP->nProgressTotal = nTotal;
	pSBP->nProgressCurr = 0;
	pSBP->clrProgressText = clrProgressText;

	if (clrBarDest != (COLORREF)-1 && pSBP->bProgressText)
	{
		// Progress text is not available when the gradient is ON
		ASSERT (FALSE);
		pSBP->bProgressText = FALSE;
	}

	InvalidatePaneContent (nIndex);
}
//*******************************************************************************
void CBCGStatusBar::SetPaneProgress (int nIndex, long nCurr, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT (nCurr >= 0);
	ASSERT (nCurr <= pSBP->nProgressTotal);

	long lPos = min (max (0, nCurr), pSBP->nProgressTotal);
	if (pSBP->nProgressCurr != lPos)
	{
		pSBP->nProgressCurr = lPos;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
long CBCGStatusBar::GetPaneProgress (int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	return pSBP->nProgressCurr;
}
//*******************************************************************************
void CBCGStatusBar::SetPaneTextColor (int nIndex, COLORREF clrText, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->clrText != clrText)
	{
		pSBP->clrText = clrText;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
void CBCGStatusBar::SetPaneBackgroundColor (int nIndex, COLORREF clrBackground, BOOL bUpdate)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->clrBackground != clrBackground)
	{
		pSBP->clrBackground = clrBackground;

		if (bUpdate)
		{
			InvalidatePaneContent (nIndex);
		}
	}
}
//*******************************************************************************
CString CBCGStatusBar::GetTipText(int nIndex) const
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return _T("");
	}

	CString s = pSBP->lpszToolTip == NULL ? _T("") : pSBP->lpszToolTip;
	return s;
}
//*******************************************************************************
void CBCGStatusBar::SetTipText(int nIndex, LPCTSTR pszTipText)
{
	ASSERT_VALID(this);

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (pSBP->lpszToolTip != NULL)
	{
		if (pszTipText != NULL && lstrcmp(pSBP->lpszToolTip, pszTipText) == 0)
		{
			return;        // nothing to change
		}

		free((LPVOID)pSBP->lpszToolTip);
	}
	else if (pszTipText == NULL || *pszTipText == '\0')
	{
		return; // nothing to change
	}

	if (pszTipText == NULL || *pszTipText == '\0')
	{
		pSBP->lpszToolTip = NULL;
	}
	else
	{
		pSBP->lpszToolTip = _tcsdup(pszTipText);
	}

	SetBarStyle (GetBarStyle() | CBRS_TOOLTIPS);
}
//*******************************************************************************
void CBCGStatusBar::InvalidatePaneContent (int nIndex)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	// invalidate the text of the pane - not including the border

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
	else
		rect.top -= CY_BORDER;  // base line adjustment

	InvalidateRect (rect, FALSE);
	UpdateWindow ();
}
//*********************************************************************************
void CBCGStatusBar::EnablePaneDoubleClick (BOOL bEnable)
{
	m_bPaneDoubleClick = bEnable;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar implementation

CSize CBCGStatusBar::CalcFixedLayout(BOOL, BOOL bHorz)
{
	ASSERT_VALID(this);

	// recalculate based on font height + icon height + borders
	TEXTMETRIC tm;
	{
		CClientDC dcScreen(NULL);
		HFONT hFont = GetCurrentFont ();

		HGDIOBJ hOldFont = dcScreen.SelectObject(hFont);
		VERIFY(dcScreen.GetTextMetrics(&tm));
		dcScreen.SelectObject(hOldFont);
	}

	int cyIconMax = 0;
	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		cyIconMax = max (cyIconMax, pSBP->cyIcon);
	}

	CRect rectSize;
	rectSize.SetRectEmpty();
	CalcInsideRect(rectSize, bHorz);    // will be negative size

	// sizeof text + 1 or 2 extra on top, 2 on bottom + borders
	return CSize(32767, max (cyIconMax, tm.tmHeight - tm.tmInternalLeading) +
		CY_BORDER * 4 - rectSize.Height());
}
//*******************************************************************************
void CBCGStatusBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;
	pDCPaint->GetClipBox (rectClip);

	CRect rect;
	GetClientRect(rect);

	CDC*		pDC = pDCPaint;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC (pDCPaint) &&
		bmp.CreateCompatibleBitmap (pDCPaint, rect.Width (), rect.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

	CBCGVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
		rect, rectClip);

	CControlBar::DoPaint(pDC);      // draw border

	HFONT hFont = GetCurrentFont ();
	HGDIOBJ hOldFont = pDC->SelectObject(hFont);

	int nOldMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF crTextColor = pDC->SetTextColor(globalData.clrBtnText);
	COLORREF crBkColor = pDC->SetBkColor(globalData.clrBtnFace);

	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	for (int i = 0; i < m_nCount; i++, pSBP++)
	{
		OnDrawPane (pDC, pSBP);
	}

	pDC->SelectObject(hOldFont);

	// draw the size box in the bottom right corner
	if (!m_rectSizeBox.IsRectEmpty ())
	{
		CBCGVisualManager::GetInstance ()->OnDrawStatusBarSizeBox (pDC, this,
			m_rectSizeBox);
	}

	pDC->SetTextColor (crTextColor);
	pDC->SetBkColor (crBkColor);
	pDC->SetBkMode (nOldMode);

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		pDCPaint->BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
					   &dcMem, rectClip.left, rectClip.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar message handlers

BEGIN_MESSAGE_MAP(CBCGStatusBar, CControlBar)
	//{{AFX_MSG_MAP(CBCGStatusBar)
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_WM_SETTINGCHANGE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CBCGStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}
//****************************************************************************************
void CBCGStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_bPaneDoubleClick)
	{
		CBCGStatusBarPaneInfo* pSBP = HitTest (point);
		if (pSBP != NULL)
		{
			GetOwner()->PostMessage (WM_COMMAND, pSBP->nID);
		}
	}
	
	CControlBar::OnLButtonDblClk(nFlags, point);
}
//**********************************************************************************
void CBCGStatusBar::OnTimer(UINT_PTR nIDEvent) 
{
	CControlBar::OnTimer(nIDEvent);

	int nIndex = CommandToIndex ((UINT) nIDEvent);
	if (nIndex < 0)
	{
		return;
	}

	CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
	if (pSBP == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (++pSBP->nCurrFrame >= pSBP->nFrameCount)
	{
		pSBP->nCurrFrame = 0;
	}

	CRect rect = pSBP->rect;

	if (!(pSBP->nStyle & SBPS_NOBORDERS))
		rect.InflateRect(-CX_BORDER, -CY_BORDER);
	else
		rect.top -= CY_BORDER;  // base line adjustment

	rect.right = rect.left + pSBP->cxIcon;
	InvalidateRect(rect, FALSE);
	UpdateWindow ();

	ClientToScreen (&rect);
	CBCGPopupMenu::UpdateAllShadows (rect);
}
//**********************************************************************************
BCGNcHitTestType CBCGStatusBar::OnNcHitTest(CPoint point)
{
	// hit test the size box - convert to HTCAPTION if so
	if (!m_bHideSizeBox && m_cxSizeBox != 0)
	{
		CRect rect;
		GetClientRect(rect);
		CalcInsideRect(rect, TRUE);
		int cxMax = min(m_cxSizeBox-1, rect.Height());
		rect.left = rect.right - cxMax;
		ClientToScreen(&rect);
		if (rect.PtInRect(point))
			return (GetExStyle() && WS_EX_LAYOUTRTL) ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
	}
	return CControlBar::OnNcHitTest(point);
}
//*******************************************************************************
void CBCGStatusBar::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (!m_bHideSizeBox && m_cxSizeBox != 0 && (nID & 0xFFF0) == SC_SIZE)
	{
		CFrameWnd* pFrameWnd = GetParentFrame();
		if (pFrameWnd != NULL)
		{
			pFrameWnd->SendMessage(WM_SYSCOMMAND, (WPARAM)nID, lParam);
			return;
		}
	}

	CControlBar::OnSysCommand(nID, lParam);
}
//*******************************************************************************
void CBCGStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);

	RecalcLayout ();

	// force repaint on resize (recalculate stretchy)
	Invalidate();
	UpdateWindow ();
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	m_hFont = (HFONT)wParam;
	ASSERT(m_hFont != NULL);

	RecalcLayout ();

	if ((BOOL)lParam)
	{
		Invalidate();
		UpdateWindow ();
	}

	return 0L;      // does not re-draw or invalidate - resize parent instead
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnGetFont(WPARAM, LPARAM)
{
	HFONT hFont = GetCurrentFont ();
	return (LRESULT)(UINT_PTR)hFont;
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnSetText(WPARAM, LPARAM lParam)
{
	int nIndex = CommandToIndex(0);
	if (nIndex < 0)
		return -1;
	return SetPaneText(nIndex, (LPCTSTR)lParam) ? 0 : -1;
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnGetText(WPARAM wParam, LPARAM lParam)
{
	int nMaxLen = (int)wParam;
	if (nMaxLen == 0)
		return 0;       // nothing copied
	LPTSTR lpszDest = (LPTSTR)lParam;

	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT (FALSE);
			return 0;
		}

		nLen = pSBP->lpszText != NULL ? lstrlen(pSBP->lpszText) : 0;
		if (nLen > nMaxLen)
			nLen = nMaxLen - 1; // number of characters to copy (less term.)
		memcpy(lpszDest, pSBP->lpszText, nLen*sizeof(TCHAR));
	}
	lpszDest[nLen] = '\0';
	return nLen+1;      // number of bytes copied
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnGetTextLength(WPARAM, LPARAM)
{
	int nLen = 0;
	int nIndex = CommandToIndex(0); // use pane with ID zero
	if (nIndex >= 0)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(nIndex);
		if (pSBP == NULL)
		{
			ASSERT (FALSE);
			return 0;
		}

		if (pSBP->lpszText != NULL)
		{
			nLen = lstrlen(pSBP->lpszText);
		}
	}

	return nLen;
}
//*******************************************************************************
LRESULT CBCGStatusBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	if (lpLayout->hDWP != NULL)
	{
		// hide size box if parent is maximized
		CFrameWnd* pFrameWnd = GetParentFrame();
		if (pFrameWnd != NULL)
		{
			// the size box only appears when status bar is on the bottom
			//  of a non-maximized, sizeable frame window.
			CRect rectFrame;
			pFrameWnd->GetClientRect(rectFrame);
			BOOL bHideSizeBox = pFrameWnd->IsZoomed() ||
				!(pFrameWnd->GetStyle() & WS_THICKFRAME) ||
				rectFrame.bottom != lpLayout->rect.bottom ||
				rectFrame.right != lpLayout->rect.right;

			// update the size box hidden status, if changed
			if (bHideSizeBox != m_bHideSizeBox)
			{
				m_bHideSizeBox = bHideSizeBox;
				Invalidate();
				UpdateWindow ();
			}
		}
	}

	return CControlBar::OnSizeParent(wParam, lParam);
}
//*******************************************************************************
void CBCGStatusBar::OnDrawPane (CDC* pDC, CBCGStatusBarPaneInfo* pPane)
{
	ASSERT_VALID (pDC);
	ASSERT (pPane != NULL);

	CRect rectPane = pPane->rect;
	if (rectPane.IsRectEmpty () || !pDC->RectVisible (rectPane))
	{
		return;
	}

	// Fill pane background:
	if (pPane->clrBackground != (COLORREF)-1)
	{
		CBrush brush (pPane->clrBackground);
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		pDC->PatBlt (rectPane.left, rectPane.top, rectPane.Width(), rectPane.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
	}

	// Draw pane border:
	CBCGVisualManager::GetInstance ()->OnDrawStatusBarPaneBorder (pDC, this,
		rectPane, pPane->nID, pPane->nStyle);

	if (!(pPane->nStyle & SBPS_NOBORDERS)) // only adjust if there are borders
	{
		rectPane.DeflateRect (2 * CX_BORDER, CY_BORDER);
	}

	// Draw icon
	if (pPane->hImage != NULL && pPane->cxIcon > 0)
	{
		CRect rectIcon = rectPane;
		rectIcon.right = rectIcon.left + pPane->cxIcon;

		int x = max (0, (rectIcon.Width () - pPane->cxIcon) / 2);
		int y = max (0, (rectIcon.Height () - pPane->cyIcon) / 2);

		::ImageList_DrawEx (pPane->hImage, pPane->nCurrFrame, pDC->GetSafeHdc (),
				rectIcon.left + x, rectIcon.top + y,
				pPane->cxIcon, pPane->cyIcon, CLR_NONE, 0, ILD_NORMAL);
	}

	CRect rectText = rectPane;
	rectText.left += pPane->cxIcon;

	if (pPane->cxIcon > 0)
	{
		rectText.left += nTextMargin;
	}

	if (pPane->nProgressTotal > 0)
	{
		// Draw progress bar:
		CRect rectProgress = rectText;
		rectProgress.DeflateRect (1, 1);

		COLORREF clrBar = (pPane->clrProgressBar == (COLORREF)-1) ?
			::GetSysColor (COLOR_HIGHLIGHT) : pPane->clrProgressBar;

		CBCGVisualManager::GetInstance ()->OnDrawStatusBarProgress (pDC, this,
			rectProgress, pPane->nProgressTotal, pPane->nProgressCurr,
			clrBar, pPane->clrProgressBarDest, pPane->clrProgressText,
			pPane->bProgressText);
	}
	else
	{
		// Draw text
		if (pPane->lpszText != NULL && pPane->cxText > 0)
		{
			COLORREF clrText = pDC->SetTextColor ((pPane->nStyle & SBPS_DISABLED) ? 
				globalData.clrGrayedText : 
				pPane->clrText == (COLORREF)-1 ? 
					globalData.clrBtnText : pPane->clrText);

			pDC->DrawText (pPane->lpszText, lstrlen(pPane->lpszText), rectText,
				DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

			pDC->SetTextColor (clrText);
		}
	}
}
//**********************************************************************************
void CBCGStatusBar::RecalcLayout ()
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	// get the drawing area for the status bar
	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, TRUE);

	// the size box is based off the size of a scrollbar
	m_cxSizeBox = min(GetSystemMetrics(SM_CXVSCROLL)+1, rect.Height());

	CClientDC dcScreen (NULL);

	// protect space for size box
	int cxSizeBox = m_bHideSizeBox ? 0 : m_cxSizeBox;
	int xMax = (rect.right -= cxSizeBox);
	if (cxSizeBox == 0)
		xMax += m_cxRightBorder + 1;

	// walk through to calculate extra space
	int cxExtra = rect.Width() + m_cxDefaultGap;
	CBCGStatusBarPaneInfo* pSBP = (CBCGStatusBarPaneInfo*)m_pData;
	int i = 0;

	for (; i < m_nCount; i++, pSBP++)
	{
		cxExtra -= (pSBP->cxText + pSBP->cxIcon + CX_BORDER * 4 + m_cxDefaultGap);
		
		if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
		{
			cxExtra -= nTextMargin;
		}
	}
	// if cxExtra <= 0 then we will not stretch but just clip

	for (i = 0, pSBP = (CBCGStatusBarPaneInfo*)m_pData; i < m_nCount; i++, pSBP++)
	{
		ASSERT(pSBP->cxText >= 0);
		ASSERT(pSBP->cxIcon >= 0);

		if (rect.left >= xMax)
		{
			pSBP->rect = CRect (0, 0, 0, 0);
		}
		else
		{
			int cxPane = pSBP->cxText + pSBP->cxIcon;
			if (pSBP->cxText > 0 && pSBP->cxIcon > 0)
			{
				cxPane += nTextMargin;
			}

			if ((pSBP->nStyle & SBPS_STRETCH) && cxExtra > 0)
			{
				cxPane += cxExtra;
				cxExtra = 0;
			}

			rect.right = rect.left + cxPane + CX_BORDER * 4;
			rect.right = min(rect.right, xMax);

			pSBP->rect = rect;

			rect.left = rect.right + m_cxDefaultGap;
		}
	}

	if (cxSizeBox != 0)
	{
		int cxMax = min(cxSizeBox, rect.Height()+m_cyTopBorder);

		m_rectSizeBox = rect;
		m_rectSizeBox.left = rect.right;
		m_rectSizeBox.right = m_rectSizeBox.left + cxMax;
	}
	else
	{
		m_rectSizeBox.SetRectEmpty ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar idle update through CBCGStatusCmdUI class

class CBCGStatusCmdUI : public CCmdUI      // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CBCGStatusCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CBCGStatusBar* pStatusBar = (CBCGStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CBCGStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_DISABLED;
	if (!bOn)
		nNewStyle |= SBPS_DISABLED;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}
//*******************************************************************************
void CBCGStatusCmdUI::SetCheck(int nCheck) // "checking" will pop out the text
{
	CBCGStatusBar* pStatusBar = (CBCGStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CBCGStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_POPOUT;
	if (nCheck != 0)
		nNewStyle |= SBPS_POPOUT;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}
//*******************************************************************************
void CBCGStatusCmdUI::SetText(LPCTSTR lpszText)
{
	ASSERT(m_pOther != NULL);
	ASSERT_KINDOF(CBCGStatusBar, m_pOther);
	ASSERT(m_nIndex < m_nIndexMax);

	((CBCGStatusBar*)m_pOther)->SetPaneText(m_nIndex, lpszText);
}
//*******************************************************************************
void CBCGStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CBCGStatusCmdUI state;
	state.m_pOther = this;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = _GetPanePtr(state.m_nIndex)->nID;
		state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the status bar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}
//*************************************************************************************
INT_PTR CBCGStatusBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	// check child windows first by calling CControlBar
	INT_PTR nHit = CControlBar::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	CBCGStatusBarPaneInfo* pSBP = HitTest (point);
	if (pSBP != NULL && pSBP->lpszToolTip != NULL)
	{
		nHit = pSBP->nID;

		if (pTI != NULL)
		{
			CString strTipText = pSBP->lpszToolTip;

			pTI->lpszText = (LPTSTR) ::calloc ((strTipText.GetLength () + 1), sizeof (TCHAR));
			_tcscpy (pTI->lpszText, strTipText);

			pTI->rect = pSBP->rect;
			pTI->uId = 0;
			pTI->hwnd = m_hWnd;
		}
	}

#if _MSC_VER >= 1300
	CToolTipCtrl* pToolTip = AfxGetModuleState()->m_thread.GetDataNA()->m_pToolTip;
#else
	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
#endif
	if (pToolTip != NULL && pToolTip->GetSafeHwnd () != NULL)
	{
		pToolTip->SetFont (&globalData.fontTooltip, FALSE);
	}

	return nHit;
}
//****************************************************************************************
CBCGStatusBarPaneInfo* CBCGStatusBar::HitTest (CPoint pt) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_nCount; i++)
	{
		CBCGStatusBarPaneInfo* pSBP = _GetPanePtr(i);
		ASSERT (pSBP != NULL);

		CRect rect = pSBP->rect;
		if (rect.PtInRect (pt))
		{
			return pSBP;
		}
	}

	return NULL;
}
//**************************************************************************************
HFONT CBCGStatusBar::GetCurrentFont () const
{
	return m_hFont == NULL ? 
		(HFONT) globalData.fontRegular.GetSafeHandle () : 
		m_hFont;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGStatusBar diagnostics

#ifdef _DEBUG
void CBCGStatusBar::AssertValid() const
{
	CControlBar::AssertValid();
}
//********************************************************************************
void CBCGStatusBar::Dump(CDumpContext& dc) const
{
	CControlBar::Dump(dc);

	dc << "\nm_hFont = " << (UINT_PTR)m_hFont;

	if (dc.GetDepth() > 0)
	{
		for (int i = 0; i < m_nCount; i++)
		{
			dc << "\nstatus pane[" << i << "] = {";
			dc << "\n\tnID = " << _GetPanePtr(i)->nID;
			dc << "\n\tnStyle = " << _GetPanePtr(i)->nStyle;
			dc << "\n\tcxText = " << _GetPanePtr(i)->cxText;
			dc << "\n\tcxIcon = " << _GetPanePtr(i)->cxIcon;
			dc << "\n\tlpszText = " << _GetPanePtr(i)->lpszText;
			dc << "\n\t}";
		}
	}

	dc << "\n";
}
#endif //_DEBUG

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CBCGStatusBar, CControlBar)
