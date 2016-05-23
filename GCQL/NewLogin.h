#if !defined(AFX_NEWLOGIN_H__DFF07725_7541_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_NEWLOGIN_H__DFF07725_7541_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewLogin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewLogin dialog

class CNewLogin : public CDialog
{
// Construction
public:
	CNewLogin(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewLogin)
	enum { IDD = IDD_NEWLOGIN };
	CString	m_UID;
	CString	m_PW;
	CString	m_PW2;
	CString	m_EMail;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewLogin)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewLogin)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWLOGIN_H__DFF07725_7541_11D5_BA96_00C0DF22DE85__INCLUDED_)
