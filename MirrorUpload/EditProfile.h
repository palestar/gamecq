#if !defined(AFX_EDITPROFILE_H__BD59B2BF_DFAE_47AA_B0A0_AE308AD8FAA6__INCLUDED_)
#define AFX_EDITPROFILE_H__BD59B2BF_DFAE_47AA_B0A0_AE308AD8FAA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditProfile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditProfile dialog

class CEditProfile : public CDialog
{
// Construction
public:
	CEditProfile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditProfile)
	enum { IDD = IDD_EDIT_PROFILE };
	CString	m_sName;
	CString	m_sLocalPath;
	CString	m_sAddress;
	int		m_nPort;
	CString	m_sUser;
	CString	m_sPassword;
	CString	m_sConfirmPassword;
	CString	m_sLogFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditProfile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditProfile)
	virtual void OnOK();
	afx_msg void OnBrowsePath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPROFILE_H__BD59B2BF_DFAE_47AA_B0A0_AE308AD8FAA6__INCLUDED_)
