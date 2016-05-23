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
// BCGAppBarWnd.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGAppBarWnd.h"
#include "Globals.h"
#include "BCGVisualManager.h"
#include "RegPath.h"
#include "BCGRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CBCGAppBarWnd,CWnd,VERSIONABLE_SCHEMA | 2)

// registered message for the AppBar's callback notifications
UINT BCGM_APPBAR_CALLBACK	= ::RegisterWindowMessage (_T("BCGM_APPBAR_CALLBACK"));
UINT CBCGAppBarWnd::m_uAppBarNotifyMsg = BCGM_APPBAR_CALLBACK;

static const CString strAppBarsProfile = _T ("BCGAppBars");

#define REG_SECTION_FMT	_T("%sBCGAppBar-%d")

/////////////////////////////////////////////////////////////////////////////
// CBCGAppBarWnd

CBCGAppBarWnd::CBCGAppBarWnd()
{
	m_bAppRegistered = FALSE;
	m_bAppAutoHiding = FALSE;
	m_bHidden = FALSE;
	m_bDocked = FALSE;

	m_abs.m_bAutoHide = FALSE;
	m_abs.m_bAlwaysOnTop = FALSE;
	m_abs.m_uSide = ABE_TOP;
	m_abs.m_auDimsDock [0] = CX_DEFWIDTH;
	m_abs.m_auDimsDock [1] = CY_DEFHEIGHT;
	m_abs.m_auDimsDock [2] = CX_DEFWIDTH;
	m_abs.m_auDimsDock [3] = CY_DEFHEIGHT;
	m_abs.m_rcFloat.SetRectEmpty ();

	m_bDisableAnimation = FALSE;
	m_nCaptionHeight = globalData.GetTextHeight () + 4;
	m_hEmbeddedBar	 = NULL;
	m_dwFlags = ABF_ALLOWANYWHERE | ABF_ALLOWAUTOHIDE | ABF_ALLOWALWAYSONTOP;

	m_bMoving = FALSE;
	m_rectDrag.SetRectEmpty ();
	m_uSidePrev = m_abs.m_uSide;
	m_bSizing = FALSE;
	m_rectPrev.SetRectEmpty ();
	m_bAllowResize = FALSE;

	m_bDesktopChanging = FALSE;
}

CBCGAppBarWnd::~CBCGAppBarWnd()
{
	UnRegister ();
}

BEGIN_MESSAGE_MAP(CBCGAppBarWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGAppBarWnd)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_CANCELMODE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	ON_WM_NCHITTEST()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(m_uAppBarNotifyMsg, AppBarCallback)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE (WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_MESSAGE (WM_EXITSIZEMOVE, OnExitSizeMove)
END_MESSAGE_MAP()

BOOL CBCGAppBarWnd::Create (LPCTSTR lpszWindowName, CRect rect, CWnd* pParentWnd)
{
	ASSERT_VALID (this);

	if (globalData.m_hcurSizeAll == NULL)
	{
		globalData.m_hcurSizeAll = AfxGetApp ()->LoadStandardCursor (IDC_SIZEALL);
	}

	CString strClassName;
	
	//-----------------------------
	// Register a new window class:
	//-----------------------------
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT uiClassStyle = CS_SAVEBITS;
	HCURSOR hCursor = ::LoadCursor (NULL, IDC_ARROW);
	HBRUSH hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

	strClassName.Format (_T("BCGAppBar:%x:%x:%x:%x"), 
		(UINT_PTR)hInst, uiClassStyle, (UINT_PTR)hCursor, (UINT_PTR)hbrBackground);

	//---------------------------------
	// See if the class already exists:
	//---------------------------------
	WNDCLASS wndcls;
	if (::GetClassInfo (hInst, strClassName, &wndcls))
	{
		//-----------------------------------------------
		// Already registered, assert everything is good:
		//-----------------------------------------------
		ASSERT (wndcls.style == uiClassStyle);
	}
	else
	{
		//--------------------------------
		// Otherwise register a new class:
		//--------------------------------
		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = 0;
		wndcls.cbWndExtra = sizeof(LPVOID);
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = hCursor;
		wndcls.hbrBackground = hbrBackground;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = strClassName;
		
		if (!AfxRegisterClass (&wndcls))
		{
			AfxThrowResourceException();
		}
	}

	CString strWindowName;
	if (lpszWindowName == NULL)
	{
		strWindowName = _T("AppBar");
	}
	else
	{
		strWindowName = lpszWindowName;
	}

	//---------------------
	// Create a new window:
	//---------------------
	if (!CreateEx (WS_EX_TOOLWINDOW,
				  strClassName, strWindowName,
				  WS_POPUP | WS_OVERLAPPED | WS_CLIPCHILDREN,
				  rect, pParentWnd, NULL))
	{
		return FALSE;
	}

	// ----------------------------------------------------------------
	// Save pointer to APPBARSTATE struct into the extra window memory:
	// ----------------------------------------------------------------
	SetWindowLongPtr (GetSafeHwnd (), 0, (LONG_PTR) &m_abs);

	// ----------------------------------------------------------
	// Register the appbar and attach it to the right by default:
	// ----------------------------------------------------------
	if (!Register ())
	{
		TRACE0 ("Can't register application bar.\n");
		return FALSE;
	}
	SetSide(ABE_RIGHT);

	return TRUE;
}
//---------------------------------------------------------------------------------------------
// Registers the appbar with the system.
BOOL CBCGAppBarWnd::Register ()
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	if (hwnd == NULL)
	{
		return FALSE;
	}

	// Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
	abd.uCallbackMessage = m_uAppBarNotifyMsg;

	// Register an appbar
	m_bAppRegistered = (BOOL) SHAppBarMessage(ABM_NEW, &abd);
	return m_bAppRegistered;
}
//---------------------------------------------------------------------------------------------
// Unregisters the appbar with the system.
BOOL CBCGAppBarWnd::UnRegister ()
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	if (hwnd == NULL)
	{
		return FALSE;
	}

	m_bDesktopChanging = TRUE;

	// Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;

	// Register an appbar
	m_bAppRegistered = !SHAppBarMessage(ABM_REMOVE, &abd);

	m_bDesktopChanging = FALSE;

	return !m_bAppRegistered;

	// might set previous size and location for floating mode
}
//---------------------------------------------------------------------------------------------
// Asks the system if the AppBar can occupy the rectangle specified
// in rect. The system will change the rect rectangle to make
// it a valid rectangle on the desktop.
void CBCGAppBarWnd::QueryPos (CRect &rect)
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

	// Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
	abd.uEdge = m_abs.m_uSide;
	abd.rc = rect;

    // Ask the system for the screen space
	SHAppBarMessage(ABM_QUERYPOS, &abd);
	rect = abd.rc;	
}
//---------------------------------------------------------------------------------------------
// Request the system to set the size and screen position of an Appbar.
// The system might change the rect rectangle to make 
// it a valid rectangle on the desktop.
void CBCGAppBarWnd::SetPos (CRect &rect)
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

	// Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
	abd.uEdge = m_abs.m_uSide;
	abd.rc = rect;

    // Tell the system we're moving to this new approved position.
	SHAppBarMessage(ABM_SETPOS, &abd);
	rect = abd.rc;	
}
//---------------------------------------------------------------------------------------------
// Retrieves the state of the appbar's autohide and always-on-top attributes.
DWORD CBCGAppBarWnd::GetState ()
{
	ASSERT_VALID (this);

    // Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);

    // Get appbar state
	return (DWORD) SHAppBarMessage(ABM_GETSTATE, &abd);
}
//---------------------------------------------------------------------------------------------
// Retrieves the handle to the autohide appbar associated with an edge of the screen.
// uEdge can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT.
HWND CBCGAppBarWnd::GetAutoHideBar (UINT uEdge)
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

    // Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
	abd.uEdge = uEdge;

    // Get auto hide bar
	return (HWND) SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
}
//---------------------------------------------------------------------------------------------
// Registers or unregisters an autohide appbar for an edge of the screen.
// uEdge can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT.
BOOL CBCGAppBarWnd::SetAutoHideBar (UINT uEdge, BOOL bRegister)
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

    // Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
	abd.uEdge = uEdge;
	abd.lParam = (LPARAM) bRegister;

    // Ask the system to register/unregister an autohide bar at the edge
	return (BOOL) SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);
}
//---------------------------------------------------------------------------------------------
// Retrieves the bounding rectangle of the Windows appbar.
void CBCGAppBarWnd::GetPos (CRect& rect)
{
	ASSERT_VALID (this);

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

    // Prepare data structure of message
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = hwnd;
       
    // Get appbar position
	SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
    rect = abd.rc;
}
//---------------------------------------------------------------------------------------------
// Move the AppBar to the specified edge on the desktop.
// The rect rectangle will be validated with the system and 
// adjusted for the specified edge.
// The edge we're moving to and the bar's width/height 
// will be saved in the appbar APPBARSTATE struct.
// uEdge can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT.
// If bNoSizeMove - TRUE, prevent resizing the desktop.
void CBCGAppBarWnd::DoSetPos (UINT uEdge, CRect& rect, BOOL bMove, BOOL bNoSizeMove)
{
	ASSERT_VALID (this);
	ASSERT (m_bAppRegistered);
	m_bDesktopChanging = TRUE;

	if (m_bAppRegistered)
	{
		m_abs.m_uSide = uEdge;

		// Ask the system whether rect is valid
		// and update it if necessary.
		DoQueryPos (rect);

		// Tell the system we're moving to this new approved position.
		if (!bNoSizeMove)
		{
			SetPos (rect);
		}

   		// Move the appbar window to the new position
		if (bMove)
		{
			SetWindowPos (NULL, rect.left, rect.top,
				rect.Width (), rect.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
		}

		// Save the appbar rect.  We use this later when we autohide.
		// If we're currently hidden, then don't mess with this.
		if (!m_bAppAutoHiding || m_bAllowResize)
		{
			m_rectPrev = rect;
			switch (uEdge)
			{
			case ABE_LEFT:
				m_abs.m_auDimsDock [0] = rect.Width ();
				break;
			case ABE_TOP:
				m_abs.m_auDimsDock [1] = rect.Height ();
				break;
			case ABE_RIGHT:
				m_abs.m_auDimsDock [2] = rect.Width ();
				break;
			case ABE_BOTTOM:
				m_abs.m_auDimsDock [3] = rect.Height ();
				break;
			}
		}
	}

	m_bDesktopChanging = FALSE;
}
//---------------------------------------------------------------------------------------------
// Asks the system if the AppBar can occupy the rectangle specified
// in rect. The system will change the rect rectangle to make
// it a valid rectangle on the desktop.
// The edge we're moving to is specified in the appbar APPBARSTATE struct
void CBCGAppBarWnd::DoQueryPos (CRect& rect)
{
	ASSERT_VALID (this);
	ASSERT (m_bAppRegistered);

	if (m_bAppRegistered)
	{
		CRect rectScreen;
		GetScreenRect (rectScreen);

		// Calculate the part we want to occupy.  We only figure out the top
		// and bottom coordinates if we're on the top or bottom of the screen.
		// Likewise for the left and right.  We will always try to occupy the
		// full height or width of the screen edge.
		int iWidth = 0;
		int iHeight = 0;
		if ((ABE_LEFT == m_abs.m_uSide) || (ABE_RIGHT == m_abs.m_uSide))
		{
			iWidth = rect.right - rect.left;
			rect.top = 0;
			rect.bottom = rectScreen.Height ();
		}
		else
		{
			iHeight = rect.bottom - rect.top;
			rect.left = 0;
			rect.right = rectScreen.Width ();
		}

		// Ask the system for the screen space
		QueryPos (rect);

		// The system will return an approved position along the edge we're asking
		// for.  However, if we can't get the exact position requested, the system
		// only updates the edge that's incorrect.  For example, if we want to 
		// attach to the bottom of the screen and the taskbar is already there, 
		// we'll pass in a rect like 0, 964, 1280, 1024 and the system will return
		// 0, 964, 1280, 996.  Since the appbar has to be above the taskbar, the 
		// bottom of the rect was adjusted to 996.  We need to adjust the opposite
		// edge of the rectangle to preserve the height we want.

		switch (m_abs.m_uSide)
		{
		case ABE_LEFT:
			rect.right = rect.left + iWidth;
			break;

		case ABE_RIGHT:
			rect.left = rect.right - iWidth;
			break;

		case ABE_TOP:
			rect.bottom = rect.top + iHeight;
			break;

		case ABE_BOTTOM:
			rect.top = rect.bottom - iHeight;
			break;
		}
	}
}
//---------------------------------------------------------------------------------------------
// Dock the appbar to the screen edge.
// uSide can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT
// If bNoSizeMove - TRUE, prevent resizing the desktop and moving the window.
BOOL CBCGAppBarWnd::DoSetSide (UINT uSide, BOOL bMove, BOOL bNoSizeMove)
{
	if (!m_bAppRegistered)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	// Calculate the size of the screen so we can occupy the full width or
	// height of the screen on the edge we request.
	CRect rect;
	GetScreenRect (rect);

	// Adjust the rectangle to set our height or width depending on the
	// side we want.
	switch (uSide)
	{
	case ABE_LEFT:
		rect.right = rect.left + m_abs.m_auDimsDock [0];
		break;
	case ABE_TOP:
		rect.bottom = rect.top + m_abs.m_auDimsDock [1];
		break;
	case ABE_RIGHT:
		rect.left = rect.right - m_abs.m_auDimsDock [2];
		break;
	case ABE_BOTTOM:
		rect.top = rect.bottom - m_abs.m_auDimsDock [3];
		break;
	default:
		ASSERT (FALSE);
		return FALSE;
	}

	// Move the appbar to the new screen space.
	DoSetPos (uSide, rect, bMove, bNoSizeMove);
	m_bDocked = TRUE;

	return TRUE;
}
//---------------------------------------------------------------------------------------------
// Sets the side the AppBar is currently attached to.
// uSide can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, ABE_RIGHT, ABE_FLOAT, or ABE_UNKNOWN.
BOOL CBCGAppBarWnd::SetSide (UINT uSide)
{
	ASSERT_VALID (this);

	switch (uSide)
	{
	case ABE_LEFT:
	case ABE_RIGHT:
		// Check if this side is allowed
		if ((m_dwFlags & ABF_ALLOWLEFTRIGHT) == 0)
		{
			return FALSE;
		}
		break;
	case ABE_TOP:
	case ABE_BOTTOM:
		// Check if this side is allowed
		if ((m_dwFlags & ABF_ALLOWTOPBOTTOM) == 0)
		{
			return FALSE;
		}
		break;

	case ABE_FLOAT:
		// Check if floating is allowed
		if ((m_dwFlags & ABF_ALLOWFLOAT) == 0)
		{
			return FALSE;
		}

		// Unregister the appbar and restore its size/position 
		// according to the float rect, which is saved in the APPBARSTATE structure
		return Float ();

	case ABE_UNKNOWN:
	default:
		return FALSE;
	}

	// -----------------------------------
	// Dock the appbar to the screen edge.
	// -----------------------------------
	if (!m_bAppRegistered)
	{
		// Register the window with the system as an application toolbar
		Register ();
	}
	if (m_bAppRegistered)
	{
		// the appbar was floating or forced to be docked - restore autohiding
		BOOL bRestoreAutoHiding = !m_bAppAutoHiding && m_abs.m_bAutoHide;

		// ----------------------------
		// Unhide the autohidden appbar
		// ----------------------------
		BOOL bAutoHide = m_bAppAutoHiding;
		if (bAutoHide)
		{
			// If the appbar is autohidden, unhide it so we can move the bar
			// m_bAutoHide state flag keeps unchanged in APPBARSTATE structure 
			// so one can restore it later.
			DoSetAutoHide (FALSE, FALSE, TRUE);
		}

		// ----------------------------------------
		// Move the appbar to the new screen space.
		// ----------------------------------------
		DoSetSide (uSide, TRUE, bAutoHide || bRestoreAutoHiding);

		// -----------------
		// Rehide the appbar
		// -----------------
		if (bAutoHide ||			// If the appbar was hidden, rehide it now
			bRestoreAutoHiding)		// The appbar was floating - restore autohiding
		{
			if (!DoSetAutoHide (TRUE, FALSE))
			{
				// Failed to set autohidebar - so update the screen space anyway
				DoSetSide (uSide, FALSE);
			}
		}

		return TRUE;
	}

	return FALSE;
}
//---------------------------------------------------------------------------------------------
// Handles updating the appbar's size and position.
void CBCGAppBarWnd::OnSizeMove ()
{
	ASSERT_VALID (this);

	if (!m_bAppRegistered)
	{
		return;
	}

	if (!m_bAppAutoHiding || !m_bHidden)
	{
		// Calculate size
		CRect rectWindow;
		GetWindowRect(rectWindow);

		CSize size = rectWindow.Size ();

		// Calculate the size of the screen so we can occupy the full width or
		// height of the screen on the edge we request.
		CRect rect;
		GetScreenRect (rect);

		// Adjust the rectangle to set our height or width depending on the
		// side we want.
		switch (m_abs.m_uSide)
		{
		case ABE_TOP:
			rect.bottom = rect.top + size.cy;
			break;
		case ABE_BOTTOM:
			rect.top = rect.bottom - size.cy;
			break;
		case ABE_LEFT:
			rect.right = rect.left + size.cx;
			break;
		case ABE_RIGHT:
			rect.left = rect.right - size.cx;
			break;
		}

		// Move the appbar to the new screen space.
		if (!m_rectPrev.EqualRect (rect))
		{
			m_bAllowResize = TRUE;

			DoSetPos (m_abs.m_uSide, rect, TRUE, m_bAppAutoHiding);

			m_bAllowResize = FALSE;
		}
	}

}
//---------------------------------------------------------------------------------------------
BOOL CBCGAppBarWnd::Float ()
{
	ASSERT_VALID (this);
	
	// Check if floating is allowed
	if ((m_dwFlags & ABF_ALLOWFLOAT) == 0)
	{
		return FALSE;
	}

	if (m_bAppRegistered)
	{
		// If the appbar is autohidden, unhide it so we can move the bar
		// m_bAutoHide state flag keeps unchanged in APPBARSTATE structure 
		// so one can restore it later.
		if (m_bAppAutoHiding)
		{
			DoSetAutoHide (FALSE, FALSE, TRUE);
		}

		// Unregister the appbar
		UnRegister ();
		m_bDocked = FALSE;
		m_bHidden = FALSE;
	}

	// Restore size/position for the floating mode saved in the APPBARSTATE structure
	m_abs.m_uSide = ABE_FLOAT;

	SetWindowPos (NULL, m_abs.m_rcFloat.left, m_abs.m_rcFloat.top,
		m_abs.m_rcFloat.Width (), m_abs.m_rcFloat.Height (),
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);

	return TRUE;
}
//---------------------------------------------------------------------------------------------
BOOL CBCGAppBarWnd::Float (CRect rect)
{
	m_abs.m_rcFloat = rect;
	return Float ();
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::Hide ()
{
	// The appbar should be registered as autohide bar
	if (m_bAppRegistered && m_bAppAutoHiding)
	{

		// Calculate a hidden rectangle to use
		CRect rect = m_rectPrev;
		switch (m_abs.m_uSide)
		{
		case ABE_TOP:
			rect.bottom = rect.top + 2; 
			break;
		case ABE_BOTTOM:
			rect.top = rect.bottom - 2;
			break;
		case ABE_LEFT:
			rect.right = rect.left + 2;
			break;
		case ABE_RIGHT:
			rect.left = rect.right - 2;
			break;
		}

		m_bHidden = TRUE;
		SlideWindow(&rect);
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::UnHide ()
{
	if (m_bAppRegistered)
	{
		// Don't want to unhide if AutoHide is not set
		if (!m_bAppAutoHiding)
		{
			ASSERT (FALSE);
			TRACE (_T("The appbar is not hidden currently.\r\n"));
			return;
		}

		// Change Z-order to be over the first top-level window, but not over the topmost
		if (!m_abs.m_bAlwaysOnTop)
		{
			CWnd* pWndTop = GetWindow(GW_HWNDFIRST);
			SetWindowPos (pWndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		SlideWindow(&m_rectPrev);
		m_bHidden = FALSE;

		if (m_bAppAutoHiding)
		{
			SetTimer(AUTOHIDE_TIMER_ID, AUTOHIDE_TIMER_INTERVAL, NULL);
		}
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::SetAlwaysOnTop (BOOL bEnable)
{
	// Check if always-on-top is allowed
	if ((m_dwFlags & ABF_ALLOWALWAYSONTOP) == 0)
	{
		return;
	}

	m_abs.m_bAlwaysOnTop = bEnable;

	::SetWindowPos (GetSafeHwnd (), m_abs.m_bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
					0, 0, 0, 0, 
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------------------------
BOOL CBCGAppBarWnd::SetAutoHide (BOOL bEnable)
{
	// Check if autohiding is allowed
	if ((m_dwFlags & ABF_ALLOWAUTOHIDE) == 0)
	{
		return FALSE;
	}

	m_abs.m_bAutoHide = bEnable;

	// Enable/disable autohiding
	return (m_bAppRegistered ? DoSetAutoHide (bEnable, !bEnable) : FALSE);
}
//---------------------------------------------------------------------------------------------
// Does the work of changing the appbar to autohide or to no-autohide
// If bNoSizeMove TRUE, prevent resizing the desktop.
BOOL CBCGAppBarWnd::DoSetAutoHide (BOOL bEnable, BOOL bMove, BOOL bNoSizeMove)
{
	if (!m_bAppRegistered)
	{
		ASSERT (FALSE); // should be docked
		return FALSE;
	}

	// -----------------------------------------------
	// Enable autohiding. Register an autohide appbar.
	// -----------------------------------------------
	if (bEnable)
	{
		// Check the Autohide state to be allowed
		if (m_abs.m_bAutoHide && (m_dwFlags & ABF_ALLOWAUTOHIDE) == 0)
		{
			TRACE(_T("Cannot set to autohide. Autohide state is not allowed.\r\n"));
			ASSERT (FALSE);
			return FALSE;
		}

		// Figure out if someone already has this side for autohiding
		HWND hwndAutoHide = GetAutoHideBar (m_abs.m_uSide);
		if (hwndAutoHide != NULL)
		{
			TRACE(_T("Cannot set to autohide. Another appbar is already hiding on this edge.\r\n"));
			return FALSE;
		}


		if (!SetAutoHideBar (m_abs.m_uSide, bEnable))
		{
			TRACE(_T("Cannot set to autohide. Error trying to set autohidebar.\r\n"));
			return FALSE;
		}
		else
		{
			m_bAppAutoHiding = TRUE;
			
			// ------------------------------------------------------
			// Adjust our screen rectangle to the autohidden position.  
			// This will allow the system to resize the desktop.
			// ------------------------------------------------------

			// Calculate the size of the screen so we can occupy the full width or
			// height of the screen on the edge we request.
			CRect rect;
			GetScreenRect (rect);

			// Adjust the rectangle to set our height or width depending on the
			// side we want.
			switch (m_abs.m_uSide)
			{
			case ABE_LEFT:
				rect.right = rect.left + 2;
				break;
			case ABE_TOP:
				rect.bottom = rect.top + 2;
				break;
			case ABE_RIGHT:
				rect.left = rect.right - 2;
				break;
			case ABE_BOTTOM:
				rect.top = rect.bottom - 2;
				break;
			default:
				ASSERT (FALSE);
				return FALSE;
			}
			
			// Move the appbar to the new screen space.
			DoSetPos (m_abs.m_uSide, rect, bMove, bNoSizeMove);

			// -----------
			// Init timer:
			// -----------
			SetTimer (AUTOHIDE_TIMER_ID, AUTOHIDE_TIMER_INTERVAL, NULL);
		}
	}

	// -------------------------------------------------------------
	// Disable autohiding and re-register the appbar with the system
	// -------------------------------------------------------------
	else
	{
		// Check if the appbar already has this side for autohiding
		HWND hwndAutoHide = GetAutoHideBar (m_abs.m_uSide);
		if (hwndAutoHide != GetSafeHwnd () || !m_bAppAutoHiding)
		{
			TRACE(_T("Cannot set to autohide. The appbar is not hidden currently.\r\n"));
			return FALSE;
		}

		if (!SetAutoHideBar (m_abs.m_uSide, bEnable))
		{
			TRACE(_T("Cannot set to autohide. Error trying to set autohidebar.\r\n"));
			return FALSE;
		}
		else
		{
			// Re-registers the appbar with the system
			m_bAppAutoHiding = FALSE;
			m_bHidden = FALSE;
			DoSetSide (m_abs.m_uSide, bMove, bNoSizeMove);
		}
	}

	return TRUE;
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::GetScreenRect (CRect &rectScreen) const
{
	rectScreen.SetRect (0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}
//---------------------------------------------------------------------------------------------
// Retrieves a pointer to the APPBARSTATE structure stored in the window's extra bytes.
//      hwnd    - Handle of the window to retrieve the pointer from.
//      Returns a pointer to an APPBARSTATE struct
CBCGAppBarWnd::PAPPBARSTATE CBCGAppBarWnd::GetAppbarState(HWND hwnd)
{
	return (PAPPBARSTATE) GetWindowLongPtr (hwnd, 0);
}


/////////////////////////////////////////////////////////////////////////////
// CBCGAppBarWnd message handlers

void CBCGAppBarWnd::OnDestroy() 
{
	// Make sure the appbar is unregistered
	if (m_bAppRegistered)
	{
		UnRegister ();
	}

	CWnd::OnDestroy();
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	if (m_bAppRegistered)
	{
		HWND hwnd = GetSafeHwnd ();
		ASSERT (hwnd != NULL);

		// ----------------------------------------------
		// Always send the activate message to the appbar
		// ----------------------------------------------
		APPBARDATA abd;
		ZeroMemory (&abd, sizeof (APPBARDATA));
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hwnd;
		abd.lParam = 0;                
		SHAppBarMessage(ABM_ACTIVATE, &abd);

		// Determine if we're getting or losing activation
		switch (nState)
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			/*			
			// If we're gaining activation, make sure we're visible
			if (m_bAppAutoHiding && m_bHidden)
			{
				UnHide ();
			}
			*/
			break;

		case WA_INACTIVE:
			// If we're losing activation, check to see if we need to autohide.
			if (m_bAppAutoHiding)
			{
				SetTimer(AUTOHIDE_TIMER_ID, AUTOHIDE_TIMER_INTERVAL, NULL);
			}
			break;
		}
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CWnd::OnWindowPosChanged(lpwndpos);
	
	if (m_bAppRegistered)
	{
		HWND hwnd = GetSafeHwnd ();
		ASSERT (hwnd != NULL);
		
		// ------------------------------------------------------
		// Always send the windowposchanged message to the system 
		// ------------------------------------------------------
		APPBARDATA abd;
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hwnd;
		SHAppBarMessage(ABM_WINDOWPOSCHANGED, &abd);
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	if (m_bAppRegistered)
	{
		if (!m_bSizing && !m_bMoving)
		{
			OnSizeMove ();
		}
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (m_bAppRegistered)
	{
		if (!m_bSizing && !m_bMoving)
		{
			OnSizeMove ();
		}
	}
}
//---------------------------------------------------------------------------------------------
LRESULT CBCGAppBarWnd::OnEnterSizeMove(WPARAM,LPARAM)
{
	ASSERT_VALID (this);

	if (m_bAppRegistered)
	{
		if (!m_bMoving)
		{
			CRect rect;
			GetWindowRect (rect);

			m_rectPrev = rect;
			m_bSizing = TRUE;
		}
	}

	return 0;
}
//---------------------------------------------------------------------------------------------
LRESULT CBCGAppBarWnd::OnExitSizeMove(WPARAM,LPARAM)
{
	ASSERT_VALID (this);

	if (m_bAppRegistered)
	{
		CRect rect;
		GetWindowRect (rect);

		if (m_bSizing && !m_rectPrev.EqualRect (rect))
		{
			m_bSizing = FALSE;

			OnSizeMove ();
		}
	}
	else
	{
		// update saved window rect
		if (!m_bDocked)
		{
			CRect rect;
			GetWindowRect (rect);

			m_abs.m_rcFloat = rect;
		}
	}

	return 0;
}
//---------------------------------------------------------------------------------------------
BCGNcHitTestType CBCGAppBarWnd::OnNcHitTest(CPoint point) 
{
	ASSERT_VALID (this);

	BCGNcHitTestType uHitTest = HitTest (point, TRUE);

	BOOL bPrimaryMouseBtnDown = 
		(GetAsyncKeyState(::GetSystemMetrics(SM_SWAPBUTTON) 
		 ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0;

	if ((uHitTest == HTCAPTION) && bPrimaryMouseBtnDown)
	{
		uHitTest = HTCLIENT;
	}

	if (m_bAppRegistered)
	{
		// -------------------------------------------------------
		// Disable sizing in all directions except the inside edge
		// -------------------------------------------------------
		if ((m_abs.m_uSide == ABE_TOP) && (uHitTest == HTBOTTOM))
		{
			return HTBOTTOM;
		}
		else if ((m_abs.m_uSide == ABE_BOTTOM) && (uHitTest == HTTOP))
		{
			return HTTOP;
		}
		else if ((m_abs.m_uSide == ABE_LEFT) && (uHitTest == HTRIGHT))
		{
			return HTRIGHT;
		}
		else if ((m_abs.m_uSide == ABE_RIGHT) && (uHitTest == HTLEFT))
		{
			return HTLEFT;
		}
		else
		{
			return HTCLIENT;
		}
	}
	
	return uHitTest;
}
//---------------------------------------------------------------------------------------------
// Callback for appbar notifcations
LRESULT CBCGAppBarWnd::AppBarCallback (WPARAM wParam, LPARAM lParam)
{
	switch (wParam) // notify message
	{
		// Notifies the appbar that the taskbar's autohide or always-on-top 
		// state has changed.  The appbar can use this to conform to the settings
		// of the system taskbar.
		case ABN_STATECHANGE:
			{
			DWORD dwState = GetState ();
			OnAppBarStateChange ((dwState & ABS_AUTOHIDE), (dwState & ABS_ALWAYSONTOP));
			}
			break;

        // Notifies the appbar when a full screen application is opening or 
        // closing.  When a full screen app is opening, the appbar must drop
        // to the bottom of the Z-Order.  When the app is closing, we should 
        // restore our Z-order position.
		case ABN_FULLSCREENAPP: 
			OnAppBarFullScreenApp((BOOL) lParam); 
			break;

        // Notifies the appbar when an event has occured that may effect the 
        // appbar's size and position.  These events include changes in the 
        // taskbar's size, position, and visiblity as well as adding, removing,
        // or resizing another appbar on the same side of the screen.
		case ABN_POSCHANGED:
			OnAppBarPosChanged(); 
			break;

		// Notifies an appbar that the user has selected the Cascade, 
		// Tile Horizontally, or Tile Vertically command from the 
		// taskbar's shortcut menu.
		case ABN_WINDOWARRANGE:
			OnAppBarWindowArrange((BOOL) lParam); 
			break;
	}

   return 0;
}
//---------------------------------------------------------------------------------------------
// Handles updating the appbar's size and position.
void CBCGAppBarWnd::OnAppBarPosChanged ()
{
	if (!m_bSizing && !m_bMoving)
	{
		OnSizeMove ();
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnAppBarFullScreenApp (BOOL bOpen)
{
	ASSERT_VALID (this);

 	static HWND hwndZOrder = NULL;

	HWND hwnd = GetSafeHwnd ();
	ASSERT (hwnd != NULL);

	// A full screen app is opening. 
	// Move us to the bottom of the Z-Order.  
	if (bOpen) 
	{
		// First get the window that we're underneath so we can correctly
		// restore our position
		hwndZOrder = ::GetWindow (hwnd, GW_HWNDPREV);

		// Now move ourselves to the bottom of the Z-Order
		::SetWindowPos (hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);            
	} 

	// The app is closing. 
	// Restore the Z-order			   
	else 
	{
		::SetWindowPos (hwnd, m_abs.m_bAlwaysOnTop ? HWND_TOPMOST : hwndZOrder,
						0, 0, 0, 0, 
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		hwndZOrder = NULL;
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnAppBarWindowArrange (BOOL)
{
	// This notification is sent when the user commands the shell to rearrange 
	// the opened windows (Tile, Cascade). This notification is sent twice. 
	// Once before the arrangement and once after the arrangement. 
	// According to the bBeginning you can tell whether it is before or after 
	// the arrangement.

	// A normal response to this message is to hide our window 
	// before the arrangement and to show our window after the arrangement,
	// so our window will not participate in the arrangement and hold its position.
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnAppBarStateChange (BOOL bAutoHide, BOOL bAlwaysOnTop)
{
	ASSERT_VALID (this);

	// If state was changed, update our window accordingly
	if (m_abs.m_bAutoHide != bAutoHide)
	{
		SetAutoHide (bAutoHide);
	}

	if (m_abs.m_bAlwaysOnTop != bAlwaysOnTop)
	{
		SetAlwaysOnTop (bAlwaysOnTop);
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetWindowRect (rect);

	// An user is beginning to drag the appbar around the screen
	m_bMoving = TRUE;
	m_rectDrag = rect;
	m_ptDragBegin = point;
	if (m_bAppRegistered)
	{
		m_uSidePrev = m_abs.m_uSide;
	}

	if (m_bAppAutoHiding)
	{
		KillTimer (AUTOHIDE_TIMER_ID);
	}

	SetCapture();
	::SetCursor (globalData.m_hcurSizeAll);

	CWnd::OnLButtonDown(nFlags, point);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	if (m_bMoving)
	{
		// ---------------------
		// Calc screen rectangle
		// ---------------------
		CRect rectScreen;

		CPoint ptCursor = point;
		ClientToScreen (&ptCursor);

		MONITORINFO mi;
		mi.cbSize = sizeof (MONITORINFO);
		if (GetMonitorInfo (MonitorFromPoint (ptCursor, MONITOR_DEFAULTTONEAREST), &mi))
		{
			rectScreen = mi.rcWork;
		}
		else
		{
			::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
		}

		UINT uSide = ABE_UNKNOWN;

		// ---------------------------------------------------
		// Determine whether the appbar should be float or not
		// ---------------------------------------------------
		if ((m_dwFlags & ABF_ALLOWFLOAT) != 0) // allowed
		{
			// Leave a 1/2 width/height-of-a-scrollbar gutter around the workarea
			CRect rect = rectScreen;
			rect.InflateRect(-::GetSystemMetrics(SM_CXVSCROLL), -::GetSystemMetrics(SM_CYHSCROLL));

			// If the point is in the adjusted workarea
			// OR no edges are allowed
			// OR user holds Ctrl
			if (rect.PtInRect(ptCursor) || ((m_dwFlags & ABF_ALLOWANYEDGE) == 0) ||
				(::GetAsyncKeyState (VK_CONTROL) & 0x8000))
			{
				uSide = ABE_FLOAT;
			}
		}

		// -----------------------------------------
		// Try to dock the appbar to the screen edge
		// -----------------------------------------
		if (uSide != ABE_FLOAT)
		{
			CRect rectScreen;
			GetScreenRect (rectScreen);
			int nCXScreen = rectScreen.Width ();
			int nCYScreen = rectScreen.Height ();
			
			// --------------------------------------------------
			// Find out which edge of the screen we're closest to
			// --------------------------------------------------
			int nWidth;
			UINT uHorz;
			if (ptCursor.x < (nCXScreen / 2))
			{
				nWidth = ptCursor.x;
				uHorz = ABE_LEFT;
			}
			else
			{
				nWidth = nCXScreen - ptCursor.x;
				uHorz = ABE_RIGHT;
			}

			int nHeight;
			UINT uVert;
			if (ptCursor.y < (nCYScreen / 2))
			{
				nHeight = ptCursor.y;
				uVert = ABE_TOP;
			}
			else
			{
				nHeight = nCYScreen - ptCursor.y;
				uVert = ABE_BOTTOM;
			}

			// -------------------------------------------------------
			// Build a drag rectangle based on the edge of the screen
			// that we're closest to.
			// -------------------------------------------------------
			if ((nCXScreen * nHeight) > (nCYScreen * nWidth) &&
				(m_dwFlags & ABF_ALLOWLEFTRIGHT) != 0)
			{
				m_rectDrag.top = 0;
				m_rectDrag.bottom = nCYScreen;
				if (uHorz == ABE_LEFT)
				{
					m_rectDrag.left = 0;
					m_rectDrag.right = m_rectDrag.left + m_abs.m_auDimsDock [0];
					uSide = ABE_LEFT;
				}
				else
				{
					m_rectDrag.right = nCXScreen;
					m_rectDrag.left = m_rectDrag.right - m_abs.m_auDimsDock [2];
					uSide = ABE_RIGHT;
				}
			}
			else if ((m_dwFlags & ABF_ALLOWTOPBOTTOM) != 0)
			{
				m_rectDrag.left = 0;
				m_rectDrag.right = nCXScreen;
				if (uVert == ABE_TOP)
				{
					m_rectDrag.top = 0;
					m_rectDrag.bottom = m_rectDrag.top + m_abs.m_auDimsDock [1];
					uSide = ABE_TOP;
				}
				else
				{
					m_rectDrag.bottom = nCYScreen;
					m_rectDrag.top = m_rectDrag.bottom - m_abs.m_auDimsDock [3];
					uSide = ABE_BOTTOM;
				}
			}
			else
			{
				uSide = ABE_FLOAT;
			}
		}


		switch (uSide)
		{
		case ABE_LEFT:
		case ABE_RIGHT:
		case ABE_TOP:
		case ABE_BOTTOM:
			if (!m_bAppRegistered)
			{
				// Register the window with the system as an application toolbar
				Register ();
			}
			if (!m_bDocked)
			{
				// Save cursor position for floating mode
				m_ptDragBegin = point;
			}
			
			// The appbar should be docked
			// It stays undocked untill WM_LBUTTONUP
			m_bDocked = TRUE;
			break;

		case ABE_FLOAT:
			{
				CRect rectFloat = m_abs.m_rcFloat;
				
				CPoint ptOffset = point - m_ptDragBegin;
				rectFloat.OffsetRect (ptOffset);
				
				// Adjust window position to fit the cursor in:
				if (!rectFloat.PtInRect (ptCursor))
				{
					CRect rectCaption;
					GetCaptionRect (rectCaption);

					CPoint ptOffset = ptCursor - rectFloat.TopLeft ();
					ptOffset.x -= rectCaption.Height ();
					ptOffset.y -= rectCaption.Height ();

					rectFloat.OffsetRect (ptOffset);
				}

				m_abs.m_rcFloat = rectFloat;
				MoveWindow(m_abs.m_rcFloat, TRUE);

				// the appbar should float
				// m_abs.m_rcFloat stays unchanged untill WM_LBUTTONUP
				m_bDocked = FALSE;
				m_bHidden = FALSE;
			}
			return;

		case ABE_UNKNOWN:
		default:
			// Unregister the appbar and restore its size/position 
			// according to the float rect, which is saved in the APPBARSTATE structure
			Float ();
			ASSERT (FALSE);
			return;
		}

		
		// ---------------------------------------------
		// Finally, make sure this is an OK position
		// with the system and move the window.
		// The appbar stays undocked untill WM_LBUTTONUP
		// ---------------------------------------------
		m_abs.m_uSide = uSide;

		DoQueryPos (m_rectDrag);
		MoveWindow(m_rectDrag, TRUE);
	}

	// If the AppBar is auto-hidden, show it when the user moves over the appbar
	if (!m_bMoving && !m_bSizing && m_bHidden && m_abs.m_bAutoHide)
	{
		UnHide ();
	}
	
	CWnd::OnMouseMove(nFlags, point);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	// If we are a docked, auto-hidden AppBar, shown us
	// when the user moves over our non-client area
	if (m_bHidden && m_abs.m_bAutoHide)
	{
		UnHide ();
	}
	
	CWnd::OnNcMouseMove(nHitTest, point);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	if (m_bMoving)
	{
		::ReleaseCapture ();
		::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_ARROW));
		m_bMoving = FALSE;

		// -----------------------
		// The appbar should float
		// -----------------------
		if (!m_bDocked)
		{
			// Unregister the appbar and restore its size/position 
			// according to the float rect, which is saved in the APPBARSTATE structure
			Float ();
		}

		// ---------------------------
		// The appbar should be docked
		// ---------------------------
		if (m_bAppRegistered)
		{
			// the appbar was floating or forced to be docked - restore autohiding
			BOOL bRestoreAutoHiding = !m_bAppAutoHiding && m_abs.m_bAutoHide;

			// -------------------------------------------------------------
			// If the appbar is autohidden, unhide it so we can move the bar
			// -------------------------------------------------------------
			BOOL bAutoHide = m_bAppAutoHiding;
			if (bAutoHide)
			{
				UINT uSideNew = m_abs.m_uSide;
				m_abs.m_uSide = m_uSidePrev;

				// If the appbar is autohidden, unhide it so we can move the bar
				// m_bAutoHide state flag keeps unchanged in APPBARSTATE structure 
				// so one can restore it later.
				DoSetAutoHide (FALSE, FALSE, TRUE);

				m_abs.m_uSide = uSideNew;
			}

			// ---------------------------------------
			// Move the appbar to the new screen space
			// ---------------------------------------
			DoSetPos (m_abs.m_uSide, m_rectDrag, TRUE, bAutoHide || bRestoreAutoHiding);
			m_bDocked = TRUE;

			// -----------------
			// Rehide the appbar
			// -----------------
			if (bAutoHide ||		// If the appbar was hidden, rehide it now
				bRestoreAutoHiding)	// The appbar was floating - restore autohiding
			{
				if (!DoSetAutoHide (TRUE, FALSE))
				{
					// Failed to set autohidebar - so update the screen space anyway
					DoSetPos (m_abs.m_uSide, m_rectDrag, FALSE);
				}
			}
		}

	}
	
	CWnd::OnLButtonUp(nFlags, point);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnCancelMode() 
{
	CWnd::OnCancelMode();

	if (m_bMoving)
	{
		// -----------------------------------------
		// Restore size/position for the docked mode
		// -----------------------------------------
		if (m_bAppRegistered)
		{
			m_abs.m_uSide = m_uSidePrev;
			SetWindowPos (NULL, m_rectPrev.left, m_rectPrev.top,
				m_rectPrev.Width (), m_rectPrev.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
		}
		// -------------------------------------------
		// Restore size/position for the floating mode
		// -------------------------------------------
		else
		{
			SetWindowPos (NULL, m_abs.m_rcFloat.left, m_abs.m_rcFloat.top,
				m_abs.m_rcFloat.Width (), m_abs.m_rcFloat.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
		}

		// -------------
		// Restore timer
		// -------------
		if (m_bAppAutoHiding)
		{
			SetTimer (AUTOHIDE_TIMER_ID, AUTOHIDE_TIMER_INTERVAL, NULL);
		}
	}
	
	// ------------------
	// Release capturing:
	// ------------------
	::ReleaseCapture ();
	m_bMoving = FALSE;
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::SlideWindow(LPRECT prc)
{
	const int g_dtSlideHide = 400;
	const int g_dtSlideShow = 200;
	//BOOL bChangeThreadPriority = FALSE;

	if (m_bAppRegistered)
	{
		CRect rectNew = *prc;

		BOOL bFullDragEnabled = FALSE;
		::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bFullDragEnabled, 0);

		if (!m_bDisableAnimation && bFullDragEnabled && 
			(g_dtSlideShow > 0) && (g_dtSlideHide > 0))	
		{
			CRect rectOld;
			GetWindowRect(&rectOld);

			int dx = (rectNew.right - rectOld.right) + (rectNew.left - rectOld.left);
			int dy = (rectNew.bottom - rectOld.bottom) + (rectNew.top - rectOld.top);

			BOOL bEnlarging = 
					(rectNew.bottom - rectNew.top) > (rectOld.bottom - rectOld.top) ||
					(rectNew.right - rectNew.left) > (rectOld.right - rectOld.left);

			if (bEnlarging)
			{
				rectOld = rectNew;
				rectOld.OffsetRect (-dx, -dy);
				SetWindowPos(NULL, rectOld.left, rectOld.top,
						rectOld.Width (), rectOld.Height (),
						SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
			}

			int t0 = GetTickCount();
			int dt = g_dtSlideShow;
			int t;
			while ((t = GetTickCount()) < t0 + dt)
			{
				int x = rectOld.left + dx * (t - t0) / dt;
				int y = rectOld.top + dy * (t - t0) / dt;

				SetWindowPos(NULL, x, y, 0, 0,
							 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				if (bEnlarging)
					UpdateWindow ();
				else
					::UpdateWindow(::GetDesktopWindow());
			}
		}

		SetWindowPos(NULL, rectNew.left, rectNew.top,
					 rectNew.right - rectNew.left, rectNew.bottom - rectNew.top,
					 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	ASSERT_VALID (this);

	CRect rectBorderSize;
	rectBorderSize.SetRectEmpty ();
	CalcBorderSize (rectBorderSize);

	lpncsp->rgrc[0].top += m_nCaptionHeight + rectBorderSize.top;
	lpncsp->rgrc[0].bottom -= rectBorderSize.bottom;
	lpncsp->rgrc[0].left += rectBorderSize.left;
	lpncsp->rgrc[0].right -= rectBorderSize.right;
	
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnNcPaint() 
{
	CWindowDC dc(this); // device context for painting
	
	CDC*		pDC = &dc;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;

	CRect rectWindow;
	GetWindowRect (rectWindow);
	CRect rect;
	rect.SetRect (0, 0, rectWindow.Width(), rectWindow.Height());

	if (dcMem.CreateCompatibleDC (&dc) &&
		bmp.CreateCompatibleBitmap (&dc, rect.Width (),
								  rect.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;
	}

    // client area is not our bussiness :)
    CRect rcClient, rcBar;
    GetWindowRect(rcBar);

    GetClientRect(rcClient);
    ClientToScreen(rcClient);
    rcClient.OffsetRect(-rcBar.TopLeft());

    dc.ExcludeClipRect (rcClient);
	
//	CRgn rgn;
//	if (!m_rectRedraw.IsRectEmpty ())
//	{
//		rgn.CreateRectRgnIndirect (m_rectRedraw);
//		dc.SelectClipRgn (&rgn);
//	}

	// draw border
	OnDrawBorder (pDC);

	// draw caption
	OnDrawCaption (pDC);

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		CRect rectClip;
		int nClipType = dc.GetClipBox (rectClip);
		if (nClipType != NULLREGION)
		{
			if (nClipType != SIMPLEREGION)
			{
				rectClip = rect;
			}

			dc.BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
						   &dcMem, rectClip.left, rectClip.top, SRCCOPY);
		}

		dcMem.SelectObject(pOldBmp);
	}

	// Do not call CWnd::OnNcPaint() for painting messages
//	CWnd::OnNcPaint ();
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_bMoving)
	{
		if (nIDEvent == AUTOHIDE_TIMER_ID)
		{
			// If the mouse cursor is over the bar then postpone a hidding
			CRect rectWnd;
			GetWindowRect (rectWnd);

			CPoint ptCursor;
			GetCursorPos (&ptCursor);

			if (!rectWnd.PtInRect (ptCursor))
			{
				Hide ();
				KillTimer (AUTOHIDE_TIMER_ID);
			}
		}
		else if (nIDEvent == AUTOUNHIDE_TIMER_ID)
		{
			UnHide ();
			KillTimer (AUTOUNHIDE_TIMER_ID);
		}
	}
	
	CWnd::OnTimer(nIDEvent);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::CalcBorderSize (CRect& rectBorderSize) const
{
	rectBorderSize.SetRectEmpty ();

	const int cx = GetSystemMetrics (SM_CXSIZEFRAME);
	const int cy = GetSystemMetrics (SM_CYSIZEFRAME);

	switch (m_abs.m_uSide)
	{
	case ABE_LEFT:
		rectBorderSize.right = cx;
		break;

	case ABE_RIGHT:
		rectBorderSize.left = cx;
		break;

	case ABE_TOP:
		rectBorderSize.bottom = cy;
		break;

	case ABE_BOTTOM:
		rectBorderSize.top = cy;
		break;

	case ABE_FLOAT:
		rectBorderSize.SetRect (cx, cy, cx, cy);
	}
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::GetCaptionRect (CRect& rectCaption) const
{
	CRect rectBorderSize;
	rectBorderSize.SetRectEmpty ();
	CalcBorderSize (rectBorderSize);

	CRect rectWnd;
	GetWindowRect (&rectWnd);
	ScreenToClient (&rectWnd);
	rectWnd.OffsetRect (rectBorderSize.left, m_nCaptionHeight + rectBorderSize.top);

	rectCaption.SetRect (	rectWnd.left + rectBorderSize.left,
							rectWnd.top + rectBorderSize.top,
							rectWnd.right - rectBorderSize.right,
							rectWnd.top + rectBorderSize.top + m_nCaptionHeight);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnDrawBorder (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectBorderSize;
	rectBorderSize.SetRectEmpty ();
	CalcBorderSize (rectBorderSize);

	CRect rectWnd;
	GetWindowRect (&rectWnd);
	ScreenToClient (&rectWnd);
	rectWnd.OffsetRect (rectBorderSize.left, m_nCaptionHeight + rectBorderSize.top);

	CBCGVisualManager::GetInstance ()->OnDrawAppBarBorder (pDC, this, rectWnd, rectBorderSize);
}
//---------------------------------------------------------------------------------------------
void CBCGAppBarWnd::OnDrawCaption (CDC* pDC)
{
	ASSERT_VALID (this);

	CRect rectCaption;
	GetCaptionRect (rectCaption);
	
	CBCGVisualManager::GetInstance ()->OnDrawAppBarCaption (pDC, this, rectCaption, 
		GetCaptionText ());
}
//---------------------------------------------------------------------------------------------
CString CBCGAppBarWnd::GetCaptionText () const
{
	if (m_hEmbeddedBar == NULL)
	{
		return _T ("SideBar");
	}

	CString strCaption;
	CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent (m_hEmbeddedBar);
	if (pEmbeddedWnd != NULL)
	{
		pEmbeddedWnd->GetWindowText (strCaption);
	}

	return strCaption;
}
//---------------------------------------------------------------------------------------------
BCGNcHitTestType CBCGAppBarWnd::HitTest (CPoint point, BOOL bDetectCaption)
{
	CRect rectWnd;
	GetWindowRect (&rectWnd);

	if (!rectWnd.PtInRect (point))
	{
		return HTNOWHERE;
	}

	CRect rectClient;
	GetClientRect (&rectClient);
	ClientToScreen (&rectClient);

	if (rectClient.PtInRect (point))
	{
		return HTCLIENT;
	}

	// --------------------
	// HitTest the caption:
	// --------------------
	CRect rectCaption;
	GetCaptionRect (rectCaption);
	rectCaption.OffsetRect (rectWnd.TopLeft ());

	if (rectCaption.PtInRect (point))
	{
		if (bDetectCaption)
		{
			return HTCAPTION;
		}

		// HitTest caption buttons:
		// ...

		return HTCLIENT;
	}

	// ----------------
	// HitTest borders:
	// ----------------
	CRect rectBorderSize;
	rectBorderSize.SetRectEmpty ();
	CalcBorderSize (rectBorderSize);

	BOOL bEnableCornerArrows = IsConerArrowsEnabled ();

	int nCursorWidth  = bEnableCornerArrows ? (GetSystemMetrics (SM_CXCURSOR) / 2) : 0;
	int nCursorHeight = bEnableCornerArrows ? (GetSystemMetrics (SM_CYCURSOR) / 2) : 0;

	CRect rectBorder;

	// top left corner - border
	if (bEnableCornerArrows)
	{
		rectBorder.SetRect (rectWnd.left, rectWnd.top, rectWnd.left + nCursorWidth, rectWnd.top + nCursorHeight);
		if (rectBorder.PtInRect (point))
		{
			return HTTOPLEFT;
		}
	}

	// top border
	rectBorder.SetRect (rectWnd.left + nCursorWidth, rectWnd.top, 
				  rectWnd.right - nCursorWidth, rectWnd.top + rectBorderSize.top);
	if (rectBorder.PtInRect (point))
	{
		return HTTOP;
	}

	// top right border
	if (bEnableCornerArrows)
	{
		rectBorder.SetRect (rectWnd.right - nCursorWidth, rectWnd.top, 
					  rectWnd.right, rectWnd.top + nCursorHeight);
		if (rectBorder.PtInRect (point))
		{
			return HTTOPRIGHT;
		}
	}

	// right border
	rectBorder.SetRect (rectWnd.right - rectBorderSize.right, rectWnd.top + nCursorHeight, 
				  rectWnd.right, rectWnd.bottom - nCursorHeight);
	if (rectBorder.PtInRect (point))
	{
		return HTRIGHT;
	}

	// bottom right
	if (bEnableCornerArrows)
	{
		rectBorder.SetRect (rectWnd.right - nCursorWidth, rectWnd.bottom - nCursorHeight, 
					  rectWnd.right, rectWnd.bottom);
		if (rectBorder.PtInRect (point))
		{
			return HTBOTTOMRIGHT;
		}
	}

	// bottom
	rectBorder.SetRect (rectWnd.left + nCursorWidth, rectWnd.bottom - rectBorderSize.bottom, 
				  rectWnd.right - nCursorWidth, rectWnd.bottom);
	if (rectBorder.PtInRect (point))
	{
		return HTBOTTOM;
	}

	// bottom left
	if (bEnableCornerArrows)
	{
		rectBorder.SetRect (rectWnd.left, rectWnd.bottom - nCursorHeight, 
					  rectWnd.left + nCursorWidth, rectWnd.bottom);
		if (rectBorder.PtInRect (point))
		{
			return HTBOTTOMLEFT;
		}
	}

	// left
	rectBorder.SetRect (rectWnd.left, rectWnd.top + nCursorHeight, 
				  rectWnd.left + rectBorderSize.left, rectWnd.bottom - nCursorHeight);

	if (rectBorder.PtInRect (point))
	{
		return HTLEFT;
	}

	return CWnd::OnNcHitTest(point);
}
//---------------------------------------------------------------------------------------------
LRESULT CBCGAppBarWnd::OnIdleUpdateCmdUI(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	SendMessageToDescendants(WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0, TRUE, TRUE);

	return 0L;
}
//*************************************************************************************
BOOL CBCGAppBarWnd::LoadState (LPCTSTR lpszProfileName, int nIndex)
{
	CString strProfileName = ::BCGGetRegPath (strAppBarsProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	reg.Read (_T ("IsAutoHide"), m_abs.m_bAutoHide);
	reg.Read (_T ("IsAlwaysOnTop"), m_abs.m_bAlwaysOnTop);
	reg.Read (_T ("Side"), (int&) m_abs.m_uSide);
	reg.Read (_T ("RectFloat"), m_abs.m_rcFloat);

	for (int i = 0; i < 4; i++)
	{
		CString strKey;
		strKey.Format (_T("DimsDock%d"), i);

		reg.Read (strKey, (int&) m_abs.m_auDimsDock [i]);
	}

	if (GetSafeHwnd () != NULL)
	{
		if (m_abs.m_uSide == ABE_FLOAT)
		{
			Float ();
		}
		else
		{
			SetSide (m_abs.m_uSide);
		}
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGAppBarWnd::SaveState (LPCTSTR lpszProfileName, int nIndex)
{
	CString strProfileName = ::BCGGetRegPath (strAppBarsProfile, lpszProfileName);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);

	CBCGRegistrySP regSP;
	CBCGRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (strSection))
	{
		return FALSE;
	}

	reg.Write (_T ("IsAutoHide"), m_abs.m_bAutoHide);
	reg.Write (_T ("IsAlwaysOnTop"), m_abs.m_bAlwaysOnTop);
	reg.Write (_T ("Side"), (int) m_abs.m_uSide);
	reg.Write (_T ("RectFloat"), m_abs.m_rcFloat);

	for (int i = 0; i < 4; i++)
	{
		CString strKey;
		strKey.Format (_T("DimsDock%d"), i);

		reg.Write (strKey, (int) m_abs.m_auDimsDock [i]);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGAppBarWnd::SetFlags (DWORD dwFlags)
{
	ASSERT_VALID (this);
	m_dwFlags = dwFlags;
}
//****************************************************************************************
void CBCGAppBarWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CWnd::OnSettingChange(uFlags, lpszSection);

	if (!m_bDesktopChanging)
	{
		UINT uSide = GetSide ();
		SetSide (uSide);
	}
}
