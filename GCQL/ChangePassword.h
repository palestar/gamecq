#if !defined(AFX_CHANGEPASSWORD_H__C514B7C3_7D2C_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHANGEPASSWORD_H__C514B7C3_7D2C_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangePassword.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangePassword dialog

class CChangePassword : public CDialog
{
// Construction
public:
	CChangePassword(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangePassword)
	enum { IDD = IDD_CHANGE_PASSWORD };
	CString	m_NewPassword;
	CString	m_ConfirmPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangePassword)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangePassword)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEPASSWORD_H__C514B7C3_7D2C_11D5_BA96_00C0DF22DE85__INCLUDED_)
