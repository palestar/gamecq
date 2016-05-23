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
//
// BCGDlgImpl.cpp: implementation of the CBCGDlgImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGPopupMenu.h"
#include "BCGToolbarMenuButton.h"
#include "BCGDialog.h"
#include "BCGPropertyPage.h"
#include "BCGDlgImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

HHOOK CBCGDlgImpl::m_hookMouse = NULL;
CBCGDlgImpl* CBCGDlgImpl::m_pMenuDlgImpl = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGDlgImpl::CBCGDlgImpl(CWnd& dlg) :
	m_Dlg (dlg)
{
}

CBCGDlgImpl::~CBCGDlgImpl()
{

}
//*******************************************************************************************
BOOL CBCGDlgImpl::ProcessMouseClick (POINT pt)
{
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
								m_Dlg.SetFocus ();

								return TRUE;
							}
						}
					}
				}
			}

			if (!CBCGPopupMenu::m_pActivePopupMenu->InCommand ())
			{
				CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);

				CWnd* pWndFocus = CWnd::GetFocus ();
				if (pWndFocus != NULL && pWndFocus->IsKindOf (RUNTIME_CLASS (CBCGToolBar)))
				{
					m_Dlg.SetFocus ();
				}

				if (clickArea != CBCGPopupMenu::OUTSIDE)	// Click on shadow
				{
					return TRUE;
				}
			}
		}
		else if (clickArea == CBCGPopupMenu::SHADOW_RIGHT ||
				clickArea == CBCGPopupMenu::SHADOW_BOTTOM)
		{
			CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
			m_Dlg.SetFocus ();

			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGDlgImpl::ProcessMouseMove (POINT pt)
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
//**************************************************************************************
BOOL CBCGDlgImpl::PreTranslateMessage(MSG* pMsg) 
{
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
		break;

	case WM_SYSKEYUP:
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
		if (CBCGPopupMenu::m_pActivePopupMenu != NULL &&
			::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_KEYDOWN, (int) pMsg->wParam);
			return TRUE;
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

			if (ProcessMouseClick (pt))
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

					m_Dlg.SendMessage (BCGM_TOOLBARMENU,
								(WPARAM) m_Dlg.GetSafeHwnd (),
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
		if (ProcessMouseClick (CPoint (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam))))
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

			if (ProcessMouseMove (pt))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//**********************************************************************************
LRESULT CALLBACK CBCGDlgImpl::BCGDlgMouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	MOUSEHOOKSTRUCT* lpMS = (MOUSEHOOKSTRUCT*) lParam;
	ASSERT (lpMS != NULL);

	if (m_pMenuDlgImpl != NULL)
	{
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			{
				CPoint ptCursor;
				::GetCursorPos (&ptCursor);

				CRect rectWindow;
				m_pMenuDlgImpl->m_Dlg.GetWindowRect (rectWindow);

				if (!rectWindow.PtInRect (ptCursor))
				{
					m_pMenuDlgImpl->ProcessMouseClick (ptCursor);
				}
			}
		}
	}

	return CallNextHookEx (m_hookMouse, nCode, wParam, lParam);
}
//****************************************************************************************
void CBCGDlgImpl::SetActiveMenu (CBCGPopupMenu* pMenu)
{
	CBCGPopupMenu::m_pActivePopupMenu = pMenu;

	if (pMenu != NULL)
	{
		if (m_hookMouse == NULL)
		{
			m_hookMouse = ::SetWindowsHookEx (WH_MOUSE, BCGDlgMouseProc, 
				0, GetCurrentThreadId ());
		}

		m_pMenuDlgImpl = this;
	}
	else 
	{
		if (m_hookMouse != NULL)
		{
			::UnhookWindowsHookEx (m_hookMouse);
			m_hookMouse = NULL;
		}

		m_pMenuDlgImpl = NULL;
	}

}
//****************************************************************************************
void CBCGDlgImpl::OnDestroy ()
{
	if (m_pMenuDlgImpl != NULL &&
		m_pMenuDlgImpl->m_Dlg.GetSafeHwnd () == m_Dlg.GetSafeHwnd ())
	{
		m_pMenuDlgImpl = NULL;
	}
}
//****************************************************************************************
BOOL CBCGDlgImpl::OnCommand (WPARAM wParam, LPARAM /*lParam*/)
{
	if (HIWORD (wParam) == 1)
	{
		UINT uiCmd = LOWORD (wParam);

		CBCGToolBar::AddCommandUsage (uiCmd);

		//---------------------------
		// Simmulate ESC keystroke...
		//---------------------------
		if (CBCGPopupMenu::m_pActivePopupMenu != NULL &&
			::IsWindow (CBCGPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			CBCGPopupMenu::m_pActivePopupMenu->SendMessage (WM_KEYDOWN, VK_ESCAPE);
			return TRUE;
		}

		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->InvokeTool (uiCmd))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//***************************************************************************************
void CBCGDlgImpl::OnNcActivate (BOOL& bActive)
{
	//----------------------------------------
	// Stay active if WF_STAYACTIVE bit is on:
	//----------------------------------------
	if (m_Dlg.m_nFlags & WF_STAYACTIVE)
	{
		bActive = TRUE;
	}

	//--------------------------------------------------
	// But do not stay active if the window is disabled:
	//--------------------------------------------------
	if (!m_Dlg.IsWindowEnabled ())
	{
		bActive = FALSE;
	}
}
//****************************************************************************************
void CBCGDlgImpl::OnActivate(UINT nState, CWnd* pWndOther)
{
	m_Dlg.m_nFlags &= ~WF_STAYACTIVE;

	//--------------------------------------------------
	// Determine if this window should be active or not:
	//--------------------------------------------------
	CWnd* pWndActive = (nState == WA_INACTIVE) ? pWndOther : &m_Dlg;
	if (pWndActive != NULL)
	{
		BOOL bStayActive = (pWndActive->GetSafeHwnd () == m_Dlg.GetSafeHwnd () ||
			pWndActive->SendMessage (WM_FLOATSTATUS, FS_SYNCACTIVE));

		if (bStayActive)
		{
			m_Dlg.m_nFlags |= WF_STAYACTIVE;
		}
	}
	else 
	{
		//------------------------------------------
		// Force painting on our non-client area....
		//------------------------------------------
		m_Dlg.SendMessage (WM_NCPAINT, 1);
	}
}
