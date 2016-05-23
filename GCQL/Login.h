#if !defined(AFX_LOGIN_H__797CB08D_D68B_11D4_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_LOGIN_H__797CB08D_D68B_11D4_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Login.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogin dialog
//#define CDialog		CBCGDialog

class CLogin : public CBCGDialog
{
// Construction
public:
	CLogin(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLogin)
	enum { IDD = IDD_LOGIN };
	CComboBox	m_UIDControl;
	CString	m_PW;
	CString	m_UID;
	BOOL	m_RememberPW;
	BOOL	m_AutoLogin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogin)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLogin)
	afx_msg void OnNewLogin();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL	m_bIsAutomatedLogin;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGIN_H__797CB08D_D68B_11D4_BA92_00C0DF22DE85__INCLUDED_)
