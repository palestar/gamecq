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

// bcgtabwnd.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_TABCTRL

#include "bcgtabwnd.h"
#include "BCGVisualManager.h"
#include "BCGContextMenuManager.h"
#include "bcglocalres.h"
#include "bcgbarres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT BCGM_CHANGE_ACTIVE_TAB	= ::RegisterWindowMessage (_T("BCGM_ONCHANGE_ACTIVE_TAB"));
UINT BCGM_ON_HSCROLL		= ::RegisterWindowMessage (_T("BCGM_ON_HSCROLL"));
UINT BCGM_ON_RENAME_TAB		= ::RegisterWindowMessage (_T("BCGM_ON_RENAME_TAB"));
UINT BCGM_ON_BEFOREMOVE_TAB	= ::RegisterWindowMessage (_T("BCGM_ON_BEFOREMOVE_TAB"));
UINT BCGM_ON_MOVE_TAB		= ::RegisterWindowMessage (_T("BCGM_ON_MOVE_TAB"));

/////////////////////////////////////////////////////////////////////////////
// CBCGTabInfo

class CBCGTabInfo : public CObject
{
	friend class CBCGTabWnd;

	CBCGTabInfo(const CString&	strText,
				const UINT		uiIcon,
				CWnd*			pWnd,
				const int		iTabID) :
		m_pWnd (pWnd),
		m_uiIcon (uiIcon),
		m_iTabID (iTabID)
	{
		m_strText = strText;
		m_rect.SetRectEmpty ();
		m_bVisible = TRUE;
		m_nFullWidth = 0;
		m_clrText = (COLORREF) -1;
		m_clrBack = (COLORREF) -1;
		m_bIconOnly = FALSE;
		m_bAlwaysShowToolTip = FALSE;

		if (m_pWnd != NULL)
		{
			TCHAR szClass [256];
			::GetClassName (m_pWnd->GetSafeHwnd (), szClass, 255);

			CString strClass = szClass;
			m_bIsListView = (strClass == _T("SysListView32"));
		}
		else
		{
			m_bIsListView = FALSE;
		}
	}

	CString		m_strText;
	UINT		m_uiIcon;
	CRect		m_rect;
	BOOL		m_bVisible;
	CWnd*		m_pWnd;
	BOOL		m_bIsListView;
	int			m_nFullWidth;
	const int	m_iTabID;
	COLORREF	m_clrText;
	COLORREF	m_clrBack;
	BOOL		m_bIconOnly;
	BOOL		m_bAlwaysShowToolTip;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGTabButton

void CBCGTabButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	CBCGVisualManager::GetInstance ()->OnEraseTabsButton (pDC, rectClient, this,
		DYNAMIC_DOWNCAST (CBCGTabWnd, GetParent ()));
}

void CBCGTabButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	CBCGVisualManager::GetInstance ()->OnDrawTabsButtonBorder (pDC, rectClient,
		this, uiState, DYNAMIC_DOWNCAST (CBCGTabWnd, GetParent ()));
}

/////////////////////////////////////////////////////////////////////////////
// CBCGTabWnd

IMPLEMENT_DYNCREATE(CBCGTabWnd, CWnd)

int CBCGTabWnd::TAB_TEXT_MARGIN = 4;
int CBCGTabWnd::TAB_IMAGE_MARGIN = 4;

#define DEFAULT_TAB_BORDER_SIZE	3
#define TEXT_MARGIN				4
#define MIN_SROLL_WIDTH			(::GetSystemMetrics (SM_CXHSCROLL) * 2)
#define SPLITTER_WIDTH			5
#define TABS_FONT				_T("Arial")

static const int nScrollLeftEventId = 1;
static const int nScrollRightEventId = 2;
static const int nScrollDelay = 250;	// ms

CMap<UINT,UINT,HICON,HICON>	CBCGTabWnd::m_mapDocIcons;

CBCGTabWnd::CBCGTabWnd()
{
	m_iTabsNum = 0;
	m_bIsOneNoteStyle = FALSE;
	m_bIsVS2005Style = FALSE;
	m_bLeftRightRounded = FALSE;
	m_iActiveTab = -1;
	m_sizeImage = CSize (0, 0);
	m_hImageList = NULL;
	m_bFlat = FALSE;
	m_bScroll = FALSE;
	m_bCloseBtn = FALSE;
	m_bSharedScroll = FALSE;
	m_rectTabsArea.SetRectEmpty ();
	m_rectWndArea.SetRectEmpty ();
	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;
	m_nTabsHorzOffsetMax = 0;
	m_nTabsTotalWidth = 0;
	m_nHorzScrollWidth = 0;
	m_nScrollBarRight = 0;
	m_rectTabSplitter.SetRectEmpty ();
	m_bTrackSplitter = FALSE;
	m_location = LOCATION_BOTTOM;
	m_bFlatFrame = TRUE;

	m_clrActiveTabBk = (COLORREF) -1;
	m_clrActiveTabFg = (COLORREF) -1;
	
	m_nTabsHeight = 0;
	m_nTabBorderSize = DEFAULT_TAB_BORDER_SIZE;

	m_bHideInactiveWnd = TRUE;
	m_bAutoSizeWindow = TRUE;
	m_bAutoDestoyWindow = TRUE;

	m_bTransparent = FALSE;
	m_bTopEdge = FALSE;
	m_bDrawFrame = TRUE;

	m_iCurTab = -1;
	m_nNextTabID = 1;

	m_iHighlighted = -1;

	m_bHideSingleTab = FALSE;
	m_bHideNoTabs = FALSE;

	m_bIsInPlaceEdit = FALSE;
	m_pInPlaceEdit = NULL;
	m_iEditedTab = -1;

	m_bAutoHideScroll = FALSE;
	m_bLabelNoPrefix = FALSE;

	m_bDragDrop = FALSE;
	m_bDragging = FALSE;

	m_bIsAutoColor = FALSE;
	m_bIsDefaultAutoColor = TRUE;

	m_bIsDlgControl = FALSE;
	m_bTabDocumentsMenu = FALSE;
	m_bHiddenDocuments = FALSE;

	m_rectCurrClip.SetRectEmpty ();
}
//***************************************************************************************
CBCGTabWnd::~CBCGTabWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGTabWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGTabWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGTabWnd::Create (Style style, const RECT& rect, CWnd* pParentWnd, 
						 UINT nID, Location location /* = LOCATION_BOTTOM*/,
						 BOOL bCloseBtn /*= FALSE */)
{
	m_bFlat = (style == STYLE_FLAT) || (style == STYLE_FLAT_SHARED_HORZ_SCROLL);
	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || 
		style == STYLE_3D_ROUNDED_SCROLL);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_bSharedScroll = style == STYLE_FLAT_SHARED_HORZ_SCROLL;
	m_location = location;
	m_bScroll = (m_bFlat || style == STYLE_3D_SCROLLED || 
				 style == STYLE_3D_ONENOTE || /*style == STYLE_3D_VS2005 ||*/
				 style == STYLE_3D_ROUNDED_SCROLL);
	m_bCloseBtn = bCloseBtn;

	m_bFlatFrame = !m_bFlat;

	if (!m_bFlat && m_bSharedScroll)
	{
		//--------------------------------------
		// Only flat tab has a shared scrollbar!
		//--------------------------------------
		ASSERT (FALSE);
		m_bSharedScroll = FALSE;
	}

	return CWnd::Create (globalData.RegisterWindowClass (_T("BCGTabWnd")),
		_T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, rect,
		pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGTabWnd message handlers

void CBCGTabWnd::OnDestroy() 
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (m_bAutoDestoyWindow)
		{
			pTab->m_pWnd->DestroyWindow ();
		}

		delete pTab;
	}

	if (m_brActiveTab.GetSafeHandle () != NULL)
	{
		m_brActiveTab.DeleteObject ();
	}

	if (m_ToolTip.GetSafeHwnd () != NULL)
	{
		m_ToolTip.DestroyWindow ();
	}

	CWnd::OnDestroy();
}
//***************************************************************************************
void CBCGTabWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBCGMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	dc.GetClipBox (&m_rectCurrClip);

	COLORREF	clrDark;
	COLORREF	clrBlack;
	COLORREF	clrHighlight;
	COLORREF	clrFace;
	COLORREF	clrDarkShadow;
	COLORREF	clrLight;
	CBrush*		pbrFace = NULL;
	CBrush*		pbrBlack = NULL;
				   
	CBCGVisualManager::GetInstance ()->GetTabFrameColors (
		this, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight,
		pbrFace, pbrBlack);

	ASSERT_VALID (pbrFace);
	ASSERT_VALID (pbrBlack);

	CRect rectClient;
	GetClientRect (&rectClient);

	CBrush* pOldBrush = pDC->SelectObject (pbrFace);
	ASSERT (pOldBrush != NULL);

	CPen penDark (PS_SOLID, 1, clrDark);
	CPen penBlack (PS_SOLID, 1, clrBlack);
	CPen penHiLight (PS_SOLID, 1, clrHighlight);

	CPen* pOldPen = (CPen*) pDC->SelectObject (&penDark);
	ASSERT(pOldPen != NULL);

	const int nTabBorderSize = GetTabBorderSize ();

	CRect rectTabs = rectClient;

	if (m_location == LOCATION_BOTTOM)
	{
		rectTabs.top = m_rectTabsArea.top;
	}
	else
	{
		rectTabs.bottom = m_rectTabsArea.bottom;
	}
	
	pDC->ExcludeClipRect (m_rectWndArea);

	BOOL bBackgroundIsReady =
		CBCGVisualManager::GetInstance ()->OnEraseTabsFrame (pDC, rectClient, this);

	if (!m_bDrawFrame && !bBackgroundIsReady)
	{
		pDC->FillRect (rectClient, pbrFace);
	}

	CBCGVisualManager::GetInstance ()->OnEraseTabsArea (pDC, rectTabs, this);

	CRect rectFrame = rectClient;

	if (nTabBorderSize == 0)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			rectFrame.bottom = m_rectTabsArea.top + 1;
		}
		else
		{
			rectFrame.top = m_rectTabsArea.bottom - 1;
		}

		if (m_bFlat)
		{
			pDC->FrameRect(&rectFrame, pbrBlack);
		}
		else
		{
			pDC->FrameRect(&rectFrame, pbrFace);
		}
	}
	else
	{
		int yLine = m_location == LOCATION_BOTTOM ? 
			m_rectTabsArea.top : m_rectTabsArea.bottom;

		if (!m_bFlat)
		{
			if (m_location == LOCATION_BOTTOM)
			{
				rectFrame.bottom = m_rectTabsArea.top;
			}
			else
			{
				rectFrame.top = m_rectTabsArea.bottom;
			}
		}

		//-----------------------------------------------------
		// Draw wide 3-dimensional frame around the Tabs area:
		//-----------------------------------------------------
		if (m_bFlatFrame)
		{
			CRect rectBorder (rectFrame);

			if (m_bFlat)
			{
				if (m_location == LOCATION_BOTTOM)
				{
					rectBorder.bottom = m_rectTabsArea.top + 1;
				}
				else
				{
					rectBorder.top = m_rectTabsArea.bottom - 1;
				}
			}

			rectFrame.DeflateRect (1, 1);

			if (m_bDrawFrame && !bBackgroundIsReady && rectFrame.Width () > 0 && rectFrame.Height () > 0)
			{
				pDC->PatBlt (rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
				pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), nTabBorderSize, PATCOPY);
				pDC->PatBlt (rectFrame.right - nTabBorderSize - 1, rectFrame.top, nTabBorderSize + 1, rectFrame.Height (), PATCOPY);
				pDC->PatBlt (rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width (), nTabBorderSize, PATCOPY);

				if (m_location == LOCATION_BOTTOM)
				{
					pDC->PatBlt (rectFrame.left, m_rectWndArea.bottom, rectFrame.Width (), 
						rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
				}
				else
				{
					pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), 
						m_rectWndArea.top - rectFrame.top, PATCOPY);
				}
			}

			if (m_bFlat)
			{
				//---------------------------
				// Draw line below the tabs:
				//---------------------------
				pDC->SelectObject (&penBlack);
				pDC->MoveTo (rectFrame.left + nTabBorderSize, yLine);
				pDC->LineTo (rectFrame.right - nTabBorderSize, yLine);
			}

			pDC->Draw3dRect (&rectBorder, clrFace, clrFace);

			if (GetTabsHeight () == 0)
			{
				pDC->Draw3dRect (&rectBorder, clrFace, clrFace);
			}
			else
			{
				if (m_bDrawFrame)
				{
					pDC->Draw3dRect (&rectBorder, clrDark, clrDark);
				}

				if (!m_bIsOneNoteStyle)
				{
					int xRight = rectBorder.right - 1;

					if (!m_bDrawFrame)
					{
						xRight -= nTabBorderSize;
					}

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->SelectObject (&penBlack);

						pDC->MoveTo (rectBorder.left, rectBorder.bottom - 1);
						pDC->LineTo (xRight, rectBorder.bottom - 1);
					}
					else
					{
						pDC->SelectObject (&penHiLight);

						pDC->MoveTo (rectBorder.left, rectBorder.top);
						pDC->LineTo (xRight, rectBorder.top);
					}
				}
			}
		}
		else
		{
			if (m_bDrawFrame)
			{
				pDC->Draw3dRect (&rectFrame, clrHighlight, clrDarkShadow);
				
				rectFrame.DeflateRect (1, 1);
				pDC->Draw3dRect (&rectFrame, clrLight, clrDark);
				
				rectFrame.DeflateRect (1, 1);
				
				if (!bBackgroundIsReady &&
					rectFrame.Width () > 0 && rectFrame.Height () > 0)
				{
					pDC->PatBlt (rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
					pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), nTabBorderSize, PATCOPY);
					pDC->PatBlt (rectFrame.right - nTabBorderSize, rectFrame.top, nTabBorderSize, rectFrame.Height (), PATCOPY);
					pDC->PatBlt (rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width (), nTabBorderSize, PATCOPY);
					
					if (m_location == LOCATION_BOTTOM)
					{
						pDC->PatBlt (rectFrame.left, m_rectWndArea.bottom, rectFrame.Width (), 
							rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
					}
					else
					{
						pDC->PatBlt (rectFrame.left, rectFrame.top, rectFrame.Width (), 
							m_rectWndArea.top - rectFrame.top, PATCOPY);
					}

					if (m_bFlat)
					{
						//---------------------------
						// Draw line below the tabs:
						//---------------------------
						pDC->SelectObject (&penBlack);

						pDC->MoveTo (rectFrame.left + nTabBorderSize, yLine);
						pDC->LineTo (rectFrame.right - nTabBorderSize, yLine);
					}

					if (nTabBorderSize > 2)
					{
						rectFrame.DeflateRect (nTabBorderSize - 2, nTabBorderSize - 2);
					}
					
					if (rectFrame.Width () > 0 && rectFrame.Height () > 0)
					{
						pDC->Draw3dRect (&rectFrame, clrDarkShadow, clrHighlight);
					}
				}
				else
				{
					rectFrame.DeflateRect (2, 2);
				}
			}
		}
	}

	if (m_bTopEdge && m_location == LOCATION_TOP)
	{
		pDC->SelectObject (&penDark);

		pDC->MoveTo (rectClient.left, m_rectTabsArea.bottom);
		pDC->LineTo (rectClient.left, rectClient.top);
		pDC->LineTo (rectClient.right - 1, rectClient.top);
		pDC->LineTo (rectClient.right - 1, m_rectTabsArea.bottom);
	}

	CFont* pOldFont = pDC->SelectObject (m_bFlat ?	&m_fntTabs : 
													&globalData.fontRegular);
	ASSERT(pOldFont != NULL);

	pDC->SetBkMode (TRANSPARENT);
	pDC->SetTextColor (globalData.clrBtnText);

	if (m_rectTabsArea.Width () > 5 && m_rectTabsArea.Height () > 5)
	{
		//-----------
		// Draw tabs:
		//-----------
		CRect rectClip = m_rectTabsArea;
		rectClip.InflateRect (1, nTabBorderSize);

		CRgn rgn;
		rgn.CreateRectRgnIndirect (rectClip);

		for (int i = m_iTabsNum - 1; i >= 0; i--)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);

			if (!pTab->m_bVisible)
				continue;

			m_iCurTab = i;

			if (i != m_iActiveTab)	// Draw active tab last
			{
				pDC->SelectClipRgn (&rgn);

				if (m_bFlat)
				{
					pDC->SelectObject (&penBlack);
					DrawFlatTab (pDC, pTab, FALSE);
				}
				else
				{
					Draw3DTab (pDC, pTab, FALSE);
				}
			}
		}

		if (m_iActiveTab >= 0)
		{
			//-----------------
			// Draw active tab:
			//-----------------
			pDC->SetTextColor (globalData.clrWindowText);

			CBCGTabInfo* pTabActive = (CBCGTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTabActive);

			m_iCurTab = m_iActiveTab;

			pDC->SelectClipRgn (&rgn);

			if (m_bFlat)
			{
				pDC->SelectObject (&m_brActiveTab);
				pDC->SelectObject (&m_fntTabsBold);
				pDC->SetTextColor (GetActiveTabTextColor ());
				pDC->SelectObject (&penBlack);

				DrawFlatTab (pDC, pTabActive, TRUE);

				//---------------------------------
				// Draw line bellow the active tab:
				//---------------------------------
				const int xLeft = max (	m_rectTabsArea.left + 1,
										pTabActive->m_rect.left + 1);

				if (pTabActive->m_rect.right > m_rectTabsArea.left + 1)
				{
					CPen penLight (PS_SOLID, 1, GetActiveTabColor ());
					pDC->SelectObject (&penLight);

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->MoveTo (xLeft, pTabActive->m_rect.top);
						pDC->LineTo (pTabActive->m_rect.right, pTabActive->m_rect.top);
					}
					else
					{
						pDC->MoveTo (xLeft, pTabActive->m_rect.bottom);
						pDC->LineTo (pTabActive->m_rect.right, pTabActive->m_rect.bottom);
					}

					pDC->SelectObject (pOldPen);
				}
			}
			else
			{
				Draw3DTab (pDC, pTabActive, TRUE);
			}
		}

		pDC->SelectClipRgn (NULL);
	}

	if (!m_rectTabSplitter.IsRectEmpty ())
	{
		pDC->FillRect (m_rectTabSplitter, pbrFace);

		CRect rectTabSplitter = m_rectTabSplitter;

		pDC->Draw3dRect (rectTabSplitter, clrDarkShadow, clrDark);
		rectTabSplitter.DeflateRect (1, 1);
		pDC->Draw3dRect (rectTabSplitter, clrHighlight, clrDark);
	}
	
	if (m_bFlat && m_nTabsHorzOffset > 0)
	{
		pDC->SelectObject (&penDark);

		const int xDivider = m_rectTabsArea.left - 1;

		if (m_location == LOCATION_BOTTOM)
		{
			pDC->MoveTo (xDivider, m_rectTabsArea.top + 1);
			pDC->LineTo (xDivider, m_rectTabsArea.bottom - 2);
		}
		else
		{
			pDC->MoveTo (xDivider, m_rectTabsArea.bottom);
			pDC->LineTo (xDivider, m_rectTabsArea.top + 2);
		}
	}

	pDC->SelectObject (pOldFont);
	pDC->SelectObject (pOldBrush);
	pDC->SelectObject (pOldPen);

	if (memDC.IsMemDC ())
	{
		dc.ExcludeClipRect (m_rectWndArea);
	}
}
//***************************************************************************************
void CBCGTabWnd::DrawInsertionMark (CDC* pDC, int left, int top, int bottom, int nHalfHeight)
{
	if (!IsFlatTab ())
	{
		return;
	}

	#define POINTS_NUM 3
	POINT pts [POINTS_NUM];

	if (GetLocation () == CBCGTabWnd::LOCATION_BOTTOM)
	{
		pts [0].x = left;
		pts [0].y = bottom;

		pts [1].x = left + nHalfHeight;
		pts [1].y = bottom;

		pts [2].x = left + nHalfHeight / 2;
		pts [2].y = top + nHalfHeight;
	}
	else
	{
		pts [0].x = left + nHalfHeight;
		pts [0].y = top;

		pts [1].x = left;
		pts [1].y = top;

		pts [2].x = left + nHalfHeight / 2;
		pts [2].y = bottom - nHalfHeight;
	}

   CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject (BLACK_BRUSH);

	pDC->Polygon (pts, POINTS_NUM);

	if (pOldBrush != NULL)
	{
		pDC->SelectObject (pOldBrush);
	}
}
//***************************************************************************************
void CBCGTabWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	int nTabsAreaWidth = cx - 4 * ::GetSystemMetrics (SM_CXVSCROLL) 
							- 2 * GetTabBorderSize ();

	if (nTabsAreaWidth <= MIN_SROLL_WIDTH)
	{
		m_nHorzScrollWidth = 0;
	}
	else if (nTabsAreaWidth / 2 > MIN_SROLL_WIDTH)
	{
		m_nHorzScrollWidth = nTabsAreaWidth / 2;
	}
	else
	{
		m_nHorzScrollWidth = nTabsAreaWidth; 
	}

	RecalcLayout ();
	SynchronizeScrollBar ();

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;

		EnsureVisible (m_iActiveTab);
	}
}
//***************************************************************************************
void CBCGTabWnd::AddTab (CWnd* pNewWnd, LPCTSTR lpszName, UINT uiImageId)
{
	InsertTab (pNewWnd, lpszName, -1, uiImageId);
}
//***************************************************************************************
void CBCGTabWnd::AddTab (CWnd* pTabWnd, UINT uiResTabLabel, UINT uiImageId)
{
	CString strLabel;
	strLabel.LoadString (uiResTabLabel);

	AddTab (pTabWnd, strLabel, uiImageId);
}
//***************************************************************************************
void CBCGTabWnd::InsertTab(CWnd* pNewWnd, LPCTSTR lpszTabLabel, int nInsertAt, UINT uiImageId)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pNewWnd);
	ASSERT (lpszTabLabel != NULL);

	if (m_bFlat)
	{
		ASSERT (uiImageId == (UINT) -1);
		uiImageId = (UINT) -1;
	}

	if (nInsertAt < 0 || nInsertAt > m_iTabsNum)
	{
		nInsertAt = m_iTabsNum;
	}

	CWnd* pActiveWnd = GetActiveWnd();

	m_arTabs.InsertAt (nInsertAt, new CBCGTabInfo (
		lpszTabLabel, uiImageId, pNewWnd, m_nNextTabID));

	m_iTabsNum++;

	if (!m_bFlat && m_ToolTip.GetSafeHwnd () != NULL)
	{
		CRect rectEmpty (0, 0, 0, 0);
		m_ToolTip.AddTool (this, lpszTabLabel, &rectEmpty, m_nNextTabID);
	}

	m_nNextTabID ++;
	RecalcLayout ();

	if (m_iTabsNum == 1)
	{
		//----------------------------------------
		// First tab automatically becames active:
		//----------------------------------------
		SetActiveTab (0);
	}
	else if (pNewWnd->GetSafeHwnd () != NULL)
	{
		if (m_iActiveTab == nInsertAt)
		{
			if (m_bHideInactiveWnd && pActiveWnd != NULL)
			{
				pActiveWnd->ShowWindow (SW_HIDE);
			}

			pNewWnd->ShowWindow (SW_SHOWNORMAL);
		}
		else if (m_bHideInactiveWnd)
		{
			pNewWnd->ShowWindow (SW_HIDE);
		}
	}

	if (!m_bHideInactiveWnd && pActiveWnd != NULL)
	{
		pActiveWnd->BringWindowToTop ();
	}
}
//***************************************************************************************
void CBCGTabWnd::InsertTab(CWnd* pNewWnd, UINT uiResTabLabel, int nInsertAt, UINT uiImageId)
{
	CString strLabel;
	strLabel.LoadString (uiResTabLabel);

	InsertTab (pNewWnd, strLabel, nInsertAt, uiImageId);
}
//***************************************************************************************
int CBCGTabWnd::GetVisibleTabsNum () const
{
	int	nCount	= 0;

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (pTab->m_bVisible)
		{
			nCount++;
		}
	}

	return nCount;
}
//***************************************************************************************
BOOL CBCGTabWnd::IsTabVisible(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("IsTabVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}
	
	const CBCGTabInfo* pTab = (const CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	return pTab->m_bVisible;
}
//***************************************************************************************
BOOL CBCGTabWnd::ShowTab(int iTab, BOOL bShow /*= TRUE*/, BOOL bRecalcLayout /*= TRUE*/)
{
	// save the old tab index:
	int iOldActiveTab = m_iActiveTab;

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("ShowTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (pTab->m_bVisible == bShow)
	{
		return TRUE;
	}

	int	nVisibleCount = GetVisibleTabsNum ();
	pTab->m_bVisible = bShow;

	int iActiveTab = (bShow ? m_iActiveTab : -1);

	if (!bShow)
	{
		//----------
		// Hide tab:
		//----------
		if (m_bHideInactiveWnd)
		{
			ASSERT_VALID (pTab->m_pWnd);
			pTab->m_pWnd->ShowWindow(SW_HIDE);
		}

		if (iTab == m_iActiveTab)
		{
			// Find the next best tab to be activated
			for (int i = m_iTabsNum - 1; i >= 0; --i)
			{
				CBCGTabInfo* pNextActiveTab = (CBCGTabInfo*) m_arTabs [i];
				ASSERT_VALID (pNextActiveTab);

				if (i < iTab && iActiveTab >= 0)
				{
					break;
				}

				if (pNextActiveTab->m_bVisible)
				{
					iActiveTab	= i;
				}
			}

			m_iActiveTab = -1;
		}
	}

	// If there was no tab visible, activate this first one
	if (bShow && nVisibleCount == 0)
	{
		iActiveTab	= iTab;
	}

	if (bRecalcLayout)
	{
		RecalcLayout ();
	}

	if (iActiveTab >= 0)
	{
		SetActiveTab (iActiveTab);
	}

	GetParent ()->SendMessage (BCGM_CHANGE_ACTIVE_TAB, m_iActiveTab, iOldActiveTab);
	return TRUE;
}
//***************************************************************************************
BOOL CBCGTabWnd::RemoveTab (int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("RemoveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (m_iTabsNum == 1)
	{
		RemoveAllTabs ();
		return TRUE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (!m_bFlat && m_ToolTip.GetSafeHwnd () != NULL)
	{
		m_ToolTip.DelTool (this, pTab->m_iTabID);
	}

	//----------------------------
	// Detach tab from collection:
	//----------------------------
	m_arTabs.RemoveAt (iTab);
	m_iTabsNum --;

	//-----------------------------------
	// Destroy tab window and delete tab:
	//-----------------------------------
	if (m_bAutoDestoyWindow)
	{
		ASSERT_VALID (pTab->m_pWnd);
		pTab->m_pWnd->DestroyWindow ();
	}

	delete pTab;

	int iActiveTab = m_iActiveTab;
	if (m_iActiveTab >= iTab)
	{
		iActiveTab = max (0, min (m_iTabsNum, m_iActiveTab - 1));
		m_iActiveTab = -1;
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style)
	{
		if (m_nFirstVisibleTab >= iTab)
		{
			m_nFirstVisibleTab = max (0, min (m_iTabsNum, m_nFirstVisibleTab - 1));
		}
	}

	RecalcLayout ();

	if ((m_bIsOneNoteStyle || m_bIsVS2005Style) && m_nTabsHorzOffset == 0)
	{
		m_nFirstVisibleTab = 0;
	}

	int iOldActiveTab = m_iActiveTab;
	SetActiveTab (iActiveTab);
	GetParent ()->SendMessage (BCGM_CHANGE_ACTIVE_TAB, m_iActiveTab, iOldActiveTab);

	return TRUE;
}
//***************************************************************************************
void CBCGTabWnd::RemoveAllTabs ()
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (!m_bFlat && m_ToolTip.GetSafeHwnd () != NULL)
		{
			m_ToolTip.DelTool (this, pTab->m_iTabID);
		}

		if (m_bAutoDestoyWindow)
		{
			pTab->m_pWnd->DestroyWindow ();
		}

		delete pTab;
	}

	if (!m_bFlat && m_ToolTip.GetSafeHwnd () != NULL)
	{
		ASSERT (m_ToolTip.GetToolCount () == 0);
	}

	m_arTabs.RemoveAll ();

	m_iTabsNum = 0;
	m_iActiveTab = -1;
	m_nNextTabID = 1;

	RecalcLayout ();
}
//***************************************************************************************
BOOL CBCGTabWnd::SetActiveTab (int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("SetActiveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (iTab >= m_arTabs.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bIsFirstTime = (m_iActiveTab == -1);

	if (m_iActiveTab == iTab)	// Already active, do nothing
	{
		return TRUE;
	}

	if (m_iActiveTab != -1 && m_bHideInactiveWnd)
	{
		//--------------------
		// Hide active window:
		//--------------------
		GetActiveWnd()->ShowWindow (SW_HIDE);
	}

	m_iActiveTab = iTab;
	
	//------------------------
	// Show new active window:
	//------------------------
	HideActiveWindowHorzScrollBar ();

	CWnd* pWndActive = GetActiveWnd ();
	if (pWndActive == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (pWndActive);

	pWndActive->ShowWindow (SW_SHOW);
	if (!m_bHideInactiveWnd)
	{
		pWndActive->BringWindowToTop ();
	}

	if (m_bAutoSizeWindow)
	{
		//----------------------------------------------------------------------
		// Small trick: to adjust active window scroll sizes, I should change an
		// active window size twice (+1 pixel and -1 pixel):
		//----------------------------------------------------------------------
		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width () + 1, m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		pWndActive->SetWindowPos (NULL,
				-1, -1,
				m_rectWndArea.Width (), m_rectWndArea.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	EnsureVisible (m_iActiveTab);

	if (m_bFlat)
	{
		SynchronizeScrollBar ();
	}

	//-------------
	// Redraw tabs:
	//-------------
	RedrawWindow (NULL, NULL,
		RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	if (!bIsFirstTime)
	{
		CView* pActiveView = DYNAMIC_DOWNCAST (CView, pWndActive);
		if (pActiveView != NULL)
		{
			CFrameWnd* pFrame = pActiveView->GetParentFrame ();
			if (pFrame != NULL && pFrame->IsChild (pActiveView))
			{
				ASSERT_VALID (pFrame);
				pFrame->SetActiveView (pActiveView);
			}
			else
			{
				pWndActive->SetFocus ();
			}
		}
		else
		{
			pWndActive->SetFocus ();
		}
		////
	}

	return TRUE;
}
//***************************************************************************************
int CBCGTabWnd::GetTabFromPoint (CPoint& pt) const
{
	ASSERT_VALID (this);

	if (!m_rectTabsArea.PtInRect (pt))
	{
		return -1;
	}

	if (!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);

			if (pTab->m_bVisible && pTab->m_rect.PtInRect (pt))
			{
				return i;
			}
		}

		return -1;
	}

	//------------------------
	// Check active tab first:
	//------------------------
	if (m_iActiveTab >= 0)
	{
		CBCGTabInfo* pActiveTab = (CBCGTabInfo*) m_arTabs [m_iActiveTab];
		ASSERT_VALID (pActiveTab);

		CRect rectTab = pActiveTab->m_rect;

		if (rectTab.PtInRect (pt))
		{
			if (m_iActiveTab > 0 && pt.x < rectTab.left + rectTab.Height ())
			{
				const int x = pt.x - rectTab.left;
				const int y = pt.y - rectTab.top;

				if (x * x + y * y < rectTab.Height () * rectTab.Height () / 2)
				{
					for (int i = m_iActiveTab - 1; i >= 0; i--)
					{
						CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
						ASSERT_VALID (pTab);

						if (pTab->m_bVisible)
						{
							return i;
						}
					}
				}
			}

			return m_iActiveTab;
		}
	}

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && pTab->m_rect.PtInRect (pt))
		{
			return i;
		}
	}

	return -1;
}
//***************************************************************************************
int CBCGTabWnd::GetTabFromHwnd (HWND hwnd) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_pWnd != NULL && pTab->m_pWnd->GetSafeHwnd () == hwnd)
		{
			return i;
		}
	}

	return -1;
}
//***************************************************************************************
void CBCGTabWnd::AdjustTabs ()
{
	m_bHiddenDocuments = FALSE;

	int	nVisibleTabsNum	= GetVisibleTabsNum ();
	if (nVisibleTabsNum == 0 || GetTabsHeight () == 0)
	{
		return;
	}

	if (m_bHideSingleTab && nVisibleTabsNum <= 1)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			pTab->m_rect.SetRectEmpty ();
		}

		return;
	}

	if (m_bAutoHideScroll)
	{
		BOOL bIsScrollVisible = m_btnScrollLeft.IsWindowVisible ();
		if (!bIsScrollVisible)
		{
			m_rectTabsArea.left = GetTabBorderSize ();
		}
	}

	//-------------------------
	// Define tab's full width:
	//-------------------------
	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (m_bFlat ? 
		&m_fntTabsBold : &globalData.fontRegular);
	ASSERT(pOldFont != NULL);

	m_nTabsTotalWidth = 0;

	//----------------------------------------------
	// First, try set all tabs in its original size:
	//----------------------------------------------
	int x = m_rectTabsArea.left - m_nTabsHorzOffset;
	int i = 0;

	for (;i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		CSize sizeImage (0, 0);
		if (pTab->m_uiIcon != (UINT)-1)
		{
			sizeImage = m_sizeImage;
		}

		int nExtraWidth = 0;

		if (pTab->m_bVisible)
		{
			pTab->m_nFullWidth = sizeImage.cx + TAB_IMAGE_MARGIN +
				(pTab->m_bIconOnly ? 
					0 : dc.GetTextExtent (pTab->m_strText).cx) + 2 * TAB_TEXT_MARGIN;

			if (m_bLeftRightRounded)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height () / 2;
				nExtraWidth = m_rectTabsArea.Height () / 2;
			}
			else if (m_bIsOneNoteStyle)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height () + 2 * TAB_IMAGE_MARGIN;
				nExtraWidth = m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1;
			}
			else if (m_bIsVS2005Style)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height () - TAB_IMAGE_MARGIN;
				nExtraWidth = m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1;
			}
		}
		else
		{
			pTab->m_nFullWidth = 0;
		}

		pTab->m_rect = CRect (CPoint (x, m_rectTabsArea.top),
						CSize (pTab->m_nFullWidth, m_rectTabsArea.Height () - 2));
		
		if (!pTab->m_bVisible)
		{
			if (m_ToolTip.GetSafeHwnd () != NULL)
			{
				m_ToolTip.SetToolRect (this, pTab->m_iTabID, CRect (0, 0, 0, 0));
			}
			continue;
		}
		
		if (m_location == LOCATION_TOP)
		{
			pTab->m_rect.OffsetRect (0, 2);
		}

		if (m_bTabDocumentsMenu && pTab->m_rect.right > m_rectTabsArea.right)
		{
			BOOL bHideTab = TRUE;

			if (i == m_iActiveTab && i == 0)
			{
				int nWidth = m_rectTabsArea.right - pTab->m_rect.left;

				if (nWidth >= nExtraWidth + 2 * TAB_TEXT_MARGIN)
				{
					pTab->m_rect.right = m_rectTabsArea.right;
					bHideTab = FALSE;
				}
			}

			if (bHideTab)
			{
				pTab->m_nFullWidth = 0;
				pTab->m_rect.SetRectEmpty ();
				m_bHiddenDocuments = TRUE;
				continue;
			}
		}
			
		if (m_ToolTip.GetSafeHwnd () != NULL)
		{
			BOOL bShowTooltip = pTab->m_bAlwaysShowToolTip;

			if (pTab->m_rect.left < m_rectTabsArea.left ||
				pTab->m_rect.right > m_rectTabsArea.right)
			{
				bShowTooltip = TRUE;
			}

			m_ToolTip.SetToolRect (this, pTab->m_iTabID, 
				bShowTooltip ? pTab->m_rect : CRect (0, 0, 0, 0));
		}

		x += pTab->m_rect.Width () + 1 - nExtraWidth;
		m_nTabsTotalWidth += pTab->m_rect.Width () + 1;

		if (i > 0)
		{
			m_nTabsTotalWidth -= nExtraWidth;
		}

		if (m_bFlat)
		{
			//--------------------------------------------
			// In the flat mode tab is overlapped by next:
			//--------------------------------------------
			pTab->m_rect.right += m_nTabsHeight / 2;
		}
	}

	if (m_bScroll || x < m_rectTabsArea.right)
	{
		m_nTabsTotalWidth += m_nTabsHeight / 2;
	}
	else
	{
		//-----------------------------------------
		// Not enouth space to show the whole text.
		//-----------------------------------------
		int nTabsWidth = m_rectTabsArea.Width ();
		int nTabWidth = nTabsWidth / nVisibleTabsNum - 1;

		if (m_bLeftRightRounded)
		{
			nTabWidth = max (
				m_sizeImage.cx + m_rectTabsArea.Height () / 2,
				(nTabsWidth - m_rectTabsArea.Height () / 3) / nVisibleTabsNum);
		}

		//------------------------------------
		// May be it's too wide for some tabs?
		//------------------------------------
		int nRest = 0;
		int nCutTabsNum = nVisibleTabsNum;

		for (i = 0; i < m_iTabsNum; i ++)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			if (!pTab->m_bVisible)
			{
				continue;
			}
			
			if (pTab->m_nFullWidth < nTabWidth)
			{
				nRest += nTabWidth - pTab->m_nFullWidth;
				nCutTabsNum --;
			}
		}

		if (nCutTabsNum > 0)
		{
			nTabWidth += nRest / nCutTabsNum;

			//----------------------------------
			// Last pass: set actual rectangles:
			//----------------------------------
			x = m_rectTabsArea.left;
			for (i = 0; i < m_iTabsNum; i ++)
			{
				CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
				ASSERT_VALID (pTab);
				
				if (!pTab->m_bVisible)
				{
					m_ToolTip.SetToolRect (this, pTab->m_iTabID, CRect (0, 0, 0, 0));
					continue;
				}
				
				CSize sizeImage (0, 0);
				if (pTab->m_uiIcon != (UINT)-1)
				{
					sizeImage = m_sizeImage;
				}

				BOOL bIsTrucncated = pTab->m_nFullWidth > nTabWidth;
				int nCurrTabWidth = (bIsTrucncated) ? nTabWidth : pTab->m_nFullWidth;

				if (nTabWidth < sizeImage.cx + TAB_IMAGE_MARGIN)
				{
					// Too narrow!
					nCurrTabWidth = (m_rectTabsArea.Width () + m_nTabBorderSize * 2) / nVisibleTabsNum;
				}
				else
				{
					if (pTab->m_strText.IsEmpty () || pTab->m_bIconOnly)
					{
						nCurrTabWidth = sizeImage.cx + 2 * CBCGTabWnd::TAB_TEXT_MARGIN;
					}
				}

				if (m_bLeftRightRounded)
				{
					nCurrTabWidth += m_rectTabsArea.Height () / 2 - 1;
				}

				pTab->m_rect = CRect (CPoint (x, m_rectTabsArea.top),
								CSize (nCurrTabWidth, m_rectTabsArea.Height () - 2));

				if (!m_bFlat)
				{
					if (m_location == LOCATION_TOP)
					{
						pTab->m_rect.OffsetRect (0, 2);
					}

					if (m_ToolTip.GetSafeHwnd () != NULL)
					{
						m_ToolTip.SetToolRect (this, pTab->m_iTabID, 
							bIsTrucncated || pTab->m_bAlwaysShowToolTip ? 
								pTab->m_rect : CRect (0, 0, 0, 0));
					}
				}

				x += nCurrTabWidth;
				if (m_bLeftRightRounded)
				{
					x -= m_rectTabsArea.Height () / 2;
				}

				if (nRest > 0)
				{
					x ++;
				}
			}
		}
	}

	dc.SelectObject(pOldFont);
}
//***************************************************************************************
void CBCGTabWnd::Draw3DTab (CDC* pDC, CBCGTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID (pTab);
	ASSERT_VALID (pDC);

	if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		&& pTab->m_rect.left < m_rectTabsArea.left)
	{
		return;
	}

	if (pTab->m_bVisible)
	{
		CRect rectInter;
		if (m_rectCurrClip.IsRectEmpty () ||
			rectInter.IntersectRect (pTab->m_rect, m_rectCurrClip))
		{
			CBCGVisualManager::GetInstance ()->OnDrawTab (
				pDC, pTab->m_rect, m_iCurTab, bActive, this);
		}
	}
}
//***************************************************************************************
void CBCGTabWnd::DrawFlatTab (CDC* pDC, CBCGTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID (pTab);
	ASSERT_VALID (pDC);

	if (pTab->m_bVisible)
	{
		CBCGVisualManager::GetInstance ()->OnDrawTab (
			pDC, pTab->m_rect, m_iCurTab, bActive, this);
	}
}
//***************************************************************************************
void CBCGTabWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	if (m_rectTabSplitter.PtInRect (point))
	{
		m_bTrackSplitter = TRUE;
		SetCapture ();
		return;
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (!pTab->m_bVisible)
			continue;
		
		if (pTab->m_rect.PtInRect (point))
		{
			if (i != m_iActiveTab && m_rectTabsArea.PtInRect (point))
			{
				m_iHighlighted = -1;
				ReleaseCapture ();

				int iOldActiveTab = m_iActiveTab;
				SetActiveTab (i);

				pWndParent->SendMessage (BCGM_CHANGE_ACTIVE_TAB, i, iOldActiveTab);
			}

			if (m_bDragDrop && m_rectTabsArea.PtInRect (point))
			{
				SetCapture();
				m_bDragging = TRUE;
				m_iDropTarget = m_iDragTab = i;
			}

			return;
		}
	}

	CWnd* pWndTarget = FindTargetWnd (point);
	if (pWndTarget == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndTarget);

	MapWindowPoints (pWndTarget, &point, 1);
	pWndTarget->SendMessage (WM_LBUTTONDOWN, nFlags, 
							MAKELPARAM (point.x, point.y));
}
//***************************************************************************************
void CBCGTabWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDblClk(nFlags, point);

	int iTab = GetTabFromPoint (point);
	if (iTab >= 0)
	{
		if (BeginLabelEdit (iTab))
		{
			return;
		}
	}

	CWnd* pWndTarget = FindTargetWnd (point);
	if (pWndTarget == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndTarget);

	MapWindowPoints (pWndTarget, &point, 1);
	pWndTarget->SendMessage (WM_LBUTTONDBLCLK, nFlags, 
							MAKELPARAM (point.x, point.y));
}
//****************************************************************************************
int CBCGTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectDummy (0, 0, 0, 0);

	if (m_bScroll)
	{
		//-----------------------
		// Create scroll buttons:
		//-----------------------
		if (m_bFlat)
		{
			m_btnScrollFirst.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollFirst.SetStdImage (CMenuImages::IdArowFirst);
			m_btnScrollFirst.m_bDrawFocus = FALSE;
			m_btnScrollFirst.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail (m_btnScrollFirst.GetSafeHwnd ());
		}

		m_btnScrollLeft.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollLeft.SetStdImage (CMenuImages::IdArowRightLarge,
			CMenuImages::IdArowRightLargeDsbl);
		m_btnScrollLeft.m_bDrawFocus = FALSE;
		m_btnScrollLeft.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollLeft.SetAutorepeatMode (50);
		}

		m_lstButtons.AddTail (m_btnScrollLeft.GetSafeHwnd ());

		m_btnScrollRight.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollRight.SetStdImage (CMenuImages::IdArowLeftLarge,
			CMenuImages::IdArowLeftLargeDsbl);
		m_btnScrollRight.m_bDrawFocus = FALSE;
		m_btnScrollRight.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollRight.SetAutorepeatMode (50);
		}

		m_lstButtons.AddTail (m_btnScrollRight.GetSafeHwnd ());

		if (m_bFlat)
		{
			m_btnScrollLast.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollLast.SetStdImage (CMenuImages::IdArowLast);
			m_btnScrollLast.m_bDrawFocus = FALSE;
			m_btnScrollLast.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail (m_btnScrollLast.GetSafeHwnd ());
		}

		m_btnClose.Create (_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnClose.SetStdImage (CMenuImages::IdClose);
		m_btnClose.m_bDrawFocus = FALSE;
		m_btnClose.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
		m_lstButtons.AddTail (m_btnClose.GetSafeHwnd ());

		if (!m_bFlat && m_bScroll)
		{
			m_btnClose.SetTooltip (_T("Close"));
			m_btnScrollLeft.SetTooltip (_T("Scroll Left"));
			m_btnScrollRight.SetTooltip (_T("Scroll Right"));
		}
	}

	if (m_bSharedScroll)
	{
		m_wndScrollWnd.Create (WS_CHILD | WS_VISIBLE | SBS_HORZ, rectDummy,
			this, (UINT) -1);
	}

	if (m_bFlat)
	{
		//---------------------
		// Create active brush:
		//---------------------
		m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());
	}
	else if (!m_bScroll)
	{
		//---------------------------------------
		// Text may be truncated. Create tooltip.
		//---------------------------------------
		m_ToolTip.Create (this, TTS_ALWAYSTIP);
		m_ToolTip.Activate (TRUE);
		if(globalData.m_nMaxToolTipWidth != -1)
		{
			m_ToolTip.SetMaxTipWidth(globalData.m_nMaxToolTipWidth);
		}

		m_ToolTip.SetWindowPos (&wndTop, -1, -1, -1, -1,    
								SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	SetTabsHeight ();
	return 0;
}
//***************************************************************************************
BOOL CBCGTabWnd::SetImageList (UINT uiID, int cx, COLORREF clrTransp)
{
	if (m_bFlat)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_Images.GetSafeHandle () != NULL)
	{
		m_Images.DeleteImageList ();
		m_sizeImage = CSize (0, 0);
	}

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiID))
	{
		TRACE(_T("CBCGTabWnd::SetImageList Can't load bitmap: %x\n"), uiID);
		return FALSE;
	}

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransp == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_Images.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	m_Images.Add (&bmp, clrTransp);

	m_sizeImage = CSize (cx, bmpObj.bmHeight);

	SetTabsHeight ();
	return TRUE;
}
//***************************************************************************************
BOOL CBCGTabWnd::SetImageList (HIMAGELIST hImageList)
{
	if (m_bFlat)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (hImageList != NULL);
	ASSERT (m_Images.GetSafeHandle () == NULL);

	CImageList* pImageList = CImageList::FromHandle (hImageList);
	if (pImageList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	IMAGEINFO info;
	pImageList->GetImageInfo (0, &info);

	CRect rectImage = info.rcImage;
	m_sizeImage = rectImage.Size ();

	m_hImageList = hImageList;

	SetTabsHeight ();
	return TRUE;
}
//***************************************************************************************
BOOL CBCGTabWnd::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bTransparent && m_iTabsNum == 0)
	{
		CRect rectClient;
		GetClientRect (rectClient);
		pDC->FillRect (rectClient, &globalData.brBtnFace);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGTabWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (m_pInPlaceEdit != NULL)
	{
		if (pMsg->message >= WM_KEYFIRST &&
			pMsg->message <= WM_KEYLAST)
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				if (!RenameTab ())
				{
					MessageBeep ((UINT)-1);
					return TRUE;
				}
				// Slide down!

			case VK_ESCAPE:
				m_pInPlaceEdit->DestroyWindow ();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				ReleaseCapture ();
				break;

			default:
				return FALSE;
			}

			return TRUE;
		}
		else if (pMsg->message >= WM_MOUSEFIRST &&
				 pMsg->message <= WM_MOUSELAST)
		{
			CRect rectEdit;
			m_pInPlaceEdit->GetClientRect (rectEdit);
			m_pInPlaceEdit->MapWindowPoints (this, rectEdit);

			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			if (rectEdit.PtInRect (ptCursor))
			{
				m_pInPlaceEdit->SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
			}
			else if (pMsg->message != WM_MOUSEMOVE)
			{
				if (RenameTab ())
				{
					m_pInPlaceEdit->DestroyWindow ();
					delete m_pInPlaceEdit;
					m_pInPlaceEdit = NULL;
					ReleaseCapture ();
				}
				else
				{
					MessageBeep ((UINT)-1);
				}
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	int iOldActiveTab = m_iActiveTab;

   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_iActiveTab != -1 &&
			::GetAsyncKeyState (VK_CONTROL) & 0x8000)	// Ctrl is pressed
		{
			switch (pMsg->wParam)
			{
			case VK_NEXT:
				{
					for (int i = m_iActiveTab + 1; i < m_iActiveTab + m_iTabsNum; ++i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible (iTabIndex))
						{
							SetActiveTab (iTabIndex);
							GetActiveWnd ()->SetFocus ();
							GetParent ()->SendMessage (BCGM_CHANGE_ACTIVE_TAB, m_iActiveTab, iOldActiveTab);
							break;
						}
					}
				}
				return TRUE;

			case VK_PRIOR:
				{
					for (int i = m_iActiveTab - 1 + m_iTabsNum; i > m_iActiveTab; --i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible (iTabIndex))
						{
							SetActiveTab (iTabIndex);
							GetActiveWnd ()->SetFocus ();
							GetParent ()->SendMessage (BCGM_CHANGE_ACTIVE_TAB, m_iActiveTab, iOldActiveTab);
							break;
						}
					}
				}
				return TRUE;
			}
		}
		// Continue....

	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_ToolTip.GetSafeHwnd () != NULL)
		{
			m_ToolTip.RelayEvent(pMsg);
		}
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}
//****************************************************************************************
CWnd* CBCGTabWnd::GetTabWnd (int iTab) const
{
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
		ASSERT_VALID (pTab);

		return pTab->m_pWnd;
	}
	else
	{
		return NULL;
	}
}
//******************************************************************************************
CWnd* CBCGTabWnd::GetActiveWnd () const
{
	return m_iActiveTab == -1 ? 
		NULL : 
		((CBCGTabInfo*) m_arTabs [m_iActiveTab])->m_pWnd;
}
//******************************************************************************************
void CBCGTabWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!m_bFlat)
	{
		CWnd::OnHScroll (nSBCode, nPos, pScrollBar);
		return;
	}

	if (pScrollBar->GetSafeHwnd () == m_wndScrollWnd.GetSafeHwnd ())
	{
		static BOOL bInsideScroll = FALSE;

		if (m_iActiveTab != -1 && !bInsideScroll)
		{
			CWnd* pWndActive = GetActiveWnd ();
			ASSERT_VALID (pWndActive);

			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTab);

			WPARAM wParam = MAKEWPARAM (nSBCode, nPos);

			//----------------------------------
			// Pass scroll to the active window:
			//----------------------------------
			bInsideScroll = TRUE;

			if (pTab->m_bIsListView &&
				(LOBYTE (nSBCode) == SB_THUMBPOSITION ||
				LOBYTE (nSBCode) == SB_THUMBTRACK))
			{
				int dx = nPos - pWndActive->GetScrollPos (SB_HORZ);
				pWndActive->SendMessage (LVM_SCROLL, dx, 0);
			}

			pWndActive->SendMessage (WM_HSCROLL, wParam, 0);

			bInsideScroll = FALSE;

			m_wndScrollWnd.SetScrollPos (pWndActive->GetScrollPos (SB_HORZ));

			HideActiveWindowHorzScrollBar ();
			GetParent ()->SendMessage (BCGM_ON_HSCROLL, wParam);
		}

		return;
	}

	CWnd::OnHScroll (nSBCode, nPos, pScrollBar);
}
//******************************************************************************************
CWnd* CBCGTabWnd::FindTargetWnd (const CPoint& point)
{
	if (point.y < m_nTabsHeight)
	{
		return NULL;
	}

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (!pTab->m_bVisible)
			continue;
		
		if (pTab->m_rect.PtInRect (point))
		{
			return NULL;
		}
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	return pWndParent;
}
//************************************************************************************
void CBCGTabWnd::AdjustTabsScroll ()
{
	ASSERT_VALID (this);

	if (!m_bScroll)
	{
		m_nTabsHorzOffset = 0;
		return;
	}

	if (GetVisibleTabsNum () == 0)
	{
		if (m_bAutoHideScroll)
		{
			for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
			{
				HWND hWndButton = m_lstButtons.GetNext (pos);
				ASSERT (hWndButton != NULL);
				
				::ShowWindow (hWndButton, SW_HIDE);
			}
		}

		m_nTabsHorzOffsetMax = 0;
		m_nTabsHorzOffset = 0;
		return;
	}

	int nPrevHorzOffset = m_nTabsHorzOffset;

	m_nTabsHorzOffsetMax = max (0, m_nTabsTotalWidth - m_rectTabsArea.Width ());

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = max (0, m_nTabsHorzOffset);
	}
	else
	{
		m_nTabsHorzOffset = min (max (0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
	}


	BOOL bIsScrollLeft = TRUE;
	if (m_nTabsHorzOffset <= 0)
	{
		m_nTabsHorzOffset = 0;
		bIsScrollLeft = FALSE;
	}

	BOOL bIsScrollRight = TRUE;
	if (m_nTabsHorzOffset >= m_nTabsHorzOffsetMax)
	{
		m_nTabsHorzOffset = m_nTabsHorzOffsetMax;
		bIsScrollRight = FALSE;
	}

	if (m_bAutoHideScroll)
	{
		BOOL bShowScroll = bIsScrollLeft || bIsScrollRight;
		for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
		{
			HWND hWndButton = m_lstButtons.GetNext (pos);
			ASSERT (hWndButton != NULL);
			
			::ShowWindow (hWndButton, bShowScroll ? SW_SHOWNOACTIVATE : SW_HIDE);
		}
	}

	if (nPrevHorzOffset != m_nTabsHorzOffset)
	{
		AdjustTabs ();
		InvalidateRect (m_rectTabsArea);
		UpdateWindow ();
	}

	UpdateScrollButtonsState ();
}
//*************************************************************************************
void CBCGTabWnd::RecalcLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (this);

	int nTabsHeight = GetTabsHeight ();
	const int nTabBorderSize = GetTabBorderSize ();
	int nVisiableTabs = GetVisibleTabsNum ();

	BOOL bHideTabs =	(m_bHideSingleTab && nVisiableTabs <= 1) ||
						(m_bHideNoTabs && nVisiableTabs == 0);

	CRect rectClient;
	GetClientRect (rectClient);

	m_rectTabsArea = rectClient;
	m_rectTabsArea.DeflateRect (2, 0);

	int nScrollBtnWidth = 0;
	int nButtons = 0;
	int nButtonsWidth = 0;
	int nButtonsHeight = 0;
	int nButtonMargin = 0;

	if (m_bScroll)
	{
		nScrollBtnWidth = min (nTabsHeight - 4 , CMenuImages::Size ().cx + 6);
		
		nButtons = (int) m_lstButtons.GetCount ();
		if (!m_bCloseBtn)
		{
			nButtons--;
		}

		if (m_bTabDocumentsMenu)
		{
			nButtons--;
		}

		nButtonMargin = 3;
		nButtonsWidth = bHideTabs ? 0 : (nScrollBtnWidth + nButtonMargin) * nButtons;
	}

	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.bottom -= nTabBorderSize - 1;
			}

			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.top += nTabBorderSize - 1;
			}

			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		m_rectTabsArea.left += nButtonsWidth + 1;
		m_rectTabsArea.right--;

		nButtonsHeight = m_rectTabsArea.Height ();

		if (m_rectTabsArea.Height () + nTabBorderSize > rectClient.Height ())
		{
			nButtonsHeight = 0;
			m_rectTabsArea.left = 0;
			m_rectTabsArea.right = 0;
		}

		int y = m_rectTabsArea.top;

		if (nButtonsHeight != 0)
		{
			y += max (0, (nButtonsHeight - nScrollBtnWidth) / 2);
			nButtonsHeight = nScrollBtnWidth;
		}

		// Reposition scroll butons:
		ReposButtons (	CPoint (rectClient.left + nTabBorderSize + 1, y), 
						CSize (nScrollBtnWidth, nButtonsHeight),
						bHideTabs, nButtonMargin);
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		if (m_bScroll)
		{
			m_rectTabsArea.right -= nButtonsWidth;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && !m_bTabDocumentsMenu)
			{
				m_rectTabsArea.OffsetRect (nScrollBtnWidth, 0);
			}

			// Reposition scroll butons:
			ReposButtons (	
				CPoint (m_rectTabsArea.right + 1, m_rectTabsArea.CenterPoint ().y - nScrollBtnWidth / 2),
				CSize (nScrollBtnWidth, nScrollBtnWidth), bHideTabs, nButtonMargin);
		}
	}

	m_rectWndArea = rectClient;
	m_nScrollBarRight = m_rectTabsArea.right - ::GetSystemMetrics (SM_CXVSCROLL);

	if (nTabBorderSize > 0)
	{
		m_rectWndArea.DeflateRect (nTabBorderSize + 1, nTabBorderSize + 1);
	}
	
	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.top++;
				m_rectWndArea.left++;
			}
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.bottom--;
				m_rectWndArea.left++;
			}
		}
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top - nTabBorderSize;
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize;
		}
	}

	if (m_bAutoSizeWindow)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);
			
			if (pTab->m_bVisible && pTab->m_pWnd->GetSafeHwnd () != NULL)
			{
				pTab->m_pWnd->SetWindowPos (NULL,
					m_rectWndArea.left, m_rectWndArea.top,
					m_rectWndArea.Width (), m_rectWndArea.Height (),
					SWP_NOACTIVATE | SWP_NOZORDER);
			}
		}
	}

	AdjustWndScroll ();
	AdjustTabs ();
	AdjustTabsScroll ();

	RedrawWindow (NULL, NULL,
		RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
}
//*************************************************************************************
void CBCGTabWnd::AdjustWndScroll ()
{
	ASSERT_VALID (this);

	if (!m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID (this);

	CRect rectScroll = m_rectTabsArea;

	int nVisiableTabs = GetVisibleTabsNum ();

	BOOL bHideTabs =	(m_bHideSingleTab && nVisiableTabs <= 1) ||
						(m_bHideNoTabs && nVisiableTabs == 0);

	if (!bHideTabs)
	{
		if (m_nHorzScrollWidth >= MIN_SROLL_WIDTH)
		{
			rectScroll.right = m_nScrollBarRight;
			rectScroll.left = rectScroll.right - m_nHorzScrollWidth;

			if (m_location == LOCATION_BOTTOM)
			{
				rectScroll.bottom -= 2;
			}
			else
			{
				rectScroll.top += 2;
			}

			m_rectTabSplitter = rectScroll;

			if (m_location == LOCATION_BOTTOM)
			{
				m_rectTabSplitter.top ++;
			}

			m_rectTabSplitter.right = rectScroll.left;
			m_rectTabSplitter.left = m_rectTabSplitter.right - SPLITTER_WIDTH;

			CRect rectLastBtn;
			m_btnScrollLast.GetWindowRect (rectLastBtn);
			ScreenToClient (rectLastBtn);

			if (m_rectTabSplitter.left < rectLastBtn.right)
			{
				m_rectTabSplitter.SetRectEmpty ();
			}
			else
			{
				m_rectTabsArea.right = m_rectTabSplitter.left;
			}

			if (rectScroll.left < rectLastBtn.right)
			{
				rectScroll.SetRectEmpty ();
			}
		}
		else
		{
			rectScroll.SetRectEmpty ();
			m_rectTabSplitter.SetRectEmpty ();
		}
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			rectScroll.bottom -= 2;
		}
		else
		{
			rectScroll.top += 2;
		}

		m_rectTabSplitter.SetRectEmpty ();
	}

	m_wndScrollWnd.SetWindowPos (NULL,
		rectScroll.left, rectScroll.top,
		rectScroll.Width (), rectScroll.Height (),
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
}
//***************************************************************************************
BOOL CBCGTabWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	ASSERT_VALID (this);

	if (m_pInPlaceEdit != NULL)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);

		CRect rectEdit;
		m_pInPlaceEdit->GetWindowRect (rectEdit);

		if (rectEdit.PtInRect (ptCursor))
		{
			::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
			return TRUE;
		}
	}
	else if (m_bFlat && !m_rectTabSplitter.IsRectEmpty ())
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (m_rectTabSplitter.PtInRect (ptCursor))
		{
			::SetCursor (globalData.m_hcurStretch);
			return TRUE;
		}
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
//***************************************************************************************
void CBCGTabWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	if (m_bTrackSplitter)
	{
		m_bTrackSplitter = FALSE;
		ReleaseCapture ();
	}

	if (m_bDragging)
	{
		KillTimer (nScrollLeftEventId);
		KillTimer (nScrollRightEventId);
		ReleaseCapture();

		if (m_iDropTarget != m_iDragTab &&
			GetParent()->SendMessage (BCGM_ON_BEFOREMOVE_TAB, m_iDragTab, m_iDropTarget) == 0)
		{
			CBCGTabInfo* pTab = (CBCGTabInfo*)m_arTabs[m_iDragTab];
			m_arTabs.InsertAt (m_iDropTarget, pTab);
			
			// now pTab is there twice, so let's remove the old one
			// if m_iDragTab < m_iDropTarget, remove at m_iDragTab
			// if m_iDragTab > m_iDropTarget, remove at m_iDragTab+1
			if (m_iDragTab < m_iDropTarget)
			{
				m_arTabs.RemoveAt (m_iDragTab);
				m_iActiveTab = m_iDropTarget-1;
			}
			else
			{
				m_arTabs.RemoveAt (m_iDragTab+1);
				m_iActiveTab = m_iDropTarget;
			}

			GetParent ()->SendMessage (BCGM_ON_MOVE_TAB, m_iDragTab, m_iDropTarget);
			
			AdjustTabs ();
		}

		m_bDragging = FALSE;

		InvalidateRect (m_rectTabsArea);
		UpdateWindow ();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}
//***************************************************************************************
void CBCGTabWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	CWnd::OnMouseMove (nFlags, point);

	if (m_bTrackSplitter)
	{
		int nSplitterLeftPrev = m_rectTabSplitter.left;

		m_nHorzScrollWidth = min (
			m_nScrollBarRight - m_rectTabsArea.left - SPLITTER_WIDTH, 
			m_nScrollBarRight - point.x);

		m_nHorzScrollWidth = max (MIN_SROLL_WIDTH, m_nHorzScrollWidth);
		AdjustWndScroll ();

		if (m_rectTabSplitter.left > nSplitterLeftPrev)
		{
			CRect rect = m_rectTabSplitter;
			rect.left = nSplitterLeftPrev - 20;
			rect.right = m_rectTabSplitter.left;
			rect.InflateRect (0, GetTabBorderSize () + 1);

			InvalidateRect (rect);
		}

		CRect rectTabSplitter = m_rectTabSplitter;
		rectTabSplitter.InflateRect (0, GetTabBorderSize ());

		InvalidateRect (rectTabSplitter);
		UpdateWindow ();
		AdjustTabsScroll ();

		return;
	}
	
	if (m_bDragging)
	{
		if (point.x < m_rectTabsArea.left)
		{
			// set a timer to scroll to the left
			KillTimer (nScrollRightEventId);
			SetTimer (nScrollLeftEventId, nScrollDelay, NULL);
		}
		else if (point.x > m_rectTabsArea.right)
		{
			// set a timer to scroll to the left
			KillTimer (nScrollLeftEventId);
			SetTimer (nScrollRightEventId, nScrollDelay, NULL);
		}
		else
		{
			KillTimer (nScrollLeftEventId);
			KillTimer (nScrollRightEventId);
			
			for (int i = 0; i < m_iTabsNum; i ++)
			{
				CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
				ASSERT_VALID (pTab);
				
				if (!pTab->m_bVisible)
					continue;
				
				if (point.x >= pTab->m_rect.left && 
					point.x <= pTab->m_rect.left + pTab->m_rect.Width()/2 &&
					m_iDropTarget != i)
				{
					m_iDropTarget = i;
					InvalidateRect (m_rectTabsArea);
					UpdateWindow ();
					break;
				}
				else if (point.x >= pTab->m_rect.left + pTab->m_rect.Width()/2 && 
					point.x <= pTab->m_rect.right &&
					m_iDropTarget != i+1)
				{
					m_iDropTarget = i+1;
					InvalidateRect (m_rectTabsArea);
					UpdateWindow ();
					break;
				}
			}
		}

		return;
	}

	if (!m_bFlat)
	{
		if (CBCGVisualManager::GetInstance ()->AlwaysHighlight3DTabs ())
		{
			m_bHighLightTabs = TRUE;
		}
		else if (m_bIsOneNoteStyle &&
			!CBCGVisualManager::GetInstance ()->IsHighlightOneNoteTabs ())
		{
			m_bHighLightTabs = FALSE;
		}
	}

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = GetTabFromPoint (point);

	if (m_iHighlighted != iPrevHighlighted && m_bHighLightTabs)
	{
		if (iPrevHighlighted < 0)
		{
			if (m_iHighlighted >= 0)
			{
				SetCapture ();
			}
		}
		else
		{
			if (m_iHighlighted < 0)
			{
				ReleaseCapture ();
			}
		}

		InvalidateTab (m_iHighlighted);
		InvalidateTab (iPrevHighlighted);
	}
}
//***************************************************************************************
void CBCGTabWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == nScrollLeftEventId)
	{
		OnCommand (0, (LPARAM) m_btnScrollLeft.GetSafeHwnd ());
	}
	else if (nIDEvent == nScrollRightEventId)
	{
		OnCommand (0, (LPARAM) m_btnScrollRight.GetSafeHwnd ());
	}
	else
	{
		CWnd::OnTimer (nIDEvent);
	}
}
//***************************************************************************************
void CBCGTabWnd::OnCancelMode() 
{
	ASSERT_VALID (this);

	CWnd::OnCancelMode();
	
	if (m_bTrackSplitter)
	{
		m_bTrackSplitter = FALSE;
		ReleaseCapture ();
	}

	if (m_pInPlaceEdit != NULL)
	{
		m_pInPlaceEdit->DestroyWindow ();
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;
		ReleaseCapture ();
	}

	if (m_bDragging)
	{
		KillTimer(nScrollLeftEventId);
		KillTimer(nScrollRightEventId);
		m_bDragging = FALSE;
		ReleaseCapture ();
	}

	if (m_iHighlighted >= 0)
	{
		int iTab = m_iHighlighted;

		ReleaseCapture ();
		m_iHighlighted = -1;

		InvalidateTab (iTab);
	}
}
//***********************************************************************************
void CBCGTabWnd::SetActiveTabTextColor (COLORREF clr)
{
	ASSERT_VALID (this);
	ASSERT (m_bFlat);

	m_clrActiveTabFg = clr;
}
//***********************************************************************************
void CBCGTabWnd::SetActiveTabColor (COLORREF clr)
{
	ASSERT_VALID (this);
	ASSERT (m_bFlat);

	m_clrActiveTabBk = clr;

	if (m_brActiveTab.GetSafeHandle () != NULL)
	{
		m_brActiveTab.DeleteObject ();
	}

	m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());
}
//**********************************************************************************
void CBCGTabWnd::OnSysColorChange() 
{
	ASSERT_VALID (this);

	CWnd::OnSysColorChange();

	if (m_bFlat && m_clrActiveTabFg == (COLORREF) -1)
	{
		if (m_brActiveTab.GetSafeHandle () != NULL)
		{
			m_brActiveTab.DeleteObject ();
		}

		m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());

		RedrawWindow (NULL, NULL,
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	InitAutoColors ();
}
//***********************************************************************************
BOOL CBCGTabWnd::SynchronizeScrollBar (SCROLLINFO* pScrollInfo/* = NULL*/)
{
	ASSERT_VALID (this);

	if (!m_bSharedScroll)
	{
		return FALSE;
	}

	ASSERT_VALID (this);

	SCROLLINFO scrollInfo;
	memset (&scrollInfo, 0, sizeof (SCROLLINFO));

	scrollInfo.cbSize = sizeof (SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	CWnd* pWndActive = GetActiveWnd ();

	if (pScrollInfo != NULL)
	{
		scrollInfo = *pScrollInfo;
	}
	else if (pWndActive != NULL)
	{
		if (!pWndActive->GetScrollInfo (SB_HORZ, &scrollInfo) ||
			scrollInfo.nMin + (int) scrollInfo.nPage >= scrollInfo.nMax)
		{
			m_wndScrollWnd.EnableScrollBar (ESB_DISABLE_BOTH);
			return TRUE;
		}
	}

	m_wndScrollWnd.EnableScrollBar (ESB_ENABLE_BOTH);
	m_wndScrollWnd.SetScrollInfo (&scrollInfo);

	HideActiveWindowHorzScrollBar ();
	return TRUE;
}
//*************************************************************************************
void CBCGTabWnd::HideActiveWindowHorzScrollBar ()
{
	ASSERT_VALID (this);

	CWnd* pWnd = GetActiveWnd ();
	if (pWnd == NULL || !m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID (pWnd);

	pWnd->ShowScrollBar (SB_HORZ, FALSE);
	pWnd->ModifyStyle (WS_HSCROLL, 0, SWP_DRAWFRAME);
}
//************************************************************************************
void CBCGTabWnd::SetTabsHeight ()
{
	ASSERT_VALID (this);

	if (m_bFlat)
	{
		m_nTabsHeight = ::GetSystemMetrics (SM_CYHSCROLL) + TEXT_MARGIN / 2;

		CFont* pDefaultFont = 
			CFont::FromHandle ((HFONT) ::GetStockObject (DEFAULT_GUI_FONT));
		ASSERT_VALID (pDefaultFont);

		if (pDefaultFont != NULL)	// Just to be relaxed....
		{
			LOGFONT lfDefault;
			pDefaultFont->GetLogFont (&lfDefault);

			LOGFONT lf;
			memset (&lf, 0, sizeof (LOGFONT));

			lf.lfCharSet = lfDefault.lfCharSet;
			lf.lfHeight = lfDefault.lfHeight;
			_tcscpy (lf.lfFaceName, TABS_FONT);

			CClientDC dc (this);

			TEXTMETRIC tm;

			do
			{
				m_fntTabs.DeleteObject ();
				m_fntTabs.CreateFontIndirect (&lf);

				CFont* pFont = dc.SelectObject (&m_fntTabs);
				ASSERT (pFont != NULL);

				dc.GetTextMetrics (&tm);
				dc.SelectObject (pFont);

				if (tm.tmHeight + TEXT_MARGIN / 2 <= m_nTabsHeight)
				{
					break;
				}

				//------------------
				// Try smaller font:
				//------------------
				if (lf.lfHeight < 0)
				{
					lf.lfHeight ++;
				}
				else
				{
					lf.lfHeight --;
				}
			}
			while (lf.lfHeight != 0);

			//------------------
			// Create bold font:
			//------------------
			lf.lfWeight = FW_BOLD;
			m_fntTabsBold.DeleteObject ();
			m_fntTabsBold.CreateFontIndirect (&lf);
		}
	}
	else if (m_bIsVS2005Style)
	{
		const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 4;
		m_nTabsHeight = (max (nImageHeight, globalData.GetTextHeight () + 2));
	}
	else
	{
		const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
		m_nTabsHeight = (max (nImageHeight, globalData.GetTextHeight () + 5));
	}
}
//*************************************************************************************
void CBCGTabWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	ASSERT_VALID (this);

	CWnd::OnSettingChange(uFlags, lpszSection);
	
	//-----------------------------------------------------------------
	// In the flat modetabs height should be same as scroll bar height
	//-----------------------------------------------------------------
	if (m_bFlat)
	{
		SetTabsHeight ();
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*************************************************************************************
BOOL CBCGTabWnd::EnsureVisible (int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("EnsureVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (!m_bScroll || m_rectTabsArea.Width () <= 0)
	{
		return TRUE;
	}

	//---------------------------------------------------------
	// Be sure, that active tab is visible (not out of scroll):
	//---------------------------------------------------------
	CRect rectTab = ((CBCGTabInfo*) m_arTabs [iTab])->m_rect;

	if (m_bTabDocumentsMenu)
	{
		if (rectTab.left >= m_rectTabsArea.right || rectTab.IsRectEmpty ())
		{
			MoveTab (iTab, 0);
		}

		return TRUE;
	}

	BOOL bAdjustTabs = FALSE;

	if (rectTab.left < m_rectTabsArea.left)
	{
		m_nTabsHorzOffset -= (m_rectTabsArea.left - rectTab.left);

		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			m_nFirstVisibleTab = iTab;
		}

		bAdjustTabs = TRUE;
	}
	else if (rectTab.right > m_rectTabsArea.right &&
		rectTab.Width () <= m_rectTabsArea.Width ())
	{
		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			if (rectTab.Width () < m_rectTabsArea.Width ())
			{
				while (rectTab.right > m_rectTabsArea.right)
				{
					if (m_nFirstVisibleTab >= m_iTabsNum)
					{
						return FALSE;
					}

					int nDelta = ((CBCGTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width () - 
						(m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1);
					
					m_nTabsHorzOffset += nDelta;
					rectTab.OffsetRect (-nDelta, 0);

					m_nFirstVisibleTab ++;
				}
			}
		}
		else
		{
			m_nTabsHorzOffset += (rectTab.right - m_rectTabsArea.right);
		}

		bAdjustTabs = TRUE;
	}

	if (bAdjustTabs)
	{
		AdjustTabs ();
		AdjustTabsScroll ();

		RedrawWindow ();
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGTabWnd::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT_VALID (this);

	BOOL bRes = CWnd::OnNotify (wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT (pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW && m_ToolTip.GetSafeHwnd () != NULL)
	{
		m_ToolTip.SetWindowPos (&wndTop, -1, -1, -1, -1,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	if (pNMHDR->code == HDN_ITEMCHANGED)
	{
		SynchronizeScrollBar ();
	}

	return bRes;
}
//*********************************************************************************
BOOL CBCGTabWnd::GetTabRect (int iTab, CRect& rect) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	if (!pTab->m_bVisible)
	{
		rect.SetRectEmpty ();
		return FALSE;
	}
	
	rect = pTab->m_rect;
	return TRUE;
}
//********************************************************************************
UINT CBCGTabWnd::GetTabIcon (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (UINT) -1;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return pTab->m_uiIcon;
}
//*********************************************************************************
BOOL CBCGTabWnd::SetTabIcon (int iTab, UINT uiIcon)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	pTab->m_uiIcon = uiIcon;

	SetTabsHeight ();
	return TRUE;
}
//*********************************************************************************
BOOL CBCGTabWnd::GetTabLabel (int iTab, CString& strLabel) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	strLabel = pTab->m_bIconOnly ? _T("") : pTab->m_strText;
	return TRUE;
};
//*********************************************************************************
BOOL CBCGTabWnd::SetTabLabel (int iTab, const CString& strLabel)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum || strLabel.IsEmpty ())
	{
		return FALSE;
	}
	
	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);
	
	pTab->m_strText = strLabel;
	
	if (!m_bFlat && m_ToolTip.GetSafeHwnd () != NULL)
	{
		m_ToolTip.UpdateTipText (strLabel, this, pTab->m_iTabID);
	}

	RecalcLayout();
	return TRUE;
}
//*********************************************************************************
BOOL CBCGTabWnd::IsTabIconOnly (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	return pTab->m_bIconOnly;
};
//*********************************************************************************
BOOL CBCGTabWnd::SetTabIconOnly (int iTab, BOOL bIconOnly, BOOL bAlwaysShowToolTip)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);
	
	pTab->m_bIconOnly = bIconOnly;
	pTab->m_bAlwaysShowToolTip = bAlwaysShowToolTip;
	
	RecalcLayout();
	return TRUE;
}
//*********************************************************************************
COLORREF CBCGTabWnd::GetTabBkColor (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (COLORREF)-1;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	COLORREF color = pTab->m_clrBack;

	if (color == (COLORREF)-1 && m_bIsAutoColor)
	{
		color = m_arAutoColors [iTab % m_arAutoColors.GetSize ()];
	}

	return color;
}
//*********************************************************************************
BOOL CBCGTabWnd::SetTabBkColor (int iTab, COLORREF color)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrBack = color;
	return TRUE;
}
//*********************************************************************************
COLORREF CBCGTabWnd::GetTabTextColor (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (COLORREF)-1;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_clrText;
}
//*********************************************************************************
BOOL CBCGTabWnd::SetTabTextColor (int iTab, COLORREF color)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrText = color;
	return TRUE;
}
//*********************************************************************************
void CBCGTabWnd::HideSingleTab (BOOL bHide)
{
	ASSERT_VALID (this);

	if (m_bHideSingleTab == bHide)
	{
		return;
	}

	m_bHideSingleTab = bHide;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//*********************************************************************************
void CBCGTabWnd::HideNoTabs (BOOL bHide)
{
	ASSERT_VALID (this);

	if (m_bHideNoTabs == bHide)
	{
		return;
	}

	m_bHideNoTabs = bHide;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
		SynchronizeScrollBar ();
	}
}
//***********************************************************************************
BOOL CBCGTabWnd::BeginLabelEdit (int iTab)
{
	ASSERT_VALID (this);

	if (!m_bIsInPlaceEdit)
	{
		return FALSE;
	}

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		ASSERT (FALSE);
		return FALSE;
	};
	
	EnsureVisible (iTab);

	ASSERT (m_pInPlaceEdit == NULL);

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);
	
	if (pTab->m_bIconOnly)
	{
		return FALSE;
	}

	m_pInPlaceEdit = new CEdit;
	ASSERT_VALID (m_pInPlaceEdit);

	CRect rectEdit = pTab->m_rect;
	rectEdit.DeflateRect (m_nTabsHeight / 2, 1);

	if (!m_pInPlaceEdit->Create (WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, rectEdit, 
		this, (UINT)-1))
	{
		TRACE(_T("Unable to create edit control\n"));
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;

		return FALSE;
	}

	m_pInPlaceEdit->SetWindowText (pTab->m_strText);
	m_pInPlaceEdit->SetFont (&globalData.fontRegular);
	m_pInPlaceEdit->SetSel (0, -1, TRUE);

	m_pInPlaceEdit->SetFocus ();
	m_iEditedTab = iTab;

	SetCapture ();
	::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
	return TRUE;
}
//************************************************************************************
void CBCGTabWnd::ReposButtons (CPoint pt, CSize sizeButton, BOOL bHide, int nButtonMargin)
{
	BOOL bIsFirst = TRUE;

	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext (pos);
		ASSERT (hWndButton != NULL);

		if (bHide || (!m_bCloseBtn && hWndButton == m_btnClose.GetSafeHwnd ()) ||
			(m_bTabDocumentsMenu && bIsFirst))
		{
			::SetWindowPos (hWndButton, NULL,
							0, 0, 0, 0,
							SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else
		{
			int x = pt.x;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && bIsFirst)	// Scroll left is on left
			{
				x = m_rectTabsArea.left - sizeButton.cx - 1;
			}

			::SetWindowPos (hWndButton, NULL,
				x, pt.y,
				sizeButton.cx, sizeButton.cy,
				SWP_NOACTIVATE | SWP_NOZORDER);

			if ((!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded) || !bIsFirst)
			{
				pt.x += sizeButton.cx + nButtonMargin;
			}
		}

		::InvalidateRect (hWndButton, NULL, TRUE);
		::UpdateWindow (hWndButton);

		bIsFirst = FALSE;
	}
}
//**************************************************************************************
void CBCGTabWnd::EnableInPlaceEdit (BOOL bEnable)
{
	ASSERT_VALID (this);

	if (!m_bFlat)
	{
		// In-place editing is available for the flat tabs only!
		ASSERT (FALSE);
		return;
	}

	m_bIsInPlaceEdit = bEnable;
}
//************************************************************************************
void CBCGTabWnd::AutoHideTabsScroll (BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT (m_bFlat);

	if (m_bAutoHideScroll != bHide && m_bFlat)
	{
		m_bAutoHideScroll = bHide;
		RecalcLayout ();
	}
}
//*********************************************************************************
BOOL CBCGTabWnd::RenameTab ()
{
	ASSERT_VALID (this);
	ASSERT (m_bIsInPlaceEdit);

	if (m_pInPlaceEdit == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CString strName;
	m_pInPlaceEdit->GetWindowText (strName);

	if (!strName.IsEmpty () &&
		OnRenameTab (m_iEditedTab, strName) &&
		GetParent ()->SendMessage (BCGM_ON_RENAME_TAB, m_iEditedTab,
									(LPARAM)(LPCTSTR) strName) == 0)
	{
		return SetTabLabel (m_iEditedTab, strName);
	}

	return FALSE;
}
//********************************************************************************
void CBCGTabWnd::SetDrawNoPrefix (BOOL bNoPrefix, BOOL bRedraw)
{
	ASSERT_VALID (this);
	m_bLabelNoPrefix = bNoPrefix;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGTabWnd::EnableDragDrop (BOOL bDragDrop)
{
	ASSERT_VALID (this);
	
	if (!m_bFlat)
	{
		// Darg and drop is avalible for the flat tabs only
		ASSERT(FALSE);
		return;
	}

	m_bDragDrop = bDragDrop;
}
//****************************************************************************************
BOOL CBCGTabWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	const int nScrollOffset = 20;

	BOOL bScrollTabs = FALSE;
	int nPrevOffset = m_nTabsHorzOffset;

	if ((HWND)lParam == m_btnScrollLeft.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;

		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			if (m_nFirstVisibleTab > 0)
			{
				m_nTabsHorzOffset -= ((CBCGTabInfo*) m_arTabs [m_nFirstVisibleTab - 1])->m_rect.Width () - 
					(m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 2);
				m_nFirstVisibleTab --;
			}
		}
		else
		{
			m_nTabsHorzOffset -= nScrollOffset;
		}
	}
	else if ((HWND)lParam == m_btnScrollRight.GetSafeHwnd ())
	{
		if (m_bTabDocumentsMenu)
		{
			CRect rectButton;
			m_btnScrollRight.GetWindowRect (&rectButton);

			m_btnScrollRight.SetPressed (TRUE);

			OnShowTabDocumentsMenu (CPoint (rectButton.left, rectButton.bottom));

			m_btnScrollRight.SetPressed (FALSE);
		}
		else
		{
			bScrollTabs = TRUE;

			if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
			{
				if (m_nFirstVisibleTab < m_iTabsNum)
				{
					m_nTabsHorzOffset += ((CBCGTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width () - 
						(m_rectTabsArea.Height () - TAB_IMAGE_MARGIN - 1);
					m_nFirstVisibleTab ++;
				}
			}
			else
			{
				m_nTabsHorzOffset += nScrollOffset;
			}
		}
	}
	else if ((HWND)lParam == m_btnScrollFirst.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = 0;
	}
	else if ((HWND)lParam == m_btnScrollLast.GetSafeHwnd ())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = m_nTabsHorzOffsetMax;
	}
	else if ((HWND)lParam == m_btnClose.GetSafeHwnd ())
	{
		CWnd* pWndActive = GetActiveWnd ();
		if (pWndActive != NULL)
		{
			pWndActive->SendMessage (WM_CLOSE);
		}

		return TRUE;
	}

	if (bScrollTabs)
	{
		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			m_nTabsHorzOffset = max (0, m_nTabsHorzOffset);
		}
		else
		{
			m_nTabsHorzOffset = min (max (0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
		}

		if (nPrevOffset != m_nTabsHorzOffset)
		{
			AdjustTabs ();
			UpdateScrollButtonsState ();

			RedrawWindow (NULL, NULL,
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}

		return TRUE;
	}
	
	return CWnd::OnCommand(wParam, lParam);
}
//**********************************************************************************
void CBCGTabWnd::EnableAutoColor (BOOL bEnable/* = TRUE*/)
{
	if (m_bIsAutoColor && !bEnable)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGTabInfo* pTabInfo = (CBCGTabInfo*) m_arTabs.GetAt (i);
			ASSERT_VALID (pTabInfo);

			pTabInfo->m_clrBack = (COLORREF)-1;
		}
	}

	m_bIsAutoColor = bEnable;
	InitAutoColors ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//***********************************************************************************
void CBCGTabWnd::InitAutoColors ()
{
	if (!m_bIsDefaultAutoColor)
	{
		return;
	}

	m_arAutoColors.RemoveAll ();

	if (globalData.m_nBitsPerPixel > 8)
	{
		m_arAutoColors.Add (RGB (148, 175, 230));
		m_arAutoColors.Add (RGB (255, 219, 117));
		m_arAutoColors.Add (RGB (189, 205, 159));
		m_arAutoColors.Add (RGB (240, 158, 159));
		m_arAutoColors.Add (RGB (186, 166, 225));
		m_arAutoColors.Add (RGB (154, 191, 180));
		m_arAutoColors.Add (RGB (247, 182, 131));
		m_arAutoColors.Add (RGB (213, 164, 187));
	}
	else
	{
		m_arAutoColors.Add (RGB (0, 255, 0));
		m_arAutoColors.Add (RGB (0, 255, 255));
		m_arAutoColors.Add (RGB (255, 0, 255));
		m_arAutoColors.Add (RGB (192, 192, 192));
		m_arAutoColors.Add (RGB (255, 255, 0));
	}
}
//*********************************************************************************
void CBCGTabWnd::SetAutoColors (const CArray<COLORREF, COLORREF>& arColors)
{
	m_arAutoColors.RemoveAll ();

	if (arColors.GetSize () == 0)
	{
		m_bIsDefaultAutoColor = TRUE;
		InitAutoColors ();
	}
	else
	{
		m_bIsDefaultAutoColor = FALSE;

		for (int i = 0; i < arColors.GetSize (); i++)
		{
			m_arAutoColors.Add (arColors [i]);
		}
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//************************************************************************************
void CBCGTabWnd::InvalidateTab (int iTab)
{
	ASSERT_VALID (this);

	if (iTab < 0)
	{
		return;
	}

	CRect rectTab;

	if (GetTabRect (iTab, rectTab))
	{
		InvalidateRect (rectTab);
		UpdateWindow ();
	}
}
//*************************************************************************************
BOOL CBCGTabWnd::ModifyTabStyle (Style style)
{
	ASSERT_VALID (this);

	m_bFlat = (style == STYLE_FLAT);

	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || style == STYLE_3D_ROUNDED_SCROLL);

	SetScrollButtons ();
	SetTabsHeight ();

	return TRUE;
}
//***********************************************************************************
void CBCGTabWnd::SetScrollButtons ()
{
	const int nAutoRepeat = m_bIsOneNoteStyle || m_bTabDocumentsMenu ? 0 : 50;

	m_btnScrollLeft.SetAutorepeatMode (nAutoRepeat);
	m_btnScrollRight.SetAutorepeatMode (nAutoRepeat);

	m_btnScrollLeft.SetStdImage (CMenuImages::IdArowRightLarge,
			CMenuImages::IdArowRightLargeDsbl);

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage (
			CMenuImages::IdCustomizeArowDown,
			CMenuImages::IdCustomizeArowDown);
	}
	else
	{
		m_btnScrollRight.SetStdImage (CMenuImages::IdArowLeftLarge,
			CMenuImages::IdArowLeftLargeDsbl);
	}

	m_btnClose.SetStdImage (CMenuImages::IdClose);
}
//*************************************************************************************
void CBCGTabWnd::UpdateScrollButtonsState ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL || !m_bScroll || m_bFlat)
	{
		return;
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		if (m_arTabs.GetSize () == 0)
		{
			m_btnScrollLeft.EnableWindow (FALSE);
			m_btnScrollRight.EnableWindow (FALSE);
		}
		else
		{
			m_btnScrollLeft.EnableWindow (m_nFirstVisibleTab > 0);

			CBCGTabInfo* pLastTab = (CBCGTabInfo*) m_arTabs [m_arTabs.GetSize () - 1];
			ASSERT_VALID (pLastTab);

			m_btnScrollRight.EnableWindow (m_bTabDocumentsMenu ||
				(pLastTab->m_rect.right > m_rectTabsArea.right &&
				m_nFirstVisibleTab < m_arTabs.GetSize () - 1));
		}
	}
	else
	{
		m_btnScrollLeft.EnableWindow (m_nTabsHorzOffset > 0);
		m_btnScrollRight.EnableWindow (m_bTabDocumentsMenu || m_nTabsHorzOffset < m_nTabsHorzOffsetMax);
	}

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage (
			m_bHiddenDocuments ? CMenuImages::IdCustomizeArowDown : CMenuImages::IdArowDownLarge);
	}

	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext (pos);
		ASSERT (hWndButton != NULL);

		if (!::IsWindowEnabled (hWndButton))
		{
			::SendMessage (hWndButton, WM_CANCELMODE, 0, 0);
		}
	}
}
//*********************************************************************************
void CBCGTabWnd::SetLocation (Location location)
{
	ASSERT_VALID (this);

	m_location = location;
	RecalcLayout ();

	if (GetSafeHwnd () != NULL)
	{
		GetParent ()->RedrawWindow (NULL, NULL, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}
//*********************************************************************************
BOOL CBCGTabWnd::IsColored () const
{
	for (int iTab = 0; iTab < m_iTabsNum; iTab++)
	{
		if (GetTabBkColor (iTab) != (COLORREF)-1)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGTabWnd::HasImage (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return GetImageList () != NULL && pTab->m_uiIcon != (UINT)-1;
}
//***************************************************************************************
void CBCGTabWnd::EnableTabDocumentsMenu (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);

	if (m_bFlat || !m_bScroll)
	{
		ASSERT (FALSE);
		return;
	}

	CBCGLocalResource locaRes;

	m_bTabDocumentsMenu = bEnable;

	m_btnScrollRight.SetTooltip (m_bTabDocumentsMenu ? _T("Open Documents") : _T("Scroll Right"));

	SetScrollButtons ();
	RecalcLayout ();

	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;

	EnsureVisible (m_iActiveTab);
}
//****************************************************************************
void CBCGTabWnd::OnShowTabDocumentsMenu (CPoint point)
{
	if (g_pContextMenuManager == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const UINT idStart = (UINT) -100;

	CMenu menu;
	menu.CreatePopupMenu ();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGTabInfo* pTab = (CBCGTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (!pTab->m_bVisible)
		{
			continue;
		}

		const UINT uiID = idStart - i;
		CString strTabName = pTab->m_strText;

		//--------------------------------
		// Replace all single '&' by '&&':
		//--------------------------------
		const CString strDummyAmpSeq = _T("\001\001");

		strTabName.Replace (_T("&&"), strDummyAmpSeq);
		strTabName.Replace (_T("&"), _T("&&"));
		strTabName.Replace (strDummyAmpSeq, _T("&&"));

		// Insert sorted:
		BOOL bInserted = FALSE;

		for (UINT iMenu = 0; iMenu < menu.GetMenuItemCount (); iMenu++)
		{
			CString strMenuItem;
			menu.GetMenuString (iMenu, strMenuItem, MF_BYPOSITION);

			if (strTabName.CompareNoCase (strMenuItem) < 0)
			{
				menu.InsertMenu (iMenu, MF_BYPOSITION, uiID, strTabName);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			menu.AppendMenu (MF_STRING, uiID, strTabName);
		}

		if (pTab->m_pWnd->GetSafeHwnd () != NULL)
		{
			HICON hIcon = pTab->m_pWnd->GetIcon (FALSE);
			if (hIcon == NULL)
			{
				hIcon = (HICON)(LONG_PTR) GetClassLongPtr (pTab->m_pWnd->GetSafeHwnd (), GCLP_HICONSM);
			}

			m_mapDocIcons.SetAt (uiID, hIcon);
		}
	}

	int nMenuResult = g_pContextMenuManager->TrackPopupMenu (
			menu, point.x, point.y, this);

	int iTab = idStart - nMenuResult;
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		SetActiveTab (iTab);
	}

	m_mapDocIcons.RemoveAll ();
}
//****************************************************************************
HICON CBCGTabWnd::GetDocumentIcon (UINT nCmdID)
{
	HICON hIcon = NULL;
	m_mapDocIcons.Lookup (nCmdID, hIcon);

	return hIcon;
}
//********************************************************************************
void CBCGTabWnd::MoveTab (int nSource, int nDest)
{
	ASSERT_VALID (this);

	if (nSource == nDest)
	{
		return;
	}

	CBCGTabInfo* pSource = (CBCGTabInfo*) m_arTabs [nSource];
	CBCGTabInfo* pActive = (CBCGTabInfo*) m_arTabs [m_iActiveTab];

	ASSERT (nDest < m_arTabs.GetSize ());

	if (nDest == -1)
	{
		m_arTabs.Add (pSource);
		m_arTabs.RemoveAt (nSource);
	}
	else
	{
		m_arTabs.RemoveAt (nSource);
		m_arTabs.InsertAt (nDest, pSource);
	}

	for (int iTab = 0; iTab < m_arTabs.GetSize(); iTab++)
	{
		if (pActive == (CBCGTabInfo*) m_arTabs [iTab])
		{
			if (iTab != m_iActiveTab)
			{
				SetActiveTab (iTab);
			}

			break;
		}
	}

	RecalcLayout ();
}

#endif // BCG_NO_TABCTRL
