#if !defined(AFX_EULA_H__0B1FB461_EABE_11D4_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_EULA_H__0B1FB461_EABE_11D4_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Eula.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEula dialog

class CEula : public CDialog
{
// Construction
public:
	CEula(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEula)
	enum { IDD = IDD_EULA };
	CEdit	m_EulaControl;
	CString	m_Eula;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEula)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEula)
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EULA_H__0B1FB461_EABE_11D4_BA92_00C0DF22DE85__INCLUDED_)
