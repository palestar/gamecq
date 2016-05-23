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

// BCGMDIFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "BCGMDIFrameWnd.h"
#include "BCGToolbar.h"
#include "BCGMenuBar.h"
#include "BCGPopupMenu.h"
#include "BCGToolbarMenuButton.h"
#include "BCGSizingControlBar.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "BCGDockContext.h"
#include "BCGWindowsManagerDlg.h"
#include "BCGUserToolsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGMDIFrameWnd

IMPLEMENT_DYNCREATE(CBCGMDIFrameWnd, CMDIFrameWnd)

#pragma warning (disable : 4355)

CBCGMDIFrameWnd::CBCGMDIFrameWnd() :
	m_Impl (this),
	m_hmenuWindow (NULL),
	m_bContextHelp (FALSE),
	m_bDoSubclass (TRUE),
	m_uiWindowsDlgMenuId (0),
	m_bShowWindowsDlgAlways (FALSE),
	m_bShowWindowsDlgHelpButton (FALSE)
{
}

#pragma warning (default : 4355)

CBCGMDIFrameWnd::~CBCGMDIFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGMDIFrameWnd, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CBCGMDIFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(BCGM_DELETETOOLBAR, OnToolbarDelete)
	ON_COMMAND( ID_CONTEXT_HELP, OnContextHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGMDIFrameWnd message handlers

BOOL CBCGMDIFrameWnd::OnSetMenu (HMENU hmenu)
{
	COleClientItem*	pActiveItem = GetInPlaceActiveItem ();
	if (pActiveItem != NULL && 
		pActiveItem->GetInPlaceWindow () != NULL)
	{
		return FALSE;
	}

	if (m_Impl.m_pMenuBar != NULL)
	{
		SetMenu (NULL);
		m_Impl.m_pMenuBar->CreateFromMenu (hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGMDIFrameWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (!CMDIFrameWnd::OnCreateClient(lpcs, pContext))
	{
		return FALSE;
	}

	if (m_bDoSubclass)
	{
		m_wndClientArea.SubclassWindow (m_hWndMDIClient);
	}

	return TRUE;
}
//*******************************************************************************************
LRESULT CBCGMDIFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	if (m_Impl.OnMenuChar (nChar))
	{
		return MAKELPARAM (MNC_EXECUTE, -1);
	}
		
	return CMDIFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}
//*******************************************************************************************
void CBCGMDIFrameWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMDIFrameWnd::OnWindowPosChanged(lpwndpos);
	
	if (m_Impl.m_pMenuBar != NULL)
	{
		BOOL bMaximized;
		CMDIChildWnd* pChild = MDIGetActive (&bMaximized);

		if (pChild == NULL || !bMaximized)
		{
			m_Impl.m_pMenuBar->SetMaximizeMode (FALSE);
		}
		else
		{
			m_Impl.m_pMenuBar->SetMaximizeMode (TRUE, pChild);
		}
	}
}
//*******************************************************************************************
BOOL CBCGMDIFrameWnd::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bProcessAccel = TRUE;

	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
	case WM_CONTEXTMENU:
		if (CBCGPopupMenu::m_pActivePopupMenu != NULL &&
			::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd) &&
			pMsg->wParam == VK_MENU)
		{
			CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
			return TRUE;
		}
		else if (m_Impl.ProcessKeyboard ((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if (m_Impl.m_pMenuBar != NULL && 
			(pMsg->wParam == VK_MENU || pMsg->wParam == VK_F10))
		{
			if (m_Impl.m_pMenuBar == GetFocus ())
			{
				SetFocus ();
			}
			else
			{
				if ((pMsg->lParam & (1 << 29)) == 0)
				{
					m_Impl.m_pMenuBar->SetFocus ();
				}
			}
			return TRUE;
		}

		if (CBCGPopupMenu::m_pActivePopupMenu != NULL &&
			::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			return TRUE;	// To prevent system menu opening
		}
		break;

	case WM_KEYDOWN:
		//-----------------------------------------
		// Pass keyboard action to the active menu:
		//-----------------------------------------
		if (!CBCGFrameImpl::IsHelpKey (pMsg) && 
			m_Impl.ProcessKeyboard ((int) pMsg->wParam, &bProcessAccel))
		{
			return TRUE;
		}

		if (!bProcessAccel)
		{
			return FALSE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);

			if (!::IsWindow (pMsg->hwnd))
			{
				return TRUE;
			}

			if (pWnd != NULL)
			{
				pWnd->ClientToScreen (&pt);
			}

			if (m_Impl.ProcessMouseClick (pMsg->message, pt, pMsg->hwnd))
			{
				return TRUE;
			}

			if (!::IsWindow (pMsg->hwnd))
			{
				return TRUE;
			}

			if (pMsg->message == WM_RBUTTONUP &&
				!CBCGToolBar::IsCustomizeMode ())
			{
				//---------------------------------------
				// Activate the control bar context menu:
				//---------------------------------------
				CDockBar* pBar = DYNAMIC_DOWNCAST(CDockBar, pWnd);
				if (pBar != NULL)
				{
					CPoint pt;

					pt.x = BCG_GET_X_LPARAM(pMsg->lParam);
					pt.y = BCG_GET_Y_LPARAM(pMsg->lParam);
					pBar->ClientToScreen(&pt);

					SendMessage (BCGM_TOOLBARMENU,
								(WPARAM) GetSafeHwnd (),
								MAKELPARAM (pt.x, pt.y));
				}
			}
		}
		break;

	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
		if (m_Impl.ProcessMouseClick (pMsg->message,
			CPoint (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam)),
			pMsg->hwnd))
		{
			return TRUE;
		}
		break;

	case WM_MOUSEMOVE:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);
			if (pWnd != NULL)
			{
				pWnd->ClientToScreen (&pt);
			}

			if (m_Impl.ProcessMouseMove (pt))
			{
				return TRUE;
			}
		}
	}

	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
BOOL CBCGMDIFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == 1)
	{
		UINT uiCmd = LOWORD (wParam);

		CBCGToolBar::AddCommandUsage (uiCmd);

		//---------------------------
		// Simmulate ESC keystroke...
		//---------------------------
		if (m_Impl.ProcessKeyboard (VK_ESCAPE))
		{
			return TRUE;
		}

		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->InvokeTool (uiCmd))
		{
			return TRUE;
		}
	}

	if (!CBCGToolBar::IsCustomizeMode ())
	{
		return CMDIFrameWnd::OnCommand(wParam, lParam);
	}

	return FALSE;
}
//*******************************************************************************************
HMENU CBCGMDIFrameWnd::GetWindowMenuPopup (HMENU hMenuBar)
{
	m_hmenuWindow = CMDIFrameWnd::GetWindowMenuPopup (hMenuBar);
	return m_hmenuWindow;
}
//********************************************************************************************
BOOL CBCGMDIFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	
	if (!CMDIFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	m_Impl.OnLoadFrame ();

	if (GetMenuBar () != NULL)
	{
		m_hMenuDefault = m_Impl.m_hDefaultMenu;
	}

	return TRUE;
}
//*******************************************************************************************
void CBCGMDIFrameWnd::OnClose() 
{
	// Deactivate OLE container first:
	COleClientItem*	pActiveItem = GetInPlaceActiveItem ();
	if (pActiveItem != NULL)
	{
		pActiveItem->Deactivate ();
	}

	m_Impl.OnCloseFrame();	
	CMDIFrameWnd::OnClose();
}
//*******************************************************************************************
BOOL CBCGMDIFrameWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_Impl.RestorePosition(cs);	
	return CMDIFrameWnd::PreCreateWindow(cs);
}
//*******************************************************************************************
BOOL CBCGMDIFrameWnd::ShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu (pMenuPopup, this))
	{
		return FALSE;
	}

	if (!CBCGToolBar::IsCustomizeMode () && m_hmenuWindow != NULL &&
		pMenuPopup != NULL && pMenuPopup->GetMenu () != NULL)
	{
		//-----------------------------------------------------------
		// Check the popup menu for the "Windows..." menu maching...:
		//-----------------------------------------------------------
		HMENU hMenuPop = pMenuPopup->GetMenu ();
		BOOL bIsWindowMenu = FALSE;

		int iItemMax = ::GetMenuItemCount (hMenuPop);
		for (int iItemPop = 0; !bIsWindowMenu && iItemPop < iItemMax; iItemPop ++)
		{
			UINT nID = ::GetMenuItemID( hMenuPop, iItemPop);
			bIsWindowMenu =  (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST);
		}

		if (bIsWindowMenu)
		{
			CMenu* pMenu = CMenu::FromHandle (m_hmenuWindow);
			if (pMenu != NULL)
			{
				int iCount = (int) pMenu->GetMenuItemCount ();
				BOOL bIsFirstWindowItem = TRUE;
				BOOL bIsStandradWindowsDlg = FALSE;

				for (int i = 0; i < iCount; i ++)
				{
					UINT uiCmd = pMenu->GetMenuItemID (i);
					if (uiCmd < AFX_IDM_FIRST_MDICHILD || uiCmd == (UINT) -1)
					{
						continue;
					}

					if (m_uiWindowsDlgMenuId != 0 &&
						uiCmd == AFX_IDM_FIRST_MDICHILD + 9)
					{
						// Don't add standrd "Windows..." command
						bIsStandradWindowsDlg = TRUE;
						continue;
					}

					if (bIsFirstWindowItem)
					{
						pMenuPopup->InsertSeparator ();
						bIsFirstWindowItem = FALSE;

						::SendMessage (m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
     				}

					CString strText;
					pMenu->GetMenuString (i, strText, MF_BYPOSITION);

					CBCGToolbarMenuButton button (uiCmd, NULL /* No submenus - assume */, 
													-1, strText);

					UINT uiState = pMenu->GetMenuState (i, MF_BYPOSITION);
					if (uiState & MF_CHECKED)
					{
						button.m_nStyle |= TBBS_CHECKED;
					}

					pMenuPopup->InsertItem (button);
				}

				if (m_uiWindowsDlgMenuId != 0 &&
					(bIsStandradWindowsDlg || m_bShowWindowsDlgAlways))
				{
					if (!CBCGToolBar::GetBasicCommands ().IsEmpty ())
					{
						CBCGToolBar::AddBasicCommand (m_uiWindowsDlgMenuId);
					}

					//-----------------------------
					// Add our "Windows..." dialog:
					//-----------------------------
					pMenuPopup->InsertItem (
						CBCGToolbarMenuButton (m_uiWindowsDlgMenuId,
								NULL, -1, m_strWindowsDlgMenuText));
				}
			}
		}
	}

	if (pMenuPopup != NULL && pMenuPopup->m_bShown)
	{
		return TRUE;
	}

	BOOL bResult = OnShowPopupMenu (pMenuPopup);
	
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		pMenuPopup->DestroyRecentMenu ();
		pMenuPopup->ExportToMenu ();
	}

	return bResult; 
}
//**********************************************************************************
void CBCGMDIFrameWnd::OnClosePopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		globalData.NotifyWinEvent(EVENT_SYSTEM_MENUEND, 
			pMenuPopup->GetSafeHwnd (), OBJID_WINDOW , CHILDID_SELF);

		pMenuPopup->DestroyRecentMenu ();
	}

	if (CBCGPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CBCGPopupMenu::m_pActivePopupMenu = NULL;
	}
}
//*******************************************************************************************
LRESULT CBCGMDIFrameWnd::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ASSERT (lp != NULL);
	return (LRESULT) m_Impl.CreateNewToolBar ((LPCTSTR) lp);
}
//***************************************************************************************
LRESULT CBCGMDIFrameWnd::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CBCGToolBar* pToolbar = (CBCGToolBar*) lp;
	ASSERT_VALID (pToolbar);

	return (LRESULT) m_Impl.DeleteToolBar (pToolbar);
}
//***************************************************************************************
void CBCGMDIFrameWnd::WinHelp(DWORD dwData, UINT nCmd) 
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CMDIFrameWnd::WinHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp ();
	}
}
//***************************************************************************************
void CBCGMDIFrameWnd::OnContextHelp ()
{
	m_bContextHelp = TRUE;

	if (!m_bHelpMode && CanEnterHelpMode())
	{
		CBCGToolBar::SetHelpMode ();
	}

	CMDIFrameWnd::OnContextHelp ();

	if (!m_bHelpMode)
	{
		CBCGToolBar::SetHelpMode (FALSE);
	}

	m_bContextHelp = FALSE;
}
//****************************************************************************************
void CBCGMDIFrameWnd::EnableDocking (DWORD dwDockStyle)
{
	m_Impl.FrameEnableDocking (this, dwDockStyle);
	m_pFloatingFrameClass = RUNTIME_CLASS(CBCGMiniDockFrameWnd);
}
//***************************************************************************************
void CBCGMDIFrameWnd::DockControlBarLeftOf (CControlBar* pBar, CControlBar* pLeftOf)
{
	m_Impl.DockControlBarLeftOf (pBar, pLeftOf);
}
//***************************************************************************************
void CBCGMDIFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
	switch (nState)
	{
	case WA_CLICKACTIVE:
		UpdateWindow ();
		break;

	case WA_INACTIVE:
		if (!CBCGToolBar::IsCustomizeMode ())
		{
			m_Impl.DeactivateMenu ();
		}
		break;
	}
}
//***************************************************************************************
void CBCGMDIFrameWnd::EnableWindowsDialog (UINT uiMenuId, 
										   LPCTSTR lpszMenuText,
										   BOOL bShowAllways,
										   BOOL bShowHelpButton)
{
	ASSERT (lpszMenuText != NULL);
	ASSERT (uiMenuId != 0);

	m_uiWindowsDlgMenuId = uiMenuId;
	m_strWindowsDlgMenuText = lpszMenuText;
	m_bShowWindowsDlgAlways = bShowAllways;
	m_bShowWindowsDlgHelpButton = bShowHelpButton;
}
//****************************************************************************
void CBCGMDIFrameWnd::EnableWindowsDialog (UINT uiMenuId, 
										   UINT uiMenuTextResId,
										   BOOL bShowAllways,
										   BOOL bShowHelpButton)
{
	CString strMenuText;
	VERIFY (strMenuText.LoadString (uiMenuTextResId));

	EnableWindowsDialog (uiMenuId, strMenuText, bShowAllways, bShowHelpButton);
}
//****************************************************************************
void CBCGMDIFrameWnd::ShowWindowsDialog ()
{
	CBCGLocalResource locaRes;

	CBCGWindowsManagerDlg dlg (this, m_bShowWindowsDlgHelpButton);
	dlg.DoModal ();
}
//****************************************************************************
COleClientItem*	CBCGMDIFrameWnd::GetInPlaceActiveItem ()
{
	CFrameWnd* pActiveFrame = GetActiveFrame ();
	if (pActiveFrame == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pActiveFrame);

	CView* pView = pActiveFrame->GetActiveView ();
	if (pView == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pView);

	COleDocument* pDoc = DYNAMIC_DOWNCAST (COleDocument, pView->GetDocument ());
	if (pDoc == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pDoc);
	return pDoc->GetInPlaceActiveItem (pView);
}
//****************************************************************************
void CBCGMDIFrameWnd::OnUpdateFrameMenu (HMENU hMenuAlt)
{
	CMDIFrameWnd::OnUpdateFrameMenu (hMenuAlt);

	if (m_Impl.m_pMenuBar != NULL &&
		(m_Impl.m_pMenuBar->GetStyle () & WS_VISIBLE))
	{
		COleClientItem*	pActiveItem = GetInPlaceActiveItem ();
		if (pActiveItem == NULL ||
			pActiveItem->GetInPlaceWindow () == NULL)
		{
			SetMenu (NULL);
		}
	}
}
//****************************************************************************************
void CBCGMDIFrameWnd::OnDestroy() 
{
    if (m_hAccelTable != NULL)
    {
        ::DestroyAcceleratorTable (m_hAccelTable);
        m_hAccelTable = NULL;
    }

	CMDIFrameWnd::OnDestroy();
}
//*************************************************************************************
void CBCGMDIFrameWnd::EnableMDITabs (BOOL bEnable/* = TRUE*/,
				BOOL bIcons/* = TRUE*/,
				CBCGTabWnd::Location tabLocation /* = CBCGTabWnd::LOCATION_BOTTOM*/,
				BOOL bHideNoTabs/* = FALSE*/,
				BOOL bTabCloseButton/* = FALSE*/,
				CBCGTabWnd::Style style/* = CBCGTabWnd::STYLE_3D_SCROLLED*/)
{
	ASSERT (style == CBCGTabWnd::STYLE_3D_SCROLLED ||
		style == CBCGTabWnd::STYLE_3D_ONENOTE ||
		style == CBCGTabWnd::STYLE_3D_VS2005);

	m_wndClientArea.EnableMDITabs (bEnable, bIcons, tabLocation, 
		bHideNoTabs, bTabCloseButton, style);
}
//*************************************************************************************
void CBCGMDIFrameWnd::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID (this);

	if (m_wndClientArea.DoesMDITabExist ())
	{
		((CWnd&)m_wndClientArea.GetMDITabs ()).ShowWindow (
			bPreview ? SW_HIDE : SW_SHOWNOACTIVATE);
	}

	CMDIFrameWnd::OnSetPreviewMode (bPreview, pState);
}

#if _MSC_VER >= 1300
void CBCGMDIFrameWnd::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::HtmlHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp ();
	}
}
#endif
//**************************************************************************************
BOOL CBCGMDIFrameWnd::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		globalData.NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART,
			pMenuPopup->GetSafeHwnd (), OBJID_WINDOW, CHILDID_SELF);

	}

	return TRUE;
}