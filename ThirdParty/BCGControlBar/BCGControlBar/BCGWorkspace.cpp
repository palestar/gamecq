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

#include "stdafx.h"
#include "multimon.h"

#include "globals.h"
#include "bcgcontrolbar.h"
#include "BCGToolBar.h"
#include "BCGSizingControlBar.h"
#include "BCGWorkspace.h"

#include "BCGFrameImpl.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"

#include "BCGMouseManager.h"
#include "BCGShellManager.h"
#include "BCGContextMenuManager.h"
#include "BCGKeyboardManager.h"
#include "BCGUserToolsManager.h"
#include "BCGTearOffManager.h"
#include "BCGSkinManager.h"

#include "RebarState.h"

#include "BCGRegistry.h"
#include "RegPath.h"
#include "bcgcbver.h"	// Library version info.

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGWorkspace* g_pWorkspace = NULL;
BOOL           g_bWorkspaceAutocreated = FALSE;

CBCGWorkspace* GetWorkspace()
{ 
	//---------------------------------------------------------------------
	// You must either:
	// ----------------
	// a) construct a CBCGWorkspace object
	// b) mix a CBCGWorkspace class somewhere in (e.g. your CWinApp object)
	// c) call CBCGWorkspace::UseWorkspaceManager() to automatically
	//    initialize an object for you
	//---------------------------------------------------------------------
	ASSERT (g_pWorkspace != NULL);
	return g_pWorkspace; 
}

class CBCGDockState : CDockState
{
	friend class CBCGWorkspace;

public:
	CBCGDockState ()
	{
#if _MSC_VER >= 1300
		m_rectClip.left -= GetSystemMetrics(SM_CXFRAME);
		m_rectClip.top -= GetSystemMetrics(SM_CYSMCAPTION) + GetSystemMetrics(SM_CYFRAME);
#else
		m_rectDevice.left = 0;
		m_rectDevice.top = 0;
		m_rectDevice.right = GetSystemMetrics (SM_CXVIRTUALSCREEN);
		m_rectDevice.bottom = GetSystemMetrics (SM_CYVIRTUALSCREEN);

		m_rectClip = m_rectDevice;
		m_rectClip.right -= GetSystemMetrics(SM_CXICON);
		m_rectClip.bottom -= GetSystemMetrics(SM_CYICON);
#endif
	}
};

//-----------------------
// clean up if necessary:
//-----------------------
struct _WORKSPACE_TERM
{
	~_WORKSPACE_TERM()
	{
		if (g_pWorkspace != NULL && g_bWorkspaceAutocreated)
		{
			delete g_pWorkspace;
			g_pWorkspace = NULL;
			g_bWorkspaceAutocreated = FALSE;
		}
	}
};
static const _WORKSPACE_TERM workspaceTerm;

//*************************************************************************************

static const CString strRegEntryNameControlBars		= _T("\\ControlBars");
static const CString strWindowPlacementRegSection	= _T("WindowPlacement");
static const CString strRectMainKey					= _T("MainWindowRect");
static const CString strFlagsKey					= _T("Flags");
static const CString strShowCmdKey					= _T("ShowCmd");
static const CString strRegEntryNameSizingBars		= _T("\\SizingBars");
static const CString strRegEntryVersion				= _T("BCGControlBarVersion");
static const CString strVersionMajorKey				= _T("Major");
static const CString strVersionMinorKey				= _T("Minor");

extern CObList	gAllToolbars;
extern CObList	gAllSizingControlBars;

//*************************************************************************************
BOOL CBCGWorkspace::UseWorkspaceManager(LPCTSTR lpszSectionName /*=NULL*/)
{
	if(g_pWorkspace != NULL)
	{
		return FALSE;	// already exists
	}

	g_pWorkspace = new CBCGWorkspace;
	g_bWorkspaceAutocreated = TRUE;	// Cleanup

	if(lpszSectionName != NULL)
	{
		g_pWorkspace->m_strRegSection = lpszSectionName;
	}
	
	return TRUE;
}
//*************************************************************************************
LPCTSTR CBCGWorkspace::SetRegistryBase(LPCTSTR lpszSectionName /*= NULL*/)
{
	m_strRegSection = (lpszSectionName != NULL) ? 
			lpszSectionName : 
			lpszSectionName;

	return m_strRegSection;
}
//*************************************************************************************
CBCGWorkspace::CBCGWorkspace (BOOL bResourceSmartUpdate /*= FALSE*/) :
							m_bResourceSmartUpdate (bResourceSmartUpdate)
{
	// ONLY ONE ALLOWED
	ASSERT(g_pWorkspace == NULL);
	g_pWorkspace = this;

	m_bKeyboardManagerAutocreated = FALSE;
	m_bContextMenuManagerAutocreated = FALSE;
	m_bMouseManagerAutocreated = FALSE;
	m_bUserToolsManagerAutoCreated = FALSE;
	m_bTearOffManagerAutoCreated = FALSE;
	m_bSkinManagerAutocreated = FALSE;
	m_bShellManagerAutocreated = FALSE;

	const CString strRegEntryNameWorkspace = _T("BCGWorkspace");
	m_strRegSection = strRegEntryNameWorkspace;

	m_iSavedVersionMajor = -1;
	m_iSavedVersionMinor = -1;

	m_bForceDockStateLoad = FALSE;
	m_bLoadSaveFrameBarsOnly = FALSE;

	m_bSaveState = TRUE;
	m_bForceImageReset = FALSE;
}
//*************************************************************************************
CBCGWorkspace::~CBCGWorkspace()
{
	// NO OTHER !!
	ASSERT(g_pWorkspace == this);
	g_pWorkspace = NULL;

	// Delete autocreated managers
	if(m_bKeyboardManagerAutocreated && g_pKeyboardManager != NULL)
	{
		delete g_pKeyboardManager;
		g_pKeyboardManager = NULL;
	}

	if (m_bContextMenuManagerAutocreated && g_pContextMenuManager != NULL)
	{
		delete g_pContextMenuManager;
		g_pContextMenuManager = NULL;
	}

	if (m_bMouseManagerAutocreated && g_pMouseManager != NULL)
	{
		delete g_pMouseManager;
		g_pMouseManager = NULL;
	}

	if (m_bUserToolsManagerAutoCreated && g_pUserToolsManager != NULL)
	{
		delete g_pUserToolsManager;
		g_pUserToolsManager = NULL;
	}

	if (m_bTearOffManagerAutoCreated && g_pTearOffMenuManager != NULL)
	{
		delete g_pTearOffMenuManager;
		g_pTearOffMenuManager = NULL;
	}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	if (m_bSkinManagerAutocreated && g_pSkinManager != NULL)
	{
		delete g_pSkinManager;
		g_pSkinManager = NULL;
	}
#endif

	if (m_bShellManagerAutocreated && g_pShellManager != NULL)
	{
		delete g_pShellManager;
		g_pShellManager = NULL;
	}

}
//*************************************************************************************
BOOL CBCGWorkspace::InitMouseManager()
{
	if (g_pMouseManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pMouseManager = new CBCGMouseManager;
	m_bMouseManagerAutocreated = TRUE;
	return TRUE;
}
//*************************************************************************************
BOOL CBCGWorkspace::InitShellManager()
{
	if (g_pShellManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pShellManager = new CBCGShellManager;
	m_bShellManagerAutocreated = TRUE;
	return TRUE;
}
//*************************************************************************************
BOOL CBCGWorkspace::InitContextMenuManager()
{
	if (g_pContextMenuManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pContextMenuManager = new CBCGContextMenuManager;
	m_bContextMenuManagerAutocreated = TRUE;
	
	return TRUE;
}
//*************************************************************************************
BOOL CBCGWorkspace::InitKeyboardManager()
{
	if (g_pKeyboardManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pKeyboardManager = new CBCGKeyboardManager;
	m_bKeyboardManagerAutocreated = TRUE;

	return TRUE;
}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version

BOOL CBCGWorkspace::InitSkinManager(LPCTSTR lpszSkinsDirectory/* = BCG_DEFAULT_SKINS_DIR*/)
{
	if (g_pSkinManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pSkinManager = new CBCGSkinManager (lpszSkinsDirectory);
	m_bSkinManagerAutocreated = TRUE;

	return TRUE;
}

#endif

BOOL CBCGWorkspace::EnableUserTools (const UINT uiCmdToolsDummy,
									 const UINT uiCmdFirst, const UINT uiCmdLast,
									CRuntimeClass* pToolRTC,
									UINT uArgMenuID, UINT uInitDirMenuID)

{
	if (g_pUserToolsManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pUserToolsManager = new 
		CBCGUserToolsManager (	uiCmdToolsDummy, uiCmdFirst, uiCmdLast, pToolRTC,
								uArgMenuID, uInitDirMenuID);
	m_bUserToolsManagerAutoCreated = TRUE;

	return TRUE;
}
//*************************************************************************************
BOOL CBCGWorkspace::EnableTearOffMenus (LPCTSTR lpszRegEntry,
							const UINT uiCmdFirst, const UINT uiCmdLast)
{
	if (g_pTearOffMenuManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pTearOffMenuManager = new CBCGTearOffManager;
	m_bTearOffManagerAutoCreated = TRUE;

	return g_pTearOffMenuManager->Initialize (lpszRegEntry, uiCmdFirst, uiCmdLast);
}
//**************************************************************************************
CBCGMouseManager* CBCGWorkspace::GetMouseManager()
{
	if (g_pMouseManager == NULL)
	{
		InitMouseManager ();
	}

	ASSERT_VALID (g_pMouseManager);
	return g_pMouseManager;
}
//**************************************************************************************
CBCGShellManager* CBCGWorkspace::GetShellManager()
{
	if (g_pShellManager == NULL)
	{
		InitShellManager ();
	}

	ASSERT_VALID (g_pShellManager);
	return g_pShellManager;
}
//*************************************************************************************
CBCGContextMenuManager* CBCGWorkspace::GetContextMenuManager()
{
	if (g_pContextMenuManager == NULL)
	{
		InitContextMenuManager();
	}

	ASSERT_VALID (g_pContextMenuManager);
	return g_pContextMenuManager;
}
//*************************************************************************************
CBCGKeyboardManager* CBCGWorkspace::GetKeyboardManager()
{
	if (g_pKeyboardManager == NULL)
	{
		InitKeyboardManager ();
	}

	ASSERT_VALID (g_pKeyboardManager);
	return g_pKeyboardManager;
}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version

CBCGSkinManager* CBCGWorkspace::GetSkinManager()
{
	if (g_pSkinManager == NULL)
	{
		InitSkinManager ();
	}

	ASSERT_VALID (g_pSkinManager);
	return g_pSkinManager;
}

#endif

CBCGUserToolsManager* CBCGWorkspace::GetUserToolsManager()
{
	return g_pUserToolsManager;
}
//*************************************************************************************
CString	CBCGWorkspace::GetRegSectionPath(LPCTSTR szSectionAdd /*=NULL*/)
{
	CString strSectionPath = ::BCGGetRegPath (m_strRegSection);
	if (szSectionAdd != NULL && _tcslen (szSectionAdd) != 0)
	{
		strSectionPath += szSectionAdd;
		strSectionPath += _T("\\");
	}

	return strSectionPath;
}
//*************************************************************************************
BOOL CBCGWorkspace::LoadState (LPCTSTR lpszSectionName /*=NULL*/, CBCGFrameImpl* pFrameImpl /*= NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreLoadState();

	//------------------------
	// Loaded library version:
	//------------------------
	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (GetRegSectionPath (strRegEntryVersion)))
	{
		reg.Read (strVersionMajorKey, m_iSavedVersionMajor);
		reg.Read (strVersionMinorKey, m_iSavedVersionMinor);
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CBCGToolBar::LoadParameters (strSection);
	CMD_MGR.LoadState (strSection);

	BOOL bResetImages = FALSE;	// Reset images to default 

	if (m_bResourceSmartUpdate)
	{
		CBCGToolbarButton::m_bUpdateImages = FALSE;
	}

	if (pFrameImpl != NULL) 
	{
		//-------------------
		// Load rebars state:
		//-------------------
#ifndef BCG_NO_REBAR
		CBCGRebarState::LoadState (strSection, pFrameImpl->m_pFrame);
#endif

		//-----------------------------------------------------
		// Load all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pToolBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					if (!pToolBar->m_bLocked && 
						!pFrameImpl->IsUserDefinedToolbar(pToolBar)) 
					{
						pToolBar->LoadState (strSection);
						if (pToolBar->IsResourceChanged ())
						{
							bResetImages = TRUE;
						}
					}
				}
			}
		}

#ifndef BCG_NO_SIZINGBAR
		for (POSITION posCb = gAllSizingControlBars.GetHeadPosition (); posCb != NULL;)
		{
			CBCGSizingControlBar* pBar = (CBCGSizingControlBar*) gAllSizingControlBars.GetNext (posCb);
			ASSERT (pBar != NULL);

			if (CWnd::FromHandlePermanent (pBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					pBar->LoadState (m_strRegSection + strRegEntryNameSizingBars);
				}
			}
		}
#endif // BCG_NO_SIZINGBAR

		//----------------------------
		// Load user defined toolbars:
		//----------------------------
		pFrameImpl->LoadUserToolbars ();

		//------------------------
		// Load tear-off toolbars:
		//------------------------
		pFrameImpl->LoadTearOffMenus ();

		CBCGDockState dockState;
		dockState.LoadState(m_strRegSection + strRegEntryNameControlBars);

		if (m_bForceDockStateLoad || pFrameImpl->IsDockStateValid (dockState))
		{
			pFrameImpl->SetDockState (dockState);
		}
	}

	//--------------------------------------
	// Load mouse/keyboard/menu managers:
	//--------------------------------------
	if (g_pMouseManager != NULL)
	{
		g_pMouseManager->LoadState (strSection);
	}

	if (g_pContextMenuManager != NULL)
	{
		g_pContextMenuManager->LoadState(strSection);
	}

	if (g_pKeyboardManager != NULL)
	{
		g_pKeyboardManager->LoadState (strSection,
			pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (g_pUserToolsManager != NULL)
	{
		g_pUserToolsManager->LoadState (strSection);
	}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager != NULL)
	{
		g_pSkinManager->LoadState (strSection);
	}
#endif

	if (m_bResourceSmartUpdate)
	{
		CBCGToolbarButton::m_bUpdateImages = TRUE;
	}

	if (m_bForceImageReset || (m_bResourceSmartUpdate && bResetImages))
	{
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);

				pToolBar->ResetImages ();
			}
		}

		if (pFrameImpl != NULL)
		{
			ASSERT_VALID (pFrameImpl->m_pFrame);
			pFrameImpl->m_pFrame->RecalcLayout ();
		}
	}

	//----------
	// Call Hook
	//----------
	LoadCustomState();

	//----------------------------------------------------------------------
	// To not confuse internal serialization, set version number to current:
	//----------------------------------------------------------------------
	m_iSavedVersionMajor = _BCGCB_VERSION_MAJOR;
	m_iSavedVersionMinor = _BCGCB_VERSION_MINOR;

	return TRUE;
}
//*************************************************************************************
BOOL CBCGWorkspace::LoadState (CBCGMDIFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl); 
}
//*************************************************************************************
BOOL CBCGWorkspace::LoadState (CBCGFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGWorkspace::LoadState (CBCGOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGWorkspace::LoadState (CBCGOleDocIPFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl);
}
//*************************************************************************************
BOOL CBCGWorkspace::CleanState (LPCTSTR lpszSectionName /*=NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	return reg.DeleteKey(strSection);
}
//*************************************************************************************
BOOL CBCGWorkspace::SaveState (LPCTSTR lpszSectionName  /*=NULL*/, CBCGFrameImpl* pFrameImpl /*= NULL*/)
{
	if (!m_bSaveState)
	{
		return FALSE;
	}

	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreSaveState();

	//----------------------
	// Save library version:
	//----------------------
	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (GetRegSectionPath (strRegEntryVersion)))
	{
		reg.Write (strVersionMajorKey, _BCGCB_VERSION_MAJOR);
		reg.Write (strVersionMinorKey, _BCGCB_VERSION_MINOR);
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CBCGToolBar::SaveParameters (strSection);
	CMD_MGR.SaveState (strSection);

	if (pFrameImpl != NULL) 
	{
		CBCGDockState dockState;
		
		pFrameImpl->m_pFrame->GetDockState (dockState);
		dockState.SaveState (m_strRegSection + strRegEntryNameControlBars);

		//-----------------------------------------------------
		// Save all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pToolBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					if (!pToolBar->m_bLocked && 
						!pFrameImpl->IsUserDefinedToolbar (pToolBar))
					{
						pToolBar->SaveState (strSection);
					}
				}
			}
		}

#ifndef BCG_NO_SIZINGBAR
		for (POSITION posCb = gAllSizingControlBars.GetHeadPosition (); posCb != NULL;)
		{
			CBCGSizingControlBar* pBar = (CBCGSizingControlBar*) gAllSizingControlBars.GetNext (posCb);
			ASSERT (pBar != NULL);

			if (CWnd::FromHandlePermanent (pBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					pBar->SaveState (m_strRegSection + strRegEntryNameSizingBars);
				}
			}
		}
#endif //BCG_NO_SIZINGBAR

		//----------------------------
		// Save user defined toolbars:
		//----------------------------
		pFrameImpl->SaveUserToolbars (m_bLoadSaveFrameBarsOnly);

		//------------------------
		// Save tear-off toolbars:
		//------------------------
		pFrameImpl->SaveTearOffMenus (m_bLoadSaveFrameBarsOnly);

		//-------------------
		// Save rebars state:
		//-------------------
#ifndef BCG_NO_REBAR
		CBCGRebarState::SaveState (strSection, pFrameImpl->m_pFrame);
#endif
	}

	//------------------
	// Save user images:
	//------------------
	if (CBCGToolBar::m_pUserImages != NULL)
	{
		ASSERT_VALID (CBCGToolBar::m_pUserImages);
		CBCGToolBar::m_pUserImages->Save ();
	}

	//--------------------------------------
	// Save mouse/keyboard/menu managers:
	//--------------------------------------
	if (g_pMouseManager != NULL)
	{
		g_pMouseManager->SaveState (strSection);
	}

	if (g_pContextMenuManager != NULL)
	{
		g_pContextMenuManager->SaveState (strSection);
	}

	if (g_pKeyboardManager != NULL)
	{
		g_pKeyboardManager->SaveState (strSection,
			pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (g_pUserToolsManager != NULL)
	{
		g_pUserToolsManager->SaveState (strSection);
	}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager != NULL)
	{
		g_pSkinManager->SaveState (strSection);
	}
#endif

	SaveCustomState();
	return TRUE;
}

//*************************************************************************************
// Overidables for customization

void CBCGWorkspace::OnClosingMainFrame (CBCGFrameImpl* pFrame)
{
	// Defaults to automatically saving state.
	SaveState(0, pFrame);
}

//--------------------------------------------------------
// the next one have to be called explicitly in your code:
//--------------------------------------------------------
BOOL CBCGWorkspace::OnViewDoubleClick (CWnd* pWnd, int iViewId)
{
	if (g_pMouseManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (g_pMouseManager);

	UINT uiCmd = g_pMouseManager->GetViewDblClickCommand (iViewId);
	if (uiCmd > 0 && uiCmd != (UINT) -1)
	{
		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->InvokeTool (uiCmd))
		{
			return TRUE;
		}

		CWnd* pTargetWnd = (pWnd == NULL) ? 
			AfxGetMainWnd () : 
			BCGGetTopLevelFrame (pWnd);
		ASSERT_VALID (pTargetWnd);

		pTargetWnd->SendMessage (WM_COMMAND, uiCmd);
		return TRUE;
	}

	MessageBeep ((UINT) -1);
	return FALSE;
}
//***********************************************************************************
BOOL CBCGWorkspace::ShowPopupMenu (UINT uiMenuResId, const CPoint& point, CWnd* pWnd)
{
	if (g_pContextMenuManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (g_pContextMenuManager);
	return g_pContextMenuManager->ShowPopupMenu (uiMenuResId,
				point.x, point.y, pWnd);
}
//***********************************************************************************
BOOL CBCGWorkspace::LoadWindowPlacement (
					CRect& rectNormalPosition, int& nFlags, int& nShowCmd)
{
	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (GetRegSectionPath (strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return	reg.Read (strRectMainKey, rectNormalPosition) &&
			reg.Read (strFlagsKey, nFlags) &&
			reg.Read (strShowCmdKey, nShowCmd);
}
//***********************************************************************************
BOOL CBCGWorkspace::StoreWindowPlacement (
					const CRect& rectNormalPosition, int nFlags, int nShowCmd)
{
	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (GetRegSectionPath (strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return	reg.Write (strRectMainKey, rectNormalPosition) &&
			reg.Write (strFlagsKey, nFlags) &&
			reg.Write (strShowCmdKey, nShowCmd);
}
//*************************************************************************************
//*************************************************************************************
// These functions load and store values from the "Custom" subkey
// To use subkeys of the "Custom" subkey use GetSectionInt() etc.
// instead
int CBCGWorkspace::GetInt(LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	return GetSectionInt(_T(""), lpszEntry, nDefault);
}
//*************************************************************************************
CString	CBCGWorkspace::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	return GetSectionString(_T(""), lpszEntry, lpszDefault);
}
//*************************************************************************************
BOOL CBCGWorkspace::GetBinary(LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	return GetSectionBinary(_T(""), lpszEntry, ppData, pBytes);
}
//*************************************************************************************
BOOL CBCGWorkspace::GetObject(LPCTSTR lpszEntry, CObject& obj)
{
	return GetSectionObject(_T(""), lpszEntry, obj);
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteInt(LPCTSTR lpszEntry, int nValue )
{
	return WriteSectionInt(_T(""), lpszEntry, nValue);
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	return WriteSectionString(_T(""), lpszEntry, lpszValue);
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	return WriteSectionBinary(_T(""), lpszEntry, pData, nBytes);
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteObject(LPCTSTR lpszEntry, CObject& obj)
{
	return WriteSectionObject(_T(""), lpszEntry, obj);
}
//*************************************************************************************
//*************************************************************************************
// These functions load and store values from a given subkey
// of the "Custom" subkey. For simpler access you may use
// GetInt() etc.
int CBCGWorkspace::GetSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	int nRet = nDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection))
	{
		reg.Read (lpszEntry, nRet);
	}
	return nRet;
}
//*************************************************************************************
CString CBCGWorkspace::GetSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(lpszDefault);
	
	CString strRet = lpszDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection))
	{
		reg.Read (lpszEntry, strRet);
	}
	return strRet;
}
//*************************************************************************************
BOOL CBCGWorkspace::GetSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(ppData);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection) 
		&& reg.Read (lpszEntry, ppData, pBytes) ) 
	{
		return TRUE;
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGWorkspace::GetSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT_VALID(&obj);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection) && reg.Read (lpszEntry, obj)) 
	{
		return TRUE;
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nValue )
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, nValue);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(lpszValue);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, lpszValue);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(pData);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, pData, nBytes);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGWorkspace::WriteSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT_VALID(&obj);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, obj);
	}

	return FALSE;
}
//**********************************************************************************
void CBCGWorkspace::OnAppContextHelp (CWnd* pWndControl, const DWORD dwHelpIDArray [])
{
	::WinHelp (pWndControl->GetSafeHwnd (), 
				AfxGetApp()->m_pszHelpFilePath, 
				HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) dwHelpIDArray);
}
//*************************************************************************************
BOOL CBCGWorkspace::SaveState (CBCGMDIFrameWnd* pFrame, LPCTSTR
							   lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl); 
}
//*************************************************************************************
BOOL CBCGWorkspace::SaveState (CBCGFrameWnd* pFrame, LPCTSTR lpszSectionName
							   /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGWorkspace::SaveState (CBCGOleIPFrameWnd* pFrame, LPCTSTR
							   lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGWorkspace::SaveState (CBCGOleDocIPFrameWnd* pFrame, LPCTSTR
							   lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGWorkspace::IsStateExists(LPCTSTR lpszSectionName /*=NULL*/)
{
	 if (lpszSectionName != NULL)
	 {
		m_strRegSection = lpszSectionName;
	 }

	 CString strSection = GetRegSectionPath ();

	//------------------------
	// Loaded library version:
	//------------------------
	 CBCGRegistrySP regSP;
	 CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	 return reg.Open (GetRegSectionPath (strRegEntryVersion));
}
//***********************************************************************************
int CBCGWorkspace::GetDataVersion () const
{
	if (m_iSavedVersionMajor == -1 || m_iSavedVersionMinor == -1)
	{
		return 0xFFFFFFFF;
	}

	int nVersionMinor = m_iSavedVersionMinor / 10;
	int nVersionDigit = m_iSavedVersionMinor % 10;
	
	nVersionMinor *= 0x100;
	nVersionDigit *= 0x10;

	if (nVersionMinor < 10)
	{
		nVersionDigit *=0x10;
	}

	return m_iSavedVersionMajor * 0x10000 + nVersionMinor + nVersionDigit;
}

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_	// Skins manager can not be used in the static version

void CBCGWorkspace::OnSelectSkin ()
{
	CFrameWnd* pMainWnd = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	if (pMainWnd != NULL)
	{
		pMainWnd->RecalcLayout ();
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			pToolBar->AdjustLayout ();
		}
	}

	CBCGVisualManager::GetInstance ()->RedrawAll ();
}

#endif
