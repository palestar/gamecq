#if !defined(AFX_FriendList_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_FriendList_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FriendList.h : header file
//

#include "TFXDataTip.h"
#include "GCQ/MetaClient.h"

/////////////////////////////////////////////////////////////////////////////
// CFriendList window

class CFriendList : public CListCtrl
{
// Construction
public:
	CFriendList();

// Attributes
public:
	CImageList		m_AvatarIcons;

// Operations
	void checkForToolTip( CPoint point );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFriendList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFriendList();

private:
	TFXDataTip		m_Tip;
	int				m_OldY;

	// Generated message map functions
protected:
	//{{AFX_MSG(CFriendList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FriendList_H__A750B0A5_76E1_11D5_BA96_00C0DF22DE85__INCLUDED_)
