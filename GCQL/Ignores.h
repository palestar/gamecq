#if !defined(AFX_IGNORES_H__F9C63E21_BBF8_11D5_BA97_00C0DF22DE85__INCLUDED_)
#define AFX_IGNORES_H__F9C63E21_BBF8_11D5_BA97_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ignores.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIgnores dialog

class CIgnores : public CDialog
{
// Construction
public:
	CIgnores(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIgnores)
	enum { IDD = IDD_IGNORES };
	CListCtrl	m_IgnoreList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIgnores)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIgnores)
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IGNORES_H__F9C63E21_BBF8_11D5_BA97_00C0DF22DE85__INCLUDED_)
