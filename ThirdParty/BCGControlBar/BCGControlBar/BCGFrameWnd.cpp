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

// BCGFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "BCGFrameWnd.h"
#include "BCGMenuBar.h"
#include "BCGPopupMenu.h"
#include "BCGSizingControlBar.h"
#include "BCGDockContext.h"
#include "BCGUserToolsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGFrameWnd

IMPLEMENT_DYNCREATE(CBCGFrameWnd, CFrameWnd)

#pragma warning (disable : 4355)

CBCGFrameWnd::CBCGFrameWnd() :
	m_Impl (this),
	m_bContextHelp (FALSE)
{
}

#pragma warning (default : 4355)

CBCGFrameWnd::~CBCGFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CBCGFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(BCGM_DELETETOOLBAR, OnToolbarDelete)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGFrameWnd message handlers

LRESULT CBCGFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	if (m_Impl.OnMenuChar (nChar))
	{
		return MAKELPARAM (MNC_EXECUTE, -1);
	}
		
	return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}
//*******************************************************************************************
afx_msg LRESULT CBCGFrameWnd::OnSetMenu (WPARAM wp, LPARAM lp)
{
	OnSetMenu ((HMENU) wp);
	return DefWindowProc (WM_MDISETMENU, NULL, lp);
}
//*******************************************************************************************
BOOL CBCGFrameWnd::OnSetMenu (HMENU hmenu)
{
	if (m_Impl.m_pMenuBar != NULL)
	{
		m_Impl.m_pMenuBar->CreateFromMenu 
			(hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGFrameWnd::PreTranslateMessage(MSG* pMsg) 
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
			CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);

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
			CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);

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

	return CFrameWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
BOOL CBCGFrameWnd::ShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu (pMenuPopup, this))
	{
		return FALSE;
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
//*******************************************************************************************
void CBCGFrameWnd::OnClosePopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		globalData.NotifyWinEvent (EVENT_SYSTEM_MENUEND,
			pMenuPopup->GetSafeHwnd (), OBJID_WINDOW, CHILDID_SELF);

		pMenuPopup->DestroyRecentMenu ();
	}

	if (CBCGPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CBCGPopupMenu::m_pActivePopupMenu = NULL;
	}
}
//*******************************************************************************************
BOOL CBCGFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
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
		return CFrameWnd::OnCommand(wParam, lParam);
	}

	return FALSE;
}
//******************************************************************
BOOL CBCGFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	m_Impl.m_nIDDefaultResource = nIDResource;

	if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	m_Impl.OnLoadFrame ();
	return TRUE;
}
//***************************************************************************************
void CBCGFrameWnd::OnClose() 
{
	m_Impl.OnCloseFrame();	
	CFrameWnd::OnClose();
}
//***************************************************************************************
BOOL CBCGFrameWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_Impl.RestorePosition(cs);
	return CFrameWnd::PreCreateWindow(cs);
}
//***************************************************************************************
void CBCGFrameWnd::WinHelp(DWORD dwData, UINT nCmd) 
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::WinHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp ();
	}
}
//***************************************************************************************
void CBCGFrameWnd::OnContextHelp ()
{
	m_bContextHelp = TRUE;

	if (!m_bHelpMode && CanEnterHelpMode())
	{
		CBCGToolBar::SetHelpMode ();
	}

	CFrameWnd::OnContextHelp ();

	if (!m_bHelpMode)
	{
		CBCGToolBar::SetHelpMode (FALSE);
	}

	m_bContextHelp = FALSE;
}
//****************************************************************************************
void CBCGFrameWnd::EnableDocking (DWORD dwDockStyle)
{
	m_Impl.FrameEnableDocking (this, dwDockStyle);
	m_pFloatingFrameClass = RUNTIME_CLASS(CBCGMiniDockFrameWnd);
}
//*******************************************************************************************
LRESULT CBCGFrameWnd::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ASSERT (lp != NULL);
	return (LRESULT) m_Impl.CreateNewToolBar ((LPCTSTR) lp);
}
//***************************************************************************************
LRESULT CBCGFrameWnd::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CBCGToolBar* pToolbar = (CBCGToolBar*) lp;
	ASSERT_VALID (pToolbar);

	return (LRESULT) m_Impl.DeleteToolBar (pToolbar);
}
//***************************************************************************************
void CBCGFrameWnd::DockControlBarLeftOf (CControlBar* pBar, CControlBar* pLeftOf)
{
	m_Impl.DockControlBarLeftOf (pBar, pLeftOf);
}
//************************************************************************************
void CBCGFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
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
//****************************************************************************
void CBCGFrameWnd::OnUpdateFrameMenu (HMENU hMenuAlt)
{
	CFrameWnd::OnUpdateFrameMenu (hMenuAlt);

	if (m_Impl.m_pMenuBar != NULL &&
		(m_Impl.m_pMenuBar->GetStyle () & WS_VISIBLE))
	{
		SetMenu (NULL);
	}
}
//*****************************************************************************
void CBCGFrameWnd::OnDestroy() 
{
    if (m_hAccelTable != NULL)
    {
        ::DestroyAcceleratorTable (m_hAccelTable);
        m_hAccelTable = NULL;
    }

	CFrameWnd::OnDestroy();
}

#if _MSC_VER >= 1300
void CBCGFrameWnd::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
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
BOOL CBCGFrameWnd::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		globalData.NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART,
			pMenuPopup->GetSafeHwnd (), OBJID_WINDOW, CHILDID_SELF);
	}
	return TRUE;
}