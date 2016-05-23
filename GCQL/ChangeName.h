#if !defined(AFX_CHANGENAME_H__EC533741_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CHANGENAME_H__EC533741_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeName dialog

class CChangeName : public CDialog
{
// Construction
public:
	CChangeName(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangeName)
	enum { IDD = IDD_CHANGE_NAME };
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeName)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGENAME_H__EC533741_8BD1_11D5_BA96_00C0DF22DE85__INCLUDED_)
