#if !defined(AFX_BCGSPINBUTTONCTRL_H__F2D4A019_5942_4EF8_8B6A_8451028845B0__INCLUDED_)
#define AFX_BCGSPINBUTTONCTRL_H__F2D4A019_5942_4EF8_8B6A_8451028845B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGSpinButtonCtrl.h : header file
//

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
/////////////////////////////////////////////////////////////////////////////
// CBCGSpinButtonCtrl window

#include "bcgcontrolbar.h"

class BCGCONTROLBARDLLEXPORT CBCGSpinButtonCtrl : public CSpinButtonCtrl
{
// Construction
public:
	CBCGSpinButtonCtrl();

// Attributes
protected:
	BOOL	m_bIsButtonPressedUp;
	BOOL	m_bIsButtonPressedDown;

	BOOL	m_bIsButtonHighligtedUp;
	BOOL	m_bIsButtonHighligtedDown;

	BOOL	m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGSpinButtonCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGSpinButtonCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGSpinButtonCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGSPINBUTTONCTRL_H__F2D4A019_5942_4EF8_8B6A_8451028845B0__INCLUDED_)
