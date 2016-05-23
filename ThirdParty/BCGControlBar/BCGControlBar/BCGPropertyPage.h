#if !defined(AFX_BCGPROPERTYPAGE_H__10512154_37FE_41AA_B469_7D5D3A28BEB7__INCLUDED_)
#define AFX_BCGPROPERTYPAGE_H__10512154_37FE_41AA_B469_7D5D3A28BEB7__INCLUDED_

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
//
// BCGPropertyPage.h : header file
//

#include "bcgcontrolbar.h"
#include "bcgdlgimpl.h"

class CBCGPropSheetCategory;

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertyPage dialog

class BCGCONTROLBARDLLEXPORT CBCGPropertyPage : public CPropertyPage
{
	friend class CBCGPopupMenu;
	friend class CBCGContextMenuManager;
	friend class CBCGPropertySheet;

	DECLARE_DYNCREATE(CBCGPropertyPage)

// Construction
public:
	CBCGPropertyPage();
	CBCGPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0);
	CBCGPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption = 0);
	~CBCGPropertyPage();

// Dialog Data
	//{{AFX_DATA(CBCGPropertyPage)
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPropertyPage)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnSetActive();
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPropertyPage)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetActiveMenu (CBCGPopupMenu* pMenu);
	void CommonInit ();

	CBCGDlgImpl				m_Impl;
	CBCGPropSheetCategory*	m_pCategory;
	int						m_nIcon;
	int						m_nSelIconNum;
	HTREEITEM				m_hTreeNode;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPROPERTYPAGE_H__10512154_37FE_41AA_B469_7D5D3A28BEB7__INCLUDED_)
