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

#if !defined(AFX_BCGDOCKCONTEXT_H__A19352D0_7DE0_11D3_A9DB_005056800000__INCLUDED_)
#define AFX_BCGDOCKCONTEXT_H__A19352D0_7DE0_11D3_A9DB_005056800000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGDockContext.h :
//

class CBCGPopupMenu;

/////////////////////////////////////////////////////////////////////////////
// CBCGDockContext

class CBCGDockContext : public CDockContext
{
public:
	CBCGDockContext(CControlBar* pBar);

	void Stretch(CPoint pt);
	void StartResize(int nHitTest, CPoint pt);
	BOOL Track();

public:
	virtual ~CBCGDockContext();
};

#define ROLLUP_DELAY (6) // delay before rollup (*250 milliseconds)
						// default 1.5 second.

class BCGCONTROLBARDLLEXPORT CBCGMiniDockFrameWnd : public CMiniDockFrameWnd
{
	DECLARE_DYNCREATE(CBCGMiniDockFrameWnd)

public:
	friend class CDockBar;
	friend class CBCGSizingControlBar;

	CBCGMiniDockFrameWnd();
	virtual ~CBCGMiniDockFrameWnd();

	BOOL StartTearOff (CBCGPopupMenu* pMenu);

	void Roll ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGMiniDockFrameWnd)
	public:
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CBCGMiniDockFrameWnd)
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG

	afx_msg void OnWindowPosChanging( WINDOWPOS* lpwndpos );
	afx_msg void OnNcMouseMove( UINT nHitTest, CPoint point );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

	CBCGSizingControlBar* GetSizingControlBar(); // helper to get the controlbar...

protected:
	int		m_nRollingType;
	BOOL	m_bIsRolled;
	CSize	m_LastSize;
	CRect	m_rectRollupBox;
	CRect	m_rectCaptionDynamic; // maintain the minimal dynamic size...
	int		m_RollupDelay; // for better rollup delay
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_BCGDOCKCONTEXT_H__A19352D0_7DE0_11D3_A9DB_005056800000__INCLUDED_
