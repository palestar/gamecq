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

// BCGOleDocIPFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGOleDocIPFrameWnd.h"
#include "BCGMenuBar.h"
#include "BCGSizingControlBar.h"
#include "BCGDockContext.h"
#include "BCGPopupMenu.h"
#include "BCGUserToolsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGOleDocIPFrameWnd

IMPLEMENT_DYNCREATE(CBCGOleDocIPFrameWnd, COleDocIPFrameWnd)

#pragma warning (disable : 4355)

CBCGOleDocIPFrameWnd::CBCGOleDocIPFrameWnd() :
	m_Impl (this),
	m_bContextHelp (FALSE),
	m_hwndLastTopLevelFrame (NULL)
{
}

#pragma warning (default : 4355)

CBCGOleDocIPFrameWnd::~CBCGOleDocIPFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGOleDocIPFrameWnd, COleDocIPFrameWnd)
	//{{AFX_MSG_MAP(CBCGOleDocIPFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(BCGM_DELETETOOLBAR, OnToolbarDelete)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGOleDocIPFrameWnd message handlers

LRESULT CBCGOleDocIPFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	if (m_Impl.OnMenuChar (nChar))
	{
		return MAKELPARAM (MNC_EXECUTE, -1);
	}
		
	return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}
//*******************************************************************************************
afx_msg LRESULT CBCGOleDocIPFrameWnd::OnSetMenu (WPARAM wp, LPARAM lp)
{
	OnSetMenu ((HMENU) wp);
	return DefWindowProc (WM_MDISETMENU, NULL, lp);
}
//*******************************************************************************************
BOOL CBCGOleDocIPFrameWnd::OnSetMenu (HMENU hmenu)
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
BOOL CBCGOleDocIPFrameWnd::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (!CBCGFrameImpl::IsHelpKey (pMsg) && 
			m_Impl.ProcessKeyboard ((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
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

	return CFrameWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
BOOL CBCGOleDocIPFrameWnd::ShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu (pMenuPopup, this))
	{
		return FALSE;
	}

	if (pMenuPopup != NULL && pMenuPopup->m_bShown)
	{
		return TRUE;
	}

	return OnShowPopupMenu (pMenuPopup);
}
//*******************************************************************************************
void CBCGOleDocIPFrameWnd::OnClosePopupMenu (CBCGPopupMenu* pMenuPopup)
{
	if (CBCGPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CBCGPopupMenu::m_pActivePopupMenu = NULL;
	}
}
//*******************************************************************************************
BOOL CBCGOleDocIPFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
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
BOOL CBCGOleDocIPFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	return CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);
}
//***************************************************************************************
void CBCGOleDocIPFrameWnd::WinHelp(DWORD dwData, UINT nCmd) 
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
void CBCGOleDocIPFrameWnd::OnContextHelp ()
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
void CBCGOleDocIPFrameWnd::EnableDocking (DWORD dwDockStyle)
{
	m_Impl.FrameEnableDocking (this, dwDockStyle);
	m_pFloatingFrameClass = RUNTIME_CLASS(CBCGMiniDockFrameWnd);
}
//*******************************************************************************************
LRESULT CBCGOleDocIPFrameWnd::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ASSERT (lp != NULL);
	return (LRESULT) m_Impl.CreateNewToolBar ((LPCTSTR) lp);
}
//***************************************************************************************
LRESULT CBCGOleDocIPFrameWnd::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CBCGToolBar* pToolbar = (CBCGToolBar*) lp;
	ASSERT_VALID (pToolbar);

	return (LRESULT) m_Impl.DeleteToolBar (pToolbar);
}
//***************************************************************************************
void CBCGOleDocIPFrameWnd::DockControlBarLeftOf (CControlBar* pBar, CControlBar* pLeftOf)
{
	m_Impl.DockControlBarLeftOf (pBar, pLeftOf);
}
//***************************************************************************************
void CBCGOleDocIPFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	COleDocIPFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
	switch (nState)
	{
	case WA_CLICKACTIVE:
		UpdateWindow ();
		break;

	case WA_INACTIVE:
		m_Impl.DeactivateMenu ();
		break;
	}

	if (nState == WA_INACTIVE)
	{
		if (CBCGPopupMenu::GetActiveMenu () != NULL)
		{
			CBCGPopupMenu::GetActiveMenu ()->SendMessage (WM_CLOSE);
		}

		if (g_pTopLevelFrame == this)
		{
			CFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST (CFrameWnd,
				CWnd::FromHandlePermanent (m_hwndLastTopLevelFrame));

			BCGSetTopLevelFrame (pTopLevelFrame);
		}
	}
	else
	{
		m_hwndLastTopLevelFrame = g_pTopLevelFrame->GetSafeHwnd ();
		g_pTopLevelFrame = this;
	}
}
//***************************************************************************************
void CBCGOleDocIPFrameWnd::OnClose() 
{
	m_Impl.OnCloseFrame();
	COleDocIPFrameWnd::OnClose();
}
//***************************************************************************************
void CBCGOleDocIPFrameWnd::OnDestroy() 
{
	if (CBCGPopupMenu::GetActiveMenu () != NULL)
	{
		CBCGPopupMenu::GetActiveMenu ()->SendMessage (WM_CLOSE);
	}

	if (g_pTopLevelFrame == this)
	{
		CFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST (CFrameWnd,
			CWnd::FromHandlePermanent (m_hwndLastTopLevelFrame));

		g_pTopLevelFrame = pTopLevelFrame;
	}

	m_Impl.DeactivateMenu ();

    if (m_hAccelTable != NULL)
    {
        ::DestroyAcceleratorTable (m_hAccelTable);
        m_hAccelTable = NULL;
    }

	COleDocIPFrameWnd::OnDestroy();
}

#if _MSC_VER >= 1300
void CBCGOleDocIPFrameWnd::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
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