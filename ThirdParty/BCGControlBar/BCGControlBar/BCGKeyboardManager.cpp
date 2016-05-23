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

// BCGKeyboardManager.cpp: implementation of the CBCGKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGKeyboardManager.h"
#include "BCGMultiDocTemplate.h"
#include "BCGFrameWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGRegistry.h"
#include "KeyHelper.h"
#include "BCGToolBar.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REG_SECTION_FMT		_T("%sBCGKeyboard-%d")
#define REG_ENTRY_DATA		_T("Accelerators")

CBCGKeyboardManager*	g_pKeyboardManager = NULL;

static const CString strKbProfile = _T("BCGKeyboardManager");

LPACCEL CBCGKeyboardManager::m_lpAccel = NULL;
LPACCEL CBCGKeyboardManager::m_lpAccelDefault = NULL;
int	CBCGKeyboardManager::m_nAccelDefaultSize = 0;
int	CBCGKeyboardManager::m_nAccelSize = 0;
HACCEL CBCGKeyboardManager::m_hAccelDefaultLast = NULL;
HACCEL CBCGKeyboardManager::m_hAccelLast = NULL;

// a special struct that will cleanup automatically
class _KBD_TERM
{
public:
	~_KBD_TERM()
	{
		CBCGKeyboardManager::CleanUp ();
	}
};

static const _KBD_TERM kbdTerm;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGKeyboardManager::CBCGKeyboardManager()
{
	ASSERT (g_pKeyboardManager == NULL);
	g_pKeyboardManager = this;
}
//******************************************************************
CBCGKeyboardManager::~CBCGKeyboardManager()
{
	g_pKeyboardManager = NULL;
}
//******************************************************************
BOOL CBCGKeyboardManager::UpdateAcellTable (CMultiDocTemplate* pTemplate,
											LPACCEL lpAccel, int nSize,
											CFrameWnd* pDefaultFrame)
{
	ASSERT (lpAccel != NULL);

	//--------------------------------
	// Create a new accelerator table:
	//--------------------------------
    HACCEL hAccelNew = ::CreateAcceleratorTable(lpAccel, nSize);
	if (hAccelNew == NULL)
	{
		TRACE(_T("Can't create accelerator table!\n"));
		return FALSE;
	}

	if (!UpdateAcellTable (pTemplate, hAccelNew, pDefaultFrame))
	{
		::DestroyAcceleratorTable (hAccelNew);
		return FALSE;
	}

	return TRUE;
}
//******************************************************************
BOOL CBCGKeyboardManager::UpdateAcellTable (CMultiDocTemplate* pTemplate,
											HACCEL hAccelNew,
											CFrameWnd* pDefaultFrame)
{
	ASSERT (hAccelNew != NULL);

	//-------------------------------------------------------------
	// Find an existing accelerator table associated with template:
	//-------------------------------------------------------------
	HACCEL hAccelTable = NULL;

	if (pTemplate != NULL)
	{
		ASSERT (pDefaultFrame == NULL);

		ASSERT_VALID (pTemplate);
		hAccelTable = pTemplate->m_hAccelTable;
		ASSERT (hAccelTable != NULL);

		pTemplate->m_hAccelTable = hAccelNew;

		//--------------------------------------------------
		// Walk trougth all template's documents and change
		// frame's accelerator tables:
		//--------------------------------------------------
		for (POSITION pos = pTemplate->GetFirstDocPosition(); pos != NULL;)
		{
			CDocument* pDoc = pTemplate->GetNextDoc (pos);
			ASSERT_VALID (pDoc);

			for (POSITION posView = pDoc->GetFirstViewPosition(); 
				posView != NULL;)
			{
				CView* pView = pDoc->GetNextView (posView);
				ASSERT_VALID (pView);

				CFrameWnd* pFrame = pView->GetParentFrame ();
				ASSERT_VALID (pFrame);

				if (pFrame->m_hAccelTable == hAccelTable)
				{
					pFrame->m_hAccelTable = hAccelNew;
				}
			}
		}
	}
	else
	{
		if (pDefaultFrame == NULL)
		{
			pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
		}

		if (pDefaultFrame != NULL)
		{
			hAccelTable = pDefaultFrame->m_hAccelTable;
			pDefaultFrame->m_hAccelTable = hAccelNew;
		}
	}

	if (hAccelTable == NULL)
	{
		TRACE(_T("Accelerator table not found!\n"));
		return FALSE;
	}

	::DestroyAcceleratorTable (hAccelTable);
	return TRUE;
}
//************************************************************************************************
BOOL CBCGKeyboardManager::SaveAccelaratorState (LPCTSTR lpszProfileName, 
	UINT uiResId, HACCEL hAccelTable)
{
	ASSERT (hAccelTable != NULL);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, lpszProfileName, uiResId);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strSection))
	{
		return FALSE;
	}

	int nAccelSize = ::CopyAcceleratorTable (hAccelTable, NULL, 0);

	LPACCEL lpAccel = new ACCEL [nAccelSize];
	ASSERT (lpAccel != NULL);

	::CopyAcceleratorTable (hAccelTable, lpAccel, nAccelSize);

	reg.Write (REG_ENTRY_DATA, (LPBYTE) lpAccel, nAccelSize * sizeof (ACCEL));

	delete lpAccel;
	return TRUE;
}
//************************************************************************************************
BOOL CBCGKeyboardManager::LoadAccelaratorState (LPCTSTR lpszProfileName, 
	UINT uiResId, HACCEL& hAccelTable)
{
	ASSERT (hAccelTable == NULL);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, lpszProfileName, uiResId);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	UINT uiSize;
	LPACCEL lpAccel;

	if (reg.Read (REG_ENTRY_DATA, (LPBYTE*) &lpAccel, &uiSize))
	{
		int nAccelSize = uiSize / sizeof (ACCEL);

		ASSERT (lpAccel != NULL);

		for (int i = 0; i < nAccelSize; i ++)
		{
			if (!CBCGToolBar::IsCommandPermitted (lpAccel [i].cmd))
			{
				lpAccel [i].cmd = 0;
			}
		}

		hAccelTable = ::CreateAcceleratorTable(lpAccel, nAccelSize);
	}

	delete lpAccel;
	return hAccelTable != NULL;
}
//************************************************************************************************
BOOL CBCGKeyboardManager::LoadState (LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::BCGGetRegPath (strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGMultiDocTemplate* pTemplate = 
				(CBCGMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects with
			// the sahred menu only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			HACCEL hAccellTable = NULL;
			if (LoadAccelaratorState (strProfileName, uiResId, hAccellTable))
			{
				UpdateAcellTable (pTemplate, hAccellTable);
			}
		}
	}

	//--------------------------------
	// Save default accelerator table:
	//--------------------------------
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		HACCEL hAccelTable = NULL;
		if (LoadAccelaratorState (strProfileName, 0, hAccelTable))
		{
			UpdateAcellTable (NULL, hAccelTable, pDefaultFrame);
		}
	}

	return TRUE;
}
//************************************************************************************************
BOOL CBCGKeyboardManager::SaveState (LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::BCGGetRegPath (strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGMultiDocTemplate* pTemplate = 
				(CBCGMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			SaveAccelaratorState (strProfileName, uiResId, pTemplate->m_hAccelTable);
		}
	}

	//--------------------------------
	// Save default accelerator table:
	//--------------------------------
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		SaveAccelaratorState (strProfileName, 0, pDefaultFrame->m_hAccelTable);
	}

	return TRUE;
}
//******************************************************************
void CBCGKeyboardManager::ResetAll ()
{
	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGMultiDocTemplate* pTemplate = 
				(CBCGMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE (uiResId), RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, MAKEINTRESOURCE (uiResId));
			if (hAccellTable != NULL)
			{
				UpdateAcellTable (pTemplate, hAccellTable);
			}
		}
	}

	//-----------------------------------
	// Restore default accelerator table:
	//-----------------------------------
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		UINT uiResId = 0;

		CBCGMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, AfxGetMainWnd ());
		if (pMDIFrame != NULL)
		{
			uiResId = pMDIFrame->GetDefaultResId ();
		}
		else	// Maybe, SDI frame...
		{
			CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, AfxGetMainWnd ());
			if (pFrame != NULL)
			{
				uiResId = pFrame->GetDefaultResId ();
			}
			else	// Maybe, OLE frame...
			{
				CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, AfxGetMainWnd ());
				if (pOleFrame != NULL)
				{
					uiResId = pOleFrame->GetDefaultResId ();
				}
			}
		}
		
		if (uiResId != 0)
		{
			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE (uiResId), RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, MAKEINTRESOURCE (uiResId));
			if (hAccellTable != NULL)
			{
				UpdateAcellTable (NULL, hAccellTable);
			}
		}
	}
}
//******************************************************************
BOOL CBCGKeyboardManager::FindDefaultAccelerator (UINT uiCmd, CString& str, 
												CFrameWnd* pWndFrame,
												BOOL bIsDefaultFrame)
{
	str.Empty ();

	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator ();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable (	lpAccel,
					bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast,
					nSize, hAccelTable);

	ASSERT (lpAccel != NULL);

	BOOL bFound = FALSE;
	for (int i = 0; !bFound && i < nSize; i ++)
	{
		if (lpAccel [i].cmd == uiCmd)
		{
			CBCGKeyHelper helper (&lpAccel [i]);
			helper.Format (str);

			bFound = TRUE;
		}
	}

	return bFound;
}
//******************************************************************
BOOL CBCGKeyboardManager::IsKeyHandled (WORD nKey, BYTE fVirt,
										CFrameWnd* pWndFrame,
										BOOL bIsDefaultFrame)
{
	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator ();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable (	lpAccel,
					bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast,
					nSize, hAccelTable);

	ASSERT (lpAccel != NULL);

	for (int i = 0; i < nSize; i ++)
	{
		if (lpAccel [i].key == nKey &&
			lpAccel [i].fVirt == fVirt)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************************************
void CBCGKeyboardManager::SetAccelTable (LPACCEL& lpAccel, HACCEL& hAccelLast, 
										 int& nSize, const HACCEL hAccelCur)
{
	ASSERT (hAccelCur != NULL);
	if (hAccelCur == hAccelLast)
	{
		ASSERT (lpAccel != NULL);
		return;
	}

	//--------------------------------
	// Destroy old acceleration table:
	//--------------------------------
	if (lpAccel != NULL)
	{
		delete lpAccel;
		lpAccel = NULL;
	}

	nSize = ::CopyAcceleratorTable (hAccelCur, NULL, 0);

	lpAccel = new ACCEL [nSize];
	ASSERT (lpAccel != NULL);

	::CopyAcceleratorTable (hAccelCur, lpAccel, nSize);

	hAccelLast = hAccelCur;
}
//************************************************************************************
UINT CBCGKeyboardManager::TranslateCharToUpper (const UINT nChar)
{
	if (nChar < VK_NUMPAD0 || nChar > VK_NUMPAD9 ||
		(::GetAsyncKeyState (VK_MENU) & 0x8000))
	{
		if (!CBCGToolBar::m_bExtCharTranslation ||
			nChar < 0x41 || nChar > 0x5A)
		{
			if (::GetAsyncKeyState (VK_MENU) & 0x8000)
			{
				return nChar;
			}
			else
			{
				return toupper (nChar);
			}
		}
	}

	WORD wChar;
	BYTE lpKeyState [256];
	::GetKeyboardState (lpKeyState);

	::ToAsciiEx (nChar,
				MapVirtualKey (nChar, 0),
				lpKeyState,
				&wChar,
				1,
				::GetKeyboardLayout (AfxGetThread()->m_nThreadID));

	TCHAR szChar [2] = {(TCHAR) wChar, '\0'};
	CharUpper(szChar);

	return (UINT)szChar [0];
}
//************************************************************************************
void CBCGKeyboardManager::CleanUp ()
{
	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
		m_lpAccel = NULL;

	}

	if (m_lpAccelDefault != NULL)
	{
		delete [] m_lpAccelDefault;
		m_lpAccelDefault = NULL;
	}
}
