#if !defined(AFX_CHATVIEW_H__614A0721_7900_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHATVIEW_H__614A0721_7900_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatView html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>
#include "mshtml.h"
#include "comdef.h"
#include "afxtempl.h"

class CChatView : public CHtmlView
{
protected:
	CChatView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CChatView)

// html Data
public:
	//{{AFX_DATA(CChatView)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	CString				m_ChatBuffer;
	CString				m_Body;
	int					m_TopChatMessage;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	virtual void OnDownloadComplete();
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	virtual void OnStatusTextChange(LPCTSTR strText);
	virtual void OnNavigateComplete2(LPCTSTR strURL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChatView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CChatView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void				ShowMemoryHtml();
	void				NavigateMemory();
	BOOL				SetCharset( LPSTR lpstrCharset);
	BOOL				SetTitle( LPSTR sTitle);
	BOOL				PutBodyContent( const TCHAR * lpstrContent);

	BOOL				m_bMemoryMode;
	LPSTR				m_lpstrCharset;
	LPSTR				m_lpstrTitle;
	IHTMLDocument2 *	m_pHtmlDoc2;
	IHTMLWindow2 *		m_pParentWindow;
	const TCHAR *		m_lpstrBodyContent;

	byte				m_UpdateBlockedTime;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATVIEW_H__614A0721_7900_11D5_BA96_00C0DF22DE85__INCLUDED_)
