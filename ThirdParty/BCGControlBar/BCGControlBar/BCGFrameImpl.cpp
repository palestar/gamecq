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

// BCGFrameImpl.cpp: implementation of the CBCGFrameImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "multimon.h"

#include "BCGFrameImpl.h"
#include "BCGToolBar.h"
#include "BCGDialogBar.h"
#include "BCGSizingControlBar.h"
#include "BCGMenuBar.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "BCGPopupMenu.h"
#include "BCGToolbarMenuButton.h"
#include "BCGWorkspace.h"
#include "RegPath.h"
#include "BCGRegistry.h"
#include "BCGTearOffManager.h"
#include "BCGVisualManager.h"
#include "BCGKeyboardManager.h"
#include "BCGLocalRes.h"
#include "CustomizeButton.h"
#include "BCGCustomizeMenuButton.h"
#include "CBCGToolbarCustomize.h"
#include "BCGVisualManager.h"
#include "BCGVisualManager.h"
#include "BCGDropDown.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CObList	gAllToolbars;
extern CBCGWorkspace* g_pWorkspace;

static const CString strTearOffBarsRegEntry = _T("ControlBars-TearOff");

BOOL CBCGFrameImpl::m_bControlBarExtraPixel = TRUE;
BOOL CBCGFrameImpl::m_bToolbarWrapMode = TRUE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGFrameImpl::CBCGFrameImpl(CFrameWnd* pFrame) :
	m_pFrame (pFrame),
	m_uiUserToolbarFirst ((UINT)-1),
	m_uiUserToolbarLast ((UINT)-1),
	m_pMenuBar (NULL),
	m_hDefaultMenu (NULL),
	m_nIDDefaultResource (0)
{
	ASSERT_VALID (m_pFrame);
}
//**************************************************************************************
CBCGFrameImpl::~CBCGFrameImpl()
{
	//-----------------------------
	// Clear user-defined toolbars:
	//-----------------------------
	while (!m_listUserDefinedToolbars.IsEmpty ())
	{
		delete m_listUserDefinedToolbars.RemoveHead ();
	}

	//-------------------------
	// Clear tear-off toolbars:
	//-------------------------
	while (!m_listTearOffToolbars.IsEmpty ())
	{
		delete m_listTearOffToolbars.RemoveHead ();
	}
}
//**************************************************************************************
void CBCGFrameImpl::OnCloseFrame()
{
	ASSERT_VALID (m_pFrame);

	//----------------------------------------
	// First, remove all hidden tear-off bars:
	//----------------------------------------
	for (POSITION pos = m_listTearOffToolbars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CControlBar* pBar = (CControlBar*) m_listTearOffToolbars.GetNext (pos);
		ASSERT_VALID (pBar);

		if (!pBar->IsWindowVisible ())
		{
			pBar->DestroyWindow ();
			delete pBar;

			m_listTearOffToolbars.RemoveAt (posSave);
		}
	}

	//----------------------------------------------------------------------
	// Automatically load state and frame position if CBCGWorkspace is used:
	//----------------------------------------------------------------------
	if (g_pWorkspace != NULL)
	{
		g_pWorkspace->OnClosingMainFrame (this);

		//---------------------------
		// Store the Windowplacement:
		//---------------------------
		if (::IsWindow (m_pFrame->GetSafeHwnd ()))
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof (WINDOWPLACEMENT);

			if (m_pFrame->GetWindowPlacement (&wp))
			{
				//---------------------------
				// Make sure we don't pop up 
				// minimized the next time
				//---------------------------
				if (wp.showCmd != SW_SHOWMAXIMIZED)
				{
					wp.showCmd = SW_SHOWNORMAL;
				}

				RECT rectDesktop;
				SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&rectDesktop,0);
				OffsetRect(&wp.rcNormalPosition, rectDesktop.left, rectDesktop.top);
      
    			g_pWorkspace->StoreWindowPlacement (
					wp.rcNormalPosition, wp.flags, wp.showCmd);
			}
		}
	}
}
//**************************************************************************************
void CBCGFrameImpl::RestorePosition(CREATESTRUCT& cs)
{
	if (g_pWorkspace != NULL &&
		cs.hInstance != NULL)
	{
		CRect rectNormal (CPoint (cs.x, cs.y), CSize (cs.cx, cs.cy));
		int nFlags = 0;
		int nShowCmd = SW_SHOWNORMAL;

		if (!g_pWorkspace->LoadWindowPlacement (rectNormal, nFlags, nShowCmd))
		{
			return;
		}

		if (nShowCmd != SW_MAXIMIZE)
		{
			nShowCmd = SW_SHOWNORMAL;
		}

		switch (AfxGetApp()->m_nCmdShow)
		{
		case SW_MAXIMIZE:
		case SW_MINIMIZE:
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
			break;	// don't change!

		default:
			AfxGetApp()->m_nCmdShow = nShowCmd;
		}

		CRect rectDesktop;
		CRect rectInter;

		MONITORINFO mi;
		mi.cbSize = sizeof (MONITORINFO);
		if (GetMonitorInfo (MonitorFromPoint (rectNormal.TopLeft (), 
			MONITOR_DEFAULTTONEAREST), &mi))
		{
			rectDesktop = mi.rcWork;
		}
		else
		{
			::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectDesktop, 0);
		}

		if (rectInter.IntersectRect (&rectDesktop, &rectNormal))
		{
			cs.x = rectInter.left;
			cs.y = rectInter.top;
			cs.cx = rectNormal.Width ();
			cs.cy = rectNormal.Height ();
		}
	}
}
//**************************************************************************************
void CBCGFrameImpl::OnLoadFrame()
{
	//---------------------------------------------------
	// Automatically load state if CBCGWorkspace is used:
	//---------------------------------------------------
	if (g_pWorkspace != NULL)
	{
		g_pWorkspace->LoadState (0, this);
	}
}
//**************************************************************************************
void CBCGFrameImpl::LoadUserToolbars ()
{
	ASSERT_VALID (m_pFrame);

	if (m_uiUserToolbarFirst == (UINT) -1 ||
		m_uiUserToolbarLast == (UINT) -1)
	{
		return;
	}

	for (UINT uiNewToolbarID = m_uiUserToolbarFirst;
		uiNewToolbarID <= m_uiUserToolbarLast;
		uiNewToolbarID ++)
	{
		CBCGToolBar* pNewToolbar = new CBCGToolBar;
		if (!pNewToolbar->Create (m_pFrame, 
			dwDefaultToolbarStyle,
			uiNewToolbarID))
		{
			TRACE0 ("Failed to create a new toolbar!\n");
			delete pNewToolbar;
			continue;
		}

		if (!pNewToolbar->LoadState (m_strControlBarRegEntry))
		{
			pNewToolbar->DestroyWindow ();
			delete pNewToolbar;
		}
		else
		{
			pNewToolbar->SetBarStyle (pNewToolbar->GetBarStyle () |
				CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
			pNewToolbar->EnableDocking (CBRS_ALIGN_ANY);

			m_pFrame->DockControlBar (pNewToolbar);
			m_listUserDefinedToolbars.AddTail (pNewToolbar);
		}
	}
}
//**********************************************************************************************
void CBCGFrameImpl::SaveUserToolbars (BOOL bFrameBarsOnly)
{
	for (POSITION pos = m_listUserDefinedToolbars.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolBar* pUserToolBar = 
			(CBCGToolBar*) m_listUserDefinedToolbars.GetNext (pos);
		ASSERT_VALID(pUserToolBar);

		if (!bFrameBarsOnly || pUserToolBar->GetTopLevelFrame () == m_pFrame)
		{
			pUserToolBar->SaveState (m_strControlBarRegEntry);
		}
	}
}
//**********************************************************************************************
CBCGToolBar* CBCGFrameImpl::GetUserBarByIndex (int iIndex) const
{
	POSITION pos = m_listUserDefinedToolbars.FindIndex (iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	CBCGToolBar* pUserToolBar = 
		(CBCGToolBar*) m_listUserDefinedToolbars.GetAt (pos);
	ASSERT_VALID (pUserToolBar);

	return pUserToolBar;
}
//**********************************************************************************************
BOOL CBCGFrameImpl::IsUserDefinedToolbar (const CBCGToolBar* pToolBar) const
{
	ASSERT_VALID (pToolBar);

	UINT uiCtrlId = pToolBar->GetDlgCtrlID ();
	return	uiCtrlId >= m_uiUserToolbarFirst &&
			uiCtrlId <= m_uiUserToolbarLast;
}
//*******************************************************************************************
BOOL CBCGFrameImpl::IsDockStateValid (const CDockState& state)
{
	ASSERT_VALID (m_pFrame);

	//----------------------------------------------------------------
	// This function helps to avoid GPF during CFrameWnd::LoadBarState
	// execution: when one of the previously saved toolbars is not
	// created, LoadBarState fails.
	//----------------------------------------------------------------

	for (int i = 0; i < state.m_arrBarInfo.GetSize (); i ++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*) state.m_arrBarInfo [i];
		ASSERT (pInfo != NULL);

		if (!pInfo->m_bFloating)
		{
			//---------------------------------------
			// Find the control bar with the same ID:
			//---------------------------------------
			CControlBar* pCtrlBar = m_pFrame->GetControlBar (pInfo->m_nBarID);
			if (pCtrlBar == NULL)
			{
				TRACE (_T("CBCGFrameImpl::IsDockStateValid ControlBar %x is not valid!\n"), pInfo->m_nBarID);
				return FALSE;
			}
			else
			{
				//-----------------------------------------------------
				// Just check if the bar info says that this bar should
				// be dockable and if the bar is really capable:
				//-----------------------------------------------------
				if (pInfo->m_bDocking && pCtrlBar->m_pDockBar == NULL)
				{
					//-------------------------------------------------
					// This bar is in fact not dockable, clear the flag
					//-------------------------------------------------
					TRACE (_T("CBCGFrameImpl::IsDockStateValid CControlBarInfo for bar %x is reset to be non dockable!\n"), pInfo->m_nBarID);
					pInfo->m_bDocking = FALSE;
				}
			}
		}
 	}

	return TRUE;
}
//**********************************************************************************
void CBCGFrameImpl::InitUserToobars (	LPCTSTR lpszRegEntry,
										UINT uiUserToolbarFirst, 
										UINT uiUserToolbarLast)
{
	ASSERT (uiUserToolbarLast >= uiUserToolbarFirst);

	if (uiUserToolbarFirst == (UINT) -1 ||
		uiUserToolbarLast == (UINT) -1)
	{
		ASSERT (FALSE);
		return;
	}

	m_uiUserToolbarFirst = uiUserToolbarFirst;
	m_uiUserToolbarLast = uiUserToolbarLast;

	// ET: get Path automatically from workspace if needed
	m_strControlBarRegEntry = (lpszRegEntry == NULL) ? 
		( g_pWorkspace ? g_pWorkspace->GetRegSectionPath() : _T("") )
		: lpszRegEntry;
}
//**************************************************************************************
UINT CBCGFrameImpl::GetFreeCtrlBarID (UINT uiFirstID, UINT uiLastID, const CObList& lstCtrlBars)
{
	if (uiFirstID == (UINT)-1 || uiLastID == (UINT)-1)
	{
		return 0;
	}

	int iMaxToolbars = uiLastID - uiFirstID + 1;
	if (lstCtrlBars.GetCount () == iMaxToolbars)
	{
		return 0;
	}

	for (UINT uiNewToolbarID = uiFirstID; uiNewToolbarID <= uiLastID;
		uiNewToolbarID ++)
	{
		BOOL bUsed = FALSE;

		for (POSITION pos = lstCtrlBars.GetHeadPosition (); 
			!bUsed && pos != NULL;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) lstCtrlBars.GetNext (pos);
			ASSERT_VALID (pToolBar);

			bUsed = (pToolBar->GetDlgCtrlID () == (int) uiNewToolbarID);
		}

		if (!bUsed)
		{
			return uiNewToolbarID;
		}
	}

	return 0;
}
//**************************************************************************************
const CBCGToolBar* CBCGFrameImpl::CreateNewToolBar (LPCTSTR lpszName)
{
	ASSERT_VALID (m_pFrame);
	ASSERT (lpszName != NULL);

	UINT uiNewToolbarID = 
		GetFreeCtrlBarID (m_uiUserToolbarFirst, m_uiUserToolbarLast, m_listUserDefinedToolbars);

	if (uiNewToolbarID == 0)
	{
		CBCGLocalResource locaRes;

		CString strError;
		strError.Format (IDS_BCGBARRES_TOO_MANY_TOOLBARS_FMT, 
			m_uiUserToolbarLast - m_uiUserToolbarFirst + 1);

		AfxMessageBox (strError, MB_OK | MB_ICONASTERISK);
		return NULL;
	}

	CBCGToolBar* pNewToolbar = new CBCGToolBar;
	if (!pNewToolbar->Create (m_pFrame,
		dwDefaultToolbarStyle,
		uiNewToolbarID))
	{
		TRACE0 ("Failed to create a new toolbar!\n");
		delete pNewToolbar;
		return NULL;
	}

	pNewToolbar->SetWindowText (lpszName);

	pNewToolbar->SetBarStyle (pNewToolbar->GetBarStyle () |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	pNewToolbar->EnableDocking (CBRS_ALIGN_ANY);
	
	m_pFrame->FloatControlBar (pNewToolbar,
		CPoint (::GetSystemMetrics (SM_CXFULLSCREEN) / 2, 
				::GetSystemMetrics (SM_CYFULLSCREEN) / 2));
	m_pFrame->RecalcLayout ();

	m_listUserDefinedToolbars.AddTail (pNewToolbar);
	return pNewToolbar;
}
//**************************************************************************************
void CBCGFrameImpl::AddTearOffToolbar (CControlBar* pToolBar)
{
	ASSERT_VALID (pToolBar);
	m_listTearOffToolbars.AddTail (pToolBar);
}
//**************************************************************************************
void CBCGFrameImpl::RemoveTearOffToolbar (CControlBar* pToolBar)
{
	ASSERT_VALID (pToolBar);

	POSITION pos = m_listTearOffToolbars.Find (pToolBar);
	if (pos != NULL)
	{
		m_listTearOffToolbars.RemoveAt (pos);
	}
}
//**************************************************************************************
void CBCGFrameImpl::LoadTearOffMenus ()
{
	ASSERT_VALID (m_pFrame);

	//------------------------------
	// Remove current tear-off bars:
	//------------------------------
	for (POSITION pos = m_listTearOffToolbars.GetHeadPosition (); pos != NULL;)
	{
		CControlBar* pBar = (CControlBar*) m_listTearOffToolbars.GetNext (pos);
		ASSERT_VALID (pBar);
		
		pBar->DestroyWindow ();
		delete pBar;
	}
 
	m_listTearOffToolbars.RemoveAll ();

	CString strProfileName = g_pWorkspace != NULL ?
		g_pWorkspace->GetRegSectionPath() : _T("");

	strProfileName += strTearOffBarsRegEntry;

	for (int iIndex = 0;; iIndex++)
	{
		CString strKey;
		strKey.Format (_T("%s-%d"), strProfileName, iIndex);

		int iId = 0;
		CBCGToolBar* pToolBar = NULL;
		CString strName;

		CBCGRegistrySP regSP;
		CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

		if (!reg.Open (strKey) ||
			!reg.Read (_T("ID"), iId) ||
			!reg.Read (_T("Name"), strName) ||
			!reg.Read (_T("State"), (CObject*&) pToolBar))
		{
			break;
		}

		ASSERT_VALID (pToolBar);

		if (!pToolBar->Create (m_pFrame,
			dwDefaultToolbarStyle,
			(UINT) iId))
		{
			TRACE0 ("Failed to create a new toolbar!\n");
			delete pToolBar;
			break;
		}

		pToolBar->SetWindowText (strName);

		pToolBar->SetBarStyle (pToolBar->GetBarStyle () |
			CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
		pToolBar->EnableDocking (CBRS_ALIGN_ANY);

		m_pFrame->DockControlBar (pToolBar);
		m_listTearOffToolbars.AddTail (pToolBar);
	}
}
//**************************************************************************************
void CBCGFrameImpl::SaveTearOffMenus (BOOL bFrameBarsOnly)
{
	CString strProfileName = g_pWorkspace != NULL ?
		g_pWorkspace->GetRegSectionPath() : _T("");
	strProfileName += strTearOffBarsRegEntry;

	//------------------------------------------------
	// First, clear old tear-off toolbars in registry:
	//------------------------------------------------
	int iIndex = 0;

	for (;; iIndex++)
	{
		CString strKey;
		strKey.Format (_T("%s-%d"), strProfileName, iIndex);

		CBCGRegistrySP regSP;
		CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

		if (!reg.DeleteKey (strKey))
		{
			break;
		}
	}

	iIndex = 0;
	for (POSITION pos = m_listTearOffToolbars.GetHeadPosition (); pos != NULL; iIndex ++)
	{
		CControlBar* pBar = (CControlBar*) m_listTearOffToolbars.GetNext (pos);
		ASSERT_VALID (pBar);

		if (!bFrameBarsOnly || pBar->GetTopLevelFrame () == m_pFrame)
		{
			CControlBarInfo info;
			pBar->GetBarInfo (&info);

			CString strName;
			pBar->GetWindowText (strName);

			CString strKey;
			strKey.Format (_T("%s-%d"), strProfileName, iIndex);

			CBCGRegistrySP regSP;
			CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

			reg.CreateKey (strKey);

			reg.Write (_T("ID"), (int) info.m_nBarID);
			reg.Write (_T("Name"), strName);
			reg.Write (_T("State"), pBar);
		}
	}
}
//**************************************************************************************
BOOL CBCGFrameImpl::DeleteToolBar (CBCGToolBar* pToolBar)
{
	ASSERT_VALID (m_pFrame);
	ASSERT_VALID (pToolBar);

	POSITION pos = m_listUserDefinedToolbars.Find (pToolBar);
	if (pos == NULL)
	{
		return FALSE;
	}

	m_listUserDefinedToolbars.RemoveAt (pos);
	pToolBar->RemoveStateFromRegistry (m_strControlBarRegEntry);

	pToolBar->DestroyWindow ();
	delete pToolBar;

	m_pFrame->RecalcLayout ();
	return TRUE;
}
//*******************************************************************************************
void CBCGFrameImpl::SetMenuBar (CBCGMenuBar* pMenuBar)
{
	ASSERT_VALID (m_pFrame);
	ASSERT_VALID (pMenuBar);
	ASSERT (m_pMenuBar == NULL);	// Method should be called once!

	m_pMenuBar = pMenuBar;

	m_hDefaultMenu=*m_pFrame->GetMenu();

	// ET: Better support for dynamic menu
	m_pMenuBar->OnDefaultMenuLoaded (m_hDefaultMenu);
	m_pMenuBar->CreateFromMenu (m_hDefaultMenu, TRUE /* Default menu */);

	m_pFrame->SetMenu (NULL);
	
	m_pMenuBar->SetDefaultMenuResId (m_nIDDefaultResource);
}
//*******************************************************************************************
BOOL CBCGFrameImpl::ProcessKeyboard (int nKey, BOOL* pbProcessAccel)
{
	ASSERT_VALID (m_pFrame);

	if (pbProcessAccel != NULL)
	{
		*pbProcessAccel = TRUE;
	}

	//--------------------------------------------------------
	// If popup menu is active, pass keyboard control to menu:
	//--------------------------------------------------------
	if (CBCGPopupMenu::m_pActivePopupMenu != NULL &&
		::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_KEYDOWN, nKey);
		return TRUE;
	}

	//------------------------------------------
	// If appication is minimized, don't handle
	// any keyboard accelerators:
	//------------------------------------------
	if (m_pFrame->IsIconic ())
	{
		return TRUE;
	}

	//----------------------------------------------------------
	// Don't handle keybaord accererators in customization mode:
	//----------------------------------------------------------
	if (CBCGToolBar::IsCustomizeMode ())
	{
		return FALSE;
	}

	//-----------------------------------------------------
	// Is any toolbar control (such as combobox) has focus?
	//-----------------------------------------------------
	BOOL bIsToolbarCtrlFocus = FALSE;
	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); 
		!bIsToolbarCtrlFocus && posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			for (int i = 0; i < pToolBar->GetCount (); i++)
			{
				CBCGToolbarButton* pButton = pToolBar->GetButton (i);
				ASSERT_VALID (pButton);

				if (pButton->HasFocus ())
				{
					bIsToolbarCtrlFocus = TRUE;
					break;
				}
			}
		}
	}

	//-------------------------------------
	// Check for the keyboard accelerators:
	//-------------------------------------
	BYTE fVirt = 0;

	if (::GetAsyncKeyState (VK_CONTROL) & 0x8000)
	{
		fVirt |= FCONTROL;
	}

	if (::GetAsyncKeyState (VK_MENU) & 0x8000)
	{
		fVirt |= FALT;
	}

	if (::GetAsyncKeyState (VK_SHIFT) & 0x8000)
	{
		fVirt |= FSHIFT;
	}

	if (!bIsToolbarCtrlFocus)
	{
		if (CBCGKeyboardManager::IsKeyHandled ((WORD) nKey, (BYTE)(fVirt | FVIRTKEY), 
												m_pFrame, TRUE) ||
			CBCGKeyboardManager::IsKeyHandled ((WORD) nKey, (BYTE)(fVirt | FVIRTKEY), 
												m_pFrame->GetActiveFrame (), FALSE))
		{
			return FALSE;
		}
	}

	if (fVirt == FALT)
	{
		//--------------------------------------------
		// Handle menu accelerators (such as "Alt-F"):
		//--------------------------------------------
		if (OnMenuChar (nKey))
		{
			return TRUE;
		}
	}

	if (bIsToolbarCtrlFocus && pbProcessAccel != NULL)
	{
		//---------------------------------------------
		// Don't process default keyboard accelerators:
		//---------------------------------------------
		*pbProcessAccel = FALSE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGFrameImpl::ProcessMouseClick (UINT uiMsg, POINT pt, HWND hwnd)
{
	ASSERT_VALID (m_pFrame);

	//------------------------------------------------
	// Maybe user start drag the button with control?
	//------------------------------------------------
	if (uiMsg == WM_LBUTTONDOWN &&
		(CBCGToolBar::IsCustomizeMode () ||
		(::GetAsyncKeyState (VK_MENU) & 0x8000)))	// ALT is pressed
	{
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				CPoint ptToolBar = pt;
				pToolBar->ScreenToClient (&ptToolBar);

				int iHit = pToolBar->HitTest (ptToolBar);
				if (iHit >= 0)
				{
					CBCGToolbarButton* pButton = pToolBar->GetButton (iHit);
					ASSERT_VALID (pButton);

					if (pButton->GetHwnd () != NULL &&
						pButton->GetHwnd () == hwnd &&
						pButton->Rect ().PtInRect (ptToolBar))
					{
						pToolBar->SendMessage (WM_LBUTTONDOWN, 
							0, MAKELPARAM (ptToolBar.x, ptToolBar.y));
						return TRUE;
					}

					break;
				}
			}
		}
	}

	if (!CBCGToolBar::IsCustomizeMode () &&
		CBCGPopupMenu::m_pActivePopupMenu != NULL &&
		::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		CBCGPopupMenu::MENUAREA_TYPE clickArea = CBCGPopupMenu::m_pActivePopupMenu->CheckArea (pt);

		if (clickArea == CBCGPopupMenu::OUTSIDE)
		{
			// Click outside of menu

			//--------------------------------------------
			// Maybe secondary click on the parent button?
			//--------------------------------------------
			CBCGToolbarMenuButton* pParentButton = 
				CBCGPopupMenu::m_pActivePopupMenu->GetParentButton ();
			if (pParentButton != NULL)
			{
				CWnd* pWndParent = pParentButton->GetParentWnd ();
				if (pWndParent != NULL)
				{
					CBCGPopupMenuBar* pWndParentPopupMenuBar = 
						DYNAMIC_DOWNCAST (CBCGPopupMenuBar, pWndParent);

					CPoint ptClient = pt;
					pWndParent->ScreenToClient (&ptClient);

					if (pParentButton->Rect ().PtInRect (ptClient))
					{
						//-------------------------------------------------------
						// If user clicks second time on the parent button,
						// we should close an active menu on the toolbar/menubar
						// and leave it on the popup menu:
						//-------------------------------------------------------
						if (pWndParentPopupMenuBar == NULL &&
							!CBCGPopupMenu::m_pActivePopupMenu->InCommand ())
						{
							//----------------------------------------
							// Toolbar/menu bar: close an active menu!
							//----------------------------------------
							CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
						}

						return TRUE;
					}

					if (pWndParentPopupMenuBar != NULL)
					{
						pWndParentPopupMenuBar->CloseDelayedSubMenu ();
						
						CBCGPopupMenu* pWndParentPopupMenu = 
							DYNAMIC_DOWNCAST (CBCGPopupMenu, 
							pWndParentPopupMenuBar->GetParent ());

						if (pWndParentPopupMenu != NULL)
						{
							CBCGPopupMenu::MENUAREA_TYPE clickAreaParent = 
								pWndParentPopupMenu->CheckArea (pt);

							switch (clickAreaParent)
							{
							case CBCGPopupMenu::MENU:
							case CBCGPopupMenu::TEAROFF_CAPTION:
							case CBCGPopupMenu::LOGO:
								return FALSE;

							case CBCGPopupMenu::SHADOW_RIGHT:
							case CBCGPopupMenu::SHADOW_BOTTOM:
								pWndParentPopupMenu->SendMessage (WM_CLOSE);
								m_pFrame->SetFocus ();

								return TRUE;
							}
						}
					}
				}
			}

			if (!CBCGPopupMenu::m_pActivePopupMenu->InCommand ())
			{
				BOOL bMenuTrackMode = CBCGPopupMenu::m_pActivePopupMenu->m_bTrackMode;

				CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);

				CWnd* pWndFocus = CWnd::GetFocus ();
				if (pWndFocus != NULL && pWndFocus->IsKindOf (RUNTIME_CLASS (CBCGToolBar)))
				{
					m_pFrame->SetFocus ();
				}

				if (bMenuTrackMode ||
					clickArea != CBCGPopupMenu::OUTSIDE)	// Click on shadow
				{
					return TRUE;
				}
			}
		}
		else if (clickArea == CBCGPopupMenu::SHADOW_RIGHT ||
				clickArea == CBCGPopupMenu::SHADOW_BOTTOM)
		{
			CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
			m_pFrame->SetFocus ();

			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGFrameImpl::ProcessMouseMove (POINT pt)
{
	if (!CBCGToolBar::IsCustomizeMode () &&
		CBCGPopupMenu::m_pActivePopupMenu != NULL)
	{
		CRect rectMenu;
		CBCGPopupMenu::m_pActivePopupMenu->GetWindowRect (rectMenu);

		if (rectMenu.PtInRect (pt) ||
			CBCGPopupMenu::m_pActivePopupMenu->GetMenuBar ()->FindDestBar (pt) != NULL)
		{
			return FALSE;	// Default processing
		}

		return TRUE;		// Active menu "capturing"
	}

	return FALSE;	// Default processing
}
//*******************************************************************************************
BOOL CBCGFrameImpl::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup, CFrameWnd* /*pWndFrame*/)
{
	CBCGPopupMenu::m_pActivePopupMenu = pMenuPopup;

	if (pMenuPopup != NULL && IsCustomizePane(pMenuPopup))
    {
		ShowQuickCustomizePane (pMenuPopup);
	}

	if (pMenuPopup != NULL && !CBCGToolBar::IsCustomizeMode ())
	{
		ASSERT_VALID (pMenuPopup);

		CControlBar* pTopLevelBar = NULL;

		for (CBCGPopupMenu* pMenu = pMenuPopup; pMenu != NULL;
			pMenu = pMenu->GetParentPopupMenu ())
		{
			CBCGToolbarMenuButton* pParentButton = pMenu->GetParentButton ();
			if (pParentButton == NULL)
			{
				break;
			}
		
			pTopLevelBar = 
				DYNAMIC_DOWNCAST (CControlBar, pParentButton->GetParentWnd ());
		}

		if (pTopLevelBar != NULL && 
			!pTopLevelBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
		{
			ASSERT_VALID (pTopLevelBar);

			if (!pTopLevelBar->IsFloating () &&
				::GetFocus () != pTopLevelBar->GetSafeHwnd () &&
				CBCGPopupMenu::GetForceMenuFocus ())
			{
				pTopLevelBar->SetFocus ();
			}
		}
	}

	return TRUE;
}
//*************************************************************************************
void CBCGFrameImpl::DockControlBarLeftOf(CControlBar* pBar, CControlBar* pLeftOf)
{
	ASSERT_VALID (m_pFrame);
	ASSERT_VALID (pBar);
	ASSERT_VALID (pLeftOf);

	CRect rect;
	DWORD dw;
	UINT n;
	
	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	m_pFrame->RecalcLayout(TRUE);
	
	pLeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,1);
	dw=pLeftOf->GetBarStyle();

	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
	
	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. By calculating a rectangle, we
	// are simulating a Toolbar being dragged to that location and docked.
	m_pFrame->DockControlBar (pBar,n,&rect);
}
//****************************************************************************************
void CBCGFrameImpl::SetupToolbarMenu (CMenu& menu, 
									  const UINT uiViewUserToolbarCmdFirst,
									  const UINT uiViewUserToolbarCmdLast)
{
	//---------------------------------------------------------------
	// Replace toolbar dummy items to the user-defined toolbar names:
	//---------------------------------------------------------------
	for (int i = 0; i < (int) menu.GetMenuItemCount ();)
	{
		UINT uiCmd = menu.GetMenuItemID (i);

		if (uiCmd >= uiViewUserToolbarCmdFirst && 
			uiCmd <= uiViewUserToolbarCmdLast)
		{
			//-------------------------------------------------------------------
			// "User toolbar" item. First check that toolbar number 'x' is exist:
			//-------------------------------------------------------------------
			CBCGToolBar* pToolBar = GetUserBarByIndex (uiCmd - uiViewUserToolbarCmdFirst);
			if (pToolBar != NULL)
			{
				ASSERT_VALID (pToolBar);

				//-----------------------------------------------------------
				// Modify the current menu item text to the toolbar title and
				// move next:
				//-----------------------------------------------------------
				CString strToolbarName;
				pToolBar->GetWindowText (strToolbarName);

				menu.ModifyMenu (i ++, MF_BYPOSITION | MF_STRING, uiCmd, strToolbarName);
			}
			else
			{
				menu.DeleteMenu (i, MF_BYPOSITION);
			}
		}
		else	// Not "user toolbar" item, move next
		{
			i ++;
		}
	}
}


// Copyright (C) 1997,'98 by Joerg Koenig
// All rights reserved
//

/////////////////////////////////////////////////////////////////////////////
// helpers for docking 
/////////////////////////////////////////////////////////////////////////////


// We need our own version of a dock bar, because the original
// MFC implementation overlapps toolbars. CBCGToolBar don't want
// such a overlapping, because this makes it impossible to draw
// a real 3d border ...
class CBCGToolDockBar : public CDockBar {
	DECLARE_DYNAMIC(CBCGToolDockBar)

	public:
		// this is the one and only method of interest
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void DoPaint(CDC* pDC);
	
	//{{AFX_MSG(CBCGToolDockBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CBCGToolDockBar, CDockBar);

BEGIN_MESSAGE_MAP(CBCGToolDockBar, CDockBar)
	//{{AFX_MSG_MAP(CBCGToolDockBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGToolDockBar::DoPaint(CDC* pDC)
{
	CRect rectClient;
	GetClientRect (rectClient);

	CBCGVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
		rectClient, rectClient);
}
//****************************************************************************************
CSize CBCGToolDockBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	CSize sizeFixed = CControlBar::CalcFixedLayout(bStretch, bHorz);

	// get max size
	CSize sizeMax;
	if (!m_rectLayout.IsRectEmpty())
		sizeMax = m_rectLayout.Size();
	else
	{
		CRect rectFrame;
		CFrameWnd* pFrame = GetParentFrame();
		pFrame->GetClientRect(&rectFrame);
		sizeMax = rectFrame.Size();
	}

	// prepare for layout
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = m_bLayoutQuery ?
		NULL : ::BeginDeferWindowPos ((int) m_arrBars.GetSize());
	int cxBorder = afxData.cxBorder2; 
	int cyBorder = afxData.cyBorder2;

	CPoint pt(-cxBorder, -cyBorder);
	int nWidth = 0;

	BOOL bWrapped = FALSE;
	BOOL bFirstBar = TRUE;

	// layout all the control bars
	for (int nPos = 0; nPos < m_arrBars.GetSize(); nPos++)
	{
		CControlBar* pBar = GetDockedControlBar(nPos);
		void* pVoid = m_arrBars[nPos];

		if (pBar != NULL)
		{
			if (pBar->IsKindOf (RUNTIME_CLASS(CBCGToolBar))
#ifndef BCG_NO_SIZINGBAR
				|| pBar->IsKindOf (RUNTIME_CLASS(CBCGSizingControlBar))
				|| pBar->IsKindOf (RUNTIME_CLASS(CBCGDialogBar))
#endif // BCG_NO_SIZINGBAR
				)
			{
				cxBorder = cyBorder = 0;
			}
			else
			{
				cxBorder = afxData.cxBorder2;
				cyBorder = afxData.cyBorder2;
			}

			if (pBar->IsVisible())
			{
				// get ideal rect for bar
				DWORD dwMode = 0;
				if ((pBar->m_dwStyle & CBRS_SIZE_DYNAMIC) &&
					(pBar->m_dwStyle & CBRS_FLOATING))
					dwMode |= LM_HORZ | LM_MRUWIDTH;
				else if (pBar->m_dwStyle & CBRS_ORIENT_HORZ)
					dwMode |= LM_HORZ | LM_HORZDOCK;
				else
					dwMode |=  LM_VERTDOCK;

				CSize sizeBar = pBar->CalcDynamicLayout(-1, dwMode);
				BOOL bIsMenuBar = FALSE;

				if (pBar->IsKindOf (RUNTIME_CLASS(CBCGMenuBar)))
				{
					bIsMenuBar = TRUE;

					if (dwMode & LM_HORZDOCK)
					{
						sizeBar.cx = sizeMax.cx;
					}
					else if (dwMode & LM_VERTDOCK)
					{
						sizeBar.cy = sizeMax.cy;
					}
				}

				if (bFirstBar)
				{
					if (dwMode & LM_HORZDOCK)
					{
						pt.x = -cxBorder;
					}
					else if (dwMode & LM_VERTDOCK)
					{
						pt.y = -cyBorder;
					}
				}

				CRect rect(pt, sizeBar);

				// get current rect for bar
				CRect rectBar;
				pBar->GetWindowRect(&rectBar);
				ScreenToClient(&rectBar);

				BOOL bMenuIsCutted = FALSE;

				if (bHorz)
				{
					// Offset Calculated Rect out to Actual
					if (rectBar.left > rect.left && !m_bFloating)
						rect.OffsetRect(rectBar.left - rect.left, 0);

					// If ControlBar goes off the right, then right justify

					if (rect.right > sizeMax.cx && !m_bFloating)
					{
						int x = rect.Width() - cxBorder;
						x = max(sizeMax.cx - x, pt.x);
						rect.OffsetRect(x - rect.left, 0);

						if (bIsMenuBar)
						{
							bMenuIsCutted = TRUE;
						}

						if (rect.right > sizeMax.cx)
						{
							rect.right = sizeMax.cx;
						}
					}

					// If ControlBar has been wrapped, then left justify
					if (bWrapped)
					{
						bWrapped = FALSE;
						rect.OffsetRect(-(rect.left + cxBorder), 0);
					}
					// If ControlBar is completely invisible, then wrap it
					else if (((rect.left >= (sizeMax.cx - cxBorder) || bMenuIsCutted) &&
						(nPos > 0) && (m_arrBars[nPos - 1] != NULL)) && CBCGFrameImpl::m_bToolbarWrapMode)
					{
						m_arrBars.InsertAt(nPos, (CObject*)NULL);
						pBar = NULL; pVoid = NULL;
						bWrapped = TRUE;
					}
					if (!bWrapped)
					{
						if (rect != rectBar)
						{
							if (!m_bLayoutQuery &&
								!(pBar->m_dwStyle & CBRS_FLOATING))
							{
								pBar->m_pDockContext->m_rectMRUDockPos = rect;
							}

							AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						}
						pt.x = rect.left + sizeBar.cx - cxBorder;
						nWidth = max(nWidth, sizeBar.cy);
					}
				}
				else
				{
					// Offset Calculated Rect out to Actual
					if (rectBar.top > rect.top && !m_bFloating)
						rect.OffsetRect(0, rectBar.top - rect.top);

					// If ControlBar goes off the bottom, then bottom justify
					if (rect.bottom > sizeMax.cy && !m_bFloating)
					{
						int y = rect.Height() - cyBorder;
						y = max(sizeMax.cy - y, pt.y);
						rect.OffsetRect(0, y - rect.top);

						if (bIsMenuBar)
						{
							bMenuIsCutted = TRUE;
						}

						if (rect.bottom > sizeMax.cy)
						{
							rect.bottom = sizeMax.cy;
						}
					}

					// If ControlBar has been wrapped, then top justify
					if (bWrapped)
					{
						bWrapped = FALSE;
						rect.OffsetRect(0, -(rect.top + cyBorder));
					}
					// If ControlBar is completely invisible, then wrap it
					else if (((rect.top >= (sizeMax.cy - cyBorder) || bIsMenuBar) &&
						(nPos > 0) && (m_arrBars[nPos - 1] != NULL)) && CBCGFrameImpl::m_bToolbarWrapMode)
					{
						m_arrBars.InsertAt(nPos, (CObject*)NULL);
						pBar = NULL; pVoid = NULL;
						bWrapped = TRUE;
					}

					if (!bWrapped)
					{
						if (rect != rectBar)
						{
							if (!m_bLayoutQuery &&
								!(pBar->m_dwStyle & CBRS_FLOATING) &&
								pBar->m_pDockContext != NULL)
							{
								pBar->m_pDockContext->m_rectMRUDockPos = rect;
							}

							AfxRepositionWindow(&layout, pBar->m_hWnd, &rect);
						}
						pt.y = rect.top + sizeBar.cy - cyBorder;
						nWidth = max(nWidth, sizeBar.cx);
					}
				}

				bFirstBar = FALSE;
			}

			if (!bWrapped)
			{
				// handle any delay/show hide for the bar
				pBar->RecalcDelayShow(&layout);
			}
		}

		if (pBar == NULL && pVoid == NULL && nWidth != 0)
		{
			// end of row because pBar == NULL
			if (bHorz)
			{
				pt.y += nWidth - cyBorder;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.x = -cxBorder;

				sizeFixed.cy --;

			}
			else
			{
				pt.x += nWidth - cxBorder;
				sizeFixed.cx = max(sizeFixed.cx, pt.x);
				sizeFixed.cy = max(sizeFixed.cy, pt.y);
				pt.y = -cyBorder;

				sizeFixed.cx --;
			}
			nWidth = 0;
		}
	}
	if (!m_bLayoutQuery)
	{
		// move and resize all the windows at once!
		if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
			TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
	}

	// adjust size for borders on the dock bar itself
	CRect rect;
	rect.SetRectEmpty();
	CalcInsideRect(rect, bHorz);

	int nExtra = CBCGFrameImpl::m_bControlBarExtraPixel ? 1 :0;

	if ((!bStretch || !bHorz) && sizeFixed.cx != 0)
	{
		sizeFixed.cx += -rect.right + rect.left + nExtra;
	}

	if ((!bStretch || bHorz) && sizeFixed.cy != 0)
	{
		sizeFixed.cy += -rect.bottom + rect.top + nExtra;
	}

	return sizeFixed;
}

// dwDockBarMap
const DWORD dwDockBarMap[4][2] =
{
	{ AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};


// Unfortunataly a simple rewrite of CFrameWnd's EnableDocking() is not possible,
// because we have not enough permissions to access some data in this class.
// That's why we call CFrameWnd::EnableDocking() first and exchange all occurencies
// of CDockBar objects with our own version of a dock bar.
void CBCGFrameImpl::FrameEnableDocking(CFrameWnd * pFrame, DWORD dwDockStyle) 
{
	ASSERT_VALID(pFrame);

	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

	pFrame->EnableDocking(dwDockStyle);

	for (int i = 0; i < 4; i++) 
	{
		if (dwDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY) 
		{
			CDockBar* pDock = (CDockBar*)pFrame->GetControlBar(dwDockBarMap[i][0]);
			ASSERT_VALID (pDock);
			
			// make sure the dock bar is of correct type
			if (!pDock->IsKindOf(RUNTIME_CLASS(CBCGToolDockBar)))
			{
				BOOL bNeedDelete = !pDock->m_bAutoDelete;
				pDock->m_pDockSite->RemoveControlBar(pDock);
				pDock->m_pDockSite = 0;	// avoid problems in destroying the dockbar
				pDock->DestroyWindow();
				if( bNeedDelete )
					delete pDock;
				pDock = 0;
			}

			if( pDock == 0 ) 
			{
				pDock = new CBCGToolDockBar;
				if (!pDock->Create(pFrame,
					WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
						dwDockBarMap[i][1], dwDockBarMap[i][0])) {
					AfxThrowResourceException();
				}
			}
		}
	}
}
//********************************************************************************
BOOL CBCGFrameImpl::OnMenuChar (UINT nChar)
{
	ASSERT_VALID (m_pFrame);

	if (m_pMenuBar != NULL &&
		(m_pMenuBar->GetStyle () & WS_VISIBLE) &&
		m_pMenuBar->TranslateChar (nChar))
	{
		return TRUE;
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGToolBar* pToolBar = (CBCGToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL &&
			pToolBar != m_pMenuBar &&
			(pToolBar->GetStyle () & WS_VISIBLE) &&
			pToolBar->GetTopLevelFrame () == m_pFrame &&
			pToolBar->TranslateChar (nChar))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//************************************************************************************
void CBCGFrameImpl::SetDockState(const CDockState& state)
{
	ASSERT_VALID (m_pFrame);
	int i = 0;

	// first pass through barinfo's sets the m_pBar member correctly
	// creating floating frames if necessary
	for (; i < state.m_arrBarInfo.GetSize(); i++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		if (pInfo->m_bFloating)
		{
			// need to create floating frame to match
			CMiniDockFrameWnd* pDockFrame = m_pFrame->CreateFloatingFrame(
				pInfo->m_bHorz ? CBRS_ALIGN_TOP : CBRS_ALIGN_LEFT);
			ASSERT(pDockFrame != NULL);
			CRect rect(pInfo->m_pointPos, CSize(10, 10));
			pDockFrame->CalcWindowRect(&rect);
			pDockFrame->SetWindowPos(NULL, rect.left, rect.top, 0, 0,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			CDockBar* pDockBar =
				(CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
			ASSERT(pDockBar != NULL);
			ASSERT_KINDOF(CDockBar, pDockBar);
			pInfo->m_pBar = pDockBar;
		}
		else // regular dock bar or toolbar
		{
			pInfo->m_pBar = m_pFrame->GetControlBar(pInfo->m_nBarID);
			//ASSERT(pInfo->m_pBar != NULL); //toolbar id's probably changed
		}
		if( pInfo->m_pBar )
			pInfo->m_pBar->m_nMRUWidth = pInfo->m_nMRUWidth;
	}
	
	// the second pass will actually dock all of the control bars and
	//  set everything correctly
	for (i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		if (pInfo->m_pBar != NULL)
			pInfo->m_pBar->SetBarInfo(pInfo, m_pFrame);
	}
	
	// last pass shows all the floating windows that were previously shown
	for (i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		//ASSERT(pInfo->m_pBar != NULL);
		if(pInfo->m_pBar && pInfo->m_bFloating)
		{
			CFrameWnd* pFrameWnd = pInfo->m_pBar->GetParentFrame();
			CDockBar* pDockBar = (CDockBar*)pInfo->m_pBar;
			ASSERT_KINDOF(CDockBar, pDockBar);
			if (pDockBar->GetDockedVisibleCount() > 0)
			{
				pFrameWnd->RecalcLayout();
				pFrameWnd->ShowWindow(SW_SHOWNA);
			}
		}
	}

	m_pFrame->DelayRecalcLayout();

	int nCmdShow = AfxGetApp()->m_nCmdShow;
	if (nCmdShow == SW_SHOWMINNOACTIVE  || nCmdShow == SW_MINIMIZE)
	{
		m_pFrame->m_nIdleFlags &= ~(CFrameWnd::idleLayout);
	}
}
//**************************************************************************************
BOOL CBCGFrameImpl::IsHelpKey (LPMSG lpMsg)
{
	return lpMsg->message == WM_KEYDOWN &&
		   lpMsg->wParam == VK_F1 &&
		   !(HIWORD(lpMsg->lParam) & KF_REPEAT) &&
		   GetKeyState(VK_SHIFT) >= 0 &&
		   GetKeyState(VK_CONTROL) >= 0 &&
		   GetKeyState(VK_MENU) >= 0;
}
//***************************************************************************************
void CBCGFrameImpl::DeactivateMenu ()
{
	if (!CBCGToolBar::IsCustomizeMode () &&
		CBCGPopupMenu::m_pActivePopupMenu != NULL)
	{
		if (m_pMenuBar != NULL)
		{
			m_pMenuBar->Deactivate ();
		}
	}
}
//*********************************************************************************
void CBCGFrameImpl::ShowQuickCustomizePane (CBCGPopupMenu* pMenuPopup)
{
	//---------------------------
	// Get Actual toolbar pointer
	//---------------------------
	CBCGToolBar* pWndParentToolbar = NULL;

	CBCGPopupMenu* pPopupLevel2 = pMenuPopup->GetParentPopupMenu ();
	if (pPopupLevel2 == NULL)
	{
		return;
	}
	
	CBCGPopupMenu* pPopupLevel1 = pPopupLevel2->GetParentPopupMenu ();
	if (pPopupLevel1 == NULL)
	{
		return;
	}

	CCustomizeButton* pCustom = (CCustomizeButton*)pPopupLevel1->GetParentButton ();
	if (pCustom == NULL)
	{
		return;
	}
	else
	{
		if (!pCustom->IsKindOf (RUNTIME_CLASS (CCustomizeButton)))
		{
			return;
		}

		CBCGToolBar* pCurrentToolBar = pCustom->GetParentToolbar ();

		CBCGToolbarMenuButton* btnDummy = pMenuPopup->GetMenuItem (0);
		int nID = _ttoi (btnDummy->m_strText);

		const CObList& gAllToolbars = CBCGToolBar::GetAllToolbars ();
	
		CBCGToolBar* pRealToolBar = NULL;
		for (POSITION pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
		{
			pRealToolBar = (CBCGToolBar*) gAllToolbars.GetNext (pos);
			ASSERT (pRealToolBar != NULL);
			if (nID == pRealToolBar->GetDlgCtrlID () &&
				pRealToolBar->IsAddRemoveQuickCustomize ())
			{
				break;
			}
			
			pRealToolBar = NULL;
		}

		if (pRealToolBar == NULL)
		{
			pWndParentToolbar = pCurrentToolBar;
		}
		else
		{
			pWndParentToolbar = pRealToolBar;
		}
	}

	pMenuPopup->RemoveAllItems ();

	CBCGToolbarCustomize* pStdCust = new CBCGToolbarCustomize (
											m_pFrame,
											TRUE,
											BCGCUSTOMIZE_MENUAMPERS);

	CBCGCustomizeMenuButton::SetParentToolbar (pWndParentToolbar);

	//--------------------------
	// Populate pop-up menu
	//-------------------------
	UINT uiRealCount = 0;
	CBCGCustomizeMenuButton::m_mapPresentIDs.RemoveAll ();

	UINT uiCount = pWndParentToolbar->GetCount ();
	for (UINT i=0; i< uiCount; i++)
	{
		CBCGToolbarButton* pBtn = pWndParentToolbar->GetButton (i);

		if (pBtn->IsKindOf (RUNTIME_CLASS (CCustomizeButton)) || (pBtn->m_nStyle & TBBS_SEPARATOR))
		{
			continue;
		}

		CBCGCustomizeMenuButton::m_mapPresentIDs.SetAt (pBtn->m_nID, 0);

		//---------------------------
		//Find Command Text if empty
		//---------------------------
		CString strText = pBtn->m_strText;
		if (pBtn->m_strText.IsEmpty())
		{
			strText = pStdCust->GetCommandName (pBtn->m_nID);
		}

		UINT uiID = pBtn->m_nID;

		if (pBtn->IsKindOf (RUNTIME_CLASS (CBCGToolbarMenuButton)))
		{
			uiID = BCGCUSTOMIZE_INTERNAL_ID;
		}

		if ((pBtn->m_nID == 0) || (pBtn->m_nID == -1))
		{
			uiID = BCGCUSTOMIZE_INTERNAL_ID;
		}

		CBCGCustomizeMenuButton button (uiID, NULL, pBtn->GetImage (), strText, pBtn->m_bUserButton);
		button.SetItemIndex (i);
		pMenuPopup->InsertItem (button);

		uiRealCount++;
	}

	delete pStdCust;

	pMenuPopup->SetQuickCustomizeType(CBCGPopupMenu::QUICK_CUSTOMIZE_PANE);

	//------------------------------------------
	//Give User ability to customize pane
	//-----------------------------------------
	OnShowCustomizePane (pMenuPopup, pWndParentToolbar->GetResourceID ());

	if (uiRealCount > 0)
	{
		pMenuPopup->InsertSeparator ();
	}

	//--------------------------
	// Add Reset Toolbar Button
	//--------------------------
	CString strCommand;

	{
		CBCGLocalResource locaRes;
		strCommand.LoadString (IDS_BCGBARRES_RESET_TOOLBAR); 
	}

	CBCGCustomizeMenuButton btnReset (BCGCUSTOMIZE_INTERNAL_ID, NULL, -1, strCommand, FALSE);

	btnReset.SetItemIndex (ID_BCGBARRES_TOOLBAR_RESET_PROMT); 

	pMenuPopup->InsertItem (btnReset);
}
//********************************************************************************
BOOL CBCGFrameImpl::OnShowCustomizePane (CBCGPopupMenu* pMenuPane, UINT uiToolbarID)
{
	//return TRUE;
	BOOL bResult = FALSE;

	CBCGMDIFrameWnd* pMainFrame =
				DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, m_pFrame);

	if (pMainFrame != NULL)
	{
		bResult = pMainFrame->OnShowCustomizePane (pMenuPane, uiToolbarID);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, m_pFrame);
		if (pFrame != NULL)
		{
			bResult = pFrame->OnShowCustomizePane (pMenuPane, uiToolbarID);

		}else	// Maybe, OLE frame
		{
			CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, m_pFrame);
			if (pOleFrame != NULL)
			{
				bResult = pOleFrame->OnShowCustomizePane (pMenuPane, uiToolbarID);
			}
			else
			{
				CBCGOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, m_pFrame);
				if (pOleDocFrame != NULL)
				{
					bResult = pOleDocFrame->OnShowCustomizePane (pMenuPane, uiToolbarID);
				}
			}
		}
	}

	return bResult;
}
//********************************************************************************
void CBCGFrameImpl::AddDefaultButtonsToCustomizePane (
						CBCGPopupMenu* pMenuPane, UINT /*uiToolbarID*/)
{
	CBCGToolBar* pWndParentToolbar = CBCGCustomizeMenuButton::GetParentToolbar ();
	
	if (pWndParentToolbar == NULL)
	{
		return;
	}
	
	CBCGToolbarCustomize* pStdCust = new CBCGToolbarCustomize (m_pFrame, TRUE, 
		BCGCUSTOMIZE_MENUAMPERS);

	
	const CObList& lstOrigButtons = pWndParentToolbar->GetOrigResetButtons (); 

	int i = 0;
	int nTmp = 0;
	for (POSITION posCurr = lstOrigButtons.GetHeadPosition (); posCurr != NULL; i++)
	{
		CBCGToolbarButton* pButtonCurr = (CBCGToolbarButton*)lstOrigButtons.GetNext (posCurr);

		UINT uiID = pButtonCurr->m_nID;

		if ((pButtonCurr == NULL) || 
			(pButtonCurr->m_nStyle & TBBS_SEPARATOR) ||
			(pButtonCurr->IsKindOf (RUNTIME_CLASS (CCustomizeButton))) ||
			 CBCGCustomizeMenuButton::m_mapPresentIDs.Lookup (uiID, nTmp))
		{
				continue;
		}

		if (pButtonCurr->IsKindOf (RUNTIME_CLASS (CBCGDropDownToolbarButton)))
		{
			//continue;

			CBCGDropDownToolbarButton* pDropButton = 
				DYNAMIC_DOWNCAST (CBCGDropDownToolbarButton, pButtonCurr);

			CBCGToolBar* pDropToolBar = pDropButton->GetDropDownToolBar ();
			if (pDropToolBar != NULL)
			{
				int nIndex = pDropToolBar->CommandToIndex (uiID);
				if (nIndex != -1 || uiID == 0)
				{
					continue;
				}
			}
		}

		if (pButtonCurr->IsKindOf (RUNTIME_CLASS (CBCGToolbarMenuButton)))
		{
			uiID = BCGCUSTOMIZE_INTERNAL_ID;
		}

		if ((pButtonCurr->m_nID == 0) || (pButtonCurr->m_nID == -1))
		{
			uiID = BCGCUSTOMIZE_INTERNAL_ID;
		}

		CBCGCustomizeMenuButton button (uiID, NULL, pButtonCurr->GetImage (), 
			pStdCust->GetCommandName (pButtonCurr->m_nID), pButtonCurr->m_bUserButton); 

		button.SetItemIndex(i, FALSE);

		int nIndex = pMenuPane->InsertItem (button, i);
		if (nIndex == -1)
		{
			pMenuPane->InsertItem (button);
		}
	}

	delete pStdCust;
}
//********************************************************************************
BOOL CBCGFrameImpl::IsCustomizePane (const CBCGPopupMenu* pMenuPopup) const
{
	CBCGPopupMenu* pPopupLevel2 = pMenuPopup->GetParentPopupMenu ();

	if (pPopupLevel2 == NULL)
	{
		return FALSE;
	}

	CString strLabel; 

	{
		CBCGLocalResource locaRes;  
		strLabel.LoadString (IDS_BCGBARRES_ADD_REMOVE_BTNS);
	}

	CBCGToolbarMenuButton* pButton = pPopupLevel2->GetParentButton ();
	if (pButton != NULL && pButton->m_strText.Find (strLabel) == -1)
	{
		return FALSE;
	}

		
	CBCGPopupMenu* pPopupLevel1 = pPopupLevel2->GetParentPopupMenu ();

	if (pPopupLevel1 == NULL)
	{
		return FALSE;
	}

	if (pPopupLevel1->GetQuickCustomizeType () == CBCGPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE)
	{
		return TRUE;
	}

	return FALSE;
}



