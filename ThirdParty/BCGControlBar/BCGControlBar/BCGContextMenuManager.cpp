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

// BCGContextMenuManager.cpp: implementation of the CBCGContextMenuManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGContextMenuManager.h"
#include "BCGTearOffManager.h"
#include "BCGPopupMenu.h"
#include "MenuHash.h"
#include "globals.h"
#include "RegPath.h"
#include "BCGDialog.h"
#include "BCGPropertyPage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const CString strMenusProfile = _T("BCGContextMenuManager");

CBCGContextMenuManager*	g_pContextMenuManager = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGContextMenuManager::CBCGContextMenuManager()
{
	ASSERT (g_pContextMenuManager == NULL);
	g_pContextMenuManager = this;
	m_nLastCommandID = 0;
	m_bTrackMode = FALSE;
}
//***********************************************************************************************
CBCGContextMenuManager::~CBCGContextMenuManager()
{
	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		::DestroyMenu (hMenu);
	}

	g_pContextMenuManager = NULL;
}
//**********************************************************************************
BOOL CBCGContextMenuManager::AddMenu(UINT uiMenuNameResId, UINT uiMenuResId)
{
	CString strMenuName;
	strMenuName.LoadString (uiMenuNameResId);

	return AddMenu (strMenuName, uiMenuResId);
}
//***********************************************************************************************
BOOL CBCGContextMenuManager::AddMenu(LPCTSTR lpszName, UINT uiMenuResId)
{
	ASSERT (lpszName != NULL);

	CMenu menu;
	if (!menu.LoadMenu (uiMenuResId))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	HMENU hExMenu;
	if (m_Menus.Lookup (uiMenuResId, hExMenu))
	{
		//------------------------------------------
		// Menu with the same name is already exist!
		//------------------------------------------
		return FALSE;
	}

	HMENU hMenu = menu.Detach ();

	if (g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetupTearOffMenus (hMenu);
	}

	m_Menus.SetAt (uiMenuResId, hMenu);
	m_MenuNames.SetAt (lpszName, hMenu);

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGContextMenuManager::ShowPopupMenu (UINT uiMenuResId, int x, int y, 
											CWnd* pWndOwner, BOOL bOwnMessage,
											BOOL bRightAlign)
{
	HMENU hMenu;
	if (!m_Menus.Lookup (uiMenuResId, hMenu) || hMenu == NULL)
	{
		return FALSE;
	}

	if (x == -1 && y == -1 &&	// Undefined position
		pWndOwner != NULL)
	{
		CRect rectParent;
		pWndOwner->GetClientRect (&rectParent);
		pWndOwner->ClientToScreen (&rectParent);

		x = rectParent.left + 5;
		y = rectParent.top + 5;
	}

	HMENU hmenuPopup = ::GetSubMenu (hMenu, 0);
	if (hmenuPopup == NULL)
	{
		#ifdef _DEBUG

		MENUITEMINFO info;
		memset (&info, 0, sizeof (MENUITEMINFO));

		if (!::GetMenuItemInfo (hMenu, 0, TRUE, &info))
		{
			TRACE (_T("Invalid menu: %d\n"), uiMenuResId);
		}
		else
		{
			ASSERT (info.hSubMenu == NULL);
			TRACE (_T("Menu %d, first option '%s' doesn't contain popup menu!\n"), 
					uiMenuResId, info.dwTypeData);
		}

		#endif // _DEBUG
		return FALSE;
	}

	return ShowPopupMenu (hmenuPopup, x, y, pWndOwner, bOwnMessage, TRUE,
		bRightAlign) != NULL;
}
//***********************************************************************************************
CBCGPopupMenu* CBCGContextMenuManager::ShowPopupMenu (HMENU hmenuPopup, int x, int y,
											CWnd* pWndOwner, BOOL bOwnMessage,
											BOOL bAutoDestroy, BOOL bRightAlign)
{
	if (pWndOwner != NULL &&
		pWndOwner->IsKindOf (RUNTIME_CLASS (CBCGDialog)) && !bOwnMessage)
	{
		// CBCGDialog should own menu messages
		ASSERT (FALSE);
		return NULL;
	}

	if (pWndOwner != NULL &&
		pWndOwner->IsKindOf (RUNTIME_CLASS (CBCGPropertyPage)) && !bOwnMessage)
	{
		// CBCGPropertyPage should own menu messages
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT (hmenuPopup != NULL);
	if (g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetupTearOffMenus (hmenuPopup);
	}

	if (m_bTrackMode)
	{
		bOwnMessage = TRUE;
	}

	if (!bOwnMessage)
	{
		while (pWndOwner != NULL && pWndOwner->GetStyle() & WS_CHILD)
		{
			pWndOwner = pWndOwner->GetParent ();
		}
	}

	CBCGPopupMenu* pPopupMenu = new CBCGPopupMenu;
	if (!globalData.bIsWindowsNT4 || bAutoDestroy)
	{
		pPopupMenu->SetAutoDestroy (FALSE);
	}

	pPopupMenu->m_bTrackMode = m_bTrackMode;
	pPopupMenu->SetRightAlign (bRightAlign);

	CBCGPopupMenu* pMenuActive = CBCGPopupMenu::GetActiveMenu ();
	if (pMenuActive != NULL)
	{
		pMenuActive->SendMessage (WM_CLOSE);
	}

	if (!pPopupMenu->Create (pWndOwner, x, y, hmenuPopup, FALSE, bOwnMessage))
	{
		return NULL;
	}

	return pPopupMenu;
}
//***********************************************************************************************
UINT CBCGContextMenuManager::TrackPopupMenu (HMENU hmenuPopup, int x, int y,
											 CWnd* pWndOwner, BOOL bRightAlign/* = FALSE*/)
{
	m_nLastCommandID = 0;

	CWinThread* pCurrThread = ::AfxGetThread ();
	if (pCurrThread == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	m_bTrackMode = TRUE;

	CBCGPopupMenu* pMenu = ShowPopupMenu (hmenuPopup, x, y, pWndOwner, 
		FALSE, TRUE, bRightAlign);

	if (pMenu != NULL)
	{
		CRect rect;
		pMenu->GetWindowRect (&rect);
		pMenu->UpdateShadow (&rect);
	}

	CBCGDialog* pParentDlg = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentDlg = DYNAMIC_DOWNCAST (CBCGDialog, pWndOwner->GetParent ());
		if (pParentDlg != NULL)
		{
			pParentDlg->SetActiveMenu (pMenu);
		}
	}

	CBCGPropertyPage* pParentPropPage = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentPropPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, pWndOwner->GetParent ());
		if (pParentPropPage != NULL)
		{
			pParentPropPage->SetActiveMenu (pMenu);
		}
	}

	m_bTrackMode = FALSE;

	if (pMenu != NULL && pCurrThread != NULL)
	{
		ASSERT_VALID (pMenu);

		HWND hwndMenu = pMenu->GetSafeHwnd ();
		while (::IsWindow (hwndMenu))
		{
			if (pMenu->m_bTobeDstroyed)
			{
				break;
			}

			MSG msg;
			while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostThreadMessage (GetCurrentThreadId(), 
						msg.message, msg.wParam, msg.lParam);
					return 0;
				}

				if (!::IsWindow (hwndMenu))
				{
					break;
				}

				if (pMenu->m_bTobeDstroyed)
				{
					break;
				}

				if (::IsWindow (hwndMenu) &&
					!pCurrThread->PreTranslateMessage (&msg))
				{
					::TranslateMessage (&msg);
					::DispatchMessage (&msg);
				}

				if (::IsWindow (hwndMenu) && pMenu->IsIdle ())
				{
					pCurrThread->OnIdle (0);
				}
			}

			if (!::IsWindow (hwndMenu))
			{
				break;
			}

			WaitMessage ();
		}
	}

	if (pParentDlg != NULL)
	{
		pParentDlg->SetActiveMenu (NULL);
	}

	if (pParentPropPage != NULL)
	{
		pParentPropPage->SetActiveMenu (NULL);
	}

	return m_nLastCommandID;
}
//***********************************************************************************************
void CBCGContextMenuManager::GetMenuNames (CStringList& listOfNames) const
{
	listOfNames.RemoveAll ();

	for (POSITION pos = m_MenuNames.GetStartPosition (); pos != NULL;)
	{
		CString strName;
		HMENU hMenu;

		m_MenuNames.GetNextAssoc (pos, strName, hMenu);
		listOfNames.AddTail (strName);
	}
}
//***********************************************************************************************
HMENU CBCGContextMenuManager::GetMenuByName (LPCTSTR lpszName, UINT* puiOrigResID) const
{
	HMENU hMenu;
	if (!m_MenuNames.Lookup (lpszName, hMenu))
	{
		return NULL;
	}

	if (puiOrigResID != NULL)
	{
		*puiOrigResID = 0;

		for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
		{
			UINT uiResId;
			HMENU hMenuMap;

			m_Menus.GetNextAssoc (pos, uiResId, hMenuMap);
			if (hMenuMap == hMenu)
			{
				*puiOrigResID = uiResId;
				break;
			}
		}
	}

	return hMenu;
}
//***********************************************************************************************
BOOL CBCGContextMenuManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		CBCGPopupMenuBar bar;
		if (!bar.ImportFromMenu (hPopupMenu))
		{
			return FALSE;
		}

		if (bar.LoadState (strProfileName, 0, uiResId))
		{
			g_menuHash.SaveMenuBar (hPopupMenu, &bar);
		}
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGContextMenuManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGGetRegPath (strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		CBCGPopupMenuBar bar;
		if (g_menuHash.LoadMenuBar (hPopupMenu, &bar))
		{
			if (!bar.SaveState (strProfileName, 0, uiResId))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGContextMenuManager::ResetState ()
{
	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		g_menuHash.RemoveMenu (hPopupMenu);
	}

	return TRUE;
}
