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

// BCGPopupMenu.cpp : implementation file
//

#include "stdafx.h"

#pragma warning (disable : 4706)

#ifdef _AFXDLL
#define COMPILE_MULTIMON_STUBS
#endif // _AFXDLL

#include "multimon.h"

#pragma warning (default : 4706)

#include "BCGPopupMenu.h"
#include "BCGMenuBar.h"
#include "globals.h"
#include "BCGDockContext.h"
#include "BCGToolbarMenuButton.h"
#include "BCGMDIFrameWnd.h"
#include "BCGFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"
#include "BCGMenuBar.h"
#include "menupage.h"
#include "MenuHash.h"
#include "MenuImages.h"
#include "BCGShowAllButton.h"
#include "BCGUserToolsManager.h"
#include "BCGTearOffManager.h"
#include "BCGUserTool.h"
#include "bcgsound.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGDialog.h"
#include "BCGPropertyPage.h"
#include "BCGVisualManager.h"
#include "BCGDrawManager.h"
#include "BCGCustomizeMenuButton.h"
#include "BCGAccessibility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


UINT BCGM_ACCGETOBGECT	= ::RegisterWindowMessage (_T("BCGM_GETOBGECT"));
UINT BCGM_ACCHITTEST	= ::RegisterWindowMessage (_T("BCGM_ACCHITTEST"));
UINT BCGM_ACCGETSTATE	= ::RegisterWindowMessage (_T("BCGM_ACCGETSTATE"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenu

static const int iFadeStep = 10;
static const int iAnimTimerId = 1;
static const int iScrollTimerId = 2;
static const int iScrollTimerDuration = 80;
static const int iMenuBarId = 1;
static const int iTearOffBarHeight = 10;

CBCGPopupMenu::ANIMATION_TYPE CBCGPopupMenu::m_AnimationType = NO_ANIMATION;
UINT CBCGPopupMenu::m_AnimationSpeed = 30;
CBCGPopupMenu* CBCGPopupMenu::m_pActivePopupMenu = NULL;
BOOL CBCGPopupMenu::m_bForceShadow = TRUE;
BOOL CBCGPopupMenu::m_bForceMenuFocus = TRUE;
BOOL CBCGPopupMenu::m_bMenuSound = TRUE;
BOOL CBCGPopupMenu::m_bSendMenuSelectMsg = FALSE;

static clock_t nLastAnimTime = 0;

IMPLEMENT_SERIAL(CBCGPopupMenu, CMiniFrameWnd, VERSIONABLE_SCHEMA | 1)

CBCGPopupMenu::CBCGPopupMenu() :
	m_pMenuCustomizationPage (NULL)
{
	Initialize ();
}
//****************************************************************************************
CBCGPopupMenu::CBCGPopupMenu(CBCGMenuPage* pCustPage, LPCTSTR lpszTitle) :
	m_pMenuCustomizationPage (pCustPage),
	m_strCaption (lpszTitle)
{
	Initialize ();
}
//****************************************************************************************
void CBCGPopupMenu::Initialize ()
{
	if (globalData.bIsRemoteSession)
	{
		m_AnimationType = NO_ANIMATION;
	}

	m_hMenu = NULL;
	m_ptLocation = CPoint (0, 0);
	m_ptLocationInitial = CPoint (0, 0);
	m_pParentBtn = NULL;
	m_bAutoDestroyParent = TRUE;
	m_bAutoDestroy = TRUE;
	m_FinalSize = CSize (0, 0);
	m_AnimSize = CSize (0, 0);
	m_nMenuBarHeight = 0;
	m_bAnimationIsDone = (m_AnimationType == NO_ANIMATION);
	m_bScrollable = FALSE;
	m_bTobeDstroyed = FALSE;
	m_bShown = FALSE;

	m_iMaxWidth = -1;
	m_iLogoWidth = 0;
	m_nLogoLocation = MENU_LOGO_LEFT;

	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();
	m_iScrollBtnHeight = CMenuImages::Size ().cy + 
		2 * CBCGVisualManager::GetInstance ()->GetPopupMenuBorderSize ();
	m_iScrollMode = 0;

	m_bIsAnimRight = TRUE;
	m_bIsAnimDown = TRUE;

	m_iShadowSize = CBCGMenuBar::IsMenuShadows () &&
					!CBCGToolBar::IsCustomizeMode () &&
					globalData.m_nBitsPerPixel > 8 ? // Don't draw shadows in 256 colors or less
						CBCGVisualManager::GetInstance ()->GetMenuShadowDepth () : 0;

	m_iFadePercent = 0;
	if (m_AnimationType == FADE && globalData.m_nBitsPerPixel <= 8)
	{
		m_AnimationType = NO_ANIMATION;
		m_bAnimationIsDone = TRUE;
	}

	m_bTearOffTracking = FALSE;
	m_bIsTearOffCaptionActive = FALSE;
	m_rectTearOffCaption.SetRectEmpty ();

	m_DropDirection = DROP_DIRECTION_NONE;

	m_pMessageWnd = NULL;
	m_bTrackMode = FALSE;
	m_bRightAlign = FALSE;
	m_bQuickCusomize = FALSE;
	m_QuickType = QUICK_CUSTOMIZE_NONE;

	m_pMenuPopupAcc = NULL;
	m_pAccessible = NULL;
}
//****************************************************************************************
CBCGPopupMenu::~CBCGPopupMenu()
{
	if (m_bAutoDestroy && m_hMenu != NULL)
	{
		::DestroyMenu (m_hMenu);
	}
}

BEGIN_MESSAGE_MAP(CBCGPopupMenu, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CBCGPopupMenu)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE (WM_GETOBJECT, OnGetObject)
	ON_REGISTERED_MESSAGE (BCGM_ACCHITTEST, OnAccHitTest)
	ON_REGISTERED_MESSAGE (BCGM_ACCGETSTATE, OnAccGetState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupMenu message handlers

BOOL CBCGPopupMenu::Create (CWnd* pWndParent, int x, int y, HMENU hMenu, BOOL bLocked, BOOL bOwnMessage)
{
	BCGPlaySystemSound (BCGSOUND_MENU_POPUP);

	ASSERT (pWndParent != NULL);

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	m_hMenu = hMenu;

	if (x == -1 && y == -1)	// Undefined position
	{
		if (pWndParent != NULL)
		{
			CRect rectParent;
			pWndParent->GetClientRect (&rectParent);
			pWndParent->ClientToScreen (&rectParent);

			m_ptLocation = CPoint (rectParent.left + 5, rectParent.top + 5);
		}
		else
		{
			m_ptLocation = CPoint (0, 0);
		}
	}
	else
	{
		m_ptLocation = CPoint (x, y);
	}

	m_ptLocationInitial = m_ptLocation;

	DWORD dwStyle = WS_POPUP;
	if (m_pMenuCustomizationPage != NULL)
	{
		dwStyle |= (WS_CAPTION | WS_SYSMENU);
	}

	BOOL bIsAnimate = (m_AnimationType != NO_ANIMATION) && 
					!CBCGToolBar::IsCustomizeMode ();

	CBCGPopupMenu* pParentMenu = GetParentPopupMenu ();
	if (pParentMenu != NULL)
	{
		m_bTrackMode = pParentMenu->m_bTrackMode;
	}

	if (bOwnMessage)
	{
		m_pMessageWnd = pWndParent;
	}
	else if (pParentMenu != NULL)
	{
		m_pMessageWnd = pParentMenu->GetMessageWnd ();
	}

   // ---------------------------
	CRect rect (x, y, x, y);
	BOOL bCreated = CMiniFrameWnd::CreateEx (
				pWndParent->GetExStyle() & WS_EX_LAYOUTRTL,
				strClassName, m_strCaption,
				dwStyle, rect,
				pWndParent->GetOwner () == NULL ? 
					pWndParent : pWndParent->GetOwner ());
	if (!bCreated)
	{
		return FALSE;
	}

	if (m_bRightAlign)
	{
		m_ptLocation.x -= m_FinalSize.cx - 1;
		m_ptLocationInitial = m_ptLocation;

		RecalcLayout ();
	}

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	pMenuBar->m_bLocked = bLocked;

	if (bIsAnimate)
	{
		//--------------------------
		// Adjust initial menu size:
		//--------------------------
		m_AnimSize = m_FinalSize + CSize (m_iShadowSize, m_iShadowSize);

		switch (m_AnimationType)
		{
		case UNFOLD:
			m_AnimSize.cx = pMenuBar->GetColumnWidth ();

		case SLIDE:
			m_AnimSize.cy = pMenuBar->GetRowHeight ();
			break;
		}

		if (pMenuBar->IsWindowVisible ())
		{
			pMenuBar->ShowWindow (SW_HIDE);
		}

		SetTimer (iAnimTimerId, m_AnimationSpeed, NULL);
		nLastAnimTime = clock ();
	}

	//--------------------------------
	// Update windows covered by menu:
	//--------------------------------
	UpdateBottomWindows ();
	if (m_iShadowSize == 0 && m_AnimationType == FADE)
	{
		m_AnimSize = m_FinalSize;
	}

	SetWindowPos (&wndTop, -1, -1, -1, -1,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	if (CBCGToolBar::IsCustomizeMode ())
	{
		pMenuBar->Invalidate ();
		pMenuBar->UpdateWindow ();
	}

	return TRUE;
}
//****************************************************************************************

//-----------------------------------------------------
// My "classic " trick - how I can access to protected
// member m_pRecentFileList?
//-----------------------------------------------------
class CBCGApp : public CWinApp
{
	friend class CBCGPopupMenu;
};

int CBCGPopupMenu::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD toolbarStyle = dwDefaultToolbarStyle;
	if (m_AnimationType != NO_ANIMATION && !CBCGToolBar::IsCustomizeMode ())
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	m_wndMenuBar.m_bTrackMode = m_bTrackMode;

	if (!m_wndMenuBar.Create (this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, iMenuBarId))
	{
		TRACE(_T("Can't create popup menu bar\n"));
		return FALSE;
	}

	CBCGPopupMenu* pParentPopupMenu = GetParentPopupMenu ();
	if (pParentPopupMenu != NULL)
	{
		m_iMaxWidth = pParentPopupMenu->m_iMaxWidth;
	}

	m_wndMenuBar.m_iMaxWidth = m_iMaxWidth;
	m_wndMenuBar.SetOwner (GetParent ());

    return InitMenuBar()? 0 : -1;
}
//****************************************************************************************
void CBCGPopupMenu::OnSize(UINT nType, int cx, int cy) 
{
	CMiniFrameWnd::OnSize(nType, cx, cy);
	
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	if (pMenuBar->GetSafeHwnd () != NULL)
	{
		AdjustScroll (TRUE /*bForceMenuBarResize*/);
	}

	if (m_iLogoWidth > 0)
	{
		CRect rectLogo;
		GetClientRect (rectLogo);

		const int iBorderSize = CBCGVisualManager::GetInstance ()->GetPopupMenuBorderSize ();

		switch (m_nLogoLocation)
		{
		case MENU_LOGO_LEFT:
			rectLogo.right = rectLogo.left + iBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_RIGHT:
			rectLogo.left = rectLogo.right - iBorderSize - m_iLogoWidth;
			break;

		case MENU_LOGO_TOP:
			rectLogo.bottom = rectLogo.top + iBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_BOTTOM:
			rectLogo.top = rectLogo.bottom - iBorderSize - m_iLogoWidth;
			break;
		}

		InvalidateRect (rectLogo);
		UpdateWindow ();
	}
}
//****************************************************************************************
void CBCGPopupMenu::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!m_bAnimationIsDone && !CBCGToolBar::IsCustomizeMode ())
	{
		DrawFade (&dc);
	}
	else
	{
		DoPaint (&dc);
    }
}
//****************************************************************************************
int CBCGPopupMenu::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	return MA_NOACTIVATE;
}
//****************************************************************************************
void CBCGPopupMenu::RecalcLayout (BOOL /*bNotify*/) 
{
#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID (m_pParentBtn);
		ASSERT (m_pParentBtn->m_pPopupMenu == this);
	}
#endif // _DEBUG

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	if (!::IsWindow (m_hWnd) ||
		pMenuBar == NULL ||
		!::IsWindow (pMenuBar->m_hWnd))
	{
		return;
	}

	//-------------------------
	// Set tear-off attributes:
	//-------------------------
	BOOL bIsTearOff = (m_pParentBtn != NULL && m_pParentBtn->IsTearOffMenu () &&
						!CBCGToolBar::IsCustomizeMode ());

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (m_ptLocation, MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	//---------------------------------------
	// Normalize location in the screen area:
	//---------------------------------------
	m_ptLocation.x = max (rectScreen.left, min (m_ptLocation.x, rectScreen.right));

	if (m_pParentBtn == NULL)
	{
		m_ptLocation.y = max (rectScreen.top, min (m_ptLocation.y, rectScreen.bottom));
	}

	CSize size = pMenuBar->CalcSize (TRUE);

	const int nBorderSize = CBCGVisualManager::GetInstance ()->GetPopupMenuBorderSize ();

	size.cx += nBorderSize * 2;
	size.cy += nBorderSize * 2;

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
	case MENU_LOGO_RIGHT:
		size.cx += m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
	case MENU_LOGO_BOTTOM:
		size.cy += m_iLogoWidth;
		break;
	}

	if (m_pMenuCustomizationPage != NULL)
	{
		size.cy += ::GetSystemMetrics (SM_CYSMCAPTION);
		size.cy += 2 * ::GetSystemMetrics (SM_CYBORDER) + 5;
	}
	else if (bIsTearOff)
	{
		m_rectTearOffCaption = CRect (CPoint (nBorderSize, nBorderSize), 
			CSize (size.cx - 2 * nBorderSize, iTearOffBarHeight));
		size.cy += iTearOffBarHeight;

		if (!CBCGToolBar::IsCustomizeMode () && m_wndToolTip.GetSafeHwnd () == NULL)
		{
			CBCGLocalResource locaRes;

			m_wndToolTip.Create (this);
			m_wndToolTip.Activate (TRUE);
			if (globalData.m_nMaxToolTipWidth != -1)
			{
				m_wndToolTip.SetMaxTipWidth(globalData.m_nMaxToolTipWidth);
			}
			m_wndToolTip.AddTool (this, IDS_TEAR_OFF, m_rectTearOffCaption, 1);
		}
	}

	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;
	BOOL bIsRightAlign = bRTL || m_bRightAlign;

	CBCGMenuBar* pParentMenuBar = m_pParentBtn == NULL ? NULL :
		DYNAMIC_DOWNCAST (CBCGMenuBar, m_pParentBtn->m_pWndParent);

	if (pParentMenuBar != NULL && pParentMenuBar->IsFloating ())
	{
		// When the popup menu is dropped-down from the floating menu bar,
		// it should not cover other menu bar items. Ask parent menu bar about
		// right popup menu location:
		DROP_DIRECTION direction = (DROP_DIRECTION) 
			pParentMenuBar->GetFloatPopupDirection (m_pParentBtn);

		switch (direction)
		{
		case DROP_DIRECTION_TOP:
			m_ptLocation.y = m_ptLocationInitial.y - size.cy - m_pParentBtn->Rect ().Height () + 1;
			m_DropDirection = direction;
			break;

		case DROP_DIRECTION_LEFT:
			m_ptLocation.y = m_ptLocationInitial.y - m_pParentBtn->Rect ().Height () + 1;
			m_DropDirection = direction;

			if (bRTL)
			{
				m_ptLocation.x = m_ptLocationInitial.x + size.cx;

				if (m_ptLocation.x > rectScreen.right)
				{
					m_ptLocation.x = rectScreen.right;
					m_DropDirection = DROP_DIRECTION_NONE;
				}
			}
			else
			{
				m_ptLocation.x = m_ptLocationInitial.x - size.cx;

				if (m_ptLocation.x < rectScreen.left)
				{
					m_ptLocation.x = rectScreen.left;
					m_DropDirection = DROP_DIRECTION_NONE;
				}
			}
			break;

		case DROP_DIRECTION_RIGHT:
			if (bRTL)
			{
				m_ptLocation.x = m_ptLocationInitial.x - m_pParentBtn->Rect ().Width ();
			}
			else
			{
				m_ptLocation.x = m_ptLocationInitial.x + m_pParentBtn->Rect ().Width ();
			}

			m_ptLocation.y = m_ptLocationInitial.y - m_pParentBtn->Rect ().Height () + 1;
			m_DropDirection = direction;
			break;
		}
	}

	//---------------------------------
	// Prepare Quick Customize Drawing 
	//----------------------------------
	CRect rectQCParent;
	rectQCParent.SetRectEmpty ();

	BOOL bConnectQCToParent = FALSE;

	if (CBCGVisualManager::GetInstance ()->IsOfficeXPStyleMenus ())
	{
		CBCGPopupMenu* pParentPopup = GetParentPopupMenu();
		
		if (pParentPopup != NULL)
		{
			CBCGToolbarMenuButton* pParentBtn = GetParentButton ();
			if ((pParentBtn != NULL) && (pParentBtn->IsQuickMode()))
			{
				if (!bRTL)
				{
					pParentPopup->RedrawWindow();
				}
				
				if (pParentPopup->IsQuickCustomize ())
				{
					if (!m_bQuickCusomize)
					{
						rectQCParent = pParentBtn->Rect ();
						CWnd* pParentWnd = pParentBtn->GetParentWnd ();

						if (pParentWnd->GetSafeHwnd () != NULL)
						{
							pParentWnd->ClientToScreen (&rectQCParent);
							bConnectQCToParent = TRUE;

							m_ptLocation.y = rectQCParent.top;

							if (m_DropDirection == DROP_DIRECTION_LEFT)
							{
								m_ptLocation.x = 
									bRTL ?
										rectQCParent.left :
										rectQCParent.left - size.cx;
							}
							else
							{
								m_ptLocation.x = 
									bRTL ?
										rectQCParent.right + size.cx :
										rectQCParent.right;
							}
						}
					}
				}
			}
		}
	}

	//---------------------------------------------
	// Adjust the menu position by the screen size:
	//---------------------------------------------
	if ((bRTL && (m_ptLocation.x - size.cx < rectScreen.left)) || 
		(!bIsRightAlign && (m_ptLocation.x + size.cx > rectScreen.right)))
	{
		//-----------------------------------------------------
		// Menu can't be overlapped with the parent popup menu!
		//-----------------------------------------------------
		CBCGPopupMenu* pParentMenu = GetParentPopupMenu ();

		if (pParentMenu != NULL)
		{
			CRect rectParent;
			pParentMenu->GetWindowRect (rectParent);
			
			m_ptLocation.x = bRTL ? rectParent.right + size.cx : rectParent.left - size.cx;
			m_DropDirection = bRTL ? DROP_DIRECTION_RIGHT : DROP_DIRECTION_LEFT;
		}
		else if (pParentMenuBar != NULL && 
			(pParentMenuBar->m_dwStyle & CBRS_ORIENT_HORZ) == 0)
		{
			//------------------------------------------------
			// Parent menu bar is docked vertical, place menu 
			// in the left or right side of the parent frame:
			//------------------------------------------------
			CRect rectParentBtn = m_pParentBtn->Rect();
			pParentMenuBar->ClientToScreen(&rectParentBtn);

			m_ptLocation.x = bRTL ? rectParentBtn.right + size.cx : rectParentBtn.left - size.cx;
			m_DropDirection = DROP_DIRECTION_LEFT;
		}
		else
		{
			m_ptLocation.x = bRTL ? rectScreen.left + size.cx + 1 : m_bRightAlign ? rectScreen.left + 1 : rectScreen.right - size.cx - 1;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (!bRTL && (m_ptLocation.x < rectScreen.left))
		{
			m_ptLocation.x = rectScreen.left;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (bRTL && (m_ptLocation.x > rectScreen.right))
		{
			m_ptLocation.x = rectScreen.right;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (m_AnimationType == UNFOLD)
		{
			m_bIsAnimRight = FALSE;
		}
		else if (m_AnimationType == FADE)
		{
			m_bIsAnimRight = FALSE;
			m_bIsAnimDown = FALSE;
		}
	}

	if (m_ptLocation.y + size.cy > rectScreen.bottom)
	{
		m_bIsAnimDown = FALSE;

		if (m_pParentBtn != NULL && m_pParentBtn->GetParentWnd () != NULL &&
			GetParentPopupMenu () == NULL)
		{
			CPoint ptRight (m_pParentBtn->Rect ().right, 0);
            m_pParentBtn->GetParentWnd ()->ClientToScreen (&ptRight);

            CPoint ptTop (0, m_pParentBtn->Rect ().top - size.cy);
			m_pParentBtn->GetParentWnd ()->ClientToScreen (&ptTop);

            if (ptTop.y < 0)
            {
				int yParentButtonTop = ptTop.y + size.cy;

				//-----------------------------------------------------
				// Where more space: on top or on bottom of the button?
				//-----------------------------------------------------
				if (rectScreen.bottom - yParentButtonTop < 
					yParentButtonTop - rectScreen.top)
				{
					m_ptLocation.y = rectScreen.top;
					m_DropDirection = DROP_DIRECTION_NONE;
					size.cy += ptTop.y;
				}
				else
				{
					size.cy = rectScreen.bottom - m_ptLocation.y;
					m_bIsAnimDown = TRUE;
				}

				m_bScrollable = TRUE;
            }
            else
            {
                m_ptLocation.y = ptTop.y;

				m_DropDirection = 
					(pParentMenuBar != NULL && (pParentMenuBar->m_dwStyle & CBRS_ORIENT_HORZ)) ?
									DROP_DIRECTION_TOP : DROP_DIRECTION_NONE;
            }
		}
		else
		{
			if (bConnectQCToParent)
			{
				m_ptLocation.y = rectQCParent.bottom - size.cy - 1;
			}
			else
			{
				m_ptLocation.y -= size.cy;
				if (GetParentPopupMenu () != NULL)
				{
					m_ptLocation.y += pMenuBar->GetRowHeight () + nBorderSize * 2;
				}
			}
		}

		if (m_ptLocation.y < rectScreen.top)
		{
			m_ptLocation.y = rectScreen.top;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (m_ptLocation.y + size.cy > rectScreen.bottom)
		{
			size.cy = rectScreen.bottom - m_ptLocation.y;
			m_bScrollable = TRUE;
		}
	}

	if (m_ptLocation.y < rectScreen.top)
	{
		if (m_pParentBtn != NULL && m_pParentBtn->GetParentWnd () != NULL &&
			GetParentPopupMenu () == NULL)
		{
			CPoint ptRight (m_pParentBtn->Rect ().right, 0);
            m_pParentBtn->GetParentWnd ()->ClientToScreen (&ptRight);

            CPoint ptBottom (0, m_pParentBtn->Rect ().bottom);
			m_pParentBtn->GetParentWnd ()->ClientToScreen (&ptBottom);

			m_ptLocation.y = ptBottom.y;

			m_DropDirection = 
				(pParentMenuBar != NULL && (pParentMenuBar->m_dwStyle & CBRS_ORIENT_HORZ)) ?
								DROP_DIRECTION_BOTTOM : DROP_DIRECTION_NONE;
		}
		else
		{
			m_ptLocation.y = rectScreen.top;
		}

		if (m_ptLocation.y + size.cy > rectScreen.bottom)
		{
			m_ptLocation.y = rectScreen.top;

			if (size.cy > rectScreen.Height ())
			{
				size.cy = rectScreen.Height ();
				m_bScrollable = TRUE;
			}

			m_DropDirection = DROP_DIRECTION_NONE;
		}
	}

	m_FinalSize = size;

	if (m_AnimationType != NO_ANIMATION || m_bAnimationIsDone || 
		CBCGToolBar::IsCustomizeMode ())
	{
		if (!CBCGToolBar::IsCustomizeMode ())
		{
			size.cx += m_iShadowSize;
			size.cy += m_iShadowSize;
		}

		if (m_pMenuCustomizationPage != NULL)
		{
			SetWindowPos (NULL, -1, -1, size.cx, size.cy,
						SWP_NOMOVE | SWP_NOZORDER |
						SWP_NOACTIVATE);
		}
		else
		{
			SetWindowPos (NULL, m_ptLocation.x - (bRTL ? size.cx : 0), m_ptLocation.y, size.cx, size.cy,
						SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	if (CBCGToolBar::IsCustomizeMode ())
	{
		pMenuBar->AdjustLocations ();
	}

	//---------------------------------------------------
	// Parent button maybe covered by shadow, repaint it:
	//---------------------------------------------------
	if (m_iShadowSize != 0 &&
		!CBCGToolBar::IsCustomizeMode () &&
		m_pParentBtn != NULL &&
		m_pParentBtn->GetParentWnd () != NULL)
	{
		BOOL bOldShown = m_bShown;
		m_bShown = TRUE;

		CWnd* pWndParent = m_pParentBtn->GetParentWnd ();

		CRect rectInter;

		CRect rectMenu;
		GetWindowRect (rectMenu);

		CRect rectShadowRight (CPoint (bRTL ? rectMenu.left - 1 - m_iShadowSize : rectMenu.right + 1, rectMenu.top), 
					CSize (m_iShadowSize, rectMenu.Height () + m_iShadowSize));
		pWndParent->ScreenToClient (&rectShadowRight);

		if (rectInter.IntersectRect (rectShadowRight, m_pParentBtn->m_rect))
		{
			pWndParent->InvalidateRect (m_pParentBtn->m_rect);
			pWndParent->UpdateWindow ();
		}

		CRect rectShadowBottom (CPoint (rectMenu.left, rectMenu.bottom + 1),
					CSize (rectMenu.Width () + m_iShadowSize, m_iShadowSize));
		pWndParent->ScreenToClient (&rectShadowBottom);

		if (rectInter.IntersectRect (rectShadowBottom, m_pParentBtn->m_rect))
		{
			pWndParent->InvalidateRect (m_pParentBtn->m_rect);
			pWndParent->UpdateWindow ();
		}

		m_bShown = bOldShown;
	}
}
//****************************************************************************************
void CBCGPopupMenu::OnDestroy() 
{
	if (m_bQuickCusomize)
	{
		//------------------------------
		// Restore recently used state
		//------------------------------
		CBCGMenuBar::SetRecentlyUsedMenus(CBCGCustomizeMenuButton::m_bRecentlyUsedOld);	
	}

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	//---------------------------------------------------------
	// First, maybe we have a dragged menu item. Remove it now:
	//---------------------------------------------------------
	if (pMenuBar->m_pDragButton != NULL &&
		!pMenuBar->m_bIsDragCopy)
	{
		pMenuBar->RemoveButton (
			pMenuBar->ButtonToIndex (pMenuBar->m_pDragButton));
		pMenuBar->m_pDragButton = NULL;
	}

	if (m_pParentBtn != NULL)
	{
		ASSERT (m_pParentBtn->m_pPopupMenu == this);

		SaveState ();

		m_pParentBtn->m_pPopupMenu = NULL;
		m_pParentBtn->m_bClickedOnMenu = FALSE;

		CBCGPopupMenu* pParentMenu = GetParentPopupMenu ();
		if (pParentMenu != NULL)
		{
			if (m_bAutoDestroyParent && !CBCGToolBar::IsCustomizeMode ())
			{
				//-------------------------------------------
				// Automatically close the parent popup menu:
				//-------------------------------------------
				pParentMenu->SendMessage (WM_CLOSE);
				m_pParentBtn = NULL;
			}
		}
	}
	else
	{
		CBCGMenuBar::SetShowAllCommands (FALSE);
	}

#ifndef BCG_NO_CUSTOMIZATION

	if (m_pMenuCustomizationPage != NULL)
	{
		m_pMenuCustomizationPage->CloseContextMenu (this);
	}
				
#endif // BCG_NO_CUSTOMIZATION

	NotifyParentDlg (FALSE);

	//------------------------------------------------
	// Inform the main frame about the menu detsroyng:
	//------------------------------------------------
	CFrameWnd* pWndMain = BCGGetTopLevelFrame (this);

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pWndMain);
	if (pMainFrame != NULL)
	{
		pMainFrame->OnClosePopupMenu (this);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pWndMain);
		if (pFrame != NULL)
		{
			pFrame->OnClosePopupMenu (this);
		}
		else	// Maybe, OLE frame...
		{
			CBCGOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pWndMain);
			if (pOleFrame != NULL)
			{
				pOleFrame->OnClosePopupMenu (this);
			}
			else
			{
				CBCGOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pWndMain);
				if (pOleDocFrame != NULL)
				{
					pOleDocFrame->OnClosePopupMenu (this);
				}
			}
		}
	}

	if (m_bTrackMode && CBCGPopupMenu::m_pActivePopupMenu == this)
	{
		CBCGPopupMenu::m_pActivePopupMenu = NULL;
	}

	if (!CBCGToolBar::IsCustomizeMode () &&
		pWndMain != NULL && m_pActivePopupMenu == NULL &&
		GetParentToolBar () != NULL &&
		GetParentToolBar () != GetFocus ())
	{
		GetParentToolBar ()->Deactivate ();
	}

	CMiniFrameWnd::OnDestroy();
}
//****************************************************************************************
void CBCGPopupMenu::PostNcDestroy() 
{
	if (m_pParentBtn != NULL)
	{
		m_pParentBtn->OnCancelMode ();
	}
		
	CMiniFrameWnd::PostNcDestroy();
}
//****************************************************************************************
void CBCGPopupMenu::SaveState ()
{
	if (!CBCGToolBar::IsCustomizeMode ())
	{
		return;
	}

	if (m_pParentBtn == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pParentBtn);

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	HMENU hmenu = pMenuBar->ExportToMenu ();
	ASSERT (hmenu != NULL);

	m_pParentBtn->CreateFromMenu (hmenu);
	::DestroyMenu (hmenu);

	CBCGPopupMenu* pParentMenu = GetParentPopupMenu ();
	if (pParentMenu != NULL)
	{
		pParentMenu->SaveState ();
	}
}
//****************************************************************************************
void CBCGPopupMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	ASSERT_VALID (this);

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID (m_pParentBtn);
		ASSERT (m_pParentBtn->m_pPopupMenu == this);
	}
#endif // _DEBUG

	BOOL bHightlightWasChanged = FALSE;
	BOOL bIsRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	if (bIsRTL)
	{
		if (nChar == VK_LEFT)
		{
			nChar = VK_RIGHT;
		}
		else if (nChar == VK_RIGHT)
		{
			nChar = VK_LEFT;
		}
	}

	switch (nChar)
	{
	case VK_RIGHT:
		{
			//-------------------------------------------
			// Save animation type and disable animation:
			//-------------------------------------------
			ANIMATION_TYPE animType = m_AnimationType;
			m_AnimationType = NO_ANIMATION;

			//-------------------------------
			// Try to open next cascade menu:
			//-------------------------------
			CBCGToolbarMenuButton* pSelItem = GetSelItem ();
			if (pSelItem != NULL && 
				(pSelItem->m_nID == (UINT) -1 || pSelItem->m_nID == 0 ||
				pSelItem->IsEmptyMenuAllowed ()) &&
				pSelItem->OpenPopupMenu ())
			{
				if (pSelItem->m_pPopupMenu != NULL)
				{
					//--------------------------
					// Select a first menu item:
					//--------------------------
					if (GetSelItem () == pSelItem)
					{
						pSelItem->m_pPopupMenu->OnKeyDown (VK_HOME, 0, 0);
					}
				}
			}
			else
			{
				//------------------------------------------------------
				// No next menu, first try to go to the parent menu bar:
				//------------------------------------------------------
				CBCGToolBar* pToolBar = GetParentToolBar ();
				if (pToolBar != NULL && 
					!pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPopupMenuBar)))
				{
					pToolBar->NextMenu ();
				}
				else
				{
					//------------------------------------------------------
					// Close the current menu and move control to the parent
					// popup menu:
					//------------------------------------------------------
					CBCGPopupMenu* pParenMenu = GetParentPopupMenu ();
					if (pParenMenu != NULL)
					{
						pParenMenu->SendMessage (WM_KEYDOWN, 
							bIsRTL ? VK_LEFT : VK_RIGHT);
					}
				}
			}

			//------------------------
			// Restore animation type:
			//------------------------
			m_AnimationType = animType;
		}
		return;
				
	case VK_LEFT:
		{
			CBCGToolBar* pToolBar = GetParentToolBar ();
			if (pToolBar != NULL)
			{
				pToolBar->PrevMenu ();
			}
			else if (m_pParentBtn != NULL && m_pParentBtn->IsDroppedDown ())		
			{
				CloseMenu ();
			}
		}
		return;

	case VK_DOWN:
		if ((::GetAsyncKeyState (VK_CONTROL) & 0x8000) &&	// Ctrl+Down
			!pMenuBar->m_bAreAllCommandsShown)
		{
			ShowAllCommands ();
			break;
		}

	case VK_UP:
	case VK_HOME:
	case VK_END:
		bHightlightWasChanged = TRUE;

	case VK_RETURN:
		if (!CBCGToolBar::IsCustomizeMode ())
		{
			pMenuBar->OnKey (nChar);
		}
		break;

	case VK_ESCAPE:
		CloseMenu (TRUE);
		return;

	default:
		if (pMenuBar->OnKey (nChar))
		{
			return;
		}
		else
		{
			CMiniFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}
	}

	if (bHightlightWasChanged &&
		m_bScrollable && pMenuBar->m_iHighlighted >= 0)
	{
		//---------------------------------------
		// Maybe, selected item is invisible now?
		//---------------------------------------
		CBCGToolbarButton* pItem = pMenuBar->GetButton (pMenuBar->m_iHighlighted);
		if (pItem == NULL && pMenuBar->GetRowHeight () == 0)
		{
			ASSERT (FALSE);
		}
		else
		{
			CRect rectBar;
			pMenuBar->GetClientRect (rectBar);

			int iOffset = pMenuBar->GetOffset ();
			int iOffsetDelta = 0;

			if (pItem->Rect ().top < rectBar.top)
			{
				//---------------------
				// Scroll up is needed!
				//---------------------
				iOffsetDelta = (pItem->Rect ().top - rectBar.top) / 
					pMenuBar->GetRowHeight () - 1;
			}
			else if (pItem->Rect ().bottom > rectBar.bottom)
			{
				//-----------------------
				// Scroll down is needed!
				//-----------------------
				iOffsetDelta = (pItem->Rect ().bottom - rectBar.bottom) / 
					pMenuBar->GetRowHeight () + 1;
			}

			if (iOffsetDelta != 0)
			{
				int iTotalRows = m_FinalSize.cy / pMenuBar->GetRowHeight () - 2;

				iOffset += iOffsetDelta;
				iOffset = min (max (0, iOffset), 
					(int) pMenuBar->m_Buttons.GetCount () - iTotalRows - 1);

				pMenuBar->SetOffset (iOffset);

				if (AdjustScroll ())
				{
					//------------------------------------------
					// Scroll buttons were changed, adjust again
					//------------------------------------------
					AdjustScroll ();
				}
			}
		}
	}
}
//****************************************************************************************
CBCGPopupMenu* CBCGPopupMenu::GetParentPopupMenu () const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}

	CBCGPopupMenuBar* pParentBar = 
		DYNAMIC_DOWNCAST (CBCGPopupMenuBar, m_pParentBtn->m_pWndParent);
	if (pParentBar != NULL)
	{
		CBCGPopupMenu* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPopupMenu, pParentBar->GetParentFrame ());
		ASSERT_VALID (pParentMenu);

		return pParentMenu;
	}
	else
	{
		return NULL;
	}
}
//****************************************************************************************
CBCGToolBar* CBCGPopupMenu::GetParentToolBar () const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}

	CBCGToolBar* pParentBar = 
		DYNAMIC_DOWNCAST (CBCGToolBar, m_pParentBtn->m_pWndParent);
	return pParentBar;
}
//****************************************************************************************
CBCGToolbarMenuButton* CBCGPopupMenu::GetSelItem ()
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	return DYNAMIC_DOWNCAST (CBCGToolbarMenuButton,
							pMenuBar->GetHighlightedButton ());
}
//****************************************************************************************
void CBCGPopupMenu::CloseMenu (BOOL bSetFocusToBar)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	m_bTobeDstroyed = TRUE;

	SaveState ();

	CBCGPopupMenu* pParentMenu = GetParentPopupMenu ();
	CBCGToolBar* pParentToolBar = GetParentToolBar ();

	CFrameWnd* pWndMain = BCGGetTopLevelFrame (this);
	if (pParentMenu != NULL)
	{
		m_bAutoDestroyParent = FALSE;
		ActivatePopupMenu (pWndMain, pParentMenu);
	}
	else if (pParentToolBar != NULL)
	{
		ActivatePopupMenu (pWndMain, NULL);
		NotifyParentDlg (FALSE);

		if (bSetFocusToBar)
		{
			pParentToolBar->SetFocus ();
		}
	}
	else
	{
		ActivatePopupMenu (pWndMain, NULL);
		NotifyParentDlg (FALSE);
	}

	SendMessage (WM_CLOSE);
}
//****************************************************************************************
int CBCGPopupMenu::InsertItem (const CBCGToolbarMenuButton& button, int iInsertAt)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	return pMenuBar->InsertButton (button, iInsertAt);
}
//****************************************************************************************
int CBCGPopupMenu::InsertSeparator (int iInsertAt)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	return pMenuBar->InsertSeparator (iInsertAt);
}
//****************************************************************************************
int CBCGPopupMenu::GetMenuItemCount () const
{
	return (int) m_wndMenuBar.m_Buttons.GetCount ();
}
//****************************************************************************************
CBCGToolbarMenuButton* CBCGPopupMenu::GetMenuItem (int iIndex) const
{
	return (CBCGToolbarMenuButton*) m_wndMenuBar.GetButton (iIndex);
}
//****************************************************************************************
BOOL CBCGPopupMenu::RemoveItem (int iIndex)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	return pMenuBar->RemoveButton (iIndex);
}
//****************************************************************************************
void CBCGPopupMenu::RemoveAllItems ()
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	pMenuBar->RemoveAllButtons ();
}
//****************************************************************************************
BOOL CBCGPopupMenu::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPopupMenu::ActivatePopupMenu (CFrameWnd* pTopFrame, CBCGPopupMenu* pPopupMenu)
{
	if (pPopupMenu != NULL)
	{
		pPopupMenu->NotifyParentDlg (TRUE);
	}

	if (pTopFrame != NULL)
	{
		ASSERT_VALID (pTopFrame);

		BOOL bRes = TRUE;

		CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pTopFrame);
		if (pMainFrame != NULL)
		{
			bRes = pMainFrame->ShowPopupMenu (pPopupMenu);
		}
		else	// Maybe, SDI frame...
		{
			CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pTopFrame);
			if (pFrame != NULL)
			{
				bRes = pFrame->ShowPopupMenu (pPopupMenu);
			}
			else	// Maybe, OLE frame
			{
				CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pTopFrame);
				if (pOleFrame != NULL)
				{
					bRes = pOleFrame->ShowPopupMenu (pPopupMenu);
				}
				else
				{
					CBCGOleDocIPFrameWnd* pOleDocFrame = 
						DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pTopFrame);
					if (pOleDocFrame != NULL)
					{
						bRes = pOleDocFrame->ShowPopupMenu (pPopupMenu);
					}
				}
			}
		}

		if (!bRes)
		{
			if (pPopupMenu != NULL && !pPopupMenu->m_bTobeDstroyed)
			{
				pPopupMenu->CloseMenu ();
			}

			return FALSE;
		}
	}

	if (pPopupMenu != NULL)
	{
		CBCGPopupMenuBar* pMenuBar = pPopupMenu->GetMenuBar ();
		ASSERT_VALID (pMenuBar);

		CBCGPopupMenu* pParentMenu = DYNAMIC_DOWNCAST (CBCGPopupMenu, pMenuBar->GetParent ());
		if (pParentMenu != NULL && pParentMenu->GetParentButton () != NULL &&
			!pMenuBar->m_bAreAllCommandsShown)
		{
			// Check if "Show all" button is not exist yet:
			if (pMenuBar->m_Buttons.IsEmpty () ||
				DYNAMIC_DOWNCAST (CBCGShowAllButton, pMenuBar->m_Buttons.GetTail ()) == NULL)
			{
				pMenuBar->InsertButton (CBCGShowAllButton ());
			}
		}

		if (pPopupMenu->m_bTrackMode)
		{
			CBCGPopupMenu::m_pActivePopupMenu = pPopupMenu;
		}
	}

	return TRUE;
}
//************************************************************************************
#if _MSC_VER >= 1300
void CBCGPopupMenu::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
#else
void CBCGPopupMenu::OnActivateApp(BOOL bActive, HTASK /*hTask*/) 
#endif
{
	if (!bActive && !CBCGToolBar::IsCustomizeMode () &&
		!InCommand ())
	{
		if (m_bTrackMode)
		{
			m_bTobeDstroyed = TRUE;
		}

		PostMessage (WM_CLOSE);
	}
}
//*************************************************************************************
void CBCGPopupMenu::OnTimer(UINT_PTR nIDEvent) 
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	switch (nIDEvent)
	{
	case iAnimTimerId:
		if (!m_bAnimationIsDone)
		{
			clock_t nCurrAnimTime = clock ();

			int nDuration = nCurrAnimTime - nLastAnimTime;
			int nSteps = (int) (.5 + (float) nDuration / m_AnimationSpeed);

			switch (m_AnimationType)
			{
			case UNFOLD:
				m_AnimSize.cx += nSteps * pMenuBar->GetColumnWidth ();
				// no break intentionally

			case SLIDE:
				m_AnimSize.cy += nSteps * pMenuBar->GetRowHeight ();
				break;

			case FADE:
				m_iFadePercent += iFadeStep;
				if (m_iFadePercent > 100 + nSteps * iFadeStep)
				{
					m_iFadePercent = 101;
				}
				break;
			}

			if ((m_AnimationType != FADE && m_AnimSize.cy - m_iShadowSize >= m_FinalSize.cy) ||
				(m_AnimationType == UNFOLD && m_AnimSize.cx - m_iShadowSize >= m_FinalSize.cx) ||
				(m_AnimationType == FADE && m_iFadePercent > 100))
			{

				m_AnimSize.cx = m_FinalSize.cx + m_iShadowSize;
				m_AnimSize.cy = m_FinalSize.cy + m_iShadowSize;

				KillTimer (iAnimTimerId);

				GetMenuBar()->SetWindowPos (NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW|SWP_NOZORDER|SWP_SHOWWINDOW);
				GetMenuBar()->ValidateRect (NULL);
    
				m_bAnimationIsDone = TRUE;
			}

			Invalidate ();
			UpdateWindow ();

			nLastAnimTime = nCurrAnimTime;
		}
		break;

	case iScrollTimerId:
		{
			CPoint point;
			::GetCursorPos (&point);
			ScreenToClient (&point);

			CBCGToolbarMenuButton* pSelItem = GetSelItem ();
			if (pSelItem != NULL)
			{
				pSelItem->OnCancelMode ();
			}

			int iOffset = pMenuBar->GetOffset ();

			if (m_rectScrollUp.PtInRect (point) && m_iScrollMode < 0)	// Scroll Up
			{
				pMenuBar->SetOffset (iOffset - 1);
				AdjustScroll ();
			}
			else if (m_rectScrollDn.PtInRect (point) && m_iScrollMode > 0)	// Scroll Down
			{
				pMenuBar->SetOffset (iOffset + 1);
				AdjustScroll ();
			}
			else
			{
				KillTimer (iScrollTimerId);
				m_iScrollMode = 0;
				InvalidateRect (m_rectScrollDn);
				InvalidateRect (m_rectScrollUp);
			}
		}
		break;
	}

	CMiniFrameWnd::OnTimer(nIDEvent);
}
//****************************************************************************************
void CBCGPopupMenu::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bTearOffTracking)
	{
		if (!m_rectTearOffCaption.PtInRect (point))
		{
			ReleaseCapture ();
			m_bTearOffTracking = FALSE;

			TearOff (point);
		}

		return;
	}

	CMiniFrameWnd::OnMouseMove(nFlags, point);

	if (!m_bScrollable || m_iScrollMode != 0)
	{
		return;
	}
	
	if (m_rectScrollUp.PtInRect (point) && IsScrollUpAvailable ())
	{
		m_iScrollMode = -1;
		InvalidateRect (m_rectScrollUp);
	}
	else if (m_rectScrollDn.PtInRect (point) && IsScrollDnAvailable ())
	{
		m_iScrollMode = 1;
		InvalidateRect (m_rectScrollDn);
	}
	else
	{
		m_iScrollMode = 0;
	}

	if (m_iScrollMode != 0)
	{
		SetTimer (iScrollTimerId, iScrollTimerDuration, NULL);
	}
}
//****************************************************************************************
BOOL CBCGPopupMenu::IsScrollUpAvailable ()
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	return pMenuBar->GetOffset () > 0;
}
//****************************************************************************************
BOOL CBCGPopupMenu::IsScrollDnAvailable ()
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	if (pMenuBar->GetCount () == 0)
	{
		return FALSE;
	}

	CRect rectLastItem;
	pMenuBar->GetItemRect (pMenuBar->GetCount () - 1, rectLastItem);

	return rectLastItem.bottom > m_nMenuBarHeight + pMenuBar->GetRowHeight ();
}
//****************************************************************************************
void CBCGPopupMenu::CollapseSubmenus ()
{
	for (POSITION pos = m_wndMenuBar.m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGToolbarButton* pButton = 
			(CBCGToolbarButton*) m_wndMenuBar.m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		pButton->OnCancelMode ();
	}
}
//****************************************************************************************
void CBCGPopupMenu::DrawImage (CDC* pDC, const CRect& rect, int iImage, BOOL bDrawFrame)
{
	CRect rectImage (CPoint (0, 0), CMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= 2;

	pDC->FillRect (rectFill, &globalData.brBarFace);
	CMenuImages::Draw (pDC, (CMenuImages::IMAGES_IDS) iImage, rect);

	if (bDrawFrame)
	{
		pDC->Draw3dRect (rect,
			globalData.clrBarHilite,
			globalData.clrBarShadow);
	}
}
//****************************************************************************************
void CBCGPopupMenu::ShowAllCommands ()
{
	CBCGToolbarMenuButton* pParentMenuButton =
		DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, m_pParentBtn);
	if (pParentMenuButton != NULL)
	{
		m_wndMenuBar.SetHot (NULL);

		CBCGMenuBar::SetShowAllCommands ();

		// Play standard menu popup sound!
		BCGPlaySystemSound (BCGSOUND_MENU_POPUP);

		ShowWindow (SW_HIDE);
		m_bShown = FALSE;

		if (m_bmpShadowRight.GetSafeHandle () != NULL)
		{
			m_bmpShadowRight.DeleteObject ();
		}
		
		if (m_bmpShadowBottom.GetSafeHandle () != NULL)
		{
			m_bmpShadowBottom.DeleteObject ();
		}

		m_ptLocation = m_ptLocationInitial;

		InitMenuBar ();

		if (m_bScrollable)
		{
			AdjustScroll ();
		}

		UpdateBottomWindows ();

		ShowWindow (SW_SHOWNOACTIVATE);

		if (pParentMenuButton->m_pWndParent != NULL 
			&& ::IsWindow (pParentMenuButton->m_pWndParent->m_hWnd))
		{
			pParentMenuButton->m_pWndParent->InvalidateRect (
				pParentMenuButton->Rect ());
			pParentMenuButton->m_pWndParent->UpdateWindow ();
		}
	}
}
//**************************************************************************************
void CBCGPopupMenu::SetMaxWidth (int iMaxWidth)
{
	if (iMaxWidth == m_iMaxWidth)
	{
		return;
	}

	m_iMaxWidth = iMaxWidth;
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	if (!::IsWindow (m_hWnd) ||
		!::IsWindow (pMenuBar->m_hWnd))
	{
		return;
	}

	pMenuBar->m_iMaxWidth = m_iMaxWidth;
	RecalcLayout ();
}
//*************************************************************************************
BOOL CBCGPopupMenu::InitMenuBar ()
{
	if (m_hMenu != NULL)
	{
		ASSERT (::IsMenu (m_hMenu));

		if (m_pParentBtn != NULL || 
			!g_menuHash.LoadMenuBar (m_hMenu, &m_wndMenuBar))
		{
			//-------------------------------------------
			// Failed to restore, load the default state:
			//-------------------------------------------
			if (!m_wndMenuBar.ImportFromMenu (m_hMenu, !HideRarelyUsedCommands ()))
			{
				TRACE(_T("Can't import menu\n"));
				return FALSE;
			}
		}
	}

	POSITION pos;

	//----------------------------------------
	// Maybe, we should process the MRU files:
	//----------------------------------------
	CRecentFileList* pMRUFiles = 
		((CBCGApp*) AfxGetApp ())->m_pRecentFileList;

	if (pMRUFiles != NULL && !CBCGToolBar::IsCustomizeMode ())
	{
		int iMRUItemIndex = 0;
		BOOL bIsPrevSeparator = FALSE;

		for (pos = m_wndMenuBar.m_Buttons.GetHeadPosition (); 
			pos != NULL; iMRUItemIndex ++)
		{
			POSITION posSave = pos;

			CBCGToolbarButton* pButton = 
				(CBCGToolbarButton*) m_wndMenuBar.m_Buttons.GetNext (pos);
			ASSERT (pButton != NULL);

			if (pButton->m_nID == ID_FILE_MRU_FILE1 &&
				pButton->m_strText == _T("Recent File"))
			{
				//------------------------------
				// Remove dummy item ("Recent"):
				//------------------------------
				m_wndMenuBar.m_Buttons.RemoveAt (posSave);
				delete pButton;

				TCHAR szCurDir [_MAX_PATH];
				::GetCurrentDirectory (_MAX_PATH, szCurDir);

				int nCurDir = lstrlen (szCurDir);
				ASSERT (nCurDir >= 0);

				szCurDir [nCurDir] = _T('\\');
				szCurDir [++ nCurDir] = _T('\0');

				//---------------
				// Add MRU files:
				//---------------
				int iNumOfFiles = 0;	// Actual added to menu
				for (int i = 0; i < pMRUFiles->GetSize (); i ++)
				{
					CString strName;

					if (pMRUFiles->GetDisplayName (strName, i, 
						szCurDir, nCurDir))
					{
						//---------------------
						// Add shortcut number:
						//---------------------
						CString strItem;
						strItem.Format (_T("&%d %s"), ++iNumOfFiles, strName);

						m_wndMenuBar.InsertButton (
							CBCGToolbarMenuButton (
								ID_FILE_MRU_FILE1 + i, NULL,
								-1, strItem),
							iMRUItemIndex ++);
					}
				}

				//------------------------------------------------------
				// Usualy, the MRU group is "covered" by two seperators.
				// If MRU list is empty, remove redandant separator:
				//------------------------------------------------------
				if (iNumOfFiles == 0 &&	// No files were added
					bIsPrevSeparator &&	// Prev. button was separator
					pos != NULL)		// Not a last button
				{
					posSave = pos;

					pButton = (CBCGToolbarButton*) 
						m_wndMenuBar.m_Buttons.GetNext (pos);
					ASSERT (pButton != NULL);

					if (pButton->m_nStyle & TBBS_SEPARATOR)
					{
						//---------------------------------------
						// Next button also separator, remove it:
						//---------------------------------------
						m_wndMenuBar.m_Buttons.RemoveAt (posSave);
						delete pButton;
					}
				}

				break;
			}

			bIsPrevSeparator = (pButton->m_nStyle & TBBS_SEPARATOR);
		}
	}

	//--------------------------
	// Setup user-defined tools:
	//--------------------------
	if (g_pUserToolsManager != NULL && !CBCGToolBar::IsCustomizeMode ())
	{
		BOOL bToolsAreReady = FALSE;
		int iToolItemIndex = 0;

		BOOL bIsPrevSeparator = FALSE;

		for (pos = m_wndMenuBar.m_Buttons.GetHeadPosition (); pos != NULL; iToolItemIndex ++)
		{
			POSITION posSave = pos;

			CBCGToolbarButton* pButton = 
				(CBCGToolbarButton*) m_wndMenuBar.m_Buttons.GetNext (pos);
			ASSERT (pButton != NULL);

			if (g_pUserToolsManager->GetToolsEntryCmd () == pButton->m_nID)
			{
				//----------------------------------------------------
				// Replace dummy tools command by the user tools list:
				//----------------------------------------------------
				m_wndMenuBar.m_Buttons.RemoveAt (posSave);
				delete pButton;

				if (!bToolsAreReady)
				{
					const CObList& lstTools = g_pUserToolsManager->GetUserTools ();
					if (!bIsPrevSeparator && !lstTools.IsEmpty () &&
						!m_wndMenuBar.m_Buttons.IsEmpty ())
					{
						//-------------------------------------
						// Add separator before the first tool:
						//-------------------------------------
						m_wndMenuBar.InsertSeparator (iToolItemIndex++);
					}

					for (POSITION posTool = lstTools.GetHeadPosition (); posTool != NULL;)
					{
						CBCGUserTool* pTool = (CBCGUserTool*) lstTools.GetNext (posTool);
						ASSERT_VALID (pTool);

						//----------------------------------------------
						// Is user tool associated with the user image?
						//----------------------------------------------
						int iUserImage = CMD_MGR.GetCmdImage (pTool->GetCommandId (), TRUE);

						m_wndMenuBar.InsertButton (
							CBCGToolbarMenuButton (
								pTool->GetCommandId (), NULL,
								iUserImage == -1 ? 0 : iUserImage, pTool->m_strLabel,
								iUserImage != -1),
							iToolItemIndex ++);
					}

					if (pos != NULL)
					{
						//-------------------------------------
						// Add separator after the last tool:
						//-------------------------------------
						m_wndMenuBar.InsertSeparator (iToolItemIndex++);
						bIsPrevSeparator = TRUE;
					}

					bToolsAreReady = TRUE;
				}
			}
			else if (pButton->m_nStyle & TBBS_SEPARATOR)
			{
				if (bIsPrevSeparator)
				{
					m_wndMenuBar.m_Buttons.RemoveAt (posSave);
					delete pButton;
				}

				bIsPrevSeparator = TRUE;
			}
			else	
			{
				bIsPrevSeparator = FALSE;
			}
		}
	}

    CFrameWnd* pTarget = (CFrameWnd*) m_wndMenuBar.GetCommandTarget ();
	if (pTarget == NULL || !pTarget->IsFrameWnd())
	{
		pTarget = GetParentFrame();
	}
	if (pTarget != NULL)
	{
		m_wndMenuBar.OnUpdateCmdUI(pTarget, FALSE);
	}

	//-----------------------------------------------------------------------------
	// Maybe, main application frame should update the popup menu context before it
	// displayed (example - windows list):
	//-----------------------------------------------------------------------------
	if (!ActivatePopupMenu (BCGGetTopLevelFrame (this), this))
	{
		return FALSE;
	}

	RecalcLayout ();
	return TRUE;
}
//************************************************************************************
BOOL CBCGPopupMenu::HideRarelyUsedCommands () const
{
	return (m_pParentBtn != NULL);
}
//************************************************************************************
void CBCGPopupMenu::UpdateBottomWindows (BOOL bCheckOnly)
{
	CWnd* pWndMain = GetTopLevelParent ();
	if (pWndMain == NULL)
	{
		return;
	}

	if (m_iShadowSize == 0 || GetForceShadow ())
	{
		pWndMain->UpdateWindow ();
		return;
	}

	//---------------------------------------------------------
	// If menu will be shown outside of the application window,
	// don't show shadows!
	//---------------------------------------------------------
	CRect rectMain;
	pWndMain->GetWindowRect (rectMain);

	CRect rectMenu (m_ptLocation, 
		CSize (m_FinalSize.cx + m_iShadowSize, m_FinalSize.cy + m_iShadowSize));

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rectMenu.OffsetRect (-(m_FinalSize.cx + m_iShadowSize), 0);
	}

	CRect rectInter;
	rectInter.UnionRect (&rectMenu, &rectMain);

	if (rectInter != rectMain)
	{
		m_iShadowSize = 0;

		if (!bCheckOnly)
		{
			SetWindowPos (NULL, -1, -1, m_FinalSize.cx, m_FinalSize.cy,
						SWP_NOMOVE | SWP_NOZORDER |
						SWP_NOACTIVATE);
		}
	}
	else
	{
		pWndMain->UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPopupMenu::DoPaint (CDC* pPaintDC)
{
	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	BOOL bDrawShadows = m_iShadowSize != 0 &&
						!CBCGToolBar::IsCustomizeMode ();

	if (bDrawShadows)
	{
		BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

		if (bRTL)
		{
			rectClient.left += m_iShadowSize;
		}
		else
		{
			rectClient.right -= m_iShadowSize;
		}

		rectClient.bottom -= m_iShadowSize;

		const int iMinBrightness = 100;
		const int iMaxBrightness = 65;

		//--------------------------------------------
		// Draw the shadow, exclude the parent button:
		//--------------------------------------------
		CRect rectParentBtn;
		rectParentBtn.SetRectEmpty ();

		if (m_pParentBtn != NULL && GetParentPopupMenu () == NULL)
		{
			CWnd* pWnd = m_pParentBtn->GetParentWnd();
			if (pWnd!= NULL && pWnd->GetSafeHwnd() != NULL)
			{
				rectParentBtn = m_pParentBtn->Rect ();
				rectParentBtn.right--;
				rectParentBtn.bottom--;
				pWnd->MapWindowPoints(this, &rectParentBtn);
			}
		}

		//--------------------------------------------------------------
		// Prevent shadow drawing over Quick Customize Add-Remove button
		//--------------------------------------------------------------
		if (CBCGVisualManager::GetInstance ()->IsOfficeXPStyleMenus ())
		{
			CBCGPopupMenu* pParentPopup = GetParentPopupMenu ();
			if (pParentPopup != NULL)
			{
				ASSERT_VALID (pParentPopup);

				CBCGToolbarMenuButton* pParentBtn = GetParentButton ();
				if ((pParentBtn != NULL) && (pParentBtn->IsQuickMode ()))
				{
					if (pParentPopup->IsQuickCustomize ())
					{
						if (!m_bQuickCusomize && (m_DropDirection == DROP_DIRECTION_LEFT))
						{
							CWnd* pWnd = m_pParentBtn->GetParentWnd ();
							if (pWnd != NULL && pWnd->GetSafeHwnd () != NULL)
							{
								rectParentBtn = m_pParentBtn->Rect ();
								rectParentBtn.bottom += 2;
								pWnd->MapWindowPoints (this, &rectParentBtn);
							}
						}
					}
				}
			}
		}

		CBCGVisualManager::GetInstance ()->OnDrawMenuShadow (
			pPaintDC, rectClient, rectParentBtn,
			m_iShadowSize, iMinBrightness, iMaxBrightness,
			&m_bmpShadowBottom, &m_bmpShadowRight, bRTL);

		if (bRTL)
		{
			rectClient.OffsetRect(-m_iShadowSize,0);
		}
	}

	CBCGVisualManager::GetInstance ()->OnDrawMenuBorder (pPaintDC, this, rectClient);
	const int nBorderSize = CBCGVisualManager::GetInstance ()->GetPopupMenuBorderSize ();

	rectClient.DeflateRect (nBorderSize, nBorderSize);

	//---------------------------
	// Draw menu logo (if exist):
	//---------------------------
	if (m_iLogoWidth > 0)
	{
		CRect rectLogo = rectClient;

		switch (m_nLogoLocation)
		{
		case MENU_LOGO_LEFT:
			rectLogo.right = rectLogo.left + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_RIGHT:
			rectLogo.left = rectLogo.right - nBorderSize - m_iLogoWidth;
			break;

		case MENU_LOGO_TOP:
			rectLogo.bottom = rectLogo.top + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_BOTTOM:
			rectLogo.top = rectLogo.bottom - nBorderSize - m_iLogoWidth;
			break;
		}

		CFrameWnd* pWndMain = BCGGetTopLevelFrame (this);

		CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pWndMain);
		if (pMainFrame != NULL)
		{
			pMainFrame->OnDrawMenuLogo (pPaintDC, this, rectLogo);
		}
		else	// Maybe, SDI frame...
		{
			CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pWndMain);
			if (pFrame != NULL)
			{
				pFrame->OnDrawMenuLogo (pPaintDC, this, rectLogo);
			}
			else	// Maybe, OLE frame...
			{
				CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pWndMain);
				if (pOleFrame != NULL)
				{
					pOleFrame->OnDrawMenuLogo (pPaintDC, this, rectLogo);
				}
				else
				{
					CBCGOleDocIPFrameWnd* pOleDocFrame = 
						DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pWndMain);
					if (pOleDocFrame != NULL)
					{
						pOleDocFrame->OnDrawMenuLogo (pPaintDC, this, rectLogo);
					}
				}

			}
		}
	}

	if (!m_rectTearOffCaption.IsRectEmpty ())
	{
		CBCGVisualManager::GetInstance ()->OnDrawTearOffCaption (
			pPaintDC, m_rectTearOffCaption, m_bIsTearOffCaptionActive);
	}

	if (IsScrollUpAvailable ())
	{
		DrawImage (pPaintDC, m_rectScrollUp, 
			CMenuImages::IdArowUp, m_iScrollMode < 0);
	}

	if (IsScrollDnAvailable ())
	{
		DrawImage (pPaintDC, m_rectScrollDn, 
			CMenuImages::IdArowDown, m_iScrollMode > 0);
	}

	m_bShown = TRUE;
}

// ==================================================================
// 
// FUNCTION :  DrawFade ()
// 
// * Description : Draw the popup menu window with fading effect
// 
// * Authors: [Guillaume Nodet ]
//			  
// * Function parameters : 
// [pPaintDC] -	Pointer to the device context to draw into
// ==================================================================
void CBCGPopupMenu::DrawFade (CDC* pPaintDC)
{
	CRect rectClient;
	GetClientRect (&rectClient);

	int cx = m_FinalSize.cx + m_iShadowSize;
	int cy = m_FinalSize.cy + m_iShadowSize;

	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (pPaintDC))
	{
		return;
	}

	// create the three bitmaps if not done yet
	if (m_bmpScreenDst.GetSafeHandle() == NULL)
	{
		CBitmap* pBmpOld = NULL;

		if (m_AnimationType == FADE || globalData.m_nBitsPerPixel > 8)
		{
			// Fill in the BITMAPINFOHEADER
			BITMAPINFOHEADER bih;
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = cx;
			bih.biHeight = cy;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biSizeImage = cx * cy;
			bih.biXPelsPerMeter = 0;
			bih.biYPelsPerMeter = 0;
			bih.biClrUsed = 0;
			bih.biClrImportant = 0;

			HBITMAP hmbpDib;
			// Create a DIB section and attach it to the source bitmap
			hmbpDib = CreateDIBSection (
				dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeSrcBits,
				NULL, NULL);
			if (hmbpDib == NULL || m_cFadeSrcBits == NULL)
			{
				return;
			}

			m_bmpScreenSrc.Attach( hmbpDib );

			// Create a DIB section and attach it to the destination bitmap
			hmbpDib = CreateDIBSection (
				dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeDstBits,
				NULL, NULL);
			if (hmbpDib == NULL || m_cFadeDstBits == NULL)
			{
				return;
			}
			m_bmpScreenDst.Attach( hmbpDib );

			// Create a DIB section and attach it to the temporary bitmap
			hmbpDib = CreateDIBSection (
				dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeTmpBits,
				NULL, NULL);
			if (hmbpDib == NULL || m_cFadeTmpBits == NULL)
			{
				return;
			}

			m_bmpScreenTmp.Attach( hmbpDib );

			// get source image, representing the window below the popup menu
			pBmpOld = dcMem.SelectObject (&m_bmpScreenSrc);
			dcMem.BitBlt (0, 0, cx, cy, pPaintDC, rectClient.left, rectClient.top, SRCCOPY);

			// copy it to the destination so that shadow will be ok
			memcpy (m_cFadeDstBits, m_cFadeSrcBits, sizeof(COLORREF)* cx*cy);
			dcMem.SelectObject (&m_bmpScreenDst);
		}
		else
		{
			m_bmpScreenDst.CreateCompatibleBitmap (pPaintDC, cx, cy);
			pBmpOld = dcMem.SelectObject (&m_bmpScreenDst);
		}

		// get final image
		CRect rect;

		DoPaint (&dcMem);

		GetMenuBar()->GetWindowRect (&rect);
		ScreenToClient (&rect);

		dcMem.SetViewportOrg (rect.TopLeft());
		GetMenuBar()->DoPaint (&dcMem);
		dcMem.SetViewportOrg (CPoint (0,0));

		dcMem.SelectObject (pBmpOld);
	}
	
	COLORREF *src = m_cFadeSrcBits;
	COLORREF *dst = m_cFadeDstBits;
	COLORREF *tmp = m_cFadeTmpBits;

	CBitmap* pBmpOld = NULL;

	switch (m_AnimationType)
	{
	case UNFOLD:
	case SLIDE:
		pBmpOld = dcMem.SelectObject (&m_bmpScreenDst);

		pPaintDC->BitBlt (
			m_bIsAnimRight ? rectClient.left : rectClient.right - m_AnimSize.cx,
			m_bIsAnimDown ? rectClient.top : rectClient.bottom - m_AnimSize.cy,
			m_AnimSize.cx, m_AnimSize.cy, &dcMem, 0, 0, SRCCOPY);
		break;

	case FADE:
		pBmpOld = dcMem.SelectObject (&m_bmpScreenTmp);
		for (int pixel = 0; pixel < cx * cy; pixel++)
		{
			*tmp++ = CBCGDrawManager::PixelAlpha (*src++, *dst++, 100 - m_iFadePercent);
		}

		pPaintDC->BitBlt (rectClient.left, rectClient.top, cx, cy, 
						&dcMem, 0, 0, SRCCOPY);
	}

	dcMem.SelectObject (pBmpOld);
}

//************************************************************************************
BOOL CBCGPopupMenu::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (!CMiniFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return m_pMessageWnd != NULL ? 
			m_pMessageWnd->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo) : FALSE;
	}

	return TRUE;
}
//************************************************************************************
BOOL CBCGPopupMenu::PostCommand (UINT commandID)
{
	if (m_pMessageWnd != NULL)
	{
		return m_pMessageWnd->PostMessage (WM_COMMAND, commandID);
	}

	return FALSE;
}
//************************************************************************************
void CBCGPopupMenu::EnableMenuLogo (int iLogoSize, LOGO_LOCATION nLogoLocation/* = MENU_LOGO_LEFT*/)
{
	m_iLogoWidth = iLogoSize;
	m_nLogoLocation = nLogoLocation;

	RecalcLayout ();
}
//************************************************************************************
BOOL CBCGPopupMenu::AdjustScroll (BOOL bForceMenuBarResize)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	CRect rectClient;
	GetClientRect (rectClient);

	if (!CBCGToolBar::IsCustomizeMode ())
	{
		rectClient.right -= m_iShadowSize;
		rectClient.bottom -= m_iShadowSize;
	}

	const int iBorderSize = CBCGVisualManager::GetInstance ()->GetPopupMenuBorderSize ();
	rectClient.DeflateRect (iBorderSize, iBorderSize);

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
		rectClient.left += m_iLogoWidth;
		break;

	case MENU_LOGO_RIGHT:
		rectClient.right -= m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
		rectClient.top += m_iLogoWidth;
		break;

	case MENU_LOGO_BOTTOM:
		rectClient.bottom -= m_iLogoWidth;
		break;
	}

	rectClient.top += m_rectTearOffCaption.Height ();

	CRect rectScrollUpOld = m_rectScrollUp;
	CRect rectScrollDnOld = m_rectScrollDn;

	m_rectScrollUp.SetRectEmpty ();
	m_rectScrollDn.SetRectEmpty ();

	UINT uiSWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

	if (m_bScrollable)
	{
		if (IsScrollUpAvailable ())
		{
			m_rectScrollUp = rectClient;
			m_rectScrollUp.top += iBorderSize;
			m_rectScrollUp.bottom = m_rectScrollUp.top + m_iScrollBtnHeight;

			rectClient.top += m_iScrollBtnHeight + iBorderSize;
		}

		if (IsScrollDnAvailable ())
		{
			m_rectScrollDn = rectClient;
			m_rectScrollDn.top = m_rectScrollDn.bottom - m_iScrollBtnHeight;

			rectClient.bottom -= m_iScrollBtnHeight + iBorderSize;
		}
	}
	else if (!m_bAnimationIsDone)
	{
		uiSWPFlags |= SWP_NOREDRAW;

		KillTimer (iScrollTimerId);
		m_iScrollMode = 0;
	}

	if (bForceMenuBarResize ||
		rectScrollUpOld != m_rectScrollUp ||
		rectScrollDnOld != m_rectScrollDn)
	{
		pMenuBar->SetWindowPos (NULL, rectClient.left, rectClient.top,
					rectClient.Width (), 
					rectClient.Height (),
					uiSWPFlags);
		m_nMenuBarHeight = rectClient.Height ();
	}
	else
	{
		pMenuBar->AdjustLayout ();
	}

	BOOL bScrollButtonsChanged = FALSE;

	if (rectScrollUpOld != m_rectScrollUp)
	{
		InvalidateRect (rectScrollUpOld);
		InvalidateRect (m_rectScrollUp);

		bScrollButtonsChanged = TRUE;
	}

	if (rectScrollDnOld != m_rectScrollDn)
	{
		InvalidateRect (rectScrollDnOld);
		InvalidateRect (m_rectScrollDn);

		bScrollButtonsChanged = TRUE;
	}

	if (bScrollButtonsChanged)
	{
		UpdateWindow ();
	}

	return bScrollButtonsChanged;
}
//************************************************************************************
CBCGPopupMenu::MENUAREA_TYPE CBCGPopupMenu::CheckArea (const CPoint& ptScreen) const
{
	ASSERT_VALID (this);

	CRect rectWindow;
	GetClientRect (rectWindow);
	ClientToScreen (rectWindow);

	if (!rectWindow.PtInRect (ptScreen))
	{
		return OUTSIDE;
	}

	CRect rectLogo = rectWindow;

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
		rectLogo.right = rectLogo.left + m_iLogoWidth;
		break;

	case MENU_LOGO_RIGHT:
		rectLogo.left = rectLogo.right - m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
		rectLogo.bottom = rectLogo.top + m_iLogoWidth;
		break;

	case MENU_LOGO_BOTTOM:
		rectLogo.top = rectLogo.bottom - m_iLogoWidth;
		break;
	}

	if (rectLogo.PtInRect (ptScreen))
	{
		return LOGO;
	}

	if (ptScreen.x > rectWindow.right - m_iShadowSize)
	{
		return SHADOW_RIGHT;
	}

	if (ptScreen.y > rectWindow.bottom - m_iShadowSize)
	{
		return SHADOW_BOTTOM;
	}

	if (!m_rectTearOffCaption.IsRectEmpty ())
	{
		CRect rectTearOffCaption = m_rectTearOffCaption;
		ClientToScreen (&rectTearOffCaption);

		if (rectTearOffCaption.PtInRect (ptScreen))
		{
			return TEAROFF_CAPTION;
		}
	}

	return MENU;
}
//************************************************************************************
void CBCGPopupMenu::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!CBCGToolBar::IsCustomizeMode () &&
		m_rectTearOffCaption.PtInRect (point))
	{
		m_bIsTearOffCaptionActive = TRUE;
		InvalidateRect (m_rectTearOffCaption);

		m_bTearOffTracking = TRUE;
		SetCapture ();
		return;
	}
	
	CMiniFrameWnd::OnLButtonDown(nFlags, point);
}
//*************************************************************************************
void CBCGPopupMenu::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTearOffTracking)
	{
		m_bTearOffTracking = FALSE;
		ReleaseCapture ();

		m_bIsTearOffCaptionActive = FALSE;
		InvalidateRect (m_rectTearOffCaption);
	}
	
	CMiniFrameWnd::OnLButtonUp(nFlags, point);
}
//*************************************************************************************
BOOL CBCGPopupMenu::TearOff (CPoint point)
{
	if (m_pParentBtn == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	UINT uiID = m_pParentBtn->m_uiTearOffBarID;
	if (uiID == 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	CFrameWnd* pWndMain = BCGGetTopLevelFrame (this);
	if (pWndMain == NULL)
	{
		return FALSE;
	}

	CControlBar* pBar = pWndMain->GetControlBar (uiID);
	if (pBar != NULL)	// Already exist, just show it
	{
		pWndMain->FloatControlBar (pBar, ptScreen);
		pWndMain->ShowControlBar (pBar, TRUE, FALSE);
	}
	else
	{
		CBCGFrameImpl* pFrameImpl = NULL;

		CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pWndMain);
		if (pMainFrame != NULL)
		{
			pFrameImpl = &pMainFrame->m_Impl;
		}
		else	// Maybe, SDI frame...
		{
			CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pWndMain);
			if (pFrame != NULL)
			{
				pFrameImpl = &pFrame->m_Impl;
			}
			else	// Maybe, OLE frame...
			{
				CBCGOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pWndMain);
				if (pOleFrame != NULL)
				{
					pFrameImpl = &pOleFrame->m_Impl;
				}
				else
				{
					CBCGOleDocIPFrameWnd* pOleDocFrame = 
						DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pWndMain);
					if (pOleDocFrame != NULL)
					{
						pFrameImpl = &pOleDocFrame->m_Impl;
					}
				}
			}
		}

		if (pFrameImpl == NULL)
		{
			return FALSE;
		}

		CString strCaption = m_pParentBtn->m_strText;
		strCaption.Remove (_T('&'));

		if ((pBar = CreateTearOffBar (pWndMain, uiID, strCaption)) == NULL)
		{
			return FALSE;
		}

		pFrameImpl->AddTearOffToolbar (pBar);
	}

	ASSERT_VALID (pBar);

	//--------------------------------
	// Send trigger to the main frame:
	//--------------------------------
	BOOL bTearOff = TRUE;

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pWndMain);
	if (pMainFrame != NULL)
	{
		bTearOff = pMainFrame->OnTearOffMenu (this, pBar);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pWndMain);
		if (pFrame != NULL)
		{
			bTearOff = pFrame->OnTearOffMenu (this, pBar);
		}
		else	// Maybe, OLE frame...
		{
			CBCGOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pWndMain);
			if (pOleFrame != NULL)
			{
				bTearOff = pOleFrame->OnTearOffMenu (this, pBar);
			}
			else
			{
				CBCGOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGOleDocIPFrameWnd, pWndMain);
				if (pOleDocFrame != NULL)
				{
					bTearOff = pOleDocFrame->OnTearOffMenu (this, pBar);
				}
			}
		}
	}

	if (!bTearOff)
	{
		pBar->DestroyWindow ();
		delete pBar;

		return FALSE;
	}

	pBar->OnUpdateCmdUI (pWndMain, TRUE);

	pWndMain->FloatControlBar (pBar, ptScreen);
	pWndMain->RecalcLayout ();

	CBCGMiniDockFrameWnd* pFloatFrame = 
		DYNAMIC_DOWNCAST (CBCGMiniDockFrameWnd, pBar->GetParentFrame ());
	if (pFloatFrame != NULL)
	{
		return pFloatFrame->StartTearOff (this);
	}

	return FALSE;
}
//*************************************************************************************
CControlBar* CBCGPopupMenu::CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID,
											  LPCTSTR lpszName)
{
	ASSERT_VALID (pWndMain);
	ASSERT (lpszName != NULL);
	ASSERT (uiID != 0);

	if (m_hMenu == NULL)
	{
		return NULL;
	}

	CMenu* pMenu = CMenu::FromHandle (m_hMenu);
	if (pMenu == NULL)
	{
		return NULL;
	}

	CBCGToolBar* pNewToolbar = new CBCGToolBar;
	if (!pNewToolbar->Create (pWndMain,
		dwDefaultToolbarStyle,
		uiID))
	{
		TRACE0 ("Failed to create a new toolbar!\n");
		delete pNewToolbar;
		return NULL;
	}

	pNewToolbar->SetWindowText (lpszName);

	int iCount = (int) pMenu->GetMenuItemCount ();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID (i);

		CString strText;
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);

		switch (uiCmd)
		{
		case 0:
			if (i != iCount - 1)
			{
				pNewToolbar->InsertSeparator ();
			}
			break;

		case -1:
			{
				UINT uiTearOffId = 0;
				if (g_pTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pTearOffMenuManager->Parse (strText);
				}

				//---------------
				// Remove hotkey:
				//---------------
				int iTabOffset = strText.Find (_T('\t'));
				if (iTabOffset >= 0)
				{
					strText = strText.Left (iTabOffset);
				}

				CBCGToolbarMenuButton menuButton ((UINT) -1, 
					pMenu->GetSubMenu (i)->GetSafeHmenu (),
					-1, strText);
				if (menuButton.GetImage () == -1)
				{
					menuButton.m_bImage = FALSE;
					menuButton.m_bText = TRUE;
				}

				menuButton.SetTearOff (uiTearOffId);
				pNewToolbar->InsertButton (menuButton);
			}
			break;

		default:
			if (!IsStandardCommand (uiCmd))
			{
				CBCGToolbarButton button (uiCmd, -1, strText);
				if (button.GetImage () == -1)
				{
					button.m_bImage = FALSE;
					button.m_bText = TRUE;
				}

				pNewToolbar->InsertButton (button);
			}
		}
	}

	pNewToolbar->SetBarStyle (pNewToolbar->GetBarStyle () |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	pNewToolbar->EnableDocking (CBRS_ALIGN_ANY);

	return pNewToolbar;
}
//*************************************************************************************
BOOL CBCGPopupMenu::PreTranslateMessage(MSG* pMsg) 
{
	if (m_wndToolTip.GetSafeHwnd () != NULL)
	{
		m_wndToolTip.RelayEvent (pMsg);
	}
	
	return CMiniFrameWnd::PreTranslateMessage(pMsg);
}
//*************************************************************************************
BOOL CBCGPopupMenu::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	ASSERT (pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		m_bIsTearOffCaptionActive = TRUE;
		InvalidateRect (m_rectTearOffCaption);
		UpdateWindow ();
	}
	else if (pNMHDR->code == TTN_POP)
	{
		m_bIsTearOffCaptionActive = FALSE;
		InvalidateRect (m_rectTearOffCaption);
		UpdateWindow ();
	}
	
	return CMiniFrameWnd::OnNotify(wParam, lParam, pResult);
}
//**************************************************************************************
void CBCGPopupMenu::SetAnimationSpeed (UINT nElapse)
{
	if (nElapse == 0 || nElapse > 200)
	{
		ASSERT (FALSE);
		return;
	}

	m_AnimationSpeed = nElapse;
}
//**************************************************************************************
BOOL CBCGPopupMenu::NotifyParentDlg (BOOL bActivate)
{
	CBCGDialog* pDlg = DYNAMIC_DOWNCAST (CBCGDialog, m_pMessageWnd);
	CBCGPropertyPage* pPropPage = DYNAMIC_DOWNCAST (CBCGPropertyPage, m_pMessageWnd);

	if (pDlg == NULL && pPropPage == NULL)
	{
		return FALSE;
	}

	if (!bActivate && m_pActivePopupMenu != this)
	{
		return FALSE;
	}

	if (pDlg != NULL)
	{
		pDlg->SetActiveMenu (bActivate ? this : NULL);
	}

	if (pPropPage != NULL)
	{
		pPropPage->SetActiveMenu (bActivate ? this : NULL);
	}

	return TRUE;
}
//*******************************************************************************
void CBCGPopupMenu::UpdateShadow (LPRECT lprectScreen)
{
	ASSERT_VALID (this);

	if (m_iShadowSize <= 0)
	{
		// No menu shadow, nothing to do.
		return;
	}

	CWnd* pWndMain = GetTopLevelParent ();
	if (pWndMain == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndMain);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectUpdate1 = rectClient;
	CRect rectUpdate2 = rectClient;

	if (lprectScreen != NULL)
	{
		CRect rectRedraw = lprectScreen;
		ScreenToClient (&rectRedraw);

		CRect rectShadowRight = rectClient;
		rectShadowRight.left = rectShadowRight.right - m_iShadowSize - 1;

		if (!rectUpdate1.IntersectRect (rectRedraw, rectShadowRight))
		{
			rectUpdate1.SetRectEmpty ();
		}

		CRect rectShadowBottom = rectClient;
		rectShadowBottom.top = rectShadowBottom.bottom - m_iShadowSize - 1;

		if (!rectUpdate2.IntersectRect (rectRedraw, rectShadowBottom))
		{
			rectUpdate2.SetRectEmpty ();
		}

		if (rectUpdate1.IsRectEmpty () && rectUpdate2.IsRectEmpty ())
		{
			return;
		}
	}

	CRect rectMenu;
	GetWindowRect (rectMenu);

	if (m_bForceShadow)
	{
		CRect rectMain;
		pWndMain->GetWindowRect (rectMain);

		CRect rectInter;
		rectInter.UnionRect (&rectMenu, &rectMain);

		if (rectInter != rectMain)
		{
			return;
		}
	}

	int iShadowSize = m_iShadowSize;
	m_iShadowSize = 0;

	//------------------------------------------------------------
	// m_bmpShadowRight and m_bmpShadowBottom contain the previous
	// screen shots. Delete them now:
	//------------------------------------------------------------
	if (m_bmpShadowRight.GetSafeHandle () != NULL)
	{
		m_bmpShadowRight.DeleteObject ();
	}
	
	if (m_bmpShadowBottom.GetSafeHandle () != NULL)
	{
		m_bmpShadowBottom.DeleteObject ();
	}

	UINT uiSWPFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING;

	//--------------------------------------------------
	// Reduce menu size ("cut" left and bottom shadows):
	//--------------------------------------------------
	SetWindowPos (NULL, -1, -1,
		rectClient.Width () - iShadowSize, rectClient.Height () - iShadowSize,
		uiSWPFlags);

	UINT uiRDWFlags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN;

	CRect rectParent = rectUpdate1;

	if (!rectUpdate1.IsRectEmpty ())
	{
		MapWindowPoints (pWndMain, &rectParent);
		pWndMain->RedrawWindow (rectParent, NULL, uiRDWFlags);
	}

	rectParent = rectUpdate2;

	if (!rectUpdate2.IsRectEmpty () && rectUpdate1 != rectUpdate2)
	{
		MapWindowPoints (pWndMain, &rectParent);
		pWndMain->RedrawWindow (rectParent, NULL, uiRDWFlags);
	}

	pWndMain->UpdateWindow ();

	//-----------------------------------------------------------------
	// Restore original size and update windows under the menu shadows:
	//-----------------------------------------------------------------
	m_iShadowSize = iShadowSize;

	SetWindowPos (NULL, -1, -1,
		rectClient.Width (), rectClient.Height (),
		uiSWPFlags);

	if (!rectUpdate1.IsRectEmpty ())
	{
		InvalidateRect (rectUpdate1);
	}

	if (!rectUpdate2.IsRectEmpty () && rectUpdate1 != rectUpdate2)
	{
		InvalidateRect (rectUpdate2);
	}

	UpdateWindow ();
}
//*******************************************************************************
void CBCGPopupMenu::UpdateAllShadows (LPRECT lprectScreen)
{
	for (CBCGPopupMenu* pMenu = m_pActivePopupMenu; pMenu != NULL;
		pMenu = pMenu->GetParentPopupMenu ())
	{
		ASSERT_VALID (pMenu);
		pMenu->UpdateShadow (lprectScreen);
	}
}
//*******************************************************************************
void CBCGPopupMenu::SetQuickMode()
{
	//-------------------------------
	// Store recently used mode state
	//------------------------------
	CBCGCustomizeMenuButton::m_bRecentlyUsedOld = CBCGMenuBar::IsRecentlyUsedMenus();
	CBCGMenuBar::SetRecentlyUsedMenus(FALSE);

	m_bQuickCusomize = TRUE;	
}
//********************************************************************************
LRESULT CBCGPopupMenu::OnGetObject (WPARAM wParam, LPARAM lParam)
{ 	
	if (lParam != OBJID_CLIENT || !globalData.IsAccessibilitySupport ())
	{
		return (LRESULT)0L;
	}

	CWnd* pWndMain = BCGGetTopLevelFrame (this);
	ASSERT_VALID (pWndMain);

	if (pWndMain != NULL)
	{
		BCGACCDATA accData;
		accData.m_wParam = wParam;
		accData.m_lParam = lParam;
		accData.m_nObjType = ACC_POPUPMENU;
		accData.m_objData = (void*)m_pMenuPopupAcc->GetSafeHmenu ();
		accData.m_pWnd = this;
		pWndMain->SendMessage (BCGM_ACCGETOBGECT, (WPARAM)&accData);

		return globalData.GetAccObjectRes (&accData, m_pAccessible);
	}

	return (LRESULT)0L;
}
//********************************************************************************
LRESULT CBCGPopupMenu::OnAccHitTest (WPARAM /*wParam*/, LPARAM lParam)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	int xLeft = LOWORD (lParam);
	int yTop = HIWORD (lParam);
	CPoint point (xLeft, yTop);
	ScreenToClient (&point);
	int iButton = pMenuBar->HitTest(point);
	return iButton;
}
//********************************************************************************
LRESULT CBCGPopupMenu::OnAccGetState (WPARAM wParam, LPARAM /*lParam*/)
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();
	int nIndex = (int) wParam;

	if (nIndex < 0 || nIndex > pMenuBar->GetCount ())
	{
		return -1;
	}


	CBCGToolbarButton* pButton
		 = pMenuBar->GetButton (nIndex);
	return pButton->m_nStyle;
}
//********************************************************************************
BOOL CBCGPopupMenu::ExportToMenu ()
{
	CBCGPopupMenuBar* pMenuBar = GetMenuBar ();

	m_pMenuPopupAcc = new CMenu;
	if (!m_pMenuPopupAcc->CreatePopupMenu ())
	{
		TRACE(_T("Can't create popup menu!\n"));
		return FALSE;
	}

	BOOL bRes = TRUE;
	DWORD dwLastError = 0;  

	for (int i=0; i< pMenuBar->GetCount (); i++)
	{
		CBCGToolbarButton* pButton = pMenuBar->GetButton (i);
		ASSERT (pButton != NULL);

		CBCGToolbarMenuButton* pItem = DYNAMIC_DOWNCAST (CBCGToolbarMenuButton, pButton);
		if (pItem == NULL) // Maybe Separator
		{
			if (pButton->m_nID == 0)
			{
				bRes = m_pMenuPopupAcc->AppendMenu (MF_SEPARATOR);
			}
			continue;
		}

		UINT uiStyle = MF_STRING;

		if (pItem->m_nStyle & TBBS_BREAK)
		{
			uiStyle |= MF_MENUBREAK;
		}

		if (pItem->IsTearOffMenu ())
		{
			uiStyle |= MF_MENUBARBREAK;
		}

		switch (pItem->m_nID)
		{
		case 0:	// Separator
			bRes = m_pMenuPopupAcc->AppendMenu (MF_SEPARATOR);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
			break;

		case -1:			// Sub-menu
			{
				HMENU hSubMenu = pItem->CreateMenu ();
				ASSERT (hSubMenu != NULL);
				CString strText = pItem->m_strText;

				bRes = m_pMenuPopupAcc->AppendMenu (uiStyle | MF_POPUP, (UINT_PTR) hSubMenu, strText);
				if (!bRes)
				{
					dwLastError = GetLastError ();
				}

				//--------------------------------------------------------
				// This is incompatibility between Windows 95 and 
				// NT API! (IMHO). CMenu::AppendMenu with MF_POPUP flag 
				// COPIES sub-menu resource under the Win NT and 
				// MOVES sub-menu under Win 95/98 and 2000!
				//--------------------------------------------------------
				if (globalData.bIsWindowsNT4)
				{
					::DestroyMenu (hSubMenu);
				}
			}
			break;

		default:
			bRes = m_pMenuPopupAcc->AppendMenu (uiStyle, pItem->m_nID, pItem->m_strText);
			if (!bRes)
			{
				dwLastError = GetLastError ();
			}
		}

		if (!bRes)
		{
			TRACE(_T("Can't add menu item: %d\n Last error = %x\n"), pItem->m_nID, dwLastError);
			return FALSE;
		}
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPopupMenu::DestroyRecentMenu ()
{
	if (m_pMenuPopupAcc != NULL)
	{
		m_pMenuPopupAcc->DestroyMenu ();
		delete m_pMenuPopupAcc;
		m_pMenuPopupAcc = NULL;
		return TRUE;
	}
	return FALSE;
}
//**************************************************************************************
void CBCGPopupMenu::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CMiniFrameWnd::OnShowWindow(bShow, nStatus);
	
	if (!bShow)
	{
		m_bShown = FALSE;
	}
}
