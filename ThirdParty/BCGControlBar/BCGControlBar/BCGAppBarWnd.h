#if !defined(AFX_BCGAPPBARWND_H__553A0D63_DBB6_47F8_A9DD_2FA98300CE0A__INCLUDED_)
#define AFX_BCGAPPBARWND_H__553A0D63_DBB6_47F8_A9DD_2FA98300CE0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGAppBarWnd.h : header file
//

#include "bcgcontrolbar.h"
#include <shellapi.h>

// Registered message for the AppBar's callback notifications
extern BCGCONTROLBARDLLEXPORT UINT BCGM_APPBAR_CALLBACK;

// ----------------------
// The AppBar edge state:
// ----------------------

// Defined in SHELLAPI.H:
// #define ABE_LEFT			0
// #define ABE_TOP			1
// #define ABE_RIGHT		2
// #define ABE_BOTTOM		3

#define ABE_UNKNOWN			((UINT) -1)
#define ABE_FLOAT			((UINT) -2)


// ----------------
// AppBar messages:
// ----------------

// Defined in SHELLAPI.H:
// #define ABM_NEW				0x00000000
// #define ABM_REMOVE			0x00000001
// #define ABM_QUERYPOS			0x00000002
// #define ABM_SETPOS			0x00000003
// #define ABM_GETSTATE			0x00000004
// #define ABM_GETTASKBARPOS	0x00000005
// #define ABM_ACTIVATE			0x00000006
// #define ABM_GETAUTOHIDEBAR	0x00000007
// #define ABM_SETAUTOHIDEBAR	0x00000008
// #define ABM_WINDOWPOSCHANGED	0x00000009


// ----------------------
// AppBar behavior flags:
// ----------------------

#define ABF_ALLOWLEFTRIGHT		0x00000001	// Allow dock at the left/right of the screen
#define ABF_ALLOWTOPBOTTOM		0x00000002	// Allow dock at the top/bottom of the screen
#define ABF_ALLOWANYEDGE		(ABF_ALLOWLEFTRIGHT | ABF_ALLOWTOPBOTTOM)
#define ABF_ALLOWFLOAT			0x00000004	// Allow floating mode
#define ABF_ALLOWANYWHERE		(ABF_ALLOWANYEDGE | ABF_ALLOWFLOAT)
#define ABF_ALLOWAUTOHIDE		0x00000010	// Follow Autohide state of TaskBar
#define ABF_ALLOWALWAYSONTOP	0x00000020	// Follow AlwaysOnTop state of TaskBar



/////////////////////////////////////////////////////////////////////////////
// CBCGAppBarWnd window

class BCGCONTROLBARDLLEXPORT CBCGAppBarWnd : public CWnd
{
	DECLARE_SERIAL(CBCGAppBarWnd)

// Construction
public:
	CBCGAppBarWnd();
	virtual ~CBCGAppBarWnd();

// Attributes
public:
	void SetFlags (DWORD dwFlags);
	DWORD GetFlags () const
	{
		return m_dwFlags;
	}

protected:
	// ---------------
	// AppBar settings
	// ---------------
	typedef struct BCGCONTROLBARDLLEXPORT tagAPPBARSTATE
	{
		BOOL m_bAutoHide;		// TRUE, if the appbar should autohide when docked
		BOOL m_bAlwaysOnTop;	// TRUE, if the appbar should be the topmost window
		UINT m_uSide;			// ABE_LEFT, ABE_TOP, ABE_RIGHT, ABE_BOTTOM
		UINT m_auDimsDock[4];	// Width/height for docked bar on 4 edges
		CRect m_rcFloat;		// Window rect for floating mode (in screen coordinates)
	} APPBARSTATE, *PAPPBARSTATE;
	APPBARSTATE	m_abs;

	BOOL	m_bDisableAnimation;
	int		m_nCaptionHeight;
	HWND	m_hEmbeddedBar;
	DWORD	m_dwFlags;          // ABF_* AppBar behavior flags

	
	// ---------------------------------------
	// Internal implementation state variables
	// ---------------------------------------
	BOOL	m_bAppRegistered;	// TRUE if the appbar is registered
	BOOL	m_bAppAutoHiding;	// TRUE if the appbar is registered as AutoHide bar
	BOOL	m_bHidden;			// TRUE if the appbar is autohiding and hidden
	BOOL	m_bDocked;			// TRUE if the appbar reserves some screen space at the screen edge

	// used when moving
	BOOL	m_bMoving;			// TRUE, when an user is moving the appbar
	CRect	m_rectDrag;			// current dragging rectangle
	CPoint	m_ptDragBegin;		// saved cursor position
	UINT	m_uSidePrev;		// the screen edge the appbar was docked to

	// used when sizing and autohiding
	BOOL	m_bSizing;			// TRUE, when an user is sizing the appbar
	CRect	m_rectPrev;			// saved window rect
	BOOL	m_bAllowResize;		// used in OnSizeMove handler

	// used in OnSettingChange to prevent loop
	BOOL	m_bDesktopChanging; // TRUE, when the appbar forces the WM_SETTINGCHANGE

	// ---------
	// Constants
	// ---------
	static UINT m_uAppBarNotifyMsg;

	enum {	AUTOHIDE_TIMER_ID = 1,			AUTOUNHIDE_TIMER_ID = 2, 
			AUTOHIDE_TIMER_INTERVAL = 500,	AUTOUNHIDE_TIMER_INTERVAL = 200	};

	enum {	CX_DEFWIDTH = 80, CY_DEFHEIGHT = 60 }; // default appbar width/height


// Operations
public:

	// ---------------------------
	// Appbar specific operations:
	// ---------------------------
	BOOL Create (LPCTSTR lpszWindowName, CRect rect, CWnd* pParentWnd = NULL);
	BOOL SetSide (UINT uSide); // uSide can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, ABE_RIGHT, ABE_FLOAT, or ABE_UNKNOWN
	BOOL Float ();
	BOOL Float (CRect rect);
	void Hide ();
	void UnHide ();

	void SetAlwaysOnTop (BOOL bEnable);

	BOOL GetAlwaysOnTop () const
	{
		return m_abs.m_bAlwaysOnTop;
	}

	BOOL SetAutoHide (BOOL bEnable);

	BOOL GetAutoHide () const
	{
		return m_abs.m_bAutoHide;
	}

	UINT GetSide () const
	{
		return m_abs.m_uSide;
	}

	virtual BOOL LoadState (LPCTSTR lpszProfileName, int nIndex = 0);
	virtual BOOL SaveState (LPCTSTR lpszProfileName, int nIndex = 0);

	// --------------------------
	// Appbar wrapping functions:
	// --------------------------
	BOOL Register ();									// wrapper for ABM_NEW
	BOOL UnRegister ();									// wrapper for ABM_REMOVE
	void QueryPos (CRect& rect);						// wrapper for ABM_QUERYPOS
	void SetPos (CRect& rect);							// wrapper for ABM_SETPOS
	void GetPos (CRect& rect);							// wrapper for ABM_GETTASKBARPOS
	DWORD GetState ();									// wrapper for ABM_GETSTATE
	HWND GetAutoHideBar (UINT uEdge);					// wrapper for ABM_GETAUTOHIDEBAR
	BOOL SetAutoHideBar (UINT uEdge, BOOL bRegister);	// wrapper for ABM_SETAUTOHIDEBAR
														//	uEdge can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT

	static PAPPBARSTATE GetAppbarState(HWND hwnd);

protected:
	// --------------
	// Implemetation:
	// --------------
	void DoQueryPos (CRect& rect);
	void DoSetPos (UINT uEdge, CRect& rect,	// uEdge can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT
				   BOOL bMove, BOOL bNoSizeMove = FALSE); 

	BOOL DoSetSide (UINT uSide,				// uSide can be ABE_TOP, ABE_BOTTOM, ABE_LEFT, or ABE_RIGHT
					BOOL bMove, BOOL bNoSizeMove = FALSE);
	BOOL DoSetAutoHide (BOOL bEnable,
						BOOL bMove, BOOL bNoSizeMove = FALSE);

	void SlideWindow(LPRECT prc);

	
	// ----------------------
	// Overridable functions:
	// ----------------------
public:
	virtual void CalcBorderSize (CRect& rectBorderSize) const;
	virtual void GetCaptionRect (CRect& rectCaption) const;
	virtual CString GetCaptionText () const;

protected:
	virtual void OnDrawBorder (CDC* pDC);
	virtual void OnDrawCaption (CDC* pDC);

	virtual BCGNcHitTestType HitTest (CPoint point, BOOL bDetectCaption);
	virtual BOOL IsConerArrowsEnabled () const	{ return !m_bAppRegistered; }

	virtual void GetScreenRect (CRect &rectScreen) const;

public:
	// -----------------
	// Message handlers:
	// -----------------
	LRESULT AppBarCallback (WPARAM wParam, LPARAM lParam);

	void OnSizeMove ();

	// ---------------------
	// AppBar Notifications:
	// ---------------------
	virtual void OnAppBarPosChanged ();
	virtual void OnAppBarFullScreenApp (BOOL bOpen);
	virtual void OnAppBarWindowArrange (BOOL bBeginning);
	virtual void OnAppBarStateChange (BOOL bAutoHide, BOOL bAlwaysOnTop);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGAppBarWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGAppBarWnd)
	afx_msg void OnDestroy();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCancelMode();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnEnterSizeMove(WPARAM,LPARAM);
	afx_msg LRESULT OnExitSizeMove(WPARAM,LPARAM);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGAPPBARWND_H__553A0D63_DBB6_47F8_A9DD_2FA98300CE0A__INCLUDED_)
