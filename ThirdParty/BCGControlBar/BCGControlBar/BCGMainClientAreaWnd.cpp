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

// BCGMainClientAreaWnd.cpp : implementation file
//

#include "stdafx.h"
#include "BCGMainClientAreaWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGMDIChildWnd.h"
#include "BCGVisualManager.h"
#include "BCGMenuBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define UM_UPDATE_TABS	(WM_USER + 101)

/////////////////////////////////////////////////////////////////////////////
// CBCGMainClientAreaWnd

CBCGMainClientAreaWnd::CBCGMainClientAreaWnd()
{
	m_bTabIsVisible = FALSE;
	m_bTabIcons = TRUE;
	m_tabLocation = CBCGTabWnd::LOCATION_BOTTOM;
	m_bTabCloseButton = FALSE;
}
//*************************************************************************************
CBCGMainClientAreaWnd::~CBCGMainClientAreaWnd()
{
}
//*************************************************************************************
void CBCGMainClientAreaWnd::EnableMDITabs (BOOL bEnable/* = TRUE*/,
		BOOL bIcons/* = TRUE*/,
		CBCGTabWnd::Location tabLocation /* = CBCGTabWnd::LOCATION_BOTTOM*/,
		BOOL bHideNoTabs/* = FALSE*/,
		BOOL bCloseButton/* = FALSE */,
		CBCGTabWnd::Style style/* = CBCGTabWnd::STYLE_3D_SCROLLED*/)
{
	m_bTabIsVisible = bEnable;
	m_bTabIcons = bIcons;
	m_wndTab.m_location = m_tabLocation = tabLocation;
	m_wndTab.HideNoTabs (bHideNoTabs);
	m_wndTab.m_bCloseBtn = m_bTabCloseButton = bCloseButton;
	
	m_wndTab.ModifyTabStyle (style);

	if (bEnable)
	{
		UpdateTabs ();
	}

	m_wndTab.RecalcLayout ();

	if (GetSafeHwnd () != NULL && GetParentFrame () != NULL)
	{
		GetParentFrame ()->RecalcLayout ();

		UINT uiRedrawFlags =	RDW_ALLCHILDREN | RDW_FRAME | RDW_INVALIDATE | 
								RDW_UPDATENOW | RDW_ERASE;

		if (m_wndTab.GetSafeHwnd () != NULL)
		{
			m_wndTab.RedrawWindow (NULL, NULL, uiRedrawFlags);
		}

		RedrawWindow (NULL, NULL, uiRedrawFlags);
	}
}

BEGIN_MESSAGE_MAP(CBCGMainClientAreaWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGMainClientAreaWnd)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MDISETMENU,OnSetMenu)
	ON_MESSAGE(WM_MDIREFRESHMENU, OnMDIRefreshMenu)
	ON_MESSAGE(WM_MDIDESTROY, OnMDIDestroy)
	ON_MESSAGE(UM_UPDATE_TABS, OnUpdateTabs)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGMainClientAreaWnd message handlers

afx_msg LRESULT CBCGMainClientAreaWnd::OnSetMenu (WPARAM wp, LPARAM lp)
{
	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, GetParentFrame ());
	if (pMainFrame != NULL && ::IsWindow (pMainFrame->GetSafeHwnd ()))
	{
		if (pMainFrame->OnSetMenu ((HMENU) wp))
		{
			wp = NULL;
		}
	}
	else
	{
		wp = NULL;
	}

	return DefWindowProc (WM_MDISETMENU, wp, lp);
}
//*********************************************************************************
LRESULT CBCGMainClientAreaWnd::OnMDIRefreshMenu (WPARAM /*wp*/, LPARAM /*lp*/)
{
	LRESULT lRes = Default ();

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, GetParentFrame());
	if (pMainFrame != NULL && pMainFrame->GetMenuBar() != NULL) 
	{

		CBCGMenuBar *MenuBar = const_cast <CBCGMenuBar *>(pMainFrame->GetMenuBar());
		pMainFrame->m_hmenuWindow = pMainFrame->GetWindowMenuPopup( MenuBar->GetMenu());
	}

	return lRes;
}
//*********************************************************************************
BOOL CBCGMainClientAreaWnd::OnEraseBkgnd(CDC* pDC) 
{
	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, GetParentFrame ());
	if (pMainFrame != NULL && pMainFrame->OnEraseMDIClientBackground (pDC))
	{
		return TRUE;
	}

	return CWnd::OnEraseBkgnd(pDC);
}
//***********************************************************************************
LRESULT CBCGMainClientAreaWnd::OnMDIDestroy(WPARAM wParam, LPARAM)
{
	int nTabsHeight = m_wndTab.GetTabsHeight ();

	int iTab = m_wndTab.GetTabFromHwnd ((HWND)wParam);
	if (iTab >= 0)
	{
		CBCGMDIChildWnd* pMDIChild = DYNAMIC_DOWNCAST(CBCGMDIChildWnd, m_wndTab.GetTabWnd (iTab));
		if (pMDIChild != NULL)
		{
			pMDIChild->m_bToBeDestroyed = TRUE;
		}

		m_wndTab.RemoveTab (iTab);
	}

	LRESULT lRes = Default ();

	if (nTabsHeight != m_wndTab.GetTabsHeight () && GetParentFrame () != NULL)
	{
		GetParentFrame ()->RecalcLayout ();
	}

	return lRes;
}
//***********************************************************************************
void CBCGMainClientAreaWnd::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	if (m_wndTab.GetSafeHwnd () != NULL)
	{
		BOOL bRedraw = FALSE;

		if (m_bTabIsVisible)
		{
			CRect rectOld;
			m_wndTab.GetWindowRect (rectOld);

			m_wndTab.SetWindowPos (NULL, 
				lpClientRect->left, lpClientRect->top,
				lpClientRect->right - lpClientRect->left,
				lpClientRect->bottom - lpClientRect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			CRect rectTabClient;
			m_wndTab.GetClientRect (rectTabClient);

			CRect rectTabWnd;
			m_wndTab.GetWndArea (rectTabWnd);

			lpClientRect->top += (rectTabWnd.top - rectTabClient.top);
			lpClientRect->bottom += (rectTabWnd.bottom - rectTabClient.bottom);
			lpClientRect->left += (rectTabWnd.left - rectTabClient.left);
			lpClientRect->right += (rectTabWnd.right - rectTabClient.right);

			m_wndTab.ShowWindow (SW_SHOWNA);

			CRect rectNew;
			m_wndTab.GetWindowRect (rectNew);

			bRedraw = (rectOld != rectNew);
		}
		else
		{
			m_wndTab.ShowWindow (SW_HIDE);
		}

		SetWindowPos (NULL, 
				lpClientRect->left, lpClientRect->top,
				lpClientRect->right - lpClientRect->left,
				lpClientRect->bottom - lpClientRect->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, 
				RDW_ALLCHILDREN | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW |
				RDW_ERASE | RDW_INTERNALPAINT);
		}
	}

	CWnd::CalcWindowRect(lpClientRect, nAdjustType);
}
//***********************************************************************************
void CBCGMainClientAreaWnd::SetActiveTab (HWND hwnd)
{
	if (m_bTabIsVisible)
	{
		int iTab = m_wndTab.GetTabFromHwnd (hwnd);
		if (iTab >= 0)
		{
			m_wndTab.SetActiveTab (iTab);
		}
	}
}
//************************************************************************************
LRESULT CBCGMainClientAreaWnd::OnUpdateTabs (WPARAM, LPARAM)
{
	UpdateTabs ();
	return 0;
}
//**************************************************************************************
void CBCGMainClientAreaWnd::PreSubclassWindow() 
{
	CWnd::PreSubclassWindow();

	//-------------------------
	// Create MDI tabs control:
	//-------------------------
    if (!m_wndTab.Create (CBCGTabWnd::STYLE_3D_SCROLLED, CRect (0, 0, 0, 0), 
		GetParentFrame (), (UINT)-1, m_tabLocation, m_bTabCloseButton))
	{
		TRACE(_T("CBCGMainClientAreaWnd::OnCreate: can't create tabs window\n"));
		return;
	}

	m_wndTab.HideInactiveWindow (FALSE);
	m_wndTab.AutoSizeWindow (FALSE);
	m_wndTab.AutoDestroyWindow (FALSE);
	m_wndTab.SetFlatFrame ();
	m_wndTab.m_bTransparent = TRUE;
	m_wndTab.m_bTopEdge = TRUE;
	m_wndTab.SetDrawNoPrefix (TRUE, FALSE);

	if (!m_bTabIsVisible)
	{
		m_wndTab.ShowWindow (SW_HIDE);
	}

	//------------------
	// Create tab icons:
	//------------------
	m_TabIcons.Create (
		::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON), 
		ILC_COLOR32 | ILC_MASK, 0, 1);
}
//*************************************************************************************
void CBCGMainClientAreaWnd::UpdateTabs (BOOL bSetActiveTabVisible/* = FALSE*/)
{
	if (m_wndTab.GetSafeHwnd () == NULL || !m_bTabIsVisible)
	{
		return;
	}

	BOOL bRecalcLayout = FALSE;

	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);

		CBCGMDIChildWnd* pMDIChild = DYNAMIC_DOWNCAST(CBCGMDIChildWnd, pWndChild);

		//--------------
		// Get tab icon:
		//--------------
		int iIcon = -1;
		if (m_bTabIcons)
		{
			HICON hIcon = NULL;
			if (pMDIChild != NULL)
			{
				hIcon = pMDIChild->GetFrameIcon ();
			}
			else
			{
				if ((hIcon = pWndChild->GetIcon (FALSE)) == NULL)
				{
					hIcon = (HICON)(LONG_PTR) GetClassLongPtr (*pWndChild, GCLP_HICONSM);
				}
			}

			if (hIcon != NULL)
			{
				if (!m_mapIcons.Lookup (hIcon, iIcon))
				{
					iIcon = m_TabIcons.Add (hIcon);
					m_mapIcons.SetAt (hIcon, iIcon);

					if (m_TabIcons.GetImageCount () == 1)
					{
						m_wndTab.SetImageList (m_TabIcons.GetSafeHandle ());
					}
				}
			}
		}

		//--------------------------------
		// Get tab label (window caption):
		//--------------------------------
		CString strTabLabel;
		if (pMDIChild != NULL)
		{
			strTabLabel = pMDIChild->GetFrameText ();
		}
		else
		{
			pWndChild->GetWindowText (strTabLabel);
		}

		int iTabIndex = m_wndTab.GetTabFromHwnd (pWndChild->GetSafeHwnd ());
		if (iTabIndex >= 0)
		{
			//---------------------------------
			// Tab is already exist, update it:
			//---------------------------------
			if (pWndChild->GetStyle () & WS_VISIBLE)
			{
				CString strCurTabLabel;
				m_wndTab.GetTabLabel (iTabIndex, strCurTabLabel);

				if (strCurTabLabel != strTabLabel)
				{
					//-----------------------------
					// Text was changed, update it:
					//-----------------------------
					m_wndTab.SetTabLabel (iTabIndex, strTabLabel);
					bRecalcLayout = TRUE;
				}

				if (m_wndTab.GetTabIcon (iTabIndex) != (UINT) iIcon)
				{
					//-----------------------------
					// Icon was changed, update it:
					//-----------------------------
					m_wndTab.SetTabIcon (iTabIndex, iIcon);
					bRecalcLayout = TRUE;
				}
			}
			else
			{
				//----------------------------------
				// Window is hidden now, remove tab:
				//----------------------------------
				m_wndTab.RemoveTab (iTabIndex);
				bRecalcLayout = TRUE;
			}
		}
		else if (pMDIChild == NULL || !pMDIChild->m_bToBeDestroyed)
		{
			//----------------------
			// New item, add it now:
			//----------------------
			m_wndTab.AddTab (pWndChild, strTabLabel, iIcon);
			m_wndTab.SetActiveTab (m_wndTab.GetTabsNum () - 1);

			bRecalcLayout = TRUE;
		}

		pWndChild = pWndChild->GetNextWindow ();
    }

	if (bRecalcLayout && GetParentFrame () != NULL)
	{
		GetParentFrame ()->RecalcLayout ();
	}

	if (bSetActiveTabVisible)
	{
		m_wndTab.EnsureVisible (m_wndTab.GetActiveTab ());
	}
}
