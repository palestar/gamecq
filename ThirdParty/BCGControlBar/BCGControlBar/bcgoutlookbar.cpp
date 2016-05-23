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
//********************************************************************
//
// bcgoutlookbar.cpp : implementation file
//
// REVISION HISTORY
// ----------------
// 0.00 ?
//   created
// 0.01 28april2000 (Rui Lopes <ruiglopes@yahoo.com>)
//   + Splitter now changes place depending on the dock position
//   + Changed the border drawing code
//   ~ Other misc stuff
//
//********************************************************************

#include "stdafx.h"

#ifndef BCG_NO_OUTLOOKBAR

#include "bcgoutlookbar.h"
#include "MenuImages.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGWorkspace.h"
#include "BCGRegistry.h"
#include "BCGCBVer.h"
#include "BCGVisualManager.h"
#include "customizebutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REG_ENTRY_ORIG_PAGES	_T("OriginalPages")

extern CBCGWorkspace* g_pWorkspace;

//------------------
// Timer event IDs:
//------------------
static const UINT uiScrollDelay = 200;		// ms
static const UINT uiCagngePageDelay = 500;	// ms

const UINT CBCGOutlookBar::m_nIDScrollUp	= 1;
const UINT CBCGOutlookBar::m_nIDScrollDn	= 2;
const UINT CBCGOutlookBar::m_nIDChangePage	= 3;
const UINT CBCGOutlookBar::m_nIDClose		= HTCLOSE;

static const int nSplitterHeight = 8;
static const int nToolbarMarginHeight = 4;
static const UINT idShowMoreButtons = 0xf200;
static const UINT idShowFewerButtons = 0xf201;
static const UINT idToolbarCommandID = 0xf203;

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBarButton

class CBCGOutlookBarPageButton : public CBCGButton
{
	friend class CBCGOutlookBar;

	DECLARE_DYNAMIC(CBCGOutlookBarPageButton)

	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient)
	{
		COLORREF colorText = (COLORREF) -1;

		if (!CBCGVisualManager::GetInstance ()->OnFillOutlookPageButton (
			this, pDC, rectClient, colorText))
		{
			CBCGButton::OnFillBackground (pDC, rectClient);
		}
		else
		{
			if (m_bHighlighted)
			{
				m_clrHover = colorText;
			}
			else
			{
				m_clrRegular = colorText;
			}
		}
	}

	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
	{
		if (!CBCGVisualManager::GetInstance ()->OnDrawOutlookPageButtonBorder (
			this, pDC, rectClient, uiState))
		{
			CBCGButton::OnDrawBorder (pDC, rectClient, uiState);
		}
	}
};

IMPLEMENT_DYNAMIC(CBCGOutlookBarPageButton, CBCGButton)

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBarPage

class CBCGOutlookBarPage : public CObject
{
	DECLARE_SERIAL(CBCGOutlookBarPage)

	friend class CBCGOutlookBar;

// Constructor/destructor:
	CBCGOutlookBarPage();
	CBCGOutlookBarPage(const CBCGOutlookBarPage& other);

	virtual ~CBCGOutlookBarPage ();

// Operations:
	void Serialize (CArchive& ar);
	BOOL CreateButton (CWnd* pWndParent);

	void SetRect (const CRect& rect);
	void SetName (LPCTSTR lpszName);

	BOOL Compare (const CBCGOutlookBarPage& other) const;

// Attributes:
	UINT						m_uiID;
	CString						m_strName;

	HWND						m_hwndControl;
	CBCGOutlookBarPageButton	m_btnPage;
	CRect						m_rect;
};

IMPLEMENT_SERIAL(CBCGOutlookBarPage, CObject, 1)

CBCGOutlookBarPage::CBCGOutlookBarPage ()
{
	m_uiID = 0;
	m_hwndControl = NULL;
	m_rect.SetRectEmpty ();
}

CBCGOutlookBarPage::CBCGOutlookBarPage(const CBCGOutlookBarPage& other)
{
	m_uiID			= other.m_uiID;
	m_strName		= other.m_strName;
	m_hwndControl	= other.m_hwndControl;

	m_rect.SetRectEmpty ();
}

CBCGOutlookBarPage::~CBCGOutlookBarPage ()
{
	m_btnPage.DestroyWindow ();

	if (m_hwndControl != NULL)
	{
		::ShowWindow (m_hwndControl, SW_HIDE);
	}
}

void CBCGOutlookBarPage::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

	if (ar.IsLoading ())
	{
		ar >> m_uiID;
		ar >> m_strName;
		
		m_hwndControl = NULL;
	}
	else
	{
		ar << m_uiID;
		ar << m_strName;
	}
}

BOOL CBCGOutlookBarPage::CreateButton (CWnd* pWndParent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndParent);

	if (m_btnPage.GetSafeHwnd () != NULL)
	{
		return TRUE;
	}

	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	if (!m_btnPage.Create (m_strName, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					rectDummy,
					pWndParent, m_uiID))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_btnPage.SetMouseCursorHand ();
	m_btnPage.m_bDrawFocus = FALSE;
	m_btnPage.m_nFlatStyle = CBCGButton::BUTTONSTYLE_SEMIFLAT;
	m_btnPage.EnableFullTextTooltip ();
	m_btnPage.m_bDontUseWinXPTheme = TRUE;

	return TRUE;
}

void CBCGOutlookBarPage::SetRect (const CRect& rect)
{
	if (m_rect == rect)
	{
		return;
	}

	m_rect = rect;
	m_btnPage.SetWindowPos (NULL, rect.left, rect.top,
							rect.Width (), rect.Height (),
							SWP_NOZORDER | SWP_NOACTIVATE);
}

void CBCGOutlookBarPage::SetName (LPCTSTR lpszName)
{
	ASSERT (lpszName != NULL);

	m_strName = lpszName;
	if (m_btnPage.GetSafeHwnd () != NULL)
	{
		m_btnPage.SetWindowText (m_strName);
	}
}

BOOL CBCGOutlookBarPage::Compare (const CBCGOutlookBarPage& other) const
{
	return	other.m_strName == m_strName &&
			other.m_uiID == other.m_uiID;		
}

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBar

IMPLEMENT_DYNAMIC(CBCGOutlookBar, CBCGToolBar)

#pragma warning (disable : 4355)

CBCGOutlookBar::CBCGOutlookBar() :
	m_wndToolBar (this)
{
	m_nSize = -1;
	m_bEnableAnimation = TRUE;
	m_bEnableCloseButton = TRUE;

	m_iScrollOffset = 0;
	m_iFirstVisibleButton = 0;
	m_bScrollDown = FALSE;

	m_clrRegText = (COLORREF)-1;
	m_clrBackColor = globalData.clrBarShadow;

	m_clrTransparentColor = RGB (255, 0, 255);

	m_csImage = CSize (0, 0);

	m_uiBackImageId = 0;

	m_btnUp.m_nFlatStyle = CBCGButton::BUTTONSTYLE_3D;
	m_btnUp.m_bDrawFocus = FALSE;
	m_btnUp.m_bDontUseWinXPTheme = TRUE;

	m_btnDown.m_nFlatStyle = CBCGButton::BUTTONSTYLE_3D;
	m_btnDown.m_bDrawFocus = FALSE;
	m_btnDown.m_bDontUseWinXPTheme = TRUE;

	m_bDrawShadedHighlight = FALSE;

	m_uiActivePageID = (UINT)-1;
	m_uiEditedPageID = (UINT)-1;
	m_uiDelayedPageID = (UINT)-1;
	m_rectActivePageBtn.SetRectEmpty ();

	m_bDisableControlsIfNoHandler = FALSE;

	m_rectWorkArea.SetRectEmpty ();

	m_pInPlaceEdit = NULL;
	m_rectCaption.SetRectEmpty ();

	m_btnClose.m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;
	m_btnClose.m_bDrawFocus = FALSE;
	m_btnClose.m_bDontUseWinXPTheme = TRUE;

	m_bFlatBorder = FALSE;
	m_nExtraSpace = 0;
	m_bPageScrollMode = FALSE;

	m_bMode2003 = FALSE;
	m_nVisiblePageButtons = -1;
	m_nMaxVisiblePageButtons = 0;

	m_rectSplitterPage.SetRectEmpty ();
	m_bIsTrackingPages = FALSE;

	m_sizeToolbarImage = CSize (0, 0);
	m_sizePageImage = CSize (0, 0);

	m_nPageButtonHeight = 0;
	m_pFontButtons = NULL;

	EnableSplitter ();
}

#pragma warning (default : 4355)

CBCGOutlookBar::~CBCGOutlookBar()
{
	while (!m_lstPages.IsEmpty ())
	{
		delete m_lstPages.RemoveHead ();
	}

	while (!m_lstOrigPages.IsEmpty ())
	{
		delete m_lstOrigPages.RemoveHead ();
	}
}

BEGIN_MESSAGE_MAP(CBCGOutlookBar, CBCGToolBar)
	//{{AFX_MSG_MAP(CBCGOutlookBar)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
    ON_WM_NCPAINT()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
	ON_WM_STYLECHANGING()
	ON_UPDATE_COMMAND_UI_RANGE(idShowMoreButtons, idShowMoreButtons + 10, OnUpdateToolbarCommand)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBar message handlers

BOOL CBCGOutlookBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	m_bCaption = (dwStyle & WS_CAPTION) || m_bMode2003;
	dwStyle &= ~WS_CAPTION;

	return CBCGToolBar::Create (pParentWnd,
		dwStyle | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, nID);
}
//*************************************************************************************
BOOL CBCGOutlookBar::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*************************************************************************************
BOOL CBCGOutlookBar::AddButton (LPCTSTR szBmpFileName, LPCTSTR szLabel, 
								UINT iIdCommand, UINT uiPageID, int iInsertAt)
//
// Adds a button by loading the image from disk instead of a resource
//
{
	ASSERT (szBmpFileName != NULL);

	HBITMAP hBmp = (HBITMAP) ::LoadImage (	
								NULL, szBmpFileName, IMAGE_BITMAP, 0, 0, 
								LR_DEFAULTSIZE | LR_LOADFROMFILE); 
	if (hBmp == NULL)
	{
		TRACE(_T("Can't load bitmap resource: %s"), szBmpFileName);
		ASSERT (FALSE);

		return FALSE;
	}

	int iImageIndex = AddBitmapImage (hBmp);
	ASSERT (iImageIndex >= 0);

	return InternalAddButton (iImageIndex, szLabel, iIdCommand, uiPageID, iInsertAt);
}
//*************************************************************************************
BOOL CBCGOutlookBar::AddButton (UINT uiImage, UINT uiLabel, UINT iIdCommand, 
								UINT uiPageID, int iInsertAt)
{
	CString strLable;
	strLable.LoadString (uiLabel);

	return AddButton (uiImage, strLable, iIdCommand, uiPageID, iInsertAt);
}
//*************************************************************************************
BOOL CBCGOutlookBar::AddButton (UINT uiImage, LPCTSTR lpszLabel, UINT iIdCommand,
								UINT uiPageID, int iInsertAt)
{
	int iImageIndex = -1;
	if (uiImage != 0)
	{
		CBitmap bmp;
		if (!bmp.LoadBitmap (uiImage))
		{
			TRACE(_T("Can't load bitmap resource: %d"), uiImage);
			return FALSE;
		}

		iImageIndex = AddBitmapImage ((HBITMAP) bmp.GetSafeHandle ());
	}

	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, uiPageID, iInsertAt);
}
//**************************************************************************************
BOOL CBCGOutlookBar::AddButton (HBITMAP hBmp, LPCTSTR lpszLabel, UINT iIdCommand, UINT uiPageID, int iInsertAt)
{
	ASSERT (hBmp != NULL);

	int iImageIndex = AddBitmapImage (hBmp);
	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, uiPageID, iInsertAt);
}
//**************************************************************************************
BOOL CBCGOutlookBar::AddButton (HICON hIcon, LPCTSTR lpszLabel, UINT iIdCommand, UINT uiPageID, int iInsertAt,
								BOOL bAlphaBlend)
{
	ASSERT (hIcon != NULL);

	int iImageIndex = -1;

	ICONINFO iconInfo;
	::GetIconInfo (hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

	CSize size (bitmap.bmWidth, bitmap.bmHeight);

	if (bAlphaBlend)
	{
		if (m_Images.GetCount() == 0)	// First image
		{
			m_csImage = size;
			m_Images.SetImageSize (size);
		}

		iImageIndex = m_Images.AddIcon (hIcon, TRUE);
	}
	else
	{
		CClientDC dc (this);

		CDC dcMem;
		dcMem.CreateCompatibleDC (&dc);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap (&dc, size.cx, size.cy);

		CBitmap* pOldBmp = dcMem.SelectObject (&bmp);

		if (m_clrTransparentColor != (COLORREF)-1)
		{
			dcMem.FillSolidRect (0, 0, size.cx, size.cy, m_clrTransparentColor);
		}

		::DrawIconEx (dcMem.GetSafeHdc (), 0, 0, hIcon, size.cx, size.cy,
			0, NULL, DI_NORMAL);

		dcMem.SelectObject (pOldBmp);

		::DeleteObject (iconInfo.hbmColor);
		::DeleteObject (iconInfo.hbmMask);

		iImageIndex = AddBitmapImage ((HBITMAP) bmp.GetSafeHandle ());
	}

	return InternalAddButton (iImageIndex, lpszLabel, iIdCommand, uiPageID, iInsertAt);
}
//**************************************************************************************
BOOL CBCGOutlookBar::RemoveButton (UINT iIdCommand, UINT uiPageID)
{
	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == iIdCommand &&
			(uiPageID == (UINT) -1 || pButton->m_uiPageID == uiPageID))
		{
			m_Buttons.RemoveAt (posSave);
			delete pButton;

			if (GetSafeHwnd () != NULL)
			{
				AdjustLocations ();
				UpdateWindow ();
				Invalidate ();
			}

			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
BOOL CBCGOutlookBar::InternalAddButton (int iImageIndex, LPCTSTR lpszLabel,
										UINT iIdCommand, UINT uiPageID, int iInsertAt)
{
	if (!m_lstPages.IsEmpty ())
	{
		CBCGOutlookBarPage* pPage = NULL;
		if (uiPageID == (UINT) -1)
		{
			//----------------------------------
			// Add to the first page by default:
			//----------------------------------
			pPage = (CBCGOutlookBarPage*) m_lstPages.GetHead ();
			ASSERT_VALID (pPage);

			uiPageID = pPage->m_uiID;
		}
		else
		{
			if ((pPage = GetPage (uiPageID)) == NULL)
			{
				TRACE(_T("Can't add button to the invalid page %d\n"), uiPageID);
				ASSERT (FALSE);

				return FALSE;
			}
		}

		if (pPage->m_hwndControl != NULL)	// This page has control
		{
			TRACE(_T("Can't add button to the page %d. This page has attached window.\n"), uiPageID);

			ASSERT (FALSE);
			return FALSE;
		}
	}

	CBCGOutlookButton* pButton = new CBCGOutlookButton;
	ASSERT (pButton != NULL);

	pButton->m_nID = iIdCommand;
	pButton->m_strText = (lpszLabel == NULL) ? _T("") : lpszLabel;
	pButton->SetImage (iImageIndex);
	pButton->m_bTextBelow = m_bTextLabels;
	pButton->m_uiPageID = uiPageID;

	if (iInsertAt == -1)
	{
		iInsertAt = (int) m_Buttons.GetCount ();
	}
	
	InsertButton (pButton, iInsertAt);

	AdjustLayout ();
	return TRUE;
}
//*************************************************************************************
int CBCGOutlookBar::AddBitmapImage (HBITMAP hBitmap)
{
	ASSERT (hBitmap != NULL);

	BITMAP	bitmap;
	::GetObject (hBitmap, sizeof (BITMAP), &bitmap);

	CSize csImage = CSize (bitmap.bmWidth, bitmap.bmHeight);

	if (m_Images.GetCount() == 0)	// First image
	{
		m_csImage = csImage;
		m_Images.SetImageSize(csImage);

		if (m_bEnableSplitter && m_nSize == -1)
		{
			BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;

			m_nSize = (bHorz) ? m_csImage.cy * 2 + 2 * BORDER_SIZE:
					  m_csImage.cx * 2 + 2 * BORDER_SIZE;
			m_nSize *= 2;
		}
	}
	else
	{
		ASSERT (m_csImage == csImage);	// All buttons should be of the same size!
	}

	int nRes = m_Images.AddImage (hBitmap);
	if (nRes < 0)
	{
		return -1;
	}

	return nRes;
}
//*************************************************************************************
void CBCGOutlookBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGToolBar::OnSize(nType, cx, cy);

	AdjustLayout ();
	SetSplitterRect ();

	int iButtons = (int) m_Buttons.GetCount ();
	if (iButtons > 0)
	{
		POSITION posLast = m_Buttons.FindIndex (iButtons - 1);
		CBCGOutlookButton* pButtonLast = (CBCGOutlookButton*) m_Buttons.GetAt (posLast);
		ASSERT (pButtonLast != NULL);

		while (m_iScrollOffset > 0 &&
			pButtonLast->Rect ().bottom < cy)
		{
			ScrollUp ();
		}
	}
}
//*************************************************************************************
void CBCGOutlookBar::SetSplitterRect ()
{
	if (m_bEnableSplitter && GetSafeHwnd () != NULL)
	{
		GetClientRect (&m_rectSplitter);

		//------------------------------
		//RGL: Compute splitter position
		//------------------------------
		if (m_dwStyle&CBRS_ORIENT_HORZ)
		{
			if (m_dwStyle&CBRS_TOP) 
				m_rectSplitter.top = m_rectSplitter.bottom - BORDER_SIZE;
			else 
				m_rectSplitter.bottom = m_rectSplitter.top + BORDER_SIZE;
		}
		else 
		{
			if (m_dwStyle&CBRS_LEFT) 
				m_rectSplitter.left = m_rectSplitter.right - BORDER_SIZE;
			else 
				m_rectSplitter.right = m_rectSplitter.left + BORDER_SIZE;
		}
	}
	else
	{
		m_rectSplitter.SetRectEmpty ();
	}
}
//*************************************************************************************
void CBCGOutlookBar::SetTextColor (COLORREF clrRegText, COLORREF/* clrSelText obsolete*/)
{
	m_clrRegText = clrRegText;
	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
int CBCGOutlookBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetBarStyle (m_dwStyle & ~(CBRS_BORDER_ANY | CBRS_GRIPPER));

	m_cxLeftBorder = m_cxRightBorder = 0;
	m_cyTopBorder = m_cyBottomBorder = 0;

	//-------------------------------------------
	// Adjust Z-order in the parent frame window:
	//-------------------------------------------
	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;
	if (bVert)
	{
		SetWindowPos(&wndBottom, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
	else
	{
		CFrameWnd* pParent = GetParentFrame ();
		ASSERT (pParent != NULL);

		CWnd* pWndStatusBar = pParent->GetDlgItem (AFX_IDW_STATUS_BAR);

		if (pWndStatusBar != NULL)
		{
			SetWindowPos(pWndStatusBar, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}
	}

	//-----------------------
	// Create scroll buttons:
	//-----------------------
	CRect rectDummy (CPoint (0, 0), CMenuImages::Size ());
	rectDummy.InflateRect (BORDER_SIZE - 1, BORDER_SIZE - 1);

	m_btnUp.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, m_nIDScrollUp);
	m_btnUp.SetStdImage (bVert ? CMenuImages::IdArowUp : CMenuImages::IdArowRight);

	m_btnDown.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, m_nIDScrollDn);
	m_btnDown.SetStdImage (bVert ? CMenuImages::IdArowDown : CMenuImages::IdArowLeft);

	m_btnClose.Create (_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy,
					this, m_nIDClose);

	CString strClose;
	{
		CBCGLocalResource locaRes;
		strClose.LoadString (IDS_BCGBARRES_CLOSE);
	}

	m_btnClose.SetTooltip (strClose);
	m_btnClose.SetStdImage (CMenuImages::IdClose);

	SetWindowText (_T("Shortcuts"));

	if (m_bEnableSplitter)
	{
		//--------------------------
		// Initialize split cursors:
		//--------------------------
		if (globalData.m_hcurStretch == NULL)
		{
			globalData.m_hcurStretch = AfxGetApp ()->LoadCursor (AFX_IDC_HSPLITBAR);
		}

		if (globalData.m_hcurStretchVert == NULL)
		{
			globalData.m_hcurStretchVert = AfxGetApp ()->LoadCursor (AFX_IDC_VSPLITBAR);
		}
	}

	m_wndToolBar.CreateEx (this, TBSTYLE_FLAT,
		dwDefaultToolbarStyle, CRect(0, 0, 0, 0));
	m_wndToolBar.SetOwner (this);
	m_wndToolBar.SetRouteCommandsViaFrame (FALSE);

	m_wndToolBar.SetBarStyle (
		m_wndToolBar.GetBarStyle () & 
			~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	return 0;
}
//*************************************************************************************
CBCGOutlookButton* CBCGOutlookBar::GetButtonInPage (int iPageIndex) const
{
	int iIndex = 0;	// Index inside the active page

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (m_lstPages.IsEmpty () || pButton->m_uiPageID == m_uiActivePageID)
		{
			if (iIndex++ == iPageIndex)
			{
				return pButton;
			}
		}
	}

	return NULL;
}
//*************************************************************************************
int CBCGOutlookBar::GetPageCount () const
{
	int nCount = 0;

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (m_lstPages.IsEmpty () || pButton->m_uiPageID == m_uiActivePageID)
		{
			nCount++;
		}
	}

	return nCount;
}
//*************************************************************************************
void CBCGOutlookBar::ScrollUp ()
{
	if (m_iScrollOffset <= 0 ||
		m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer (m_nIDScrollUp);
		return;
	}

	CBCGOutlookButton* pFirstVisibleButton = GetButtonInPage (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	m_iFirstVisibleButton--;

	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;
	if (bVert)
	{
		m_iScrollOffset -= pFirstVisibleButton->Rect ().Height ();
	}
	else
	{
		m_iScrollOffset -= pFirstVisibleButton->Rect ().Width ();
	}

	if (m_iFirstVisibleButton == 0)
	{
		m_iScrollOffset = 0;
	}

	ASSERT (m_iScrollOffset >= 0);

	AdjustLocations ();
	InvalidateRect (m_rectWorkArea);
	UpdateWindow ();
}
//*************************************************************************************
void CBCGOutlookBar::ScrollDown ()
{
	if (!m_bScrollDown ||
		m_iFirstVisibleButton + 1 >= GetPageCount ())
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	CBCGOutlookButton* pFirstVisibleButton = GetButtonInPage (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	m_iFirstVisibleButton++;

	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;
	if (bVert)
	{
		m_iScrollOffset += pFirstVisibleButton->Rect ().Height ();
	}
	else
	{
		m_iScrollOffset += pFirstVisibleButton->Rect ().Width ();
	}

	AdjustLocations ();
	InvalidateRect (m_rectWorkArea);
	UpdateWindow ();
}
//*************************************************************************************
CSize CBCGOutlookBar::CalcFixedLayout (BOOL /*bStretch*/, BOOL bHorz)
{
	if (m_nSize == -1)
	{
		m_nSize = (bHorz) ? 2 * m_csImage.cy + 2 * (BORDER_SIZE - 1) + 2 * BORDER_SIZE:
						    2 * m_csImage.cx + 2 * (BORDER_SIZE - 1) + 2 * BORDER_SIZE;

		if (m_bTextLabels)
		{
			CClientDC dc (this);

			for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
			{
				CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
				ASSERT (pButton != NULL);

				CRect rectText (0, 0, 1, 1);
				dc.DrawText (pButton->m_strText, rectText, DT_CALCRECT | DT_WORDBREAK);

				if (!bHorz)
				{
					m_nSize = max (m_nSize, rectText.Width ());
				}
				else
				{
					m_nSize = max (m_nSize, rectText.Height ());
				}
			}
		}
	}

	CSize size = (bHorz) ? CSize (32767, m_nSize) : CSize (m_nSize, 32767);
	return size;
}
//*************************************************************************************
void CBCGOutlookBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CRect rect; 
	rect.SetRectEmpty();

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
	CControlBar::CalcInsideRect(rect, bHorz);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}
//*************************************************************************************
void CBCGOutlookBar::SetBackImage (UINT uiImageID)
{
	if (m_uiBackImageId == uiImageID)
	{
		return;
	}

	HBITMAP hbmp = NULL;
	if (uiImageID != 0)
	{
		hbmp = (HBITMAP) ::LoadImage (
				AfxGetResourceHandle (),
				MAKEINTRESOURCE (uiImageID),
				IMAGE_BITMAP,
				0, 0,
				LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
	}

	SetBackImage (hbmp);
	m_uiBackImageId = uiImageID;
}
//*************************************************************************************
void CBCGOutlookBar::SetBackImage (HBITMAP hbmp)
{
	m_bDrawShadedHighlight = FALSE;
	m_uiBackImageId = 0;

	if (m_bmpBack.GetCount () > 0)
	{
		m_bmpBack.Clear ();
	}

	if (hbmp != NULL)
	{
		BITMAP bitmap;
		::GetObject (hbmp, sizeof (BITMAP), (LPVOID) &bitmap);

		m_bmpBack.SetImageSize (CSize (bitmap.bmWidth, bitmap.bmHeight));
		m_bmpBack.AddImage (hbmp);

		m_bDrawShadedHighlight = (globalData.m_nBitsPerPixel > 8);	// For 16 bits or greater
	}

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGOutlookBar::SetBackColor (COLORREF color)
{
	m_clrBackColor = color;
	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGOutlookBar::SetTransparentColor (COLORREF color)
{
	m_clrTransparentColor = color;
	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGOutlookBar::OnSysColorChange() 
{
	CBCGToolBar::OnSysColorChange();
	
	m_clrBackColor = globalData.clrBarShadow;

	if (m_uiBackImageId > 0)
	{
		int uiImage = m_uiBackImageId;
		m_uiBackImageId = (UINT) -1;

		SetBackImage (uiImage);
	}
	else
	{
		Invalidate ();
	}
}
//*****************************************************************************************
void CBCGOutlookBar::AdjustLayout ()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	int nButtonsTextHeight = globalData.GetTextHeight () + 4;

	if (m_pFontButtons != NULL)
	{
		CClientDC dc (this);

		CFont* pOldFont = dc.SelectObject (m_pFontButtons);
		ASSERT_VALID (pOldFont);

		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);

		nButtonsTextHeight = tm.tmHeight + 8;

		dc.SelectObject (pOldFont);
	}

	m_nPageButtonHeight = max (m_sizePageImage.cy, nButtonsTextHeight) + 2;

	CRect rectClient;
	GetClientRect (&rectClient);

	if (m_bCaption || m_bMode2003)
	{
		BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;

		m_rectCaption = rectClient;
		m_rectCaption.DeflateRect (1, 1);

		if (bHorz)
		{
			m_rectCaption.right = m_rectCaption.left + nButtonsTextHeight + 3;

			if (m_bEnableSplitter)
			{
				if (m_dwStyle & CBRS_BOTTOM)
				{
					m_rectCaption.top += BORDER_SIZE;
				}
				else
				{
					m_rectCaption.bottom -= BORDER_SIZE;
				}
			}
		}
		else
		{
			m_rectCaption.bottom = m_rectCaption.top + nButtonsTextHeight + 2;

			if (m_bEnableSplitter)
			{
				if (m_dwStyle & CBRS_LEFT)
				{
					m_rectCaption.right -= BORDER_SIZE + 1;
				}
				else
				{
					m_rectCaption.left += BORDER_SIZE + 1;
				}
			}
		}

		if (m_rectCaption.right > rectClient.right ||
			m_rectCaption.bottom > rectClient.bottom)
		{
			m_rectCaption.SetRectEmpty ();
		}
	}
	else
	{
		m_rectCaption.SetRectEmpty ();
	}

	CBCGToolBar::AdjustLayout ();
}
//*****************************************************************************************
void CBCGOutlookBar::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID(this);

	m_btnClose.SendMessage (WM_CANCELMODE);

	int nToolBarHeight = 0;

	m_rectToolBar.SetRectEmpty ();

	if (m_bMode2003)
	{
		CSize sizeImage (0, 0);
			
		if (m_imagesToolbar.GetSafeHandle () != NULL)
		{
			sizeImage = m_sizeToolbarImage;
		}
		else if (m_imagesPages.GetSafeHandle () != NULL)
		{
			sizeImage = m_sizePageImage;
		}
		else
		{
			sizeImage = CSize (16, 16);
		}

		if (CBCGToolBar::IsLargeIcons ())
		{
			sizeImage.cx *= 2;
			sizeImage.cy *= 2;
		}

		nToolBarHeight = sizeImage.cy + 6 + 2 * nToolbarMarginHeight - 2;
	}

	CSize sizeBtn = CMenuImages::Size () + CSize (BORDER_SIZE - 1, BORDER_SIZE - 1);

	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;

	CClientDC dc (this);
	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);

	CRect rectClient;
	GetWorkArea (rectClient);

	CSize sizeDefault;

	if (bVert)
	{
		rectClient.DeflateRect (2, 0);

		if (m_rectCaption.IsRectEmpty ())
		{
			rectClient.top ++;
		}

		rectClient.bottom --;
		if (m_bEnableSplitter)
		{
			rectClient.right -= BORDER_SIZE;
		}

		sizeDefault = CSize (rectClient.Width (), m_csImage.cy);
	}
	else
	{
		rectClient.DeflateRect (0, 1);
		rectClient.right --;

		if (m_rectCaption.IsRectEmpty ())
		{
			rectClient.left ++;
		}

		if (m_bEnableSplitter)
		{
			rectClient.top += BORDER_SIZE;
		}

		sizeDefault = CSize (m_csImage.cx, rectClient.Height ());
	}

	if (IsButtonExtraSizeAvailable ())
	{
		sizeDefault += CBCGVisualManager::GetInstance ()->GetButtonExtraBorder ();
	}

	int nToolbarPagesNum = 0;

	if (m_bMode2003)
	{
		const int nPagesCount = (int) m_lstPages.GetCount ();

		m_rectToolBar = rectClient;
		m_rectToolBar.top = m_rectToolBar.bottom - nToolBarHeight;

		rectClient.bottom = m_rectToolBar.top;

		if (m_nVisiblePageButtons == -1)
		{
			m_nVisiblePageButtons = nPagesCount;
		}

		if (m_nVisiblePageButtons > nPagesCount)
		{
			// Maybe, pages were removed?
			m_nVisiblePageButtons = nPagesCount;
		}

		m_nMaxVisiblePageButtons = m_nPageButtonHeight == 0 ? 0 :
			min (nPagesCount,	(rectClient.Height () - m_nPageButtonHeight) / 
								(2 * m_nPageButtonHeight));
		m_nVisiblePageButtons = min (m_nMaxVisiblePageButtons, m_nVisiblePageButtons);

		nToolbarPagesNum = nPagesCount - m_nVisiblePageButtons;

		m_rectSplitterPage = rectClient;
	}
	else
	{
		m_rectSplitterPage.SetRectEmpty ();
		m_rectToolBar.SetRectEmpty ();
	}

	//------------------
	// Set page buttons:
	//------------------
	if (!m_bMode2003)
	{
		for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
		{
			CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
			ASSERT_VALID (pPage);

			CRect rectPage = rectClient;

			rectPage.bottom = rectPage.top + m_nPageButtonHeight;
			rectClient.top = rectPage.bottom;

			pPage->SetRect (rectPage);
			pPage->m_btnPage.m_nAlignStyle = CBCGButton::ALIGN_CENTER;
			pPage->m_btnPage.SetFont (m_pFontButtons);
			pPage->m_btnPage.m_bChecked = FALSE;

			if (pPage->m_uiID == m_uiActivePageID)	// Last top page
			{
				break;
			}
		}
	}

	int nPageCount = 0;
	POSITION pos = NULL;

	for (pos = m_lstPages.GetTailPosition (); pos != NULL; nPageCount++)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetPrev (pos);
		ASSERT_VALID (pPage);

		if (nPageCount < nToolbarPagesNum)
		{
			pPage->SetRect (CRect (0, 0, 0, 0));
			continue;
		}

		if (!m_bMode2003 && pPage->m_uiID == m_uiActivePageID)	// Last top page
		{
			break;
		}

		CRect rectPage = rectClient;

		rectPage.top = rectPage.bottom - m_nPageButtonHeight;
		rectClient.bottom = rectPage.top;

		pPage->SetRect (rectPage);
		pPage->m_btnPage.m_nAlignStyle = 
			m_bMode2003 ? CBCGButton::ALIGN_LEFT : CBCGButton::ALIGN_CENTER;
		pPage->m_btnPage.SetFont (m_pFontButtons);

		pPage->m_btnPage.m_bChecked = m_bMode2003 && pPage->m_uiID == m_uiActivePageID;
	}

	if (m_bMode2003)
	{
		m_rectSplitterPage.bottom = rectClient.bottom;
		m_rectSplitterPage.top = m_rectSplitterPage.bottom - nSplitterHeight;

		rectClient.bottom -= nSplitterHeight;
	}

	int iOffset = bVert ? 
				rectClient.top - m_iScrollOffset + m_nExtraSpace:
				rectClient.left - m_iScrollOffset + m_nExtraSpace;

	if (m_iFirstVisibleButton > 0 && 
		sizeBtn.cx <= rectClient.Width () - SCROLL_BUTTON_OFFSET && 
		sizeBtn.cy <= rectClient.Height () - SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = SCROLL_BUTTON_OFFSET;

		if (m_bEnableSplitter)
		{
			nAdjButton *= 2;
		}

		m_btnUp.SetWindowPos (NULL, 
			bVert ? 
				rectClient.right - sizeBtn.cx - nAdjButton :
				rectClient.left + nAdjButton,
				rectClient.top + SCROLL_BUTTON_OFFSET,
				-1, -1,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

		m_btnUp.ShowWindow (SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnUp.ShowWindow (SW_HIDE);
	}

	for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (!m_lstPages.IsEmpty () &&
			pButton->m_uiPageID != m_uiActivePageID)
		{
			// Hide the button:
			pButton->SetRect (CRect (-1, -1, -1, -1));
			continue;
		}

		pButton->m_bTextBelow = m_bTextLabels;
		pButton->m_sizeImage = m_csImage;

		CSize sizeButton = pButton->OnCalculateSize (&dc, sizeDefault, !bVert);

		CRect rectButton;

		if (bVert)
		{
			int nWidth = rectClient.Width () - 1;
			sizeButton.cx = min (nWidth, sizeButton.cx);

			rectButton = CRect (
				CPoint (rectClient.left + (nWidth - sizeButton.cx) / 2, iOffset), 
				sizeButton);
			iOffset = rectButton.bottom + m_nExtraSpace;
		}
		else
		{
			int nHeight = rectClient.Height () - 1;
			sizeButton.cy = min (nHeight, sizeButton.cy);

			rectButton = CRect (CPoint (rectClient.left + iOffset, 
								rectClient.top + (nHeight - sizeButton.cy) / 2),
								sizeButton);
			iOffset = rectButton.right + m_nExtraSpace;
		}

		pButton->SetRect (rectButton);
	}

	CBCGOutlookBarPage* pActivePage = GetPage (m_uiActivePageID);
	if (pActivePage != NULL && pActivePage->m_hwndControl != NULL)
	{
		::SetWindowPos (pActivePage->m_hwndControl, NULL,
			rectClient.left, rectClient.top, 
			rectClient.Width (), rectClient.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if (bVert)
	{
		m_bScrollDown = (iOffset > rectClient.bottom);
	}
	else
	{
		m_bScrollDown = (iOffset > rectClient.right);
	}

	if (m_bScrollDown && 
		sizeBtn.cx <= rectClient.Width () - SCROLL_BUTTON_OFFSET && 
		sizeBtn.cy <= rectClient.Height () - SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = SCROLL_BUTTON_OFFSET;

		if (m_bEnableSplitter)
		{
			nAdjButton *= 2;
		}

		m_btnDown.SetWindowPos (&wndTop, rectClient.right - sizeBtn.cx - nAdjButton,
					bVert ?	
					rectClient.bottom - sizeBtn.cy - SCROLL_BUTTON_OFFSET :
					rectClient.top + SCROLL_BUTTON_OFFSET,
					-1, -1,
					SWP_NOSIZE | SWP_NOACTIVATE);

		m_btnDown.ShowWindow (SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnDown.ShowWindow (SW_HIDE);
	}

	if (!m_rectCaption.IsRectEmpty ())
	{
		int x = bVert ? 
			m_rectCaption.right - m_rectCaption.Height () + BORDER_SIZE:
			m_rectCaption.left + BORDER_SIZE;
		int y = m_rectCaption.top + BORDER_SIZE;
		int size = bVert ? 
			m_rectCaption.Height () - 2 * BORDER_SIZE :
			m_rectCaption.Width () - 2 * BORDER_SIZE;

		if (m_bEnableCloseButton && !m_bMode2003 &&
			((bVert && x > m_rectCaption.left) ||
			(!bVert && y + size < m_rectCaption.bottom)))
		{
			CRect rectClose;
			m_btnClose.GetClientRect (&rectClose);
			m_btnClose.MapWindowPoints (this, &rectClose);

			if (rectClose.left != x || rectClose.top != y)
			{
				m_btnClose.SetWindowPos (&wndTop, x, y,
							size, size,
							SWP_NOACTIVATE);
				m_btnClose.ShowWindow (SW_SHOWNOACTIVATE);
			}
		}
		else
		{
			m_btnClose.ShowWindow (SW_HIDE);
		}
	}
	else
	{
		m_btnClose.ShowWindow (SW_HIDE);
		m_btnClose.SetWindowPos (&wndTop, 0, 0,
					0, 0,
					SWP_NOACTIVATE);
	}

	dc.SelectObject (pOldFont);

	// Redraw page buttons:
	for (POSITION posPages = m_lstPages.GetHeadPosition (); posPages != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (posPages);
		ASSERT_VALID (pPage);

		if (pPage->m_btnPage.GetSafeHwnd () != NULL)
		{
			pPage->m_btnPage.RedrawWindow ();
		}
	}

	if (m_bMode2003)
	{
		m_wndToolBar.ShowWindow (SW_SHOWNOACTIVATE);
		m_wndToolBar.SetWindowPos (NULL, 
			m_rectToolBar.left, m_rectToolBar.top,
			m_rectToolBar.Width (), m_rectToolBar.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
		RebuildToolBar ();
	}
	else
	{
		m_wndToolBar.ShowWindow (SW_HIDE);
	}

	m_btnUp.RedrawWindow ();
	m_btnDown.RedrawWindow ();

	m_rectWorkArea = rectClient;
	OnMouseLeave (0, 0);
}
//*************************************************************************************
void CBCGOutlookBar::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;
	int nClipType = pDCPaint->GetClipBox (rectClip);
	if (nClipType == NULLREGION)
	{
		return;
	}

	if (nClipType != SIMPLEREGION)
	{
		GetClientRect (rectClip);
	}

	rectClip.top = max (rectClip.top, m_rectWorkArea.top);
	rectClip.bottom = min (rectClip.bottom, m_rectWorkArea.bottom);
	rectClip.left = max (rectClip.left, m_rectWorkArea.left);
	rectClip.right = min (rectClip.right, m_rectWorkArea.right);

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;

	CRect rectClient;
	GetClientRect (rectClient);

	CDC*		pDC = pDCPaint;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC (pDCPaint) &&
		bmp.CreateCompatibleBitmap (pDCPaint, rectClient.Width (),
								  rectClient.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

	//-----------------------
	// RGL: Draw the margins
	//-----------------------
	CRect rectWindow = rectClient;

	//-----------------------------------
	//RGL: To remove the splitter side we
	//have to see were we are docked
	//-----------------------------------
	if (m_bEnableSplitter)
	{
		if (bHorz)
		{	
			if (m_dwStyle & CBRS_TOP)
			{
				rectClient.bottom -= BORDER_SIZE;
			}
			else
			{
				rectClient.top += BORDER_SIZE;
			}
		}
		else 
		{
			if (m_dwStyle & CBRS_LEFT)
			{
				rectClient.right -= BORDER_SIZE;
			}
			else 
			{
				rectClient.left += BORDER_SIZE;
			}
		}
	}

	CRect rectMargins = rectClient;

	if (m_bFlatBorder)
	{
		pDCPaint->Draw3dRect (rectClient, globalData.clrBarFace, globalData.clrBarFace);

		CRect rectInner = rectClient;
		rectInner.DeflateRect (1, 1);

		pDCPaint->Draw3dRect (rectInner, globalData.clrBarHilite, globalData.clrBarShadow);
	}
	else
	{
		pDCPaint->DrawEdge (rectClient, EDGE_SUNKEN, BF_RECT | BF_ADJUST); //draw the edge and inflates the rect
	}

	//----------------------
	//RGL: Draw the splitter
	//----------------------
	if (m_bEnableSplitter)
	{
		pDCPaint->FillRect (m_rectSplitter, &globalData.brBarFace);
	}

	//--------------
	// Draw caption:
	//--------------
	if (!m_rectCaption.IsRectEmpty ())
	{
		CString strTitle;

		if (m_bMode2003 && m_uiActivePageID != (UINT)-1)
		{
			LPCTSTR lpszTitle = GetPageName (m_uiActivePageID);
			if (lpszTitle != NULL)
			{
				strTitle = lpszTitle;
			}
		}
		else
		{
			GetWindowText (strTitle);
		}

		pDCPaint->SetBkMode(TRANSPARENT);
		CFont* pOldFont = pDCPaint->SelectObject
			(bHorz ? &globalData.fontVertCaption : 
			(m_pFontButtons != NULL ? m_pFontButtons : &globalData.fontRegular));

		pDCPaint->SetTextColor (globalData.clrBarText);
		OnDrawCaption (pDCPaint, m_rectCaption, strTitle, bHorz);

		pDCPaint->SelectObject(pOldFont);
	}

	//---------------------
	// Draw pages splitter:
	//---------------------
	if (!m_rectSplitterPage.IsRectEmpty ())
	{
		CBCGVisualManager::GetInstance ()->OnDrawOutlookBarSplitter (pDCPaint,
			m_rectSplitterPage);
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (m_rectWorkArea);
	pDC->SelectClipRgn (&rgn);

	CBCGOutlookBarPage* pActivePage = GetPage (m_uiActivePageID);
	if (pActivePage == NULL || pActivePage->m_hwndControl == NULL)
	{
		OnEraseWorkArea (pDC, m_rectWorkArea);
	}

	if (!m_Buttons.IsEmpty ())
	{
		pDC->SetTextColor (globalData.clrBarText);
		pDC->SetBkMode (TRANSPARENT);

		m_Images.SetTransparentColor (m_clrTransparentColor);

		CBCGDrawState ds;
		if (!m_Images.PrepareDrawImage (ds))
		{
			ASSERT (FALSE);
			return;     // something went wrong
		}

		CFont* pOldFont = pDC->SelectObject (&globalData.fontRegular);

		//--------------
		// Draw buttons:
		//--------------
		int iButton = 0;
		for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL; iButton ++)
		{
			CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
			ASSERT_VALID (pButton);

			CRect rect = pButton->Rect ();

			BOOL bHighlighted = FALSE;

			if (IsCustomizeMode () && !m_bLocked)
			{
				bHighlighted = FALSE;
			}
			else
			{
				bHighlighted = ((iButton == m_iHighlighted ||
								iButton == m_iButtonCapture) &&
								(m_iButtonCapture == -1 ||
								iButton == m_iButtonCapture));
			}

			CRect rectInter;
			if (rectInter.IntersectRect (rect, rectClip))
			{
				pButton->OnDraw (pDC, rect, &m_Images, bHorz, IsCustomizeMode (),
								bHighlighted);
			}
		}

		//-------------------------------------------------------------
		// Highlight selected button in the toolbar customization mode:
		//-------------------------------------------------------------
		if (m_iSelected >= m_Buttons.GetCount ())
		{
			m_iSelected = -1;
		}

		if (IsCustomizeMode () && m_iSelected >= 0 && !m_bLocked)
		{
			CBCGToolbarButton* pSelButton = GetButton (m_iSelected);
			ASSERT (pSelButton != NULL);

			if (pSelButton != NULL && pSelButton->CanBeStored ())
			{
				CRect rectDrag = pSelButton->Rect ();
				if (pSelButton->GetHwnd () != NULL)
				{
					rectDrag.InflateRect (0, 1);
				}

				pDC->DrawDragRect (&rectDrag, CSize (2, 2), NULL, CSize (2, 2));
			}
		}

		if (IsCustomizeMode () && m_iDragIndex >= 0 && !m_bLocked)
		{
			DrawDragMarker (pDC);
		}

		pDC->SelectClipRgn (NULL);
		pDC->SelectObject (pOldFont);

		m_Images.EndDrawImage (ds);
	}

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		pDCPaint->BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
					   &dcMem, rectClip.left, rectClip.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}

	//-----------------------------
	//RGL: At last draw the margins
	//-----------------------------
	pDCPaint->ExcludeClipRect(&rectMargins);

	if (m_bEnableSplitter)
	{
		pDCPaint->ExcludeClipRect(m_rectSplitter);
	}

	pDCPaint->FillSolidRect(&rectWindow, globalData.clrBarFace);
}
//****************************************************************************************
INT_PTR CBCGOutlookBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	if (m_rectToolBar.PtInRect (point))
	{
		ClientToScreen (&point);
		m_wndToolBar.ScreenToClient (&point);

		return m_wndToolBar.OnToolHitTest (point, pTI);
	}

	return CBCGToolBar::OnToolHitTest (point, pTI);
}
//****************************************************************************************
DROPEFFECT CBCGOutlookBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CBCGOutlookBarPage* pPage = GetPageFromPoint (point);
	UINT uiDelayedPageID = 0;

	if (pPage != NULL &&
		pPage->m_uiID != m_uiActivePageID &&
		pPage->m_hwndControl == NULL)
	{
		uiDelayedPageID = pPage->m_uiID;
	}

	if (m_uiDelayedPageID != uiDelayedPageID)
	{
		m_uiDelayedPageID = uiDelayedPageID;
		KillTimer (m_nIDChangePage);

		if (m_uiDelayedPageID != 0)
		{
			SetTimer (m_nIDChangePage, uiCagngePageDelay, NULL); 
		}
	}

	if (pPage != NULL)
	{
		return m_uiDelayedPageID != 0 ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
	}

	CBCGToolbarButton* pButton = CBCGToolbarButton::CreateFromOleData (pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bAllowDrop = pButton->IsKindOf (RUNTIME_CLASS (CBCGOutlookButton));
	delete pButton;

	if (!bAllowDrop)
	{
		return DROPEFFECT_NONE;
	}

	CRect rectClient;
	GetWorkArea (rectClient);

	if (point.y < rectClient.top)
	{
		ScrollUp ();
		return DROPEFFECT_NONE;
	}

	if (point.y > rectClient.bottom)
	{
		ScrollDown ();
		return DROPEFFECT_NONE;
	}

	return CBCGToolBar::OnDragOver (pDataObject, dwKeyState, point);
}
//***************************************************************************************
CBCGToolbarButton* CBCGOutlookBar::CreateDroppedButton (COleDataObject* pDataObject)
{
	CBCGToolbarButton* pButton = CBCGToolBar::CreateDroppedButton (pDataObject);
	ASSERT (pButton != NULL);

	CBCGOutlookButton* pOutlookButton = DYNAMIC_DOWNCAST (CBCGOutlookButton, pButton);
	if (pOutlookButton == NULL)
	{
		delete pButton;

		ASSERT (FALSE);
		return NULL;
	}

	pOutlookButton->m_uiPageID = m_uiActivePageID;
	return pButton;
}
//***************************************************************************************
BOOL CBCGOutlookBar::EnableContextMenuItems (CBCGToolbarButton* pButton, CMenu* pPopup)
{
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPopup);

	if (IsCustomizeMode ())
	{
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_APPEARANCE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_START_GROUP, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_RESET, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem (ID_BCGBARRES_COPY_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	}

	CBCGToolBar::EnableContextMenuItems (pButton, pPopup);
	return TRUE;
}
//**************************************************************************************
BOOL CBCGOutlookBar::PreTranslateMessage(MSG* pMsg) 
{
	if (m_pInPlaceEdit != NULL)
	{
		if (pMsg->message >= WM_KEYFIRST &&
			pMsg->message <= WM_KEYLAST)
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				{
					CString strName;
					m_pInPlaceEdit->GetWindowText (strName);

					if (strName.IsEmpty ())
					{
						MessageBeep ((UINT)-1);
						return TRUE;
					}

					SetPageName (m_uiEditedPageID, strName);
				}

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
				m_pInPlaceEdit->DestroyWindow ();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				ReleaseCapture ();
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

   	switch (pMsg->message)
	{
	case WM_LBUTTONUP:
		KillTimer (m_nIDScrollUp);
		KillTimer (m_nIDScrollDn);

	case WM_LBUTTONDOWN:
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);

			if (m_rectActivePageBtn.PtInRect (ptCursor))
			{
				return TRUE;
			}
		}

	case WM_MOUSEMOVE:
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			CRect rect;
			m_btnDown.GetClientRect (rect);
			m_btnDown.MapWindowPoints (this, rect);

			if (rect.PtInRect (ptCursor))
			{
				m_btnDown.SendMessage (pMsg->message, pMsg->wParam, pMsg->wParam);
				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer (m_nIDScrollDn, uiScrollDelay, NULL);

					if(m_bPageScrollMode)
					{
						ScrollPageDown ();
					}else
					{
						ScrollDown ();
					}
				}
			}

			m_btnUp.GetClientRect (rect);
			m_btnUp.MapWindowPoints (this, rect);

			if (rect.PtInRect (ptCursor))
			{
				m_btnUp.SendMessage (pMsg->message, pMsg->wParam, pMsg->wParam);

				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer (m_nIDScrollUp, uiScrollDelay, NULL);

					if(m_bPageScrollMode)
					{
						ScrollPageUp ();
					}else
					{
						ScrollUp ();
					}
				}
			}
		}
		break;

	case WM_KEYDOWN:
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (m_bIsTracking)
			{
				StopTracking ();
			}

			// CBCGToolBar "eats" ESC key
			return CControlBar::PreTranslateMessage(pMsg);
		}
		break;
	}

	return CBCGToolBar::PreTranslateMessage(pMsg);
}
//**************************************************************************************
void CBCGOutlookBar::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case m_nIDScrollUp:
		if (m_btnUp.IsPressed ())
		{
			if(m_bPageScrollMode)
			{
				ScrollPageUp ();

			}else
			{
				ScrollUp ();
			}
			
		}
		return;

	case m_nIDScrollDn:
		if (m_btnDown.IsPressed ())
		{
			if(m_bPageScrollMode)
			{
				ScrollPageDown ();

			}else
			{
				ScrollDown ();
			}
		}
		return;

	case m_nIDChangePage:
		if (m_uiDelayedPageID != 0)
		{
			CPoint point;

			::GetCursorPos (&point);
			ScreenToClient (&point);

			CBCGOutlookBarPage* pPage = GetPageFromPoint (point);
			if (pPage != NULL && pPage->m_uiID == m_uiDelayedPageID)
			{
				SetActivePage (m_uiDelayedPageID);
			}

			m_uiDelayedPageID = 0;
		}

		KillTimer (m_nIDChangePage);
		return;
	}

	CBCGToolBar::OnTimer(nIDEvent);
}
//**************************************************************************************
BOOL CBCGOutlookBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		CPoint point;

		::GetCursorPos (&point);
		ScreenToClient (&point);

		if (m_bIsTracking || m_rectSplitter.PtInRect (point))
		{
			BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
			SetCursor (!bHorz ?
				globalData.m_hcurStretch : globalData.m_hcurStretchVert);
			return TRUE;
		}

		if (m_rectSplitterPage.PtInRect (point))
		{
			SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZENS));
			return TRUE;
		}
	}
	
	return CBCGToolBar::OnSetCursor(pWnd, nHitTest, message);
}
//**************************************************************************************
void CBCGOutlookBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bIsTracking && m_bEnableSplitter)
	{
		BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
		m_nSplitterOffset = bHorz ? point.y : point.x;

		ShowInvertSplitter ();
		return;
	}

	if (m_bIsTrackingPages)
	{
		if (m_nVisiblePageButtons == -1)
		{
			return;
		}

		int nDelta = (m_rectSplitterPage.top - point.y) / m_nPageButtonHeight;
		if (nDelta == 0)
		{
			return;
		}

		int nVisiblePageButtonsPrev = m_nVisiblePageButtons;

		m_nVisiblePageButtons += nDelta;

		m_nVisiblePageButtons = min ((int) m_lstPages.GetCount (), 
			max (0, m_nVisiblePageButtons));

		if (nVisiblePageButtonsPrev != m_nVisiblePageButtons)
		{
			//FIX
			if (m_nVisiblePageButtons > m_nMaxVisiblePageButtons)
			{
				m_nVisiblePageButtons = nVisiblePageButtonsPrev;
				return;
			}

			AdjustLocations ();
			RedrawWindow ();

			point.y = m_rectSplitterPage.CenterPoint ().y;
			ClientToScreen (&point);

			::SetCursorPos (point.x, point.y);
		}

		return;
	}

	CBCGToolBar::OnMouseMove(nFlags, point);
}
//**************************************************************************************
void CBCGOutlookBar::ShowInvertSplitter ()
{
	CPoint point (m_nSplitterOffset, m_nSplitterOffset);
	ClientToScreen (&point);

	CWnd* pParentWnd = GetParent();
	ASSERT_VALID (pParentWnd);

	CRect rectParent;
	pParentWnd->GetClientRect (&rectParent);
	pParentWnd->ClientToScreen (&rectParent);

	//----------------------------------------------------
	//Make sure that the gripper stays in a valid position
	//----------------------------------------------------
	if (m_dwStyle & CBRS_ORIENT_HORZ)
	{
		point.y = max (rectParent.top, min (rectParent.bottom, point.y));
	}
	else 
	{
		point.x = max (rectParent.left, min (rectParent.right, point.x));
	}

	CPoint ptClient = point;
	ScreenToClient (&ptClient);
	m_nSplitterOffset = (m_dwStyle & CBRS_ORIENT_HORZ) ? ptClient.y : ptClient.x;

	CRect rectSpliterSrc;
	GetClientRect (&rectSpliterSrc);
	ClientToScreen (&rectSpliterSrc);

	if (m_dwStyle & CBRS_ORIENT_HORZ)
	{
		rectSpliterSrc.DeflateRect (2, 0);
		if (m_dwStyle & CBRS_BOTTOM)
		{
			rectSpliterSrc.top = point.y + BORDER_SIZE;
			rectSpliterSrc.bottom = rectSpliterSrc.top + BORDER_SIZE;
		}
		else
		{
			rectSpliterSrc.bottom = point.y;
			rectSpliterSrc.top = rectSpliterSrc.bottom - BORDER_SIZE;
		}
	}
	else
	{
		rectSpliterSrc.DeflateRect (0, 2);
		if (m_dwStyle&CBRS_LEFT)
		{
			rectSpliterSrc.left = point.x;
			rectSpliterSrc.right = point.x + BORDER_SIZE;
		}
		else
		{
			rectSpliterSrc.right = point.x;
			rectSpliterSrc.left = point.x - BORDER_SIZE;
		}
	}

	CClientDC dc (NULL);

	//------------------------------------------------------
	// If there is a splitter drawed in screen just erase it
	//------------------------------------------------------
	if (!m_rectMoveSplitterScreen.IsRectEmpty ())
	{
		DrawTracker( &dc, m_rectMoveSplitterScreen);
	}

	m_rectMoveSplitterScreen = rectSpliterSrc;
	DrawTracker (&dc, m_rectMoveSplitterScreen);
}
//**************************************************************************************
void CBCGOutlookBar::StopTracking ()
{
	if (m_bIsTracking)
	{
		CClientDC dc (NULL);
		DrawTracker(&dc, m_rectMoveSplitterScreen);

		ReleaseCapture ();
		RestoreFocus ();

		m_bIsTracking = FALSE;
		m_rectMoveSplitterScreen.SetRectEmpty ();
		m_nSplitterOffset = 0;

		::ShowCursor (TRUE);
	}
}
//**************************************************************************************
void CBCGOutlookBar::OnCancelMode() 
{
	CBCGToolBar::OnCancelMode();

	if (m_pInPlaceEdit != NULL)
	{
		m_pInPlaceEdit->DestroyWindow ();
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;
		ReleaseCapture ();
	}

	StopTracking ();

	if (m_bIsTrackingPages)
	{
		ReleaseCapture ();
		m_bIsTrackingPages = FALSE;

		::ShowCursor (TRUE);
	}
}
//*******************************************************************************
void CBCGOutlookBar::OnCaptureChanged(CWnd *pWnd) 
{
	if (m_bIsTrackingPages)
	{
		ReleaseCapture ();
		m_bIsTrackingPages = FALSE;

		::ShowCursor (TRUE);
	}
	
	CBCGToolBar::OnCaptureChanged(pWnd);
}

/////////////////////////////////////////////////////////////////////////////
// COutlookCustomizeButton

class COutlookCustomizeButton : public CCustomizeButton
{
	DECLARE_DYNCREATE(COutlookCustomizeButton)

	friend class CBCGOutlookBar;

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);
	virtual CBCGPopupMenu* CreatePopupMenu ();
	virtual BOOL HaveHotBorder () const			{	return TRUE;	}
};

IMPLEMENT_DYNCREATE(COutlookCustomizeButton, CCustomizeButton)

CBCGPopupMenu* COutlookCustomizeButton::CreatePopupMenu ()
{
	CBCGPopupMenu* pMenu = CCustomizeButton::CreatePopupMenu ();
	if (pMenu == NULL)
	{
		return NULL;
	}

	pMenu->RemoveItem (pMenu->GetMenuBar ()->CommandToIndex (m_uiCustomizeCmdId));

	if (pMenu->GetMenuItemCount () > 0)
	{
		pMenu->InsertSeparator ();
	}

	CBCGLocalResource locaRes;
	CString strItem;

	strItem.LoadString (IDS_BCGBARRES_SHOW_MORE_BUTTONS);
	pMenu->InsertItem (CBCGToolbarMenuButton (idShowMoreButtons, NULL, -1, strItem));

	strItem.LoadString (IDS_BCGBARRES_SHOW_FEWER_BUTTONS);
	pMenu->InsertItem (CBCGToolbarMenuButton (idShowFewerButtons, NULL, -1, strItem));

	return pMenu;
}

void COutlookCustomizeButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
			BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
			BOOL /*bDrawBorder*/, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	m_bDefaultDraw = TRUE;

	CBCGToolbarButton::OnDraw (pDC, rect, pImages,
			bHorz, bCustomizeMode, bHighlight,
			CBCGVisualManager::GetInstance ()->IsOutlookToolbarHotBorder (), 
			bGrayDisabledButtons);

	CSize sizeImage = CMenuImages::Size ();

	int x = rect.left + max (0, (rect.Width () - sizeImage.cx) / 2);
	int y = rect.top + max (0, (rect.Height () - 2 * sizeImage.cy) / 2);

	CMenuImages::Draw (pDC, CMenuImages::IdMoreButtons, CPoint (x, y));

	y += sizeImage.cy;

	CMenuImages::Draw (pDC, CMenuImages::IdArowDown, CPoint (x, y));
}

void CBCGOutlookBar::RebuildToolBar ()
{
	ASSERT_VALID (this);

	if (!m_bMode2003)
	{
		return;
	}

	m_wndToolBar.RemoveAllButtons ();
	m_wndToolBar.m_TabButtons.RemoveAll ();

	m_wndToolBar.EnableCustomizeButton (TRUE, 0, _T(""), FALSE);

	CSize sizeImage (0, 0);
		
	if (m_imagesToolbar.GetSafeHandle () != NULL)
	{
		sizeImage = m_sizeToolbarImage;
	}
	else if (m_imagesPages.GetSafeHandle () != NULL)
	{
		sizeImage = m_sizePageImage;
	}
	else
	{
		sizeImage = CSize (16, 16);
	}

	CSize sizeButton = sizeImage + CSize (6, 6 + 2 * nToolbarMarginHeight);
	m_wndToolBar.SetLockedSizes (sizeButton, sizeImage);
	m_wndToolBar.m_ImagesLocked.Clear ();
	m_wndToolBar.m_ImagesLocked.SetImageSize (sizeImage);

	if (m_wndToolBar.m_pCustomizeBtn != NULL)
	{
		COutlookCustomizeButton customizeButton;
		customizeButton.CopyFrom (*m_wndToolBar.m_pCustomizeBtn);

		customizeButton.SetPipeStyle (FALSE);
		customizeButton.SetMenuRightAlign (FALSE);
		customizeButton.SetMessageWnd (this);
		customizeButton.m_bShowAtRightSide = TRUE;

		m_wndToolBar.m_Buttons.RemoveHead ();
		delete m_wndToolBar.m_pCustomizeBtn;
		m_wndToolBar.m_pCustomizeBtn = NULL;

		m_wndToolBar.InsertButton (customizeButton);

		m_wndToolBar.m_pCustomizeBtn = (CCustomizeButton*) m_wndToolBar.m_Buttons.GetHead ();
	}

	int nButtonNum = 0;
	int i = 0;
	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL; i++)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (i >= m_nVisiblePageButtons)
		{
			CBCGToolbarButton button (idToolbarCommandID + nButtonNum, nButtonNum, pPage->m_strName);
			m_wndToolBar.InsertButton (button);

			m_wndToolBar.m_TabButtons.SetAt (nButtonNum, pPage->m_uiID);

			HICON hIcon = NULL;
			
			if (m_imagesToolbar.GetSafeHandle () != NULL)
			{
				hIcon = m_imagesToolbar.ExtractIcon (i);
			}
			else if (m_imagesPages.GetSafeHandle () != NULL)
			{
				hIcon = m_imagesPages.ExtractIcon (i);
			}

			m_wndToolBar.m_ImagesLocked.AddIcon (hIcon);
			::DestroyIcon (hIcon); // Free the icon
			nButtonNum++;
		}
	}

	m_wndToolBar.SetHotBorder (CBCGVisualManager::GetInstance ()->IsOutlookToolbarHotBorder ());

	m_wndToolBar.AdjustLocations ();
	m_wndToolBar.RedrawWindow ();
}
//*******************************************************************************
void CBCGOutlookBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bEnableSplitter && m_rectSplitter.PtInRect (point))
	{
		SetCapture ();
		m_hwndLastFocus = SetFocus ()->GetSafeHwnd ();

		m_bIsTracking = TRUE;

		BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
		m_nSplitterOffset = bHorz ? point.y : point.x;

		ShowInvertSplitter ();
		return;
	}

	if (m_rectSplitterPage.PtInRect (point))
	{
		m_bIsTrackingPages = TRUE;
		SetCapture ();
		return;
	}
	
	CBCGToolBar::OnLButtonDown(nFlags, point);
}
//*******************************************************************************
void CBCGOutlookBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsTrackingPages)
	{
		ReleaseCapture ();
		::ShowCursor (TRUE);
		m_bIsTrackingPages = FALSE;
	}
	else if (m_bIsTracking && m_bEnableSplitter)
	{
		CRect rectClient;
		GetClientRect (&rectClient);

		if (m_dwStyle & (CBRS_LEFT | CBRS_TOP))
		{
			m_nSize = m_nSplitterOffset + BORDER_SIZE;
		}
		else if (m_dwStyle & CBRS_RIGHT)
		{
			m_nSize = rectClient.right - m_nSplitterOffset;
		}
		else
		{
			m_nSize = rectClient.bottom - m_nSplitterOffset;
		}

		m_nSize = max (BORDER_SIZE, m_nSize);

		StopTracking ();

		CFrameWnd* pParentFrame = GetParentFrame ();
		if (pParentFrame == NULL)
		{
			SetWindowPos (NULL, 0, 0, m_nSize, rectClient.Height (),
						SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			pParentFrame->RecalcLayout ();
		}
	}
	
	HWND hWnd = GetSafeHwnd ();

	CBCGToolBar::OnLButtonUp(nFlags, point);

	if (::IsWindow (hWnd))
	{		
		OnMouseLeave (0, 0);
	}
}
//*********************************************************************************
void CBCGOutlookBar::Serialize(CArchive& ar)
{
	CBCGToolBar::Serialize (ar);

	//-----------------------------------------------------------
	// First, save accociations between page control and page ID:
	//-----------------------------------------------------------

	CMap<UINT,UINT,HWND,HWND> mapPageControls;

	POSITION pos = NULL;

	for (pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		mapPageControls.SetAt (pPage->m_uiID, pPage->m_hwndControl);
	}

	if (ar.IsLoading ())
	{
		while (!m_lstPages.IsEmpty ())
		{
			delete m_lstPages.RemoveHead ();
		}

		SetActivePage ((UINT)-1);

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x40700)
		{
			ar >> m_nSize;
		}
	}
	else
	{
		ar << m_nSize;
	}

	if (ar.IsLoading ())
	{
		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x40710)
		{
			m_lstPages.Serialize (ar);
			m_nVisiblePageButtons = -1;

			BOOL bIsFirst = TRUE;
			UINT uiFirstPageID = (UINT)-1;

			int i = 0;

			for (pos = m_lstPages.GetHeadPosition (); pos != NULL; i++)
			{
				CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
				ASSERT_VALID (pPage);

				HWND hWnd = NULL;
				mapPageControls.Lookup (pPage->m_uiID, hWnd);
				pPage->m_hwndControl = hWnd;

				pPage->CreateButton (this);

				pPage->m_btnPage.SetImage (m_imagesPages.GetSafeHandle () == NULL ?
					NULL : m_imagesPages.ExtractIcon (i));

				if (bIsFirst)
				{
					uiFirstPageID = pPage->m_uiID;
					bIsFirst = FALSE;
				}
			}

			if (uiFirstPageID != (UINT)-1)
			{
				SetActivePage (uiFirstPageID);
			}

			if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60320)
			{
				ar >> m_nVisiblePageButtons;
			}
		}
	}
	else
	{
		m_lstPages.Serialize (ar);
		ar << m_nVisiblePageButtons;
	}
}
//*********************************************************************************
void CBCGOutlookBar::DrawTracker(CDC *pDC, const CRect& rectTracker)
{
	CBrush* pBrush = CDC::GetHalftoneBrush ();
	ASSERT_VALID (pBrush);

	CBrush* pBrushOld = pDC->SelectObject (pBrush);
	ASSERT_VALID (pBrushOld);

	pDC->PatBlt (rectTracker.left, rectTracker.top,
		rectTracker.right - rectTracker.left, 
		rectTracker.bottom - rectTracker.top, PATINVERT);

	pDC->SelectObject (pBrushOld);
}
//***********************************************************************************
void CBCGOutlookBar::SetActivePage (UINT uiPageID, BOOL bAnimate)
{
	if (uiPageID == m_uiActivePageID)
	{
		// Already active, do nothing
		return;
	}

	CBCGOutlookBarPage* pNewActivePage = NULL;
	if (uiPageID != (UINT)-1 &&
		(pNewActivePage = GetPage (uiPageID)) == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CRect rectButtonPrev;
	rectButtonPrev.SetRectEmpty ();

	CBCGOutlookBarPage* pPrevActivePage = GetPage (m_uiActivePageID);
	if (pPrevActivePage != NULL)
	{
		rectButtonPrev = pPrevActivePage->m_rect;

		if (pPrevActivePage->m_hwndControl != NULL)
		{
			::ShowWindow (pPrevActivePage->m_hwndControl, SW_HIDE);
		}
	}

	if (pNewActivePage != NULL &&
		pNewActivePage->m_hwndControl != NULL)
	{
		::ShowWindow (pNewActivePage->m_hwndControl, SW_SHOW);
	}

	m_uiActivePageID = uiPageID;
	OnActivatePage ();

	m_iScrollOffset = 0;
	m_iFirstVisibleButton = 0;
	m_bScrollDown = FALSE;

	CRect rectButtonStart = pNewActivePage == NULL ? 
						CRect (0, 0, 0, 0) :
						pNewActivePage->m_rect;
	AdjustLocations ();
	
	m_rectActivePageBtn = pNewActivePage == NULL ? 
						CRect (0, 0, 0, 0) :
						pNewActivePage->m_rect;

	ClientToScreen (&m_rectActivePageBtn);
	
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	bAnimate = (m_bEnableAnimation && bAnimate);

	if (!bAnimate || rectButtonPrev.IsRectEmpty ())
	{
		Invalidate ();
		UpdateWindow ();

		return;
	}

	CBCGOutlookBarPage* pAnimPage = pNewActivePage;
	CRect rectButtonFinish = pNewActivePage->m_rect;

	if (rectButtonStart == rectButtonFinish)
	{
		pAnimPage = pPrevActivePage;
		rectButtonFinish = pPrevActivePage->m_rect;
	}

	int yDelta = rectButtonStart.top < rectButtonFinish.top ? 
		rectButtonFinish.Height () : -rectButtonFinish.Height ();

	GetWorkArea (m_rectWorkArea);

	CClientDC dc (this);

	CRect rectClip = m_rectWorkArea;
	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;

	if (bHorz)
	{
		if (m_bEnableSplitter)
		{
			if (m_dwStyle & CBRS_BOTTOM)
			{
				rectClip.top += BORDER_SIZE;
			}
			else
			{
				rectClip.bottom -= BORDER_SIZE;
			}
		}
	}
	else
	{
		if (m_bEnableSplitter)
		{
			if (m_dwStyle & CBRS_LEFT)
			{
				rectClip.right -= BORDER_SIZE;
			}
			else
			{
				rectClip.left += BORDER_SIZE;
			}
		}
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rectClip);
	dc.SelectClipRgn (&rgn);

	for (int y = rectButtonStart.top;; y += yDelta)
	{
		if (yDelta < 0)
		{
			if (y <= rectButtonFinish.top)
			{
				break;
			}
		}
		else
		{
			if (y >= rectButtonFinish.top)
			{
				break;
			}
		}

		InvalidateRect (rectButtonStart);

		rectButtonStart.OffsetRect (0, yDelta);
		pAnimPage->SetRect (rectButtonStart);

		OnEraseWorkArea (&dc, rectButtonFinish);

		UpdateWindow ();
		Sleep (10);
	}

	dc.SelectClipRgn (NULL);

	AdjustLocations ();

	Invalidate ();
	UpdateWindow ();
}
//*******************************************************************************
BOOL CBCGOutlookBar::AddPage (UINT uiPageID, LPCTSTR lpszPageLabel,
							  CWnd* pWndPageCtrl, int iInsertAt)
{
	ASSERT_VALID (this);
	ASSERT (lpszPageLabel != NULL);

	if (iInsertAt != -1 &&
		(iInsertAt < 0 || iInsertAt > m_lstPages.GetCount ()))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (GetPage (uiPageID) != NULL)
	{
		//-------------------------------------------
		// The page with the same ID is already exist
		//-------------------------------------------
		ASSERT (FALSE);
		return FALSE;
	}

	m_nVisiblePageButtons = -1;

	CBCGOutlookBarPage* pPage = new CBCGOutlookBarPage;
	ASSERT_VALID (pPage);

	pPage->m_uiID = uiPageID;

	if (iInsertAt == -1)
	{
		m_lstPages.AddTail (pPage);
	}
	else
	{
		POSITION pos = m_Buttons.FindIndex (iInsertAt);
		ASSERT (pos != NULL);

		m_lstPages.InsertBefore (pos, pPage);
	}

	pPage->m_strName = lpszPageLabel;
	pPage->m_hwndControl = pWndPageCtrl->GetSafeHwnd ();
	pPage->CreateButton (this);

	if (m_lstPages.GetCount () == 1)	// First added page
	{
		SetActivePage (uiPageID);
	}
	else
	{
		AdjustLayout ();
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGOutlookBar::RemovePage (UINT uiPageID)
{
	POSITION pos = NULL;

	if (GetPage (uiPageID) == NULL)
	{
		return FALSE;
	}

	m_nVisiblePageButtons = -1;

	//------------------------------------------
	// First, remove all buttons from this page:
	//------------------------------------------
	for (pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_uiPageID == uiPageID)
		{
			m_Buttons.RemoveAt (posSave);
			delete pButton;
		}
	}

	//-------------
	// Remove page:
	//-------------
	UINT uiPrevPageID = (UINT)-1;

	for (pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (pPage->m_uiID == uiPageID)
		{
			m_lstPages.RemoveAt (posSave);
			delete pPage;
			break;
		}

		uiPrevPageID = pPage->m_uiID;
	}

	//----------------------
	// Reset an active page:
	//----------------------
	if (m_lstPages.IsEmpty ())
	{
		SetActivePage ((UINT)-1);	// No pages left
	}
	else if (m_uiActivePageID == uiPageID)
	{
		//---------------------------------------------
		// This page was active, activate another page:
		//---------------------------------------------
		if (uiPrevPageID == (UINT)-1)	// First page removed
		{
			CBCGOutlookBarPage* pFirstPage = (CBCGOutlookBarPage*) m_lstPages.GetHead ();
			ASSERT_VALID (pFirstPage);

			uiPrevPageID = pFirstPage->m_uiID;
		}

		SetActivePage (uiPrevPageID);
	}
	else if (GetSafeHwnd () != NULL)
	{
		AdjustLocations ();
		UpdateWindow ();
		Invalidate ();
	}

	return TRUE;
}
//*******************************************************************************
void CBCGOutlookBar::RemoveAllPages ()
{
	m_nVisiblePageButtons = -1;

	//---------------------------
	// First, remove all buttons:
	//---------------------------
	while (!m_Buttons.IsEmpty ())
	{
		delete m_Buttons.RemoveHead ();
	}

	//--------------
	// Remove pages:
	//--------------
	while (!m_lstPages.IsEmpty ())
	{
		delete m_lstPages.RemoveHead ();
	}; 

	//----------------------
	// Reset an active page:
	//----------------------
	SetActivePage ((UINT)-1);	// No pages left
}
//*******************************************************************************
CBCGOutlookBarPage* CBCGOutlookBar::GetPage (UINT uiPageID) const
{
	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (pPage->m_uiID == uiPageID)
		{
			return pPage;
		}
	}

	return NULL;
}
//*******************************************************************************
BOOL CBCGOutlookBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD (wParam))
	{
	case idShowMoreButtons:
		OnShowMorePageButtons ();
		return TRUE;

	case idShowFewerButtons:
		OnShowFewerPageButtons ();
		return TRUE;
	}

	if (LOWORD (wParam) == m_nIDClose)
	{
		if (m_pDockSite != NULL)
		{
			m_pDockSite->ShowControlBar(this, FALSE, FALSE); // hide
		}

		return TRUE;
	}

	// Find the control send the message:
	HWND hWndCtrl = (HWND)lParam;

	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (pPage->m_btnPage.GetSafeHwnd () == hWndCtrl)
		{
			SetActivePage (pPage->m_uiID, TRUE /* Animate */);
			return TRUE;
		}
	}

	return CBCGToolBar::OnCommand (wParam, lParam);
}
//*************************************************************************************
void CBCGOutlookBar::EnableSplitter (BOOL bEnable)
{
	m_bEnableSplitter	= bEnable;

	m_rectSplitter.SetRectEmpty ();
	m_rectMoveSplitterScreen.SetRectEmpty ();
	m_bIsTracking		= FALSE;
	m_nSplitterOffset	= 0;

	SetSplitterRect ();

	if (GetSafeHwnd () != NULL)
	{
		AdjustLayout ();
	}
}
//*************************************************************************************
void CBCGOutlookBar::EnableAnimation (BOOL bEnable/* = TRUE*/)
{
	m_bEnableAnimation = bEnable;
}
//*************************************************************************************
void CBCGOutlookBar::EnableCloseButton (BOOL bEnable/* = TRUE*/)
{
	m_bEnableCloseButton = bEnable;
	AdjustLocations ();
}
//*************************************************************************************
BOOL CBCGOutlookBar::StartAddPage (UINT uiNewPageID, LPCTSTR lpszNewPageDefaultName)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	if (!AddPage (uiNewPageID, lpszNewPageDefaultName))
	{
		return FALSE;
	}

	BOOL bSuccess = StartRenamePage (uiNewPageID);
	if (!bSuccess)
	{
		RemovePage (uiNewPageID);
	}

	return bSuccess;
}
//*************************************************************************************
BOOL CBCGOutlookBar::StartRenamePage (UINT uiPageID)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	CBCGOutlookBarPage* pPage = GetPage (uiPageID);
	if (pPage == NULL)
	{
		return FALSE;
	}

	ASSERT (m_pInPlaceEdit == NULL);

	m_pInPlaceEdit = new CEdit;
	ASSERT_VALID (m_pInPlaceEdit);

	CRect rectEdit;
	pPage->m_btnPage.GetClientRect (rectEdit);
	rectEdit.DeflateRect (2, 2);

	if (!m_pInPlaceEdit->Create (WS_VISIBLE | WS_CHILD | WS_BORDER, rectEdit, 
		&pPage->m_btnPage, 1))
	{
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;

		return FALSE;
	}

	m_pInPlaceEdit->SetWindowText (pPage->m_strName);
	m_pInPlaceEdit->SetFont (&globalData.fontRegular);

	m_pInPlaceEdit->SetFocus ();
	m_uiEditedPageID = uiPageID;

	SetCapture ();
	return TRUE;
}
//************************************************************************************
void CBCGOutlookBar::SetPageName (UINT uiPageID, LPCTSTR lpszPageLabel)
{
	CBCGOutlookBarPage* pPage = GetPage (uiPageID);
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pPage->SetName (lpszPageLabel);
}
//************************************************************************************
LPCTSTR CBCGOutlookBar::GetPageName (UINT uiPageID) const
{
	CBCGOutlookBarPage* pPage = GetPage (uiPageID);
	if (pPage == NULL)
	{
		return NULL;
	}

	return pPage->m_strName;
}
//************************************************************************************
CWnd* CBCGOutlookBar::GetPageControl (UINT uiPageID) const
{
	CBCGOutlookBarPage* pPage = GetPage (uiPageID);
	if (pPage == NULL || pPage->m_hwndControl == NULL)
	{
		return NULL;
	}

	return CWnd::FromHandle (pPage->m_hwndControl);
}
//************************************************************************************
UINT CBCGOutlookBar::PageFromPoint (const CPoint& pt) const
{
	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (pPage->m_rect.PtInRect (pt))
		{
			return pPage->m_uiID;
		}
	}

	return (UINT) -1;
}
//********************************************************************************
int CBCGOutlookBar::FindDropIndex (const CPoint p, CRect& rectDrag) const
{
	// Maybe, this page is empty?
	BOOL bEmptyPage = FALSE;

	if (!m_lstPages.IsEmpty ())
	{
		bEmptyPage = TRUE;

		for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
		{
			CBCGOutlookButton* pButton = (CBCGOutlookButton*) m_Buttons.GetNext (pos);
			ASSERT (pButton != NULL);

			if (pButton->m_uiPageID == m_uiActivePageID)
			{
				bEmptyPage = FALSE;
				break;
			}
		}
	}

	if (bEmptyPage)
	{
		const int iCursorSize = 6;
		GetWorkArea (rectDrag);

		BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
		if (bHorz)
		{
			rectDrag.right = rectDrag.left + iCursorSize;
		}
		else
		{
			rectDrag.bottom = rectDrag.top + iCursorSize;
		}

		return 0;
	}

	return CBCGToolBar::FindDropIndex (p, rectDrag);
}
//************************************************************************************
CBCGOutlookBarPage* CBCGOutlookBar::GetPageFromPoint (const CPoint& point) const
{
	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		if (pPage->m_rect.PtInRect (point))
		{
			return pPage;
		}
	}

	return NULL;
}
//************************************************************************************
void CBCGOutlookBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	CBCGOutlookBarPage* pPage = GetPageFromPoint (ptClient);
	if (pPage != NULL)
	{
		pPage->m_btnPage.SendMessage (WM_CANCELMODE);
	}

	CBCGToolBar::OnContextMenu (pWnd, point);
}
//************************************************************************************
void CBCGOutlookBar::RemoveAllButtons()
{
	CBCGToolBar::RemoveAllButtons();

	m_iFirstVisibleButton = 0; 
	m_iScrollOffset = 0; 

	AdjustLocations();

	UpdateWindow();
	Invalidate();
}
//*************************************************************************************
void CBCGOutlookBar::ClearAll ()
{
	RemoveAllPages ();
	m_Images.Clear ();
}
//*************************************************************************************
void CBCGOutlookBar::GetWorkArea (CRect& rect) const
{
	GetClientRect (rect);

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
	if (bHorz)
	{
		rect.left = m_rectCaption.right;
		if (m_dwStyle & CBRS_TOP)
		{
			rect.OffsetRect (0, -BORDER_SIZE);
		}
	}
	else
	{
		rect.top = m_rectCaption.bottom;
		if (m_dwStyle & CBRS_RIGHT)
		{
			rect.OffsetRect (BORDER_SIZE, 0);
		}
	}
}
//*************************************************************************************
void CBCGOutlookBar::OnDrawCaption (CDC* pDC, CRect rectCaption, CString strTitle,
									BOOL bHorz)
{
	ASSERT_VALID (pDC);

	if (m_bMode2003)
	{
		COLORREF clrText = globalData.clrBarText;
		CBCGVisualManager::GetInstance ()->OnFillOutlookBarCaption (pDC,
			rectCaption, clrText);
		pDC->SetTextColor (clrText);
	}
	else
	{
		pDC->FillRect (rectCaption, &globalData.brBarFace);

		if (!m_bFlatBorder)
		{
			pDC->Draw3dRect (rectCaption, globalData.clrBarShadow, globalData.clrBarDkShadow);
			rectCaption.DeflateRect(1, 1);
		}

		pDC->Draw3dRect (rectCaption, globalData.clrBarHilite, globalData.clrBarShadow);
	}

	rectCaption.DeflateRect (5, 0);

	if (!bHorz)
	{
		pDC->DrawText (strTitle, rectCaption, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		CPoint ptOrg = bHorz ?
			CPoint(rectCaption.left, rectCaption.bottom - 3) :
			CPoint(rectCaption.left + 3, rectCaption.top);

		pDC->ExtTextOut (ptOrg.x, ptOrg.y,
			ETO_CLIPPED, rectCaption, strTitle, NULL);
	}
}
//*************************************************************************************
void CBCGOutlookBar::OnEraseWorkArea (CDC* pDC, CRect rectWorkArea)
{
	CBCGVisualManager::GetInstance ()->OnFillBarBackground (pDC, this, m_rectWorkArea, rectWorkArea);
}
//*************************************************************************************
void CBCGOutlookBar::FillWorkArea (CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	if (m_bmpBack.GetCount() == 0)
	{
		pDC->FillSolidRect (rectWorkArea, m_clrBackColor);
	}
	else
	{
		ASSERT (m_bmpBack.GetCount () == 1);

		CBCGDrawState ds;
		m_bmpBack.PrepareDrawImage (ds);
		CSize sizeBack = m_bmpBack.GetImageSize ();

		for (int x = rectWorkArea.left; x < rectWorkArea.right; x += sizeBack.cx)
		{
			for (int y = rectWorkArea.top; y < rectWorkArea.bottom; y += sizeBack.cy)
			{
				m_bmpBack.Draw (pDC, x, y, 0);
			}
		}

		m_bmpBack.EndDrawImage (ds);
	}
}
//************************************************************************************
void CBCGOutlookBar::CopyButtonsList (const CObList& lstSrc, CObList& lstDst)
{
	while (!lstDst.IsEmpty ())
	{
		delete lstDst.RemoveHead ();
	}

	for (POSITION pos = lstSrc.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButtonSrc = (CBCGToolbarButton*) lstSrc.GetNext (pos);
		ASSERT_VALID (pButtonSrc);

		CRuntimeClass* pClass = pButtonSrc->GetRuntimeClass ();
		ASSERT (pClass != NULL);

		CBCGToolbarButton* pButton = (CBCGToolbarButton*) pClass->CreateObject ();
		ASSERT_VALID(pButton);

		pButton->CopyFrom (*pButtonSrc);
		pButton->OnChangeParentWnd (this);

		lstDst.AddTail (pButton);
	}
}
//************************************************************************************
void CBCGOutlookBar::CopyPagesList (const CObList& lstSrc, CObList& lstDst,
									BOOL bCreateButton)
{
	while (!lstDst.IsEmpty ())
	{
		delete lstDst.RemoveHead ();
	}

	for (POSITION pos = lstSrc.GetHeadPosition (); pos != NULL;)
	{
		CBCGOutlookBarPage* pPageSrc = (CBCGOutlookBarPage*) lstSrc.GetNext (pos);
		ASSERT_VALID (pPageSrc);

		CBCGOutlookBarPage* pPage = new CBCGOutlookBarPage (*pPageSrc);
		ASSERT_VALID (pPage);

		if (bCreateButton)
		{
			pPage->CreateButton (this);
		}

		lstDst.AddTail (pPage);
	}
}
//****************************************************************************************
void CBCGOutlookBar::SetDefaultState ()
{
	CopyButtonsList (m_Buttons, m_OrigButtons);
	CopyPagesList (m_lstPages, m_lstOrigPages);
}
//****************************************************************************************
BOOL CBCGOutlookBar::RestoreOriginalstate ()
{
	if (m_OrigButtons.IsEmpty ())
	{
		return FALSE;
	}

	m_nVisiblePageButtons = -1;

	CopyButtonsList (m_OrigButtons, m_Buttons);
	CopyPagesList (m_lstOrigPages, m_lstPages, TRUE);

	AdjustLayout ();

	if (m_uiActivePageID != (UINT)-1 &&
		GetPage (m_uiActivePageID) == NULL)
	{
		SetActivePage ((UINT)-1);
	}
	else
	{
		UINT uiActivePageID = m_uiActivePageID;
		m_uiActivePageID = (UINT)-1;
		SetActivePage (uiActivePageID);
	}

	Invalidate ();
	return TRUE;
}
//************************************************************************************
BOOL CBCGOutlookBar::SmartUpdate (const CObList& lstPrevButtons)
{
	if (lstPrevButtons.IsEmpty ())
	{
		return FALSE;
	}

	m_bResourceWasChanged = FALSE;	// Outlook bar has its own resources

	BOOL bIsModified = FALSE;

	//-----------------------------------
	// Compare current and prev. buttons:
	//------------------------------------
	if (lstPrevButtons.GetCount () != m_OrigButtons.GetCount ())
	{
		bIsModified = TRUE;
	}
	else
	{
		POSITION posCurr, posPrev;
		for (posCurr = m_OrigButtons.GetHeadPosition (),
			posPrev = lstPrevButtons.GetHeadPosition (); posCurr != NULL;)
		{
			ASSERT (posPrev != NULL);

			CBCGToolbarButton* pButtonCurr = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, m_OrigButtons.GetNext (posCurr));
			ASSERT_VALID (pButtonCurr);

			CBCGToolbarButton* pButtonPrev = 
				DYNAMIC_DOWNCAST (CBCGToolbarButton, lstPrevButtons.GetNext (posPrev));
			ASSERT_VALID (pButtonPrev);

			if (!pButtonCurr->CompareWith (*pButtonPrev))
			{
				bIsModified = TRUE;
				break;
			}
		}
	}

	if (bIsModified)
	{
		RestoreOriginalstate ();
	}

	return bIsModified;
}
//************************************************************************************
void CBCGOutlookBar::SaveOriginalState (CBCGRegistry& reg)
{
	CBCGToolBar::SaveOriginalState (reg);

	if (!m_lstOrigPages.IsEmpty ())
	{
		reg.Write (REG_ENTRY_ORIG_PAGES, m_lstOrigPages);
	}
}
//*************************************************************************************
BOOL CBCGOutlookBar::LoadLastOriginalState (CBCGRegistry& reg)
{
	if (CBCGToolBar::LoadLastOriginalState (reg))
	{
		return TRUE;
	}

	BOOL bIsModified = FALSE;

	CObList lstOrigPages;	// Original (resource) data in the last session
	if (reg.Read (REG_ENTRY_ORIG_PAGES, lstOrigPages))
	{
		//---------------------------------
		// Compare current and prev. pages:
		//---------------------------------
		if (lstOrigPages.GetCount () != m_lstOrigPages.GetCount ())
		{
			bIsModified = TRUE;
		}
		else
		{
			POSITION posCurr, posPrev;
			for (posCurr = m_lstOrigPages.GetHeadPosition (),
				posPrev = lstOrigPages.GetHeadPosition (); posCurr != NULL;)
			{
				ASSERT (posPrev != NULL);

				CBCGOutlookBarPage* pPageCurr = 
					DYNAMIC_DOWNCAST (CBCGOutlookBarPage, m_lstOrigPages.GetNext (posCurr));
				ASSERT_VALID (pPageCurr);

				CBCGOutlookBarPage* pPagePrev = 
					DYNAMIC_DOWNCAST (CBCGOutlookBarPage, lstOrigPages.GetNext (posPrev));
				ASSERT_VALID (pPagePrev);

				if (!pPageCurr->Compare (*pPagePrev))
				{
					bIsModified = TRUE;
					break;
				}
			}
		}
	}

	while (!lstOrigPages.IsEmpty ())
	{
		delete lstOrigPages.RemoveHead ();
	}

	if (bIsModified)
	{
		RestoreOriginalstate ();
	}

	return bIsModified;
}
//***************************************************************************************
void CBCGOutlookBar::SetInitialSize (int nSize)
{
	ASSERT_VALID (this);
	m_nSize = nSize;
}
//***************************************************************************************
void CBCGOutlookBar::OnStyleChanging (int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	m_bCaption = (lpStyleStruct->styleNew & WS_CAPTION) || m_bMode2003;
	lpStyleStruct->styleNew &= ~WS_CAPTION;
	
	CBCGToolBar::OnStyleChanging (nStyleType, lpStyleStruct);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBarCaptionButton

void CBCGOutlookBarCaptionButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	CBCGVisualManager::GetInstance ()->OnEraseOutlookCaptionButton (pDC, rectClient, this);
}

void CBCGOutlookBarCaptionButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	CBCGVisualManager::GetInstance ()->OnDrawOutlookCaptionButtonBorder (pDC, rectClient,
		this, uiState);
}
//************************************************************************************
void CBCGOutlookBar::ScrollPageDown ()
{

	CBCGOutlookButton* pFirstVisibleButton = GetButtonInPage (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	UINT uiID = GetActivePageID();
	CBCGOutlookBarPage* pPage = GetPage(uiID);
	CRect rcBtn;
	pPage->m_btnPage.GetWindowRect(rcBtn);
	int nExtra = GetCount() + 1 - GetPageCount ();


	CRect rcArea;
	GetWorkArea(rcArea);
	int nVisibleCount = 0;
	
	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;
	if(bVert)
	{
		 nVisibleCount = (rcArea.Height() - nExtra*rcBtn.Height())/(pFirstVisibleButton->Rect ().Height () + m_nExtraSpace);

	}else
	{
		 nVisibleCount = rcArea.Width()/(pFirstVisibleButton->Rect ().Width() + m_nExtraSpace);
	}



	if(!m_bScrollDown || 
		m_iFirstVisibleButton + nVisibleCount >= GetPageCount ())
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	m_iFirstVisibleButton += nVisibleCount;

	if (bVert)
	{
		m_iScrollOffset += nVisibleCount*pFirstVisibleButton->Rect ().Height ();
	}
	else
	{
		m_iScrollOffset += nVisibleCount*pFirstVisibleButton->Rect ().Width ();
	}

	AdjustLocations ();
	InvalidateRect (m_rectWorkArea);
	UpdateWindow ();
}

//*************************************************************************************
void CBCGOutlookBar::ScrollPageUp ()
{
	if (m_iScrollOffset <= 0 ||
		m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer (m_nIDScrollUp);
		return;
	}

	CBCGOutlookButton* pFirstVisibleButton = GetButtonInPage (m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer (m_nIDScrollDn);
		return;
	}

	UINT uiID = GetActivePageID();
	CBCGOutlookBarPage* pPage = GetPage(uiID);
	CRect rcBtn;
	pPage->m_btnPage.GetWindowRect(rcBtn);
	int nExtra = GetCount() + 1 - GetPageCount ();

	CRect rcArea;
	GetWorkArea(rcArea);
	int nVisibleCount = 0;

	BOOL bVert = (m_dwStyle & CBRS_ORIENT_HORZ) == 0;
	if(bVert)
	{
		nVisibleCount = (rcArea.Height() - nExtra*rcBtn.Height())/(pFirstVisibleButton->Rect ().Height () + m_nExtraSpace);

	}else
	{
		nVisibleCount = (rcArea.Width()/pFirstVisibleButton->Rect ().Width() + m_nExtraSpace);
	}


	m_iFirstVisibleButton -= nVisibleCount;


	if (bVert)
	{
		m_iScrollOffset -= nVisibleCount*pFirstVisibleButton->Rect ().Height ();
	}
	else
	{
		m_iScrollOffset -= nVisibleCount*pFirstVisibleButton->Rect ().Width ();
	}

	if (m_iFirstVisibleButton == 0)
	{
		m_iScrollOffset = 0;
	}

	ASSERT (m_iScrollOffset >= 0);

	AdjustLocations ();
	InvalidateRect (m_rectWorkArea);
	UpdateWindow ();
}
//********************************************************************************
void CBCGOutlookBar::SetMode2003 (BOOL bMode2003/* = TRUE*/)
{
	ASSERT_VALID (this);
	m_bMode2003 = bMode2003;

	AdjustLayout ();
}
//***************************************************************************************
BOOL CBCGOutlookBar::SetPageImages (UINT uiID, int cx, COLORREF clrTransp)
{
	if (!LoadPageImages (uiID, cx, clrTransp, m_imagesPages, m_sizePageImage))
	{
		return FALSE;
	}

	int i = 0;
	for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL; i++)
	{
		CBCGOutlookBarPage* pPage = (CBCGOutlookBarPage*) m_lstPages.GetNext (pos);
		ASSERT_VALID (pPage);

		pPage->m_btnPage.SetImage (m_imagesPages.GetSafeHandle () == NULL ?
			NULL : m_imagesPages.ExtractIcon (i));
	}

	AdjustLayout ();
	return TRUE;
}
//********************************************************************************
BOOL CBCGOutlookBar::SetPageToolbarImages (UINT uiID, int cx, COLORREF clrTransp)
{
	if (!LoadPageImages (uiID, cx, clrTransp, m_imagesToolbar, m_sizeToolbarImage))
	{
		return FALSE;
	}

	AdjustLayout ();
	return TRUE;
}
//********************************************************************************
BOOL CBCGOutlookBar::LoadPageImages (UINT uiID, int cx, COLORREF clrTransp,
					 CImageList& images, CSize& imageSize)
{
	if (images.GetSafeHandle () != NULL)
	{
		images.DeleteImageList ();
	}

	if (uiID == 0)
	{
		imageSize = CSize (0, 0);
		return TRUE;
	}

	CBitmap bmp;
	if (!bmp.LoadBitmap (uiID))
	{
		TRACE(_T("CBCGOutlookBar::LoadPageImages: Can't load bitmap: %x\n"), uiID);
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

	images.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	images.Add (&bmp, clrTransp);

	imageSize = CSize (cx, bmpObj.bmHeight);

	return TRUE;
}
//********************************************************************************
void CBCGOutlookBar::OnUpdateToolbarCommand (CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case idShowMoreButtons:
		pCmdUI->Enable (CanShowMorePageButtons ());
		break;

	case idShowFewerButtons:
		pCmdUI->Enable (CanShowFewerPageButtons ());
		break;

	default:
		pCmdUI->Enable();
		break;
	}
}
//*******************************************************************************
void CBCGOutlookBar::OnShowMorePageButtons ()
{
	m_nVisiblePageButtons++;

	AdjustLocations ();
	RedrawWindow ();
}
//*******************************************************************************
void CBCGOutlookBar::OnShowFewerPageButtons ()
{
	m_nVisiblePageButtons--;

	AdjustLocations ();
	RedrawWindow ();
}
//*******************************************************************************
BOOL CBCGOutlookBar::CanShowMorePageButtons () const
{
	return m_nVisiblePageButtons < m_nMaxVisiblePageButtons;
}
//*******************************************************************************
BOOL CBCGOutlookBar::CanShowFewerPageButtons () const
{
	return m_nVisiblePageButtons > 0;
}
//*******************************************************************************
void CBCGOutlookBar::SetButtonsFont (CFont* pFont, BOOL bRedraw/* = TRUE*/)
{
	ASSERT_VALID (this);
	m_pFontButtons = pFont;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGOutlookBarToolBar

CBCGOutlookBarToolBar::CBCGOutlookBarToolBar (CBCGOutlookBar* pParentBar) :
	m_pParentBar (pParentBar)
{
	m_bLocked = TRUE;
}

BEGIN_MESSAGE_MAP(CBCGOutlookBarToolBar, CBCGToolBar)
	//{{AFX_MSG_MAP(CBCGOutlookBarToolBar)
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

BOOL CBCGOutlookBarToolBar::OnSendCommand (const CBCGToolbarButton* pButton)
{
	int nIndex = ButtonToIndex (pButton);
	if (nIndex >= 0)
	{
		UINT iTab = (UINT) -1;
		if (m_TabButtons.Lookup (nIndex, iTab))
		{
			m_pParentBar->SetActivePage (iTab);
			return TRUE;
		}
	}

	return FALSE;
}

void CBCGOutlookBarToolBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
	for (int i = 0; i < m_Buttons.GetCount (); i++)
	{
		UINT nNewStyle = GetButtonStyle(i) &
					~(TBBS_CHECKED | TBBS_INDETERMINATE);
		
		UINT iTab = (UINT) -1;
		if (m_TabButtons.Lookup (i, iTab))
		{
			if (m_pParentBar->GetActivePageID () == iTab)
			{
				nNewStyle |= TBBS_CHECKED;
			}

			SetButtonStyle (i, nNewStyle | TBBS_CHECKBOX);
		}
	}
}

BOOL CBCGOutlookBarToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);

	ScreenToClient (&ptCursor);

	if (HitTest (ptCursor) >= 0)
	{
		if (globalData.m_hcurHand == NULL)
		{
			CBCGLocalResource locaRes;
			globalData.m_hcurHand = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_HAND);
		}

		::SetCursor (globalData.m_hcurHand);
		return TRUE;
	}

	return CBCGToolBar::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CBCGOutlookBarToolBar::OnUserToolTip (CBCGToolbarButton* pButton, CString& strTTText) const
{
	strTTText = pButton->m_strText;
	return TRUE;
}

void CBCGOutlookBarToolBar::AdjustLocations ()
{
	CSize sizeImage = GetImageSize ();
	if (sizeImage == CSize (0, 0))
	{
		sizeImage = CSize (16, 16);
	}

	CSize sizeButton = sizeImage + CSize (10, 6 + 2 * nToolbarMarginHeight);
	int y = -1;
	int xRightMargin = 2;

	if (CBCGVisualManager::GetInstance ()->IsOutlookToolbarHotBorder ())
	{
		y = 0;
		sizeButton.cy -= 2;
		xRightMargin = 0;
	}

	CSize sizeCustomizeButton (0, 0);
	if (m_pCustomizeBtn != NULL)
	{
		sizeCustomizeButton = sizeButton;
		sizeCustomizeButton.cx = 
			max (sizeCustomizeButton.cx, CMenuImages::Size ().cx + 10);
	}

	CRect rectToolbar;
	GetClientRect (rectToolbar);

	int nCount = sizeCustomizeButton == CSize (0, 0) ? 
		(int) m_Buttons.GetCount () :
		(int) m_Buttons.GetCount () - 1;

	int x = rectToolbar.right -  sizeCustomizeButton.cx + xRightMargin;

	int nCountToHide = 0;
	nCountToHide = nCount - (rectToolbar.Width () - sizeCustomizeButton.cx + xRightMargin) / 
		(sizeButton.cx - 2);

	for (POSITION pos = m_Buttons.GetTailPosition ();  pos != NULL;)
	{
		CBCGToolbarButton* pButton = (CBCGToolbarButton*) m_Buttons.GetPrev (pos); 
		ASSERT_VALID (pButton);
		CCustomizeButton* pCustomizeBtn = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);

		if (nCountToHide >0 && pCustomizeBtn == NULL)
		{
			CObList& list = const_cast<CObList&> (m_pCustomizeBtn->GetInvisibleButtons ());
            list.AddHead (pButton);
			pButton->SetRect (CRect (0, 0, 0, 0));
			nCountToHide--;
		}
		else
		{
			CSize sizeCurrButton = sizeButton;

			if (pButton == m_pCustomizeBtn)
			{
				sizeCurrButton = sizeCustomizeButton;
			}

			sizeCurrButton.cy++;
			pButton->SetRect (CRect (CPoint (x, y), sizeCurrButton));

			x -= sizeButton.cx - 2;
		}
	}
}

void CBCGOutlookBarToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/)
{
}

void CBCGOutlookBarToolBar::OnNcPaint()
{
}

void CBCGOutlookBarToolBar::OnCustomizeMode (BOOL bSet)
{
	CBCGToolBar::OnCustomizeMode (bSet);
	EnableWindow (!bSet);
}

#endif // BCG_NO_OUTLOOKBAR
