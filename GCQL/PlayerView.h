#if !defined(AFX_PLAYERVIEW_H__5FBEAC63_8A3C_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_PLAYERVIEW_H__5FBEAC63_8A3C_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayerView.h : header file
//

#include "PlayerList.h"

/////////////////////////////////////////////////////////////////////////////
// CPlayerView view

class CPlayerView : public CView
{
protected:
	CPlayerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPlayerView)

// Attributes
public:
	CPlayerList		m_wndPlayerList;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerView)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPlayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERVIEW_H__5FBEAC63_8A3C_11D5_BA96_00C0DF22DE85__INCLUDED_)
