#if !defined(AFX_FIELDVALUE_H__60FA9AEA_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_FIELDVALUE_H__60FA9AEA_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FieldValue.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFieldValue dialog

class CFieldValue : public CDialog
{
// Construction
public:
	CFieldValue(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFieldValue)
	enum { IDD = IDD_PROFILE_FIELD };
	CString	m_Value;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldValue)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFieldValue)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDVALUE_H__60FA9AEA_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_)
