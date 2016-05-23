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
// BCGShellManager.cpp: implementation of the CBCGShellManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGShellManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGShellManager* g_pShellManager = NULL;

UINT BCGM_ON_AFTER_SHELL_COMMAND	= ::RegisterWindowMessage (_T("BCGM_ON_AFTER_SHELL_COMMAND"));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGShellManager::CBCGShellManager()
{
	ASSERT (g_pShellManager == NULL);
	g_pShellManager = this;
	m_lpszInitialPath = NULL;

	SHGetMalloc (&m_pBCGMalloc);
}
//***************************************************************************************
CBCGShellManager::~CBCGShellManager()
{
	g_pShellManager = NULL;

	if (m_pBCGMalloc != NULL)
	{
		m_pBCGMalloc->Release ();
		m_pBCGMalloc = NULL;
	}
}
//****************************************************************************************
int CALLBACK CBCGShellManager::BrowseCallbackProc (
	HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		CBCGShellManager* pThis = (CBCGShellManager*) lpData;
		ASSERT_VALID (pThis);

		if (pThis->m_lpszInitialPath != NULL)
		{
			SendMessage (hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, 
				(LPARAM)pThis->m_lpszInitialPath);
		}
	}

	return 0;
}
//****************************************************************************************
BOOL CBCGShellManager::BrowseForFolder (CString& strFolder,
										CWnd* pWndParent/* = NULL*/,
										LPCTSTR lplszInitialFolder/* = NULL*/,
										LPCTSTR lpszTitle/* = NULL*/,
										UINT ulFlags/* = BIF_RETURNONLYFSDIRS */,
										LPINT piFolderImage/* = NULL*/)
{
	TCHAR szDisplayName [MAX_PATH];

	BROWSEINFO bi;
	ZeroMemory (&bi, sizeof (bi));

	bi.lpszTitle = lpszTitle != NULL ? lpszTitle : _T("");
	bi.pszDisplayName = szDisplayName;
	bi.hwndOwner = pWndParent->GetSafeHwnd ();
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM) this;
	bi.pidlRoot = NULL;
	bi.ulFlags = ulFlags;
	bi.iImage = -1;

	m_lpszInitialPath = lplszInitialFolder;

	BOOL bRes = FALSE;

	LPITEMIDLIST pidlRes = SHBrowseForFolder (&bi);
	if (pidlRes != NULL)
	{
		TCHAR szPath [MAX_PATH];
		if (SHGetPathFromIDList (pidlRes, szPath))
		{
			strFolder = szPath;

			if (piFolderImage != NULL)
			{
				*piFolderImage = bi.iImage;
			}

			bRes = TRUE;
		}

		FreeItem (pidlRes);
	}

	m_lpszInitialPath = NULL;
	return bRes;
}
//***************************************************************************************
LPITEMIDLIST CBCGShellManager::GetNextItem (LPCITEMIDLIST pidl)
{
	if (pidl == NULL)
	{
		return NULL;
	}

	return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl) + pidl->mkid.cb);
}
//***************************************************************************************
LPITEMIDLIST CBCGShellManager::CreateItem (UINT cbSize)
{
	ASSERT (m_pBCGMalloc != NULL);

	LPITEMIDLIST pidl = (LPITEMIDLIST) m_pBCGMalloc->Alloc (cbSize);
	if (pidl != NULL)
	{
		ZeroMemory (pidl, cbSize);
	}
	
	return pidl;
}
//***************************************************************************************
UINT CBCGShellManager::GetItemCount (LPCITEMIDLIST pidl)
{
	if (pidl == NULL)
	{
		return 0;
	}

	UINT nCount = 0;

	for (UINT nSizeCurr = pidl->mkid.cb; nSizeCurr != 0; nCount++)
	{
		pidl = GetNextItem (pidl);
		nSizeCurr = pidl->mkid.cb;
	} 
	
	return nCount;
}
//***************************************************************************************
UINT CBCGShellManager::GetItemSize (LPCITEMIDLIST pidl)
{
	UINT           cbTotal = 0;
	LPITEMIDLIST   pidlTemp = (LPITEMIDLIST) pidl;
	
	if (pidlTemp != NULL)
	{
		while (pidlTemp->mkid.cb != 0)
		{
			cbTotal += pidlTemp->mkid.cb;
			pidlTemp = GetNextItem (pidlTemp);
		}  
		
		// Requires a 16 bit zero value for the NULL terminator
		cbTotal += 2 * sizeof(BYTE);
	}
	
	return cbTotal;
}
//****************************************************************************************
LPITEMIDLIST CBCGShellManager::ConcatenateItem (LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	UINT cb1 = 0;
	
	//-----------------------------------------------------------------------
	// Pidl1 can possibly be NULL if it points to the desktop.  Since we only
	// need a single NULL terminator, we remove the extra 2 bytes from the
	// size of the first ITEMIDLIST.
	//-----------------------------------------------------------------------
	if (pidl1 != NULL)
	{
		cb1 = GetItemSize (pidl1) - (2 * sizeof(BYTE));
	}
	
	UINT cb2 = GetItemSize (pidl2);
	
	//-----------------------------------------------------------------------
	// Create a new ITEMIDLIST that is the size of both pidl1 and pidl2, then
	// copy pidl1 and pidl2 to the new list.
	//-----------------------------------------------------------------------
	LPITEMIDLIST pidlNew = CreateItem (cb1 + cb2);

	if (pidlNew != NULL)
	{
		if (pidl1 != NULL)
		{
			CopyMemory(pidlNew, pidl1, cb1);
		}
		
		CopyMemory (((LPBYTE)pidlNew) + cb1, pidl2, cb2);
	}
	
	return pidlNew;
}
//****************************************************************************************
LPITEMIDLIST CBCGShellManager::CopyItem (LPCITEMIDLIST pidlSource)
{
	ASSERT (m_pBCGMalloc != NULL);

	if (pidlSource == NULL)
	{
		return NULL;
	}
	
	UINT cbSource = GetItemSize (pidlSource);
	LPITEMIDLIST pidlTarget = (LPITEMIDLIST) m_pBCGMalloc->Alloc (cbSource);

	if (pidlTarget == NULL)
	{
		return NULL;
	}
	
	CopyMemory (pidlTarget, pidlSource, cbSource);
	return pidlTarget;
}
//****************************************************************************************
void CBCGShellManager::FreeItem (LPITEMIDLIST pidl)
{
	ASSERT (m_pBCGMalloc != NULL);

	if (pidl != NULL)
	{
		m_pBCGMalloc->Free (pidl);
	}
}
//***************************************************************************************
HRESULT CBCGShellManager::ItemFromPath (LPCTSTR lpszPath, LPITEMIDLIST& pidl)
{
	ASSERT (lpszPath != NULL);

	LPSHELLFOLDER pDesktopFolder;
	HRESULT hr = SHGetDesktopFolder (&pDesktopFolder);

	if (FAILED (hr))
	{
		return hr;
	}

	OLECHAR olePath [MAX_PATH];

	//------------------------------------------------------------ 
	// IShellFolder::ParseDisplayName requires the file name be in
	// Unicode.
	//------------------------------------------------------------ 
#ifdef _UNICODE
	lstrcpy (olePath, lpszPath);
#else
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszPath, -1,
		olePath, MAX_PATH);
#endif
	
	//-----------------------------------
	// Convert the path to an ITEMIDLIST.
	//-----------------------------------
	ULONG chEaten;
	ULONG dwAttributes;
	hr = pDesktopFolder->ParseDisplayName (
		NULL,
		NULL,
		olePath,
		&chEaten,
		&pidl,
		&dwAttributes);
	
	pDesktopFolder->Release ();
	return hr;
}
//***************************************************************************************
int CBCGShellManager::GetParentItem (LPCITEMIDLIST lpidl, LPITEMIDLIST& lpidlParent)
{
    UINT nCount = GetItemCount (lpidl);

	if (nCount == 0) // Desktop folder
	{
		return -1;
	}

	if (nCount == 1)
	{
		//----------------
		// Assume desktop:
		//----------------
		SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOP, &lpidlParent);
		return 0;
	}

	USHORT uiParentSize = 0;
	LPCITEMIDLIST lpidlCurr = lpidl;

    for (UINT i = 0; i < nCount - 1; i++)
    {
        uiParentSize = (USHORT)(uiParentSize + lpidlCurr->mkid.cb);
      	lpidlCurr = GetNextItem (lpidlCurr);
    } 
	
	lpidlParent = CreateItem (uiParentSize + 2);
    CopyMemory ((LPBYTE) lpidlParent, (LPBYTE) lpidl, uiParentSize);

	return nCount - 1;
}
