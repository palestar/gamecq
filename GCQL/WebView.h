#if !defined(AFX_WEBVIEW_H__CFE79A65_7452_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_WEBVIEW_H__CFE79A65_7452_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WebView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWebView html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>

class CWebView : public CHtmlView
{
protected:
	CWebView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CWebView)

// html Data
public:
	//{{AFX_DATA(CWebView)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	static CWebView *	getWebView();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWebView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	virtual ~CWebView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CWebView)
	afx_msg void OnNavigationBack();
	afx_msg void OnNavigationForward();
	afx_msg void OnNavigationRefresh();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEBVIEW_H__CFE79A65_7452_11D5_BA96_00C0DF22DE85__INCLUDED_)
