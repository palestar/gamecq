#if !defined(AFX_BCGPROPERTYSHEET_H__607F72FB_BD7B_4264_BDEF_3C535162B0C3__INCLUDED_)
#define AFX_BCGPROPERTYSHEET_H__607F72FB_BD7B_4264_BDEF_3C535162B0C3__INCLUDED_

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
// BCGPropertySheet.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "bcgcontrolbar.h"
#include "BCGOutlookBar.h"
#include "BCGTabWnd.h"
#include "BCGDlgImpl.h"

class CBCGPropertySheet;
class CBCGPropertyPage;

/////////////////////////////////////////////////////////////////////////////
// CBCGPropSheetBar

class CBCGPropSheetBar : public CBCGOutlookBar
{
	friend class CBCGPropertySheet;

	virtual BOOL OnSendCommand (const CBCGToolbarButton* pButton);
	void EnsureVisible (int iButton);

	CBCGPropertySheet* m_pParent;
};

class CBCGPropSheetTab : public CBCGTabWnd
{
	friend class CBCGPropertySheet;

	CBCGPropSheetTab();

	virtual BOOL SetActiveTab (int iTab);

	CBCGPropertySheet* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPropSheetCategory

class BCGCONTROLBARDLLEXPORT CBCGPropSheetCategory : public CObject
{
	friend class CBCGPropertySheet;

	DECLARE_DYNAMIC(CBCGPropSheetCategory)

	CBCGPropSheetCategory(LPCTSTR lpszName, int nIcon, int nSelectedItem,
		const CBCGPropSheetCategory* pParentCategory);
	virtual ~CBCGPropSheetCategory();

	const CString					m_strName;
	const int						m_nIcon;
	const int						m_nSelectedIcon;
	const CBCGPropSheetCategory*	m_pParentCategory;
	HTREEITEM						m_hTreeItem;

	CList<CBCGPropSheetCategory*,CBCGPropSheetCategory*>	m_lstSubCategories;
	CList<CBCGPropertyPage*, CBCGPropertyPage*>				m_lstPages;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPropertySheet

class BCGCONTROLBARDLLEXPORT CBCGPropertySheet : public CPropertySheet
{
	friend class CBCGPropSheetBar;

	DECLARE_DYNAMIC(CBCGPropertySheet)

// Construction
public:
	CBCGPropertySheet();
	CBCGPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CBCGPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	enum PropSheetLook
	{
		PropSheetLook_Tabs,
		PropSheetLook_OutlookBar,
		PropSheetLook_Tree,
		PropSheetLook_OneNoteTabs
	};

	// Should be called BEFORE DoModal or Create!
	void SetLook (PropSheetLook look, int nNavControlWidth = 100);

// Attributes
public:
	CBCGTabWnd& GetTab () const;	// for PropSheetLook_OneNoteTabs only

protected:
	PropSheetLook		m_look;
	CBCGPropSheetBar	m_wndOutlookBar;
	CBCGPropSheetTab	m_wndTab;
	CTreeCtrl			m_wndTree;
	int					m_nBarWidth;
	int					m_nActivePage;
	CImageList			m_Icons;
	BOOL				m_bAlphaBlendIcons;

	CList<CBCGPropSheetCategory*,CBCGPropSheetCategory*>	m_lstTreeCategories;

	BOOL				m_bIsInSelectTree;
	CBCGDlgImpl			m_Impl;

// Operations
public:
	BOOL SetIconsList (UINT uiImageListResID, int cx, COLORREF clrTransparent = RGB (255, 0, 255));
	void SetIconsList (HIMAGELIST hIcons);

	void AddPage(CPropertyPage* pPage);

	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);

	// PropSheetLook_Tree methods:
	CBCGPropSheetCategory* AddTreeCategory (LPCTSTR lpszLabel, 
		int nIconNum = -1, int nSelectedIconNum = -1,
		const CBCGPropSheetCategory* pParentCategory = NULL);
	void AddPageToTree (CBCGPropSheetCategory* pCategory, 
		CBCGPropertyPage* pPage, int nIconNum = -1, int nSelIconNum = -1);

// Overrides
	virtual void OnActivatePage (CPropertyPage* pPage);
	virtual CWnd* InitNavigationControl ();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPropertySheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPropertySheet)
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	afx_msg LRESULT OnAfterActivatePage(WPARAM,LPARAM);
	afx_msg void OnSelectTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	void InternalAddPage (int nTab);
	void AddCategoryToTree (CBCGPropSheetCategory* pCategory);
	void CommonInit ();
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPROPERTYSHEET_H__607F72FB_BD7B_4264_BDEF_3C535162B0C3__INCLUDED_)
