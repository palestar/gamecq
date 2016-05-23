#if !defined(AFX_BCGPOPUPDLG_H__9EC5BC9D_ED2B_4255_A14E_E130CF5E49CA__INCLUDED_)
#define AFX_BCGPOPUPDLG_H__9EC5BC9D_ED2B_4255_A14E_E130CF5E49CA__INCLUDED_

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
// BCGPopupDlg.h : header file
//
#include "bcgcontrolbar.h"

#include "BCGDialog.h"
#include "BCGURLLinkButton.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupWndParams

class BCGCONTROLBARDLLEXPORT CBCGPopupWndParams
{
public:
	CBCGPopupWndParams()
	{
		m_hIcon = NULL;
		m_nURLCmdID = 0;
	}

	HICON	m_hIcon;
	CString	m_strText;
	CString	m_strURL;
	UINT	m_nURLCmdID;

	CBCGPopupWndParams& operator= (CBCGPopupWndParams& src)
	{
		m_hIcon		= src.m_hIcon;
		m_strText	= src.m_strText;
		m_strURL	= src.m_strURL;
		m_nURLCmdID	= src.m_nURLCmdID;

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPopupDlg window

class CBCGPopupWindow;

class BCGCONTROLBARDLLEXPORT CBCGPopupDlg : public CBCGDialog
{
	DECLARE_DYNCREATE (CBCGPopupDlg)

	friend class CBCGPopupWindow;

// Construction
public:
	CBCGPopupDlg();

	BOOL CreateFromParams (CBCGPopupWndParams& params, CBCGPopupWindow* pParent);

// Attributes
protected:
	CBCGPopupWindow*	m_pParentPopup;
	BOOL				m_bDefault;
	CBCGPopupWndParams	m_Params;

	CStatic				m_wndIcon;
	CStatic				m_wndText;
	CBCGURLLinkButton	m_btnURL;

	CSize				m_sizeDlg;

	BOOL				m_bDontSetFocus;
	BOOL				m_bMenuIsActive;

// Operations
public:
	BOOL HasFocus () const;
	CSize GetDlgSize ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPopupDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPopupDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPopupDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	virtual void OnDraw (CDC* pDC);
	CSize GetOptimalTextSize (CString str);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOPUPDLG_H__9EC5BC9D_ED2B_4255_A14E_E130CF5E49CA__INCLUDED_)
