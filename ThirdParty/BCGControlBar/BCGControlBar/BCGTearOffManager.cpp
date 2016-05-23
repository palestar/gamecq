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

// CBCGToolbarCustomize.cpp : implementation file
//

// BCGTearOffManager.cpp: implementation of the CBCGTearOffManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGTearOffManager.h"
#include "BCGWorkSpace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGTearOffManager*	g_pTearOffMenuManager = NULL;
extern CBCGWorkspace* g_pWorkspace;

//static const TCHAR cIDChar = _T('\n');
static const TCHAR cIDChar = 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGTearOffManager::CBCGTearOffManager() :
	m_uiTearOffMenuFirst (0),
	m_uiTearOffMenuLast (0)
{
}
//**************************************************************************************
CBCGTearOffManager::~CBCGTearOffManager()
{
	g_pTearOffMenuManager = NULL;
}
//**************************************************************************************
BOOL CBCGTearOffManager::Initialize (	LPCTSTR lpszRegEntry, 
										UINT uiTearOffMenuFirst, 
										UINT uiTearOffMenuLast)
{
	ASSERT (g_pTearOffMenuManager != NULL);
	ASSERT (uiTearOffMenuLast >= uiTearOffMenuFirst);

	if (uiTearOffMenuFirst == 0 || uiTearOffMenuLast == 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pTearOffMenuManager = this;

	m_uiTearOffMenuFirst = uiTearOffMenuFirst;
	m_uiTearOffMenuLast = uiTearOffMenuLast;

	m_strTearOfBarRegEntry = (lpszRegEntry == NULL) ? 
		( g_pWorkspace ? g_pWorkspace->GetRegSectionPath() : _T(""))
		: lpszRegEntry;

	int nCount = uiTearOffMenuLast - uiTearOffMenuFirst + 1;
	m_arTearOffIDsUsage.SetSize (nCount);

	for (int i = 0; i < nCount; i ++)
	{
		m_arTearOffIDsUsage [i] = 0;
	}

	return TRUE;
}
//**************************************************************************************
void CBCGTearOffManager::Reset (HMENU hMenu)
{
	int nCount = m_uiTearOffMenuLast - m_uiTearOffMenuFirst + 1;

	if (hMenu == NULL)	// Reset all
	{
		for (int i = 0; i < nCount; i ++)
		{
			m_arTearOffIDsUsage [i] = 0;
		}

		return;
	}

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		CString str;
		pMenu->GetMenuString (i, str, MF_BYPOSITION);

		UINT uiTearOffID = Parse (str);
		if (uiTearOffID >= m_uiTearOffMenuFirst && uiTearOffID <= m_uiTearOffMenuLast)
		{
			m_arTearOffIDsUsage [uiTearOffID - m_uiTearOffMenuFirst] = 0;
		}

		if (pMenu->GetMenuItemID (i) == (UINT)-1)
		{
			CMenu* pPopupMenu = pMenu->GetSubMenu (i);
			ASSERT (pPopupMenu != NULL);

			Reset (pPopupMenu->GetSafeHmenu ());
		}
	}
}
//**************************************************************************************
UINT CBCGTearOffManager::GetFreeTearOffID ()
{
	if (m_uiTearOffMenuFirst == 0 || m_uiTearOffMenuLast == 0)
	{
		ASSERT (FALSE);
		return 0;
	}

	int nCount = m_uiTearOffMenuLast - m_uiTearOffMenuFirst + 1;
	for (int i = 0; i < nCount; i ++)
	{
		if (m_arTearOffIDsUsage [i] == 0)
		{
			m_arTearOffIDsUsage [i] = 1;
			return m_uiTearOffMenuFirst + i;
		}
	}

	return 0;
}
//**************************************************************************************
void CBCGTearOffManager::SetupTearOffMenus (HMENU hMenu)
{
	ASSERT (hMenu != NULL);

	CMenu* pMenu = CMenu::FromHandle (hMenu);
	if (pMenu == NULL)
	{
		return;
	}

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiID = pMenu->GetMenuItemID (i);
		if (uiID != (UINT) -1)
		{
			continue;
		}

		UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);
		if (uiState & MF_MENUBARBREAK)
		{
			CString str;
			pMenu->GetMenuString (i, str, MF_BYPOSITION);

			if (str [0] != cIDChar)
			{
				UINT uiCtrlBarId = GetFreeTearOffID ();
				if (uiCtrlBarId == 0)	// No more free IDs!
				{						// Reserve more IDs in Initialize!!!
					ASSERT (FALSE);
					return;
				}

				Build (uiCtrlBarId, str);
				pMenu->ModifyMenu (i, MF_BYPOSITION, i, str);
			}
		}

		CMenu* pPopupMenu = pMenu->GetSubMenu (i);
		if (pPopupMenu != NULL)
		{
			SetupTearOffMenus (pPopupMenu->GetSafeHmenu ());
		}
	}
}
//*************************************************************************************
void CBCGTearOffManager::SetInUse (UINT uiCmdId, BOOL bUse/* = TRUE*/)
{
	if (uiCmdId < m_uiTearOffMenuFirst || uiCmdId > m_uiTearOffMenuLast)
	{
		return;
	}

	int nDelta = bUse ? 1 : -1;
	int iIndex = uiCmdId - m_uiTearOffMenuFirst;

	m_arTearOffIDsUsage [iIndex] += nDelta;

	if (m_arTearOffIDsUsage [iIndex] < 0)
	{
		m_arTearOffIDsUsage [iIndex] = 0;
	}
}
//*************************************************************************************
UINT CBCGTearOffManager::Parse (CString& str)
{
	if (str.IsEmpty () || str [0] != cIDChar)
	{
		return 0;
	}

	UINT uiID = _ttol (str.Mid (1));
	ASSERT (uiID != 0);

	int iOffset = str.ReverseFind (cIDChar);
	if (iOffset == -1)
	{
		ASSERT (FALSE);
		return 0;
	}

	str = str.Mid (iOffset + 1);
	return uiID;
}
//*************************************************************************************
void CBCGTearOffManager::Build (UINT uiTearOffBarID, CString& strText)
{
	ASSERT (uiTearOffBarID != 0);

	CString strNew;
	strNew.Format (_T("%c%d%c%s"), cIDChar, uiTearOffBarID, cIDChar, strText);
	strText = strNew;
}
