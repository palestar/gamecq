#if !defined(AFX_CREATEROOM_H__01FC8A61_8A42_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_CREATEROOM_H__01FC8A61_8A42_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreateRoom.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateRoom dialog

class CCreateRoom : public CDialog
{
// Construction
public:
	CCreateRoom(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateRoom)
	enum { IDD = IDD_CREATE_ROOM };
	CEdit	m_PasswordControl;
	CEdit	m_NameControl;
	CString	m_Name;
	BOOL	m_PasswordProtect;
	CButton	m_StaticControl;
	CButton m_HiddenControl;
	CButton m_ModeratedControl;

	CString	m_Password;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateRoom)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateRoom)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPasswordProtect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEROOM_H__01FC8A61_8A42_11D5_BA96_00C0DF22DE85__INCLUDED_)
