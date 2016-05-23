#if !defined(AFX_CHANGEEMAIL_H__EC533742_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHANGEEMAIL_H__EC533742_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeEmail.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeEmail dialog

class CChangeEmail : public CDialog
{
// Construction
public:
	CChangeEmail(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangeEmail)
	enum { IDD = IDD_CHANGE_EMAIL };
	CString	m_Email;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeEmail)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeEmail)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEEMAIL_H__EC533742_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_)
