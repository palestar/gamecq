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
// BCGPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGPropertySheet.h"
#include "BCGPropertyPage.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int idTree = 101;
const int idTab = 102;

/////////////////////////////////////////////////////////////////////////////
// CBCGPropSheetBar

BOOL CBCGPropSheetBar::OnSendCommand (const CBCGToolbarButton* pButton)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	CWaitCursor wait;
	m_pParent->SetActivePage (ButtonToIndex (pButton));

	return TRUE;
}
//****************************************************************************************
void CBCGPropSheetBar::EnsureVisible (int iButton)
{
	ASSERT_VALID (this);

	CBCGToolbarButton* pButton = GetButton (iButton);
	ASSERT_VALID (pButton);

	CRect rectButton = pButton->Rect ();

	CRect rectWork;
	GetWorkArea (rectWork);

	if (rectButton.Height () >= rectWork.Height ())
	{
		// Work area is too small, nothing to do
		return;
	}

	if (rectButton.top >= rectWork.top && rectButton.bottom <= rectWork.bottom)
	{
		// Already visible
		return;
	}

	if (rectButton.top < rectWork.top)
	{
		while (pButton->Rect ().top < rectWork.top)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollUp ();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
	else
	{
		while (pButton->Rect ().bottom > rectWork.bottom)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollDown ();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPropSheetTab

CBCGPropSheetTab::CBCGPropSheetTab ()
{
	m_bIsDlgControl = TRUE;
}
//*********************************************************************************
BOOL CBCGPropSheetTab::SetActiveTab (int iTab)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	CWaitCursor wait;

	if (m_pParent->GetActiveIndex () != iTab)
	{
		m_pParent->SetActivePage (iTab);
	}

	CBCGTabWnd::SetActiveTab (iTab);

	CRect rectWndArea = m_rectWndArea;
	MapWindowPoints (m_pParent, rectWndArea);

	CPropertyPage* pPage = m_pParent->GetPage (iTab);
	if (pPage != NULL)
	{
		pPage->SetWindowPos (NULL, rectWndArea.left, rectWndArea.top,
			rectWndArea.Width (), rectWndArea.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CBCGPropSheetCategory

IMPLEMENT_DYNAMIC(CBCGPropSheetCategory, CObject)

CBCGPropSheetCategory::CBCGPropSheetCategory (LPCTSTR lpszName, int nIcon, 
											  int nSelectedIcon,
											  const CBCGPropSheetCategory* pParentCategory) :
	m_strName (lpszName),
	m_nIcon (nIcon),
	m_nSelectedIcon (nSelectedIcon),
	m_pParentCategory (pParentCategory)
{
	m_hTreeItem = NULL;

	if (pParentCategory != NULL)
	{
		ASSERT_VALID (pParentCategory);
		((CBCGPropSheetCategory*)pParentCategory)->m_lstSubCategories.AddTail (this);
	}
}

CBCGPropSheetCategory::~CBCGPropSheetCategory()
{
	while (!m_lstSubCategories.IsEmpty ())
	{
		delete m_lstSubCategories.RemoveHead ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertySheet

#define UM_AFTERACTIVATEPAGE	(WM_USER + 1001)

IMPLEMENT_DYNAMIC(CBCGPropertySheet, CPropertySheet)

#pragma warning (disable : 4355)

CBCGPropertySheet::CBCGPropertySheet() :
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPropertySheet::CBCGPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPropertySheet::CBCGPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage),
	m_Impl (*this)
{
	CommonInit ();
}

#pragma warning (default : 4355)

void CBCGPropertySheet::SetLook (PropSheetLook look, int nNavBarWidth)
{
	ASSERT (GetSafeHwnd () == NULL);

	m_look = look;
	m_nBarWidth = nNavBarWidth;

	if (m_look != PropSheetLook_Tabs)
	{
		EnableStackedTabs (FALSE);
	}
}

CBCGPropertySheet::~CBCGPropertySheet()
{
	while (!m_lstTreeCategories.IsEmpty ())
	{
		delete m_lstTreeCategories.RemoveHead ();
	}
}

void CBCGPropertySheet::CommonInit ()
{
	m_nBarWidth = 100;
	m_nActivePage = -1;
	m_look = PropSheetLook_Tabs;
	m_bIsInSelectTree = FALSE;
	m_bAlphaBlendIcons = FALSE;
}

BEGIN_MESSAGE_MAP(CBCGPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CBCGPropertySheet)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_AFTERACTIVATEPAGE, OnAfterActivatePage)
	ON_NOTIFY(TVN_SELCHANGEDA, idTree, OnSelectTree)
	ON_NOTIFY(TVN_SELCHANGEDW, idTree, OnSelectTree)
	ON_NOTIFY(TVN_GETDISPINFOA, idTree, OnGetDispInfo)
	ON_NOTIFY(TVN_GETDISPINFOW, idTree, OnGetDispInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertySheet message handlers

void CBCGPropertySheet::AddPage(CPropertyPage* pPage)
{
	CPropertySheet::AddPage (pPage);

	if (GetSafeHwnd () == NULL || m_look == PropSheetLook_Tabs)
	{
		return;
	}

	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	InternalAddPage (pTab->GetItemCount () - 1);
}
//****************************************************************************************
void CBCGPropertySheet::InternalAddPage (int nTab)
{
	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	TCHAR szTab [256];

	TCITEM item;
	item.mask = TCIF_TEXT;
	item.cchTextMax = 255;
	item.pszText = szTab;

	pTab->GetItem (nTab, &item);

	if (m_wndOutlookBar.GetSafeHwnd () != NULL)
	{
		HICON hIcon = m_Icons.ExtractIcon (nTab);
		m_wndOutlookBar.AddButton (hIcon, szTab, 0, (UINT) -1, (UINT) -1, m_bAlphaBlendIcons);
		::DestroyIcon (hIcon);
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		CBCGPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, GetPage (nTab));
		if (pPage == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		HTREEITEM hParent = NULL;
		if (pPage->m_pCategory != NULL)
		{
			ASSERT_VALID (pPage->m_pCategory);
			hParent = pPage->m_pCategory->m_hTreeItem;
		}

		HTREEITEM hTreeItem = m_wndTree.InsertItem (szTab, 
			I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
		m_wndTree.SetItemData (hTreeItem, (DWORD_PTR) pPage);
		pPage->m_hTreeNode = hTreeItem;
	}

	if (m_wndTab.GetSafeHwnd () != NULL)
	{
		CBCGPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, GetPage (nTab));
		if (pPage == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		UINT uiImage = m_Icons.GetSafeHandle () == NULL ? (UINT)-1 : nTab;

		m_wndTab.AddTab (pPage, szTab, uiImage);
	}
}
//****************************************************************************************
void CBCGPropertySheet::RemovePage(CPropertyPage* pPage)
{
	int nPage = GetPageIndex (pPage);
	ASSERT (nPage >= 0);

	CPropertySheet::RemovePage (pPage);

	if (m_wndOutlookBar.GetSafeHwnd () != NULL)
	{
		m_wndOutlookBar.RemoveButton (nPage);
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		ASSERT (FALSE);
	}
}
//****************************************************************************************
void CBCGPropertySheet::RemovePage(int nPage)
{
	CPropertySheet::RemovePage (nPage);

	if (m_wndOutlookBar.GetSafeHwnd () != NULL)
	{
		m_wndOutlookBar.RemoveButton (nPage);
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		ASSERT (FALSE);
	}
}
//****************************************************************************************
CBCGPropSheetCategory* CBCGPropertySheet::AddTreeCategory (LPCTSTR lpszLabel, 
	int nIconNum, int nSelectedIconNum, const CBCGPropSheetCategory* pParentCategory)
{
	ASSERT_VALID (this);
	ASSERT (m_look == PropSheetLook_Tree);

	if (nSelectedIconNum == -1)
	{
		nSelectedIconNum = nIconNum;
	}

	CBCGPropSheetCategory* pCategory = new CBCGPropSheetCategory (
		lpszLabel, nIconNum, nSelectedIconNum,
		pParentCategory);

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		HTREEITEM hParent = NULL;
		if (pParentCategory != NULL)
		{
			hParent = pParentCategory->m_hTreeItem;
		}

		pCategory->m_hTreeItem = m_wndTree.InsertItem (
			lpszLabel, I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
		m_wndTree.SetItemData (pCategory->m_hTreeItem, (DWORD_PTR) pCategory);
	}

	if (pParentCategory == NULL)
	{
		m_lstTreeCategories.AddTail (pCategory);
	}

	return pCategory;
}
//***************************************************************************************
void CBCGPropertySheet::AddPageToTree (CBCGPropSheetCategory* pCategory, 
									   CBCGPropertyPage* pPage, int nIconNum,
									   int nSelIconNum)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);
	ASSERT (m_look == PropSheetLook_Tree);

	if (pCategory != NULL)
	{
		ASSERT_VALID (pCategory);
		pCategory->m_lstPages.AddTail (pPage);
	}

	pPage->m_pCategory = pCategory;
	pPage->m_nIcon = nIconNum;
	pPage->m_nSelIconNum = nSelIconNum;

	CPropertySheet::AddPage (pPage);

	if (GetSafeHwnd () != NULL)
	{
		CTabCtrl* pTab = GetTabControl ();
		ASSERT_VALID (pTab);

		InternalAddPage (pTab->GetItemCount () - 1);
	}
}
//****************************************************************************************
BOOL CBCGPropertySheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	BOOL bReposButtons = FALSE;

	const int nVertMargin = 5;
	const int nHorzMargin = 5;

	CWnd* pWndNavigator = InitNavigationControl ();

	CRect rectClient;
	GetClientRect (rectClient);

	if (m_wndTab.GetSafeHwnd () != NULL)
	{
		CTabCtrl* pTab = GetTabControl ();
		ASSERT_VALID (pTab);

		CRect rectTab;
		pTab->GetWindowRect (rectTab);
		ScreenToClient (rectTab);

		rectTab.InflateRect (2, 0);

		if (pTab->GetItemCount () > 0)
		{
			CRect rectTabsNew;
			m_wndTab.GetTabsRect (rectTabsNew);

			CRect rectTabsOld;
			pTab->GetItemRect (0, rectTabsOld);

			const int nOldHeight = rectTabsOld.Height ();
			const int nNewHeight = rectTabsNew.Height ();

			if (nNewHeight > nOldHeight)
			{
				rectClient.bottom += nNewHeight - nOldHeight + nVertMargin;
				rectTab.bottom += nNewHeight - nOldHeight;

				SetWindowPos (NULL, -1, -1,
					rectClient.Width (), rectClient.Height (),
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

		m_wndTab.MoveWindow (rectTab);

		pTab->ModifyStyle (WS_TABSTOP, 0);
		pTab->ShowWindow (SW_HIDE);

		if (pTab->GetItemCount () > 0)
		{
			m_wndTab.SetActiveTab (0);
		}

		bReposButtons = TRUE;
	}
	else if (pWndNavigator != NULL)
	{
		CTabCtrl* pTab = GetTabControl ();
		ASSERT_VALID (pTab);

		pTab->ModifyStyle (WS_TABSTOP, 0);

		CRect rectTabItem;
		pTab->GetItemRect (0, rectTabItem);
		pTab->MapWindowPoints (this, &rectTabItem);

		const int nTabsHeight = rectTabItem.Height () + nVertMargin;

		SetWindowPos (NULL, -1, -1, rectClient.Width () + m_nBarWidth,
			rectClient.Height () - nTabsHeight + 4 * nVertMargin,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		
		GetClientRect (rectClient);
		pTab->MoveWindow (m_nBarWidth, -nTabsHeight, rectClient.right, rectClient.bottom - 2 * nVertMargin);

		CRect rectTab;
		pTab->GetWindowRect (rectTab);
		ScreenToClient (rectTab);

		CRect rectNavigator = rectClient;
		rectNavigator.right = rectNavigator.left + m_nBarWidth;
		rectNavigator.bottom = rectTab.bottom;
		rectNavigator.DeflateRect (1, 1);

		pWndNavigator->SetWindowPos (&wndTop, 
							rectNavigator.left, rectNavigator.top,
							rectNavigator.Width (), 
							rectNavigator.Height (),
							SWP_NOACTIVATE);

		SetActivePage (GetActivePage ());

		bReposButtons = TRUE;
	}

	if (bReposButtons)
	{
		int ids[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP	};

		int nTotalButtonsWidth = 0;

		for (int iStep = 0; iStep < 2; iStep++)
		{
			for (int i = 0; i < sizeof (ids) / sizeof (ids [0]); i++)
			{
				CWnd* pButton = GetDlgItem (ids[i]);

				if (pButton != NULL)
				{
					if (ids [i] == IDHELP && (m_psh.dwFlags & PSH_HASHELP) == 0)
					{
						continue;
					}

					if (ids [i] == ID_APPLY_NOW && (m_psh.dwFlags & PSH_NOAPPLYNOW))
					{
						continue;
					}

					CRect rectButton;
					pButton->GetWindowRect (rectButton);
					ScreenToClient (rectButton);

					if (iStep == 0)
					{
						// Align buttons at the bottom
						pButton->SetWindowPos (&wndTop, rectButton.left, 
							rectClient.bottom - rectButton.Height () - nVertMargin, 
							-1, -1, SWP_NOSIZE | SWP_NOACTIVATE);

						nTotalButtonsWidth = rectButton.right;
					}
					else
					{
						// Right align the buttons
						pButton->SetWindowPos (&wndTop, 
							rectButton.left + rectClient.right - nTotalButtonsWidth - nHorzMargin,
							rectButton.top,
							-1, -1, SWP_NOSIZE | SWP_NOACTIVATE);
					}
				}
			}
		}
	}

	return bResult;
}
//***************************************************************************************
CWnd* CBCGPropertySheet::InitNavigationControl ()
{
	ASSERT_VALID (this);

	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	if (m_look == PropSheetLook_OutlookBar)
	{
		m_wndOutlookBar.m_pParent = this;

		m_wndOutlookBar.Create (this, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT, AFX_IDW_TOOLBAR);
		m_wndOutlookBar.SetBarStyle (CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY |
									CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

		m_wndOutlookBar.SetRouteCommandsViaFrame (FALSE);
		m_wndOutlookBar.SetOwner (this);

		m_wndOutlookBar.EnableTextLabels (TRUE);
		m_wndOutlookBar.EnableSplitter (FALSE);

		ASSERT (m_Icons.GetSafeHandle () != NULL);
		ASSERT (m_Icons.GetImageCount () == pTab->GetItemCount ());

		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndOutlookBar;
	}

	if (m_look == PropSheetLook_Tree)
	{
		CRect rectDummy (0, 0, 0, 0);
		const DWORD dwTreeStyle =	WS_CHILD | WS_VISIBLE;
		m_wndTree.Create (dwTreeStyle, rectDummy, this, (UINT) idTree);

		m_wndTree.ModifyStyleEx (0, WS_EX_CLIENTEDGE);

		if (m_Icons.GetSafeHandle () != NULL)
		{
			m_wndTree.SetImageList (&m_Icons, TVSIL_NORMAL);
			m_wndTree.SetImageList (&m_Icons, TVSIL_STATE);
		}

		//----------------
		// Add categories:
		//----------------
		for (POSITION pos = m_lstTreeCategories.GetHeadPosition (); pos != NULL;)
		{
			AddCategoryToTree (m_lstTreeCategories.GetNext (pos));
		}

		//-----------
		// Add pages:
		//-----------
		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndTree;
	}

	if (m_look == PropSheetLook_OneNoteTabs)
	{
		CRect rectDummy (0, 0, 0, 0);

		m_wndTab.Create (CBCGTabWnd::STYLE_3D_ONENOTE, rectDummy, this, 
			(UINT) idTab, CBCGTabWnd::LOCATION_TOP, FALSE);

		m_wndTab.m_pParent = this;

		if (m_Icons.GetSafeHandle () != NULL)
		{
			ASSERT (m_Icons.GetImageCount () == pTab->GetItemCount ());
			m_wndTab.SetImageList (m_Icons.GetSafeHandle ());
		}

		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndTab;
	}

	return NULL;
}
//****************************************************************************************
void CBCGPropertySheet::SetIconsList (HIMAGELIST hIcons)
{
	ASSERT_VALID(this);
	ASSERT (hIcons != NULL);
	ASSERT (m_Icons.GetSafeHandle () == NULL);

	m_Icons.Create (CImageList::FromHandle (hIcons));
}
//******************************************************************************************
void CBCGPropertySheet::AddCategoryToTree (CBCGPropSheetCategory* pCategory)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pCategory);
	ASSERT (m_look == PropSheetLook_Tree);

	HTREEITEM hParent = NULL;
	if (pCategory->m_pParentCategory != NULL)
	{
		hParent = pCategory->m_pParentCategory->m_hTreeItem;
	}

	pCategory->m_hTreeItem = m_wndTree.InsertItem (pCategory->m_strName, 
		I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
	m_wndTree.SetItemData (pCategory->m_hTreeItem, (DWORD_PTR) pCategory);

	for (POSITION pos = pCategory->m_lstSubCategories.GetHeadPosition (); pos != NULL;)
	{
		AddCategoryToTree (pCategory->m_lstSubCategories.GetNext (pos));
	}
}
//***************************************************************************************
BOOL CBCGPropertySheet::SetIconsList (UINT uiImageListResID, int cx,
							  COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiImageListResID);
	ASSERT(lpszResourceName != NULL);

	HBITMAP hbmp = (HBITMAP) ::LoadImage (
		AfxGetResourceHandle (),
		lpszResourceName,
		IMAGE_BITMAP,
		0, 0,
		LR_CREATEDIBSECTION);

	if (hbmp == NULL)
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;
	m_bAlphaBlendIcons = FALSE;

	BITMAP bmpObj;
	::GetObject (hbmp, sizeof (BITMAP), &bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

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
		m_bAlphaBlendIcons = TRUE;
		break;
	}

	icons.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	icons.Add (CBitmap::FromHandle (hbmp), clrTransparent);

	SetIconsList (icons);
	return TRUE;
}
//***************************************************************************************
void CBCGPropertySheet::OnActivatePage (CPropertyPage* pPage)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);

	if (m_wndOutlookBar.GetSafeHwnd () != NULL)
	{
		if (m_nActivePage >= 0)
		{
			m_wndOutlookBar.SetButtonStyle (m_nActivePage, 0);
		}

		int nPage = GetPageIndex (pPage);
		ASSERT (nPage >= 0);

		m_nActivePage = nPage;

		PostMessage (UM_AFTERACTIVATEPAGE);
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		CBCGPropertyPage* pBCGPropPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, pPage);
		if (pBCGPropPage != NULL)
		{
			if (!m_bIsInSelectTree)
			{
				m_wndTree.SelectItem (pBCGPropPage->m_hTreeNode);
			}

			m_wndTree.EnsureVisible (pBCGPropPage->m_hTreeNode);
		}
	}

	if (m_wndTab.GetSafeHwnd () != NULL)
	{
		const int nTab = GetPageIndex (pPage);

		m_wndTab.SetActiveTab (nTab);
		m_wndTab.EnsureVisible (nTab);
	}
}
//****************************************************************************************
LRESULT CBCGPropertySheet::OnAfterActivatePage(WPARAM,LPARAM)
{
	ASSERT_VALID (this);

	if (m_nActivePage >= 0)
	{
		if (m_wndOutlookBar.GetSafeHwnd () != NULL)
		{
			m_wndOutlookBar.SetButtonStyle (m_nActivePage, TBBS_CHECKED);
			m_wndOutlookBar.EnsureVisible (m_nActivePage);
		}
	}

	return 0;
}
//************************************************************************************
void CBCGPropertySheet::OnSelectTree(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	*pResult = 0;

	HTREEITEM hTreeItem = m_wndTree.GetSelectedItem ();
	if (hTreeItem == NULL)
	{
		return;
	}

	CBCGPropSheetCategory* pNewCategory = NULL;
	CBCGPropSheetCategory* pOldCategory = NULL;

	CBCGPropertyPage* pCurrPage = DYNAMIC_DOWNCAST (CBCGPropertyPage,
		GetActivePage ());
	if (pCurrPage != NULL)
	{
		ASSERT_VALID (pCurrPage);
		pOldCategory = pCurrPage->m_pCategory;
	}

	m_bIsInSelectTree = TRUE;

	CBCGPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPropertyPage,
		(CObject*) m_wndTree.GetItemData (hTreeItem));
	if (pPage != NULL)
	{
		CBCGPropertyPage* pPrevPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, GetActivePage ());

		ASSERT_VALID (pPage);
		SetActivePage (pPage);

		pNewCategory = pPage->m_pCategory;

		if (pPrevPage != NULL)
		{
			ASSERT_VALID (pPrevPage);

			CRect rectItem;
			m_wndTree.GetItemRect (pPrevPage->m_hTreeNode, rectItem, FALSE);
			m_wndTree.InvalidateRect (rectItem);
		}
	}
	else
	{
		CBCGPropSheetCategory* pCategory = DYNAMIC_DOWNCAST (CBCGPropSheetCategory,
			(CObject*) m_wndTree.GetItemData (hTreeItem));
		if (pCategory != NULL)
		{
			ASSERT_VALID (pCategory);

			while (!pCategory->m_lstSubCategories.IsEmpty ())
			{
				pCategory = pCategory->m_lstSubCategories.GetHead ();
				ASSERT_VALID (pCategory);
			}

			if (!pCategory->m_lstPages.IsEmpty ())
			{
				pPage = pCategory->m_lstPages.GetHead ();
				ASSERT_VALID (pPage);

				SetActivePage (pPage);

				CRect rectItem;
				m_wndTree.GetItemRect (pPage->m_hTreeNode, rectItem, FALSE);
				m_wndTree.InvalidateRect (rectItem);
			}

			pNewCategory = pCategory;
		}
	}

	if (pNewCategory != pOldCategory)
	{
		if (pOldCategory != NULL)
		{
			ASSERT_VALID (pOldCategory);
			HTREEITEM hItem = pOldCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand (hItem, TVE_COLLAPSE);
				hItem = m_wndTree.GetParentItem (hItem);
			}
			while (hItem != NULL);
		}

		if (pNewCategory != NULL)
		{
			ASSERT_VALID (pNewCategory);
			HTREEITEM hItem = pNewCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand (hItem, TVE_EXPAND);
				hItem = m_wndTree.GetParentItem (hItem);
			}
			while (hItem != NULL);
		}
	}

	m_bIsInSelectTree = FALSE;
}
//***************************************************************************************
void CBCGPropertySheet::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO) pNMHDR;

	CBCGPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPropertyPage,
		(CObject*) m_wndTree.GetItemData (lptvdi->item.hItem));
	if (pPage != NULL)
	{
		ASSERT_VALID (pPage);

		if (pPage == GetActivePage ())
		{
			lptvdi->item.iImage = pPage->m_nSelIconNum;
			lptvdi->item.iSelectedImage = pPage->m_nSelIconNum;
		}
		else
		{
			lptvdi->item.iImage = pPage->m_nIcon;
			lptvdi->item.iSelectedImage = pPage->m_nIcon;
		}
	}

	CBCGPropSheetCategory* pCategory = DYNAMIC_DOWNCAST (CBCGPropSheetCategory,
		(CObject*) m_wndTree.GetItemData (lptvdi->item.hItem));
	if (pCategory != NULL)
	{
		ASSERT_VALID (pCategory);

		if (lptvdi->item.state & TVIS_EXPANDED)
		{
			lptvdi->item.iImage = pCategory->m_nSelectedIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nSelectedIcon;
		}
		else
		{
			lptvdi->item.iImage = pCategory->m_nIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nIcon;
		}
	}

	*pResult = 0;
}
//********************************************************************************
CBCGTabWnd& CBCGPropertySheet::GetTab () const
{
	ASSERT_VALID (this);
	ASSERT (m_look == PropSheetLook_OneNoteTabs);

	return (CBCGTabWnd&) m_wndTab;
}
//********************************************************************************
BOOL CBCGPropertySheet::PreTranslateMessage(MSG* pMsg) 
{
	if (m_Impl.PreTranslateMessage (pMsg))
	{
		return TRUE;
	}

	return CPropertySheet::PreTranslateMessage(pMsg);
}
//********************************************************************************
void CBCGPropertySheet::OnSysColorChange() 
{
	CPropertySheet::OnSysColorChange();
	
	if (AfxGetMainWnd () == this)
	{
		globalData.UpdateSysColors ();
	}
}
//********************************************************************************
void CBCGPropertySheet::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CPropertySheet::OnSettingChange(uFlags, lpszSection);
	
	if (AfxGetMainWnd () == this)
	{
		globalData.OnSettingChange ();
	}
}
