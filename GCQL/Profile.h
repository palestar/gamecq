#if !defined(AFX_PROFILE_H__60FA9AE6_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_)
#define AFX_PROFILE_H__60FA9AE6_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Profile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProfile dialog

#include "GCQ/MetaClient.h"

class CProfile : public CDialog
{
// Construction
public:
	CProfile(CWnd* pParent = NULL);   // standard constructor
	
	MetaClient::Profile 
			m_Profile;

// Dialog Data
	//{{AFX_DATA(CProfile)
	enum { IDD = IDD_PROFILE };
	CListCtrl	m_Fields;
	int		m_ProfileID;
	int		m_ClanID;
	BOOL	m_Server;
	BOOL	m_Moderator;
	CString	m_Alias;
	CString	m_Email;
	BOOL	m_Administrator;
	BOOL	m_News;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProfile)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEditField(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROFILE_H__60FA9AE6_A9B8_11D4_BA92_00C0DF22DE85__INCLUDED_)
